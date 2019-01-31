/*
 * ffmpegdrv.h - Movie driver using FFMPEG library and screenshot API.
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

#ifndef VICE_FFMPEGDRV_H
#define VICE_FFMPEGDRV_H

#include "screenshot.h"
#include "gfxoutput.h"

extern void gfxoutput_init_ffmpeg(int help);

/* deprecated access for UIs that do not use the gfxoutputdrv->formatlist yet: */
extern gfxoutputdrv_format_t *ffmpegdrv_formatlist;

#ifdef STATIC_FFMPEG
#define VICE_P_AV_PACKET_RESCALE_TS              av_packet_rescale_ts
#define VICE_P_AV_INTERLEAVED_WRITE_FRAME        av_interleaved_write_frame
#define VICE_P_AVCODEC_CLOSE                     avcodec_close
#define VICE_P_AV_FRAME_FREE                     av_frame_free
#define VICE_P_AV_FRAME_ALLOC                    av_frame_alloc
#define VICE_P_AV_FRAME_GET_BUFFER               av_frame_get_buffer
#define VICE_P_AV_DICT_SET                       av_dict_set
#define VICE_P_AVCODEC_OPEN2                     avcodec_open2
#define VICE_P_AV_DICT_FREE                      av_dict_free
#define VICE_P_AVFORMAT_NEW_STREAM               avformat_new_stream
#define VICE_P_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS av_get_channel_layout_nb_channels
#define VICE_P_AV_OPT_SET_INT                    av_opt_set_int
#define VICE_P_AV_OPT_SET_SAMPLE_FMT             av_opt_set_sample_fmt
#define VICE_P_AV_INIT_PACKET                    av_init_packet
#define VICE_P_AV_RESCALE_RND                    av_rescale_rnd
#define VICE_P_AV_FRAME_MAKE_WRITABLE            av_frame_make_writable
#define VICE_P_AV_RESCALE_Q                      av_rescale_q
#define VICE_P_AV_D2Q                            av_d2q
#define VICE_P_AVCODEC_ENCODE_AUDIO2             avcodec_encode_audio2
#define VICE_P_SWS_FREECONTEXT                   sws_freeContext
#define VICE_P_SWS_GETCONTEXT                    sws_getContext
#define VICE_P_AV_DUMP_FORMAT                    av_dump_format
#define VICE_P_AVIO_OPEN                         avio_open
#define VICE_P_AVFORMAT_WRITE_HEADER             avformat_write_header
#define VICE_P_AV_GUESS_FORMAT                   av_guess_format
#define VICE_P_AVCODEC_FIND_ENCODER              avcodec_find_encoder
#define VICE_P_AVFORMAT_ALLOC_CONTEXT            avformat_alloc_context
#define VICE_P_AV_WRITE_TRAILER                  av_write_trailer
#define VICE_P_AVIO_CLOSE                        avio_close
#define VICE_P_AV_FREE                           av_free
#define VICE_P_SWS_SCALE                         sws_scale
#define VICE_P_AVCODEC_ENCODE_VIDEO2             avcodec_encode_video2
#define VICE_P_AV_REGISTER_ALL                   av_register_all
#define VICE_P_SWR_FREE                          swr_free
#define VICE_P_SWR_ALLOC                         swr_alloc
#define VICE_P_SWR_INIT                          swr_init
#define VICE_P_SWR_GET_DELAY                     swr_get_delay
#define VICE_P_SWR_CONVERT                       swr_convert
#define VICE_P_AVRESAMPLE_ALLOC_CONTEXT          avresample_alloc_context
#define VICE_P_AVRESAMPLE_OPEN                   avresample_alloc_open
#define VICE_P_AVRESAMPLE_CONVERT                avresample_convert
#define VICE_P_AVRESAMPLE_FREE                   avresample_free
#define VICE_P_AVRESAMPLE_GET_DELAY              avresample_get_delay
#else
#define VICE_P_AV_PACKET_RESCALE_TS              (*ffmpeglib.p_av_packet_rescale_ts)
#define VICE_P_AV_INTERLEAVED_WRITE_FRAME        (*ffmpeglib.p_av_interleaved_write_frame)
#define VICE_P_AVCODEC_CLOSE                     (*ffmpeglib.p_avcodec_close)
#define VICE_P_AV_FRAME_FREE                     (*ffmpeglib.p_av_frame_free)
#define VICE_P_AV_FRAME_ALLOC                    (*ffmpeglib.p_av_frame_alloc)
#define VICE_P_AV_FRAME_GET_BUFFER               (*ffmpeglib.p_av_frame_get_buffer)
#define VICE_P_AV_DICT_SET                       (*ffmpeglib.p_av_dict_set)
#define VICE_P_AVCODEC_OPEN2                     (*ffmpeglib.p_avcodec_open2)
#define VICE_P_AV_DICT_FREE                      (*ffmpeglib.p_av_dict_free)
#define VICE_P_AVFORMAT_NEW_STREAM               (*ffmpeglib.p_avformat_new_stream)
#define VICE_P_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS (*ffmpeglib.p_av_get_channel_layout_nb_channels)
#define VICE_P_AV_OPT_SET_INT                    (*ffmpeglib.p_av_opt_set_int)
#define VICE_P_AV_OPT_SET_SAMPLE_FMT             (*ffmpeglib.p_av_opt_set_sample_fmt)
#define VICE_P_AV_INIT_PACKET                    (*ffmpeglib.p_av_init_packet)
#define VICE_P_AV_RESCALE_RND                    (*ffmpeglib.p_av_rescale_rnd)
#define VICE_P_AV_FRAME_MAKE_WRITABLE            (*ffmpeglib.p_av_frame_make_writable)
#define VICE_P_AV_RESCALE_Q                      (*ffmpeglib.p_av_rescale_q)
#define VICE_P_AV_D2Q                            (*ffmpeglib.p_av_d2q)
#define VICE_P_AVCODEC_ENCODE_AUDIO2             (*ffmpeglib.p_avcodec_encode_audio2)
#define VICE_P_SWS_FREECONTEXT                   (*ffmpeglib.p_sws_freeContext)
#define VICE_P_SWS_GETCONTEXT                    (*ffmpeglib.p_sws_getContext)
#define VICE_P_AV_DUMP_FORMAT                    (*ffmpeglib.p_av_dump_format)
#define VICE_P_AVIO_OPEN                         (*ffmpeglib.p_avio_open)
#define VICE_P_AVFORMAT_WRITE_HEADER             (*ffmpeglib.p_avformat_write_header)
#define VICE_P_AV_GUESS_FORMAT                   (*ffmpeglib.p_av_guess_format)
#define VICE_P_AVCODEC_FIND_ENCODER              (*ffmpeglib.p_avcodec_find_encoder)
#define VICE_P_AVFORMAT_ALLOC_CONTEXT            (*ffmpeglib.p_avformat_alloc_context)
#define VICE_P_AV_WRITE_TRAILER                  (*ffmpeglib.p_av_write_trailer)
#define VICE_P_AVIO_CLOSE                        (*ffmpeglib.p_avio_close)
#define VICE_P_AV_FREE                           (*ffmpeglib.p_av_free)
#define VICE_P_SWS_SCALE                         (*ffmpeglib.p_sws_scale)
#define VICE_P_AVCODEC_ENCODE_VIDEO2             (*ffmpeglib.p_avcodec_encode_video2)
#define VICE_P_AV_REGISTER_ALL                   (*ffmpeglib.p_av_register_all)
#define VICE_P_SWR_GET_DELAY                     (*ffmpeglib.p_swr_get_delay)
#define VICE_P_SWR_INIT                          (*ffmpeglib.p_swr_init)
#define VICE_P_SWR_FREE                          (*ffmpeglib.p_swr_free)
#define VICE_P_SWR_ALLOC                         (*ffmpeglib.p_swr_alloc)
#define VICE_P_SWR_CONVERT                       (*ffmpeglib.p_swr_convert)
#define VICE_P_AVRESAMPLE_ALLOC_CONTEXT          (*ffmpeglib.p_avresample_alloc_context)
#define VICE_P_AVRESAMPLE_OPEN                   (*ffmpeglib.p_avresample_open)
#define VICE_P_AVRESAMPLE_CONVERT                (*ffmpeglib.p_avresample_convert)
#define VICE_P_AVRESAMPLE_FREE                   (*ffmpeglib.p_avresample_free)
#define VICE_P_AVRESAMPLE_GET_DELAY              (*ffmpeglib.p_avresample_get_delay)
#endif

#endif
