
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
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

#ifndef PLUS4EMU_VIDEOREC_HPP
#define PLUS4EMU_VIDEOREC_HPP

#include "plus4emu.hpp"
#include "display.hpp"
#include "snd_conv.hpp"

namespace Plus4Emu {

  class VideoCapture {
   public:
    static const int  videoWidth = 384;
    static const int  videoHeight = 288;
    static const int  sampleRate = 48000;
    static const int  audioBuffers = 8;
   protected:
    class AudioConverter_ : public AudioConverterHighQuality {
     private:
      VideoCapture& videoCapture;
     public:
      AudioConverter_(VideoCapture& videoCapture_,
                      float inputSampleRate_, float outputSampleRate_,
                      float dcBlockFreq1 = 5.0f, float dcBlockFreq2 = 15.0f,
                      float ampScale_ = 0.7943f);
      virtual ~AudioConverter_();
     protected:
      virtual void audioOutput(int16_t outputSignal_);
    };
    // --------
    std::FILE   *aviFile;
    uint8_t     *lineBuf;               // 720 bytes
    int16_t     *audioBuf;              // 8 * (sampleRate / frameRate) samples
    int         frameRate;              // video frames per second
    int         audioBufSize;           // = (sampleRate / frameRate)
    int         audioBufReadPos;
    int         audioBufWritePos;
    int         audioBufSamples;        // write position - read position
    size_t      clockFrequency;
    int64_t     timesliceLength;
    int64_t     curTime;
    int64_t     frame0Time;
    int64_t     frame1Time;
    int32_t     soundOutputAccumulator;
    int         cycleCnt;
    int32_t     interpTime;
    int         curLine;
    int         vsyncCnt;
    bool        oddFrame;
    uint8_t     burstValue;
    unsigned int  syncLengthCnt;
    unsigned int  hsyncCnt;
    unsigned int  hsyncPeriodLength;
    unsigned int  lineLengthCnt;
    unsigned int  lineLength;
    unsigned int  lineStart;
    unsigned int  hsyncPeriodMin;
    unsigned int  hsyncPeriodMax;
    unsigned int  lineLengthMin;
    unsigned int  lineLengthMax;
    float       lineLengthFilter;
    int         vsyncThreshold1;
    int         vsyncThreshold2;
    int         vsyncReload;
    int         lineReload;
    size_t      lineBufBytes;
    size_t      lineBufLength;
    uint8_t     lineBufFlags;
    size_t      framesWritten;
    size_t      duplicateFrames;
    size_t      fileSize;
    VideoDisplay::DisplayParameters displayParameters;
    AudioConverter  *audioConverter;
    size_t      aviHeaderSize;
    void        (*errorCallback)(void *userData, const char *msg);
    void        *errorCallbackUserData;
    void        (*fileNameCallback)(void *userData, std::string& fileName);
    void        *fileNameCallbackUserData;
    // ----------------
    static void aviHeader_writeFourCC(uint8_t*& bufp, const char *s);
    static void aviHeader_writeUInt16(uint8_t*& bufp, uint16_t n);
    static void aviHeader_writeUInt32(uint8_t*& bufp, uint32_t n);
    static void defaultErrorCallback(void *userData, const char *msg);
    static void defaultFileNameCallback(void *userData, std::string& fileName);
    virtual void decodeLine(int lineNum) = 0;
    virtual void clearLine(int lineNum) = 0;
    virtual void frameDone() = 0;
    virtual void writeAVIHeader() = 0;
    virtual void writeAVIIndex() = 0;
    void lineDone();
    void closeFile();
    void errorMessage(const char *msg);
   public:
    VideoCapture(int frameRate_ = 50);
    virtual ~VideoCapture();
    void runOneCycle(const uint8_t *videoInput, int16_t audioInput);
    void setClockFrequency(size_t freq_);
    void setNTSCMode(bool ntscMode);
    void openFile(const char *fileName);
    void setErrorCallback(void (*func)(void *userData, const char *msg),
                          void *userData_);
    void setFileNameCallback(void (*func)(void *userData,
                                          std::string& fileName),
                             void *userData_);
  };

  // --------------------------------------------------------------------------

  class VideoCapture_RLE8 : public VideoCapture {
   private:
    class VideoCaptureFrameBuffer {
     private:
      uint32_t  *buf;
      uint8_t   **linePtrs;
      uint32_t  *lineBytes_;
     public:
      VideoCaptureFrameBuffer(int w, int h);
      virtual ~VideoCaptureFrameBuffer();
      inline uint8_t * operator[](long n)
      {
        return linePtrs[n];
      }
      inline const uint8_t * operator[](long n) const
      {
        return linePtrs[n];
      }
      inline uint32_t& lineBytes(long n)
      {
        return lineBytes_[n];
      }
      inline const uint32_t& lineBytes(long n) const
      {
        return lineBytes_[n];
      }
      // returns true if the lines are identical
      inline bool compareLine(long dstLine,
                              const VideoCaptureFrameBuffer& src, long srcLine);
      inline void copyLine(long dstLine, long srcLine);
      inline void copyLine(long dstLine,
                           const VideoCaptureFrameBuffer& src, long srcLine);
      inline void clearLine(long n);
    };
    // --------
    VideoCaptureFrameBuffer tmpFrameBuf;    // 384x288
    VideoCaptureFrameBuffer outputFrameBuf; // 384x288
    uint32_t    *frameSizes;
    int         cycleCnt;
    VideoDisplayColormap<uint8_t> colormap;
    // ----------------
   protected:
    virtual void decodeLine(int lineNum);
    virtual void clearLine(int lineNum);
    virtual void frameDone();
    virtual void writeAVIHeader();
    virtual void writeAVIIndex();
   private:
    size_t rleCompressLine(uint8_t *outBuf, const uint8_t *inBuf);
    void writeFrame(bool frameChanged);
   public:
    VideoCapture_RLE8(void indexToYUVFunc(uint8_t color, bool isNTSC,
                                          float& y, float& u, float& v) =
                          (void (*)(uint8_t, bool, float&, float&, float&)) 0,
                      int frameRate_ = 50);
    virtual ~VideoCapture_RLE8();
  };

  // --------------------------------------------------------------------------

  class VideoCapture_YV12 : public VideoCapture {
   private:
    uint32_t    *videoBuf;              // space for all YUV video data:
    uint8_t     *frameBuf0Y;            // 384x288
    uint8_t     *frameBuf0V;            // 192x144
    uint8_t     *frameBuf0U;            // 192x144
    uint8_t     *frameBuf1Y;            // 384x288
    uint8_t     *frameBuf1V;            // 192x144
    uint8_t     *frameBuf1U;            // 192x144
    int32_t     *interpBufY;            // 384x288
    int32_t     *interpBufV;            // 192x144
    int32_t     *interpBufU;            // 192x144
    uint8_t     *outBufY;               // 384x288
    uint8_t     *outBufV;               // 192x144
    uint8_t     *outBufU;               // 192x144
    uint8_t     *duplicateFrameBitmap;
    VideoDisplayColormap<uint32_t>  colormap;
    // ----------------
   protected:
    virtual void decodeLine(int lineNum);
    virtual void clearLine(int lineNum);
    virtual void frameDone();
    virtual void writeAVIHeader();
    virtual void writeAVIIndex();
   private:
    void resampleFrame();
    void writeFrame(bool frameChanged);
   public:
    VideoCapture_YV12(void indexToYUVFunc(uint8_t color, bool isNTSC,
                                          float& y, float& u, float& v) =
                          (void (*)(uint8_t, bool, float&, float&, float&)) 0,
                      int frameRate_ = 30);
    virtual ~VideoCapture_YV12();
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_VIDEOREC_HPP

