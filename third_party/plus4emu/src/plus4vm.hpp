
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

#ifndef PLUS4EMU_PLUS4VM_HPP
#define PLUS4EMU_PLUS4VM_HPP

#include "plus4emu.hpp"
#include "ted.hpp"
#include "display.hpp"
#include "soundio.hpp"
#include "vm.hpp"
#include "serial.hpp"
#include "acia6551.hpp"

namespace Plus4Emu {
  class VideoCapture;
}

namespace Plus4 {

  class SID;
  class ParallelIECDrive;

  class Plus4VM : public Plus4Emu::VirtualMachine {
   private:
    class TED7360_ : public TED7360 {
     private:
      Plus4VM&  vm;
      static PLUS4EMU_REGPARM2 uint8_t sidRegisterRead(
          void *userData, uint16_t addr);
      static PLUS4EMU_REGPARM3 void sidRegisterWrite(
          void *userData, uint16_t addr, uint8_t value);
      static PLUS4EMU_REGPARM3 void sidRegisterWriteC64(
          void *userData, uint16_t addr, uint8_t value);
      static PLUS4EMU_REGPARM2 uint8_t parallelIECRead(
          void *userData, uint16_t addr);
      static PLUS4EMU_REGPARM3 void parallelIECWrite(
          void *userData, uint16_t addr, uint8_t value);
      static PLUS4EMU_REGPARM2 uint8_t memoryRead0001Callback(
          void *userData, uint16_t addr);
      static PLUS4EMU_REGPARM3 void memoryWrite0001Callback(
          void *userData, uint16_t addr, uint8_t value);
      static PLUS4EMU_REGPARM2 uint8_t aciaRegisterRead(
          void *userData, uint16_t addr);
      static PLUS4EMU_REGPARM3 void aciaRegisterWrite(
          void *userData, uint16_t addr, uint8_t value);
     public:
      TED7360_(Plus4VM& vm_);
      virtual ~TED7360_();
      virtual void reset(bool cold_reset = false);
      void setEnableC64CompatibleSID(bool isEnabled);
      SerialBus serialPort;
     protected:
      virtual void playSample(int16_t sampleValue);
      virtual void videoOutputCallback(const uint8_t *buf, size_t nBytes);
      virtual void ntscModeChangeCallback(bool isNTSC_);
      virtual void breakPointCallback(int type, uint16_t addr, uint8_t value);
    };
    // ----------------
    static const int  printerDeviceNumber = 4;  // TODO: make it configurable ?
    TED7360_  *ted;
    size_t    cpuClockFrequency;        // defaults to 1
    size_t    tedInputClockFrequency;   // defaults to 17734475 Hz
    size_t    soundClockFrequency;      // fixed at single clock frequency / 4
    int64_t   tedTimesliceLength;       // in 2^-32 microsecond units
    int64_t   tedTimeRemaining;         // -"-
    int64_t   tapeTimesliceLength;      // -"-
    int64_t   tapeTimeRemaining;        // -"-
    Plus4Emu::File  *demoFile;
    // contains demo data, which is the emulator version number as a 32-bit
    // integer ((MAJOR << 16) + (MINOR << 8) + PATCHLEVEL), followed by a
    // sequence of events in the following format:
    //   uint64_t   deltaTime   (in TED cycles, stored as MSB first dynamic
    //                          length (1 to 8 bytes) value)
    //   uint8_t    eventType   (currently allowed values are 0 for end of
    //                          demo (zero data bytes), 1 for key press, and
    //                          2 for key release)
    //   uint8_t    dataLength  number of event data bytes
    //   ...        eventData   (dataLength bytes)
    // the event data for event types 1 and 2 is the key code with a length of
    // one byte:
    //   uint8_t    keyCode     key code in the range 0 to 127
    Plus4Emu::File::Buffer  demoBuffer;
    // true while recording a demo
    bool      isRecordingDemo;
    // true while playing a demo
    bool      isPlayingDemo;
    // true after loading a snapshot; if not playing a demo as well, the
    // keyboard state will be cleared
    bool      snapshotLoadFlag;
    bool      tapeCallbackFlag;
    // used for counting time between demo events (in TED cycles)
    uint64_t  demoTimeCnt;
    SID       *sid_;
    int32_t   soundOutputAccumulator;
    int32_t   soundOutputSignal;
    int32_t   sidOutputVolume;
    bool      sidEnabled;
    bool      digiBlasterEnabled;
    uint8_t   digiBlasterOutput;
    uint8_t   sidCycleCnt;
    // bit 0 = SID model is 6581
    // bit 1 = enable write access at $D400-$D41F
    // bit 2 = run SID emulation at C64 clock frequency
    uint8_t   sidFlags;
    bool      is1541HighAccuracy;
    int16_t   serialBusDelayOffset;
    SerialDevice  *serialDevices[12];
    uint8_t   *floppyROM_1541;
    uint8_t   *floppyROM_1551;
    uint8_t   *floppyROM_1581_0;
    uint8_t   *floppyROM_1581_1;
    uint8_t   *printerROM_MPS801;
    uint8_t   *printerROM_1526;
    size_t    videoBreakPointCnt;
    uint8_t   *videoBreakPoints;
    int32_t   tapeFeedbackSignal;
    // calculated from tapeFeedbackLevel and SID volume
    int32_t   tapeFeedbackMult;
    int       lightPenPositionX;
    int       lightPenPositionY;
    int       lightPenCycleCounter;
    bool      printerOutputChangedFlag;
    bool      printerFormFeedOn;
    bool      videoCaptureNTSCMode;
    int8_t    tapeFeedbackLevel;
    Plus4Emu::VideoCapture  *videoCapture;
    ACIA6551  acia_;
    int64_t   aciaTimeRemaining;        // in 2^-32 microsecond units
    bool      aciaEnabled;
    bool      aciaCallbackFlag;
    bool      drive8Is1551;
    bool      drive9Is1551;
    ParallelIECDrive  *iecDrive8;
    ParallelIECDrive  *iecDrive9;
    int       pasteTextCycleCnt;
    int       pasteTextWaitCnt;
    int       pasteTextCursorPositionX;
    int       pasteTextCursorPositionY;
    size_t    pasteTextBufferPos;
    char      *pasteTextBuffer;
    // ----------------
    void stopDemoPlayback();
    void stopDemoRecording(bool writeFile_);
    void updateTimingParameters(bool ntscMode_);
    void addFloppyCallback(int n);
    void removeFloppyCallback(int n);
    void resetACIA();
    M7501 * getDebugCPU();
    const M7501 * getDebugCPU() const;
    static PLUS4EMU_REGPARM1 void tapeCallback(void *userData);
    // run SID emulation at 10/9 * TED single clock frequency
    static PLUS4EMU_REGPARM1 void sidCallbackC64(void *userData);
    static PLUS4EMU_REGPARM1 void demoPlayCallback(void *userData);
    static PLUS4EMU_REGPARM1 void demoRecordCallback(void *userData);
    static PLUS4EMU_REGPARM1 void videoBreakPointCheckCallback(void *userData);
    static PLUS4EMU_REGPARM1 void lightPenCallback(void *userData);
    static PLUS4EMU_REGPARM1 void videoCaptureCallback(void *userData);
    static PLUS4EMU_REGPARM1 void aciaCallback(void *userData);
    inline void setEnableACIACallback(bool isEnabled)
    {
      if (isEnabled != aciaCallbackFlag) {
        ted->setCallback(&aciaCallback, this, (isEnabled ? 3 : 0));
        aciaCallbackFlag = isEnabled;
      }
    }
    static PLUS4EMU_REGPARM1 void pasteTextCallback(void *userData);
    void removePasteTextCallback();
    bool checkEditorMode() const;
    void setCursorPosition_(int xPos, int yPos);
   public:
    Plus4VM(Plus4Emu::VideoDisplay&, Plus4Emu::AudioOutput&);
    virtual ~Plus4VM();
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
        size_t memSize,
        uint64_t ramPattern = VirtualMachine::defaultRAMPattern);
    /*!
     * Load ROM segment 'n' from the specified file, skipping 'offs' bytes.
     */
    virtual void loadROMSegment(uint8_t n, const char *fileName, size_t offs);
    /*!
     * Set CPU clock frequency (in Hz, or clock multiplier if a small value
     * is specified); defaults to 1.
     */
    virtual void setCPUFrequency(size_t freq_);
    /*!
     * Set TED input clock frequency (defaults to 17734475 Hz).
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
    virtual void getVMStatus(VirtualMachine::VMStatus& vmStatus_);
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
     * name means no disk. If 'driveType' is 0, the 1541 will be emulated
     * for D64 files, while if it is 1 (for n < 2 only), the 1551 will be
     * emulated. For D81 files, this parameter is ignored.
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
    // ---------------------------- TAPE EMULATION ----------------------------
    /*!
     * Set tape image file name (if the file name is NULL or empty, tape
     * emulation is disabled).
     */
    virtual void setTapeFileName(const std::string& fileName);
    /*!
     * Send tape input/output signals to the sound output. The absolute value
     * of 'n' (in the range 0 to 10) controls the output level, negative
     * values invert the signal. If 'n' is zero, tape feedback is disabled.
     */
    virtual void setTapeFeedbackLevel(int n);
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
    // ------------------------------ DEBUGGING -------------------------------
    /*!
     * Set the debugging context (CPU number).
     *   0: main CPU
     *   1: floppy drive (unit 8)
     *   2: floppy drive (unit 9)
     *   3: floppy drive (unit 10)
     *   4: floppy drive (unit 11)
     *   5: printer
     */
    virtual void setDebugContext(int n);
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
    virtual void setBreakPoints(const Plus4Emu::BreakPointList& bpList);
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
     * Returns the current horizontal (0 to 455) and vertical (0 to 311)
     * video position.
     */
    virtual void getVideoPosition(int& xPos, int& yPos) const;
    /*!
     * Set the registers of the currently selected CPU; see cpu.hpp for more
     * information.
     */
    virtual void setCPURegisters(const M7501Registers& r);
    /*!
     * Get the registers of the currently selected CPU.
     */
    virtual void getCPURegisters(M7501Registers& r) const;
    // ------------------------------- FILE I/O -------------------------------
    /*!
     * Save snapshot of virtual machine state, including all ROM and RAM
     * segments, as well as all hardware registers. Note that the clock
     * frequency and timing settings, tape and disk state, and breakpoint list
     * are not saved.
     */
    virtual void saveState(Plus4Emu::File&);
    /*!
     * Save clock frequency and timing settings.
     */
    virtual void saveMachineConfiguration(Plus4Emu::File&);
    /*!
     * Save program.
     */
    virtual void saveProgram(Plus4Emu::File&);
    virtual void saveProgram(const char *fileName);
    /*!
     * Load program.
     */
    virtual void loadProgram(const char *fileName);
    /*!
     * Register all types of file data supported by this class, for use by
     * Plus4Emu::File::processAllChunks(). Note that loading snapshot data
     * will clear all breakpoints.
     */
    virtual void registerChunkTypes(Plus4Emu::File&);
    /*!
     * Start recording a demo to the file object, which will be used until
     * the recording is stopped for some reason.
     * Implies calling saveMachineConfiguration() and saveState() first.
     */
    virtual void recordDemo(Plus4Emu::File&);
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
    virtual void loadState(Plus4Emu::File::Buffer&);
    virtual void loadMachineConfiguration(Plus4Emu::File::Buffer&);
    virtual void loadDemo(Plus4Emu::File::Buffer&);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_PLUS4VM_HPP

