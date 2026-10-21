//
// screen.cpp
//
// See screen.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "screen.h"

#include <circle/bcm2835.h>
#include <circle/logger.h>
#include <circle/memory.h>
#include <circle/sched/scheduler.h>
#include <circle/synchronize.h>
#include <circle/timer.h>
#include <string.h>

static const char FromScreen[] = "screen";

// A frame takes at most 1/24 s; more than this and the display has gone.
static const unsigned FrameTimeoutMs = 100;

Pi45Screen *Pi45Screen::s_pThis = 0;

Pi45Screen::Pi45Screen(void)
    : m_pFrameBuffer(0), m_bAttached(false), m_bPool(false), m_pPool(0),
      m_nPoolBytes(0), m_nPoolUsed(0), m_nPoolBuffers(0), m_nPoolAddress(0),
      m_nPlanes(0), m_bFlipPending(false), m_nFlipCount(0), m_bTimedOut(false),
      m_nFirmwareWidth(0), m_nFirmwareHeight(0) {
  s_pThis = this;
}

Pi45Screen::~Pi45Screen(void) {
  if (m_bAttached) {
    m_Hvs.Restore();
  }
  delete m_pFrameBuffer;
  s_pThis = 0;
}

bool Pi45Screen::Initialize(void) {
  CLogger *log = CLogger::Get();

  // The firmware's framebuffer at the display's size, two pages: what the
  // firmware shows until the HVS is taken over, and on the Pi 5 perhaps
  // the only memory its HVS reads.
  unsigned width = 1280, height = 720;
  CBcmFrameBuffer *probe = new CBcmFrameBuffer(0, 0, 32, 0, 0, 0, TRUE);
  if (probe != 0) {
    // CBcmFrameBuffer asks the firmware for the display's size when given 0.
    if (probe->GetWidth() >= 320 && probe->GetHeight() >= 200) {
      width = probe->GetWidth();
      height = probe->GetHeight();
    }
    delete probe;
  }
  m_pFrameBuffer = new CBcmFrameBuffer(width, height, 32, width, 2 * height, 0, TRUE);
  if (m_pFrameBuffer == 0 || !m_pFrameBuffer->Initialize()) {
    delete m_pFrameBuffer;
    m_pFrameBuffer = new CBcmFrameBuffer(width, height, 32, 0, 0, 0, TRUE);
    if (m_pFrameBuffer == 0 || !m_pFrameBuffer->Initialize()) {
      log->Write(FromScreen, LogError, "Cannot allocate a framebuffer");
      delete m_pFrameBuffer;
      m_pFrameBuffer = 0;
    }
  }

  if (!m_Hvs.Attach()) {
    log->Write(FromScreen, LogError, "Neither HDMI port is fed by the HVS");
    return false;
  }

#if RASPPI >= 5
  // Where the HVS reads the firmware's framebuffer tells where it reads
  // memory: at the ARM's address, or else only through the framebuffer.
  // The firmware's plane is only taken to be the framebuffer if its pitch
  // matches.
  if (m_pFrameBuffer != 0) {
    uint64_t fw = m_Hvs.FirmwareAddress();
    uint64_t arm = (uint64_t)m_pFrameBuffer->GetBuffer();
    bool known = fw != 0 && arm != 0 && m_Hvs.FirmwarePitch() == m_pFrameBuffer->GetPitch();
    m_bPool = known && fw != arm;
    m_nPoolAddress = fw;
    m_pPool = (uint8_t *)(uintptr)m_pFrameBuffer->GetBuffer();
    m_nPoolBytes = m_pFrameBuffer->GetSize();
  }
  log->Write(FromScreen, LogNotice, "HDMI%u, HVS %c channel %u, %ux%u, pixels in %s",
             m_Hvs.Port(), m_Hvs.SteppingD() ? 'D' : 'C', m_Hvs.Channel(), m_Hvs.Width(),
             m_Hvs.Height(), m_bPool ? "the framebuffer" : "memory");
#else
  log->Write(FromScreen, LogNotice, "HDMI%u, HVS channel %u, %ux%u", m_Hvs.Port(),
             m_Hvs.Channel(), m_Hvs.Width(), m_Hvs.Height());
#endif
  m_ModeSetter.SetPort(m_Hvs.Port());
  m_bAttached = true;
  m_Hvs.Show(0, 0);
  return true;
}

uint32_t *Pi45Screen::Allocate(size_t bytes) {
  bytes = (bytes + 4095) & ~(size_t)4095;
  uint32_t *buffer;
  if (m_bPool) {
    if (m_nPoolUsed + bytes > m_nPoolBytes) {
      return 0;
    }
    buffer = (uint32_t *)(m_pPool + m_nPoolUsed);
    m_nPoolUsed += bytes;
    m_nPoolBuffers++;
  } else {
    // 4 KB aligned: whole cache lines, and pages for the GPU.
    void *block = CMemorySystem::HeapAllocate(bytes + 4096, HEAP_DMA30);
    if (block == 0) {
      return 0;
    }
    uintptr aligned = ((uintptr)block + 4096) & ~(uintptr)4095;
    // The block's start, kept just before the buffer to free it.
    ((void **)aligned)[-1] = block;
    buffer = (uint32_t *)aligned;
  }
  memset(buffer, 0, bytes);
  CleanAndInvalidateDataCacheRange((uintptr)buffer, bytes);
  return buffer;
}

void Pi45Screen::Free(uint32_t *buffer) {
  if (buffer == 0) {
    return;
  }
  if (m_bPool) {
    if (m_nPoolBuffers > 0 && --m_nPoolBuffers == 0) {
      m_nPoolUsed = 0;
    }
    return;
  }
  CMemorySystem::HeapFree(((void **)buffer)[-1]);
}

uint64_t Pi45Screen::HvsAddress(const void *p) const {
  if (m_bPool) {
    return m_nPoolAddress + (uint64_t)((const uint8_t *)p - m_pPool);
  }
#if RASPPI >= 5
  return (uint64_t)(uintptr)p;
#else
  return (uint32_t)BUS_ADDRESS((uintptr)p);
#endif
}

bool Pi45Screen::Present(const kms::Plane *planes, unsigned count, bool wait) {
  if (!m_bAttached) {
    return false;
  }
  if (count > kms::Hvs::MaxPlanes) {
    count = kms::Hvs::MaxPlanes;
  }
  unsigned now = m_Hvs.FrameCount();
  if (!m_Hvs.Show(planes, count)) {
    // Once for each number of planes, with what they were.
    static unsigned logged;
    if (!(logged & (1u << count))) {
      logged |= 1u << count;
      for (unsigned i = 0; i < count; i++) {
        CLogger::Get()->Write(FromScreen, LogWarning,
                              "Can't show %u plane(s): %u: %ux%u to %ux%u at %u,%u, pitch %u%s",
                              count, i, planes[i].src_width, planes[i].src_height,
                              planes[i].width, planes[i].height, planes[i].x, planes[i].y,
                              planes[i].pitch, planes[i].alpha ? ", alpha" : "");
      }
    }
    return false;
  }
  memcpy(m_Planes, planes, count * sizeof planes[0]);
  m_nPlanes = count;
  // The frame being composed now still shows the old list; from the next
  // one on, the new. So the old buffers are free once the count moves on.
  m_bFlipPending = true;
  m_nFlipCount = now;
  if (wait) {
    WaitForFlip();
  }
  return true;
}

void Pi45Screen::WaitForFlip(void) {
  if (!m_bFlipPending) {
    return;
  }
  if (m_Hvs.FrameCount() == m_nFlipCount && !WaitForChange(m_nFlipCount) && !m_bTimedOut) {
    m_bTimedOut = true;
    CLogger::Get()->Write(FromScreen, LogWarning, "The HVS has stopped (display off?)");
  }
  m_bFlipPending = false;
}

// Circle's tasks (the network, Wi-Fi, the web UI) run when the emulator's
// core yields, so the wait for the next frame runs them, as the Pi 3's wait
// in vc_dispmanx_update_submit_sync() does.
bool Pi45Screen::WaitForChange(unsigned from) {
  u64 start = CTimer::GetClockTicks64();
  u64 limit = (u64)FrameTimeoutMs * (CLOCKHZ / 1000);
  while (m_Hvs.FrameCount() == from) {
    if (CTimer::GetClockTicks64() - start > limit) {
      return false;
    }
    CScheduler::Get()->Yield();
  }
  return true;
}

bool Pi45Screen::MatchRate(double fps, unsigned audio_fs, unsigned lines) {
  if (!m_bAttached) {
    return false;
  }
  kms::Timing current, wanted;
  if (!m_ModeSetter.Current(&current)) {
    return false;
  }
  if (m_nFirmwareWidth == 0) {
    m_nFirmwareWidth = current.h_active;
    m_nFirmwareHeight = current.v_active;
  }
  // The size asked for (ChooseMode() takes the size from its first
  // argument).
  kms::Timing size = current;
  size.h_active = lines == 1080 ? 1920 : lines == 720 ? 1280 : m_nFirmwareWidth;
  size.v_active = lines == 1080 ? 1080 : lines == 720 ? 720 : m_nFirmwareHeight;
  bool resize = size.h_active != current.h_active || size.v_active != current.v_active;
  if (resize && !kms::ChooseMode(size, fps, m_ModeSetter.GetEdid(), &wanted)) {
    CLogger::Get()->Write(FromScreen, LogNotice,
                          "No %ux%u mode at %.2f Hz the display lists; keeping %ux%u",
                          size.h_active, size.v_active, fps, current.h_active,
                          current.v_active);
    resize = false;
  }
  if (resize) {
    // The layers are laid out for the old size: black until they are
    // shown again.
    m_nPlanes = 0;
    m_ModeSetter.Set(m_Hvs, wanted, audio_fs, Reinstall, this);
    return true;
  }
  if (!kms::ChooseMode(current, fps, m_ModeSetter.GetEdid(), &wanted)) {
    CLogger::Get()->Write(FromScreen, LogNotice,
                          "No %ux%u mode at %.2f Hz the display lists; keeping %.2f Hz",
                          current.h_active, current.v_active, fps, current.Refresh());
    return false;
  }
  if (wanted.vic == current.vic &&
      (wanted.pixel_hz > current.pixel_hz ? wanted.pixel_hz - current.pixel_hz
                                          : current.pixel_hz - wanted.pixel_hz) < 1000) {
    return false;
  }
  m_ModeSetter.Set(m_Hvs, wanted, audio_fs, Reinstall, this);
  return true;
}

void Pi45Screen::Reinstall(void *param) {
  Pi45Screen *screen = (Pi45Screen *)param;
  // The buffers last shown are the front ones.
  screen->m_Hvs.Show(screen->m_Planes, screen->m_nPlanes);
  screen->m_bFlipPending = false;
}

void Pi45Screen::Blank(void) {
  if (m_bAttached) {
    m_Hvs.Show(0, 0);
    m_nPlanes = 0;
  }
}
