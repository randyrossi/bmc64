/*
 * ffmpegdrv.c - Movie driver using FFMPEG library and screenshot API.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifdef HAVE_FFMPEG

#include <stdio.h>
#include <string.h>

#include "archdep.h"
#include "cmdline.h"
#include "ffmpegdrv.h"
#include "ffmpeglib.h"
#include "gfxoutput.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "palette.h"
#include "resources.h"
#include "screenshot.h"
#include "uiapi.h"
#include "util.h"
#include "soundmovie.h"

/** \brief  Helper macro to determine ffmpeg version
 */
#if (LIBAVCODEC_VERSION_MAJOR >= 58) && (LIBAVCODEC_VERSION_MINOR >= 18)
# define HAVE_FFMPEG4
#endif


static gfxoutputdrv_codec_t avi_audio_codeclist[] = {
    { AV_CODEC_ID_MP2, "MP2" },
    { AV_CODEC_ID_MP3, "MP3" },
    { AV_CODEC_ID_FLAC, "FLAC" },
    { AV_CODEC_ID_PCM_S16LE, "PCM uncompressed" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t mp4_audio_codeclist[] = {
    { AV_CODEC_ID_MP3, "MP3" },
    { AV_CODEC_ID_AAC, "AAC" },
    { AV_CODEC_ID_AC3, "AC3" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t avi_video_codeclist[] = {
    { AV_CODEC_ID_MPEG4, "MPEG4 (DivX)" },
    { AV_CODEC_ID_MPEG1VIDEO, "MPEG1" },
    { AV_CODEC_ID_FFV1, "FFV1 (lossless)" },
    { AV_CODEC_ID_H264, "H264" },
    { AV_CODEC_ID_THEORA, "Theora" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t mp4_video_codeclist[] = {
    { AV_CODEC_ID_H264, "H264" },
#ifdef AV_CODEC_ID_H265
    { AV_CODEC_ID_H265, "H265" },
#endif
    { 0, NULL }
};

static gfxoutputdrv_codec_t ogg_audio_codeclist[] = {
    { AV_CODEC_ID_FLAC, "FLAC" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t ogg_video_codeclist[] = {
    { AV_CODEC_ID_THEORA, "Theora" },
    { 0, NULL }
};

static gfxoutputdrv_codec_t none_codeclist[] = {
    { AV_CODEC_ID_NONE, "" },
    { 0, NULL }
};

/* formatlist is filled from with available formats and codecs at init time */
gfxoutputdrv_format_t *ffmpegdrv_formatlist = NULL;
gfxoutputdrv_format_t formats_to_test[] =
{
    { "avi", avi_audio_codeclist, avi_video_codeclist },
    { "mp4", mp4_audio_codeclist, mp4_video_codeclist },
    { "matroska", mp4_audio_codeclist, mp4_video_codeclist },
    { "ogg", ogg_audio_codeclist, ogg_video_codeclist },
    { "wav", NULL, NULL },
    { "mp3", NULL, none_codeclist }, /* formats expects png which fails in VICE */
    { "mp2", NULL, NULL },
    { NULL, NULL, NULL }
};

typedef struct OutputStream {
    AVStream *st;
    int64_t next_pts;
    int samples_count;
    AVFrame *frame;
    AVFrame *tmp_frame;
} OutputStream;

/* general */
static ffmpeglib_t ffmpeglib;
static AVFormatContext *ffmpegdrv_oc;
static AVOutputFormat *ffmpegdrv_fmt;
static int file_init_done;

/* audio */
static OutputStream audio_st = { 0 };
static AVCodec *avcodecaudio;
static soundmovie_buffer_t ffmpegdrv_audio_in;
static int audio_init_done;
static int audio_is_open;

#ifndef HAVE_FFMPEG_AVRESAMPLE
static struct SwrContext *swr_ctx;
#else
static struct AVAudioResampleContext *avr_ctx;
#endif

/* video */
static OutputStream video_st = { 0 };
static AVCodec *avcodecvideo;
static int video_init_done;
static int video_is_open;
static int video_width, video_height;
static unsigned int framecounter;
#ifdef HAVE_FFMPEG_SWSCALE
static struct SwsContext *sws_ctx;
#endif

/* resources */
static char *ffmpeg_format = NULL;
static int format_index;
static int audio_bitrate;
static int video_bitrate;
static int audio_codec;
static int video_codec;
static int video_halve_framerate;

static int ffmpegdrv_init_file(void);

static int set_container_format(const char *val, void *param)
{
    int i;

/* kludges to prevent crash at startup when using --help on the commandline */
#ifndef STATIC_FFMPEG
    if (ffmpegdrv_formatlist == NULL) {
        return 0;
    }
#endif

    format_index = -1;

    for (i = 0; ffmpegdrv_formatlist[i].name != NULL; i++) {
        if (strcmp(val, ffmpegdrv_formatlist[i].name) == 0) {
            format_index = i;
        }
    }

    if (format_index < 0) {
        return -1;
    }

    util_string_set(&ffmpeg_format, val);

    return 0;
}

static int set_audio_bitrate(int val, void *param)
{
    audio_bitrate = (CLOCK)val;

    if ((audio_bitrate < VICE_FFMPEG_AUDIO_RATE_MIN)
        || (audio_bitrate > VICE_FFMPEG_AUDIO_RATE_MAX)) {
        audio_bitrate = VICE_FFMPEG_AUDIO_RATE_DEFAULT;
    }
    return 0;
}

static int set_video_bitrate(int val, void *param)
{
    video_bitrate = (CLOCK)val;

    if ((video_bitrate < VICE_FFMPEG_VIDEO_RATE_MIN)
        || (video_bitrate > VICE_FFMPEG_VIDEO_RATE_MAX)) {
        video_bitrate = VICE_FFMPEG_VIDEO_RATE_DEFAULT;
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

static int set_video_halve_framerate(int value, void *param)
{
    int val = value ? 1 : 0;

    if (video_halve_framerate != val && screenshot_is_recording()) {
        ui_error("Can't change framerate while recording. Try again later.");
        return 0;
    }

    video_halve_framerate = val;

    return 0;
}

/*---------- Resources ------------------------------------------------*/

static const resource_string_t resources_string[] = {
    { "FFMPEGFormat", "avi", RES_EVENT_NO, NULL,
      &ffmpeg_format, set_container_format, NULL },
    RESOURCE_STRING_LIST_END
};

static const resource_int_t resources_int[] = {
    { "FFMPEGAudioBitrate", VICE_FFMPEG_AUDIO_RATE_DEFAULT,
      RES_EVENT_NO, NULL,
      &audio_bitrate, set_audio_bitrate, NULL },
    { "FFMPEGVideoBitrate", VICE_FFMPEG_VIDEO_RATE_DEFAULT,
      RES_EVENT_NO, NULL,
      &video_bitrate, set_video_bitrate, NULL },
    { "FFMPEGAudioCodec", AV_CODEC_ID_MP3, RES_EVENT_NO, NULL,
      &audio_codec, set_audio_codec, NULL },
    { "FFMPEGVideoCodec", AV_CODEC_ID_MPEG4, RES_EVENT_NO, NULL,
      &video_codec, set_video_codec, NULL },
    { "FFMPEGVideoHalveFramerate", 0, RES_EVENT_NO, NULL,
      &video_halve_framerate, set_video_halve_framerate, NULL },
    RESOURCE_INT_LIST_END
};

static int ffmpegdrv_resources_init(void)
{
    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    return resources_register_int(resources_int);
}

/*---------- Commandline options --------------------------------------*/

static const cmdline_option_t cmdline_options[] =
{
    { "-ffmpegaudiobitrate", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "FFMPEGAudioBitrate", NULL,
      "<value>", "Set bitrate for audio stream in media file" },
    { "-ffmpegvideobitrate", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "FFMPEGVideoBitrate", NULL,
      "<value>", "Set bitrate for video stream in media file" },
    CMDLINE_LIST_END
};

static int ffmpegdrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/*---------------------------------------------------------------------*/

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    /* rescale output packet timestamp values from codec to stream timebase */
    VICE_P_AV_PACKET_RESCALE_TS(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;

    /* Write the compressed frame to the media file. */
    return VICE_P_AV_INTERLEAVED_WRITE_FRAME(fmt_ctx, pkt);
}

static void close_stream(OutputStream *ost)
{
    VICE_P_AVCODEC_CLOSE(ost->st->codec);
    VICE_P_AV_FRAME_FREE(&ost->frame);
    VICE_P_AV_FRAME_FREE(&ost->tmp_frame);
}

/*-----------------------*/
/* audio stream encoding */
/*-----------------------*/

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, 
    uint64_t channel_layout,
    int sample_rate, int nb_samples)
{
    AVFrame *frame = VICE_P_AV_FRAME_ALLOC();
    int ret;

    if (!frame) {
        log_debug("ffmpegdrv: Error allocating an audio frame");
        return NULL;
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = VICE_P_AV_FRAME_GET_BUFFER(frame, 0);
        if (ret < 0) {
            log_debug("ffmpegdrv: Error allocating an audio buffer");
            return NULL;
        }
    }

    return frame;
}

static int ffmpegdrv_open_audio(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    int audio_inbuf_samples;
    int ret;
    AVDictionary *opts = NULL;

    c = st->codec;
    /* open codec */
    /* aac encoder ist flaged 'experimental' */
    VICE_P_AV_DICT_SET(&opts, "strict", "experimental", 0);
    ret = VICE_P_AVCODEC_OPEN2(c, avcodecaudio, &opts);
    VICE_P_AV_DICT_FREE(&opts);

    if (ret < 0) {
        log_debug("ffmpegdrv: could not open audio codec (%d)", ret);
        return -1;
    }

    audio_is_open = 1;
#ifdef HAVE_FFMPEG4
    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
#else
    if (c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) {
#endif
        audio_inbuf_samples = 10000;
    } else {
        audio_inbuf_samples = c->frame_size;
    }

    audio_st.frame = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, audio_inbuf_samples);
    audio_st.tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout, c->sample_rate, audio_inbuf_samples);

    if (!audio_st.frame || !audio_st.tmp_frame) {
        return -1;
    }

    ffmpegdrv_audio_in.size = audio_inbuf_samples * c->channels;
    ffmpegdrv_audio_in.buffer = (int16_t *)audio_st.tmp_frame->data[0];
    return 0;
}

static void ffmpegdrv_close_audio(void)
{
    if (audio_st.st == NULL) {
        return;
    }

    if (audio_is_open) {
        close_stream(&audio_st);
    }

    audio_is_open = 0;
    ffmpegdrv_audio_in.buffer = NULL;
    ffmpegdrv_audio_in.size = 0;
#ifndef HAVE_FFMPEG_AVRESAMPLE
    VICE_P_SWR_FREE(&swr_ctx);
#else
    VICE_P_AVRESAMPLE_FREE(&avr_ctx);
#endif
}

static int ffmpegmovie_init_audio(int speed, int channels, soundmovie_buffer_t ** audio_in)
{
    AVCodecContext *c;
    AVStream *st;
    int i;

    if (ffmpegdrv_oc == NULL || ffmpegdrv_fmt == NULL) {
        return -1;
    }

    audio_init_done = 1;

    if (ffmpegdrv_fmt->audio_codec == AV_CODEC_ID_NONE) {
        return -1;
    }

    *audio_in = &ffmpegdrv_audio_in;

    (*audio_in)->size = 0; /* not allocated yet */
    (*audio_in)->used = 0;

    st = VICE_P_AVFORMAT_NEW_STREAM(ffmpegdrv_oc, avcodecaudio);
    if (!st) {
        log_debug("ffmpegdrv: Could not alloc audio stream");
        return -1;
    }

    c = st->codec;

    /* put sample parameters */
    c->sample_fmt = avcodecaudio->sample_fmts ? avcodecaudio->sample_fmts[0] : AV_SAMPLE_FMT_S16;
    c->bit_rate = audio_bitrate;
    c->sample_rate = speed;

    if (avcodecaudio->supported_samplerates) {
        c->sample_rate = avcodecaudio->supported_samplerates[0];
        for (i = 0; avcodecaudio->supported_samplerates[i]; i++) {
            if (avcodecaudio->supported_samplerates[i] == speed)
                c->sample_rate = speed;
        }
    }
    c->channel_layout = (channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO);
    c->channels = VICE_P_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS(c->channel_layout);
 
#ifdef _MSC_VER
    st->time_base.num = 1;
	st->time_base.den = c->sample_rate;
#else
    st->time_base = (AVRational){ 1, c->sample_rate };
#endif
    audio_st.st = st;
    audio_st.next_pts = 0;
    audio_st.samples_count = 0;

    /* Some formats want stream headers to be separate. */
    if (ffmpegdrv_oc->oformat->flags & AVFMT_GLOBALHEADER) {
#ifdef HAVE_FFMPEG4
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
#else
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
#endif
    }

    /* create resampler context */
#ifndef HAVE_FFMPEG_AVRESAMPLE
    swr_ctx = VICE_P_SWR_ALLOC();
    if (!swr_ctx) {
#else
    avr_ctx = VICE_P_AVRESAMPLE_ALLOC_CONTEXT();
    if (!avr_ctx) {
#endif
        log_debug("ffmpegdrv: Could not alloc resampler context");
        return -1;
    }

    /* set options */
#ifndef HAVE_FFMPEG_AVRESAMPLE
    VICE_P_AV_OPT_SET_INT(swr_ctx, "in_channel_count", c->channels, 0);
    VICE_P_AV_OPT_SET_INT(swr_ctx, "in_sample_rate", speed, 0);
    VICE_P_AV_OPT_SET_SAMPLE_FMT(swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    VICE_P_AV_OPT_SET_INT(swr_ctx, "out_channel_count", c->channels, 0);
    VICE_P_AV_OPT_SET_INT(swr_ctx, "out_sample_rate", c->sample_rate, 0);
    VICE_P_AV_OPT_SET_SAMPLE_FMT(swr_ctx, "out_sample_fmt", c->sample_fmt, 0);
#else
    VICE_P_AV_OPT_SET_INT(avr_ctx, "in_channel_count", c->channels, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "in_channel_layout", c->channel_layout, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "in_sample_rate", speed, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "out_channel_count", c->channels, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "out_channel_layout", c->channel_layout, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "out_sample_rate", c->sample_rate, 0);
    VICE_P_AV_OPT_SET_INT(avr_ctx, "out_sample_fmt", c->sample_fmt, 0);
#endif

    /* initialize the resampling context */
#ifndef HAVE_FFMPEG_AVRESAMPLE
    if (VICE_P_SWR_INIT(swr_ctx) < 0) {
#else
    if (VICE_P_AVRESAMPLE_OPEN(avr_ctx) < 0) {
#endif
        log_debug("ffmpegdrv: Failed to initialize the resampling context");
        return -1;
    }

    if (video_init_done) {
        ffmpegdrv_init_file();
    }

    return 0;
}

/* triggered by soundffmpegaudio->write */
static int ffmpegmovie_encode_audio(soundmovie_buffer_t *audio_in)
{
    int got_packet;
    int dst_nb_samples;
    AVPacket pkt = { 0 };
    AVCodecContext *c;
    AVFrame *frame;
    int ret;

#ifdef _MSC_VER
    AVRational tmp;
#endif

    if (audio_st.st) {
        audio_st.frame->pts = audio_st.next_pts;
        audio_st.next_pts += audio_in->size;

        VICE_P_AV_INIT_PACKET(&pkt);
        c = audio_st.st->codec;

        frame = audio_st.tmp_frame;

        if (frame) {
            /* convert samples from native format to destination codec format, using the resampler */
            /* compute destination number of samples */
#ifndef HAVE_FFMPEG_AVRESAMPLE
            dst_nb_samples = (int)VICE_P_AV_RESCALE_RND(VICE_P_SWR_GET_DELAY(swr_ctx, c->sample_rate) + frame->nb_samples, c->sample_rate, c->sample_rate, AV_ROUND_UP);
#else
            dst_nb_samples = (int)VICE_P_AV_RESCALE_RND(VICE_P_AVRESAMPLE_GET_DELAY(avr_ctx, c->sample_rate) + frame->nb_samples, c->sample_rate, c->sample_rate, AV_ROUND_UP);
#endif

            /* when we pass a frame to the encoder, it may keep a reference to it
            * internally;
            * make sure we do not overwrite it here
            */
            ret = VICE_P_AV_FRAME_MAKE_WRITABLE(audio_st.frame);
            if (ret < 0)
                return -1;

            /* convert to destination format */
#ifndef HAVE_FFMPEG_AVRESAMPLE
            ret = VICE_P_SWR_CONVERT(swr_ctx, audio_st.frame->data, dst_nb_samples, (const uint8_t **)frame->data, frame->nb_samples);
#else
            ret = VICE_P_AVRESAMPLE_CONVERT(avr_ctx, audio_st.frame->data, 0, dst_nb_samples, (const uint8_t **)frame->data, 0, frame->nb_samples);
#endif
            if (ret < 0) {
                log_debug("ffmpegdrv_encode_audio: Error while converting audio frame");
                return -1;
            }
            frame = audio_st.frame;
#ifdef _MSC_VER
            tmp.num = 1;
            tmp.den = c->sample_rate;
            frame->pts = VICE_P_AV_RESCALE_Q(audio_st.samples_count, tmp, c->time_base);
#else
            frame->pts = VICE_P_AV_RESCALE_Q(audio_st.samples_count, (AVRational){ 1, c->sample_rate }, c->time_base);
#endif
            audio_st.samples_count += dst_nb_samples;
        }

        ret = VICE_P_AVCODEC_ENCODE_AUDIO2(audio_st.st->codec, &pkt, audio_st.frame, &got_packet);
        if (got_packet) {
            if (write_frame(ffmpegdrv_oc, &c->time_base, audio_st.st, &pkt)<0)
            {
                log_debug("ffmpegdrv_encode_audio: Error while writing audio frame");
            }
        }
    }

    audio_in->used = 0;
    return 0;
}

static void ffmpegmovie_close(void)
{
    /* just stop the whole recording */
    screenshot_stop_recording();
}

static soundmovie_funcs_t ffmpegdrv_soundmovie_funcs = {
    ffmpegmovie_init_audio,
    ffmpegmovie_encode_audio,
    ffmpegmovie_close
};

/*-----------------------*/
/* video stream encoding */
/*-----------------------*/
static int ffmpegdrv_fill_rgb_image(screenshot_t *screenshot, AVFrame *pic)
{
    int x, y;
    int dx, dy;
    int colnum;
    int bufferoffset;
    int x_dim = screenshot->width;
    int y_dim = screenshot->height;
    int pix = 0;
    /* center the screenshot in the video */
    dx = (video_width - x_dim) / 2;
    dy = (video_height - y_dim) / 2;
    bufferoffset = screenshot->x_offset + (dx < 0 ? -dx : 0)
        + (screenshot->y_offset + (dy < 0 ? -dy : 0)) * screenshot->draw_buffer_line_size;

    for (y = 0; y < video_height; y++) {
        for (x = 0; x < video_width; x++) {
            colnum = screenshot->draw_buffer[bufferoffset + x];
            pic->data[0][pix + 3*x] = screenshot->palette->entries[colnum].red;
            pic->data[0][pix + 3*x + 1] = screenshot->palette->entries[colnum].green;
            pic->data[0][pix + 3*x + 2] = screenshot->palette->entries[colnum].blue;
        }
        bufferoffset += screenshot->draw_buffer_line_size;
        pix += pic->linesize[0];
    }

    return 0;
}

static AVFrame* ffmpegdrv_alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = VICE_P_AV_FRAME_ALLOC();
    if (!picture) {
        return NULL;
    }
    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    ret = VICE_P_AV_FRAME_GET_BUFFER(picture, 32);
    if (ret < 0) {
        log_debug("ffmpegdrv: Could not allocate frame data");
        return NULL;
    }

    return picture;
}

static int ffmpegdrv_open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodecContext *c;
    int ret;

    c = st->codec;

    /* open the codec */
    ret = VICE_P_AVCODEC_OPEN2(c, avcodecvideo, NULL);
    if (ret < 0) {
        log_debug("ffmpegdrv: could not open video codec");
        return -1;
    }

    video_is_open = 1;

    /* allocate the encoded raw picture */
    video_st.frame = ffmpegdrv_alloc_picture(c->pix_fmt, c->width, c->height);
    if (!video_st.frame) {
        log_debug("ffmpegdrv: could not allocate picture");
        return -1;
    }

    /* if the output format is not RGB24, then a temporary RGB24
       picture is needed too. It is then converted to the required
       output format */
    video_st.tmp_frame = NULL;
    if (c->pix_fmt != VICE_AV_PIX_FMT_RGB24) {
        video_st.tmp_frame = ffmpegdrv_alloc_picture(VICE_AV_PIX_FMT_RGB24, c->width, c->height);
        if (!video_st.tmp_frame) {
            log_debug("ffmpegdrv: could not allocate temporary picture");
            return -1;
        }
    }
    return 0;
}

static void ffmpegdrv_close_video(void)
{
    if (video_st.st == NULL) {
        return;
    }

    if (video_is_open) {
        close_stream(&video_st);
    }

    video_is_open = 0;

    if (video_st.frame) {
        lib_free(video_st.frame->data[0]);
        lib_free(video_st.frame);
        video_st.frame = NULL;
    }
    if (video_st.tmp_frame) {
        lib_free(video_st.tmp_frame->data[0]);
        lib_free(video_st.tmp_frame);
        video_st.tmp_frame = NULL;
    }

    if (sws_ctx != NULL) {
        VICE_P_SWS_FREECONTEXT(sws_ctx);
    }
}

static void ffmpegdrv_init_video(screenshot_t *screenshot)
{
    AVCodecContext *c;
    AVStream *st;

    if (ffmpegdrv_oc == NULL || ffmpegdrv_fmt == NULL) {
        return;
    }

    video_init_done = 1;

    if (ffmpegdrv_fmt->video_codec == AV_CODEC_ID_NONE) {
        return;
    }

    st = VICE_P_AVFORMAT_NEW_STREAM(ffmpegdrv_oc, avcodecvideo);
    if (!st) {
        log_debug("ffmpegdrv: Could not alloc video stream\n");
        return;
    }

    c = st->codec;

    /* put sample parameters */
    c->bit_rate = video_bitrate;
    /* resolution should be a multiple of 16 */
    /* ffmpegdrv_fill_rgb_image only implements cutting so */
    /* adding black border was removed */
    video_width = c->width = screenshot->width & ~0xf;
    video_height = c->height = screenshot->height & ~0xf;
    /* frames per second */
    st->time_base = VICE_P_AV_D2Q(machine_get_cycles_per_frame() 
                                    / (double)(video_halve_framerate ? 
                                        machine_get_cycles_per_second() / 2 :
                                        machine_get_cycles_per_second()), 
                                  (1 << 16) - 1);
    c->time_base = st->time_base;

    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt = AV_PIX_FMT_YUV420P;

#if (LIBAVUTIL_VERSION_MICRO >= 100)
    /* Avoid format conversion which would lead to loss of quality */
    if (c->codec_id == AV_CODEC_ID_FFV1) {
        c->pix_fmt = AV_PIX_FMT_0RGB32;
    }
#endif

    /* Use XVID instead of FMP4 FOURCC for better compatibility */
    if (c->codec_id == AV_CODEC_ID_MPEG4) {
        c->codec_tag = MKTAG('X', 'V', 'I', 'D');
    }

    /* Allow nonstandard framerates for MPEG1 codec */
    if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    }

#ifdef HAVE_FFMPEG_SWSCALE
    /* setup scaler */
    if (c->pix_fmt != VICE_AV_PIX_FMT_RGB24) {
        sws_ctx = VICE_P_SWS_GETCONTEXT
                      (video_width, video_height, VICE_AV_PIX_FMT_RGB24,
                      video_width, video_height, c->pix_fmt,
                      SWS_BICUBIC,
                      NULL, NULL, NULL);
        if (sws_ctx == NULL) {
            log_debug("ffmpegdrv: Can't create Scaler!\n");
        }
    }
#endif

    video_st.st = st;
    video_st.next_pts = 0;
    framecounter = 0;

    /* Some formats want stream headers to be separate. */
    if (ffmpegdrv_oc->oformat->flags & AVFMT_GLOBALHEADER) {
#ifdef HAVE_FFMPEG4
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
#else
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
#endif
    }

    if (audio_init_done) {
        ffmpegdrv_init_file();
    }
}

static int ffmpegdrv_init_file(void)
{
    if (!video_init_done || !audio_init_done) {
        return 0;
    }

    VICE_P_AV_DUMP_FORMAT(ffmpegdrv_oc, 0, ffmpegdrv_oc->filename, 1);

    if (video_st.st && (ffmpegdrv_open_video(ffmpegdrv_oc, video_st.st) < 0)) {
        ui_error("ffmpegdrv: Cannot open video stream");
        screenshot_stop_recording();
        return -1;
    }
    if (audio_st.st && (ffmpegdrv_open_audio(ffmpegdrv_oc, audio_st.st) < 0)) {
        ui_error("ffmpegdrv: Cannot open audio stream");
        screenshot_stop_recording();
        return -1;
    }

    if (!(ffmpegdrv_fmt->flags & AVFMT_NOFILE)) {
        if (VICE_P_AVIO_OPEN(&ffmpegdrv_oc->pb, ffmpegdrv_oc->filename,
                            AVIO_FLAG_WRITE) < 0) {

            ui_error("ffmpegdrv: Cannot open %s", ffmpegdrv_oc->filename);
            screenshot_stop_recording();
            return -1;
        }
    }

    VICE_P_AVFORMAT_WRITE_HEADER(ffmpegdrv_oc,NULL);

    log_debug("ffmpegdrv: Initialized file successfully");

    file_init_done = 1;

    return 0;
}

static int ffmpegdrv_save(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_format_t *format;

    video_st.st = NULL;
    audio_st.st = NULL;

    audio_init_done = 0;
    video_init_done = 0;
    file_init_done = 0;

    if (format_index < 0) {
        return -1;
    }

    ffmpegdrv_fmt = VICE_P_AV_GUESS_FORMAT(ffmpeg_format, NULL, NULL);
    format = &ffmpegdrv_formatlist[format_index];

    if (format->audio_codecs != NULL) {
        /* the codec from resource */
        ffmpegdrv_fmt->audio_codec = audio_codec;
    }
    if (ffmpegdrv_fmt->audio_codec != AV_CODEC_ID_NONE) {
        avcodecaudio = VICE_P_AVCODEC_FIND_ENCODER(ffmpegdrv_fmt->audio_codec);
    }

    if (format->video_codecs != NULL) {
        /* the codec from resource */
        ffmpegdrv_fmt->video_codec = video_codec;
    }
    if (ffmpegdrv_fmt->video_codec != AV_CODEC_ID_NONE) {
        avcodecvideo = VICE_P_AVCODEC_FIND_ENCODER(ffmpegdrv_fmt->video_codec);
    }

    ffmpegdrv_oc = VICE_P_AVFORMAT_ALLOC_CONTEXT();

    if (!ffmpegdrv_oc) {
        log_debug("ffmpegdrv: Cannot allocate format context");
        return -1;
    }

    ffmpegdrv_oc->oformat = ffmpegdrv_fmt;
    snprintf(ffmpegdrv_oc->filename, sizeof(ffmpegdrv_oc->filename), "%s", filename);

    ffmpegdrv_init_video(screenshot);

    soundmovie_start(&ffmpegdrv_soundmovie_funcs);

    return 0;
}

static int ffmpegdrv_close(screenshot_t *screenshot)
{
    unsigned int i;

    /* write the trailer, if any */
    if (file_init_done) {
        VICE_P_AV_WRITE_TRAILER(ffmpegdrv_oc);
    }

    soundmovie_stop();

    if (video_st.st) {
        ffmpegdrv_close_video();
    }
    if (audio_st.st) {
        ffmpegdrv_close_audio();
    }

    if (!(ffmpegdrv_fmt->flags & AVFMT_NOFILE)) {
        /* close the output file */
            VICE_P_AVIO_CLOSE(ffmpegdrv_oc->pb);
    }

    /* free the streams */
    for (i = 0; i < ffmpegdrv_oc->nb_streams; i++) {
        VICE_P_AV_FREE((void *)ffmpegdrv_oc->streams[i]);
        ffmpegdrv_oc->streams[i] = NULL;
    }

    /* free the stream */
    log_debug("ffmpegdrv: Closed successfully");

    file_init_done = 0;

    return 0;
}

/* triggered by screenshot_record */
static int ffmpegdrv_record(screenshot_t *screenshot)
{
    AVCodecContext *c;
    int ret;

    if (audio_init_done && video_init_done && !file_init_done) {
        ffmpegdrv_init_file();
    }

    if (video_st.st == NULL || !file_init_done) {
        return 0;
    }

   if (audio_st.st && video_st.next_pts > audio_st.next_pts) {
        /* drop this frame */
        return 0;
    }

    framecounter++;
    if (video_halve_framerate && (framecounter & 1)) {
        /* drop every second frame */
        return 0;
    }

    c = video_st.st->codec;

    if (c->pix_fmt != VICE_AV_PIX_FMT_RGB24) {
        ffmpegdrv_fill_rgb_image(screenshot, video_st.tmp_frame);

        if (sws_ctx != NULL) {
            VICE_P_SWS_SCALE(sws_ctx,
#if defined(STATIC_FFMPEG) || defined(SHARED_FFMPEG)
                (const uint8_t * const *)video_st.tmp_frame->data,
#else
                video_st.tmp_frame->data,
#endif
                video_st.tmp_frame->linesize, 0, c->height,
                video_st.frame->data, video_st.frame->linesize);
        }
    } else {
        ffmpegdrv_fill_rgb_image(screenshot, video_st.frame);
    }

    video_st.frame->pts = video_st.next_pts++;

#ifdef AVFMT_RAWPICTURE
    if (ffmpegdrv_oc->oformat->flags & AVFMT_RAWPICTURE) {
        AVPacket pkt;
        VICE_P_AV_INIT_PACKET(&pkt);
        pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index = video_st.st->index;
        pkt.data = (uint8_t*)video_st.frame;
        pkt.size = sizeof(AVPicture);
        pkt.pts = pkt.dts = video_st.frame->pts;

        ret = VICE_P_AV_INTERLEAVED_WRITE_FRAME(ffmpegdrv_oc, &pkt);
    } else
#endif
    {
        AVPacket pkt = { 0 };
        int got_packet;

        VICE_P_AV_INIT_PACKET(&pkt);

        /* encode the image */
        ret = VICE_P_AVCODEC_ENCODE_VIDEO2(c, &pkt, video_st.frame, &got_packet);
        if (ret < 0) {
            log_debug("Error while encoding video frame");
            return -1;
        }
        /* if zero size, it means the image was buffered */
        if (got_packet) {
            if (write_frame(ffmpegdrv_oc, &c->time_base, video_st.st, &pkt)<0)
            {
                log_debug("ffmpegdrv_encode_audio: Error while writing audio frame");
            }
        } else {
            ret = 0;
        }
    }
    if (ret < 0) {
        log_debug("Error while writing video frame");
        return -1;
    }

    return 0;
}

static int ffmpegdrv_write(screenshot_t *screenshot)
{
    return 0;
}


static void ffmpegdrv_shutdown(void);


static gfxoutputdrv_t ffmpeg_drv = {
    "FFMPEG",
    "FFMPEG",
    NULL,
    NULL, /* filled in ffmpeg_get_formats_and_codecs */
    NULL, /* open */
    ffmpegdrv_close,
    ffmpegdrv_write,
    ffmpegdrv_save,
    NULL,
    ffmpegdrv_record,
    ffmpegdrv_shutdown,
    ffmpegdrv_resources_init,
    ffmpegdrv_cmdline_options_init
#ifdef FEATURE_CPUMEMHISTORY
    , NULL
#endif
};


static void ffmpegdrv_shutdown(void)
{
    int i = 0;

    ffmpeglib_close(&ffmpeglib);

    while (ffmpeg_drv.formatlist[i].name != NULL) {
        lib_free(ffmpeg_drv.formatlist[i].name);
        if (ffmpeg_drv.formatlist[i].audio_codecs != NULL) {
            lib_free(ffmpeg_drv.formatlist[i].audio_codecs);
        }
        if (ffmpeg_drv.formatlist[i].video_codecs != NULL) {
            lib_free(ffmpeg_drv.formatlist[i].video_codecs);
        }
        i++;
    }
    lib_free(ffmpeg_drv.formatlist);

    lib_free(ffmpeg_format);
}

static void ffmpeg_get_formats_and_codecs(void)
{
    int i, j, ai = 0, vi = 0, f;
    gfxoutputdrv_codec_t *audio_codec_list;
    gfxoutputdrv_codec_t *video_codec_list;
    gfxoutputdrv_codec_t *ac, *vc;

    f = 0;
    ffmpegdrv_formatlist = lib_malloc(sizeof(gfxoutputdrv_format_t));

    for (i = 0; formats_to_test[i].name != NULL; i++) {
        if (VICE_P_AV_GUESS_FORMAT(formats_to_test[i].name, NULL, NULL)) {
            audio_codec_list = NULL;
            video_codec_list = NULL;
            if (formats_to_test[i].audio_codecs != NULL) {
                ai = 0;
                audio_codec_list = lib_malloc(sizeof(gfxoutputdrv_codec_t));
                ac = formats_to_test[i].audio_codecs;
                for (j = 0; ac[j].name != NULL; j++) {
                    if ((ac[j].id == AV_CODEC_ID_NONE) || VICE_P_AVCODEC_FIND_ENCODER(ac[j].id)) {
                        audio_codec_list[ai++] = ac[j];
                        audio_codec_list = lib_realloc(audio_codec_list, (ai + 1) * sizeof(gfxoutputdrv_codec_t));
                    }
                }
                audio_codec_list[ai].name = NULL;
            }
            if (formats_to_test[i].video_codecs != NULL) {
                vi = 0;
                video_codec_list = lib_malloc(sizeof(gfxoutputdrv_codec_t));
                vc = formats_to_test[i].video_codecs;
                for (j = 0; vc[j].name != NULL; j++) {
                    if (vc[j].id == AV_CODEC_ID_NONE || VICE_P_AVCODEC_FIND_ENCODER(vc[j].id)) {
                        video_codec_list[vi++] = formats_to_test[i].video_codecs[j];
                        video_codec_list = lib_realloc(video_codec_list, (vi + 1) * sizeof(gfxoutputdrv_codec_t));
                    }
                }
                video_codec_list[vi].name = NULL;
            }
            if (((audio_codec_list == NULL) || (ai > 0)) && ((video_codec_list == NULL) || (vi > 0))) {
                ffmpegdrv_formatlist[f].name = lib_stralloc(formats_to_test[i].name);
                ffmpegdrv_formatlist[f].audio_codecs = audio_codec_list;
                ffmpegdrv_formatlist[f++].video_codecs = video_codec_list;
                ffmpegdrv_formatlist = lib_realloc(ffmpegdrv_formatlist, (f + 1) * sizeof(gfxoutputdrv_format_t));
            }
        }
    }
    ffmpegdrv_formatlist[f].name = NULL;
    ffmpeg_drv.formatlist = ffmpegdrv_formatlist;
}

void gfxoutput_init_ffmpeg(int help)
{
#ifndef STATIC_FFMPEG
    if (help) {
        gfxoutput_register(&ffmpeg_drv);
        return;
    }
#endif

    if (ffmpeglib_open(&ffmpeglib) < 0) {
        return;
    }
    VICE_P_AV_REGISTER_ALL();
    ffmpeg_get_formats_and_codecs();
    gfxoutput_register(&ffmpeg_drv);
}
#endif
