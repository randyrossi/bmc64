//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581 SID emulator engine.
//  Copyright (C) 2010  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#ifndef RESID_SID_HPP
#define RESID_SID_HPP

#include "plus4emu.hpp"
#include "siddefs.hpp"
#include "voice.hpp"
#include "filter.hpp"
#include "extfilt.hpp"
#include "pot.hpp"
#include "fileio.hpp"

namespace Plus4 {

  class SID {
  public:
    SID(int32_t& soundOutputAccumulator_);
    ~SID();

    void set_chip_model(chip_model model);
    void set_voice_mask(reg4 mask);
    void enable_filter(bool enable);
    void adjust_filter_bias(double dac_bias);
    void enable_external_filter(bool enable);

    // callback function for Plus4VM, 'userData' is a pointer to "this"
    static PLUS4EMU_REGPARM1 void clockCallback(void *userData);
    PLUS4EMU_INLINE void clock();
    void clock(cycle_count delta_t);
    void reset();

    // Read/write registers.
    reg8 read(reg8 offset);
    reg8 readDebug(reg8 offset) const;
    void write(reg8 offset, reg8 value);

    // 16-bit input (EXT IN).
    void input(short sample);

    // 16-bit output (AUDIO OUT).
    PLUS4EMU_INLINE short output();
    // fast inline version with no 2^-11 scaling and -32768 to 32767 clipping
    PLUS4EMU_INLINE int fast_output();

  protected:
    void write();
    // simplified version with no external filter,
    // adds the output to soundOutputAccumulator
    PLUS4EMU_INLINE void clock_fast();

    chip_model sid_model;
    Voice voice[3];
    Filter filter;
    ExternalFilter extfilt;
    Potentiometer potx;
    Potentiometer poty;

    reg8 bus_value;
    cycle_count bus_value_ttl;

    // The data bus TTL for the selected chip model
    cycle_count databus_ttl;

    // Pipeline for writes on the MOS8580.
    cycle_count write_pipeline;
    reg8 write_address;

    int32_t&  soundOutputAccumulator;

  public:
    void saveState(Plus4Emu::File::Buffer&);
    void saveState(Plus4Emu::File&);
    void loadState(Plus4Emu::File::Buffer&);
    void registerChunkType(Plus4Emu::File&);
  };

  // --------------------------------------------------------------------------
  // Inline functions.
  // The following functions are defined inline because they are called every
  // time a sample is calculated.
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Read 16-bit sample from audio output.
  // --------------------------------------------------------------------------
  PLUS4EMU_INLINE short SID::output()
  {
    return extfilt.output();
  }

  // --------------------------------------------------------------------------
  // Read 27-bit sample from audio output.
  // --------------------------------------------------------------------------
  PLUS4EMU_INLINE int SID::fast_output()
  {
    return extfilt.fast_output();
  }

  // --------------------------------------------------------------------------
  // SID clocking - 1 cycle.
  // --------------------------------------------------------------------------
  PLUS4EMU_INLINE void SID::clock()
  {
    int i;

    // Clock amplitude modulators.
    for (i = 0; i < 3; i++) {
      voice[i].envelope.clock();
    }

    // Clock oscillators.
    for (i = 0; i < 3; i++) {
      voice[i].wave.clock();
    }

    // Synchronize oscillators.
    for (i = 0; i < 3; i++) {
      voice[i].wave.synchronize();
    }

    // Calculate waveform output.
    for (i = 0; i < 3; i++) {
      voice[i].wave.set_waveform_output();
    }

    // Clock filter.
    filter.clock(voice[0].output(), voice[1].output(), voice[2].output());

    // Clock external filter.
    extfilt.clock(filter.output());

    // Pipelined writes on the MOS8580.
    if (PLUS4EMU_UNLIKELY(write_pipeline)) {
      write();
    }

    // Age bus value.
    if (PLUS4EMU_UNLIKELY(!--bus_value_ttl)) {
      bus_value = 0;
    }
  }

}       // namespace Plus4

#endif  // not RESID_SID_HPP

