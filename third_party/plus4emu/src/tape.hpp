
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

#ifndef PLUS4EMU_TAPE_HPP
#define PLUS4EMU_TAPE_HPP

#include "plus4emu.hpp"
#include <vector>
#include <sndfile.h>

namespace Plus4Emu {

  class Tape {
   protected:
    long      sampleRate;       // defaults to 24000
    int       fileBitsPerSample;
    int       requestedBitsPerSample;
    bool      isReadOnly;
    bool      isPlaybackOn;
    bool      isRecordOn;
    bool      isMotorOn;
    size_t    tapeLength;       // tape length (in samples)
    size_t    tapePosition;     // current read/write position (in samples)
    int       inputState;
    int       outputState;
    Tape(int bitsPerSample = 1);
   public:
    virtual ~Tape();
    /*!
     * Get sample rate of tape emulation.
     */
    inline long getSampleRate() const
    {
      return sampleRate;
    }
    /*!
     * Get number of bits (1, 2, 4, or 8) per sample in tape file.
     */
    inline int getSampleSize() const
    {
      return fileBitsPerSample;
    }
    /*!
     * Returns true if the tape file is opened in read-only mode.
     */
    inline bool getIsReadOnly() const
    {
      return isReadOnly;
    }
   protected:
    virtual void runOneSample_();
   public:
    /*!
     * Run tape emulation for a period of 1.0 / getSampleRate() seconds.
     */
    inline void runOneSample()
    {
      if (isPlaybackOn && isMotorOn)
        runOneSample_();
    }
    /*!
     * Turn motor on (newState = true) or off (newState = false).
     */
    virtual void setIsMotorOn(bool newState);
    /*!
     * Returns true if the motor is currently on.
     */
    inline bool getIsMotorOn() const
    {
      return isMotorOn;
    }
    /*!
     * Start playback.
     * (note: the motor should also be turned on to actually play)
     */
    inline void play()
    {
      isPlaybackOn = true;
      isRecordOn = false;
    }
    /*!
     * Start recording; if the tape file is read-only, this is equivalent
     * to calling play().
     * (note: the motor should also be turned on to actually record)
     */
    inline void record()
    {
      isPlaybackOn = true;
      isRecordOn = !isReadOnly;
    }
    /*!
     * Stop playback and recording.
     */
    virtual void stop();
    /*!
     * Set input signal for recording.
     */
    inline void setInputSignal(int newState)
    {
      inputState = newState;
    }
    /*!
     * Get output signal.
     */
    inline int getOutputSignal() const
    {
      return outputState;
    }
    /*!
     * Seek to the specified time (in seconds).
     */
    virtual void seek(double t);
    /*!
     * Returns the current tape position in seconds.
     */
    inline double getPosition() const
    {
      return (double(uint32_t(tapePosition)) / double(sampleRate));
    }
    /*!
     * Returns the current tape length in seconds.
     */
    inline double getLength() const
    {
      return (double(uint32_t(tapeLength)) / double(sampleRate));
    }
    /*!
     * Returns true if the current position is at the end of the tape;
     * should be used only when reading a tape file.
     */
    inline bool getIsEndOfTape() const
    {
      return (tapePosition >= tapeLength);
    }
    /*!
     * Seek forward (if isForward = true) or backward (if isForward = false)
     * to the nearest cue point, or by 't' seconds if no cue point is found.
     */
    virtual void seekToCuePoint(bool isForward = true, double t = 10.0);
    /*!
     * Create a new cue point at the current tape position.
     * Has no effect if the file does not have a cue point table, or it
     * is read-only.
     */
    virtual void addCuePoint();
    /*!
     * Delete the cue point nearest to the current tape position.
     * Has no effect if the file is read-only.
     */
    virtual void deleteNearestCuePoint();
    /*!
     * Delete all cue points. Has no effect if the file is read-only.
     */
    virtual void deleteAllCuePoints();
  };

  class Tape_Plus4Emu : public Tape {
   private:
    std::FILE *f;               // tape image file
    uint8_t   *buf;             // 4096 bytes
    uint32_t  *fileHeader;      // table with a length of 1024, contains:
                                //      0, 1: magic number (0x0275CD72,
                                //            0x1C445126)
                                //         2: number of bits per sample in
                                //            file (1, 2, 4, or 8)
                                //         3: sample rate in Hz (10000 to
                                //            120000)
                                // 4 to 1022: the sample positions where cue
                                //            points are defined, in sorted
                                //            order, padded with 0xFFFFFFFF
                                //            values
                                //      1023: must be 0xFFFFFFFF
    size_t    cuePointCnt;
    bool      isBufferDirty;    // true if 'buf' has been changed,
                                // and not written to file yet
    bool      usingNewFormat;
    // ----------------
    void seek_(size_t pos_);
    bool findCuePoint_(size_t& ndx_, size_t pos_);
    void packSamples_();
    bool writeBuffer_();
    bool readBuffer_();
    void unpackSamples_();
    bool writeHeader_();
    void flushBuffer_();
    // called by the constructors
    void setImageFile(std::FILE *imageFile_, const char *fileName, int mode,
                      long sampleRate_, int bitsPerSample);
   public:
    /*!
     * Open tape file 'fileName'. If the file does not exist yet, it may be
     * created (depending on the 'mode' parameter) with the specified sample
     * rate and bits per sample. Otherwise, 'sampleRate_' is ignored, and
     * samples are converted according to 'bitsPerSample'.
     * 'mode' can be one of the following values:
     *   0: open tape file read-write if possible, or create a new file if
     *      it does not exist
     *   1: open tape file read-write if possible, and fail if it does not
     *      exist
     *   2: open an existing tape file read-only
     *   3: create new tape file in read-write mode; if the file already
     *      exists, it is truncated and a new header is written
     */
    Tape_Plus4Emu(const char *fileName, int mode = 0,
                  long sampleRate_ = 24000L, int bitsPerSample = 1);
    /*!
     * Use existing tape file 'imageFile_'.
     */
    Tape_Plus4Emu(std::FILE *imageFile_, int mode = 0,
                  long sampleRate_ = 24000L, int bitsPerSample = 1);
    virtual ~Tape_Plus4Emu();
   protected:
    /*!
     * Run tape emulation for a period of 1.0 / getSampleRate() seconds.
     */
    virtual void runOneSample_();
   public:
    /*!
     * Turn motor on (newState = true) or off (newState = false).
     */
    virtual void setIsMotorOn(bool newState);
    /*!
     * Stop playback and recording.
     */
    virtual void stop();
    /*!
     * Seek to the specified time (in seconds).
     */
    virtual void seek(double t);
    /*!
     * Seek forward (if isForward = true) or backward (if isForward = false)
     * to the nearest cue point, or by 't' seconds if no cue point is found.
     */
    virtual void seekToCuePoint(bool isForward = true, double t = 10.0);
    /*!
     * Create a new cue point at the current tape position.
     * Has no effect if the file does not have a cue point table, or it
     * is read-only.
     */
    virtual void addCuePoint();
    /*!
     * Delete the cue point nearest to the current tape position.
     * Has no effect if the file is read-only.
     */
    virtual void deleteNearestCuePoint();
    /*!
     * Delete all cue points. Has no effect if the file is read-only.
     */
    virtual void deleteAllCuePoints();
  };

  class Tape_C16 : public Tape {
   private:
    std::FILE *f;
    bool      endOfTape;
    bool      usingOldTapFormat;
    int       inputSignal;
    int       inputCnt;
    int       savedInputCnt;
    long      seekIndex[2700];  // 45 minutes max
    long      posIndex[2700];  // 45 minutes max
   public:
    /*!
     * Use C16 format tape file 'imageFile_' read-only.
     */
    Tape_C16(std::FILE *imageFile_, int bitsPerSample = 1);
    virtual ~Tape_C16();
    /*!
     * Run tape emulation for a period of 1.0 / getSampleRate() seconds.
     */
   protected:
    virtual void runOneSample_();
   public:
    /*!
     * Turn motor on (newState = true) or off (newState = false).
     */
    virtual void setIsMotorOn(bool newState);
    /*!
     * Stop playback and recording.
     */
    virtual void stop();
    /*!
     * Seek to the specified time (in seconds).
     */
    virtual void seek(double t);
    /*!
     * Seek forward (if isForward = true) or backward (if isForward = false)
     * to the nearest cue point, or by 't' seconds if no cue point is found.
     */
    virtual void seekToCuePoint(bool isForward = true, double t = 10.0);
    /*!
     * Create a new cue point at the current tape position.
     * Has no effect if the file does not have a cue point table, or it
     * is read-only.
     */
    virtual void addCuePoint();
    /*!
     * Delete the cue point nearest to the current tape position.
     * Has no effect if the file is read-only.
     */
    virtual void deleteNearestCuePoint();
    /*!
     * Delete all cue points. Has no effect if the file is read-only.
     */
    virtual void deleteAllCuePoints();
  };

  class Tape_SoundFile : public Tape {
   public:
    class TapeFilter {
     private:
      std::vector<float> irFFT;
      std::vector<float> fftBuf;
      std::vector<float> outBuf;
      size_t  sampleCnt;
     public:
      TapeFilter(size_t irSamples = 1024);
      virtual ~TapeFilter();
      void setFilterParameters(float sampleRate, float minFreq, float maxFreq);
      float processSample(float inputSignal);
      static void fft(float *buf, size_t n, bool isInverse);
    };
   private:
    std::FILE   *f;             // tape image file
    SNDFILE     *sf;
    std::vector<short>  buf;    // 1024 interleaved sample frames
    int         nChannels;
    int         requestedChannel;
    bool        invertSignal;
    bool        enableFIRFilter;
    bool        isBufferDirty;  // true if 'buf' has been changed,
                                // and not written to file yet
    TapeFilter  firFilter;
    // ----------------
    void seek_(size_t pos_);
    bool writeBuffer_();
    void flushBuffer_();
    void invertBuffer_();
   public:
    /*!
     * Use existing tape file 'imageFile_' in the specified mode:
     *   0, 1: read-write
     *   2:    read-only
     */
    Tape_SoundFile(std::FILE *imageFile_, int mode = 0, int bitsPerSample = 1);
    virtual ~Tape_SoundFile();
    /*!
     * Run tape emulation for a period of 1.0 / getSampleRate() seconds.
     */
   protected:
    virtual void runOneSample_();
   public:
    /*!
     * Turn motor on (newState = true) or off (newState = false).
     */
    virtual void setIsMotorOn(bool newState);
    /*!
     * Stop playback and recording.
     */
    virtual void stop();
    /*!
     * Seek to the specified time (in seconds).
     */
    virtual void seek(double t);
    /*!
     * Seek forward (if isForward = true) or backward (if isForward = false)
     * to the nearest cue point, or by 't' seconds if no cue point is found.
     */
    virtual void seekToCuePoint(bool isForward = true, double t = 10.0);
    /*!
     * Create a new cue point at the current tape position.
     * Has no effect if the file does not have a cue point table, or it
     * is read-only.
     */
    virtual void addCuePoint();
    /*!
     * Delete the cue point nearest to the current tape position.
     * Has no effect if the file is read-only.
     */
    virtual void deleteNearestCuePoint();
    /*!
     * Delete all cue points. Has no effect if the file is read-only.
     */
    virtual void deleteAllCuePoints();
    /*!
     * Set parameters for sound file reading.
     */
    void setParameters(int requestedChannel_, bool invertSignal_,
                       bool enableFIRFilter_,
                       float filterMinFreq_, float filterMaxFreq_);
  };

  /*!
   * Open tape file 'fileName'. If the file does not exist yet, it may be
   * created (depending on the 'mode' parameter) with the specified sample
   * rate and bits per sample. Otherwise, 'sampleRate_' is ignored, and
   * samples are converted according to 'bitsPerSample'.
   * 'mode' can be one of the following values:
   *   0: open tape file read-write if possible, or create a new file if
   *      it does not exist
   *   1: open tape file read-write if possible, and fail if it does not
   *      exist
   *   2: open an existing tape file read-only
   *   3: create new tape file in read-write mode; if the file already
   *      exists, it is truncated and a new header is written
   */
  Tape *openTapeFile(const char *fileName, int mode = 0,
                     long sampleRate_ = 24000L, int bitsPerSample = 1);

}       // namespace Plus4Emu

#endif  // PLUS4EMU_TAPE_HPP

