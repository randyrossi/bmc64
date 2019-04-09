/*
 * DosBoxMenuUtility.java
 *
 * Written by
 *  Locnet <android.locnet@gmail.com>
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

package com.locnet.vice;

import org.ab.nativelayer.ImportView;
import org.ab.uae.FloppyImportView;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SubMenu;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.SeekBar;
import android.widget.Toast;

public class DosBoxMenuUtility {
    private static final String CONFIG_FILE = "sdl-vicerc";
    private static final String CONFIG_PATH = "/sdcard/";
    private static final String LICENSE_FILE = "gpl.txt";
    private static final String DATA_FILE = "sdl_sym.vkm";

    public static final int DEFAULT_MOUSE_SENSITIVITY = 20;
    public static final String DEFAULT_KEY_MAPPING = "abcxyzlr";

    public static final int KEYCODE_BUTTON_A = 96;
    public static final int KEYCODE_BUTTON_B = 97;
    public static final int KEYCODE_BUTTON_C = 98;
    public static final int KEYCODE_BUTTON_X = 99;
    public static final int KEYCODE_BUTTON_Y = 100;
    public static final int KEYCODE_BUTTON_Z = 101;
    public static final int KEYCODE_BUTTON_L1 = 102;
    public static final int KEYCODE_BUTTON_R1 = 103;
    public static final int KEYCODE_DPAD_UP_LEFT = KEYCODE_BUTTON_A;
    public static final int KEYCODE_DPAD_UP_RIGHT = KEYCODE_BUTTON_B;
    public static final int KEYCODE_DPAD_DOWN_LEFT = KEYCODE_BUTTON_X;
    public static final int KEYCODE_DPAD_DOWN_RIGHT = KEYCODE_BUTTON_Y;

    public static final int KEYCODE_PAGE_UP = 92;
    public static final int KEYCODE_ESCAPE = 111;
    public static final int KEYCODE_DELETE = 112;	 
    public static final int KEYCODE_INSERT = 124;
    public static final int KEYCODE_F1 = 131;

    //locnet, 2011-05-30, support more key
    public static final int KEYCODE_NUM_LOCK = 143;

    public static final int KEYCODE_NUMPAD_0 = 144;
    public static final int KEYCODE_NUMPAD_1 = 145;
    public static final int KEYCODE_NUMPAD_2 = 146;
    public static final int KEYCODE_NUMPAD_3 = 147;
    public static final int KEYCODE_NUMPAD_4 = 148;
    public static final int KEYCODE_NUMPAD_5 = 149;
    public static final int KEYCODE_NUMPAD_6 = 150;
    public static final int KEYCODE_NUMPAD_7 = 151;
    public static final int KEYCODE_NUMPAD_8 = 152;
    public static final int KEYCODE_NUMPAD_9 = 153;

    public final static int CONTEXT_MENU_SPECIAL_KEYS = 1;
    public final static int CONTEXT_MENU_CYCLES = 2;
    public final static int CONTEXT_MENU_FRAMESKIP = 3;
    public final static int CONTEXT_MENU_MEMORY_SIZE = 4;
    public final static int CONTEXT_MENU_JOYKEY = 5;

    private final static int MENU_QUIT = 2;
    private final static int MENU_FIT = 3;
    private final static int MENU_KEYBOARD_SPECIAL = 6;
    private final static int MENU_KEYBOARD = 7;

    private final static int MENU_SETTINGS_SCALE_FILTER = 13;
    private final static int MENU_SETTINGS_FRAMESKIP = 17;
    private final static int MENU_SETTINGS_SHOW_INFO = 19;
    private final static int MENU_SETTINGS_HELP = 21;
    private final static int MENU_SETTINGS_LICENSE = 22;
    private final static int MENU_SETTINGS_KEY_MAPPER = 23;
    private final static int MENU_SETTINGS_SCALE_FACTOR = 24;
    private final static int MENU_SETTINGS_MOUSE_SENSITIVITY = 26;
    private final static int MENU_SETTINGS_JOYKEY_SET = 27;
    private final static int MENU_SETTINGS_FAST_MOUSE = 28;
    private final static int MENU_SETTINGS_TRUE_DRIVE = 29;
    private final static int MENU_SETTINGS_BETTER_VIDEO = 30;

    private final static int MENU_INPUT_MOUSE = 41;
    private final static int MENU_INPUT_JOYSTICK = 42;
    private final static int MENU_INPUT_SCROLL = 43;	
    private final static int MENU_INPUT_HARDKEY = 44;	
    private final static int MENU_INPUT_INPUT_METHOD = 45;
    private final static int MENU_INPUT_TAP_CLICK = 46;
    private final static int MENU_INPUT_FORCE_KEYBOARD = 47;
    private final static int MENU_INPUT_USE_LEFT_ALT = 48;
    private final static int MENU_INPUT_PEN = 49;
    private final static int MENU_INPUT_KEYUP_DELAY = 50;
    private final static int MENU_INPUT_SWAP_JOYPORT = 55;

    private final static int MENU_KEYBOARD_CTRL = 61;
    private final static int MENU_KEYBOARD_ALT = 62;
    private final static int MENU_KEYBOARD_SHIFT = 63;

    private final static int MENU_KEYBOARD_ESC = 65;
    private final static int MENU_KEYBOARD_TAB = 66;
    private final static int MENU_KEYBOARD_NUMLOCK = 67;
    private final static int MENU_KEYBOARD_TURBO = 68;

    private final static int MENU_KEYBOARD_F1 = 70;
    private final static int MENU_KEYBOARD_F8 = 77;
    private final static int MENU_KEYBOARD_F12 = 81;

    private final static int MENU_KEYBOARD_PAGE_UP = 82;
    private final static int MENU_KEYBOARD_INSERT = 83;
    private final static int MENU_KEYBOARD_DELETE = 84;
    private final static int MENU_KEYBOARD_GRAVE = 85;

    private final static int MENU_BUTTON_SHOW = 91;
    private final static int MENU_BUTTON_HIDE = 92;
    private final static int MENU_BUTTON_OFF = 93;
    private final static int MENU_BUTTON_ALWAYS_SHOW_JOYSTICK = 94;
    private final static int MENU_BUTTON_LARGE_JOYSTICK = 95;

    private final static int MENU_CYCLE_1000 = 151;
    private final static int MENU_CYCLE_5000 = 159;

    private final static int MENU_FRAMESKIP_0 = 171;
    private final static int MENU_FRAMESKIP_10 = 181;

    private final static int MENU_MEMORY_1 = 201;
    private final static int MENU_MEMORY_2 = 202;
    private final static int MENU_MEMORY_4 = 203;
    private final static int MENU_MEMORY_8 = 204;

    public final static int MENU_JOYKEY_NONE = 301;
    public final static int MENU_JOYKEY_DPAD = 302;
    public final static int MENU_JOYKEY_WAXD = 303;
    public final static int MENU_JOYKEY_NUMPAD = 304;

    public final static int JOYKEY_SET_NONE = 0;
    public final static int JOYKEY_SET_DPAD = 1;
    public final static int JOYKEY_SET_WAXD = 2;
    public final static int JOYKEY_SET_NUMPAD = 3;

    private final static String PREF_KEY_SCALE_FILTER_ON = "pref_key_scale_filter_on";
    private final static String PREF_KEY_SCALE_FACTOR = "pref_key_scale_factor";
    private final static String PREF_KEY_SCALE_MODE = "pref_key_scale_mode";
    private final static String PREF_KEY_FRAMESKIP = "pref_key_frameskip";
    private final static String PREF_KEY_CYCLES = "pref_key_cycles";
    private final static String PREF_KEY_MEMORY_SIZE = "pref_key_memory_size";
    private final static String PREF_KEY_HARDKEY_ON = "pref_key_hardkey_on";
    private final static String PREF_KEY_KEY_MAPPING = "pref_key_key_mapping";
    private final static String PREF_KEY_TAP_CLICK_ON = "pref_key_tap_click_on";
    private final static String PREF_KEY_FORCE_KEYBOARD_ON = "pref_key_force_keyboard_on";
    private final static String PREF_KEY_MOUSE_SENSITIVITY = "pref_key_mouse_sensitivity";
    private final static String PREF_KEY_USE_LEFT_ALT_ON = "pref_key_left_alt_on";
    private final static String PREF_KEY_JOYKEY_SET = "pref_key_joykey_set";
    private final static String PREF_KEY_SHOW_INFO = "pref_key_show_info";
    private final static String PREF_KEY_ALWAYS_SHOW_JOYSTICK = "pref_key_always_show_joystick";
    private final static String PREF_KEY_LARGE_JOYSTICK = "pref_key_large_joystick";
    private final static String PREF_KEY_SCREEN_RESIZE = "pref_key_screen_resize";
    private final static String PREF_KEY_BUTTON_STATE = "pref_key_button_state";
    private final static String PREF_KEY_FAST_MOUSE_ON = "pref_key_fast_mouse_on";
    private final static String PREF_KEY_KEYUP_DELAY = "pref_key_keyup_delay";
    private final static String PREF_KEY_BETTER_VIDEO_ON = "pref_key_better_video_on";

    //following must sync with AndroidOSfunc.cpp
    public final static int DOSBOX_OPTION_ID_SOUND_MODULE_ON = 1;
    public final static int DOSBOX_OPTION_ID_MEMORY_SIZE = 2;
    public final static int DOSBOX_OPTION_ID_CYCLES = 10;
    public final static int DOSBOX_OPTION_ID_FRAMESKIP = 11;
    public final static int DOSBOX_OPTION_ID_TURBO_ON = 14;
    public final static int DOSBOX_OPTION_ID_SOUND_ON = 16;
    public final static int DOSBOX_OPTION_ID_SHOW_INFO_ON = 17;
    public final static int DOSBOX_OPTION_ID_CHANGE_ROM = 51;
    public final static int DOSBOX_OPTION_ID_CHANGE_DISK = 52;
    public final static int DOSBOX_OPTION_ID_NTSC = 53;
    public final static int DOSBOX_OPTION_ID_FLOPPY_COUNT = 54;
    public final static int DOSBOX_OPTION_ID_TRUE_DRIVE_ON = 56;
    public final static int DOSBOX_OPTION_ID_SWAP_JOYPORT = 57;
    public final static int DOSBOX_OPTION_ID_SCREEN_BORDER_ON = 58;
    public final static int DOSBOX_OPTION_ID_TAPE_CONTROL = 59;

    static public void loadPreference(DosBoxLauncher context) {
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (sharedPrefs != null) {
            context.mPrefScaleFilterOn = sharedPrefs.getBoolean(PREF_KEY_SCALE_FILTER_ON, false);
            context.mPrefScaleFactor = sharedPrefs.getInt(PREF_KEY_SCALE_FACTOR, 100);
            context.mPrefScaleMode = sharedPrefs.getInt(PREF_KEY_SCALE_MODE, DosBoxSurfaceView.SCALE_MODE_CUSTOM);
            context.mPrefFrameskip = sharedPrefs.getInt(PREF_KEY_FRAMESKIP, 0);
            context.mPrefCycles = sharedPrefs.getInt(PREF_KEY_CYCLES, 0);
            context.mPrefMemorySize = sharedPrefs.getInt(PREF_KEY_MEMORY_SIZE, 4);
            context.mPrefHardkeyOn = sharedPrefs.getBoolean(PREF_KEY_HARDKEY_ON, true);
            context.mPrefKeyMapping = sharedPrefs.getString(PREF_KEY_KEY_MAPPING, DEFAULT_KEY_MAPPING);
            context.mPrefMouseSensitivity = sharedPrefs.getInt(PREF_KEY_MOUSE_SENSITIVITY, DEFAULT_MOUSE_SENSITIVITY);
            context.mPrefTapClickOn = sharedPrefs.getBoolean(PREF_KEY_TAP_CLICK_ON, true);
            context.mPrefForceKeyboardOn = sharedPrefs.getBoolean(PREF_KEY_FORCE_KEYBOARD_ON, false);
            context.mPrefUseLeftAltOn = sharedPrefs.getBoolean(PREF_KEY_USE_LEFT_ALT_ON, true);
            context.mPrefJoyKeySet = sharedPrefs.getInt(PREF_KEY_JOYKEY_SET, JOYKEY_SET_DPAD);
            context.mPrefShowInfoOn = sharedPrefs.getBoolean(PREF_KEY_SHOW_INFO, false);
            context.mPrefAlwaysShowJoystick = sharedPrefs.getBoolean(PREF_KEY_ALWAYS_SHOW_JOYSTICK, false);
            context.mPrefLargeJoystick = sharedPrefs.getBoolean(PREF_KEY_LARGE_JOYSTICK, false);
            context.mPrefFastMouseOn = sharedPrefs.getBoolean(PREF_KEY_FAST_MOUSE_ON, false);
            context.mPrefKeyUpDelay = sharedPrefs.getBoolean(PREF_KEY_KEYUP_DELAY, true);
            context.mPrefBetterVideoOn = sharedPrefs.getBoolean(PREF_KEY_BETTER_VIDEO_ON, false);

            context.mSurfaceView.mScale = sharedPrefs.getBoolean(PREF_KEY_SCREEN_RESIZE, true);
            {
                int buttonState = sharedPrefs.getInt(PREF_KEY_BUTTON_STATE, MENU_BUTTON_HIDE);
                switch (buttonState) {
                    case MENU_BUTTON_SHOW:
                        context.mPrefOnscreenButtonOn = true;
                        context.mSurfaceView.mShowInfo = true;
                        break;
                    case MENU_BUTTON_OFF:
                        context.mPrefOnscreenButtonOn = false;
                        context.mSurfaceView.mShowInfo = false;
                        break;
                    case MENU_BUTTON_HIDE:
                    default:
                        context.mPrefOnscreenButtonOn = true;
                        context.mSurfaceView.mShowInfo = false;
                        break;
                }
            }
            context.mMouseScale = getMouseScale(context.mPrefMouseSensitivity);
        }
    }

    static public void copyConfigFile(DosBoxLauncher context) {
        copyFile(context, CONFIG_PATH, CONFIG_FILE, CONFIG_FILE);
    }

    static public void copyAssetFile(PreConfig context, String file, String targetFile, String targetPath) {
        try {
            InputStream myInput = new FileInputStream(targetPath + targetFile);
            myInput.close();
            myInput = null;
        }
        catch (FileNotFoundException f) {
            try {
                InputStream myInput = context.getAssets().open(file);

                File myDirectory = new File(targetPath);
                myDirectory.mkdirs();
                File outputFile = new File(myDirectory, targetFile);
                FileOutputStream myOutput = new FileOutputStream(outputFile);
                byte[] buffer = new byte[1024];
                int length;
                while ((length = myInput.read(buffer)) > 0) {
                    myOutput.write(buffer, 0, length);
                }
                myOutput.flush();
                myOutput.close();
                myInput.close();
            }
            catch (IOException e) {
            }
        }
        catch (IOException e) {
        }
    }

    static public void copyDataFile(DosBoxLauncher context, String targetPath) {
        copyFile(context, targetPath, DATA_FILE, DATA_FILE);
    }

    static public void copyLicenseFile(DosBoxLauncher context) {
        copyFile(context, CONFIG_PATH, LICENSE_FILE, LICENSE_FILE);
    }

    static private void copyFile(DosBoxLauncher context, String targetPath, String file, String targetFile) {
        try {
            InputStream myInput = new FileInputStream(targetPath + targetFile);
            myInput.close();
            myInput = null;
        }
        catch (FileNotFoundException f) {
            try {
                InputStream myInput = context.getAssets().open(file);

                File myDirectory = new File(targetPath);
                myDirectory.mkdirs();
                File outputFile = new File(myDirectory, targetFile);
                FileOutputStream myOutput = new FileOutputStream(outputFile);
                byte[] buffer = new byte[1024];
                int length;
                while ((length = myInput.read(buffer)) > 0) {
                    myOutput.write(buffer, 0, length);
                }
                myOutput.flush();
                myOutput.close();
                myInput.close();
            }
            catch (IOException e) {
            }
        }
        catch (IOException e) {
        }
    }

    static public void savePreference(DosBoxLauncher context, String key) {
        SharedPreferences sharedPrefs = PreferenceManager.getDefaultSharedPreferences(context);

        if (sharedPrefs != null) {
            SharedPreferences.Editor editor = sharedPrefs.edit();
            if (editor != null) {
                if (PREF_KEY_SCALE_FILTER_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_SCALE_FILTER_ON, context.mPrefScaleFilterOn);
                } else if (PREF_KEY_SCALE_FACTOR.equals(key)) {
                    editor.putInt(PREF_KEY_SCALE_FACTOR, context.mPrefScaleFactor);
                } else if (PREF_KEY_SCALE_MODE.equals(key)) {
                    editor.putInt(PREF_KEY_SCALE_MODE, context.mPrefScaleMode);
                } else if (PREF_KEY_FRAMESKIP.equals(key)) {
                    editor.putInt(PREF_KEY_FRAMESKIP, context.mPrefFrameskip);
                } else if (PREF_KEY_CYCLES.equals(key)) {
                    editor.putInt(PREF_KEY_CYCLES, context.mPrefCycles);
                } else if (PREF_KEY_MEMORY_SIZE.equals(key)) {
                    editor.putInt(PREF_KEY_MEMORY_SIZE, context.mPrefMemorySize);
                } else if (PREF_KEY_HARDKEY_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_HARDKEY_ON, context.mPrefHardkeyOn);
                } else if (PREF_KEY_KEY_MAPPING.equals(key)) {
                    editor.putString(PREF_KEY_KEY_MAPPING, context.mPrefKeyMapping);
                } else if (PREF_KEY_MOUSE_SENSITIVITY.equals(key)) {
                    editor.putInt(PREF_KEY_MOUSE_SENSITIVITY, context.mPrefMouseSensitivity);
                    context.mMouseScale = getMouseScale(context.mPrefMouseSensitivity);
                } else if (PREF_KEY_TAP_CLICK_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_TAP_CLICK_ON, context.mPrefTapClickOn);
                } else if (PREF_KEY_FORCE_KEYBOARD_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_FORCE_KEYBOARD_ON, context.mPrefForceKeyboardOn);
                } else if (PREF_KEY_USE_LEFT_ALT_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_USE_LEFT_ALT_ON, context.mPrefUseLeftAltOn);
                } else if (PREF_KEY_FAST_MOUSE_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_FAST_MOUSE_ON, context.mPrefFastMouseOn);
                } else if (PREF_KEY_KEYUP_DELAY.equals(key)) {
                    editor.putBoolean(PREF_KEY_KEYUP_DELAY, context.mPrefKeyUpDelay);
                } else if (PREF_KEY_SCREEN_RESIZE.equals(key)) {
                    editor.putBoolean(PREF_KEY_SCREEN_RESIZE, context.mSurfaceView.mScale);
                } else if (PREF_KEY_BUTTON_STATE.equals(key)) {
                    int buttonState;
                    if (!context.mPrefOnscreenButtonOn) {
                        buttonState = MENU_BUTTON_OFF; //off
                    } else if (context.mSurfaceView.mShowInfo) {
                        buttonState = MENU_BUTTON_SHOW; //show
                    } else {
                        buttonState = MENU_BUTTON_HIDE; //hide
                    }
                    editor.putInt(PREF_KEY_BUTTON_STATE, buttonState);
                } else if (PREF_KEY_JOYKEY_SET.equals(key)) {
                    editor.putInt(PREF_KEY_JOYKEY_SET, context.mPrefJoyKeySet);
                } else if (PREF_KEY_SHOW_INFO.equals(key)) {
                    editor.putBoolean(PREF_KEY_SHOW_INFO, context.mPrefShowInfoOn);
                } else if (PREF_KEY_ALWAYS_SHOW_JOYSTICK.equals(key)) {
                    editor.putBoolean(PREF_KEY_ALWAYS_SHOW_JOYSTICK, context.mPrefAlwaysShowJoystick);
                } else if (PREF_KEY_LARGE_JOYSTICK.equals(key)) {
                    editor.putBoolean(PREF_KEY_LARGE_JOYSTICK, context.mPrefLargeJoystick);
                } else if (PREF_KEY_BETTER_VIDEO_ON.equals(key)) {
                    editor.putBoolean(PREF_KEY_BETTER_VIDEO_ON, context.mPrefBetterVideoOn);
                }
                editor.commit();
            }
        }
    }

    static public boolean doCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_KEYBOARD_SPECIAL, 0, "Special Keys");

        menu.add(0, MENU_FIT, 0, "Resize");
        SubMenu inputModeMenu = menu.addSubMenu("Input Mode");
        inputModeMenu.add(1, MENU_INPUT_MOUSE, 0, "Trackpad").setChecked(true);
        inputModeMenu.add(1, MENU_INPUT_JOYSTICK, 0, "Joystick");
        inputModeMenu.add(1, MENU_INPUT_SCROLL, 0, "Scroll Screen");
        inputModeMenu.setGroupCheckable(1, true, true);

        inputModeMenu.add(0, MENU_INPUT_SWAP_JOYPORT, 0, "Swap Joystick Ports");

        inputModeMenu.add(0, MENU_INPUT_HARDKEY, 0, "Vol. Up/Down as button").setCheckable(true);		
        inputModeMenu.add(0, MENU_INPUT_USE_LEFT_ALT, 0, "Map Left-Alt Key").setCheckable(true);
        inputModeMenu.add(0, MENU_INPUT_TAP_CLICK, 0, "Tap Screen as click").setCheckable(true);
        inputModeMenu.add(0, MENU_INPUT_INPUT_METHOD, 0, "Input Method");
        inputModeMenu.add(0, MENU_INPUT_FORCE_KEYBOARD, 0, "Show Keyboard Fix").setCheckable(true);
        inputModeMenu.add(0, MENU_INPUT_KEYUP_DELAY, 0, "Keypress Fix").setCheckable(true);

        SubMenu buttonMenu = menu.addSubMenu("Buttons");		
        buttonMenu.add(1, MENU_BUTTON_HIDE, 0, "Hide Buttons");
        buttonMenu.add(1, MENU_BUTTON_SHOW, 0, "Show Buttons (Slower)").setChecked(true);
        buttonMenu.add(1, MENU_BUTTON_OFF, 0, "Disable Buttons");		
        buttonMenu.setGroupCheckable(1, true, true);
        buttonMenu.add(0, MENU_BUTTON_ALWAYS_SHOW_JOYSTICK, 0, "Always Show Joystick").setCheckable(true);		
        buttonMenu.add(0, MENU_BUTTON_LARGE_JOYSTICK, 0, "Large Joystick").setCheckable(true);		

        SubMenu settingMenu = menu.addSubMenu("Settings");
        settingMenu.add(0, MENU_SETTINGS_FRAMESKIP, 0, "Frameskip");
        settingMenu.add(0, MENU_SETTINGS_BETTER_VIDEO, 0, "Better Video").setCheckable(true);
        settingMenu.add(0, MENU_SETTINGS_SHOW_INFO, 0, "Show Info").setCheckable(true);
        settingMenu.add(0, MENU_SETTINGS_TRUE_DRIVE, 0, "True Drive Emulation").setCheckable(true);
        settingMenu.add(0, MENU_SETTINGS_SCALE_FILTER, 0, "Smooth Resize (Slower)").setCheckable(true);
        settingMenu.add(0, MENU_SETTINGS_SCALE_FACTOR, 0, "Resize Mode");
        settingMenu.add(0, MENU_SETTINGS_FAST_MOUSE, 0, "Better Mouse (Slower)").setCheckable(true);
        settingMenu.add(0, MENU_SETTINGS_MOUSE_SENSITIVITY, 0, "Mouse Sensitivity");
        settingMenu.add(0, MENU_SETTINGS_JOYKEY_SET, 0, "Joystick Keys");
        settingMenu.add(0, MENU_SETTINGS_KEY_MAPPER, 0, "Button Mapper");		
        settingMenu.add(0, MENU_SETTINGS_HELP, 0, "Help");		
        settingMenu.add(0, MENU_SETTINGS_LICENSE, 0, "Save license to /sdcard/gpl.txt");		

        menu.add(0, MENU_KEYBOARD, 0, "Keyboard");		
        return true;
    }
	
    static public boolean doPrepareOptionsMenu (DosBoxLauncher context, Menu menu) {
        switch (context.mSurfaceView.mInputMode) {
            case DosBoxSurfaceView.INPUT_MODE_JOYSTICK:
                menu.findItem(MENU_INPUT_JOYSTICK).setChecked(true);		
                break;
            case DosBoxSurfaceView.INPUT_MODE_SCROLL:
                menu.findItem(MENU_INPUT_SCROLL).setChecked(true);		
                break;
            case DosBoxSurfaceView.INPUT_MODE_PEN:
                menu.findItem(MENU_INPUT_PEN).setChecked(true);		
                break;
            case DosBoxSurfaceView.INPUT_MODE_MOUSE:
            default:
                menu.findItem(MENU_INPUT_MOUSE).setChecked(true);		
                break;
        }

        menu.findItem(MENU_INPUT_HARDKEY).setChecked(context.mPrefHardkeyOn);		
        menu.findItem(MENU_INPUT_TAP_CLICK).setChecked(context.mPrefTapClickOn);		
        menu.findItem(MENU_INPUT_FORCE_KEYBOARD).setChecked(context.mPrefForceKeyboardOn);		
        menu.findItem(MENU_INPUT_USE_LEFT_ALT).setChecked(context.mPrefUseLeftAltOn);		
        menu.findItem(MENU_INPUT_KEYUP_DELAY).setChecked(!context.mPrefKeyUpDelay);		

        if (!context.mPrefOnscreenButtonOn) {
            menu.findItem(MENU_BUTTON_OFF).setChecked(true);
        } else if (context.mSurfaceView.mShowInfo) {
            menu.findItem(MENU_BUTTON_SHOW).setChecked(true);
        } else {
            menu.findItem(MENU_BUTTON_HIDE).setChecked(true);
        }
        menu.findItem(MENU_BUTTON_ALWAYS_SHOW_JOYSTICK).setChecked(context.mPrefAlwaysShowJoystick);									
        menu.findItem(MENU_BUTTON_LARGE_JOYSTICK).setChecked(context.mPrefLargeJoystick);									

        menu.findItem(MENU_FIT).setTitle((context.mSurfaceView.mScale) ? "Resize: On" : "Resize: Off");		
        menu.findItem(MENU_SETTINGS_SCALE_FILTER).setChecked(context.mPrefScaleFilterOn);
        menu.findItem(MENU_SETTINGS_SHOW_INFO).setChecked(context.mPrefShowInfoOn);
        menu.findItem(MENU_SETTINGS_FAST_MOUSE).setChecked(context.mPrefFastMouseOn);		
        context.mPrefTrueDriveOn = (DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_TRUE_DRIVE_ON, 0, null) != 0);
        menu.findItem(MENU_SETTINGS_TRUE_DRIVE).setChecked(context.mPrefTrueDriveOn);
        menu.findItem(MENU_SETTINGS_BETTER_VIDEO).setChecked(context.mPrefBetterVideoOn);

        return true;
    }

    static public void doShowKeyboard(DosBoxLauncher context) {
        InputMethodManager imm = (InputMethodManager)context.getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm != null) {
            imm.showSoftInput(context.mSurfaceView, context.mPrefForceKeyboardOn ? InputMethodManager.SHOW_FORCED : 0);
        }
    }

    static public void doShowInputMethodPicker(DosBoxLauncher context) {
        InputMethodManager imm = (InputMethodManager)context.getSystemService(Context.INPUT_METHOD_SERVICE);
        if (imm != null) {
            imm.showInputMethodPicker();
        }
    }

    static private void setFloppyHandler(final DosBoxLauncher context, final AlertDialog dlg, final View parent, int id, final int requestCode) {
        Button floppyButton;

        floppyButton = (Button)parent.findViewById(id);
        if (floppyButton != null) {
            floppyButton.setEnabled(true);

            floppyButton.setOnClickListener(new Button.OnClickListener() {
                public void onClick(View view) {
                    Intent settingsIntent = new Intent();
                    settingsIntent.setClass(context, ImportView.class);
                    settingsIntent.putExtra("import", new FloppyImportView());
                    context.startActivityForResult(settingsIntent, requestCode);
                    dlg.dismiss();
                }
            });
        }
    }

    static private void setSaveStateHandler(final AlertDialog dlg, final View parent, int id, final boolean save, final String saveStatePrefix, final int slot) {
        Button floppyButton;

        floppyButton = (Button)parent.findViewById(id);
        if (floppyButton != null) {
            floppyButton.setEnabled(true);

            floppyButton.setOnClickListener(new Button.OnClickListener() {
                public void onClick(View view) {
                    if ((saveStatePrefix != null) && (saveStatePrefix.length() > 0)) {
                        if (save) {
                            DosBoxLauncher.nativeSaveState(saveStatePrefix, slot);
                        } else {
                            DosBoxLauncher.nativeLoadState(saveStatePrefix, slot);
                        }
                    }
                    dlg.dismiss();
                }
            });
        }
    }

    static private void setTapeControlHandler(final AlertDialog dlg, final View parent, int id, final int control) {
        Button tapeButton;

        tapeButton = (Button)parent.findViewById(id);
        if (tapeButton != null) {
            tapeButton.setEnabled(true);

            tapeButton.setOnClickListener(new Button.OnClickListener() {
                public void onClick(View view) {
                    DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_TAPE_CONTROL, control, null);
                    dlg.dismiss();
                }
            });
        }
    }

    static public void doConfirmQuit(final DosBoxLauncher context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        String saveStatePrefix = sp.getString(Globals.PREFKEY_F1, null);
        //TODO
        int floppyCount = 4;
        if ((saveStatePrefix != null) && (saveStatePrefix.length() > 4) && (saveStatePrefix.charAt(saveStatePrefix.length()-4) == '.')) {
            saveStatePrefix = saveStatePrefix.substring(0, saveStatePrefix.length() - 4);
        } else {
            saveStatePrefix = "/sdcard/anvice-savestate";
        }
        //TODO
        boolean enableSaveState = true;
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.app_name);

        final View view = context.getLayoutInflater().inflate(R.layout.system, null);

        builder.setView(view);

        builder.setPositiveButton("Power off", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                context.stopDosBox();
            }
        });

        builder.setNegativeButton("Cancel", null);				
        final AlertDialog dlg = builder.create();

        if (enableSaveState) {
            setSaveStateHandler(dlg, view, R.id.save_state1, true, saveStatePrefix, 1);
            setSaveStateHandler(dlg, view, R.id.load_state1, false, saveStatePrefix, 1);
            setSaveStateHandler(dlg, view, R.id.save_state2, true, saveStatePrefix, 2);
            setSaveStateHandler(dlg, view, R.id.load_state2, false, saveStatePrefix, 2);
            setSaveStateHandler(dlg, view, R.id.save_state3, true, saveStatePrefix, 3);
            setSaveStateHandler(dlg, view, R.id.load_state3, false, saveStatePrefix, 3);
        }

        if (floppyCount >= 1) {
            setFloppyHandler(context, dlg, view, R.id.change_floppy1, Globals.PREFKEY_F1_INT);
        }
        if (floppyCount >= 2) {
            setFloppyHandler(context, dlg, view, R.id.change_floppy2, Globals.PREFKEY_F2_INT);
        }
        if (floppyCount >= 3) {
            setFloppyHandler(context, dlg, view, R.id.change_floppy3, Globals.PREFKEY_F3_INT);
        }
        if (floppyCount >= 4) {
            setFloppyHandler(context, dlg, view, R.id.change_floppy4, Globals.PREFKEY_F4_INT);
        }

        setTapeControlHandler(dlg, view, R.id.tape_stop, 0);
        setTapeControlHandler(dlg, view, R.id.tape_start, 1);
        setTapeControlHandler(dlg, view, R.id.tape_forward, 2);
        setTapeControlHandler(dlg, view, R.id.tape_rewind, 3);

        dlg.show();
    }

    static public int doGetMapperChar(DosBoxLauncher context, int index) {
        String chr = doGetMapperString(context, index);

        if (chr.length() == 1) {
            return chr.charAt(0);
        } else {
            return '\n';
        }
    }

    static private String doGetMapperString(DosBoxLauncher context, int index) {
        if (index >= context.mPrefKeyMapping.length()) {
            return String.valueOf((char)DEFAULT_KEY_MAPPING.charAt(index));
        }

        String chr = String.valueOf((char)context.mPrefKeyMapping.charAt(index));

        if (chr.equals("\n")) {
            return "";
        } else {
            return chr;
        }
    }

    static private String getMapperKeyFromUI(View view, int id) {
        String chr = ((EditText)(view.findViewById(id))).getText().toString();

        if (chr.length() != 1) {
            return "\n";
        } else {
            return chr;
        }
    }
	
    static public void doKeyMapper(final DosBoxLauncher context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        final View view = context.getLayoutInflater().inflate(R.layout.keymapper, null);

        ((EditText)(view.findViewById(R.id.btn_a))).setText(doGetMapperString(context, 0));
        ((EditText)(view.findViewById(R.id.btn_b))).setText(doGetMapperString(context, 1));
        ((EditText)(view.findViewById(R.id.btn_c))).setText(doGetMapperString(context, 2));
        ((EditText)(view.findViewById(R.id.btn_x))).setText(doGetMapperString(context, 3));
        ((EditText)(view.findViewById(R.id.btn_y))).setText(doGetMapperString(context, 4));
        ((EditText)(view.findViewById(R.id.btn_z))).setText(doGetMapperString(context, 5));
        ((EditText)(view.findViewById(R.id.btn_lb))).setText(doGetMapperString(context, 6));
        ((EditText)(view.findViewById(R.id.btn_rb))).setText(doGetMapperString(context, 7));

        builder.setView(view);
        builder.setTitle(R.string.keymapper);

        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                String chr_a = getMapperKeyFromUI(view, R.id.btn_a);
                String chr_b = getMapperKeyFromUI(view, R.id.btn_b);
                String chr_c = getMapperKeyFromUI(view, R.id.btn_c);
                String chr_x = getMapperKeyFromUI(view, R.id.btn_x);
                String chr_y = getMapperKeyFromUI(view, R.id.btn_y);
                String chr_z = getMapperKeyFromUI(view, R.id.btn_z);
                String chr_lb = getMapperKeyFromUI(view, R.id.btn_lb);
                String chr_rb = getMapperKeyFromUI(view, R.id.btn_rb);

                context.mPrefKeyMapping = chr_a + chr_b + chr_c + chr_x + chr_y + chr_z + chr_lb + chr_rb;

                savePreference(context, PREF_KEY_KEY_MAPPING);				
            }
        });
        builder.setNegativeButton("Cancel", null);
        builder.create().show();		
    }

    static public void doShowTextDialog(final DosBoxLauncher context, String message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.app_name);
        builder.setMessage(message);

        builder.setPositiveButton("OK", null);

        builder.create().show();
    }

    static private String loadText(DosBoxLauncher context, String file) {
        try {
            InputStream inputStream = context.getAssets().open(file);
            ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();

            int i;

            i = inputStream.read();
            while (i != -1) {
                byteArrayOutputStream.write(i);
                i = inputStream.read();
            }
            inputStream.close();

            return byteArrayOutputStream.toString();
        }
        catch (IOException e) {
        }
        return "failed to load " + file;
    }

    static private void showRestartMessage(DosBoxLauncher context) {
        Toast.makeText(context, "restart to take effect", Toast.LENGTH_LONG).show();		
    }

    static public void doShowHideInfo(DosBoxLauncher context, boolean show) {
        context.mSurfaceView.forceRedraw();
        savePreference(context, DosBoxMenuUtility.PREF_KEY_BUTTON_STATE);
    }

    static public void doSetScaleFactor(final DosBoxLauncher context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        final View view = context.getLayoutInflater().inflate(R.layout.scale, null);

        final RadioButton scale_fit_screen = (RadioButton)view.findViewById(R.id.scale_fit_screen);
        final RadioButton scale_fit_width = (RadioButton)view.findViewById(R.id.scale_fit_width);
        final RadioButton scale_fit_height = (RadioButton)view.findViewById(R.id.scale_fit_height);
        final RadioButton scale_fullscreen = (RadioButton)view.findViewById(R.id.scale_fullscreen);
        final RadioButton scale_custom = (RadioButton)view.findViewById(R.id.scale_custom);
        final SeekBar slider = (SeekBar) view.findViewById(R.id.slider_seekbar);

        slider.setMax(100);
        slider.setProgress(context.mPrefScaleFactor);

        switch (context.mPrefScaleMode) {
            case DosBoxSurfaceView.SCALE_MODE_FIT_WIDTH:
                scale_fit_width.setChecked(true);
                break;
            case DosBoxSurfaceView.SCALE_MODE_FIT_HEIGHT:
                scale_fit_height.setChecked(true);
                break;
            case DosBoxSurfaceView.SCALE_MODE_FULLSCREEN:
                scale_fullscreen.setChecked(true);
                break;
            case DosBoxSurfaceView.SCALE_MODE_FIT_SCREEN:
                scale_fit_screen.setChecked(true);
                break;
            case DosBoxSurfaceView.SCALE_MODE_CUSTOM:
            default:
                scale_custom.setChecked(true);
                break;
        }

        builder.setView(view);
        builder.setTitle(R.string.scale_factor);

        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                if (scale_fit_width.isChecked()) {
                    context.mPrefScaleMode = DosBoxSurfaceView.SCALE_MODE_FIT_WIDTH;
                }
                if (scale_fit_height.isChecked()) {
                    context.mPrefScaleMode = DosBoxSurfaceView.SCALE_MODE_FIT_HEIGHT;
                }
                if (scale_fullscreen.isChecked()) {
                    context.mPrefScaleMode = DosBoxSurfaceView.SCALE_MODE_FULLSCREEN;
                }
                if (scale_fit_screen.isChecked()) {
                    context.mPrefScaleMode = DosBoxSurfaceView.SCALE_MODE_FIT_SCREEN;
                }
                if (scale_custom.isChecked()) {
                    context.mPrefScaleMode = DosBoxSurfaceView.SCALE_MODE_CUSTOM;
                }

                context.mPrefScaleFactor = slider.getProgress();
                context.mSurfaceView.forceRedraw();

                savePreference(context, PREF_KEY_SCALE_FACTOR);				
                savePreference(context, PREF_KEY_SCALE_MODE);				
            }
        });
        builder.setNegativeButton("Cancel", null);
        builder.create().show();				
    }

    static public void doSetMouseSensitivity(final DosBoxLauncher context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        final View view = context.getLayoutInflater().inflate(R.layout.slider, null);

        final SeekBar slider = (SeekBar) view.findViewById(R.id.slider_seekbar);
        slider.setMax(40);
        slider.setProgress(context.mPrefMouseSensitivity);

        builder.setView(view);
        builder.setTitle(R.string.mouse_sensitivity);

        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                context.mPrefMouseSensitivity = slider.getProgress();
                savePreference(context, PREF_KEY_MOUSE_SENSITIVITY);
            }
        });

        builder.setNeutralButton("Default", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                context.mPrefMouseSensitivity = DEFAULT_MOUSE_SENSITIVITY;
                savePreference(context, PREF_KEY_MOUSE_SENSITIVITY);
            }
        });

        builder.setNegativeButton("Cancel", null);
        builder.create().show();
    }

    static private float getMouseScale(int mouseSensitivity) {
        if (mouseSensitivity >= DEFAULT_MOUSE_SENSITIVITY) {
            return (mouseSensitivity - (DEFAULT_MOUSE_SENSITIVITY - 10)) / 10.0f;
        } else {
            return 10.0f / ((DEFAULT_MOUSE_SENSITIVITY + 10) - mouseSensitivity);
        }
    }

    static public void doSetInputMode(DosBoxLauncher context, int mode) {
        context.mSurfaceView.mInputMode = mode;
    }

    static public boolean doOptionsItemSelected(DosBoxLauncher context, MenuItem item) {
        switch(item.getItemId()) {
            case MENU_QUIT:
                doConfirmQuit(context);
                break;
            case MENU_FIT:
                context.mSurfaceView.mScale = !context.mSurfaceView.mScale;
                context.mSurfaceView.forceRedraw();
                savePreference(context, PREF_KEY_SCREEN_RESIZE);
                break;
            case MENU_INPUT_MOUSE:
                doSetInputMode(context, DosBoxSurfaceView.INPUT_MODE_MOUSE);
                doShowHideInfo(context, false);
                break;
            case MENU_INPUT_PEN:
                doSetInputMode(context, DosBoxSurfaceView.INPUT_MODE_PEN);
                //locnet, 2011-06-14, auto calibrate absolute mouse
                DosBoxControl.nativeMouse(-2048, -2048, -1, -1, DosBoxSurfaceView.MOUSE_ACTION_MOVE, -1);
                try {
                    Thread.sleep(DosBoxSurfaceView.BUTTON_TAP_DELAY);
                }
                catch (InterruptedException e) {
                }
                //locnet, 2011-06-14, set mouse to center
                DosBoxControl.nativeMouse(0, 0, 500, 500, DosBoxSurfaceView.MOUSE_ACTION_DOWN, -1);
                doShowHideInfo(context, false);
                break;
            case MENU_INPUT_JOYSTICK:
                doSetInputMode(context, DosBoxSurfaceView.INPUT_MODE_JOYSTICK);
                doShowHideInfo(context, false);
                break;
            case MENU_INPUT_SWAP_JOYPORT:
                context.mJoyPort = 1 - context.mJoyPort;
                DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_SWAP_JOYPORT, context.mJoyPort, null);
                break;
            case MENU_INPUT_SCROLL:
                doSetInputMode(context, DosBoxSurfaceView.INPUT_MODE_SCROLL);
                doShowHideInfo(context, false);
                break;
            case MENU_INPUT_HARDKEY:
                context.mPrefHardkeyOn = !context.mPrefHardkeyOn;
                savePreference(context, PREF_KEY_HARDKEY_ON);
                break;
            case MENU_INPUT_TAP_CLICK:
                context.mPrefTapClickOn = !context.mPrefTapClickOn;
                savePreference(context, PREF_KEY_TAP_CLICK_ON);
                break;
            case MENU_INPUT_INPUT_METHOD:
                doShowInputMethodPicker(context);
                break;
            case MENU_INPUT_FORCE_KEYBOARD:
                context.mPrefForceKeyboardOn = !context.mPrefForceKeyboardOn;
                savePreference(context, PREF_KEY_FORCE_KEYBOARD_ON);
                break;
            case MENU_INPUT_USE_LEFT_ALT:
                context.mPrefUseLeftAltOn = !context.mPrefUseLeftAltOn;
                savePreference(context, PREF_KEY_USE_LEFT_ALT_ON);
                break;
            case MENU_INPUT_KEYUP_DELAY:
                context.mPrefKeyUpDelay = !context.mPrefKeyUpDelay;
                savePreference(context, PREF_KEY_KEYUP_DELAY);
                break;
            case MENU_BUTTON_SHOW:
                context.mPrefOnscreenButtonOn = true;
                context.mSurfaceView.mShowInfo = true;
                context.mSurfaceView.setJoystickRect();
                doShowHideInfo(context, true);
                break;
            case MENU_BUTTON_HIDE:
                context.mPrefOnscreenButtonOn = true;
                context.mSurfaceView.mShowInfo = false;
                context.mSurfaceView.setJoystickRect();
                doShowHideInfo(context, false);
                break;
            case MENU_BUTTON_OFF:
                context.mPrefOnscreenButtonOn = false;
                context.mSurfaceView.mShowInfo = false;
                context.mSurfaceView.setJoystickRect();
                doShowHideInfo(context, false);
                break;
            case MENU_BUTTON_ALWAYS_SHOW_JOYSTICK:
                context.mPrefAlwaysShowJoystick = !context.mPrefAlwaysShowJoystick;
                doShowHideInfo(context, false);
                savePreference(context, PREF_KEY_ALWAYS_SHOW_JOYSTICK);
                break;
            case MENU_BUTTON_LARGE_JOYSTICK:
                context.mPrefLargeJoystick = !context.mPrefLargeJoystick;
                context.mSurfaceView.setJoystickRect();
                doShowHideInfo(context, false);
                savePreference(context, PREF_KEY_LARGE_JOYSTICK);
                break;
            case MENU_KEYBOARD_SPECIAL:
                context.mSurfaceView.mContextMenu = CONTEXT_MENU_SPECIAL_KEYS;				
                context.openContextMenu(context.mSurfaceView);
                break;
            case MENU_KEYBOARD:
                doShowKeyboard(context);
                break;
            case MENU_SETTINGS_SCALE_FILTER:
                context.mPrefScaleFilterOn = !context.mPrefScaleFilterOn;
                context.mSurfaceView.forceRedraw();
                savePreference(context, PREF_KEY_SCALE_FILTER_ON);
                break;
            case MENU_SETTINGS_SCALE_FACTOR:
                doSetScaleFactor(context);
                break;
            case MENU_SETTINGS_MOUSE_SENSITIVITY:
                doSetMouseSensitivity(context);
                break;
            case MENU_SETTINGS_FAST_MOUSE:
                context.mPrefFastMouseOn = !context.mPrefFastMouseOn;
                savePreference(context, PREF_KEY_FAST_MOUSE_ON);
                break;
            case MENU_SETTINGS_SHOW_INFO:
                context.mPrefShowInfoOn = !context.mPrefShowInfoOn;
                DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_SHOW_INFO_ON, context.mPrefShowInfoOn ? 1 : 0, null);
                context.mSurfaceView.forceRedraw();
                savePreference(context, PREF_KEY_SHOW_INFO);
                break;
            case MENU_SETTINGS_TRUE_DRIVE:
                context.mPrefTrueDriveOn = !context.mPrefTrueDriveOn;
                DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_TRUE_DRIVE_ON, context.mPrefTrueDriveOn ? 1 : -1, null);
                break;
            case MENU_SETTINGS_FRAMESKIP:
                context.mSurfaceView.mContextMenu = CONTEXT_MENU_FRAMESKIP;
                context.openContextMenu(context.mSurfaceView);
                break;
            case MENU_SETTINGS_BETTER_VIDEO:
                context.mPrefBetterVideoOn = !context.mPrefBetterVideoOn;
                savePreference(context, PREF_KEY_BETTER_VIDEO_ON);
                break;
            case MENU_SETTINGS_HELP:
                doShowTextDialog(context, context.getString(R.string.help));
                break;
            case MENU_SETTINGS_LICENSE:
                copyLicenseFile(context);
                break;
            case MENU_SETTINGS_JOYKEY_SET:
                context.mSurfaceView.mContextMenu = CONTEXT_MENU_JOYKEY;
                context.openContextMenu(context.mSurfaceView);
                break;
            case MENU_SETTINGS_KEY_MAPPER:
                doKeyMapper(context);
                break;
            default:
                break;
        }
        return true;
    }

    static public void doCreateContextMenu(DosBoxLauncher context, ContextMenu menu, View v, ContextMenu.ContextMenuInfo menuInfo) {
        switch (context.mSurfaceView.mContextMenu) {
            case CONTEXT_MENU_SPECIAL_KEYS:
                {
                    MenuItem item;
                    item = menu.add(0, MENU_KEYBOARD_CTRL, 0, "Ctrl");
                    item.setCheckable(true);
                    item.setChecked(context.mSurfaceView.mModifierCtrl);

                    item = menu.add(0, MENU_KEYBOARD_ALT, 0, "Alt (CBM)");
                    item.setCheckable(true);
                    item.setChecked(context.mSurfaceView.mModifierAlt);

                    item = menu.add(0, MENU_KEYBOARD_SHIFT, 0, "Shift");
                    item.setCheckable(true);
                    item.setChecked(context.mSurfaceView.mModifierShift);

                    menu.add(0, MENU_KEYBOARD_ESC, 0, "Esc (Run/Stop)");
                    menu.add(0, MENU_KEYBOARD_PAGE_UP, 0, "Page Up (Restore)");
                    menu.add(0, MENU_KEYBOARD_TAB, 0, "Tab (CBM)");
                    menu.add(0, MENU_KEYBOARD_INSERT, 0, "Insert (Pound)");
                    menu.add(0, MENU_KEYBOARD_DELETE, 0, "Delete (Up Arrow)");
                    menu.add(0, MENU_KEYBOARD_GRAVE, 0, "` (Left Arrow)");

                    for (int i = MENU_KEYBOARD_F1; i <= MENU_KEYBOARD_F8; i++) {
                        menu.add(0, i, 0, "F" + (i - MENU_KEYBOARD_F1 + 1));
                    }

                    menu.add(0, MENU_KEYBOARD_F12, 0, "F12 (VICE Menu)");

                    item = menu.add(0, MENU_KEYBOARD_TURBO, 0, "Unlock Speed");
                    item.setCheckable(true);
                    item.setChecked(context.mTurboOn);
                }
                break;
            case CONTEXT_MENU_CYCLES:
                {
                    for (int i = MENU_CYCLE_1000; i <= MENU_CYCLE_1000 + 6; i++) {
                        int value = (i - MENU_CYCLE_1000 + 1);
                        MenuItem item = menu.add(1, i, 0, "Speed " + value);

                        if (value == context.mPrefCycles) {
                            item.setChecked(true);
                        }
                    }
                    menu.setGroupCheckable(1, true, true);
                }
                break;
            case CONTEXT_MENU_FRAMESKIP:
                {
                    for (int i = MENU_FRAMESKIP_0; i <= MENU_FRAMESKIP_10; i++) {
                        int value = (i - MENU_FRAMESKIP_0);
                        MenuItem item = menu.add(2, i, 0, "" + value);
                        if (value == context.mPrefFrameskip) {
                            item.setChecked(true);
                        }
                    }
                    menu.setGroupCheckable(2, true, true);
                }
                break;
            case CONTEXT_MENU_MEMORY_SIZE:
                {
                    menu.add(3, MENU_MEMORY_1, 0, "1 MB").setChecked((context.mPrefMemorySize == 1));
                    menu.add(3, MENU_MEMORY_2, 0, "2 MB").setChecked((context.mPrefMemorySize == 2));
                    menu.add(3, MENU_MEMORY_4, 0, "4 MB").setChecked((context.mPrefMemorySize == 4));
                    menu.add(3, MENU_MEMORY_8, 0, "8 MB").setChecked((context.mPrefMemorySize == 8));

                    menu.setGroupCheckable(3, true, true);
                }
                break;
            case CONTEXT_MENU_JOYKEY:
                {
                    menu.add(4, MENU_JOYKEY_NONE, 0, "None").setChecked((context.mPrefJoyKeySet == JOYKEY_SET_NONE));
                    menu.add(4, MENU_JOYKEY_DPAD, 0, "DPad").setChecked((context.mPrefJoyKeySet == JOYKEY_SET_DPAD));
                    menu.add(4, MENU_JOYKEY_WAXD, 0, "WASD+Space").setChecked((context.mPrefJoyKeySet == JOYKEY_SET_WAXD));
                    menu.add(4, MENU_JOYKEY_NUMPAD, 0, "Numpad").setChecked((context.mPrefJoyKeySet == JOYKEY_SET_NUMPAD));

                    menu.setGroupCheckable(4, true, true);
                }
                break;
        }
    }


    static public void doSendDownUpKey(DosBoxLauncher context, int keyCode) {
        DosBoxControl.sendNativeKey(keyCode , true, context.mSurfaceView.mModifierCtrl, context.mSurfaceView.mModifierAlt, context.mSurfaceView.mModifierShift);
        try {
            Thread.sleep(DosBoxSurfaceView.BUTTON_TAP_DELAY);
        }
        catch (InterruptedException e) {
        }
        DosBoxControl.sendNativeKey(keyCode , false, context.mSurfaceView.mModifierCtrl, context.mSurfaceView.mModifierAlt, context.mSurfaceView.mModifierShift);
        context.mSurfaceView.mModifierCtrl = false;
        context.mSurfaceView.mModifierAlt = false;
        context.mSurfaceView.mModifierShift = false;
    }

    static public boolean doContextItemSelected(DosBoxLauncher context, MenuItem item) {
        int itemID = item.getItemId();

        switch (itemID) {
            case MENU_KEYBOARD_CTRL:
                context.mSurfaceView.mModifierCtrl = !context.mSurfaceView.mModifierCtrl; 
                break;
            case MENU_KEYBOARD_ALT:
                context.mSurfaceView.mModifierAlt = !context.mSurfaceView.mModifierAlt; 
                break;		
            case MENU_KEYBOARD_SHIFT:
                context.mSurfaceView.mModifierShift = !context.mSurfaceView.mModifierShift;
                break;		
            case MENU_KEYBOARD_NUMLOCK:
                doSendDownUpKey(context, KEYCODE_NUM_LOCK);
                break;
            case MENU_KEYBOARD_TURBO:
                context.mTurboOn = !context.mTurboOn;
                DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_TURBO_ON, context.mTurboOn ? 1 : 0, null);
                break;
            case MENU_KEYBOARD_TAB:
                doSendDownUpKey(context, KeyEvent.KEYCODE_TAB);
                break;
            case MENU_KEYBOARD_ESC:
                doSendDownUpKey(context, KEYCODE_ESCAPE);
                break;
            case MENU_KEYBOARD_PAGE_UP:
                doSendDownUpKey(context, KEYCODE_PAGE_UP);
                break;
            case MENU_KEYBOARD_INSERT:
                doSendDownUpKey(context, KEYCODE_INSERT);
                break;
            case MENU_KEYBOARD_DELETE:
                doSendDownUpKey(context, KEYCODE_DELETE);
                break;
            case MENU_KEYBOARD_GRAVE:
                doSendDownUpKey(context, KeyEvent.KEYCODE_GRAVE);
                break;
            default:
                if ((itemID >= MENU_KEYBOARD_F1) && (itemID <= MENU_KEYBOARD_F12)) {
                    doSendDownUpKey(context, KEYCODE_F1 + (itemID - MENU_KEYBOARD_F1));
                } else if ((itemID >= MENU_CYCLE_1000) && (itemID <= MENU_CYCLE_5000)) {
                    context.mPrefCycles = (itemID - MENU_CYCLE_1000 + 1);
                    savePreference(context, PREF_KEY_CYCLES);
                    DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_CYCLES, context.mPrefCycles, null);
                } else if ((itemID >= MENU_FRAMESKIP_0) && (itemID <= MENU_FRAMESKIP_10)) {
                    context.mPrefFrameskip = (itemID - MENU_FRAMESKIP_0);
                    savePreference(context, PREF_KEY_FRAMESKIP);
                    DosBoxLauncher.nativeSetOption(DOSBOX_OPTION_ID_FRAMESKIP, context.mPrefFrameskip, null);
                } else if ((itemID >= MENU_MEMORY_1) && (itemID <= MENU_MEMORY_8)) {
                    switch (itemID) {
                        case MENU_MEMORY_1:
                            context.mPrefMemorySize = 1;
                            break;
                        case MENU_MEMORY_2:
                            context.mPrefMemorySize = 2;
                            break;
                        case MENU_MEMORY_4:
                            context.mPrefMemorySize = 4;
                            break;
                        case MENU_MEMORY_8:
                            context.mPrefMemorySize = 8;
                            break;
                    }
                    savePreference(context, PREF_KEY_MEMORY_SIZE);
                    showRestartMessage(context);
                } else if ((itemID >= MENU_JOYKEY_NONE) && (itemID <= MENU_JOYKEY_NUMPAD)) {
                    switch (itemID) {
                        case MENU_JOYKEY_NONE:
                            context.mPrefJoyKeySet = JOYKEY_SET_NONE;
                            break;
                        case MENU_JOYKEY_DPAD:
                            context.mPrefJoyKeySet = JOYKEY_SET_DPAD;
                            break;
                        case MENU_JOYKEY_WAXD:
                            context.mPrefJoyKeySet = JOYKEY_SET_WAXD;
                            break;
                        case MENU_JOYKEY_NUMPAD:
                            context.mPrefJoyKeySet = JOYKEY_SET_NUMPAD;
                            break;
                    }
                    savePreference(context, PREF_KEY_JOYKEY_SET);
                }
                break;
        }
        return true;
    }
}
