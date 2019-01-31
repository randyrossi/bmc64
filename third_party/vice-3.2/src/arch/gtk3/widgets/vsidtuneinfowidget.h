/** \file   vsidmainwidget.h
 * \brief   GTK3 tune info widget for VSID - header
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
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

#ifndef VICE_VSIDTUNEINFOWIDGET_H
#define VICE_VSIDTUNEINFOWIDGET_H

#include "vice.h"
#include <gtk/gtk.h>

GtkWidget *vsid_tune_info_widget_create(void);

void vsid_tune_info_widget_set_name(const char *name);
void vsid_tune_info_widget_set_author(const char *author);
void vsid_tune_info_widget_set_copyright(const char *copyright);
void vsid_tune_info_widget_set_tune_count(int count);
void vsid_tune_info_widget_set_tune_default(int num);
void vsid_tune_info_widget_set_tune_current(int num);
void vsid_tune_info_widget_set_model(int model);
void vsid_tune_info_widget_set_sync(int sync);
void vsid_tune_info_widget_set_irq(const char *irq);
void vsid_tune_info_widget_set_time(unsigned int sec);
void vsid_tune_info_widget_set_driver(const char *text);
void vsid_tune_info_widget_set_driver_addr(uint16_t addr);
void vsid_tune_info_widget_set_load_addr(uint16_t addr);
void vsid_tune_info_widget_set_init_addr(uint16_t addr);
void vsid_tune_info_widget_set_play_addr(uint16_t addr);
void vsid_tune_info_widget_set_data_size(uint16_t size);


#endif
