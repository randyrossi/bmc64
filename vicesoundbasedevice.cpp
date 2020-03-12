//
// vicesoundbasedevice.h
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "vicesoundbasedevice.h"
#include <assert.h>
#include <circle/devicenameservice.h>
#include <circle/logger.h>
#include <circle/sched/scheduler.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VOLUME_TO_CHIP(volume) ((unsigned)-(((volume) << 8) / 100))

static const char FromVCHIQSound[] = "sndvchiq";

ViceSoundBaseDevice::ViceSoundBaseDevice(CVCHIQDevice *pVCHIQDevice,
                                         unsigned nSampleRate,
                                         unsigned nChunkSize,
                                         TVCHIQSoundDestination Destination)
    : CSoundBaseDevice(SoundFormatSigned16, 0, nSampleRate),
      m_nSampleRate(nSampleRate), m_nChunkSize(nChunkSize),
      m_Destination(Destination), m_State(VCHIQSoundCreated), m_VCHIInstance(0),
      m_hService(0), m_nVolume(VCHIQ_SOUND_VOLUME_DEFAULT) {
  // assert (44100 <= nSampleRate && nSampleRate <= 48000);
  assert(Destination < VCHIQSoundDestinationUnknown);

  CDeviceNameService::Get()->AddDevice("sndvchiq", this, FALSE);

  p_buffer = (s16*) malloc(sizeof(s16) * nChunkSize);
  memset(p_buffer, 0, sizeof(s16) * nChunkSize);
}

ViceSoundBaseDevice::~ViceSoundBaseDevice(void) { assert(0); }

int ViceSoundBaseDevice::GetRangeMin(void) const { return -32768; }

int ViceSoundBaseDevice::GetRangeMax(void) const { return 32767; }

void ViceSoundBaseDevice::SetVolume(int nVolume) {
  m_nVolume = nVolume;
}

void ViceSoundBaseDevice::SetChannels(int nChannels) {
  m_nChannels = nChannels;
}

boolean ViceSoundBaseDevice::Start() {
  if (m_State > VCHIQSoundIdle) {
    return FALSE;
  }

  VC_AUDIO_MSG_T Msg;
  int nResult;

  if (m_State == VCHIQSoundCreated) {
    nResult = vchi_initialise(&m_VCHIInstance);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError,
                            "Cannot initialize VCHI (%d)", nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }

    nResult = vchi_connect(0, 0, m_VCHIInstance);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError,
                            "Cannot connect VCHI (%d)", nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }

    SERVICE_CREATION_T Params = {
        VCHI_VERSION_EX(VC_AUDIOSERV_VER, VC_AUDIOSERV_MIN_VER),
        VC_AUDIO_SERVER_NAME,
        0,
        0,
        0, // unused
        CallbackStub,
        this,
        1,
        1,
        0 // unused (bulk)
    };

    nResult = vchi_service_open(m_VCHIInstance, &Params, &m_hService);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError,
                            "Cannot open AUDS service (%d)", nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }

    vchi_service_release(m_hService);

    Msg.type = VC_AUDIO_MSG_TYPE_CONFIG;
    Msg.u.config.channels = m_nChannels;
    Msg.u.config.samplerate = m_nSampleRate;
    Msg.u.config.bps = 16;

    nResult = CallMessage(&Msg);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError, "Cannot set config (%d)",
                            nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }

    Msg.type = VC_AUDIO_MSG_TYPE_CONTROL;
    Msg.u.control.dest = m_Destination;
    Msg.u.control.volume = VOLUME_TO_CHIP(m_nVolume);

    nResult = CallMessage(&Msg);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError, "Cannot set control (%d)",
                            nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }

    Msg.type = VC_AUDIO_MSG_TYPE_OPEN;

    nResult = QueueMessage(&Msg);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError, "Cannot open audio (%d)",
                            nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }

    m_State = VCHIQSoundIdle;
  } else {
    // Added this so we can switch channels after a Cancel.
    Msg.type = VC_AUDIO_MSG_TYPE_CONFIG;
    Msg.u.config.channels = m_nChannels;
    Msg.u.config.samplerate = m_nSampleRate;
    Msg.u.config.bps = 16;

    nResult = CallMessage(&Msg);
    if (nResult != 0) {
      CLogger::Get()->Write(FromVCHIQSound, LogError, "Cannot set config (%d)",
                            nResult);

      m_State = VCHIQSoundError;

      return FALSE;
    }
  }

  assert(m_State == VCHIQSoundIdle);

  Msg.type = VC_AUDIO_MSG_TYPE_START;

  nResult = QueueMessage(&Msg);
  if (nResult != 0) {
    CLogger::Get()->Write(FromVCHIQSound, LogError, "Cannot start audio (%d)",
                          nResult);

    m_State = VCHIQSoundError;

    return FALSE;
  }

  m_State = VCHIQSoundRunning;

  vchi_service_use(m_hService);

  short usPeerVersion = 0;
  nResult = vchi_get_peer_version(m_hService, &usPeerVersion);
  if (nResult != 0) {
    vchi_service_release(m_hService);

    CLogger::Get()->Write(FromVCHIQSound, LogError,
                          "Cannot get peer version (%d)", nResult);

    m_State = VCHIQSoundError;

    return FALSE;
  }

  // we need peer version 2, because we do not support bulk transfers
  if (usPeerVersion < 2) {
    vchi_service_release(m_hService);

    CLogger::Get()->Write(FromVCHIQSound, LogError,
                          "Peer version does not match (%u)",
                          (unsigned)usPeerVersion);

    m_State = VCHIQSoundError;

    return FALSE;
  }

  m_nWritePos = 0;
  m_nCompletePos = 0;

  nResult = WriteChunk();
  if (nResult == 0) {
    nResult = WriteChunk();
  }

  if (nResult != 0) {
    vchi_service_release(m_hService);

    CLogger::Get()->Write(FromVCHIQSound, LogError,
                          "Cannot write audio data (%d)", nResult);

    m_State = VCHIQSoundError;

    return FALSE;
  }

  vchi_service_release(m_hService);

  return TRUE;
}

void ViceSoundBaseDevice::Cancel(void) {
  if (m_State != VCHIQSoundRunning) {
    return;
  }

  m_State = VCHIQSoundCancelled;
  if (m_nWritePos - m_nCompletePos > 0) {
     while (m_State == VCHIQSoundCancelled) {
       CScheduler::Get()->Yield();
     }
  } else {
     m_State = VCHIQSoundTerminating;
  }

  assert(m_State == VCHIQSoundTerminating);

  vchi_service_use(m_hService);

  VC_AUDIO_MSG_T Msg;

  Msg.type = VC_AUDIO_MSG_TYPE_STOP;
  Msg.u.stop.draining = 0;

  int nResult = QueueMessage(&Msg);
  if (nResult != 0) {
    CLogger::Get()->Write(FromVCHIQSound, LogError, "Cannot stop audio (%d)",
                          nResult);
  }

  vchi_service_release(m_hService);

  m_State = VCHIQSoundIdle;
}

boolean ViceSoundBaseDevice::IsActive(void) const {
  return m_State >= VCHIQSoundRunning;
}

void ViceSoundBaseDevice::SetControl(int nVolume,
                                     TVCHIQSoundDestination Destination) {
  if (!(VCHIQ_SOUND_VOLUME_MIN <= nVolume &&
        nVolume <= VCHIQ_SOUND_VOLUME_MAX)) {
    nVolume = VCHIQ_SOUND_VOLUME_DEFAULT;
  }

  if (Destination < VCHIQSoundDestinationUnknown) {
    m_Destination = Destination;
  }

  VC_AUDIO_MSG_T Msg;
  Msg.type = VC_AUDIO_MSG_TYPE_CONTROL;
  Msg.u.control.dest = m_Destination;
  Msg.u.control.volume = VOLUME_TO_CHIP(nVolume);

  int nResult = CallMessage(&Msg);
  if (nResult != 0) {
    CLogger::Get()->Write(FromVCHIQSound, LogWarning, "Cannot set control (%d)",
                          nResult);
  }
}

int ViceSoundBaseDevice::CallMessage(VC_AUDIO_MSG_T *pMessage) {
  m_Event.Clear();

  int nResult = QueueMessage(pMessage);
  if (nResult == 0) {
    m_Event.Wait();
  } else {
    m_nResult = nResult;
  }

  return m_nResult;
}

int ViceSoundBaseDevice::QueueMessage(VC_AUDIO_MSG_T *pMessage) {
  vchi_service_use(m_hService);

  int nResult = vchi_msg_queue(m_hService, pMessage, sizeof *pMessage,
                               VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);

  vchi_service_release(m_hService);

  return nResult;
}

int ViceSoundBaseDevice::WriteChunk(void) {
  unsigned nWords = GetChunk(p_buffer, m_nChunkSize);
  if (nWords == 0) {
    m_State = VCHIQSoundIdle;
    return 0;
  }

  if (m_State == VCHIQSoundIdle || m_State == VCHIQSoundCancelled) {
    return 0;
  }

  unsigned nBytes = nWords * sizeof(s16);

  VC_AUDIO_MSG_T Msg;

  Msg.type = VC_AUDIO_MSG_TYPE_WRITE;
  Msg.u.write.count = nBytes;
  Msg.u.write.max_packet = 4000;
  Msg.u.write.cookie1 = VC_AUDIO_WRITE_COOKIE1;
  Msg.u.write.cookie2 = VC_AUDIO_WRITE_COOKIE2;
  Msg.u.write.silence = 0;

  int nResult = vchi_msg_queue(m_hService, &Msg, sizeof Msg,
                               VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
  if (nResult != 0) {
    return nResult;
  }

  m_nWritePos += nBytes;

  u8 *pBuffer8 = (u8 *)p_buffer;
  while (nBytes > 0) {
    unsigned nBytesToQueue =
        nBytes <= Msg.u.write.max_packet ? nBytes : Msg.u.write.max_packet;

    nResult = vchi_msg_queue(m_hService, pBuffer8, nBytesToQueue,
                             VCHI_FLAGS_BLOCK_UNTIL_QUEUED, 0);
    if (nResult != 0) {
      return nResult;
    }

    pBuffer8 += nBytesToQueue;
    nBytes -= nBytesToQueue;
  }

  return 0;
}

void ViceSoundBaseDevice::Callback(const VCHI_CALLBACK_REASON_T Reason,
                                   void *hMessage) {
  if (Reason != VCHI_CALLBACK_MSG_AVAILABLE) {
    assert(0);
    return;
  }

  vchi_service_use(m_hService);

  VC_AUDIO_MSG_T Msg;
  uint32_t nMsgLen;
  int nResult =
      vchi_msg_dequeue(m_hService, &Msg, sizeof Msg, &nMsgLen, VCHI_FLAGS_NONE);
  if (nResult != 0) {
    vchi_service_release(m_hService);

    m_State = VCHIQSoundError;

    assert(0);
    return;
  }

  switch (Msg.type) {
  case VC_AUDIO_MSG_TYPE_RESULT:
    m_nResult = Msg.u.result.success;
    m_Event.Set();
    break;

  case VC_AUDIO_MSG_TYPE_COMPLETE:
    if (m_State == VCHIQSoundIdle) {
      break;
    }
    if (m_State == VCHIQSoundError) {
      assert(0);
      break;
    }
    assert(m_State >= VCHIQSoundRunning);

    if (Msg.u.complete.cookie1 != VC_AUDIO_WRITE_COOKIE1 ||
        Msg.u.complete.cookie2 != VC_AUDIO_WRITE_COOKIE2) {
      m_State = VCHIQSoundError;

      assert(0);
      break;
    }

    m_nCompletePos += Msg.u.complete.count & 0x3FFFFFFF;

    // Report how many bytes we have buffered
    AmountBufferedBytes(m_nWritePos - m_nCompletePos);

    // No more data left?
    if (m_nWritePos - m_nCompletePos == 0 && m_State == VCHIQSoundCancelled) {
      m_State = VCHIQSoundTerminating;
    }
    break;

  default:
    assert(0);
    break;
  }

  vchi_service_release(m_hService);
}

void ViceSoundBaseDevice::CallbackStub(void *pParam,
                                       const VCHI_CALLBACK_REASON_T Reason,
                                       void *hMessage) {
  ViceSoundBaseDevice *pThis = (ViceSoundBaseDevice *)pParam;
  assert(pThis != 0);

  pThis->Callback(Reason, hMessage);
}
