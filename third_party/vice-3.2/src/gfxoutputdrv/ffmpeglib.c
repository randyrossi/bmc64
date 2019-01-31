/*
 * ffmpeglib.c - Interface to access the ffmpeg libs.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifdef HAVE_FFMPEG

#include "archdep.h"
#include "gfxoutputdrv/ffmpeglib.h"
#include "log.h"
#include "translate.h"
#include "uiapi.h"
#include "dynlib.h"

#ifndef STATIC_FFMPEG

/* define major version if its not already defined */
#ifndef LIBAVCODEC_VERSION_MAJOR
#define LIBAVCODEC_VERSION_MAJOR  51
#endif
#ifndef LIBAVFORMAT_VERSION_MAJOR
#define LIBAVFORMAT_VERSION_MAJOR 52
#define NO_AVFORMAT_CHECK 1
#endif
#ifndef LIBAVUTIL_VERSION_MAJOR
#define LIBAVUTIL_VERSION_MAJOR   49
#define NO_AVUTIL_CHECK 1
#endif
#ifndef LIBSWSCALE_VERSION_MAJOR
#define LIBSWSCALE_VERSION_MAJOR  0
#endif
#ifndef LIBSWRESAMPLE_VERSION_MAJOR
#define LIBSWRESAMPLE_VERSION_MAJOR  0
#endif
#ifndef LIBAVRESAMPLE_VERSION_MAJOR
#define LIBAVRESAMPLE_VERSION_MAJOR  2
#endif

#define AVCODEC_SO_NAME     ARCHDEP_MAKE_SO_NAME_VERSION(avcodec, LIBAVCODEC_VERSION_MAJOR)
#define AVFORMAT_SO_NAME    ARCHDEP_MAKE_SO_NAME_VERSION(avformat, LIBAVFORMAT_VERSION_MAJOR)
#define AVUTIL_SO_NAME      ARCHDEP_MAKE_SO_NAME_VERSION(avutil, LIBAVUTIL_VERSION_MAJOR)
#define SWSCALE_SO_NAME     ARCHDEP_MAKE_SO_NAME_VERSION(swscale, LIBSWSCALE_VERSION_MAJOR)
#define SWRESAMPLE_SO_NAME  ARCHDEP_MAKE_SO_NAME_VERSION(swresample, LIBSWRESAMPLE_VERSION_MAJOR)
#define AVRESAMPLE_SO_NAME  ARCHDEP_MAKE_SO_NAME_VERSION(avresample, LIBAVRESAMPLE_VERSION_MAJOR)

static void *avcodec_so = NULL;
static void *avformat_so = NULL;
static void *avutil_so = NULL;
static void *swscale_so = NULL;

#ifndef HAVE_FFMPEG_AVRESAMPLE
static void *swresample_so = NULL;
#else
static void *avresample_so = NULL;
#endif

/* macro for getting functionpointers from avcodec */
#define GET_SYMBOL_AND_TEST_AVCODEC( _name_ )                              \
    lib->p_##_name_ = (_name_##_t)vice_dynlib_symbol(avcodec_so, #_name_); \
    if (!lib->p_##_name_) {                                                \
        log_debug("getting symbol " #_name_ " failed!");                   \
        return -1;                                                         \
    }

/* macro for getting functionpointers from avformat */
#define GET_SYMBOL_AND_TEST_AVFORMAT( _name_ )                               \
    lib->p_##_name_ = (_name_##_t)vice_dynlib_symbol(avformat_so, #_name_ ); \
    if (!lib->p_##_name_) {                                                  \
        log_debug("getting symbol " #_name_ " failed!");                     \
        return -1;                                                           \
    }

/* macro for getting functionpointers from avutil */
#define GET_SYMBOL_AND_TEST_AVUTIL( _name_ )                               \
    lib->p_##_name_ = (_name_##_t)vice_dynlib_symbol(avutil_so, #_name_ ); \
    if (!lib->p_##_name_) {                                                \
        log_debug("getting symbol " #_name_ " failed!");                   \
        return -1;                                                         \
    }

/* macro for getting functionpointers from swscale */
#define GET_SYMBOL_AND_TEST_SWSCALE( _name_ )                               \
    lib->p_##_name_ = (_name_##_t)vice_dynlib_symbol(swscale_so, #_name_ ); \
    if (!lib->p_##_name_) {                                                 \
        log_debug("getting symbol " #_name_ " failed!");                    \
        return -1;                                                          \
    }

#ifndef HAVE_FFMPEG_AVRESAMPLE
/* macro for getting functionpointers from swresample */
#define GET_SYMBOL_AND_TEST_SWRESAMPLE( _name_ )                                \
    lib->p_##_name_ = (_name_##_t)vice_dynlib_symbol(swresample_so, #_name_);   \
    if (!lib->p_##_name_) {                                                     \
    log_debug("getting symbol " #_name_ " failed!");                            \
    return -1;                                                                  \
}
#else
/* macro for getting functionpointers from avresample */
#define GET_SYMBOL_AND_TEST_AVRESAMPLE( _name_ )                                \
    lib->p_##_name_ = (_name_##_t)vice_dynlib_symbol(avresample_so, #_name_);   \
    if (!lib->p_##_name_) {                                                     \
    log_debug("getting symbol " #_name_ " failed!");                            \
    return -1;                                                                  \
}
#endif

static int check_version(const char *lib_name, void *handle, const char *symbol, unsigned ver_inc)
{
    ffmpeg_version_t version_func;
    unsigned ver_lib;
    const char *result_msgs[] = { "full match","major.minor matches","major matches","unsupported" };
    enum { FULL_MATCH=0, MAJOR_MINOR_MATCH=1, MAJOR_MATCH=2, NO_MATCH=3 } result;

    version_func = (ffmpeg_version_t)vice_dynlib_symbol(handle, symbol);
    if (version_func == NULL) {
        log_debug("ffmpeg %s: version function '%s' not found! error: %s", lib_name, symbol, vice_dynlib_error());
        return -1;
    }

    ver_lib = version_func();

    /* version matches exactly */
    if (ver_lib == ver_inc) {
        result = FULL_MATCH;
    } else {
        /* compare major.minor */
        ver_lib >>= 8;
        ver_inc >>= 8;
        if (ver_lib == ver_inc) {
            result = MAJOR_MINOR_MATCH;
        } else {
            /* compare major */
            ver_lib >>= 8;
            ver_inc >>= 8;
            if (ver_lib == ver_inc) {
                result = MAJOR_MATCH;
            } else {
                result = NO_MATCH;
            }
        }
    }

    log_debug("ffmpeg %8s lib has version %06x, VICE expects %06x: %s",
              lib_name, ver_lib, ver_inc, result_msgs[result]);

    /* now decide what level of matching fails */
    if (result == NO_MATCH) {
        return -1;
    }
    return 0;
}

static int load_avcodec(ffmpeglib_t *lib)
{
    if (!avcodec_so) {
        avcodec_so = vice_dynlib_open(AVCODEC_SO_NAME);

        if (!avcodec_so) {
            log_debug("opening dynamic library " AVCODEC_SO_NAME " failed! error: %s", vice_dynlib_error());
            return -1;
        }

        GET_SYMBOL_AND_TEST_AVCODEC(av_init_packet);
        GET_SYMBOL_AND_TEST_AVCODEC(avcodec_open2);
        GET_SYMBOL_AND_TEST_AVCODEC(avcodec_close);
        GET_SYMBOL_AND_TEST_AVCODEC(avcodec_find_encoder);
        GET_SYMBOL_AND_TEST_AVCODEC(avcodec_encode_audio2);
        GET_SYMBOL_AND_TEST_AVCODEC(avcodec_encode_video2);
        GET_SYMBOL_AND_TEST_AVCODEC(avpicture_fill);
        GET_SYMBOL_AND_TEST_AVCODEC(avpicture_get_size);
        GET_SYMBOL_AND_TEST_AVCODEC(av_packet_rescale_ts);
    }

    return check_version("avcodec", avcodec_so, "avcodec_version", LIBAVCODEC_VERSION_INT);
}

static void free_avcodec(ffmpeglib_t *lib)
{
    if (avcodec_so) {
        if (vice_dynlib_close(avcodec_so) != 0) {
            log_debug("closing dynamic library " AVCODEC_SO_NAME " failed!");
        }
    }
    avcodec_so = NULL;

    lib->p_avcodec_open2 = NULL;
    lib->p_avcodec_close = NULL;
    lib->p_avcodec_find_encoder = NULL;
    lib->p_avcodec_encode_audio2 = NULL;
    lib->p_avcodec_encode_video2 = NULL;
    lib->p_avpicture_fill = NULL;
    lib->p_avpicture_get_size = NULL;
}

static int load_avformat(ffmpeglib_t *lib)
{
    if (!avformat_so) {
        avformat_so = vice_dynlib_open(AVFORMAT_SO_NAME);

        if (!avformat_so) {
            log_debug("opening dynamic library " AVFORMAT_SO_NAME " failed! error: %s", vice_dynlib_error());
            return -1;
        }

        GET_SYMBOL_AND_TEST_AVFORMAT(av_register_all);
        GET_SYMBOL_AND_TEST_AVFORMAT(avformat_alloc_context);
        GET_SYMBOL_AND_TEST_AVFORMAT(avformat_new_stream);
        GET_SYMBOL_AND_TEST_AVFORMAT(avformat_write_header);
        GET_SYMBOL_AND_TEST_AVFORMAT(av_interleaved_write_frame);
        GET_SYMBOL_AND_TEST_AVFORMAT(av_write_trailer);
        GET_SYMBOL_AND_TEST_AVFORMAT(avio_open);
        GET_SYMBOL_AND_TEST_AVFORMAT(avio_close);
        GET_SYMBOL_AND_TEST_AVFORMAT(av_dump_format);
        GET_SYMBOL_AND_TEST_AVFORMAT(av_guess_format);
    }

#ifdef NO_AVFORMAT_CHECK
    return 0;
#else
    return check_version("avformat", avformat_so, "avformat_version", LIBAVFORMAT_VERSION_INT);
#endif
}

static void free_avformat(ffmpeglib_t *lib)
{
    if (avformat_so) {
        if (vice_dynlib_close(avformat_so) != 0) {
            log_debug("closing dynamic library " AVFORMAT_SO_NAME " failed! error: %s", vice_dynlib_error());
        }
    }
    avformat_so = NULL;

    lib->p_av_init_packet = NULL;
    lib->p_av_register_all = NULL;
    lib->p_avformat_alloc_context = NULL;
    lib->p_avformat_new_stream = NULL;
    lib->p_avformat_write_header = NULL;
    lib->p_av_interleaved_write_frame = NULL;
    lib->p_av_write_trailer = NULL;
    lib->p_avio_open = NULL;
    lib->p_avio_close = NULL;
    lib->p_av_dump_format = NULL;
    lib->p_av_guess_format = NULL;
}

static int load_avutil(ffmpeglib_t *lib)
{
    if (!avutil_so) {
        avutil_so = vice_dynlib_open(AVUTIL_SO_NAME);

        if (!avutil_so) {
            log_debug("opening dynamic library " AVUTIL_SO_NAME " failed! error: %s", vice_dynlib_error());
            return -1;
        }

        GET_SYMBOL_AND_TEST_AVUTIL(av_free);
        GET_SYMBOL_AND_TEST_AVUTIL(av_frame_alloc);
        GET_SYMBOL_AND_TEST_AVUTIL(av_frame_get_buffer);
        GET_SYMBOL_AND_TEST_AVUTIL(av_compare_ts);
        GET_SYMBOL_AND_TEST_AVUTIL(av_get_channel_layout_nb_channels);
        GET_SYMBOL_AND_TEST_AVUTIL(av_opt_set_int);
#ifndef HAVE_FFMPEG_AVRESAMPLE
        GET_SYMBOL_AND_TEST_AVUTIL(av_opt_set_sample_fmt);
#endif
        GET_SYMBOL_AND_TEST_AVUTIL(av_rescale_rnd);
        GET_SYMBOL_AND_TEST_AVUTIL(av_rescale_q);
        GET_SYMBOL_AND_TEST_AVUTIL(av_d2q);
        GET_SYMBOL_AND_TEST_AVUTIL(av_frame_make_writable);
        GET_SYMBOL_AND_TEST_AVUTIL(av_frame_free);
        GET_SYMBOL_AND_TEST_AVUTIL(av_dict_set);
        GET_SYMBOL_AND_TEST_AVUTIL(av_dict_free);
    }

#ifdef NO_AVUTIL_CHECK
    return 0;
#else
    return check_version("avutil", avutil_so, "avutil_version", LIBAVUTIL_VERSION_INT);
#endif
}

static void free_avutil(ffmpeglib_t *lib)
{
    if (avutil_so) {
        if (vice_dynlib_close(avutil_so) != 0) {
            log_debug("closing dynamic library " AVUTIL_SO_NAME " failed! error: %s", vice_dynlib_error());
        }
    }
    avutil_so = NULL;

    lib->p_av_free = NULL;
    lib->p_av_frame_alloc = NULL;
    lib->p_av_frame_get_buffer = NULL;
    lib->p_av_compare_ts = NULL;
    lib->p_av_get_channel_layout_nb_channels = NULL;
    lib->p_av_opt_set_int = NULL;
    lib->p_av_opt_set_sample_fmt = NULL;
    lib->p_av_rescale_rnd = NULL;
    lib->p_av_rescale_q = NULL;
    lib->p_av_d2q = NULL;
    lib->p_av_frame_make_writable = NULL;
    lib->p_av_frame_free = NULL;
    lib->p_av_dict_set = NULL;
    lib->p_av_dict_free = NULL;
}

static int load_swscale(ffmpeglib_t *lib)
{
    if (!swscale_so) {
        swscale_so = vice_dynlib_open(SWSCALE_SO_NAME);

        if (!swscale_so) {
            log_debug("opening dynamic library " SWSCALE_SO_NAME " failed! error: %s", vice_dynlib_error());
            return -1;
        }

        GET_SYMBOL_AND_TEST_SWSCALE(sws_getContext);
        GET_SYMBOL_AND_TEST_SWSCALE(sws_freeContext);
        GET_SYMBOL_AND_TEST_SWSCALE(sws_scale);
    }

    return check_version("swscale", swscale_so, "swscale_version", LIBSWSCALE_VERSION_INT);
}

static void free_swscale(ffmpeglib_t *lib)
{
    if (swscale_so) {
        if (vice_dynlib_close(swscale_so) != 0) {
            log_debug("closing dynamic library " SWSCALE_SO_NAME " failed! error: %s", vice_dynlib_error());
        }
    }
    swscale_so = NULL;

    lib->p_sws_getContext = NULL;
    lib->p_sws_freeContext = NULL;
    lib->p_sws_scale = NULL;
}

#ifndef HAVE_FFMPEG_AVRESAMPLE
static int load_swresample(ffmpeglib_t *lib)
{
    if (!swresample_so) {
        swresample_so = vice_dynlib_open(SWRESAMPLE_SO_NAME);

        if (!swresample_so) {
            log_debug("opening dynamic library " SWRESAMPLE_SO_NAME " failed! error: %s", vice_dynlib_error());
            return -1;
        }

        GET_SYMBOL_AND_TEST_SWRESAMPLE(swr_alloc);
        GET_SYMBOL_AND_TEST_SWRESAMPLE(swr_init);
        GET_SYMBOL_AND_TEST_SWRESAMPLE(swr_convert);
        GET_SYMBOL_AND_TEST_SWRESAMPLE(swr_get_delay);
        GET_SYMBOL_AND_TEST_SWRESAMPLE(swr_free);
    }

    return check_version("swresample", swresample_so, "swresample_version", LIBSWRESAMPLE_VERSION_INT);
}

static void free_swresample(ffmpeglib_t *lib)
{
    if (swresample_so) {
        if (vice_dynlib_close(swresample_so) != 0) {
            log_debug("closing dynamic library " SWRESAMPLE_SO_NAME " failed! error: %s", vice_dynlib_error());
        }
    }
    swresample_so = NULL;

    lib->p_swr_alloc = NULL;
    lib->p_swr_init = NULL;
    lib->p_swr_convert = NULL;
    lib->p_swr_get_delay = NULL;
    lib->p_swr_free = NULL;
}
#else
static int load_avresample(ffmpeglib_t *lib)
{
    if (!avresample_so) {
        avresample_so = vice_dynlib_open(AVRESAMPLE_SO_NAME);

        if (!avresample_so) {
            log_debug("opening dynamic library " AVRESAMPLE_SO_NAME " failed! error: %s", vice_dynlib_error());
            return -1;
        }

        GET_SYMBOL_AND_TEST_AVRESAMPLE(avresample_alloc_context);
        GET_SYMBOL_AND_TEST_AVRESAMPLE(avresample_open);
        GET_SYMBOL_AND_TEST_AVRESAMPLE(avresample_convert);
        GET_SYMBOL_AND_TEST_AVRESAMPLE(avresample_get_delay);
        GET_SYMBOL_AND_TEST_AVRESAMPLE(avresample_free);
    }

    return check_version("avresample", avresample_so, "avresample_version", LIBAVRESAMPLE_VERSION_INT);
}

static void free_avresample(ffmpeglib_t *lib)
{
    if (avresample_so) {
        if (vice_dynlib_close(avresample_so) != 0) {
            log_debug("closing dynamic library " AVRESAMPLE_SO_NAME " failed! error: %s", vice_dynlib_error());
        }
    }
    avresample_so = NULL;

    lib->p_avresample_alloc_context = NULL;
    lib->p_avresample_open = NULL;
    lib->p_avresample_convert = NULL;
    lib->p_avresample_get_delay = NULL;
    lib->p_avresample_free = NULL;
}
#endif

int ffmpeglib_open(ffmpeglib_t *lib)
{
    int result;

    result = load_avformat(lib);
    if (result != 0) {
        free_avformat(lib);
        return result;
    }

    result = load_avcodec(lib);
    if (result != 0) {
        free_avformat(lib);
        free_avcodec(lib);
        return result;
    }

    result = load_avutil(lib);
    if (result != 0) {
        free_avformat(lib);
        free_avcodec(lib);
        free_avutil(lib);
        return result;
    }

    result = load_swscale(lib);
    if (result != 0) {
        free_avformat(lib);
        free_avcodec(lib);
        free_avutil(lib);
        free_swscale(lib);
        return result;
    }

#ifndef HAVE_FFMPEG_AVRESAMPLE
    result = load_swresample(lib);
    if (result != 0) {
        free_avformat(lib);
        free_avcodec(lib);
        free_avutil(lib);
        free_swscale(lib);
        free_swresample(lib);
        return result;
    }
#else
    result = load_avresample(lib);
    if (result != 0) {
        free_avformat(lib);
        free_avcodec(lib);
        free_avutil(lib);
        free_swscale(lib);
        free_avresample(lib);
        return result;
    }
#endif

    return 0;
}

void ffmpeglib_close(ffmpeglib_t *lib)
{
    free_avformat(lib);
    free_avcodec(lib);
    free_avutil(lib);
    free_swscale(lib);
#ifndef HAVE_FFMPEG_AVRESAMPLE
    free_swresample(lib);
#else
    free_avresample(lib);
#endif
}
#else
int ffmpeglib_open(ffmpeglib_t *lib)
{
    return 0;
}

void ffmpeglib_close(ffmpeglib_t *lib)
{
}
#endif
#endif /* #ifdef HAVE_FFMPEG */
