//
// phy.h
//
// The BCM2712 HDMI PHY's settings for a TMDS character rate: its PLL's
// VCO divider, the rate manager's offset, the lane driver words, and the
// fixed PLL set-up values, from the behaviour of the Linux vc4 display
// driver.
//
// Pure computation, so tools/pi4kms can test it on a PC.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_KMS_PI5_PHY_H_
#define BMC64_KMS_PI5_PHY_H_

#include <stdint.h>

namespace kms {

struct Pi5Phy {
  unsigned vco_divider;
  uint64_t vco_hz;
  uint32_t rm_offset;          // with "offset only" set
  uint32_t data_lane;          // TX0, TX1 and TX2
  uint32_t clock_lane;
  uint32_t word_select;        // TMDS_CLK_WORD_SEL
};

// False at 340 MHz and above (no scrambling), or for a rate of 0.
bool ComputePi5Phy(uint64_t tmds_hz, Pi5Phy *phy);

// The PLL's nine fixed set-up words (PLL_MISC_0 to PLL_MISC_8).
const unsigned PllMiscWords = 9;
extern const uint32_t PllMisc[PllMiscWords];

} // namespace kms

#endif
