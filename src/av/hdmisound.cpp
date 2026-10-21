//
// hdmisound.cpp
//
// See hdmisound.h: Circle's HDMI sound driver (R. Stange, GPL 3), for
// either HDMI port of the Pi 4 or 5.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "hdmisound.h"

#include <circle/bcm2835.h>
#include <circle/bcmpropertytags.h>
#include <circle/logger.h>
#include <circle/machineinfo.h>
#include <circle/memio.h>
#include <circle/synchronize.h>
#include <circle/timer.h>
#include <circle/util.h>

static const char From[] = "sndhdmi";

namespace {

// Per port: core, PHY, packet RAM, MAI (in "hd"), DMA requests (the Pi
// 5's change with its D0 stepping).
struct PortAddresses {
  uintptr core, phy, ram, mai;
  unsigned dreq, dreq_d0;
};
#if RASPPI >= 5
const PortAddresses Ports[2] = {
    {ARM_IO_BASE + 0x701400, ARM_IO_BASE + 0x701D00, ARM_IO_BASE + 0x703800,
     ARM_IO_BASE + 0x720000 + 0x10, 10, 12},
    {ARM_IO_BASE + 0x706400, ARM_IO_BASE + 0x706D00, ARM_IO_BASE + 0x708800,
     ARM_IO_BASE + 0x720000 + 0x30, 17, 13},
};
const uintptr DvpMiscConfig = ARM_IO_BASE + 0x700008;

// Core registers (from the port's core base), as Circle has them for the
// Pi 5.
const uintptr MaiChannelMap = 0xA4;
const uintptr MaiConfig = 0xA8;
const uintptr AudioPacketConfig = 0xC0;
const uintptr RamPacketConfig = 0xC4;
const uintptr RamPacketStatus = 0xCC;
const uintptr CrpConfig = 0xD0;
const uintptr Cts0 = 0xD4, Cts1 = 0xD8;
const uintptr Hotplug = 0x1C8;
#else
const PortAddresses Ports[2] = {
    {ARM_IO_BASE + 0xF00700, ARM_IO_BASE + 0xF00F00, ARM_IO_BASE + 0xF01B00,
     ARM_IO_BASE + 0xF20000 + 0x10, 10, 10},
    {ARM_IO_BASE + 0xF05700, ARM_IO_BASE + 0xF05F00, ARM_IO_BASE + 0xF06B00,
     ARM_IO_BASE + 0xF20000 + 0x30, 17, 17},
};
const uintptr DvpMiscConfig = ARM_IO_BASE + 0xF00008;

// Core registers (from the port's core base), as Circle has them for the
// Pi 4.
const uintptr MaiChannelMap = 0x9C;
const uintptr MaiConfig = 0xA0;
const uintptr AudioPacketConfig = 0xB8;
const uintptr RamPacketConfig = 0xBC;
const uintptr RamPacketStatus = 0xC4;
const uintptr CrpConfig = 0xC8;
const uintptr Cts0 = 0xCC, Cts1 = 0xD0;
const uintptr Hotplug = 0x1A8;
#endif

// The Pi 5's D0 stepping: other DMA requests, and the MAI threshold's
// fields moved.
bool SteppingD0(void) {
#if RASPPI >= 5
  return CMachineInfo::Get()->GetSoCStepping() >= SoCSteppingD0;
#else
  return false;
#endif
}
// MAI registers (from the port's MAI base).
const uintptr MaiControl = 0x00;
const uintptr MaiThreshold = 0x04;
const uintptr MaiFormat = 0x08;
const uintptr MaiData = 0x0C;
const uintptr MaiSampleRate = 0x10;
// PHY.
const uintptr PhyPowerDown = 0x04;
const u32 PhyRngPowerDown = 1u << 4;
// Packet RAM: the audio InfoFrame, slot 4.
const uintptr RamPacketAudio = 4 * 0x24;
const u32 AudioPacket = 1u << 4;
const u32 RamPacketEnable = 1u << 16;

// MAI_CTL
const u32 MaiReset = 1u << 0, MaiErrorFull = 1u << 1, MaiErrorEmpty = 1u << 2,
          MaiEnable = 1u << 3, MaiFlush = 1u << 9, MaiWholeSample = 1u << 12,
          MaiChannelAlign = 1u << 13, MaiDelayed = 1u << 15;
const unsigned MaiChannelNumberShift = 4;

const unsigned long AudioClockRate = 108000000;

// The best fraction NUM/DEN for VALUE / DIVISOR with NUM < MAX_NUM and
// DEN <= MAX_DEN, by continued fractions: each step takes the integer part
// and inverts the rest; the last convergent within the limits wins.
void BestFraction(unsigned long value, unsigned long divisor,
                  unsigned long max_num, unsigned long max_den,
                  unsigned long *num, unsigned long *den) {
  unsigned long n0 = 0, d0 = 1, n1 = 1, d1 = 0;   // convergents k-2, k-1
  unsigned long a = value, b = divisor;
  while (b != 0) {
    unsigned long q = a / b;
    unsigned long n2 = q * n1 + n0, d2 = q * d1 + d0;
    if (n2 > max_num || d2 > max_den) {
      break;
    }
    n0 = n1; d0 = d1; n1 = n2; d1 = d2;
    unsigned long r = a % b;
    a = b;
    b = r;
  }
  if (d1 == 0) {    // even the first step didn't fit
    n1 = max_num;
    d1 = 1;
  }
  *num = n1;
  *den = d1;
}

u32 SampleRateFormat(unsigned rate) {
  static const unsigned rates[] = {8000,  11025, 12000, 16000,  22050,
                                   24000, 32000, 44100, 48000,  64000,
                                   88200, 96000, 128000, 176400, 192000};
  for (unsigned i = 0; i < sizeof rates / sizeof rates[0]; i++) {
    if (rates[i] == rate) {
      return 1 + i;
    }
  }
  return 0;
}

} // namespace

Pi4HdmiSound::Pi4HdmiSound(CInterruptSystem *pInterrupt, unsigned port,
                           unsigned nSampleRate, unsigned nChunkSize)
    : CSoundBaseDevice(SoundFormatIEC958, 0, nSampleRate),
      m_nPort(port > 1 ? 0 : port), m_nSampleRate(nSampleRate),
      m_nChunkSize(nChunkSize), m_ulPixelClockRate(0), m_State(Created),
      m_pDMAChannel(new CDMAChannel(DMA_CHANNEL_LITE, pInterrupt)) {
  m_Core = Ports[m_nPort].core;
  m_Phy = Ports[m_nPort].phy;
  m_Ram = Ports[m_nPort].ram;
  m_Mai = Ports[m_nPort].mai;
  m_pDMABuffer[0] = new u32[m_nChunkSize];
  m_pDMABuffer[1] = new u32[m_nChunkSize];
}

Pi4HdmiSound::~Pi4HdmiSound(void) {
  ResetHdmi();
  delete m_pDMAChannel;
  delete[] m_pDMABuffer[0];
  delete[] m_pDMABuffer[1];
}

int Pi4HdmiSound::ConnectedPort(void) {
  for (unsigned port = 0; port < 2; port++) {
    if (read32(Ports[port].core + Hotplug) & 1) {
      return (int)port;
    }
  }
  return -1;
}

boolean Pi4HdmiSound::Start(void) {
  if (m_State > Idle) {
    return FALSE;
  }
  if (m_State == Created) {
    if (!(read32(m_Core + RamPacketConfig) & RamPacketEnable)) {
      CLogger::Get()->Write(From, LogWarning, "HDMI%u: the display takes no sound", m_nPort);
      m_State = Error;
      return FALSE;
    }
    if (m_nPort == 1) {
      // HDMI1's audio clock (bit set = gated off).
      write32(DvpMiscConfig, read32(DvpMiscConfig) & ~(1u << 4));
    }
    // As Circle on the Pi 4: the pixel clock for CTS is the BVB clock.
    m_ulPixelClockRate = CMachineInfo::Get()->GetClockRate(CLOCK_ID_PIXEL_BVB);
    if (m_ulPixelClockRate == 0) {
      m_State = Error;
      return FALSE;
    }
    RunHdmi();
    m_State = Idle;
  }
  m_State = Running;

  const size_t length = m_nChunkSize * sizeof(u32);
  memset(m_pDMABuffer[0], 0, length);
  memset(m_pDMABuffer[1], 0, length);
  const void *buffers[2] = {m_pDMABuffer[0], m_pDMABuffer[1]};
  m_pDMAChannel->SetupCyclicIOWrite(m_Mai + MaiData, buffers, 2, length,
                                    (TDREQ)(SteppingD0() ? Ports[m_nPort].dreq_d0
                                                         : Ports[m_nPort].dreq));
  m_pDMAChannel->SetCompletionRoutine(DmaCompletionStub, this);
  m_pDMAChannel->Start();

#if RASPPI == 4
  write32(m_Phy + PhyPowerDown, read32(m_Phy + PhyPowerDown) & ~PhyRngPowerDown);
#endif
  write32(m_Mai + MaiControl, IEC958_HW_CHANNELS << MaiChannelNumberShift |
                                  MaiWholeSample | MaiChannelAlign | MaiEnable);
  return TRUE;
}

void Pi4HdmiSound::Cancel(void) {
  m_SpinLock.Acquire();
  if (m_State == Running) {
    m_State = Cancelled;
    m_SpinLock.Release();
    m_pDMAChannel->Cancel();
    StopHdmi();
    m_SpinLock.Acquire();
    m_State = Idle;
  }
  m_SpinLock.Release();
}

boolean Pi4HdmiSound::IsActive(void) const {
  return m_State == Running || m_State == Cancelled;
}

void Pi4HdmiSound::RunHdmi(void) {
  // MAI sample rate: audio clock / sample rate as N / M (N 24 bits, M up
  // to 256, stored less one).
  unsigned long num, den;
  BestFraction(AudioClockRate, m_nSampleRate, 0xFFFFFF, 256, &num, &den);

  // Audio clock regeneration: N = 128 fs / 1000, CTS = f_pixel N / 128 fs.
  u32 n = m_nSampleRate * 128 / 1000;
  u32 cts = (u32)(((u64)m_ulPixelClockRate * n) / (m_nSampleRate * 128));

  write32(m_Mai + MaiControl, MaiReset | MaiFlush | MaiDelayed | MaiErrorEmpty | MaiErrorFull);
  write32(m_Mai + MaiSampleRate, (u32)num << 8 | (u32)(den - 1));
  write32(m_Mai + MaiFormat, SampleRateFormat(m_nSampleRate) << 8 | 2 << 16);  // PCM
  if (SteppingD0()) {
    write32(m_Mai + MaiThreshold, 0x10 << 23 | 0x10 << 15 | 0x1C << 7 | 0x1C);
  } else {
    write32(m_Mai + MaiThreshold, 16 << 24 | 16 << 16 | 16 << 8 | 16);
  }
  write32(m_Core + MaiConfig, 1u << 26 | 1u << 27 | 0b11);  // bit/format reverse, 2 channels
  write32(m_Core + MaiChannelMap, 0b00010000);
  write32(m_Core + AudioPacketConfig, 1u << 29 | 1u << 24 |
                                          IEC958_B_FRAME_PREAMBLE << 10 | 0b11);
  write32(m_Core + CrpConfig, 1u << 24 | n);   // external CTS
  write32(m_Core + Cts0, cts);
  write32(m_Core + Cts1, cts);

  if (!SetAudioInfoFrame()) {
    CLogger::Get()->Write(From, LogError, "Cannot set the audio InfoFrame");
  }
}

void Pi4HdmiSound::StopHdmi(void) {
  write32(m_Mai + MaiControl, MaiDelayed | MaiErrorEmpty | MaiErrorFull);
#if RASPPI == 4
  write32(m_Phy + PhyPowerDown, read32(m_Phy + PhyPowerDown) | PhyRngPowerDown);
#endif
}

void Pi4HdmiSound::ResetHdmi(void) {
  write32(m_Core + RamPacketConfig, read32(m_Core + RamPacketConfig) & ~AudioPacket);
  WaitForBit(m_Core + RamPacketStatus, AudioPacket, FALSE, 100);
  write32(m_Mai + MaiControl, MaiReset);
  write32(m_Mai + MaiControl, MaiErrorFull);
  write32(m_Mai + MaiControl, MaiFlush);
}

// The audio InfoFrame: type 0x84, version 1, 10 bytes; 2 channels; the
// rest "refer to stream header" (as Circle writes it).
boolean Pi4HdmiSound::SetAudioInfoFrame(void) {
  write32(m_Core + RamPacketConfig, read32(m_Core + RamPacketConfig) & ~AudioPacket);
  if (!WaitForBit(m_Core + RamPacketStatus, AudioPacket, FALSE, 100)) {
    return FALSE;
  }
  uintptr slot = m_Ram + RamPacketAudio;
  write32(slot + 0, 0x0A0184);   // type, version, length
  write32(slot + 4, 0x0170);     // checksum, channels - 1
  for (uintptr reg = slot + 8; reg < slot + 0x24; reg += 4) {
    write32(reg, 0);
  }
  write32(m_Core + RamPacketConfig, read32(m_Core + RamPacketConfig) | AudioPacket);
  return WaitForBit(m_Core + RamPacketStatus, AudioPacket, TRUE, 100);
}

void Pi4HdmiSound::DmaCompletion(unsigned nBuffer, boolean bStatus) {
  m_SpinLock.Acquire();
  if (m_State != Running) {
    m_SpinLock.Release();
    return;
  }
  if (!bStatus) {
    m_State = Error;
    m_SpinLock.Release();
    m_pDMAChannel->Cancel();
    StopHdmi();
    return;
  }
  if (GetChunk(m_pDMABuffer[nBuffer], m_nChunkSize) < m_nChunkSize) {
    m_pDMAChannel->Cancel();
    StopHdmi();
    m_State = Idle;
  }
  m_SpinLock.Release();
}

void Pi4HdmiSound::DmaCompletionStub(unsigned nChannel, unsigned nBuffer,
                                     boolean bStatus, void *pParam) {
  (void)nChannel;
  ((Pi4HdmiSound *)pParam)->DmaCompletion(nBuffer, bStatus);
}

boolean Pi4HdmiSound::WaitForBit(uintptr nAddress, u32 nMask, boolean bSet,
                                 unsigned nMsTimeout) {
  while (((read32(nAddress) & nMask) != 0) != (bSet != FALSE)) {
    CTimer::SimpleMsDelay(1);
    if (--nMsTimeout == 0) {
      return FALSE;
    }
  }
  return TRUE;
}
