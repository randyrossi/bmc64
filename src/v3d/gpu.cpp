//
// gpu.cpp
//
// See gpu.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "gpu.h"

#include "cl.h"
#include "regs.h"

#include <circle/bcmpropertytags.h>
#include <circle/logger.h>
#include <circle/memio.h>
#include <circle/memory.h>
#include <circle/synchronize.h>
#include <circle/timer.h>
#include <string.h>

namespace v3d {

static const char FromGpu[] = "v3d";

namespace {

// Waits until DONE() is true, at most TIMEOUT_US.
template <typename Done> bool WaitUntil(Done done, unsigned timeout_us) {
  u64 start = CTimer::GetClockTicks64();
  u64 limit = (u64)timeout_us * (CLOCKHZ / 1000000);
  while (!done()) {
    if (CTimer::GetClockTicks64() - start > limit) {
      return done();
    }
  }
  return true;
}

// Waits until (read32(ADDRESS) & MASK) is 0, at most TIMEOUT_US.
bool WaitClear(uintptr_t address, uint32_t mask, unsigned timeout_us) {
  return WaitUntil([=] { return (read32(address) & mask) == 0; }, timeout_us);
}

// Sets the V3D clock to its highest rate through the firmware; the rate
// set, or 0.
uint32_t SetClock(void) {
  CBcmPropertyTags tags;
  TPropertyTagClockRate max;
  max.nClockId = ClockV3d;
  if (!tags.GetTag(PROPTAG_GET_MAX_CLOCK_RATE, &max, sizeof max, 4) || max.nRate == 0) {
    return 0;
  }
  TPropertyTagSetClockRate set;
  set.nClockId = ClockV3d;
  set.nRate = max.nRate;
  set.nSkipSettingTurbo = SKIP_SETTING_TURBO;
  if (!tags.GetTag(PROPTAG_SET_CLOCK_RATE, &set, sizeof set, 12)) {
    return 0;
  }
  return set.nRate;
}

// The first 1 GB of memory is mapped one to one; HEAP_DMA30 memory is in
// it. The table covers the whole 4 GB the GPU can address.
const unsigned MappedPages = Gpu::MappedBytes >> PageShift;
const unsigned TablePages = (1ull << 32) >> PageShift;

} // namespace

Gpu::Gpu(void)
    : m_bReady(false), m_nRevision(0), m_nBinUs(0), m_nRenderUs(0), m_pPageTable(0),
      m_pScratch(0) {}

uint32_t Gpu::Hub(uint32_t reg) const { return read32(HubBase + reg); }
void Gpu::SetHub(uint32_t reg, uint32_t value) const { write32(HubBase + reg, value); }
uint32_t Gpu::Core(uint32_t reg) const { return read32(CoreBase + reg); }
void Gpu::SetCore(uint32_t reg, uint32_t value) const { write32(CoreBase + reg, value); }

void *Gpu::Allocate(size_t bytes, size_t align) {
  uint8_t *p = (uint8_t *)CMemorySystem::HeapAllocate(bytes + align, HEAP_DMA30);
  if (p == 0) {
    return 0;
  }
  p = (uint8_t *)(((uintptr_t)p + align - 1) & ~(uintptr_t)(align - 1));
  memset(p, 0, bytes);
  CleanAndInvalidateDataCacheRange((uintptr_t)p, bytes);
  return p;
}

#if RASPPI >= 5
// The firmware turns the V3D clock on and sets it. The GRAFX power domain
// is powered up (unless the firmware has), the V3D's reset released, and
// its SMS taken out of power off and reset, as Linux does on the BCM2712.
// The GPU isn't read before then.
bool Gpu::PowerUp(void) {
  CLogger *log = CLogger::Get();
  struct {
    TPropertyTag tag;
    u32 clock;
    u32 state;
  } PACKED clock_on;
  clock_on.clock = ClockV3d;
  clock_on.state = 1;
  CBcmPropertyTags tags;
  bool on = tags.GetTag(TagSetClockState, &clock_on, sizeof clock_on, 8);
  uint32_t rate = SetClock();
  uintptr_t grafx = PmBase + PmGrafx;
  uintptr_t reset = PmBase + PmGrafx2712;
  log->Write(FromGpu, LogNotice, "Clock %s, %u Hz, GRAFX %08X, V3D reset %08X",
             on ? "on" : "not answered", rate, read32(grafx), read32(reset));

  if ((read32(grafx) & PmPowUp) == 0) {
    // Power up, raising the inrush current allowed until power is good.
    bool good = false;
    for (uint32_t inrush = 0; inrush <= 3 && !good; inrush++) {
      write32(grafx, PmPassword | (read32(grafx) & 0x00FFFFFF & ~PmInrushMask) |
                         (inrush << PmInrushShift) | PmPowUp);
      good = WaitUntil([=] { return (read32(grafx) & PmPowOk) != 0; }, 5);
    }
    if (!good) {
      log->Write(FromGpu, LogWarning, "GRAFX power not good: %08X", read32(grafx));
      return false;
    }
    write32(grafx, PmPassword | (read32(grafx) & 0x00FFFFFF) | PmIsPow);
    write32(grafx, PmPassword | (read32(grafx) & 0x00FFFFFF) | PmMemRep);
    if (!WaitUntil([=] { return (read32(grafx) & PmMrDone) != 0; }, 1000)) {
      log->Write(FromGpu, LogWarning, "GRAFX memory repair timed out");
      return false;
    }
    write32(grafx, PmPassword | (read32(grafx) & 0x00FFFFFF) | PmIsFunc);
    log->Write(FromGpu, LogNotice, "GRAFX powered up: %08X", read32(grafx));
  }

  CTimer::SimpleusDelay(1);
  write32(reset, PmPassword | (read32(reset) & 0x00FFFFFF) | PmV3dResetN);
  CTimer::SimpleusDelay(1);

  write32(SmsBase + SmsTeeCs, SmsClearPowerOff);
  if (!WaitUntil([] { return (read32(SmsBase + SmsTeeCs) & SmsStateMask) == SmsIdle; },
                 100000)) {
    log->Write(FromGpu, LogWarning, "SMS didn't power up: %08X", read32(SmsBase + SmsTeeCs));
    return false;
  }
  write32(SmsBase + SmsReeCs, SmsReset);
  if (!WaitUntil(
          [] {
            uint32_t state = read32(SmsBase + SmsReeCs) & SmsStateMask;
            return state != SmsIsolatingForReset && state != SmsResetting;
          },
          100000)) {
    log->Write(FromGpu, LogWarning, "SMS reset timed out: %08X", read32(SmsBase + SmsReeCs));
    return false;
  }
  return Present(true);
}
#else
// The firmware turns on the V3D power domain and its clock; if the GPU
// still doesn't answer, its reset is released and its bus bridges opened,
// as Linux does on the BCM2711.
bool Gpu::PowerUp(void) {
  CLogger *log = CLogger::Get();
  CBcmPropertyTags tags;
  TPropertyTagPowerState domain;
  domain.nDeviceId = DomainV3d;
  domain.nState = POWER_STATE_ON;
  bool on = tags.GetTag(TagSetDomainState, &domain, sizeof domain, 8);
  uint32_t rate = SetClock();
  log->Write(FromGpu, LogNotice, "Power domain %s, clock %u Hz", on ? "on" : "not answered",
             rate);
  CTimer::SimpleMsDelay(1);
  if (Present(false)) {
    return true;
  }

  // On the BCM2711 the firmware leaves the GPU in reset; this is expected.
  log->Write(FromGpu, LogNotice, "Releasing the reset and opening the bus bridges");
  uintptr_t grafx = PmBase + PmGrafx;
  write32(grafx, PmPassword | (read32(grafx) & 0x00FFFFFF) | PmV3dResetN);
  CTimer::SimpleusDelay(1);
  const uint32_t bridges[] = {AsbMasterCtrl, AsbSlaveCtrl};
  for (uint32_t reg : bridges) {
    uintptr_t ctrl = AsbBase + reg;
    write32(ctrl, PmPassword | (read32(ctrl) & 0x00FFFFFF & ~AsbRequestStop));
    if (!WaitClear(ctrl, AsbAck, 1000)) {
      log->Write(FromGpu, LogWarning, "Bus bridge %02X didn't open", reg);
    }
  }
  return Present(true);
}
#endif

#if RASPPI >= 5
// HUB_IDENT1 gives the version, 7.1 on the BCM2712, and HUB_IDENT3 the
// revision; the core's IDENT1 its VPM size, which must be at least the
// 16 KB the shaders were compiled for (tools/v3dshader).
bool Gpu::Present(bool log_failure) {
  uint32_t ident1 = Hub(HubIdent1);
  unsigned tech = ident1 & 0xF, version = (ident1 >> 4) & 0xF;
  m_nRevision = (Hub(HubIdent3) >> 8) & 0xFF;
  unsigned vpm = (Core(CtlIdent1) >> 28) * 8192;
  bool found = tech == 7 && version == 1 && vpm >= 16384;
  if (!found && !log_failure) {
    return false;
  }
  CLogger::Get()->Write(FromGpu, LogNotice,
                        "IDENT0 %08X IDENT1 %08X IDENT2 %08X IDENT3 %08X: V3D %u.%u.%u, "
                        "%u core(s), VPM %u bytes",
                        Hub(HubIdent0), ident1, Hub(HubIdent2), Hub(HubIdent3), tech, version,
                        m_nRevision, (ident1 >> 8) & 0xF, vpm);
  return found;
}
#else
// HUB_IDENT1 gives the version: 4.2 on the BCM2711. LOG_FAILURE: log what
// was read even if it isn't a V3D 4.2.
bool Gpu::Present(bool log_failure) {
  uint32_t ident1 = Hub(HubIdent1);
  unsigned tech = ident1 & 0xF, rev = (ident1 >> 4) & 0xF;
  bool found = tech == 4 && rev == 2;
  if (!found && !log_failure) {
    return false;
  }
  CLogger::Get()->Write(FromGpu, LogNotice, "IDENT0 %08X IDENT1 %08X IDENT2 %08X: V3D %u.%u, %u core(s)",
                        Hub(HubIdent0), ident1, Hub(HubIdent2), tech, rev,
                        (ident1 >> 8) & 0xF);
  return found;
}
#endif

bool Gpu::SetUpMmu(void) {
  m_pPageTable = (uint32_t *)Allocate(TablePages * 4, 4096);
  m_pScratch = (uint8_t *)Allocate(4096, 4096);
  if (m_pPageTable == 0 || m_pScratch == 0) {
    CLogger::Get()->Write(FromGpu, LogError, "Not enough memory for the MMU");
    return false;
  }
  for (unsigned i = 0; i < MappedPages; i++) {
    m_pPageTable[i] = PteValid | PteWriteable | i;
  }
  CleanAndInvalidateDataCacheRange((uintptr_t)m_pPageTable, TablePages * 4);

  SetHub(MmuPtPaBase, Address(m_pPageTable) >> PageShift);
  SetHub(MmuCtl, MmuEnable | MmuPtInvalidEnable | MmuPtInvalidAbort | MmuPtInvalidInt |
                     MmuWriteViolationAbort | MmuWriteViolationInt |
                     MmuCapExceededAbort | MmuCapExceededInt);
  SetHub(MmuIllegalAddr, (Address(m_pScratch) >> PageShift) | MmuIllegalAddrEnable);
  SetHub(MmucControl, MmucEnable);

  SetHub(MmucControl, MmucFlush | MmucEnable);
  if (!WaitClear(HubBase + MmucControl, MmucFlushing, 100000)) {
    CLogger::Get()->Write(FromGpu, LogError, "MMU cache flush timed out");
    return false;
  }
  SetHub(MmuCtl, Hub(MmuCtl) | MmuTlbClear);
  if (!WaitClear(HubBase + MmuCtl, MmuTlbClearing, 100000)) {
    CLogger::Get()->Write(FromGpu, LogError, "MMU TLB clear timed out");
    return false;
  }
  return true;
}

bool Gpu::Initialize(void) {
  if (!PowerUp()) {
    CLogger::Get()->Write(FromGpu, LogWarning, "The GPU didn't start");
    return false;
  }
  // Texture cache flushes cover the whole cache.
  SetCore(CtlL2tFlushStart, 0);
  SetCore(CtlL2tFlushEnd, ~0u);
  if (!SetUpMmu()) {
    return false;
  }
  m_bReady = true;
  return true;
}

void Gpu::InvalidateCaches(void) {
  SetCore(CtlL2tcActl, L2tFlush);
  SetCore(CtlSlcActl, 0x0F0F0F0F);
}

void Gpu::LogState(const char *when) {
  CLogger::Get()->Write(FromGpu, LogWarning,
                        "%s: CT0CS %08X CT0CA %08X BFC %u CT1CS %08X CT1CA %08X RFC %u "
                        "ERR_STAT %08X MMU_CTL %08X VIO_ADDR %08X VIO_ID %08X",
                        when, Core(CleCt0Cs), Core(CleCt0Ca), Core(CleBfc), Core(CleCt1Cs),
                        Core(CleCt1Ca), Core(CleRfc), Core(ErrStat), Hub(MmuCtl),
                        Hub(MmuVioAddr), Hub(MmuVioId));
}

// Waits for the frame count in COUNTER to move on from BEFORE.
bool Gpu::Wait(uint32_t counter, uint32_t before, unsigned timeout_ms, const char *what) {
  u64 begin = CTimer::GetClockTicks64();
  u64 limit = (u64)timeout_ms * (CLOCKHZ / 1000);
  while (Core(counter) == before) {
    if (CTimer::GetClockTicks64() - begin > limit) {
      LogState(what);
      return false;
    }
  }
  return true;
}

bool Gpu::Bin(uint32_t start, uint32_t end, uint32_t tile_alloc, uint32_t size,
              uint32_t tile_state, unsigned timeout_ms) {
  if (!m_bReady) {
    return false;
  }
  SetCore(PtbBpos, 0);
  InvalidateCaches();
  uint32_t done = Core(CleBfc);
  SetCore(CleCt0Qma, tile_alloc);
  SetCore(CleCt0Qms, size);
  SetCore(CleCt0Qts, CleCt0QtsEnable | tile_state);
  u64 begin = CTimer::GetClockTicks64();
  SetCore(CleCt0Qba, start);
  SetCore(CleCt0Qea, end);
  bool ok = Wait(CleBfc, done, timeout_ms, "Binning timed out");
  m_nBinUs = (unsigned)((CTimer::GetClockTicks64() - begin) * 1000000 / CLOCKHZ);
  return ok;
}

bool Gpu::Render(uint32_t start, uint32_t end, unsigned timeout_ms) {
  if (!m_bReady) {
    return false;
  }
  InvalidateCaches();
  uint32_t done = Core(CleRfc);
  u64 begin = CTimer::GetClockTicks64();
  SetCore(CleCt1Qba, start);
  SetCore(CleCt1Qea, end);
  bool ok = Wait(CleRfc, done, timeout_ms, "Rendering timed out");
  m_nRenderUs = (unsigned)((CTimer::GetClockTicks64() - begin) * 1000000 / CLOCKHZ);
  return ok;
}

// A 256x256 picture in 64x64 tiles, cleared by the tile buffer and stored to
// memory; no binning, so no drawing.
bool Gpu::ClearTest(void) {
  const unsigned Width = 256, Height = 256, Tile = 64;
  const unsigned Stride = Width * 4;
  const uint32_t Colour = 0xFF336699;
  const unsigned TilesX = Width / Tile, TilesY = Height / Tile;

  uint32_t *picture = (uint32_t *)Allocate(Stride * Height, 4096);
  uint8_t *rcl_mem = (uint8_t *)Allocate(4096);
  uint8_t *list_mem = (uint8_t *)Allocate(256);
  uint8_t *tile_alloc = (uint8_t *)Allocate(4096, 4096);
  if (picture == 0 || rcl_mem == 0 || list_mem == 0 || tile_alloc == 0) {
    return false;
  }

  // The commands for every tile.
  Cl list(list_mem, 256);
  list.TileCoordinatesImplicit();
  list.EndOfLoads();
  list.StoreTileBuffer(BufferRt0, Address(picture), OutputRgba8, Stride);
  list.EndOfTileMarker();
  list.ReturnFromSubList();

  Cl rcl(rcl_mem, 4096);
  rcl.RenderingModeCommon(Width, Height, 1, InternalBpp32);
  rcl.RenderingModeRenderTarget0(Colour);
  rcl.RenderingModeZsClear(1.0f, 0);
  rcl.TileListInitialBlockSize(0, true);
  rcl.MulticoreTileListSetBase(Address(tile_alloc));
  rcl.MulticoreSupertileCfg(TilesX, TilesY, 1, 1);
  // Two dummy tiles that clear the tile buffer (and work around a
  // hardware problem with the first tile).
  for (int i = 0; i < 2; i++) {
    rcl.TileCoordinates(0, 0);
    rcl.EndOfLoads();
    rcl.StoreTileBuffer(BufferNone, 0, 0, 0);
    rcl.ClearTileBuffers(true, true);
    rcl.EndOfTileMarker();
  }
  rcl.FlushVcdCache();
  rcl.StartAddressOfGenericTileList(list.Start(), list.Here());
  for (unsigned y = 0; y < TilesY; y++) {
    for (unsigned x = 0; x < TilesX; x++) {
      rcl.SupertileCoordinates(x, y);
    }
  }
  rcl.EndOfRendering();
  if (list.Overflow() || rcl.Overflow()) {
    return false;
  }
  CleanAndInvalidateDataCacheRange((uintptr_t)list_mem, 256);
  CleanAndInvalidateDataCacheRange((uintptr_t)rcl_mem, 4096);

  CLogger::Get()->Write(FromGpu, LogNotice, "Clear test: %u bytes of commands", rcl.Used());
  bool ok = Render(rcl.Start(), rcl.Here(), 100);
  InvalidateDataCacheRange((uintptr_t)picture, Stride * Height);

  unsigned matching = 0;
  for (unsigned i = 0; i < Width * Height; i++) {
    if (picture[i] == picture[0]) {
      matching++;
    }
  }
  bool passed = ok && picture[0] != 0 && matching == Width * Height;
  CLogger::Get()->Write(FromGpu, passed ? LogNotice : LogWarning,
                        "Clear test %s: first pixel %08X (cleared to %08X), %u of %u the same",
                        passed ? "passed" : "failed", picture[0], Colour, matching,
                        Width * Height);
  if (!passed && ok) {
    LogState("After the clear test");
  }
  return passed;
}

} // namespace v3d
