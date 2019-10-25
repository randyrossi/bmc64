
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef PLUS4EMU_VM_HPP
#define PLUS4EMU_VM_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "bplist.hpp"
#include "display.hpp"
#include "snd_conv.hpp"
#include "soundio.hpp"
#include "tape.hpp"

namespace Plus4Emu {

  class VirtualMachine {
   protected:
    VideoDisplay&   display;
   private:
    AudioOutput&    audioOutput;
    AudioConverter  *audioConverter;
    bool            writingAudioOutput;
    bool            audioOutputEnabled;
    bool            audioOutputHighQuality;
    bool            displayEnabled;
    float           audioConverterSampleRate;
    float           audioOutputSampleRate;
    float           audioOutputVolume;
    float           audioOutputFilter1Freq;
    float           audioOutputFilter2Freq;
    int             audioOutputEQMode;
    float           audioOutputEQFrequency;
    float           audioOutputEQLevel;
    float           audioOutputEQ_Q;
    bool            tapePlaybackOn;
    bool            tapeRecordOn;
    bool            tapeMotorOn;
    Tape            *tape;
    std::string     tapeFileName;
    long            defaultTapeSampleRate;
    int             tapeSoundFileChannel;
    bool            tapeSoundFileInvertSignal;
    bool            tapeEnableSoundFileFilter;
    float           tapeSoundFileFilterMinFreq;
    float           tapeSoundFileFilterMaxFreq;
   protected:
    void            (*breakPointCallback)(void *userData,
                                          int debugContext_, int type,
                                          uint16_t addr, uint8_t value);
    void            *breakPointCallbackUserData;
    int             currentDebugContext;
   private:
    std::string     fileIOWorkingDirectory;
    void            (*fileNameCallback)(void *userData, std::string& fileName);
    void            *fileNameCallbackUserData;
   public:
    struct VMStatus {
      bool      isRecordingDemo;
      bool      isPlayingDemo;
      bool      tapeReadOnly;
      double    tapePosition;
      double    tapeLength;
      long      tapeSampleRate;
      int       tapeSampleSize;
      // floppy drive LED state is the sum of any of the following values:
      //   0x00000001: drive 0 red LED is on
      //   0x00000002: drive 0 green LED is on
      //   0x00000004: drive 0 blue LED is on
      //   0x00000100: drive 1 red LED is on
      //   0x00000200: drive 1 green LED is on
      //   0x00000400: drive 1 blue LED is on
      //   0x00010000: drive 2 red LED is on
      //   0x00020000: drive 2 green LED is on
      //   0x00040000: drive 2 blue LED is on
      //   0x01000000: drive 3 red LED is on
      //   0x02000000: drive 3 green LED is on
      //   0x04000000: drive 3 blue LED is on
      uint32_t  floppyDriveLEDState;
      // for each drive, the head position is encoded as a 16-bit value (bits
      // 0 to 15 for drive 0, bits 16 to 31 for drive 1, etc.):
      //   bits 0 to 6:   sector number
      //   bit 7:         side selected
      //   bits 8 to 14:  track number
      //   bit 15:        0: 40 tracks, 1: 80 tracks
      // if a particular drive does not exist, or no disk image is set, 0xFFFF
      // is returned as head position
      uint64_t  floppyDriveHeadPositions;
      int       printerHeadPositionX;
      int       printerHeadPositionY;
      bool      printerOutputChanged;
      uint8_t   printerLEDState;
    };
    static const uint64_t defaultRAMPattern =
        (uint64_t(0x0000E000UL) | (uint64_t(0x000001F7UL) << 32));
    static const char     *defaultRAMPatternString;
    // --------
    VirtualMachine(VideoDisplay& display_, AudioOutput& audioOutput_);
    virtual ~VirtualMachine();
    /*!
     * Returns reference to the associated video display object.
     */
    inline VideoDisplay& getVideoDisplay()
    {
      return this->display;
    }
    /*!
     * Returns reference to the associated audio output object.
     */
    inline AudioOutput& getAudioOutput()
    {
      return this->audioOutput;
    }
    /*!
     * Run emulation for the specified number of microseconds.
     */
    virtual void run(size_t microseconds);
    /*!
     * Reset emulated machine; if 'isColdReset' is true, RAM is cleared.
     */
    virtual void reset(bool isColdReset = false);
    /*!
     * Delete all ROM segments, and resize RAM to 'memSize' kilobytes;
     * implies calling reset(true). RAM is cleared to a pattern defined
     * by 'ramPattern':
     *   bits 0 to 2:   address line (0 to 7) for initial value of data bit 0
     *   bit 3:         invert bit 0
     *   bits 4 to 6:   address line (0 to 7) for initial value of data bit 1
     *   bit 7:         invert bit 1
     *   bits 8 to 10:  address line (0 to 7) for initial value of data bit 2
     *   bit 11:        invert bit 2
     *   bits 12 to 14: address line (0 to 7) for initial value of data bit 3
     *   bit 15:        invert bit 3
     *   bits 16 to 18: address line (0 to 7) for initial value of data bit 4
     *   bit 19:        invert bit 4
     *   bits 20 to 22: address line (0 to 7) for initial value of data bit 5
     *   bit 23:        invert bit 5
     *   bits 24 to 26: address line (0 to 7) for initial value of data bit 6
     *   bit 27:        invert bit 6
     *   bits 28 to 30: address line (0 to 7) for initial value of data bit 7
     *   bit 31:        invert bit 7
     *   bits 32 to 39: XOR value for bytes at the beginning of 256 byte pages
     *   bits 40 to 47: probability of random bytes (0: none, 255: maximum)
     */
    virtual void resetMemoryConfiguration(
        size_t memSize, uint64_t ramPattern = defaultRAMPattern);
    /*!
     * Load ROM segment 'n' from the specified file, skipping 'offs' bytes.
     */
    virtual void loadROMSegment(uint8_t n, const char *fileName, size_t offs);
    /*!
     * Set audio output quality.
     */
    virtual void setAudioOutputHighQuality(bool useHighQualityResample);
    /*!
     * Set cutoff frequencies of highpass filters used on audio output to
     * remove DC offset.
     */
    virtual void setAudioOutputFilters(float dcBlockFreq1_,
                                       float dcBlockFreq2_);
    /*!
     * Set parameters of audio output equalizer.
     * 'mode_' can be one of the following values: -1: disable equalizer,
     * 0: peaking EQ, 1: low shelf, 2: high shelf.
     */
    virtual void setAudioOutputEqualizer(int mode_,
                                         float freq_, float level_, float q_);
    /*!
     * Set amplitude scale for audio output (defaults to 0.7943).
     */
    virtual void setAudioOutputVolume(float ampScale_);
    /*!
     * Set if audio data is sent to sound card and output file (disabling
     * this also makes the emulation run faster than real time).
     */
    virtual void setEnableAudioOutput(bool isEnabled);
    /*!
     * Set if video data is sent to the associated VideoDisplay object.
     */
    virtual void setEnableDisplay(bool isEnabled);
    /*!
     * Set CPU clock frequency (in Hz).
     */
    virtual void setCPUFrequency(size_t freq_);
    /*!
     * Set the number of video 'slots' per second.
     */
    virtual void setVideoFrequency(size_t freq_);
    /*!
     * Set if the 6551 ACIA should be emulated.
     */
    virtual void setEnableACIAEmulation(bool isEnabled);
    /*!
     * Set SID emulation parameters. 'outputVolume' should be specified in
     * decibels (-8 to +2). 'sidFlags_' can be the sum of:
     *   1: SID model is 6581
     *   2: enable write access at $D400-$D41F
     *   4: run SID emulation at C64 clock frequency
     */
    virtual void setSIDConfiguration(uint8_t sidFlags_, bool enableDigiBlaster,
                                     int outputVolume);
    /*!
     * Disable SID emulation (which is automatically enabled by writing to
     * any of the SID registers) to reduce CPU usage.
     */
    virtual void disableSIDEmulation();
    /*!
     * Set state of key 'keyCode' (0 to 127).
     */
    virtual void setKeyboardState(int keyCode, bool isPressed);
    /*!
     * Set light pen position. 'xPos' and 'yPos' should be in the range
     * 0 to 65535 for the visible 768x576 display area. Values that are
     * out of range turn off the light pen.
     */
    virtual void setLightPenPosition(int xPos, int yPos);
    /*!
     * Set the cursor position. 'xPos' and 'yPos' should be in the range
     * 0 to 65535 for the visible 768x576 display area.
     */
    virtual void setCursorPosition(int xPos, int yPos);
    /*!
     * Copy text from the screen memory. 'xPos' and 'yPos' can be a small
     * integer (0 to 39, and 0 to 24, respectively) to copy from a specific
     * line or the word at 'xPos', a large integer (100 to 65535) to copy from
     * a line or word at a physical screen position, or -1 to copy the line or
     * word at the current cursor position.
     * If 'yPos' is less than -1, the whole screen is copied as multiple lines,
     * and 'xPos' is ignored; otherwise, if 'xPos' is less than -1, then a
     * (logical) line, otherwise a word is copied.
     */
    virtual std::string copyText(int xPos, int yPos) const;
    /*!
     * Paste the text from 's' to the keyboard buffer of the emulated machine,
     * optionally setting the cursor position to 'xPos' and 'yPos' (0 to 65535)
     * first if both are non-negative.
     */
    virtual void pasteText(const char *s, int xPos, int yPos);
    /*!
     * Set the type of printer to be emulated:
     *   0: disable printer emulation (default)
     *   1: MPS-801 (IEC level emulation)
     *   2: 1526/MPS-802 (hardware level emulation)
     *   3: 1526/MPS-802 in 1525 mode (hardware level emulation)
     */
    virtual void setPrinterType(int n);
    /*!
     * Get the current printer output as an 8-bit greyscale image.
     * 'buf_' contains 'w_' * 'h_' bytes. If there is no printer, a NULL
     * buffer pointer, and zero width and height will be returned.
     */
    virtual void getPrinterOutput(const uint8_t*& buf_,
                                  int& w_, int& h_) const;
    /*!
     * Clear the printer output buffer, and reset the head position to
     * the top of the page.
     */
    virtual void clearPrinterOutput();
    /*!
     * Returns the current state of printer LEDs as a bitmap (if bit 0 is set,
     * LED 1 is on, if bit 1 is set, LED 2 is on, etc.).
     */
    virtual uint8_t getPrinterLEDState() const;
    /*!
     * Returns the current position of the printer head. 'xPos' is in the
     * range 0 (left) to page width - 1 (right), 'yPos' is in the range 0
     * (top) to page height - 1 (bottom). The page width and height can be
     * determined with getPrinterOutput().
     * If printer emulation is not enabled, -1,-1 is returned.
     */
    virtual void getPrinterHeadPosition(int& xPos, int& yPos);
    /*!
     * Returns true if the printer output has changed since the last call of
     * clearPrinterOutputChangedFlag().
     */
    virtual bool getIsPrinterOutputChanged() const;
    virtual void clearPrinterOutputChangedFlag();
    /*!
     * Set the state of the printer form feed button (false: off, true: on).
     */
    virtual void setPrinterFormFeedOn(bool isEnabled);
    /*!
     * Write the printer output to 'fileName'. If 'asciiMode' is false, then
     * the raw character data received on the serial bus is written, otherwise
     * it is converted to ASCII and written as a standard text file.
     * If 'fileName' is NULL or empty, then a previously opened file is closed.
     * On error, such as not being able to open the output file,
     * Plus4Emu::Exception may be thrown.
     * NOTE: printer emulation must be enabled before calling this function.
     */
    virtual void setPrinterTextOutputFile(const char *fileName,
                                          bool asciiMode = false);
    /*!
     * Returns status information about the emulated machine (see also
     * struct VMStatus above, and the comments for functions that return
     * individual status values).
     */
    virtual void getVMStatus(VMStatus& vmStatus_);
    /*!
     * Create video capture object with the specified frame rate (24 to 60)
     * and format (384x288 RLE8 or 384x288 YV12) if it does not exist yet,
     * and optionally set callbacks for printing error messages and asking
     * for a new output file on reaching 2 GB file size.
     */
    virtual void openVideoCapture(
        int frameRate_ = 30,
        bool yuvFormat_ = true,
        void (*errorCallback_)(void *userData, const char *msg) =
            (void (*)(void *, const char *)) 0,
        void (*fileNameCallback_)(void *userData, std::string& fileName) =
            (void (*)(void *, std::string&)) 0,
        void *userData_ = (void *) 0);
    /*!
     * Set output file name for video capture (an empty file name means no
     * file is written). openVideoCapture() should be called first.
     */
    virtual void setVideoCaptureFile(const std::string& fileName_);
    /*!
     * Set if the TV standard for video capture should be PAL (false, default)
     * or NTSC (true).
     */
    virtual void setVideoCaptureNTSCMode(bool ntscMode);
    /*!
     * Destroy video capture object, freeing all allocated memory and closing
     * the output file.
     */
    virtual void closeVideoCapture();
    // -------------------------- DISK AND FILE I/O ---------------------------
    /*!
     * Load disk image for drive 'n' (counting from zero); an empty file
     * name means no disk.
     */
    virtual void setDiskImageFile(int n, const std::string& fileName_,
                                  int driveType = 0);
    /*!
     * Returns the current state of the floppy drive LEDs, which is the sum
     * of any of the following values:
     *   0x00000001: drive 0 red LED is on
     *   0x00000002: drive 0 green LED is on
     *   0x00000004: drive 0 blue LED is on
     *   0x00000100: drive 1 red LED is on
     *   0x00000200: drive 1 green LED is on
     *   0x00000400: drive 1 blue LED is on
     *   0x00010000: drive 2 red LED is on
     *   0x00020000: drive 2 green LED is on
     *   0x00040000: drive 2 blue LED is on
     *   0x01000000: drive 3 red LED is on
     *   0x02000000: drive 3 green LED is on
     *   0x04000000: drive 3 blue LED is on
     */
    virtual uint32_t getFloppyDriveLEDState() const;
    /*!
     * Returns the current head position for all floppy drives.
     * For each drive, the head position is encoded as a 16-bit value (bits
     * 0 to 15 for drive 0, bits 16 to 31 for drive 1, etc.):
     *   bits 0 to 6:   sector number
     *   bit 7:         side selected
     *   bits 8 to 14:  track number
     *   bit 15:        0: 40 tracks, 1: 80 tracks
     * If a particular drive does not exist, or no disk image is set, 0xFFFF
     * is returned as head position.
     */
    virtual uint64_t getFloppyDriveHeadPositions() const;
    /*!
     * Set if the floppy drive emulation should use higher timing resolution
     * at the expense of increased CPU usage. The default is 'true'.
     */
    virtual void setFloppyDriveHighAccuracy(bool isEnabled);
    /*!
     * Set the serial bus delay offset to 'n' (-100 to 100) nanoseconds for
     * devices that support this configuration option.
     */
    virtual void setSerialBusDelayOffset(int n);
    /*!
     * Disable all floppy drives with no disk image attached to reduce CPU
     * usage.
     */
    virtual void disableUnusedFloppyDrives();
    /*!
     * Reset floppy drive 'n' (0 to 3), or all drives if 'n' is negative.
     */
    virtual void resetFloppyDrive(int n);
    /*!
     * Set if the IEC drive emulation should be allowed to write to the
     * working directory.
     */
    virtual void setIECDriveReadOnlyMode(bool isReadOnly);
    /*!
     * Set working directory for IEC drive emulation.
     */
    virtual void setWorkingDirectory(const std::string& dirName_);
    /*!
     * Set function to be called when the emulated machine tries to open a
     * file with unspecified name. 'fileName' should be set to the name of the
     * file to be opened.
     */
    virtual void setFileNameCallback(void (*fileNameCallback_)(
                                         void *userData,
                                         std::string& fileName),
                                     void *userData_);
    // ---------------------------- TAPE EMULATION ----------------------------
    /*!
     * Set tape image file name (if the file name is NULL or empty, tape
     * emulation is disabled).
     */
    virtual void setTapeFileName(const std::string& fileName);
    /*!
     * Set sample rate (in Hz) to be used when creating a new tape image file.
     * If the file already exists, this setting is ignored, and the value
     * stored in the file header is used instead.
     */
    virtual void setDefaultTapeSampleRate(long sampleRate_ = 24000L);
    /*!
     * Send tape input/output signals to the sound output. The absolute value
     * of 'n' (in the range 0 to 10) controls the output level, negative
     * values invert the signal. If 'n' is zero, tape feedback is disabled.
     */
    virtual void setTapeFeedbackLevel(int n);
    /*!
     * Returns the actual sample rate of the tape file, or zero if there is no
     * tape image file opened.
     */
    virtual long getTapeSampleRate() const;
    /*!
     * Returns the number of bits per sample in the tape file, or zero if there
     * is no tape image file opened.
     */
    virtual int getTapeSampleSize() const;
    /*!
     * Returns true if the tape is opened in read-only mode.
     */
    virtual bool getIsTapeReadOnly() const;
    /*!
     * Start tape playback.
     */
    virtual void tapePlay();
    /*!
     * Start tape recording; if the tape file is read-only, this is
     * equivalent to calling tapePlay().
     */
    virtual void tapeRecord();
    /*!
     * Stop tape playback and recording.
     */
    virtual void tapeStop();
    /*!
     * Set tape position to the specified time (in seconds).
     */
    virtual void tapeSeek(double t);
    /*!
     * Returns the current tape position in seconds, or -1.0 if there is
     * no tape image file opened.
     */
    virtual double getTapePosition() const;
    /*!
     * Returns the current length of the tape file in seconds, or -1.0 if
     * there is no tape image file opened.
     */
    virtual double getTapeLength() const;
    /*!
     * Seek forward (if isForward = true) or backward (if isForward = false)
     * to the nearest cue point, or by 't' seconds if no cue point is found.
     */
    virtual void tapeSeekToCuePoint(bool isForward = true, double t = 10.0);
    /*!
     * Create a new cue point at the current tape position.
     * Has no effect if the file does not have a cue point table, or it
     * is read-only.
     */
    virtual void tapeAddCuePoint();
    /*!
     * Delete the cue point nearest to the current tape position.
     * Has no effect if the file is read-only.
     */
    virtual void tapeDeleteNearestCuePoint();
    /*!
     * Delete all cue points. Has no effect if the file is read-only.
     */
    virtual void tapeDeleteAllCuePoints();
    /*!
     * Set parameters for tape sound file I/O.
     */
    virtual void setTapeSoundFileParameters(int requestedChannel_,
                                            bool invertSignal_,
                                            bool enableFilter_,
                                            float filterMinFreq_,
                                            float filterMaxFreq_);
    // ------------------------------ DEBUGGING -------------------------------
    /*!
     * Set the debugging context (CPU number).
     */
    virtual void setDebugContext(int n);
    /*!
     * Returns the current debugging context (CPU number).
     */
    virtual int getDebugContext() const;
    /*!
     * Add new breakpoint with the specified type, address, and priority
     * (0 to 3). The allowed values for 'bpType' are:
     *   0: any memory access (read, write or CPU opcode read)
     *   1: memory read
     *   2: memory write
     *   3: any memory access, same as bpType == 0
     *   4: video (address bits 7..15 for Y, bits 0..6 for X)
     *   5: ignore other breakpoints if the program counter is at this address
     *   6: CPU opcode read
     * A negative priority value deletes any existing breakpoint at 'bpAddr'.
     */
    virtual void setBreakPoint(int bpType, uint16_t bpAddr, int bpPriority);
    /*!
     * Add breakpoints from the specified breakpoint list (see also
     * bplist.hpp).
     */
    virtual void setBreakPoints(const BreakPointList& bpList);
    /*!
     * Clear all breakpoints.
     */
    virtual void clearBreakPoints();
    /*!
     * Set breakpoint priority threshold (0 to 4); breakpoints with a
     * priority less than this value will not trigger a break.
     */
    virtual void setBreakPointPriorityThreshold(int n);
    /*!
     * Set if the breakpoint callback should be called whenever the first byte
     * of a CPU instruction is read from memory. 'mode_' can be one of the
     * following values:
     *   0: normal mode
     *   1: single step mode (break on every instruction, ignore breakpoints)
     *   2: step over mode
     *   3: trace (similar to mode 1, but does not ignore breakpoints)
     *   4: step into mode
     */
    virtual void setSingleStepMode(int mode_);
    /*!
     * Set the next address where single step mode will stop, ignoring any
     * other instructions. If 'addr' is negative, then a break is triggered
     * immediately at the next instruction.
     * Note: setSingleStepMode() must be called first with a mode parameter
     * of 2 or 4.
     */
    virtual void setSingleStepModeNextAddress(int32_t addr);
    /*!
     * Set if invalid CPU opcodes should be interpreted as NOPs with
     * a breakpoint set (priority = 3).
     */
    virtual void setBreakOnInvalidOpcode(bool isEnabled);
    /*!
     * Set function to be called when a breakpoint is triggered.
     * 'type' can be one of the following values:
     *   0: breakpoint at opcode read
     *   1: memory read
     *   2: memory write
     *   3: opcode read in single step mode
     *   4: video breakpoint ('addr' is Y * 128 + X / 2)
     */
    virtual void setBreakPointCallback(void (*breakPointCallback_)(
                                           void *userData,
                                           int debugContext_, int type,
                                           uint16_t addr, uint8_t value),
                                       void *userData_);
    /*!
     * Returns the segment at page 'n' (0 to 3).
     */
    virtual uint8_t getMemoryPage(int n) const;
    /*!
     * Read a byte from memory. If 'isCPUAddress' is false, bits 14 to 21 of
     * 'addr' define the segment number, while bits 0 to 13 are the offset
     * (0 to 0x3FFF) within the segment; otherwise, 'addr' is interpreted as
     * a 16-bit CPU address.
     */
    virtual uint8_t readMemory(uint32_t addr, bool isCPUAddress = false) const;
    /*!
     * Write a byte to memory. If 'isCPUAddress' is false, bits 14 to 21 of
     * 'addr' define the segment number, while bits 0 to 13 are the offset
     * (0 to 0x3FFF) within the segment; otherwise, 'addr' is interpreted as
     * a 16-bit CPU address.
     * NOTE: calling this function will stop any demo recording or playback.
     */
    virtual void writeMemory(uint32_t addr, uint8_t value,
                             bool isCPUAddress = false);
    /*!
     * Returns the current value of the CPU program counter (PC).
     */
    virtual uint16_t getProgramCounter() const;
    /*!
     * Returns the CPU address of the last byte pushed to the stack.
     */
    virtual uint16_t getStackPointer() const;
    /*!
     * Dumps the current values of all CPU registers to 'buf' in ASCII format.
     * The register list may be written as multiple lines separated by '\n'
     * characters, however, there is no newline character at the end of the
     * buffer. The maximum line width is 40 characters.
     */
    virtual void listCPURegisters(std::string& buf) const;
    /*!
     * Disassemble one CPU instruction, starting from memory address 'addr',
     * and write the result to 'buf' (not including a newline character).
     * 'offs' is added to the instruction address that is printed.
     * The maximum line width is 40 characters.
     * Returns the address of the next instruction. If 'isCPUAddress' is
     * true, 'addr' is interpreted as a 16-bit CPU address, otherwise it
     * is assumed to be a 22-bit physical address (8 bit segment + 14 bit
     * offset).
     */
    virtual uint32_t disassembleInstruction(std::string& buf, uint32_t addr,
                                            bool isCPUAddress = false,
                                            int32_t offs = 0) const;
    /*!
     * Returns the current horizontal and vertical video position.
     */
    virtual void getVideoPosition(int& xPos, int& yPos) const;
    // ------------------------------- FILE I/O -------------------------------
    /*!
     * Save snapshot of virtual machine state, including all ROM and RAM
     * segments, as well as all hardware registers. Note that the clock
     * frequency and timing settings, tape and disk state, and breakpoint list
     * are not saved.
     */
    virtual void saveState(File& f);
    /*!
     * Save clock frequency and timing settings.
     */
    virtual void saveMachineConfiguration(File& f);
    /*!
     * Register all types of file data supported by this class, for use by
     * File::processAllChunks(). Note that loading snapshot data will clear
     * all breakpoints.
     */
    virtual void registerChunkTypes(File& f);
    /*!
     * Start recording a demo to the file object, which will be used until
     * the recording is stopped for some reason.
     * Implies calling saveMachineConfiguration() and saveState() first.
     */
    virtual void recordDemo(File& f);
    /*!
     * Stop playing or recording demo.
     */
    virtual void stopDemo();
    /*!
     * Returns true if a demo is currently being recorded. The recording stops
     * when stopDemo() is called, any tape or disk I/O is attempted, clock
     * frequency and timing settings are changed, or a snapshot is loaded.
     * This function will also flush demo data to the associated file object
     * after recording is stopped for some reason other than calling
     * stopDemo().
     */
    virtual bool getIsRecordingDemo();
    /*!
     * Returns true if a demo is currently being played. The playback stops
     * when the end of the demo is reached, stopDemo() is called, any tape or
     * disk I/O is attempted, clock frequency and timing settings are changed,
     * or a snapshot is loaded. Note that keyboard events are ignored while
     * playing a demo.
     */
    virtual bool getIsPlayingDemo() const;
    // ----------------
    virtual void loadState(File::Buffer& buf);
    virtual void loadMachineConfiguration(File::Buffer& buf);
    virtual void loadDemo(File::Buffer& buf);
   protected:
    inline void sendAudioOutput(uint32_t audioData)
    {
      if (this->writingAudioOutput)
        this->audioConverter->sendInputSignal(audioData);
    }
    inline void sendAudioOutput(uint16_t left, uint16_t right)
    {
      if (this->writingAudioOutput)
        this->audioConverter->sendInputSignal(uint32_t(left)
                                              | (uint32_t(right) << 16));
    }
    inline void sendMonoAudioOutput(int32_t audioData)
    {
      if (this->writingAudioOutput)
        this->audioConverter->sendInputSignal(audioData);
    }
    /*!
     * This function is similar to the public setTapeFileName(), but allows
     * derived classes to use a different sample size than the default of
     * 1 bit.
     */
    void setTapeFileName(const std::string& fileName, int bitsPerSample);
   private:
    void setTapeMotorState_(bool newState);
   protected:
    inline void setTapeMotorState(bool newState)
    {
      if (newState != this->tapeMotorOn)
        this->setTapeMotorState_(newState);
    }
    inline bool getIsTapeMotorOn() const
    {
      return this->tapeMotorOn;
    }
    inline int getTapeButtonState() const
    {
      if (this->tapePlaybackOn)
        return (this->tapeRecordOn ? 2 : 1);
      return 0;
    }
    inline bool haveTape() const
    {
      return (this->tape != (Tape *) 0);
    }
    inline int runTape(int tapeInput)
    {
      if (this->tape != (Tape *) 0 &&
          this->tapeMotorOn && this->tapePlaybackOn) {
        if (this->tapeRecordOn) {
          this->tape->setInputSignal(tapeInput);
          this->tape->runOneSample();
          return 0;
        }
        else {
          this->tape->runOneSample();
          return (this->tape->getOutputSignal());
        }
      }
      return 0;
    }
    inline bool getIsDisplayEnabled() const
    {
      return this->displayEnabled;
    }
    void setAudioConverterSampleRate(float sampleRate_);
   public:
    /*!
     * Open a file in the user specified working directory. 'baseName_' is the
     * file name without any leading directory components; it is converted to
     * lower case, invalid characters are replaced with underscores, and the
     * file is searched case-insensitively. If 'baseName_' is empty, the file
     * name callback (if any) is called, which should return either a full path
     * file name that will be stored in 'baseName_', or an empty string in
     * which case this function fails and returns -2 (invalid file name).
     * 'mode' is the mode parameter to be passed to std::fopen().
     * On success, the file handle is stored in 'f', and zero is returned.
     * Otherwise, 'f' is set to NULL, and the return value is one of the
     * following error codes:
     *   -1: unknown error
     *   -2: invalid (empty) file name
     *   -3: the file is not found
     *   -4: the file is not a regular file
     *   -5: the file is found but cannot be opened (e.g. permission is
     *       denied), or a new file cannot be created; 'errno' is set
     *       according to the reason for the failure
     *   -6: the file already exists (if 'createOnly_' is true)
     */
    int openFileInWorkingDirectory(std::FILE*& f, std::string& baseName_,
                                   const char *mode, bool createOnly_ = false);
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_VM_HPP

