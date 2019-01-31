/*
 * ffmpeglib.h - Interface to access the ffmpeg libs.
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

#ifndef VICE_FFMPEGLIB_H
#define VICE_FFMPEGLIB_H

#include "vice.h"

#include "archdep.h"

#ifdef HAVE_FFMPEG_HEADER_SUBDIRS
#  include "libavutil/rational.h"
#  include "libavformat/avformat.h"
#  include "libswscale/swscale.h"
#ifndef HAVE_FFMPEG_AVRESAMPLE
#  include "libswresample/swresample.h"
#else
#  include "libavresample/avresample.h"
#endif
#else
#  include "rational.h"
#  include "avformat.h"
#  include "swscale.h"
#ifndef HAVE_FFMPEG_AVRESAMPLE
#  include "swresample.h"
#else
#  include "avresample.h"
#endif
#endif

/* "libavutil/opt.h" */
int av_opt_set_int(void*, const char*, int64_t, int);
int av_opt_set_sample_fmt(void*, const char*, enum AVSampleFormat, int);

/* generic version function */
typedef unsigned (*ffmpeg_version_t)(void);

/* Use new names */
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,18,102)
#define AV_CODEC_ID_MP2        CODEC_ID_MP2
#define AV_CODEC_ID_MP3        CODEC_ID_MP3
#define AV_CODEC_ID_FLAC       CODEC_ID_FLAC
#define AV_CODEC_ID_PCM_S16LE  CODEC_ID_PCM_S16LE
#define AV_CODEC_ID_PCM_S16BE  CODEC_ID_PCM_S16BE
#define AV_CODEC_ID_PCM_U16LE  CODEC_ID_PCM_U16LE
#define AV_CODEC_ID_PCM_U16BE  CODEC_ID_PCM_U16BE
#define AV_CODEC_ID_MPEG4      CODEC_ID_MPEG4
#define AV_CODEC_ID_MPEG1VIDEO CODEC_ID_MPEG1VIDEO
#define AV_CODEC_ID_FFV1       CODEC_ID_FFV1
#define AV_CODEC_ID_H264       CODEC_ID_H264
#define AV_CODEC_ID_THEORA     CODEC_ID_THEORA
#define AV_CODEC_ID_NONE       CODEC_ID_NONE
#define AVCodecID              CodecID
#endif

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(55,17,103)
#define VICE_AV_PIX_FMT_RGB24 PIX_FMT_RGB24
#define VICE_AV_PixelFormat PixelFormat
#else
#define VICE_AV_PIX_FMT_RGB24 AV_PIX_FMT_RGB24
#define VICE_AV_PixelFormat AVPixelFormat
#endif

/* avcodec fucntions */
typedef void(*av_init_packet_t)(AVPacket *pkt);
typedef int(*avcodec_open2_t)(AVCodecContext*, AVCodec*, AVDictionary **);
typedef int (*avcodec_close_t)(AVCodecContext*);
typedef AVCodec* (*avcodec_find_encoder_t)(enum AVCodecID);
typedef int(*avcodec_encode_audio2_t)(AVCodecContext*, AVPacket*, const AVFrame*, int*);
typedef int(*avcodec_encode_video2_t)(AVCodecContext*, AVPacket*, const AVFrame*, int*);
typedef int (*avpicture_fill_t)(AVPicture*, uint8_t*, int, int, int);
typedef int (*avpicture_get_size_t)(int, int, int);
typedef void(*av_packet_rescale_ts_t)(AVPacket*, AVRational, AVRational);

/* avformat functions */
typedef void (*av_register_all_t)(void);
typedef AVFormatContext* (*avformat_alloc_context_t)(void);
typedef AVStream* (*avformat_new_stream_t)(AVFormatContext*, AVCodec *);
typedef int (*avformat_write_header_t) (AVFormatContext*,AVDictionary **);
typedef int (*av_interleaved_write_frame_t)(AVFormatContext*, AVPacket*);
typedef int (*av_write_trailer_t)(AVFormatContext*);
typedef int (*avio_open_t) (AVIOContext**, const char*, int);
typedef int (*avio_close_t) (AVIOContext*);
typedef void (*av_dump_format_t) (AVFormatContext *, int, const char*, int);
typedef AVOutputFormat* (*av_guess_format_t)(const char*, const char*, const char*);
typedef int (*img_convert_t)(AVPicture*, int, AVPicture*, int, int, int);

/* avutil functions */
typedef void (*av_free_t)(void**);
typedef AVFrame* (*av_frame_alloc_t)(void);
typedef int (*av_frame_get_buffer_t)(AVFrame*, int);
typedef int(*av_compare_ts_t)(int64_t, AVRational, int64_t, AVRational);
typedef int(*av_get_channel_layout_nb_channels_t)(uint64_t);
typedef int(*av_opt_set_int_t)(void*, const char*, int64_t, int);
typedef int(*av_opt_set_sample_fmt_t)(void*, const char*, enum AVSampleFormat, int);
typedef int64_t(*av_rescale_rnd_t)(int64_t, int64_t, int64_t, enum AVRounding);
typedef int64_t(*av_rescale_q_t)(int64_t, AVRational, AVRational);
typedef AVRational(*av_d2q_t)(double d, int max);
typedef int(*av_frame_make_writable_t)(AVFrame*);
typedef void(*av_frame_free_t)(AVFrame**);
typedef int(*av_dict_set_t)(AVDictionary**, const char*, const char*, int);
typedef void(*av_dict_free_t)(AVDictionary**);

/* swscale functions */
typedef struct SwsContext * (*sws_getContext_t)(int srcW, int srcH,
                                                enum VICE_AV_PixelFormat srcFormat, int dstW, int dstH, enum VICE_AV_PixelFormat dstFormat,
                                                int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, double *param);
typedef void (*sws_freeContext_t)(struct SwsContext *swsContext);
typedef int (*sws_scale_t)(struct SwsContext *context, uint8_t* srcSlice[],
                           int srcStride[], int srcSliceY, int srcSliceH, uint8_t* dst[],
                           int dstStride[]);

#ifndef HAVE_FFMPEG_AVRESAMPLE
/* swresample functions */
typedef struct SwrContext* (*swr_alloc_t)(void);
typedef int (*swr_init_t)(struct SwrContext*);
typedef int (*swr_convert_t)(struct SwrContext*, uint8_t**, int, const uint8_t**, int);
typedef void (*swr_free_t)(struct SwrContext**);
typedef int64_t (*swr_get_delay_t)(struct SwrContext*, int64_t);
#else
/* avresample functions */
typedef struct AVAudioResampleContext* (*avresample_alloc_context_t)(void);
typedef int (*avresample_convert_t)(struct AVAudioResampleContext*, uint8_t**, int, int, const uint8_t**, int, int);
typedef int (*avresample_open_t)(struct AVAudioResampleContext*);
typedef void (*avresample_free_t)(struct AVAudioResampleContext**);
typedef int64_t (*avresample_get_delay_t)(struct AVAudioResampleContext*, int64_t);
#endif

struct ffmpeglib_s {
    /* avcodec */
    avcodec_open2_t              p_avcodec_open2;
    avcodec_close_t              p_avcodec_close;
    avcodec_find_encoder_t       p_avcodec_find_encoder;
    avcodec_encode_audio2_t      p_avcodec_encode_audio2;
    avcodec_encode_video2_t      p_avcodec_encode_video2;
    avpicture_fill_t             p_avpicture_fill;
    avpicture_get_size_t         p_avpicture_get_size;
    av_packet_rescale_ts_t       p_av_packet_rescale_ts;

    /* avformat */
    av_init_packet_t             p_av_init_packet;
    av_register_all_t            p_av_register_all;
    avformat_alloc_context_t     p_avformat_alloc_context;
    avformat_new_stream_t        p_avformat_new_stream;
    avformat_write_header_t      p_avformat_write_header;
    av_interleaved_write_frame_t p_av_interleaved_write_frame;
    av_write_trailer_t           p_av_write_trailer;
    avio_open_t                  p_avio_open;
    avio_close_t                 p_avio_close;
    av_dump_format_t             p_av_dump_format;
    av_guess_format_t            p_av_guess_format;

    /* avutil */
    av_free_t                    p_av_free;
    av_frame_alloc_t             p_av_frame_alloc;
    av_frame_get_buffer_t        p_av_frame_get_buffer;
    av_compare_ts_t              p_av_compare_ts;
    av_get_channel_layout_nb_channels_t p_av_get_channel_layout_nb_channels;
    av_opt_set_int_t             p_av_opt_set_int;
    av_opt_set_sample_fmt_t      p_av_opt_set_sample_fmt;
    av_rescale_rnd_t             p_av_rescale_rnd;
    av_rescale_q_t               p_av_rescale_q;
    av_d2q_t                     p_av_d2q;
    av_frame_make_writable_t     p_av_frame_make_writable;
    av_frame_free_t              p_av_frame_free;
    av_dict_set_t                p_av_dict_set;
    av_dict_free_t               p_av_dict_free;

    /* swscale */
    sws_getContext_t             p_sws_getContext;
    sws_freeContext_t            p_sws_freeContext;
    sws_scale_t                  p_sws_scale;

#ifndef HAVE_FFMPEG_AVRESAMPLE
    /* swresample */
    swr_alloc_t                  p_swr_alloc;
    swr_init_t                   p_swr_init;
    swr_convert_t                p_swr_convert;
    swr_get_delay_t              p_swr_get_delay;
    swr_free_t                   p_swr_free;
#else
    /* avresample */
    avresample_alloc_context_t   p_avresample_alloc_context;
    avresample_open_t            p_avresample_open;
    avresample_convert_t         p_avresample_convert;
    avresample_free_t            p_avresample_free;
    avresample_get_delay_t       p_avresample_get_delay;
#endif
};

typedef struct ffmpeglib_s ffmpeglib_t;

extern int ffmpeglib_open(ffmpeglib_t *lib);
extern void ffmpeglib_close(ffmpeglib_t *lib);

#endif
