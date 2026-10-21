//
// sound.cpp
//
// See sound.h for what this is and the sources it follows.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "sound.h"

#include "hdmisound.h"

#include <circle/logger.h>
#include <circle/machineinfo.h>
#include <circle/sound/pwmsoundbasedevice.h>

static const char FromSound[] = "sound";

// The queue holds up to this much; the emulator keeps it filled to the
// latency it asks for (audio_buffer_ms), which must be less.
static const unsigned QUEUE_MS = 200;

// 2 x 384 samples: 384 stereo frames, 8 ms at 48 kHz, per DMA transfer.
// Circle's default is ten times that, which would add 80 ms of delay.
// The Pi 5's HDMI plays 8 ms transfers slightly slow (about 47.8 kHz),
// which roughens the sound; 1536 frames (32 ms) play at 48 kHz.
#if RASPPI >= 5
static const unsigned CHUNK_SIZE = 1536 * 2;
#else
static const unsigned CHUNK_SIZE = 384 * 2;
#endif

Pi4Sound::Pi4Sound(void)
    : m_pDevice(0), m_nChannels(1), m_nLatency(0), m_nGain(256) {}

Pi4Sound::~Pi4Sound(void) { delete m_pDevice; }

bool Pi4Sound::Start(CSoundBaseDevice *device, const char *name) {
  // Everything but Write() and the queue level has to happen on core 0,
  // so the device is set up and started here, before the emulator runs.
  // Until the emulator writes, the driver sends silence.
  if (!device->AllocateQueueFrames(kSampleRate * QUEUE_MS / 1000)) {
    CLogger::Get()->Write(FromSound, LogError, "Cannot allocate the sound queue");
    delete device;
    return false;
  }
  device->SetWriteFormat(SoundFormatSigned16, 2);
  if (!device->Start()) {
    CLogger::Get()->Write(FromSound, LogWarning, "Cannot start %s sound", name);
    delete device;
    return false;
  }
  m_pDevice = device;
  CLogger::Get()->Write(FromSound, LogNotice, "%s sound, %u Hz", name, kSampleRate);
  return true;
}

bool Pi4Sound::Initialize(CInterruptSystem *pInterrupt, int volume, Output output) {
  SetVolume(volume);

#if RASPPI >= 5
  bool has_jack = false;
#else
  bool has_jack = CMachineInfo::Get()->GetMachineModel() != MachineModel400;
#endif
  if (output == Output::Headphones && !has_jack) {
    CLogger::Get()->Write(FromSound, LogWarning, "No headphone jack; sound goes to HDMI");
    output = Output::Hdmi;
  }

  // The HDMI port with a display on it (HDMI0 if both, or neither).
  int port = Pi4HdmiSound::ConnectedPort();
  if (port < 0) {
    port = 0;
  }
  if (output != Output::Headphones &&
      Start(new Pi4HdmiSound(pInterrupt, (unsigned)port, kSampleRate, CHUNK_SIZE),
            port == 0 ? "HDMI0" : "HDMI1")) {
    return true;
  }
  if (output == Output::Hdmi || !has_jack) {
    return false;
  }
  // The headphone jack: asked for, or HDMI takes no sound.
  return Start(new CPWMSoundBaseDevice(pInterrupt, kSampleRate, CHUNK_SIZE),
               "Headphone jack");
}

void Pi4Sound::SetVolume(int volume) {
  if (volume < 0) {
    volume = 0;
  } else if (volume > 100) {
    volume = 100;
  }
  m_nGain = volume * 256 / 100;
}

unsigned Pi4Sound::Open(int channels, unsigned latency_frames) {
  if (m_pDevice == 0 || !m_pDevice->IsActive()) {
    return 0;
  }
  m_nChannels = channels == 2 ? 2 : 1;
  m_nLatency = latency_frames;
  unsigned max = kSampleRate * QUEUE_MS / 1000;
  if (m_nLatency > max - CHUNK_SIZE) {
    m_nLatency = max - CHUNK_SIZE;
  }
  return kSampleRate;
}

unsigned Pi4Sound::FreeFrames(void) {
  if (m_pDevice == 0 || !m_pDevice->IsActive()) {
    return 0;
  }
  unsigned queued = m_pDevice->GetQueueFramesAvail();
  return queued >= m_nLatency ? 0 : m_nLatency - queued;
}

void Pi4Sound::Write(const int16_t *samples, unsigned frames) {
  if (m_pDevice == 0 || !m_pDevice->IsActive()) {
    return;
  }
  // The queue takes stereo; mono samples go to both sides.
  int16_t buffer[512 * 2];
  while (frames > 0) {
    unsigned n = frames > 512 ? 512 : frames;
    for (unsigned i = 0; i < n; i++) {
      int left = samples[i * m_nChannels];
      int right = samples[i * m_nChannels + m_nChannels - 1];
      buffer[i * 2] = (int16_t)(left * m_nGain / 256);
      buffer[i * 2 + 1] = (int16_t)(right * m_nGain / 256);
    }
    m_pDevice->Write(buffer, n * 2 * sizeof(int16_t));
    samples += n * m_nChannels;
    frames -= n;
  }
}
