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

#include "plus4emu.hpp"
#include "sid.hpp"
#include <cmath>

namespace Plus4 {

  // --------------------------------------------------------------------------
  // Constructor.
  // --------------------------------------------------------------------------
  SID::SID(int32_t& soundOutputAccumulator_)
    : soundOutputAccumulator(soundOutputAccumulator_)
  {
    sid_model = MOS6581;
    voice[0].set_sync_source(&voice[2]);
    voice[1].set_sync_source(&voice[0]);
    voice[2].set_sync_source(&voice[1]);

    bus_value = 0;
    bus_value_ttl = 0;
    write_pipeline = 0;

    databus_ttl = 0;
  }

  // --------------------------------------------------------------------------
  // Destructor.
  // --------------------------------------------------------------------------
  SID::~SID()
  {
  }

  // --------------------------------------------------------------------------
  // Set chip model.
  // --------------------------------------------------------------------------
  void SID::set_chip_model(chip_model model)
  {
    sid_model = model;

    /*
      results from real C64 (testprogs/SID/bitfade/delayfrq0.prg):

      (new SID) (250469/8580R5) (250469/8580R5)
      delayfrq0    ~7a000        ~108000

      (old SID) (250407/6581)
      delayfrq0    ~01d00

     */
#if 0
    databus_ttl = sid_model == MOS8580 ? 0xa2000 : 0x1d00;
#else
    // corrected values for TED clock frequency
    databus_ttl = sid_model == MOS8580 ?
                  int(0.663552 * RESID_CLOCK_FREQUENCY + 0.5)
                  : int(0.007424 * RESID_CLOCK_FREQUENCY + 0.5);
#endif

    for (int i = 0; i < 3; i++) {
      voice[i].set_chip_model(model);
    }

    filter.set_chip_model(model);
  }

  // --------------------------------------------------------------------------
  // SID reset.
  // --------------------------------------------------------------------------
  void SID::reset()
  {
    for (int i = 0; i < 3; i++) {
      voice[i].reset();
    }
    filter.reset();
    extfilt.reset();

    bus_value = 0;
    bus_value_ttl = 0;
  }

  // --------------------------------------------------------------------------
  // Write 16-bit sample to audio input.
  // Note that to mix in an external audio signal, the signal should be
  // resampled to 1MHz first to avoid sampling noise.
  // --------------------------------------------------------------------------
  void SID::input(short sample)
  {
    // The input can be used to simulate the MOS8580 "digi boost" hardware hack.
    filter.input(sample);
  }

  // --------------------------------------------------------------------------
  // Read registers.
  //
  // Reading a write only register returns the last byte written to any SID
  // register. The individual bits in this value start to fade down towards
  // zero after a few cycles. All bits reach zero within approximately
  // $2000 - $4000 cycles.
  // It has been claimed that this fading happens in an orderly fashion,
  // however sampling of write only registers reveals that this is not the
  // case.
  // NB! This is not correctly modeled.
  // The actual use of write only registers has largely been made in the belief
  // that all SID registers are readable. To support this belief the read
  // would have to be done immediately after a write to the same register
  // (remember that an intermediate write to another register would yield that
  // value instead). With this in mind we return the last value written to
  // any SID register for $4000 cycles without modeling the bit fading.
  // --------------------------------------------------------------------------
  reg8 SID::read(reg8 offset)
  {
    switch (offset) {
    case 0x19:
      bus_value = potx.readPOT();
      bus_value_ttl = databus_ttl;
      break;
    case 0x1a:
      bus_value = poty.readPOT();
      bus_value_ttl = databus_ttl;
      break;
    case 0x1b:
      bus_value = voice[2].wave.readOSC();
      bus_value_ttl = databus_ttl;
      break;
    case 0x1c:
      bus_value = voice[2].envelope.readENV();
      bus_value_ttl = databus_ttl;
      break;
    }
    return bus_value;
  }

  reg8 SID::readDebug(reg8 offset) const
  {
    switch (offset & 0x1f) {
    case 0x19:
      return potx.readPOT();
    case 0x1a:
      return poty.readPOT();
    case 0x1b:
      return voice[2].wave.readOSC();
    case 0x1c:
      return voice[2].envelope.readENV();
    }
    return bus_value;
  }

  // --------------------------------------------------------------------------
  // Write registers.
  // Writes are one cycle delayed on the MOS8580. This is only modeled for
  // single cycle clocking.
  // --------------------------------------------------------------------------
  void SID::write(reg8 offset, reg8 value)
  {
    // if there is already a pending write, complete it first
    if (PLUS4EMU_UNLIKELY(write_pipeline))
      write();

    write_address = offset;
    bus_value = value;
    bus_value_ttl = databus_ttl;

    if (sid_model == MOS8580) {
      write_pipeline = 1;
    }
    else {
      write();
    }
  }

  // --------------------------------------------------------------------------
  // Write registers.
  // --------------------------------------------------------------------------
  void SID::write()
  {
    switch (write_address) {
    case 0x00:
      voice[0].wave.writeFREQ_LO(bus_value);
      break;
    case 0x01:
      voice[0].wave.writeFREQ_HI(bus_value);
      break;
    case 0x02:
      voice[0].wave.writePW_LO(bus_value);
      break;
    case 0x03:
      voice[0].wave.writePW_HI(bus_value);
      break;
    case 0x04:
      voice[0].writeCONTROL_REG(bus_value);
      break;
    case 0x05:
      voice[0].envelope.writeATTACK_DECAY(bus_value);
      break;
    case 0x06:
      voice[0].envelope.writeSUSTAIN_RELEASE(bus_value);
      break;
    case 0x07:
      voice[1].wave.writeFREQ_LO(bus_value);
      break;
    case 0x08:
      voice[1].wave.writeFREQ_HI(bus_value);
      break;
    case 0x09:
      voice[1].wave.writePW_LO(bus_value);
      break;
    case 0x0a:
      voice[1].wave.writePW_HI(bus_value);
      break;
    case 0x0b:
      voice[1].writeCONTROL_REG(bus_value);
      break;
    case 0x0c:
      voice[1].envelope.writeATTACK_DECAY(bus_value);
      break;
    case 0x0d:
      voice[1].envelope.writeSUSTAIN_RELEASE(bus_value);
      break;
    case 0x0e:
      voice[2].wave.writeFREQ_LO(bus_value);
      break;
    case 0x0f:
      voice[2].wave.writeFREQ_HI(bus_value);
      break;
    case 0x10:
      voice[2].wave.writePW_LO(bus_value);
      break;
    case 0x11:
      voice[2].wave.writePW_HI(bus_value);
      break;
    case 0x12:
      voice[2].writeCONTROL_REG(bus_value);
      break;
    case 0x13:
      voice[2].envelope.writeATTACK_DECAY(bus_value);
      break;
    case 0x14:
      voice[2].envelope.writeSUSTAIN_RELEASE(bus_value);
      break;
    case 0x15:
      filter.writeFC_LO(bus_value);
      break;
    case 0x16:
      filter.writeFC_HI(bus_value);
      break;
    case 0x17:
      filter.writeRES_FILT(bus_value);
      break;
    case 0x18:
      filter.writeMODE_VOL(bus_value);
      break;
    default:
      break;
    }

    // Tell clock() that the pipeline is empty.
    write_pipeline = 0;
  }

  // --------------------------------------------------------------------------
  // Mask for voices routed into the filter / audio output stage.
  // Used to physically connect/disconnect EXT IN, and for test purposed
  // (voice muting).
  // --------------------------------------------------------------------------
  void SID::set_voice_mask(reg4 mask)
  {
    filter.set_voice_mask(mask);
  }

  // --------------------------------------------------------------------------
  // Enable filter.
  // --------------------------------------------------------------------------
  void SID::enable_filter(bool enable)
  {
    filter.enable_filter(enable);
  }

  // --------------------------------------------------------------------------
  // Adjust the DAC bias parameter of the filter.
  // This gives user variable control of the exact CF -> center frequency
  // mapping used by the filter.
  // The setting is currently only effective for 6581.
  // --------------------------------------------------------------------------
  void SID::adjust_filter_bias(double dac_bias)
  {
    filter.adjust_filter_bias(dac_bias);
  }

  // --------------------------------------------------------------------------
  // Enable external filter.
  // --------------------------------------------------------------------------
  void SID::enable_external_filter(bool enable)
  {
    extfilt.enable_filter(enable);
  }

  // --------------------------------------------------------------------------
  // SID clocking - 1 cycle.
  // --------------------------------------------------------------------------
  PLUS4EMU_INLINE void SID::clock_fast()
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

    soundOutputAccumulator =
        soundOutputAccumulator + (int32_t(filter.output()) << 3);

    // Pipelined writes on the MOS8580.
    if (PLUS4EMU_UNLIKELY(write_pipeline)) {
      write();
    }

    // Age bus value.
    if (PLUS4EMU_UNLIKELY(!--bus_value_ttl)) {
      bus_value = 0;
    }
  }

  // --------------------------------------------------------------------------
  // SID clocking - 1 cycle.
  // --------------------------------------------------------------------------
  PLUS4EMU_REGPARM1 void SID::clockCallback(void *userData)
  {
    reinterpret_cast< SID * >(userData)->clock_fast();
  }

  // --------------------------------------------------------------------------
  // SID clocking - delta_t cycles.
  // --------------------------------------------------------------------------
  void SID::clock(cycle_count delta_t)
  {
    int i;

    // Pipelined writes on the MOS8580.
    if (PLUS4EMU_UNLIKELY(write_pipeline) && PLUS4EMU_EXPECT(delta_t > 0)) {
      // Step one cycle by a recursive call to ourselves.
      write_pipeline = 0;
      clock(1);
      write();
      delta_t -= 1;
    }

    if (PLUS4EMU_UNLIKELY(delta_t <= 0)) {
      return;
    }

    // Age bus value.
    bus_value_ttl -= delta_t;
    if (PLUS4EMU_UNLIKELY(bus_value_ttl <= 0)) {
      bus_value = 0;
      bus_value_ttl = 0;
    }

    // Clock amplitude modulators.
    for (i = 0; i < 3; i++) {
      voice[i].envelope.clock(delta_t);
    }

    // Clock and synchronize oscillators.
    // Loop until we reach the current cycle.
    cycle_count delta_t_osc = delta_t;
    while (delta_t_osc) {
      cycle_count delta_t_min = delta_t_osc;

      // Find minimum number of cycles to an oscillator accumulator MSB toggle.
      // We have to clock on each MSB on / MSB off for hard sync to operate
      // correctly.
      for (i = 0; i < 3; i++) {
        WaveformGenerator& wave = voice[i].wave;

        // It is only necessary to clock on the MSB of an oscillator that is
        // a sync source and has freq != 0.
        if (PLUS4EMU_EXPECT(!(wave.sync_dest->sync && wave.freq))) {
          continue;
        }

        reg16 freq = wave.freq;
        reg24 accumulator = wave.accumulator;

        // Clock on MSB off if MSB is on, clock on MSB on if MSB is off.
        reg24 delta_accumulator =
          (accumulator & 0x800000 ? 0x1000000 : 0x800000) - accumulator;

        cycle_count delta_t_next = delta_accumulator/freq;
        if (PLUS4EMU_EXPECT(bool(delta_accumulator%freq))) {
          ++delta_t_next;
        }

        if (PLUS4EMU_UNLIKELY(delta_t_next < delta_t_min)) {
          delta_t_min = delta_t_next;
        }
      }

      // Clock oscillators.
      for (i = 0; i < 3; i++) {
        voice[i].wave.clock(delta_t_min);
      }

      // Synchronize oscillators.
      for (i = 0; i < 3; i++) {
        voice[i].wave.synchronize();
      }

      delta_t_osc -= delta_t_min;
    }

    // Calculate waveform output.
    for (i = 0; i < 3; i++) {
      voice[i].wave.set_waveform_output(delta_t);
    }

    // Clock filter.
    filter.clock(delta_t,
                 voice[0].output(), voice[1].output(), voice[2].output());

    // Clock external filter.
    extfilt.clock(delta_t, filter.output());
  }

  // --------------------------------------------------------------------------

  class ChunkType_SIDSnapshot : public Plus4Emu::File::ChunkTypeHandler {
   private:
    SID&    ref;
   public:
    ChunkType_SIDSnapshot(SID& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_SIDSnapshot()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_SID_STATE;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadState(buf);
    }
  };

  void SID::saveState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    buf.writeUInt32(0x01000001);        // version number
    buf.writeBoolean(sid_model == MOS6581);
    {
      int i, j;

      for (i = 0, j = 0; i < 3; i++, j += 7) {
        WaveformGenerator& wave = voice[i].wave;
        EnvelopeGenerator& envelope = voice[i].envelope;
        buf.writeByte(wave.freq & 0xff);
        buf.writeByte(wave.freq >> 8);
        buf.writeByte(wave.pw & 0xff);
        buf.writeByte(wave.pw >> 8);
        buf.writeByte((wave.waveform << 4)
                      | (wave.test ? 0x08 : 0)
                      | (wave.ring_mod ? 0x04 : 0)
                      | (wave.sync ? 0x02 : 0)
                      | (envelope.gate ? 0x01 : 0));
        buf.writeByte((envelope.attack << 4) | envelope.decay);
        buf.writeByte((envelope.sustain << 4) | envelope.release);
      }

      buf.writeByte(filter.fc & 0x007);
      buf.writeByte(filter.fc >> 3);
      buf.writeByte((filter.res << 4) | filter.filt);
      buf.writeByte(filter.mode | filter.vol);

      // These registers are superfluous, but are included for completeness.
      for (j = 0x19; j < 0x1d; j++) {
        buf.writeByte(readDebug(j));
      }
      for (; j < 0x20; j++) {
        buf.writeByte(0x00);
      }
    }
    buf.writeByte(uint8_t(bus_value));
    buf.writeInt32(int32_t(bus_value_ttl));
    buf.writeBoolean(bool(write_pipeline));
    buf.writeByte(uint8_t(write_address));
    buf.writeByte(uint8_t(filter.voice_mask));
    for (uint8_t i = 0; i < 3; i++) {
      buf.writeUInt32(voice[i].wave.accumulator);
      buf.writeUInt32(voice[i].wave.shift_register);
      buf.writeInt32(int32_t(voice[i].wave.shift_register_reset));
      buf.writeByte(uint8_t(voice[i].wave.shift_pipeline));
      buf.writeUInt32(voice[i].wave.pulse_output);
      buf.writeInt32(int32_t(voice[i].wave.floating_output_ttl));
      buf.writeUInt32(voice[i].envelope.rate_counter);
      buf.writeUInt32(voice[i].envelope.rate_period);
      buf.writeUInt32(voice[i].envelope.exponential_counter);
      buf.writeUInt32(voice[i].envelope.exponential_counter_period);
      buf.writeByte(uint8_t(voice[i].envelope.envelope_counter));
      if (voice[i].envelope.state == EnvelopeGenerator::ATTACK)
        buf.writeByte(1);
      else if (voice[i].envelope.state == EnvelopeGenerator::DECAY_SUSTAIN)
        buf.writeByte(2);
      else
        buf.writeByte(0);
      buf.writeBoolean(voice[i].envelope.hold_zero);
      buf.writeBoolean(bool(voice[i].envelope.envelope_pipeline));
    }
  }

  void SID::saveState(Plus4Emu::File& f)
  {
    Plus4Emu::File::Buffer  buf;
    this->saveState(buf);
    f.addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_SID_STATE, buf);
  }

  void SID::loadState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
    if (!(version >= 0x01000000 && version <= 0x01000001)) {
      buf.setPosition(buf.getDataSize());
      throw Plus4Emu::Exception("incompatible SID snapshot format");
    }
    // set default state
    for (uint8_t i = 0; i < 0x20; i++)
      write(i, 0x00);
    bus_value = 0;
    bus_value_ttl = 0;
    write_pipeline = 0;
    write_address = 0;
    filter.set_voice_mask(0xff);
    for (int i = 0; i < 3; i++) {
      voice[i].wave.accumulator = 0;
      voice[i].wave.shift_register = 0x7fffff;
      voice[i].wave.shift_register_reset = 0;
      voice[i].wave.shift_pipeline = 0;
      voice[i].wave.pulse_output = 0;
      voice[i].wave.floating_output_ttl = 0;

      voice[i].envelope.rate_counter = 0;
      voice[i].envelope.rate_period = 9;
      voice[i].envelope.exponential_counter = 0;
      voice[i].envelope.exponential_counter_period = 1;
      voice[i].envelope.envelope_counter = 0;
      voice[i].envelope.state = EnvelopeGenerator::RELEASE;
      voice[i].envelope.hold_zero = true;
      voice[i].envelope.envelope_pipeline = 0;
    }
    try {
      // load saved state
      set_chip_model(
          (version >= 0x01000001 && buf.readBoolean()) ? MOS6581 : MOS8580);
      for (uint8_t i = 0x00; i < 0x20; i++)
        write(i, buf.readByte());
      bus_value = buf.readByte();
      bus_value_ttl = buf.readInt32();
      if (version >= 0x01000001) {
        write_pipeline = cycle_count(buf.readBoolean());
        write_address = buf.readByte() & 0x1F;
        filter.set_voice_mask(buf.readByte() & 0x0F);
      }
      for (uint8_t i = 0; i < 3; i++) {
        voice[i].wave.accumulator = buf.readUInt32() & 0x00FFFFFFU;
        voice[i].wave.shift_register = buf.readUInt32() & 0x00FFFFFFU;
        if (version >= 0x01000001) {
          voice[i].wave.shift_register_reset = buf.readInt32();
          voice[i].wave.shift_pipeline = buf.readByte() & 3;
          voice[i].wave.pulse_output = uint16_t(buf.readUInt32() & 0xFFFFU);
          voice[i].wave.floating_output_ttl = buf.readInt32();
        }
        voice[i].envelope.rate_counter = buf.readUInt32() & 0x0000FFFFU;
        voice[i].envelope.rate_period = buf.readUInt32() & 0x0000FFFFU;
        voice[i].envelope.exponential_counter = buf.readUInt32() & 0x0000FFFFU;
        voice[i].envelope.exponential_counter_period =
            buf.readUInt32() & 0x0000FFFFU;
        voice[i].envelope.envelope_counter = buf.readByte();
        uint8_t tmp = buf.readByte();
        if (tmp == 1)
          voice[i].envelope.state = EnvelopeGenerator::ATTACK;
        else if (tmp == 2)
          voice[i].envelope.state = EnvelopeGenerator::DECAY_SUSTAIN;
        else
          voice[i].envelope.state = EnvelopeGenerator::RELEASE;
        voice[i].envelope.hold_zero = buf.readBoolean();
        if (version >= 0x01000001)
          voice[i].envelope.envelope_pipeline = cycle_count(buf.readBoolean());
      }
      if (buf.getPosition() != buf.getDataSize())
        throw Plus4Emu::Exception("trailing garbage at end of "
                                  "SID snapshot data");
    }
    catch (...) {
      // reset SID
      try {
        this->reset();
      }
      catch (...) {
      }
      throw;
    }
  }

  void SID::registerChunkType(Plus4Emu::File& f)
  {
    ChunkType_SIDSnapshot   *p;
    p = new ChunkType_SIDSnapshot(*this);
    try {
      f.registerChunkType(p);
    }
    catch (...) {
      delete p;
      throw;
    }
  }

}       // namespace Plus4

