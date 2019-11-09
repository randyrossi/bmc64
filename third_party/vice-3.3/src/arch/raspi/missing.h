/*
 * missing.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#ifndef RASPI_MISSING_H
#define RASPI_MISSING_H

#include "monitor.h"
#include "palette.h"
#include "uiapi.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"

char *ui_get_file(const char *format, ...);
char *uimon_get_in(char **p1, const char *p2);
char video_canvas_can_resize(struct video_canvas_s *canvas);
int archdep_rtc_get_centisecond(void);
int c128ui_init_early(void);
int c128ui_init(void);
int c64dtvui_init_early(void);
int c64dtvui_init(void);
int c64scui_init_early(void);
int c64scui_init(void);
int c64ui_init_early(void);
int c64ui_init(void);
int cbm2ui_init_early(void);
int cbm2ui_init(void);
int cbm5x0ui_init_early(void);
int cbm5x0ui_init(void);
int console_close_all(void);
int console_init(void);
int dthread_ui_init_finish(void);
int dthread_ui_init(int *argc, char **argv);
int joy_arch_cmdline_options_init(void);
int joy_arch_resources_init(void);
int joy_arch_set_device(int port_idx, int new_dev);
int mui_init(void);
int petui_init_early(void);
int petui_init(void);
int plus4ui_init_early(void);
int plus4ui_init(void);
int scpu64ui_init_early(void);
int scpu64ui_init(void);
int ui_cmdline_options_init(void);
int ui_extend_image_dialog(void);
int ui_init2(int *argc, char **argv);
int ui_init_finalize(void);
int ui_init_finish2(void);
int ui_init_finish(void);
int ui_init(int *argc, char **argv);
int uimon_out(const char *buffer);
int ui_resources_init(void);
int vic20ui_init_early(void);
int vic20ui_init(void);
int video_arch_cmdline_options_init(void);
int video_arch_resources_init(void);
int video_canvas_refresh_dx9(video_canvas_t *canvas, unsigned int xs,
                             unsigned int ys, unsigned int xi, unsigned int yi,
                             unsigned int w, unsigned int h);
int video_init(void);
int vsid_ui_init(void);
struct console_s *uimon_window_open(void);
struct console_s *uimon_window_resume(void);
ui_jam_action_t ui_jam_dialog(const char *format, ...);
video_canvas_t *video_canvas_create_ddraw(video_canvas_t *canvas);
video_canvas_t *video_canvas_create_dx9(video_canvas_t *canvas,
                                        unsigned int *width,
                                        unsigned int *height);
void archdep_signals_init(int do_core_dumps);
void c128ui_shutdown(void);
void c64dtvui_shutdown(void);
void c64scui_shutdown(void);
void c64ui_shutdown(void);
void cbm2ui_shutdown(void);
void cbm5x0ui_shutdown(void);
void fullscreen_capability(struct cap_fullscreen_s *cap_fullscreen);
void joy_arch_init_default_mapping(int joynum);
void petui_shutdown(void);
void plus4ui_shutdown(void);
void scpu64ui_shutdown(void);
void sdl_ui_init_draw_params(void);
void sdl_ui_init_finalize(void);
void signals_init(int do_core_dumps);
void tui_error(const char *format, ...);
void tui_init(void);
void ui_display_drive_current_image(unsigned int drive_number,
                                    const char *image);
void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base,
                            unsigned int half_track_number);
void ui_display_event_time(unsigned int current, unsigned int total);
void ui_display_joyport(uint8_t *joyport);
void ui_display_playback(int playback_status, char *version);
void ui_display_recording(int recording_status);
void ui_display_tape_current_image(const char *image);
void ui_error_string(const char *text);
void ui_init_checkbox_style(void);
void ui_init_drive_status_widget(void);
void ui_init_joystick_status_widget(void);
void uimon_notify_change(void);
void uimon_set_interface(struct monitor_interface_s **p1, int p2);
void uimon_window_close(void);
void uimon_window_suspend(void);
void ui_resources_shutdown(void);
void ui_set_tape_status(int tape_status);
void ui_shutdown(void);
void ui_update_menus(void);
void vic20ui_shutdown(void);
void video_arch_resources_shutdown(void);
void video_canvas_destroy_ddraw(video_canvas_t *canvas);
void video_canvas_destroy(struct video_canvas_s *canvas);
void video_canvas_refresh_ddraw(video_canvas_t *canvas, unsigned int xs,
                                unsigned int ys, unsigned int xi,
                                unsigned int yi, unsigned int w,
                                unsigned int h);
void video_canvas_resize(struct video_canvas_s *canvas, char resize_canvas);
void video_canvas_set_palette_ddraw_8bit(video_canvas_t *canvas);
void video_shutdown_dx9(void);
void video_shutdown(void);
void vsyncarch_display_speed(double speed, double fps, int warp_enabled);
void ui_display_volume(int vol);
void main_exit(void);
#endif
