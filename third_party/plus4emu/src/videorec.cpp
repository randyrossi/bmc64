
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "plus4emu.hpp"
#include "display.hpp"
#include "snd_conv.hpp"
#include "videorec.hpp"
#include "system.hpp"

#include <cmath>

static const size_t aviHeaderSize_RLE8 = 0x0546;
static const size_t aviHeaderSize_YV12 = 0x0146;

namespace Plus4Emu {

  VideoCapture::AudioConverter_::AudioConverter_(VideoCapture& videoCapture_,
                                                 float inputSampleRate_,
                                                 float outputSampleRate_,
                                                 float dcBlockFreq1,
                                                 float dcBlockFreq2,
                                                 float ampScale_)
    : AudioConverterHighQuality(inputSampleRate_, outputSampleRate_,
                                dcBlockFreq1, dcBlockFreq2, ampScale_),
      videoCapture(videoCapture_)
  {
  }

  VideoCapture::AudioConverter_::~AudioConverter_()
  {
  }

  void VideoCapture::AudioConverter_::audioOutput(int16_t outputSignal_)
  {
    if (videoCapture.audioBufSamples
        < (videoCapture.audioBufSize * videoCapture.audioBuffers)) {
      videoCapture.audioBuf[videoCapture.audioBufWritePos++] = outputSignal_;
      if (videoCapture.audioBufWritePos
          >= (videoCapture.audioBufSize * videoCapture.audioBuffers)) {
        videoCapture.audioBufWritePos = 0;
      }
      videoCapture.audioBufSamples++;
    }
  }

  // --------------------------------------------------------------------------

  void VideoCapture::aviHeader_writeFourCC(uint8_t*& bufp, const char *s)
  {
    bufp[0] = uint8_t(s[0]);
    bufp[1] = uint8_t(s[1]);
    bufp[2] = uint8_t(s[2]);
    bufp[3] = uint8_t(s[3]);
    bufp = bufp + 4;
  }

  void VideoCapture::aviHeader_writeUInt16(uint8_t*& bufp, uint16_t n)
  {
    bufp[0] = uint8_t(n & 0x00FF);
    bufp[1] = uint8_t((n & 0xFF00) >> 8);
    bufp = bufp + 2;
  }

  void VideoCapture::aviHeader_writeUInt32(uint8_t*& bufp, uint32_t n)
  {
    bufp[0] = uint8_t(n & 0x000000FFU);
    bufp[1] = uint8_t((n & 0x0000FF00U) >> 8);
    bufp[2] = uint8_t((n & 0x00FF0000U) >> 16);
    bufp[3] = uint8_t((n & 0xFF000000U) >> 24);
    bufp = bufp + 4;
  }

  void VideoCapture::defaultErrorCallback(void *userData, const char *msg)
  {
    (void) userData;
    throw Exception(msg);
  }

  void VideoCapture::defaultFileNameCallback(void *userData,
                                             std::string& fileName)
  {
    (void) userData;
    fileName.clear();
  }

  VideoCapture::VideoCapture(int frameRate_)
    : aviFile((std::FILE *) 0),
      lineBuf((uint8_t *) 0),
      audioBuf((int16_t *) 0),
      frameRate(frameRate_),
      audioBufSize(0),
      audioBufReadPos(0),
      audioBufWritePos(0),
      audioBufSamples(0),
      clockFrequency(0),
      timesliceLength(0L),
      curTime(0L),
      frame0Time(-1L),
      frame1Time(0L),
      soundOutputAccumulator(0),
      cycleCnt(0),
      interpTime(0),
      curLine(0),
      vsyncCnt(0),
      oddFrame(false),
      burstValue(0x08),
      syncLengthCnt(0U),
      hsyncCnt(0U),
      hsyncPeriodLength(570U),
      lineLengthCnt(0U),
      lineLength(570U),
      lineStart(80U),
      hsyncPeriodMin(494U),
      hsyncPeriodMax(646U),
      lineLengthMin(513U),
      lineLengthMax(627U),
      lineLengthFilter(570.0f),
      vsyncThreshold1(335),
      vsyncThreshold2(261),
      vsyncReload(-19),
      lineReload(0),
      lineBufBytes(0),
      lineBufLength(0),
      lineBufFlags(0x00),
      framesWritten(0),
      duplicateFrames(0),
      fileSize(0),
      displayParameters(),
      audioConverter((AudioConverter *) 0),
      aviHeaderSize(0),
      errorCallback(&defaultErrorCallback),
      errorCallbackUserData((void *) this),
      fileNameCallback(&defaultFileNameCallback),
      fileNameCallbackUserData((void *) this)
  {
    try {
      frameRate = (frameRate > 24 ? (frameRate < 60 ? frameRate : 60) : 24);
      while (((sampleRate / frameRate) * frameRate) != sampleRate)
        frameRate++;
      lineBuf = reinterpret_cast<uint8_t *>(new uint32_t[720 / 4]);
      std::memset(lineBuf, 0x00, 720);
      audioBufSize = sampleRate / frameRate;
      audioBuf = new int16_t[audioBufSize * audioBuffers];
      for (int i = 0; i < (audioBufSize * audioBuffers); i++)
        audioBuf[i] = int16_t(0);
      audioConverter = new AudioConverter_(*this, 221681.0f, float(sampleRate));
    }
    catch (...) {
      if (lineBuf)
        delete[] reinterpret_cast<uint32_t *>(lineBuf);
      if (audioBuf)
        delete[] audioBuf;
      if (audioConverter)
        delete audioConverter;
      throw;
    }
  }

  VideoCapture::~VideoCapture()
  {
    delete[] reinterpret_cast<uint32_t *>(lineBuf);
    delete[] audioBuf;
    delete audioConverter;
  }

  void VideoCapture::runOneCycle(const uint8_t *videoInput, int16_t audioInput)
  {
    soundOutputAccumulator += int32_t(audioInput);
    if (++cycleCnt >= 8) {
      cycleCnt = 0;
      int32_t tmp = ((soundOutputAccumulator + 262148) >> 3) - 32768;
      soundOutputAccumulator = 0;
      audioConverter->sendInputSignal(tmp);
    }
    uint8_t   c = videoInput[0];
    if (c & 0x80) {                                     // sync
      if (syncLengthCnt == 0U) {                        // hsync start
        while (hsyncCnt >= hsyncPeriodMax) {
          hsyncCnt -= hsyncPeriodLength;
          hsyncPeriodLength = (hsyncPeriodLength * 3U + hsyncPeriodMax) >> 2;
        }
        if (hsyncCnt >= hsyncPeriodMin) {
          hsyncPeriodLength = hsyncCnt;
          hsyncCnt = 0U;
        }
      }
      syncLengthCnt++;
      if (syncLengthCnt >= 26U) {                       // vsync
        if (vsyncCnt >= vsyncThreshold2) {
          vsyncCnt = vsyncReload;
          oddFrame = ((lineLengthCnt + 6U) > (lineLength >> 1));
        }
      }
    }
    else
      syncLengthCnt = 0U;
    lineBufFlags |= uint8_t(0x80 - ((c ^ burstValue) & 0x09));
    unsigned int  l = ((unsigned int) c & 0x01U) ^ 0x05U;
    if (lineLengthCnt < lineStart) {
      lineBufLength = lineLengthCnt + l;
    }
    else if (lineLengthCnt < lineLength) {
      size_t  nBytes = size_t((1 << (c & 0x02)) + 1);
      std::memcpy(&(lineBuf[lineBufBytes]), videoInput, nBytes);
      lineBufBytes += nBytes;
    }
    else {
      lineBufLength = size_t(lineLengthCnt) - lineBufLength;
      lineDone();
    }
    lineLengthCnt = lineLengthCnt + l;
    hsyncCnt = hsyncCnt + l;
    curTime += timesliceLength;
  }

  void VideoCapture::lineDone()
  {
    lineLengthCnt = lineLengthCnt - lineLength;
    while (hsyncCnt >= hsyncPeriodMax) {
      hsyncCnt -= hsyncPeriodLength;
      hsyncPeriodLength = (hsyncPeriodLength * 3U + hsyncPeriodMax) >> 2;
    }
    lineLengthFilter =
        (lineLengthFilter * 0.9f) + (float(int(hsyncPeriodLength)) * 0.1f);
    lineLength = (unsigned int) (int(lineLengthFilter + 0.5f));
    if (lineLengthCnt != hsyncCnt) {
      int     hsyncPhaseError = int(lineLengthCnt) - int(hsyncCnt);
      if (hsyncPhaseError >= int(hsyncPeriodLength >> 1))
        hsyncPhaseError -= int(hsyncPeriodLength);
      if (hsyncPhaseError <= -(int(hsyncPeriodLength >> 1)))
        hsyncPhaseError += int(hsyncPeriodLength);
      unsigned int  tmp = (unsigned int) (hsyncPhaseError >= 0 ?
                                          hsyncPhaseError : (-hsyncPhaseError));
      tmp = (tmp + 6U) >> 2;
      tmp = (tmp < 10U ? tmp : 10U);
      if (hsyncPhaseError >= 0)
        lineLength += tmp;
      else
        lineLength -= tmp;
      if (lineLength > lineLengthMax)
        lineLength = lineLengthMax;
      else if (lineLength < lineLengthMin)
        lineLength = lineLengthMin;
    }
    if (curLine >= 2 && curLine < ((videoHeight * 2) + 2))
      decodeLine((curLine - 2) >> 1);
    lineBufBytes = 0;
    lineBufLength = 0;
    lineBufFlags = 0x00;
    curLine += 2;
    if (vsyncCnt >= vsyncThreshold1) {
      vsyncCnt = vsyncReload;
      oddFrame = false;
    }
    if (vsyncCnt == 0) {
      for (int i = ((curLine - 2) >> 1); i < videoHeight; i++)
        clearLine(i);
      frameDone();
      curLine = lineReload - (!oddFrame ? 0 : 1);
      for (int i = 0; i < (curLine - 2); i += 2)
        clearLine(i >> 1);
    }
    vsyncCnt++;
  }

  void VideoCapture::setClockFrequency(size_t freq_)
  {
    freq_ = (freq_ + 4) & (~(size_t(7)));
    if (freq_ == clockFrequency)
      return;
    clockFrequency = freq_;
    timesliceLength = (int64_t(1000000) << 32) / int64_t(freq_);
    audioConverter->setInputSampleRate(float(long(freq_ >> 3)));
  }

  void VideoCapture::setNTSCMode(bool ntscMode)
  {
    if (ntscMode != displayParameters.ntscMode) {
      lineBufBytes = 0;
      lineBufLength = 0;
      lineBufFlags = 0x00;
      if (!ntscMode) {
        burstValue = 0x08;
        syncLengthCnt = 0U;
        hsyncCnt = 0U;
        hsyncPeriodLength = 570U;
        lineLengthCnt = 0U;
        lineLength = 570U;
        lineStart = 80U;
        hsyncPeriodMin = 494U;
        hsyncPeriodMax = 646U;
        lineLengthMin = 513U;
        lineLengthMax = 627U;
        lineLengthFilter = 570.0f;
        vsyncThreshold1 = 335;
        vsyncThreshold2 = 261;
        vsyncReload = -19;
        lineReload = 0;
      }
      else {
        burstValue = 0x09;
        syncLengthCnt = 0U;
        hsyncCnt = 0U;
        hsyncPeriodLength = 456U;
        lineLengthCnt = 0U;
        lineLength = 456U;
        lineStart = 64U;
        hsyncPeriodMin = 380U;
        hsyncPeriodMax = 532U;
        lineLengthMin = 399U;
        lineLengthMax = 513U;
        lineLengthFilter = 456.0f;
        vsyncThreshold1 = 292;
        vsyncThreshold2 = 242;
        vsyncReload = 0;
        lineReload = 12;
      }
      displayParameters.ntscMode = ntscMode;
    }
  }

  void VideoCapture::openFile(const char *fileName)
  {
    closeFile();
    if (fileName == (char *) 0 || fileName[0] == '\0')
      return;
    aviFile = fileOpen(fileName, "wb");
    if (!aviFile)
      throw Exception("error opening AVI file");
    framesWritten = 0;
    duplicateFrames = 0;
    fileSize = aviHeaderSize;
    writeAVIHeader();
  }

  void VideoCapture::closeFile()
  {
    if (aviFile) {
      // FIXME: file I/O errors are ignored here
      try {
        writeAVIHeader();
        writeAVIIndex();
      }
      catch (...) {
      }
      if (aviFile)
        std::fclose(aviFile);
      aviFile = (std::FILE *) 0;
      framesWritten = 0;
      duplicateFrames = 0;
      fileSize = 0;
    }
  }

  void VideoCapture::errorMessage(const char *msg)
  {
    if (msg == (char *) 0 || msg[0] == '\0')
      msg = "unknown video capture error";
    errorCallback(errorCallbackUserData, msg);
  }

  void VideoCapture::setErrorCallback(void (*func)(void *userData,
                                                   const char *msg),
                                      void *userData_)
  {
    if (func) {
      errorCallback = func;
      errorCallbackUserData = userData_;
    }
    else {
      errorCallback = &defaultErrorCallback;
      errorCallbackUserData = (void *) this;
    }
  }

  void VideoCapture::setFileNameCallback(void (*func)(void *userData,
                                                      std::string& fileName),
                                         void *userData_)
  {
    if (func) {
      fileNameCallback = func;
      fileNameCallbackUserData = userData_;
    }
    else {
      fileNameCallback = &defaultFileNameCallback;
      fileNameCallbackUserData = (void *) this;
    }
  }

  // --------------------------------------------------------------------------

  VideoCapture_RLE8::VideoCaptureFrameBuffer::VideoCaptureFrameBuffer(int w,
                                                                      int h)
    : buf((uint32_t *) 0),
      linePtrs((uint8_t **) 0),
      lineBytes_((uint32_t *) 0)
  {
    try {
      size_t  nBytes = ((size_t(w) + 3) >> 2) << 2;
      buf = new uint32_t[(nBytes >> 2) * size_t(h)];
      std::memset(buf, 0x00, nBytes * size_t(h));
      linePtrs = new uint8_t*[h];
      uint8_t *p = reinterpret_cast<uint8_t *>(buf);
      for (int i = 0; i < h; i++) {
        linePtrs[i] = p;
        p = p + nBytes;
      }
      lineBytes_ = new uint32_t[h];
      for (int i = 0; i < h; i++)
        lineBytes_[i] = uint32_t(w);
    }
    catch (...) {
      if (buf)
        delete[] buf;
      if (linePtrs)
        delete[] linePtrs;
      if (lineBytes_)
        delete[] lineBytes_;
      throw;
    }
  }

  VideoCapture_RLE8::VideoCaptureFrameBuffer::~VideoCaptureFrameBuffer()
  {
    delete[] buf;
    delete[] linePtrs;
    delete[] lineBytes_;
  }

  inline bool VideoCapture_RLE8::VideoCaptureFrameBuffer::compareLine(
      long dstLine, const VideoCaptureFrameBuffer& src, long srcLine)
  {
    if (lineBytes_[dstLine] != src.lineBytes_[srcLine])
      return false;
    if (src.lineBytes_[srcLine] == 0U)
      return true;
    return (std::memcmp(linePtrs[dstLine], src.linePtrs[srcLine],
                        src.lineBytes_[srcLine]) == 0);
  }

  inline void VideoCapture_RLE8::VideoCaptureFrameBuffer::copyLine(long dstLine,
                                                                   long srcLine)
  {
    std::memcpy(linePtrs[dstLine], linePtrs[srcLine], lineBytes_[srcLine]);
    lineBytes_[dstLine] = lineBytes_[srcLine];
  }

  inline void VideoCapture_RLE8::VideoCaptureFrameBuffer::copyLine(
      long dstLine, const VideoCaptureFrameBuffer& src, long srcLine)
  {
    std::memcpy(linePtrs[dstLine], src.linePtrs[srcLine],
                src.lineBytes_[srcLine]);
    lineBytes_[dstLine] = src.lineBytes_[srcLine];
  }

  inline void VideoCapture_RLE8::VideoCaptureFrameBuffer::clearLine(long n)
  {
    if (lineBytes_[n])
      std::memset(linePtrs[n], 0x00, size_t(lineBytes_[n]));
  }

  // --------------------------------------------------------------------------

  VideoCapture_RLE8::VideoCapture_RLE8(
      void (*indexToYUVFunc)(uint8_t color, bool isNTSC,
                             float& y, float& u, float& v),
      int frameRate_)
    : VideoCapture(frameRate_),
      tmpFrameBuf(videoWidth, videoHeight),
      outputFrameBuf(videoWidth, videoHeight),
      frameSizes((uint32_t *) 0),
      cycleCnt(0),
      colormap()
  {
    try {
      aviHeaderSize = aviHeaderSize_RLE8;
      size_t  maxFrames = 0x40000000 / size_t(audioBufSize);
      frameSizes = new uint32_t[maxFrames];
      std::memset(frameSizes, 0x00, maxFrames * sizeof(uint32_t));
      // initialize colormap
      if (indexToYUVFunc)
        displayParameters.indexToYUVFunc = indexToYUVFunc;
      colormap.setDisplayParameters(displayParameters);
    }
    catch (...) {
      if (frameSizes)
        delete[] frameSizes;
      throw;
    }
    setClockFrequency(1773448);
  }

  VideoCapture_RLE8::~VideoCapture_RLE8()
  {
    closeFile();
    delete[] frameSizes;
  }

  void VideoCapture_RLE8::decodeLine(int lineNum)
  {
    int       xc = 0;
    size_t    bufPos = 0;
    size_t    pixelSample2 = lineBufLength;
    uint8_t   *bufp = tmpFrameBuf[lineNum];
    uint8_t   videoFlags = uint8_t(((lineNum & 1) << 1)
                                   | ((lineBufFlags & 0x80) >> 2));
    if (displayParameters.ntscMode)
      videoFlags = videoFlags | 0x10;
    if (pixelSample2 == (displayParameters.ntscMode ? 392 : 490) &&
        !(lineBufFlags & 0x01)) {
      // faster code for the case when resampling is not needed
      do {
        size_t  n = colormap.convertFourPixels(&(bufp[xc]),
                                               &(lineBuf[bufPos]),
                                               videoFlags);
        bufPos = bufPos + n;
        xc = xc + 4;
      } while (xc < videoWidth);
    }
    else {
      // need to resample video signal
      uint8_t   tmpBuf[4];
      size_t    pixelSample1 = 490;
      size_t    pixelSampleCnt = 0;
      uint8_t   readPos = 4;
      do {
        if (readPos >= 4) {
          readPos = readPos & 3;
          if (bufPos >= lineBufBytes)
            break;
          pixelSample1 = ((lineBuf[bufPos] & 0x01) ? 392 : 490);
          size_t  n = colormap.convertFourPixels(&(tmpBuf[0]),
                                                 &(lineBuf[bufPos]),
                                                 videoFlags);
          bufPos = bufPos + n;
        }
        bufp[xc] = tmpBuf[readPos];
        pixelSampleCnt += pixelSample2;
        while (pixelSampleCnt >= pixelSample1) {
          pixelSampleCnt -= pixelSample1;
          readPos++;
        }
      } while (++xc < videoWidth);
      for ( ; xc < videoWidth; xc++)
        bufp[xc] = 0x00;
    }
  }

  void VideoCapture_RLE8::clearLine(int lineNum)
  {
    tmpFrameBuf.clearLine(lineNum);
  }

  void VideoCapture_RLE8::frameDone()
  {
    if (audioBufSamples >= audioBufSize) {
      bool    frameChanged = false;
      for (int i = 0; i < videoHeight; i++) {
        if (!tmpFrameBuf.compareLine(i, outputFrameBuf, i)) {
          frameChanged = true;
          outputFrameBuf.copyLine(i, tmpFrameBuf, i);
        }
      }
      do {
        audioBufSamples -= audioBufSize;
        writeFrame(frameChanged);
        frameChanged = false;
        audioBufReadPos += audioBufSize;
        while (audioBufReadPos >= (audioBufSize * audioBuffers))
          audioBufReadPos -= (audioBufSize * audioBuffers);
      } while (audioBufSamples >= audioBufSize);
    }
  }

  size_t VideoCapture_RLE8::rleCompressLine(uint8_t *outBuf,
                                            const uint8_t *inBuf)
  {
    uint8_t runLengths[512];
    uint8_t byteValues[512];
    size_t  nBytes = 0;
    int     n = 0;
    int     runLength = 1;
    for (int i = 1; i < videoWidth; i++) {
      if (inBuf[i] != inBuf[i - 1]) {
        runLengths[n] = uint8_t(runLength);
        byteValues[n++] = inBuf[i - 1];
        runLength = 1;
      }
      else {
        if (++runLength >= 256) {
          runLengths[n] = 255;
          byteValues[n++] = inBuf[i - 1];
          runLength = 1;
        }
      }
    }
    runLengths[n] = uint8_t(runLength);
    byteValues[n++] = inBuf[videoWidth - 1];
    for (int i = 0; i < n; ) {
      if (runLengths[i] >= 2 || (i + 1) >= n) {
        outBuf[nBytes++] = runLengths[i];
        outBuf[nBytes++] = byteValues[i];
        i++;
      }
      else {
        int     j = i + 1;
        int     bytesToCopy = int(runLengths[i]);
        int     minLength = 3;
        do {
          int     tmp = int(runLengths[j]);
          if (tmp >= minLength || (bytesToCopy + tmp) > 255)
            break;
          bytesToCopy += tmp;
          minLength = 4 | (bytesToCopy & 1);
        } while (++j < n);
        if (bytesToCopy >= 3) {
          outBuf[nBytes++] = 0x00;
          outBuf[nBytes++] = uint8_t(bytesToCopy);
          do {
            int     k = int(runLengths[i]);
            uint8_t tmp = byteValues[i];
            do {
              outBuf[nBytes++] = tmp;
            } while (--k != 0);
          } while (++i < j);
          if (bytesToCopy & 1)
            outBuf[nBytes++] = 0x00;
        }
        else {
          do {
            outBuf[nBytes++] = runLengths[i];
            outBuf[nBytes++] = byteValues[i];
          } while (++i < j);
        }
      }
    }
    outBuf[nBytes++] = 0x00;    // end of line
    outBuf[nBytes++] = 0x00;
    return nBytes;
  }

  void VideoCapture_RLE8::writeFrame(bool frameChanged)
  {
    if (!aviFile)
      return;
    if (!frameChanged) {
      if (framesWritten == 0 || duplicateFrames >= size_t(frameRate))
        frameChanged = true;
      else
        duplicateFrames++;
    }
    if (frameChanged)
      duplicateFrames = 0;
    try {
      if (fileSize >= 0x7F800000) {
        closeFile();
        try {
          errorMessage("AVI file is too large, starting new output file");
        }
        catch (...) {
        }
        std::string fileName = "";
        fileNameCallback(fileNameCallbackUserData, fileName);
        if (fileName.length() < 1)
          return;
        openFile(fileName.c_str());
      }
      if (std::fseek(aviFile, 0L, SEEK_END) < 0)
        throw Exception("error seeking AVI file");
      uint8_t headerBuf[8];
      uint8_t *bufp = &(headerBuf[0]);
      size_t  nBytes = 0;
      frameSizes[framesWritten] = 0U;
      aviHeader_writeFourCC(bufp, "00dc");
      aviHeader_writeUInt32(bufp, 0x00000000U);
      fileSize = fileSize + 8;
      if (std::fwrite(&(headerBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file");
      if (frameChanged) {
        long    savedFilePos = std::ftell(aviFile);
        if (savedFilePos < 4L)
          throw Exception("error seeking AVI file");
        savedFilePos = savedFilePos - 4L;
        uint8_t rleBuf[512];
        size_t  n = 0;
        for (int i = (videoHeight - 1); i >= 0; i--) {
          if (i == (videoHeight - 1) ||
              !outputFrameBuf.compareLine(i, outputFrameBuf, i + 1)) {
            n = rleCompressLine(&(rleBuf[0]), outputFrameBuf[i]);
          }
          nBytes += n;
          fileSize += n;
          if (std::fwrite(&(rleBuf[0]), 1, n, aviFile) != n)
            throw Exception("error writing AVI file");
        }
        if (std::fseek(aviFile, savedFilePos, SEEK_SET) < 0)
          throw Exception("error seeking AVI file");
        bufp = &(headerBuf[4]);
        frameSizes[framesWritten] = uint32_t(nBytes);
        aviHeader_writeUInt32(bufp, uint32_t(nBytes));
        if (std::fwrite(&(headerBuf[4]), 1, 4, aviFile) != 4)
          throw Exception("error writing AVI file");
        if (std::fseek(aviFile, 0L, SEEK_END) < 0)
          throw Exception("error seeking AVI file");
      }
      bufp = &(headerBuf[0]);
      nBytes = size_t(audioBufSize * 2);
      aviHeader_writeFourCC(bufp, "01wb");
      aviHeader_writeUInt32(bufp, uint32_t(nBytes));
      fileSize = fileSize + 8;
      if (std::fwrite(&(headerBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file");
      int     bufPos = audioBufReadPos;
      for (int i = 0; i < audioBufSize; i++) {
        if (bufPos >= (audioBufSize * audioBuffers))
          bufPos = 0;
        int16_t tmp = audioBuf[bufPos++];
        fileSize++;
        if (std::fputc(int(uint16_t(tmp) & 0xFF), aviFile) == EOF)
          throw Exception("error writing AVI file");
        fileSize++;
        if (std::fputc(int((uint16_t(tmp) >> 8) & 0xFF), aviFile) == EOF)
          throw Exception("error writing AVI file");
      }
    }
    catch (std::exception& e) {
      closeFile();
      errorMessage(e.what());
      return;
    }
    framesWritten++;
    if (!(framesWritten & 31)) {
      try {
        writeAVIHeader();
      }
      catch (std::exception& e) {
        errorMessage(e.what());
      }
    }
  }

  void VideoCapture_RLE8::writeAVIHeader()
  {
    if (!aviFile)
      return;
    try {
      if (std::fseek(aviFile, 0L, SEEK_SET) < 0)
        throw Exception("error seeking AVI file");
      uint8_t   headerBuf[1536];
      uint8_t   *bufp = &(headerBuf[0]);
      size_t    maxVideoFrameSize = size_t((videoWidth + 16) * videoHeight);
      size_t    maxFrameSize = size_t(maxVideoFrameSize + (audioBufSize * 2)
                                      + 16);
      aviHeader_writeFourCC(bufp, "RIFF");
      aviHeader_writeUInt32(bufp, uint32_t(fileSize - 8));
      aviHeader_writeFourCC(bufp, "AVI ");
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, 0x00000526U);
      aviHeader_writeFourCC(bufp, "hdrl");
      aviHeader_writeFourCC(bufp, "avih");
      aviHeader_writeUInt32(bufp, 0x00000038U);
      // microseconds per frame
      aviHeader_writeUInt32(bufp,
                            uint32_t((1000000 + (frameRate >> 1)) / frameRate));
      // max. bytes per second
      aviHeader_writeUInt32(bufp, uint32_t(maxFrameSize * size_t(frameRate)));
      // padding
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // flags (AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE)
      aviHeader_writeUInt32(bufp, 0x00000910U);
      // total frames
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten));
      // initial frames
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // number of streams
      aviHeader_writeUInt32(bufp, 0x00000002U);
      // suggested buffer size
      aviHeader_writeUInt32(bufp, uint32_t(maxFrameSize));
      // width
      aviHeader_writeUInt32(bufp, uint32_t(videoWidth));
      // height
      aviHeader_writeUInt32(bufp, uint32_t(videoHeight));
      // reserved
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, 0x00000474U);
      aviHeader_writeFourCC(bufp, "strl");
      aviHeader_writeFourCC(bufp, "strh");
      aviHeader_writeUInt32(bufp, 0x00000038U);
      aviHeader_writeFourCC(bufp, "vids");
      // video codec
      aviHeader_writeUInt32(bufp, 0x00000001U); // BI_RLE8
      // flags
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // priority
      aviHeader_writeUInt16(bufp, 0x0000);
      // language
      aviHeader_writeUInt16(bufp, 0x0000);
      // initial frames
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // scale
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // rate
      aviHeader_writeUInt32(bufp, uint32_t(frameRate));
      // start time
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // length
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten));
      // suggested buffer size
      aviHeader_writeUInt32(bufp, uint32_t(maxVideoFrameSize));
      // quality
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // sample size
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // left
      aviHeader_writeUInt16(bufp, 0x0000);
      // top
      aviHeader_writeUInt16(bufp, 0x0000);
      // right
      aviHeader_writeUInt16(bufp, uint16_t(videoWidth));
      // bottom
      aviHeader_writeUInt16(bufp, uint16_t(videoHeight));
      aviHeader_writeFourCC(bufp, "strf");
      aviHeader_writeUInt32(bufp, 0x00000428U);
      aviHeader_writeUInt32(bufp, 0x00000028U);
      // width
      aviHeader_writeUInt32(bufp, uint32_t(videoWidth));
      // height
      aviHeader_writeUInt32(bufp, uint32_t(videoHeight));
      // planes
      aviHeader_writeUInt16(bufp, 0x0001);
      // bits per pixel
      aviHeader_writeUInt16(bufp, 0x0008);
      // compression
      aviHeader_writeUInt32(bufp, 0x00000001U); // BI_RLE8
      // image size in bytes
      aviHeader_writeUInt32(bufp, uint32_t(videoWidth * videoHeight));
      // X resolution
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // Y resolution
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // color indexes used
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // color indexes required
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // colormap (256 entries)
      for (int i = 0; i < 256; i++) {
        float   y = 0.0f;
        float   u = 0.0f;
        float   v = 0.0f;
        Plus4Emu::getPlus4PaletteColor(i, y, u, v);
        float   r = (v / 0.877f) + y;
        float   b = (u / 0.492f) + y;
        float   g = (y - ((r * 0.299f) + (b * 0.114f))) / 0.587f;
        int ri = int((r > 0.0f ? (r < 1.0f ? r : 1.0f) : 0.0f) * 255.0f + 0.5f);
        int gi = int((g > 0.0f ? (g < 1.0f ? g : 1.0f) : 0.0f) * 255.0f + 0.5f);
        int bi = int((b > 0.0f ? (b < 1.0f ? b : 1.0f) : 0.0f) * 255.0f + 0.5f);
        bufp[0] = uint8_t(bi);
        bufp[1] = uint8_t(gi);
        bufp[2] = uint8_t(ri);
        bufp[3] = 0x00;
        bufp = bufp + 4;
      }
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, 0x0000005EU);
      aviHeader_writeFourCC(bufp, "strl");
      aviHeader_writeFourCC(bufp, "strh");
      aviHeader_writeUInt32(bufp, 0x00000038U);
      aviHeader_writeFourCC(bufp, "auds");
      // audio codec (WAVE_FORMAT_PCM)
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // flags
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // priority
      aviHeader_writeUInt16(bufp, 0x0000);
      // language
      aviHeader_writeUInt16(bufp, 0x0000);
      // initial frames
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // scale
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // rate
      aviHeader_writeUInt32(bufp, uint32_t(sampleRate));
      // start time
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // length
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten
                                           * size_t(audioBufSize)));
      // suggested buffer size
      aviHeader_writeUInt32(bufp, uint32_t(audioBufSize * 2));
      // quality
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // sample size
      aviHeader_writeUInt32(bufp, 0x00000002U);
      // left
      aviHeader_writeUInt16(bufp, 0x0000);
      // top
      aviHeader_writeUInt16(bufp, 0x0000);
      // right
      aviHeader_writeUInt16(bufp, 0x0000);
      // bottom
      aviHeader_writeUInt16(bufp, 0x0000);
      aviHeader_writeFourCC(bufp, "strf");
      aviHeader_writeUInt32(bufp, 0x00000012U);
      // audio format (WAVE_FORMAT_PCM)
      aviHeader_writeUInt16(bufp, 0x0001);
      // audio channels
      aviHeader_writeUInt16(bufp, 0x0001);
      // samples per second
      aviHeader_writeUInt32(bufp, uint32_t(sampleRate));
      // bytes per second
      aviHeader_writeUInt32(bufp, uint32_t(sampleRate * 2));
      // block alignment
      aviHeader_writeUInt16(bufp, 0x0002);
      // bits per sample
      aviHeader_writeUInt16(bufp, 0x0010);
      // additional format information size
      aviHeader_writeUInt16(bufp, 0x0000);
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, uint32_t((fileSize - aviHeaderSize) + 4));
      aviHeader_writeFourCC(bufp, "movi");
      size_t  nBytes = size_t(bufp - (&(headerBuf[0])));
      if (std::fwrite(&(headerBuf[0]), 1, nBytes, aviFile) != nBytes)
        throw Exception("error writing AVI file header");
      if (std::fflush(aviFile) != 0)
        throw Exception("error writing AVI file header");
    }
    catch (...) {
      std::fclose(aviFile);
      aviFile = (std::FILE *) 0;
      framesWritten = 0;
      duplicateFrames = 0;
      fileSize = 0;
      throw;
    }
  }

  void VideoCapture_RLE8::writeAVIIndex()
  {
    if (!aviFile)
      return;
    try {
      if (std::fseek(aviFile, 0L, SEEK_END) < 0)
        throw Exception("error seeking AVI file");
      uint8_t   tmpBuf[32];
      uint8_t   *bufp = &(tmpBuf[0]);
      aviHeader_writeFourCC(bufp, "idx1");
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten << 5));
      fileSize = fileSize + 8;
      if (std::fwrite(&(tmpBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file index");
      size_t    filePos = 4;
      for (size_t i = 0; i < framesWritten; i++) {
        bufp = &(tmpBuf[0]);
        aviHeader_writeFourCC(bufp, "00dc");
        size_t    frameBytes = size_t(frameSizes[i]);
        if (frameBytes > 0)
          aviHeader_writeUInt32(bufp, 0x00000010U);     // AVIIF_KEYFRAME
        else
          aviHeader_writeUInt32(bufp, 0x00000000U);
        aviHeader_writeUInt32(bufp, uint32_t(filePos));
        filePos = filePos + frameBytes + 8;
        aviHeader_writeUInt32(bufp, uint32_t(frameBytes));
        aviHeader_writeFourCC(bufp, "01wb");
        aviHeader_writeUInt32(bufp, 0x00000010U);       // AVIIF_KEYFRAME
        aviHeader_writeUInt32(bufp, uint32_t(filePos));
        frameBytes = size_t(audioBufSize) << 1;
        filePos = filePos + frameBytes + 8;
        aviHeader_writeUInt32(bufp, uint32_t(frameBytes));
        fileSize = fileSize + 32;
        if (std::fwrite(&(tmpBuf[0]), 1, 32, aviFile) != 32)
          throw Exception("error writing AVI file index");
      }
      if (std::fseek(aviFile, 0L, SEEK_SET) < 0)
        throw Exception("error seeking AVI file");
      bufp = &(tmpBuf[0]);
      aviHeader_writeFourCC(bufp, "RIFF");
      aviHeader_writeUInt32(bufp, uint32_t(fileSize - 8));
      if (std::fwrite(&(tmpBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file index");
      if (std::fflush(aviFile) != 0)
        throw Exception("error writing AVI file index");
    }
    catch (...) {
      std::fclose(aviFile);
      aviFile = (std::FILE *) 0;
      framesWritten = 0;
      duplicateFrames = 0;
      fileSize = 0;
      throw;
    }
  }

  // --------------------------------------------------------------------------

  VideoCapture_YV12::VideoCapture_YV12(
      void (*indexToYUVFunc)(uint8_t color, bool isNTSC,
                             float& y, float& u, float& v),
      int frameRate_)
    : VideoCapture(frameRate_),
      videoBuf((uint32_t *) 0),
      frameBuf0Y((uint8_t *) 0),
      frameBuf0V((uint8_t *) 0),
      frameBuf0U((uint8_t *) 0),
      frameBuf1Y((uint8_t *) 0),
      frameBuf1V((uint8_t *) 0),
      frameBuf1U((uint8_t *) 0),
      interpBufY((int32_t *) 0),
      interpBufV((int32_t *) 0),
      interpBufU((int32_t *) 0),
      outBufY((uint8_t *) 0),
      outBufV((uint8_t *) 0),
      outBufU((uint8_t *) 0),
      duplicateFrameBitmap((uint8_t *) 0),
      colormap()
  {
    try {
      aviHeaderSize = aviHeaderSize_YV12;
      size_t    bufSize1 = size_t(videoWidth * videoHeight);
      size_t    bufSize3 = (bufSize1 + 3) >> 2;
      size_t    bufSize4 = (bufSize3 + 3) >> 2;
      size_t    totalSize = (3 * (bufSize3 + bufSize4 + bufSize4));
      totalSize += (bufSize1 + bufSize3 + bufSize3);
      videoBuf = new uint32_t[totalSize];
      totalSize = 0;
      frameBuf0Y = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(frameBuf0Y, 0x10, bufSize1);
      totalSize += bufSize3;
      frameBuf0V = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(frameBuf0V, 0x80, bufSize3);
      totalSize += bufSize4;
      frameBuf0U = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(frameBuf0U, 0x80, bufSize3);
      totalSize += bufSize4;
      frameBuf1Y = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(frameBuf1Y, 0x10, bufSize1);
      totalSize += bufSize3;
      frameBuf1V = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(frameBuf1V, 0x80, bufSize3);
      totalSize += bufSize4;
      frameBuf1U = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(frameBuf1U, 0x80, bufSize3);
      totalSize += bufSize4;
      interpBufY = reinterpret_cast<int32_t *>(&(videoBuf[totalSize]));
      for (size_t i = 0; i < bufSize1; i++)
        interpBufY[i] = 0;
      totalSize += bufSize1;
      interpBufV = reinterpret_cast<int32_t *>(&(videoBuf[totalSize]));
      for (size_t i = 0; i < bufSize3; i++)
        interpBufV[i] = 0;
      totalSize += bufSize3;
      interpBufU = reinterpret_cast<int32_t *>(&(videoBuf[totalSize]));
      for (size_t i = 0; i < bufSize3; i++)
        interpBufU[i] = 0;
      totalSize += bufSize3;
      outBufY = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(outBufY, 0x10, bufSize1);
      totalSize += bufSize3;
      outBufV = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(outBufV, 0x80, bufSize3);
      totalSize += bufSize4;
      outBufU = reinterpret_cast<uint8_t *>(&(videoBuf[totalSize]));
      std::memset(outBufU, 0x80, bufSize3);
      size_t  nBytes = 0x08000000 / size_t(audioBufSize);
      duplicateFrameBitmap = new uint8_t[nBytes];
      std::memset(duplicateFrameBitmap, 0x00, nBytes);
      // initialize colormap
      if (indexToYUVFunc)
        displayParameters.indexToYUVFunc = indexToYUVFunc;
      // scale video signal to YCrCb range (Y: 16..235, U,V: 16..240)
      displayParameters.brightness = -2.0f / 255.0f;
      displayParameters.contrast = 219.0f / 255.0f;
      displayParameters.saturation = 224.0f / 219.0f;
      colormap.setDisplayParameters(displayParameters, true);
      // change pixel format for more efficient processing
      uint32_t  *p = colormap.getFirstEntry();
      while (p) {
        uint32_t  tmp = *p;
        tmp = ((tmp & 0x00FF0000U) << 4) | ((tmp & 0x0000FF00U) << 2)
              | (tmp & 0x000000FFU);
        if (!(tmp & 0x000000F0U))
          tmp = (tmp & 0x0FF3FC00U) | 0x00000010U;
        (*p) = tmp;
        p = colormap.getNextEntry(p);
      }
    }
    catch (...) {
      if (videoBuf)
        delete[] videoBuf;
      if (duplicateFrameBitmap)
        delete[] duplicateFrameBitmap;
      throw;
    }
    setClockFrequency(1773448);
  }

  VideoCapture_YV12::~VideoCapture_YV12()
  {
    closeFile();
    delete[] videoBuf;
    delete[] duplicateFrameBitmap;
  }

  void VideoCapture_YV12::decodeLine(int lineNum)
  {
    int       xc = 0;
    size_t    bufPos = 0;
    uint8_t   videoFlags = uint8_t(((lineNum & 1) << 1)
                                   | ((lineBufFlags & 0x80) >> 2));
    size_t    pixelSample2 = lineBufLength;
    if (displayParameters.ntscMode)
      videoFlags = videoFlags | 0x10;
    uint32_t  tmpBuf2[512];
    if (pixelSample2 == (displayParameters.ntscMode ? 392 : 490) &&
        !(lineBufFlags & 0x01)) {
      // faster code for the case when resampling is not needed
      do {
        size_t  n = colormap.convertFourPixels(&(tmpBuf2[xc]),
                                               &(lineBuf[bufPos]),
                                               videoFlags);
        bufPos = bufPos + n;
        xc = xc + 4;
      } while (xc < videoWidth);
    }
    else {
      // need to resample video signal
      uint32_t  tmpBuf[4];
      size_t    pixelSample1 = 980;
      size_t    pixelSampleCnt = 0;
      uint8_t   readPos = 4;
      do {
        if (readPos >= 4) {
          readPos = readPos & 3;
          if (bufPos >= lineBufBytes)
            break;
          pixelSample1 = ((lineBuf[bufPos] & 0x01) ? 784 : 980);
          size_t  n = colormap.convertFourPixels(&(tmpBuf[0]),
                                                 &(lineBuf[bufPos]),
                                                 videoFlags);
          bufPos += n;
        }
        uint32_t  pixel0 = tmpBuf[readPos];
        pixelSampleCnt += pixelSample2;
        if (pixelSampleCnt >= pixelSample1) {
          pixelSampleCnt -= pixelSample1;
          if (++readPos >= 4) {
            readPos = readPos & 3;
            if (bufPos >= lineBufBytes)
              break;
            pixelSample1 = ((lineBuf[bufPos] & 0x01) ? 784 : 980);
            size_t  n = colormap.convertFourPixels(&(tmpBuf[0]),
                                                   &(lineBuf[bufPos]),
                                                   videoFlags);
            bufPos += n;
          }
        }
        uint32_t  pixel1 = tmpBuf[readPos];
        pixelSampleCnt += pixelSample2;
        if (pixelSampleCnt >= pixelSample1) {
          pixelSampleCnt -= pixelSample1;
          readPos++;
        }
        // average two pixels for improved quality
        tmpBuf2[xc++] = ((pixel0 + pixel1 + 0x00100401U) >> 1) & 0x0FF3FCFFU;
      } while (xc < videoWidth);
      for ( ; xc < videoWidth; xc++)
        tmpBuf2[xc] = 0x08020010U;
    }
    int       offs = lineNum * videoWidth;
    uint8_t   *yPtr = &(frameBuf1Y[offs]);
    offs = (lineNum >> 1) * (videoWidth >> 1);
    uint8_t   *vPtr = &(frameBuf1V[offs]);
    uint8_t   *uPtr = &(frameBuf1U[offs]);
    if (!(lineNum & 1)) {
      for (xc = 0; xc < videoWidth; xc += 2) {
        uint32_t  pixel0 = tmpBuf2[xc + 0];
        uint32_t  pixel1 = tmpBuf2[xc + 1];
        yPtr[xc + 0] = uint8_t(pixel0 & 0xFFU);
        yPtr[xc + 1] = uint8_t(pixel1 & 0xFFU);
        pixel0 = pixel0 + pixel1 + 0x00100400U;
        vPtr[xc >> 1] = uint8_t((pixel0 >> 21) & 0xFFU);
        uPtr[xc >> 1] = uint8_t((pixel0 >> 11) & 0xFFU);
      }
    }
    else {
      for (xc = 0; xc < videoWidth; xc += 2) {
        uint32_t  pixel0 = tmpBuf2[xc + 0];
        uint32_t  pixel1 = tmpBuf2[xc + 1];
        yPtr[xc + 0] = uint8_t(pixel0 & 0xFFU);
        yPtr[xc + 1] = uint8_t(pixel1 & 0xFFU);
        pixel0 = pixel0 + pixel1 + 0x00100400U;
        uint32_t  v = ((pixel0 >> 21) & 0xFFU) + uint32_t(vPtr[xc >> 1]) + 1U;
        vPtr[xc >> 1] = uint8_t(v >> 1);
        uint32_t  u = ((pixel0 >> 11) & 0xFFU) + uint32_t(uPtr[xc >> 1]) + 1U;
        uPtr[xc >> 1] = uint8_t(u >> 1);
      }
    }
  }

  void VideoCapture_YV12::clearLine(int lineNum)
  {
    std::memset(&(frameBuf1Y[lineNum * videoWidth]), 0x10, size_t(videoWidth));
    std::memset(&(frameBuf1V[(lineNum >> 1) * (videoWidth >> 1)]),
                0x80, size_t(videoWidth >> 1));
    std::memset(&(frameBuf1U[(lineNum >> 1) * (videoWidth >> 1)]),
                0x80, size_t(videoWidth >> 1));
  }

  void VideoCapture_YV12::frameDone()
  {
    resampleFrame();
    while (audioBufSamples >= audioBufSize) {
      audioBufSamples -= audioBufSize;
      int64_t   frameTime =
          int64_t((4294967296000000.0 / double(frameRate)) + 0.5);
      if (frameTime > frame1Time)
        frameTime = frame1Time;
      int32_t   t0 =
          int32_t(((frameTime - frame0Time) + int64_t(0x80000000UL)) >> 32);
      int32_t   t1 =
          int32_t(((frame1Time - frameTime) + int64_t(0x80000000UL)) >> 32);
      double    tt = 3.1415926535898 * (double(t1) / (double(t0) + double(t1)));
      tt = 0.3183098861838 * (tt - std::sin(tt));
      int32_t   scaleFac0 = int32_t(double(t1) * tt + 0.5);
      int32_t   scaleFac1 = int32_t(double(t1) * (2.0 - tt) + 0.5);
      int32_t   outScale = int32_t(0x20000000) / (interpTime - t1);
      interpTime = t1;
      int       n = (videoWidth * videoHeight * 3) / 2;
      int       i = 0;
      uint8_t   frameChanged = 0x00;
      do {
        int32_t   tmp;
        uint8_t   tmp2;
        tmp = (int32_t(frameBuf0Y[i]) * scaleFac0)
              + (int32_t(frameBuf1Y[i]) * scaleFac1);
        tmp2 = uint8_t(((((interpBufY[i] - tmp) >> 8) * outScale)
                        + 0x00200000) >> 22);
        interpBufY[i] = tmp;
        frameChanged |= (tmp2 ^ outBufY[i]);
        outBufY[i] = tmp2;
        i++;
        tmp = (int32_t(frameBuf0Y[i]) * scaleFac0)
              + (int32_t(frameBuf1Y[i]) * scaleFac1);
        tmp2 = uint8_t(((((interpBufY[i] - tmp) >> 8) * outScale)
                        + 0x00200000) >> 22);
        interpBufY[i] = tmp;
        frameChanged |= (tmp2 ^ outBufY[i]);
        outBufY[i] = tmp2;
      } while (++i < n);
      writeFrame(bool(frameChanged));
      audioBufReadPos += audioBufSize;
      while (audioBufReadPos >= (audioBufSize * audioBuffers))
        audioBufReadPos -= (audioBufSize * audioBuffers);
      frame0Time -= frameTime;
      frame1Time -= frameTime;
      curTime -= frameTime;
    }
    int64_t   frameTime =
        ((int64_t(audioBufSamples * 10000) << 32) + int64_t(sampleRate / 200))
        / int64_t(sampleRate / 100);
    curTime += (frameTime - frame1Time);
    frame0Time += (frameTime - frame1Time);
    frame1Time = frameTime;
    uint8_t   *tmp = frameBuf0Y;
    frameBuf0Y = frameBuf1Y;
    frameBuf1Y = tmp;
    tmp = frameBuf0V;
    frameBuf0V = frameBuf1V;
    frameBuf1V = tmp;
    tmp = frameBuf0U;
    frameBuf0U = frameBuf1U;
    frameBuf1U = tmp;
  }

  void VideoCapture_YV12::resampleFrame()
  {
    frame0Time = frame1Time;
    frame1Time = curTime;
    int32_t   scaleFac =
        int32_t(((frame1Time - frame0Time) + int64_t(0x80000000UL)) >> 32);
    interpTime += scaleFac;
    int       n = (videoWidth * videoHeight * 3) / 2;
    int       i = 0;
    do {
      interpBufY[i] +=
          ((int32_t(frameBuf0Y[i]) + int32_t(frameBuf1Y[i])) * scaleFac);
      i++;
      interpBufY[i] +=
          ((int32_t(frameBuf0Y[i]) + int32_t(frameBuf1Y[i])) * scaleFac);
    } while (++i < n);
  }

  void VideoCapture_YV12::writeFrame(bool frameChanged)
  {
    if (!aviFile)
      return;
    if (!frameChanged) {
      if (framesWritten == 0 || duplicateFrames >= size_t(frameRate))
        frameChanged = true;
      else
        duplicateFrames++;
    }
    if (frameChanged) {
      duplicateFrames = 0;
      duplicateFrameBitmap[framesWritten >> 3] &=
          uint8_t((1 << (framesWritten & 7)) ^ 0xFF);
    }
    else {
      duplicateFrameBitmap[framesWritten >> 3] |=
          uint8_t(1 << (framesWritten & 7));
    }
    try {
      if (fileSize >= 0x7F800000) {
        closeFile();
        try {
          errorMessage("AVI file is too large, starting new output file");
        }
        catch (...) {
        }
        std::string fileName = "";
        fileNameCallback(fileNameCallbackUserData, fileName);
        if (fileName.length() < 1)
          return;
        openFile(fileName.c_str());
      }
      if (std::fseek(aviFile, 0L, SEEK_END) < 0)
        throw Exception("error seeking AVI file");
      uint8_t headerBuf[8];
      uint8_t *bufp = &(headerBuf[0]);
      size_t  nBytes = 0;
      if (frameChanged)
        nBytes = size_t((videoWidth * videoHeight * 3) / 2);
      aviHeader_writeFourCC(bufp, "00dc");
      aviHeader_writeUInt32(bufp, uint32_t(nBytes));
      fileSize = fileSize + 8;
      if (std::fwrite(&(headerBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file");
      if (nBytes > 0) {
        fileSize = fileSize + nBytes;
        if (std::fwrite(&(outBufY[0]), 1, nBytes, aviFile) != nBytes)
          throw Exception("error writing AVI file");
      }
      bufp = &(headerBuf[0]);
      nBytes = size_t(audioBufSize * 2);
      aviHeader_writeFourCC(bufp, "01wb");
      aviHeader_writeUInt32(bufp, uint32_t(nBytes));
      fileSize = fileSize + 8;
      if (std::fwrite(&(headerBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file");
      int     bufPos = audioBufReadPos;
      for (int i = 0; i < audioBufSize; i++) {
        if (bufPos >= (audioBufSize * audioBuffers))
          bufPos = 0;
        int16_t tmp = audioBuf[bufPos++];
        fileSize++;
        if (std::fputc(int(uint16_t(tmp) & 0xFF), aviFile) == EOF)
          throw Exception("error writing AVI file");
        fileSize++;
        if (std::fputc(int((uint16_t(tmp) >> 8) & 0xFF), aviFile) == EOF)
          throw Exception("error writing AVI file");
      }
    }
    catch (std::exception& e) {
      closeFile();
      errorMessage(e.what());
      return;
    }
    framesWritten++;
    if (!(framesWritten & 31)) {
      try {
        writeAVIHeader();
      }
      catch (std::exception& e) {
        errorMessage(e.what());
      }
    }
  }

  void VideoCapture_YV12::writeAVIHeader()
  {
    if (!aviFile)
      return;
    try {
      if (std::fseek(aviFile, 0L, SEEK_SET) < 0)
        throw Exception("error seeking AVI file");
      uint8_t   headerBuf[512];
      uint8_t   *bufp = &(headerBuf[0]);
      size_t    frameSize = size_t(((videoWidth * videoHeight * 3) / 2)
                                   + (audioBufSize * 2) + 16);
      aviHeader_writeFourCC(bufp, "RIFF");
      aviHeader_writeUInt32(bufp, uint32_t(fileSize - 8));
      aviHeader_writeFourCC(bufp, "AVI ");
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, 0x00000126U);
      aviHeader_writeFourCC(bufp, "hdrl");
      aviHeader_writeFourCC(bufp, "avih");
      aviHeader_writeUInt32(bufp, 0x00000038U);
      // microseconds per frame
      aviHeader_writeUInt32(bufp,
                            uint32_t((1000000 + (frameRate >> 1)) / frameRate));
      // max. bytes per second
      aviHeader_writeUInt32(bufp, uint32_t(frameSize * size_t(frameRate)));
      // padding
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // flags (AVIF_HASINDEX | AVIF_ISINTERLEAVED | AVIF_TRUSTCKTYPE)
      aviHeader_writeUInt32(bufp, 0x00000910U);
      // total frames
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten));
      // initial frames
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // number of streams
      aviHeader_writeUInt32(bufp, 0x00000002U);
      // suggested buffer size
      aviHeader_writeUInt32(bufp, uint32_t(frameSize));
      // width
      aviHeader_writeUInt32(bufp, uint32_t(videoWidth));
      // height
      aviHeader_writeUInt32(bufp, uint32_t(videoHeight));
      // reserved
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, 0x00000074U);
      aviHeader_writeFourCC(bufp, "strl");
      aviHeader_writeFourCC(bufp, "strh");
      aviHeader_writeUInt32(bufp, 0x00000038U);
      aviHeader_writeFourCC(bufp, "vids");
      // video codec
      aviHeader_writeFourCC(bufp, "YV12");
      // flags
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // priority
      aviHeader_writeUInt16(bufp, 0x0000);
      // language
      aviHeader_writeUInt16(bufp, 0x0000);
      // initial frames
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // scale
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // rate
      aviHeader_writeUInt32(bufp, uint32_t(frameRate));
      // start time
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // length
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten));
      // suggested buffer size
      aviHeader_writeUInt32(bufp, uint32_t((videoWidth * videoHeight * 3) / 2));
      // quality
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // sample size
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // left
      aviHeader_writeUInt16(bufp, 0x0000);
      // top
      aviHeader_writeUInt16(bufp, 0x0000);
      // right
      aviHeader_writeUInt16(bufp, uint16_t(videoWidth));
      // bottom
      aviHeader_writeUInt16(bufp, uint16_t(videoHeight));
      aviHeader_writeFourCC(bufp, "strf");
      aviHeader_writeUInt32(bufp, 0x00000028U);
      aviHeader_writeUInt32(bufp, 0x00000028U);
      // width
      aviHeader_writeUInt32(bufp, uint32_t(videoWidth));
      // height
      aviHeader_writeUInt32(bufp, uint32_t(videoHeight));
      // planes
      aviHeader_writeUInt16(bufp, 0x0001);
      // bits per pixel
      aviHeader_writeUInt16(bufp, 0x0018);
      // compression
      aviHeader_writeFourCC(bufp, "YV12");
      // image size in bytes
      aviHeader_writeUInt32(bufp, uint32_t(videoWidth * videoHeight * 3));
      // X resolution
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // Y resolution
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // color indexes used
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // color indexes required
      aviHeader_writeUInt32(bufp, 0x00000000U);
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, 0x0000005EU);
      aviHeader_writeFourCC(bufp, "strl");
      aviHeader_writeFourCC(bufp, "strh");
      aviHeader_writeUInt32(bufp, 0x00000038U);
      aviHeader_writeFourCC(bufp, "auds");
      // audio codec (WAVE_FORMAT_PCM)
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // flags
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // priority
      aviHeader_writeUInt16(bufp, 0x0000);
      // language
      aviHeader_writeUInt16(bufp, 0x0000);
      // initial frames
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // scale
      aviHeader_writeUInt32(bufp, 0x00000001U);
      // rate
      aviHeader_writeUInt32(bufp, uint32_t(sampleRate));
      // start time
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // length
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten
                                           * size_t(audioBufSize)));
      // suggested buffer size
      aviHeader_writeUInt32(bufp, uint32_t(audioBufSize * 2));
      // quality
      aviHeader_writeUInt32(bufp, 0x00000000U);
      // sample size
      aviHeader_writeUInt32(bufp, 0x00000002U);
      // left
      aviHeader_writeUInt16(bufp, 0x0000);
      // top
      aviHeader_writeUInt16(bufp, 0x0000);
      // right
      aviHeader_writeUInt16(bufp, 0x0000);
      // bottom
      aviHeader_writeUInt16(bufp, 0x0000);
      aviHeader_writeFourCC(bufp, "strf");
      aviHeader_writeUInt32(bufp, 0x00000012U);
      // audio format (WAVE_FORMAT_PCM)
      aviHeader_writeUInt16(bufp, 0x0001);
      // audio channels
      aviHeader_writeUInt16(bufp, 0x0001);
      // samples per second
      aviHeader_writeUInt32(bufp, uint32_t(sampleRate));
      // bytes per second
      aviHeader_writeUInt32(bufp, uint32_t(sampleRate * 2));
      // block alignment
      aviHeader_writeUInt16(bufp, 0x0002);
      // bits per sample
      aviHeader_writeUInt16(bufp, 0x0010);
      // additional format information size
      aviHeader_writeUInt16(bufp, 0x0000);
      aviHeader_writeFourCC(bufp, "LIST");
      aviHeader_writeUInt32(bufp, uint32_t((fileSize - aviHeaderSize) + 4));
      aviHeader_writeFourCC(bufp, "movi");
      size_t  nBytes = size_t(bufp - (&(headerBuf[0])));
      if (std::fwrite(&(headerBuf[0]), 1, nBytes, aviFile) != nBytes)
        throw Exception("error writing AVI file header");
      if (std::fflush(aviFile) != 0)
        throw Exception("error writing AVI file header");
    }
    catch (...) {
      std::fclose(aviFile);
      aviFile = (std::FILE *) 0;
      framesWritten = 0;
      duplicateFrames = 0;
      fileSize = 0;
      throw;
    }
  }

  void VideoCapture_YV12::writeAVIIndex()
  {
    if (!aviFile)
      return;
    try {
      if (std::fseek(aviFile, 0L, SEEK_END) < 0)
        throw Exception("error seeking AVI file");
      uint8_t   tmpBuf[32];
      uint8_t   *bufp = &(tmpBuf[0]);
      aviHeader_writeFourCC(bufp, "idx1");
      aviHeader_writeUInt32(bufp, uint32_t(framesWritten << 5));
      fileSize = fileSize + 8;
      if (std::fwrite(&(tmpBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file index");
      size_t    filePos = 4;
      for (size_t i = 0; i < framesWritten; i++) {
        bufp = &(tmpBuf[0]);
        aviHeader_writeFourCC(bufp, "00dc");
        size_t    frameBytes = 0;
        if (!(duplicateFrameBitmap[i >> 3] & uint8_t(1 << (i & 7)))) {
          aviHeader_writeUInt32(bufp, 0x00000010U);     // AVIIF_KEYFRAME
          frameBytes = size_t((videoWidth * videoHeight * 3) / 2);
        }
        else {
          aviHeader_writeUInt32(bufp, 0x00000000U);
        }
        aviHeader_writeUInt32(bufp, uint32_t(filePos));
        filePos = filePos + frameBytes + 8;
        aviHeader_writeUInt32(bufp, uint32_t(frameBytes));
        aviHeader_writeFourCC(bufp, "01wb");
        aviHeader_writeUInt32(bufp, 0x00000010U);       // AVIIF_KEYFRAME
        aviHeader_writeUInt32(bufp, uint32_t(filePos));
        frameBytes = size_t(audioBufSize) << 1;
        filePos = filePos + frameBytes + 8;
        aviHeader_writeUInt32(bufp, uint32_t(frameBytes));
        fileSize = fileSize + 32;
        if (std::fwrite(&(tmpBuf[0]), 1, 32, aviFile) != 32)
          throw Exception("error writing AVI file index");
      }
      if (std::fseek(aviFile, 0L, SEEK_SET) < 0)
        throw Exception("error seeking AVI file");
      bufp = &(tmpBuf[0]);
      aviHeader_writeFourCC(bufp, "RIFF");
      aviHeader_writeUInt32(bufp, uint32_t(fileSize - 8));
      if (std::fwrite(&(tmpBuf[0]), 1, 8, aviFile) != 8)
        throw Exception("error writing AVI file index");
      if (std::fflush(aviFile) != 0)
        throw Exception("error writing AVI file index");
    }
    catch (...) {
      std::fclose(aviFile);
      aviFile = (std::FILE *) 0;
      framesWritten = 0;
      duplicateFrames = 0;
      fileSize = 0;
      throw;
    }
  }

}       // namespace Plus4Emu

