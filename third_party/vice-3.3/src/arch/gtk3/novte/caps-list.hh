/*
 * Copyright (C) 2001,2002 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define ESC _VTE_CAP_ESC
#define CSI _VTE_CAP_CSI
#define ST  _VTE_CAP_ST
#define OSC _VTE_CAP_OSC
#define PM  _VTE_CAP_PM
#define APC _VTE_CAP_APC

#define ENQ "\005"
#define BEL "\007"
#define BS  "\010"
#define TAB "\011"
#define LF  "\012"
#define VT  "\013"
#define FF  "\014"
#define CR  "\015"
#define SO  "\016"
#define SI  "\017"
#define DEL "\177"

#define ENTRY(str,handler) { str, &VteTerminalPrivate::seq_ ## handler }

/* From some really old XTerm docs we had at the office, and an updated
 * version at Moy, Gildea, and Dickey. */
static const vte_matcher_entry_t entries[] = {
        ENTRY(ENQ, return_terminal_status),
        ENTRY(BEL, bell),
        ENTRY(BS,  backspace),
        ENTRY(TAB, tab),
        ENTRY(LF,  line_feed),
        ENTRY(VT,  vertical_tab),
        ENTRY(FF,  form_feed),
        ENTRY(CR,  carriage_return),
        ENTRY(SO,  shift_out),
        ENTRY(SI,  shift_in),
        ENTRY(DEL, nop),

        ENTRY(ESC " F", seven_bit_controls),
        ENTRY(ESC " G", eight_bit_controls),
        ENTRY(ESC " L", ansi_conformance_level_1),
        ENTRY(ESC " M", ansi_conformance_level_2),
        ENTRY(ESC " N", ansi_conformance_level_3),
        ENTRY(ESC "#3", double_height_top_half),
        ENTRY(ESC "#4", double_height_bottom_half),
        ENTRY(ESC "#5", single_width),
        ENTRY(ESC "#6", double_width),
        ENTRY(ESC "#8", screen_alignment_test),

        /* These are actually designate_other_coding_system from ECMA 35,
         * but we don't support the full repertoire.  Actually, we don't
         * know what the full repertoire looks like.
         */
        ENTRY(ESC "%%@", default_character_set),
        ENTRY(ESC "%%G", utf_8_character_set),

        ENTRY(ESC "(0", designate_g0_line_drawing),
        ENTRY(ESC "(A", designate_g0_british),
        ENTRY(ESC "(B", designate_g0_plain),
        ENTRY(ESC ")0", designate_g1_line_drawing),
        ENTRY(ESC ")A", designate_g1_british),
        ENTRY(ESC ")B", designate_g1_plain),

        ENTRY(ESC "7", save_cursor),
        ENTRY(ESC "8", restore_cursor),
        ENTRY(ESC "=", application_keypad),
        ENTRY(ESC ">", normal_keypad),
        ENTRY(ESC "D", index),
        ENTRY(ESC "E", next_line),
        ENTRY(ESC "F", cursor_lower_left),
        ENTRY(ESC "H", tab_set),
        ENTRY(ESC "M", reverse_index),
        ENTRY(ESC "N", single_shift_g2),
        ENTRY(ESC "O", single_shift_g3),
        ENTRY(ESC "P%s" ST, device_control_string),
        ENTRY(ESC "V", start_of_guarded_area),
        ENTRY(ESC "W", end_of_guarded_area),
        ENTRY(ESC "X%s" ST, start_or_end_of_string),
        ENTRY(ESC "Z", return_terminal_id),
        ENTRY(ESC "c", full_reset),
        ENTRY(ESC "l", memory_lock),
        ENTRY(ESC "m", memory_unlock),
        ENTRY(ESC "n", invoke_g2_character_set),
        ENTRY(ESC "o", invoke_g3_character_set),
        ENTRY(ESC "|", invoke_g3_character_set_as_gr),
        ENTRY(ESC "}", invoke_g2_character_set_as_gr),
        ENTRY(ESC "~", invoke_g1_character_set_as_gr),

        /* APC stuff omitted. */

        /* DCS stuff omitted. */

        ENTRY(CSI "@", insert_blank_characters),
        ENTRY(CSI "%d@", insert_blank_characters),
        ENTRY(CSI "A", cursor_up),
        ENTRY(CSI "%dA", cursor_up),
        ENTRY(CSI "B", cursor_down),
        ENTRY(CSI "%dB", cursor_down),
        ENTRY(CSI "C", cursor_forward),
        ENTRY(CSI "%dC", cursor_forward),
        ENTRY(CSI "D", cursor_backward),
        ENTRY(CSI "%dD", cursor_backward),
        ENTRY(CSI "E", cursor_next_line),
        ENTRY(CSI "%dE", cursor_next_line),
        ENTRY(CSI "F", cursor_preceding_line),
        ENTRY(CSI "%dF", cursor_preceding_line),
        ENTRY(CSI "G", cursor_character_absolute),
        ENTRY(CSI "%dG", cursor_character_absolute),
        ENTRY(CSI "H", cursor_position),
        ENTRY(CSI ";H", cursor_position),
        ENTRY(CSI "%dH", cursor_position),
        ENTRY(CSI "%d;H", cursor_position),
        ENTRY(CSI ";%dH", cursor_position_top_row),
        ENTRY(CSI "%d;%dH", cursor_position),
        ENTRY(CSI "I", cursor_forward_tabulation),
        ENTRY(CSI "%dI", cursor_forward_tabulation),
        ENTRY(CSI "%mJ", erase_in_display),
        ENTRY(CSI "?%mJ", selective_erase_in_display),
        ENTRY(CSI "%mK", erase_in_line),
        ENTRY(CSI "?%mK", selective_erase_in_line),
        ENTRY(CSI "L", insert_lines),
        ENTRY(CSI "%dL", insert_lines),
        ENTRY(CSI "M", delete_lines),
        ENTRY(CSI "%dM", delete_lines),
        ENTRY(CSI "P", delete_characters),
        ENTRY(CSI "%dP", delete_characters),
        ENTRY(CSI "S", scroll_up),
        ENTRY(CSI "%dS", scroll_up),
        ENTRY(CSI "T", scroll_down),
        ENTRY(CSI "%dT", scroll_down),
        ENTRY(CSI "%d;%d;%d;%d;%dT", initiate_hilite_mouse_tracking),
        ENTRY(CSI "X", erase_characters),
        ENTRY(CSI "%dX", erase_characters),
        ENTRY(CSI "Z", cursor_back_tab),
        ENTRY(CSI "%dZ", cursor_back_tab),

        ENTRY(CSI "`", character_position_absolute),
        ENTRY(CSI "%d`", character_position_absolute),
        ENTRY(CSI "b", repeat),
        ENTRY(CSI "%db", repeat),
        ENTRY(CSI "c", send_primary_device_attributes),
        ENTRY(CSI "%dc", send_primary_device_attributes),
        ENTRY(CSI ">c", send_secondary_device_attributes),
        ENTRY(CSI ">%dc", send_secondary_device_attributes),
        ENTRY(CSI "=c", send_tertiary_device_attributes),
        ENTRY(CSI "=%dc", send_tertiary_device_attributes),
        ENTRY(CSI "?%mc", linux_console_cursor_attributes),
        ENTRY(CSI "d", line_position_absolute),
        ENTRY(CSI "%dd", line_position_absolute),
        ENTRY(CSI "f", cursor_position),
        ENTRY(CSI ";f", cursor_position),
        ENTRY(CSI "%df", cursor_position),
        ENTRY(CSI "%d;f", cursor_position),
        ENTRY(CSI ";%df", cursor_position_top_row),
        ENTRY(CSI "%d;%df", cursor_position),
        ENTRY(CSI "g", tab_clear),
        ENTRY(CSI "%dg", tab_clear),

        ENTRY(CSI "%mh", set_mode),
        ENTRY(CSI "?%mh", decset),

        ENTRY(CSI "%mi", media_copy),
        ENTRY(CSI "?%mi", dec_media_copy),

        ENTRY(CSI "%ml", reset_mode),
        ENTRY(CSI "?%ml", decreset),

        ENTRY(CSI "%mm", character_attributes),

        ENTRY(CSI "%dn", device_status_report),
        ENTRY(CSI "?%dn", dec_device_status_report),
        ENTRY(CSI "!p", soft_reset),
        ENTRY(CSI "%d;%d\"p", set_conformance_level),
        ENTRY(CSI " q", set_cursor_style),
        ENTRY(CSI "%d q", set_cursor_style),
        ENTRY(CSI "%d\"q", select_character_protection),

        ENTRY(CSI "r", set_scrolling_region),
        ENTRY(CSI ";r", set_scrolling_region),
        ENTRY(CSI ";%dr", set_scrolling_region_from_start),
        ENTRY(CSI "%dr", set_scrolling_region_to_end),
        ENTRY(CSI "%d;r", set_scrolling_region_to_end),
        ENTRY(CSI "%d;%dr", set_scrolling_region),

        ENTRY(CSI "?%mr", restore_mode),
        ENTRY(CSI "s", save_cursor),
        ENTRY(CSI "?%ms", save_mode),
        ENTRY(CSI "u", restore_cursor),

        ENTRY(CSI "%mt", window_manipulation),

        ENTRY(CSI "%d;%d;%d;%dw", enable_filter_rectangle),
        ENTRY(CSI "%dx", request_terminal_parameters),
        ENTRY(CSI "%d;%d'z", enable_locator_reporting),
        ENTRY(CSI "%m'{", select_locator_events),
        ENTRY(CSI "%d'|", request_locator_position),

        /* Set text parameters, BEL_terminated versions. */
        ENTRY(OSC ";%s" BEL, set_icon_and_window_title), /* undocumented default */
        ENTRY(OSC "0;%s" BEL, set_icon_and_window_title),
        ENTRY(OSC "1;%s" BEL, set_icon_title),
        ENTRY(OSC "2;%s" BEL, set_window_title),
        ENTRY(OSC "3;%s" BEL, set_xproperty),
        ENTRY(OSC "4;%s" BEL, change_color_bel),
        ENTRY(OSC "5;0;%s" BEL, change_bold_color_bel),
        ENTRY(OSC "6;%s" BEL, set_current_file_uri),
        ENTRY(OSC "7;%s" BEL, set_current_directory_uri),
        ENTRY(OSC "8;%s;%s" BEL, set_current_hyperlink),
        ENTRY(OSC "10;%s" BEL, change_foreground_color_bel),
        ENTRY(OSC "11;%s" BEL, change_background_color_bel),
        ENTRY(OSC "12;%s" BEL, change_cursor_background_color_bel),
        ENTRY(OSC "13;%s" BEL, change_mouse_cursor_foreground_color_bel),
        ENTRY(OSC "14;%s" BEL, change_mouse_cursor_background_color_bel),
        ENTRY(OSC "15;%s" BEL, change_tek_foreground_color_bel),
        ENTRY(OSC "16;%s" BEL, change_tek_background_color_bel),
        ENTRY(OSC "17;%s" BEL, change_highlight_background_color_bel),
        ENTRY(OSC "18;%s" BEL, change_tek_cursor_color_bel),
        ENTRY(OSC "19;%s" BEL, change_highlight_foreground_color_bel),
        ENTRY(OSC "46;%s" BEL, change_logfile),
        ENTRY(OSC "50;#%d" BEL, change_font_number),
        ENTRY(OSC "50;%s" BEL, change_font_name),
        ENTRY(OSC "104" BEL, reset_color),
        ENTRY(OSC "104;%m" BEL, reset_color),
        ENTRY(OSC "105" BEL, reset_bold_color),
        ENTRY(OSC "105;0" BEL, reset_bold_color),
        ENTRY(OSC "110" BEL, reset_foreground_color),
        ENTRY(OSC "111" BEL, reset_background_color),
        ENTRY(OSC "112" BEL, reset_cursor_background_color),
        ENTRY(OSC "113" BEL, reset_mouse_cursor_foreground_color),
        ENTRY(OSC "114" BEL, reset_mouse_cursor_background_color),
        ENTRY(OSC "115" BEL, reset_tek_foreground_color),
        ENTRY(OSC "116" BEL, reset_tek_background_color),
        ENTRY(OSC "117" BEL, reset_highlight_background_color),
        ENTRY(OSC "118" BEL, reset_tek_cursor_color),
        ENTRY(OSC "119" BEL, reset_highlight_foreground_color),
        ENTRY(OSC "133;%s" BEL, iterm2_133),
        ENTRY(OSC "777;%s" BEL, urxvt_777),
        ENTRY(OSC "1337;%s" BEL, iterm2_1337),

        /* Set text parameters, ST_terminated versions. */
        ENTRY(OSC ";%s" ST, set_icon_and_window_title), /* undocumented default */
        ENTRY(OSC "0;%s" ST, set_icon_and_window_title),
        ENTRY(OSC "1;%s" ST, set_icon_title),
        ENTRY(OSC "2;%s" ST, set_window_title),
        ENTRY(OSC "3;%s" ST, set_xproperty),
        ENTRY(OSC "4;%s" ST, change_color_st),
        ENTRY(OSC "5;0;%s" ST, change_bold_color_st),
        ENTRY(OSC "6;%s" ST, set_current_file_uri),
        ENTRY(OSC "7;%s" ST, set_current_directory_uri),
        ENTRY(OSC "8;%s;%s" ST, set_current_hyperlink),
        ENTRY(OSC "10;%s" ST, change_foreground_color_st),
        ENTRY(OSC "11;%s" ST, change_background_color_st),
        ENTRY(OSC "12;%s" ST, change_cursor_background_color_st),
        ENTRY(OSC "13;%s" ST, change_mouse_cursor_foreground_color_st),
        ENTRY(OSC "14;%s" ST, change_mouse_cursor_background_color_st),
        ENTRY(OSC "15;%s" ST, change_tek_foreground_color_st),
        ENTRY(OSC "16;%s" ST, change_tek_background_color_st),
        ENTRY(OSC "17;%s" ST, change_highlight_background_color_st),
        ENTRY(OSC "18;%s" ST, change_tek_cursor_color_st),
        ENTRY(OSC "19;%s" ST, change_highlight_foreground_color_st),
        ENTRY(OSC "46;%s" ST, change_logfile),
        ENTRY(OSC "50;#%d" ST, change_font_number),
        ENTRY(OSC "50;%s" ST, change_font_name),
        ENTRY(OSC "104" ST, reset_color),
        ENTRY(OSC "104;%m" ST, reset_color),
        ENTRY(OSC "105" ST, reset_bold_color),
        ENTRY(OSC "105;0" ST, reset_bold_color),
        ENTRY(OSC "110" ST, reset_foreground_color),
        ENTRY(OSC "111" ST, reset_background_color),
        ENTRY(OSC "112" ST, reset_cursor_background_color),
        ENTRY(OSC "113" ST, reset_mouse_cursor_foreground_color),
        ENTRY(OSC "114" ST, reset_mouse_cursor_background_color),
        ENTRY(OSC "115" ST, reset_tek_foreground_color),
        ENTRY(OSC "116" ST, reset_tek_background_color),
        ENTRY(OSC "117" ST, reset_highlight_background_color),
        ENTRY(OSC "118" ST, reset_tek_cursor_color),
        ENTRY(OSC "119" ST, reset_highlight_foreground_color),
        ENTRY(OSC "133;%s" ST, iterm2_133),
        ENTRY(OSC "777;%s" ST, urxvt_777),
        ENTRY(OSC "1337;%s" ST, iterm2_1337),

        /* These may be bogus, I can't find docs for them anywhere (#104154). */
        ENTRY(OSC "21;%s" BEL, set_text_property_21),
        ENTRY(OSC "2L;%s" BEL, set_text_property_2L),
        ENTRY(OSC "21;%s" ST, set_text_property_21),
        ENTRY(OSC "2L;%s" ST, set_text_property_2L),

        ENTRY(CSI "%m*y", checksum_rectangular_area),
};

#undef ENTRY

#undef ESC
#undef CSI
#undef ST
#undef OSC
#undef PM
#undef APC

#undef ENQ
#undef BEL
#undef BS
#undef TAB
#undef LF
#undef VT
#undef FF
#undef CR
#undef SO
#undef SI
#undef DEL
