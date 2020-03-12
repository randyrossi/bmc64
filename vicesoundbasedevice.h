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

#ifndef _vice_base_sound_device_h
#define _vice_base_sound_device_h

#include <stdint.h>
#include <circle/interrupt.h>
#include <circle/sched/synchronizationevent.h>
#include <circle/soundbasedevice.h>
#include <circle/types.h>
#include <vc4/sound/vc_vchi_audioserv_defs.h>
#include <vc4/vchi/vchi.h>
#include <vc4/vchiq/vchiqdevice.h>

#define VCHIQ_SOUND_VOLUME_MIN -10000
#define VCHIQ_SOUND_VOLUME_DEFAULT 0
#define VCHIQ_SOUND_VOLUME_MAX 400

enum TVCHIQSoundDestination {
  VCHIQSoundDestinationAuto,
  VCHIQSoundDestinationHeadphones,
  VCHIQSoundDestinationHDMI,
  VCHIQSoundDestinationUnknown
};

enum TVCHIQSoundState {
  VCHIQSoundCreated,
  VCHIQSoundIdle,
  VCHIQSoundRunning,
  VCHIQSoundCancelled,
  VCHIQSoundTerminating,
  VCHIQSoundError,
  VCHIQSoundUnknown
};

class ViceSoundBaseDevice
    : public CSoundBaseDevice /// Low level access to the VCHIQ sound service
{
public:
  /// \param pVCHIQDevice	pointer to the VCHIQ interface device
  /// \param nSampleRate	sample rate in Hz (44100..48000)
  /// \param nChunkSize	number of samples transfered at once
  /// \param Destination	the target device, the sound data is sent to\n
  ///			(detected automatically, if equal to
  ///VCHIQSoundDestinationAuto)
  ViceSoundBaseDevice(
      CVCHIQDevice *pVCHIQDevice, unsigned nSampleRate = 44100,
      unsigned nChunkSize = 4000,
      TVCHIQSoundDestination Destination = VCHIQSoundDestinationAuto);

  virtual ~ViceSoundBaseDevice(void);

  /// \return Minium value of one sample
  int GetRangeMin(void) const;
  /// \return Maximum value of one sample
  int GetRangeMax(void) const;

  /// Set volume before start if desired
  void SetVolume(int nVolume);

  /// Set channels before start
  void SetChannels(int nChannels);

  /// \brief Connects to the VCHIQ sound service and starts sending sound data
  /// \return Operation successful?
  boolean Start(void);

  /// \brief Stops the transmission of sound data
  /// \note Cancel takes effect after a short delay
  void Cancel(void);

  /// \return Is the sound data transmission running?
  boolean IsActive(void) const;

  /// \param nVolume	Output volume to be set (-10000..400)
  /// \param Destination	the target device, the sound data is sent to\n
  ///			(not modified, if equal to VCHIQSoundDestinationUnknown)
  /// \note This method can be called, while the sound data transmission is
  /// running.
  void
  SetControl(int nVolume,
             TVCHIQSoundDestination Destination = VCHIQSoundDestinationUnknown);

protected:
  /// \brief May overload this to provide the sound samples!
  /// \param pBuffer	buffer where the samples have to be placed
  /// \param nChunkSize	size of the buffer in s16 words
  /// \return Number of s16 words written to the buffer (normally nChunkSize),\n
  ///	    Transfer will stop if 0 is returned
  /// \note Each sample consists of two words (Left channel, right channel)\n
  ///	  Each word must be between GetRangeMin() and GetRangeMax()
  /// virtual unsigned GetChunk (s16 *pBuffer, unsigned nChunkSize);

private:
  int CallMessage(VC_AUDIO_MSG_T *pMessage);  // waits for completion
  int QueueMessage(VC_AUDIO_MSG_T *pMessage); // does not wait for completion

protected:
  int WriteChunk(void);
  virtual void AmountBufferedBytes(unsigned) = 0;

private:
  void Callback(const VCHI_CALLBACK_REASON_T Reason, void *hMessage);
  static void CallbackStub(void *pParam, const VCHI_CALLBACK_REASON_T Reason,
                           void *hMessage);

private:
  unsigned m_nSampleRate;
  unsigned m_nChunkSize;
  TVCHIQSoundDestination m_Destination;

  volatile TVCHIQSoundState m_State;

  VCHI_INSTANCE_T m_VCHIInstance;
  VCHI_SERVICE_HANDLE_T m_hService;

  CSynchronizationEvent m_Event;
  int m_nResult;

  unsigned m_nWritePos;
  unsigned m_nCompletePos;
  s16 *p_buffer;
  int m_nVolume;
  int m_nChannels;
};

#endif
