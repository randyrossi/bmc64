/*
 * vsidui.c - Implementation of the VSID UI.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
 * based on c64ui.c written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
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

#include "log.h"
#include "machine.h"


int vsid_ui_init(void)
{
    return 0;
}

void vsid_ui_display_name(const char *name)
{
    log_message(LOG_DEFAULT, "Name: %s", name);
}

void vsid_ui_display_author(const char *author)
{
    log_message(LOG_DEFAULT, "Author: %s", author);
}

void vsid_ui_display_copyright(const char *copyright)
{
    log_message(LOG_DEFAULT, "Copyright by: %s", copyright);
}

void vsid_ui_display_sync(int sync)
{
    log_message(LOG_DEFAULT, "Using %s sync", sync == MACHINE_SYNC_PAL ? "PAL" : "NTSC");
}

void vsid_ui_display_sid_model(int model)
{
    log_message(LOG_DEFAULT, "Using %s emulation", model == 0 ? "MOS6581" : "MOS8580");
}

void vsid_ui_set_default_tune(int nr)
{
    log_message(LOG_DEFAULT, "Default Tune: %i", nr);
}

void vsid_ui_display_tune_nr(int nr)
{
    log_message(LOG_DEFAULT, "Playing Tune: %i", nr);
}

void vsid_ui_display_nr_of_tunes(int count)
{
    log_message(LOG_DEFAULT, "Number of Tunes: %i", count);
}

void vsid_ui_display_time(unsigned int sec)
{
}

void vsid_ui_display_irqtype(const char *irq)
{
}


/** \brief  Set driver address
 *
 * \param[in]   addr    driver address
 */
void vsid_ui_set_driver_addr(uint16_t addr)
{
}


/** \brief  Set load address
 *
 * \param[in]   addr    load address
 */
void vsid_ui_set_load_addr(uint16_t addr)
{
}


/** \brief  Set init routine address
 *
 * \param[in]   addr    init routine address
 */
void vsid_ui_set_init_addr(uint16_t addr)
{
}


/** \brief  Set play routine address
 *
 * \param[in]   addr    play routine address
 */
void vsid_ui_set_play_addr(uint16_t addr)
{
}


/** \brief  Set size of SID on actual machine
 *
 * \param[in]   size    size of SID
 */
void vsid_ui_set_data_size(uint16_t size)
{
}


void vsid_ui_close(void)
{
}

void vsid_ui_setdrv(char* driver_info_text)
{
}
