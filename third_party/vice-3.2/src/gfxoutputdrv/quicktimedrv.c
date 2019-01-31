/*
 * quicktimedrv.c - Movie driver using Apple QuickTime.
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_QUICKTIME

#include <QuickTime/QuickTime.h>
#include <CoreVideo/CVPixelBuffer.h>

#include "gfxoutput.h"
#include "screenshot.h"
#include "palette.h"
#include "lib.h"
#include "log.h"
#include "util.h"
#include "resources.h"
#include "cmdline.h"
#include "translate.h"
#include "../sounddrv/soundmovie.h"
#include "quicktimedrv.h"

#ifndef MAC_OS_X_VERSION_10_5
#define MAC_OS_X_VERSION_10_5 1050
#endif

#if (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5) && defined(__APPLE__)
// define missing pixel format in pre 10.5 headers
enum {
    kCVPixelFormatType_24RGB = 0x00000018
};
#endif

// ----- define formats -----------------------------------------------------

static gfxoutputdrv_codec_t mov_audio_codeclist[] = {
    { -1, "None" },
    { 1, "PCM" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t mov_video_codeclist[] = {
    { kPNGCodecType,        "PNG" },
    { kH264CodecType,       "H.264" },
    { kMotionJPEGACodecType,"Motion JPEG/A"},
    { kMotionJPEGBCodecType,"Motion JPEG/B"},
    { kVideoCodecType,      "Video"},
    { kAnimationCodecType,  "Animation"},
    { 0, NULL }
};

static gfxoutputdrv_format_t quicktimedrv_formatlist[] =
{
    { "mov", mov_audio_codeclist, mov_video_codeclist },
    { NULL, NULL, NULL }
};

// ----- global state -------------------------------------------------------

static Movie movie = NULL;
static DataHandler dataHandler = NULL;

// video
static Track videoTrack = NULL;
static Media videoMedia = NULL;
static ICMCompressionSessionRef videoCompressionSession = NULL;
static CVPixelBufferRef pixelBuffer = NULL;
static int video_width, video_height, video_xoff, video_yoff;
static int video_ready = 0;

// audio
static SoundDescriptionHandle soundDescriptionHandle = NULL;
static Track audioTrack = NULL;
static Media audioMedia = NULL;
static int audio_ready = 0;
static soundmovie_buffer_t audioBuffer = {
    NULL, 0, 0
};

static TimeScale timeScale = 1000000;
static TimeValue64 timestamp = 0;
static TimeValue64 divider = 0;

// ----- resources & command line -------------------------------------------

static char *quicktime_format = NULL;
static int audio_bitrate;
static int video_bitrate;
static int audio_codec;
static int video_codec;

static int set_format(const char *val, void *param)
{
    int i;

    util_string_set(&quicktime_format, val);
    for (i = 0; quicktimedrv_formatlist[i].name != NULL; i++) {
        if (strcmp(quicktime_format, quicktimedrv_formatlist[i].name) == 0) {
            return 0;
        }
    }
    return -1;
}

static int set_audio_bitrate(int val, void *param)
{
    audio_bitrate = (CLOCK)val;
    if (audio_bitrate < QUICKTIME_AUDIO_BITRATE_MIN || audio_bitrate > QUICKTIME_AUDIO_BITRATE_MAX) {
        audio_bitrate = QUICKTIME_AUDIO_BITRATE_DEF;
    }

    return 0;
}

static int set_video_bitrate(int val, void *param)
{
    video_bitrate = (CLOCK)val;
    if (video_bitrate < QUICKTIME_VIDEO_BITRATE_MIN || video_bitrate > QUICKTIME_VIDEO_BITRATE_MAX) {
        video_bitrate = QUICKTIME_VIDEO_BITRATE_DEF;
    }

    return 0;
}

static int set_audio_codec(int val, void *param)
{
    audio_codec = val;
    return 0;
}

static int set_video_codec(int val, void *param)
{
    video_codec = val;
    return 0;
}

static const resource_string_t resources_string[] = {
    { "QuickTimeFormat", "mov", RES_EVENT_NO, NULL,
      &quicktime_format, set_format, NULL },
    RESOURCE_STRING_LIST_END
};

static const resource_int_t resources_int[] = {
    { "QuickTimeAudioBitrate", QUICKTIME_AUDIO_BITRATE_DEF, RES_EVENT_NO, NULL,
      &audio_bitrate, set_audio_bitrate, NULL },
    { "QuickTimeVideoBitrate", QUICKTIME_VIDEO_BITRATE_DEF, RES_EVENT_NO, NULL,
      &video_bitrate, set_video_bitrate, NULL },
    { "QuickTimeAudioCodec", -1, RES_EVENT_NO, NULL,
      &audio_codec, set_audio_codec, NULL },
    { "QuickTimeVideoCodec", kPNGCodecType, RES_EVENT_NO, NULL,
      &video_codec, set_video_codec, NULL },
    RESOURCE_INT_LIST_END
};

static int quicktimedrv_resources_init(void)
{
    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    return resources_register_int(resources_int);
}

static const cmdline_option_t cmdline_options[] = {
    { "-quicktimeaudiobitrate", SET_RESOURCE, 1,
      NULL, NULL, "QuickTimeAudioBitrate", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_VALUE, IDCLS_SET_AUDIO_STREAM_BITRATE,
      NULL, NULL },
    { "-quicktimevideobitrate", SET_RESOURCE, 1,
      NULL, NULL, "QuickTimeVideoBitrate", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_VALUE, IDCLS_SET_VIDEO_STREAM_BITRATE,
      NULL, NULL },
    CMDLINE_LIST_END
};

static int quicktimedrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

// ----- video --------------------------------------------------------------

static OSStatus FrameOutputCallback(void* encodedFrameOutputRefCon,
                                    ICMCompressionSessionRef session, OSStatus error, ICMEncodedFrameRef frame,
                                    void* reserved)
{
    if (error) {
        log_debug("quicktime_video: error encoding frame!");
    } else {
        if (ICMEncodedFrameGetDecodeDuration(frame) > 0) {
            //  Adds sample data and description from an encoded frame to a media.
            OSErr theError = AddMediaSampleFromEncodedFrame(videoMedia, frame, NULL);
            if (theError) {
                log_debug("quicktime_video: error adding media sample!");
            }
        }
    }
    return error;
}

static OSStatus setup_video(void)
{
    //Add video track
    videoTrack = NewMovieTrack(movie, video_width << 16, video_height << 16, 0);
    OSStatus theError = GetMoviesError();
    if (theError) {
        log_debug("quicktime_video: error creating movie track");
        return theError;
    }

    //Create video track media
    videoMedia = NewTrackMedia(videoTrack, VideoMediaType, timeScale, 0, 0);
    theError = GetMoviesError();
    if (theError) {
        log_debug("quicktime_video: error creating track media!");
        return theError;
    }

    //Prepare media for editing
    theError = BeginMediaEdits(videoMedia);
    if (theError) {
        log_debug("quicktime_video: error beginning media edits!");
        return theError;
    }

    // ----- Setup Codec -----
    CodecType codec = (CodecType)video_codec;

    // Create compression session
    ICMEncodedFrameOutputRecord record = {
        FrameOutputCallback, NULL, NULL
    };
    theError = ICMCompressionSessionCreate(kCFAllocatorDefault,
                                           video_width, video_height, codec, timeScale, NULL /*options*/, NULL,
                                           &record, &videoCompressionSession);
    if (theError) {
        log_debug("quicktime_video: error creating compression session!");
        return theError;
    }

    // ----- PixelBuffer -----
    theError = CVPixelBufferCreate(NULL, video_width, video_height,
                                   kCVPixelFormatType_24RGB, NULL, &pixelBuffer);
    if (theError) {
        log_debug("quicktime_video: error creating pixel buffer!");
        return theError;
    }
    CVPixelBufferRetain(pixelBuffer);

    video_ready = 1;
    return noErr;
}

static OSStatus finish_video(void)
{
    video_ready = 0;

    // ----- PixelBuffer -----
    CVPixelBufferRelease(pixelBuffer);

    // ----- Codec -----

    OSErr theError = ICMCompressionSessionCompleteFrames(videoCompressionSession, true, 0, 0);
    if (theError) {
        log_debug("quicktime_video: error completing frames!");
    }

    ICMCompressionSessionRelease(videoCompressionSession);

    // ----- Movie -----

    //End media editing
    theError = EndMediaEdits(videoMedia);
    if (theError) {
        log_debug("quicktime_video: error ending media edits");
    }

    theError = ExtendMediaDecodeDurationToDisplayEndTime(videoMedia, NULL);
    if (theError) {
        log_debug("quicktime_video: error setting decode duration!");
    }

    //Add media to track
    theError = InsertMediaIntoTrack(videoTrack, 0, 0, GetMediaDisplayDuration(videoMedia), fixed1);
    if (theError) {
        log_debug("quicktime_video: error inserting media into track!");
    }

    videoTrack = NULL;
    videoMedia = NULL;
    return theError;
}

// ----- audio --------------------------------------------------------------

static int init_audio(int speed, int channels, soundmovie_buffer_t **buffer)
{
    AudioStreamBasicDescription asbd = {0}; //see CoreAudioTypes.h

    asbd.mSampleRate = speed;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsSignedInteger;
    asbd.mChannelsPerFrame = channels;
    asbd.mBitsPerChannel = sizeof (int16_t) * 8;
    asbd.mBytesPerFrame = (asbd.mBitsPerChannel >> 3)      // number of *bytes* per channel
                          * asbd.mChannelsPerFrame;         // channels per frame
    asbd.mFramesPerPacket = 1;      // For PCM, frames per packet is always 1
    asbd.mBytesPerPacket = asbd.mBytesPerFrame * asbd.mFramesPerPacket;

    UInt32 layoutSize;
    layoutSize = offsetof(AudioChannelLayout, mChannelDescriptions[0]);
    AudioChannelLayout *layout = NULL;
    layout = lib_calloc(layoutSize, 1);
    OSErr err = -1;
    if (layout != NULL) {
        if (channels == 1) {
            layout->mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
        } else if (channels == 2) {
            layout->mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
        } else {
            log_debug("quicktime_audio: unsupported channels: %d", channels);
            return -1;
        }
        err = QTSoundDescriptionCreate(
            &asbd,                      // format description
            layout, layoutSize,         // channel layout
            NULL, 0,                    // magic cookie (compression parameters)
            kQTSoundDescriptionKind_Movie_LowestPossibleVersion,
            &soundDescriptionHandle);         // SoundDescriptionHandle returned here
        lib_free(layout);
    }
    if (err != noErr) {
        log_debug("quicktime_audio: error creating sound description!");
        return -1;
    }

    //Add audio track
    audioTrack = NewMovieTrack(movie, 0, 0, kFullVolume);
    OSStatus theError = GetMoviesError();
    if (theError) {
        log_debug("quicktime_audio: error creating movie track");
        return theError;
    }

    //Create audio track media
    audioMedia = NewTrackMedia(audioTrack, SoundMediaType, speed, 0, 0);
    theError = GetMoviesError();
    if (theError) {
        log_debug("quicktime_audio: error creating track media!");
        return theError;
    }

    //Prepare media for editing
    theError = BeginMediaEdits(audioMedia);
    if (theError) {
        log_debug("quicktime_audio: error beginning media edits!");
        return theError;
    }

    *buffer = &audioBuffer;
    audioBuffer.size = speed * channels / 10;
    audioBuffer.buffer = lib_malloc(sizeof(int16_t) * audioBuffer.size);
    audioBuffer.used = 0;

    audio_ready = 1;
    return 0;
}

int encode_audio(soundmovie_buffer_t *buffer)
{
    if (!audio_ready) {
        return 0;
    }

    OSStatus err = AddMediaSample2 (audioMedia,
                                    (const UInt8 *)buffer->buffer,
                                    buffer->used * sizeof(int16_t),
                                    1,
                                    0,
                                    (SampleDescriptionHandle)soundDescriptionHandle,
                                    buffer->used,
                                    0,
                                    NULL);
    if (err != noErr) {
        log_debug("quicktime_audio: error adding samples!");
    }
    return 0;
}

void finish_audio(void)
{
    OSStatus theError;

    // flush buffer
    if (audioBuffer.used > 0) {
        encode_audio(&audioBuffer);
    }

    //End media editing
    theError = EndMediaEdits(audioMedia);
    if (theError) {
        log_debug("quicktime_audio: error ending media edits");
    }

    theError = ExtendMediaDecodeDurationToDisplayEndTime(audioMedia, NULL);
    if (theError) {
        log_debug("quicktime_audio: error setting decode duration!");
    }

    //Add media to track
    theError = InsertMediaIntoTrack(audioTrack, 0, 0, GetMediaDisplayDuration(audioMedia), fixed1);
    if (theError) {
        log_debug("quicktime_audio: error inserting media into track!");
    }

    audioTrack = NULL;
    audioMedia = NULL;

    DisposeHandle((Handle)soundDescriptionHandle);

    // free buffer
    if (audioBuffer.buffer != NULL) {
        lib_free(audioBuffer.buffer);
        audioBuffer.buffer = NULL;
    }

    audio_ready = 0;
}

static soundmovie_funcs_t quicktime_soundmovie_funcs = {
    init_audio,
    encode_audio,
    NULL
};

// ----- gfxoutputdrv interface ---------------------------------------------

static int quicktimedrv_open(screenshot_t *screenshot, const char *filename)
{
    /* UNUSED */
    return 0;
}

static int quicktimedrv_save(screenshot_t *screenshot, const char *filename)
{
    // align and center video
    video_width = screenshot->width;
    video_height = screenshot->height;
    video_width = (video_width + 15) & ~15;
    video_height = (video_height + 15) & ~15;
    video_xoff = (video_width - screenshot->width) >> 1;
    video_yoff = (video_height - screenshot->height) >> 1;

    // create cfstring from filename
    CFStringRef path = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    if (path == NULL) {
        log_debug("quicktime: error creating CFString!");
        return -1;
    }

    // create data reference
    Handle dataRef;
    OSType dataRefType;
    OSErr theError = QTNewDataReferenceFromFullPathCFString(
        path, kQTNativeDefaultPathStyle, 0, &dataRef, &dataRefType);
    if (theError) {
        log_debug("quicktime: error creating data reference for '%s'", filename);
        return -1;
    }

    // Create a movie for this file (data ref)
    theError = CreateMovieStorage(
        dataRef, dataRefType, 'TVOD', smCurrentScript, createMovieFileDeleteCurFile,
        &dataHandler, &movie);
    if (theError) {
        log_debug("quicktime: error creating movie storage for '%s'", filename);
        return -1;
    }

    // dispose of the data reference handle - we no longer need it
    DisposeHandle(dataRef);

    // define time scale and host clock divider
    divider = (TimeScale)CVGetHostClockFrequency() / timeScale;

    // setup video
    if (setup_video() != noErr) {
        return -1;
    }

    // setup audio
    if (audio_codec != -1) {
        soundmovie_start(&quicktime_soundmovie_funcs);
    }

    // set initial time stamp
    timestamp = CVGetCurrentHostTime() / divider;
    return 0;
}

static int quicktimedrv_record(screenshot_t *screenshot)
{
    if (!video_ready) {
        return 0;
    }

    OSErr theError;

    // lock buffer
    theError = CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    if (theError) {
        log_debug("quicktime: error locking pixel buffer!");
        return -1;
    }

    // fill frame
    unsigned char *buffer = (unsigned char *)CVPixelBufferGetBaseAddress(pixelBuffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);

    unsigned int line_size = screenshot->draw_buffer_line_size;
    int h = screenshot->height;
    int w = screenshot->width;
    int xoff = screenshot->x_offset;
    int yoff = screenshot->y_offset;
    uint8_t *srcBuffer = screenshot->draw_buffer;

    // move to last line in tgt buffer and to first in source
    buffer += (video_yoff) * bytesPerRow + video_xoff * 3;
    srcBuffer += yoff * line_size + xoff;

    int x, y;
    for (y = 0; y < h; y++) {
        int pix = 0;
        for (x = 0; x < w; x++) {
            uint8_t val = srcBuffer[x];
            buffer[pix++] = screenshot->palette->entries[val].red;
            buffer[pix++] = screenshot->palette->entries[val].green;
            buffer[pix++] = screenshot->palette->entries[val].blue;
        }
        buffer += bytesPerRow;
        srcBuffer += line_size;
    }

    // unlock buffer
    theError = CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    if (theError) {
        log_debug("quicktime: error unlocking pixel buffer!");
        return -1;
    }

    TimeValue64 next = CVGetCurrentHostTime() / divider;
    TimeValue64 duration = next - timestamp;
    timestamp = next;

    // encode frame
    theError = ICMCompressionSessionEncodeFrame(videoCompressionSession,
                                                pixelBuffer,
                                                timestamp, duration,
                                                kICMValidTime_DisplayTimeStampIsValid |
                                                kICMValidTime_DisplayDurationIsValid,
                                                NULL, NULL, (void *)NULL);
    if (theError) {
        log_debug("quicktime: error encoding frame!");
        return -1;
    }

    return 0;
}

static int quicktimedrv_close(screenshot_t *screenshot)
{
    OSStatus theError;

    finish_video();

    if (audio_codec != -1) {
        soundmovie_stop();
        finish_audio();
    }

    //Write movie
    theError = AddMovieToStorage(movie, dataHandler);
    if (theError) {
        log_debug("quicktime: error adding movie to storage!");
    }

    //Close movie file
    if (dataHandler) {
        CloseMovieStorage(dataHandler);
    }
    if (movie) {
        DisposeMovie(movie);
    }

    movie = NULL;
    dataHandler = NULL;
    return 0;
}

static int quicktimedrv_write(screenshot_t *screenshot)
{
    /* UNUSED */
    return 0;
}

static gfxoutputdrv_t quicktime_drv = {
    "QuickTime",
    "QuickTime",
    "mov",
    quicktimedrv_formatlist,
    quicktimedrv_open,
    quicktimedrv_close,
    quicktimedrv_write,
    quicktimedrv_save,
    NULL,
    quicktimedrv_record,
    NULL,
    quicktimedrv_resources_init,
    quicktimedrv_cmdline_options_init
#ifdef FEATURE_CPUMEMHISTORY
    , NULL
#endif
};

void gfxoutput_init_quicktime(int help)
{
    if (!help) {
        /* init quicktime */
        OSErr error = EnterMoviesOnThread(0);
        if (error != noErr) {
            log_debug("quicktime: error initializing!");
            return;
        }
    }

    gfxoutput_register(&quicktime_drv);
}
#endif
