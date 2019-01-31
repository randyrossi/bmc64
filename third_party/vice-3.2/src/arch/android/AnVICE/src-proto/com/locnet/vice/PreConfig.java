/* Be aware that the original version of this file is located in
 * src/arch/android/AnVICE/src-proto/com/locnet/vice/PreConfig.java
 *
 * when making changes only make them to the file in the proper location
 */

/*
 * PreConfig.java
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

import java.io.File;

import org.ab.nativelayer.ImportView;
import org.ab.uae.FloppyImportView;
import org.ab.uae.RomImportView;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

public class PreConfig extends PreferenceActivity implements OnSharedPreferenceChangeListener {

    /* the following variable will be used for the machine type currently being compiled.
       @VICE_MACHINE@ will be replaced by the number of the machine type.
       machine types:
       0 = x64
       1 = x64sc
       2 = xscpu64
       3 = x64dtv
       4 = x128
       5 = xcbm2
       6 = xcbm5x0
       7 = xpet
       8 = xplus4
       9 = xvic
     100 = all emulators
     */
    public static final int MACHINE_TYPE = @VICE_MACHINE@;

    public static final int MACHINE_X64 = 0;
    public static final int MACHINE_X64SC = 1;
    public static final int MACHINE_XSCPU64 = 2;
    public static final int MACHINE_X64DTV = 3;
    public static final int MACHINE_X128 = 4;
    public static final int MACHINE_XCBM2 = 5;
    public static final int MACHINE_XCBM5X0 = 6;
    public static final int MACHINE_XPET = 7;
    public static final int MACHINE_XPLUS4 = 8;
    public static final int MACHINE_XVIC = 9;
    public static final int MACHINE_ALL = 100;

    /* The following variable will be used to indicate the kind of rom checking needed:
       0 = external roms, old behavior, checks for roms and doesn't use hardcoded search path
       1 = pushed roms, checks for roms, and uses hardcoded search path
       2 = asset roms, no check needed, copied to right location before emulator start
       3 = embedded roms, no check needed, no copy needed
     */
    public static final int ROMS_TYPE = @VICE_ROMS@;

    public static final int ROMS_EXTERNAL = 0;
    public static final int ROMS_PUSHED = 1;
    public static final int ROMS_ASSET = 2;
    public static final int ROMS_EMBEDDED = 3;

    public static final String PREF_KEY_START = "pref_key_start";
    public static final String PREF_KEY_NTSC_ON = "pref_key_ntsc_on";
    public static final String PREF_KEY_ROM = "pref_key_rom";
    public static final String PREF_KEY_FLOPPY1 = "pref_key_floppy1";
    public static final String PREF_KEY_FLOPPY2 = "pref_key_floppy2";
    public static final String PREF_KEY_FLOPPY3 = "pref_key_floppy3";
    public static final String PREF_KEY_FLOPPY4 = "pref_key_floppy4";
    public static final String PREF_KEY_TRUE_DRIVE_ON = "pref_key_true_drive_on";
    public static final String PREF_KEY_SCREEN_BORDER_ON = "pref_key_screen_border_on";

    public static final String PREF_KEY_START_X64 = "pref_key_start_x64";
    public static final String PREF_KEY_START_X64SC = "pref_key_start_x64sc";
    public static final String PREF_KEY_START_XSCPU64 = "pref_key_start_xscpu64";
    public static final String PREF_KEY_START_X64DTV = "pref_key_start_x64dtv";
    public static final String PREF_KEY_START_X128 = "pref_key_start_x128";
    public static final String PREF_KEY_START_XCBM2 = "pref_key_start_xcbm2";
    public static final String PREF_KEY_START_XCBM5X0 = "pref_key_start_xcbm5x0";
    public static final String PREF_KEY_START_XPET = "pref_key_start_xpet";
    public static final String PREF_KEY_START_XPLUS4 = "pref_key_start_xplus4";
    public static final String PREF_KEY_START_XVIC = "pref_key_start_xvic";

    public static final String KERNAL_NAME = "KERNAL";
    public static final String BASIC_NAME = "BASIC";
    public static final String CHARGEN_NAME = "CHARGEN";

    public static final String C64_KERNAL_NAME = "kernal";
    public static final String C64_BASIC_NAME = "basic";
    public static final String C64_CHARGEN_NAME = "chargen";
    public static final String C64_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String C64DTV_KERNAL_NAME = "kernal";
    public static final String C64DTV_BASIC_NAME = "basic";
    public static final String C64DTV_CHARGEN_NAME = "chargen";
    public static final String C64DTV_ROM_NAME = "dtvrom.bin";
    public static final String C64DTV_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String SCPU64_CHARGEN_NAME = "chargen";
    public static final String SCPU64_ROM_NAME = "scpu64";
    public static final String SCPU64_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String CBM5X0_KERNAL_NAME = "kernal.500";
    public static final String CBM5X0_BASIC_NAME = "basic.500";
    public static final String CBM5X0_CHARGEN_NAME = "chargen.500";
    public static final String CBM5X0_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String CBM2_KERNAL_NAME = "kernal";
    public static final String CBM2_BASIC_NAME = "basic.128";
    public static final String CBM2_CHARGEN_NAME = "chargen.600";
    public static final String CBM2_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String C128_KERNAL_NAME = "kernal";
    public static final String C128_KERNALCH_NAME = "kernalch";
    public static final String C128_KERNALDE_NAME = "kernalde";
    public static final String C128_KERNALFI_NAME = "kernalfi";
    public static final String C128_KERNALFR_NAME = "kernalfr";
    public static final String C128_KERNALIT_NAME = "kernalit";
    public static final String C128_KERNALNO_NAME = "kernalno";
    public static final String C128_KERNALSE_NAME = "kernalse";
    public static final String C128_CHARGCH_NAME = "chargch";
    public static final String C128_CHARGDE_NAME = "chargde";
    public static final String C128_CHARGEN_NAME = "chargen";
    public static final String C128_CHARGFR_NAME = "chargfr";
    public static final String C128_CHARGSE_NAME = "chargse";
    public static final String C128_BASICLO_NAME = "basiclo";
    public static final String C128_BASICHI_NAME = "basichi";
    public static final String C128_BASIC64_NAME = "basic64";
    public static final String C128_KERNAL64_NAME = "kernal64";
    public static final String C128_SDL_SYM_NAME = "sdl_pos.vkm";

    public static final String PET_KERNAL_NAME = "kernal4";
    public static final String PET_BASIC_NAME = "basic4";
    public static final String PET_CHARGEN_NAME = "chargen";
    public static final String PET_EDITOR_NAME = "edit4b80";
    public static final String PET_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String PLUS4_KERNAL_NAME = "kernal";
    public static final String PLUS4_BASIC_NAME = "basic";
    public static final String PLUS4_3PLUS1LO_NAME = "3plus1lo";
    public static final String PLUS4_3PLUS1HI_NAME = "3plus1hi";
    public static final String PLUS4_SDL_SYM_NAME = "sdl_pos.vkm";

    public static final String VIC20_KERNAL_NAME = "kernal";
    public static final String VIC20_BASIC_NAME = "basic";
    public static final String VIC20_CHARGEN_NAME = "chargen";
    public static final String VIC20_SDL_SYM_NAME = "sdl_sym.vkm";

    public static final String DRIVE_NAME = "DRIVES";

    public static final String C64_PATH = "c64";
    public static final String SCPU64_PATH = "scpu64";
    public static final String C64DTV_PATH = "c64dtv";
    public static final String C128_PATH = "c128";
    public static final String CBM2_PATH = "cbm-ii";
    public static final String PET_PATH = "pet";
    public static final String PLUS4_PATH = "plus4";
    public static final String VIC20_PATH = "vic20";

    public static final String DRIVE_PATH = "drives";

    public final static String EXTRA_MESSAGE = "com.locnet.vice.MESSAGE";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.layout.prefs);

        final Context context = this;

        String key = PREF_KEY_START;
        Preference pref;

        if (MACHINE_TYPE != MACHINE_ALL) {
            key = PREF_KEY_START;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStart();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_ALL);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
        } else {
            key = PREF_KEY_START_X64;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartX64();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_X64);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_X64SC;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartX64SC();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_X64SC);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_XSCPU64;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartXSCPU64();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_XSCPU64);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_X64DTV;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartX64DTV();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_X64DTV);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_X128;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartX128();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_X128);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_XCBM2;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartXCBM2();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_XCBM2);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_XCBM5X0;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartXCBM5X0();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_XCBM5X0);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_XPET;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartXPET();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_XPET);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_XPLUS4;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartXPLUS4();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_XPLUS4);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
            key = PREF_KEY_START_XVIC;
            pref = (Preference)getPreferenceScreen().findPreference(key);
            if (pref != null) {
                setEnableStartXVIC();
                pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                    public boolean onPreferenceClick(Preference preference) {
                        Intent settingsIntent = new Intent();
                        settingsIntent.setClass(context, DosBoxLauncher.class);
                        settingsIntent.putExtra(EXTRA_MESSAGE, MACHINE_XVIC);
                        startActivityForResult(settingsIntent, 20);

                        return true;
                    }
                });
            }
        }


        /* only set rom handler if external roms are used */
        if (ROMS_TYPE == ROMS_EXTERNAL) {
            setRomHandler(PREF_KEY_ROM, Globals.PREFKEY_ROM_INT);
        }

        setFloppyHandler(PREF_KEY_FLOPPY1, Globals.PREFKEY_F1_INT);
        setFloppyHandler(PREF_KEY_FLOPPY2, Globals.PREFKEY_F2_INT);
        setFloppyHandler(PREF_KEY_FLOPPY3, Globals.PREFKEY_F3_INT);
        setFloppyHandler(PREF_KEY_FLOPPY4, Globals.PREFKEY_F4_INT);

        {
            SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);

            /* only set rom file summary if external roms are used */
            if (ROMS_TYPE == ROMS_EXTERNAL) {
                setFileSummary(PREF_KEY_ROM, sp.getString(Globals.PREFKEY_ROM, null));
            }

            setFileSummary(PREF_KEY_FLOPPY1, sp.getString(Globals.PREFKEY_F1, null));
            setFileSummary(PREF_KEY_FLOPPY2, sp.getString(Globals.PREFKEY_F2, null));
            setFileSummary(PREF_KEY_FLOPPY3, sp.getString(Globals.PREFKEY_F3, null));
            setFileSummary(PREF_KEY_FLOPPY4, sp.getString(Globals.PREFKEY_F4, null));
        }
        getPreferenceScreen().getSharedPreferences().registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    protected void onDestroy() {
        getPreferenceScreen().getSharedPreferences().unregisterOnSharedPreferenceChangeListener(this);
        super.onDestroy();
    }

    void setEnableStart() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START);
        if (pref != null) {

            /* external roms, check for rom location */
            if (ROMS_TYPE == ROMS_EXTERNAL) {
                String path = PreferenceManager.getDefaultSharedPreferences(this).getString(Globals.PREFKEY_ROM, null);
                if ((path != null) && (path.length() > 0)) {
                    pref.setEnabled(true);
                }
            }

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (MACHINE_TYPE == MACHINE_X64 || MACHINE_TYPE == MACHINE_X64SC) {
                    if (!checkC64ROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_XSCPU64) {
                    if (!checkSCPU64ROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_X64DTV) {
                    if (!checkC64DTVROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_X128) {
                    if (!checkC128ROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_XCBM2) {
                    if (!checkCBM2ROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_XCBM5X0) {
                    if (!checkCBM5X0ROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_XPET) {
                    if (!checkPETROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_XPLUS4) {
                    if (!checkPLUS4ROMS()) {
                        should_enable = false;
                    }
                }

                if (MACHINE_TYPE == MACHINE_XVIC) {
                    if (!checkVIC20ROMS()) {
                        should_enable = false;
                    }
                }

                if (should_enable) {
                    pref.setEnabled(true);
                    pref.setSummary("");
                } else {
                    pref.setSummary("Some needed files are missing.");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (MACHINE_TYPE == MACHINE_X64 || MACHINE_TYPE == MACHINE_X64SC || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkC64ROMS()) {
                        if (!copyC64ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_XSCPU64 || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkSCPU64ROMS()) {
                        if (!copySCPU64ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_X64DTV || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkC64DTVROMS()) {
                        if (!copyC64DTVROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_X128 || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkC128ROMS()) {
                        if (!copyC128ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_XCBM2 || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkCBM2ROMS()) {
                        if (!copyCBM2ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_XCBM5X0 || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkCBM5X0ROMS()) {
                        if (!copyCBM5X0ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_XPET || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkPETROMS()) {
                        if (!copyPETROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_XPLUS4 || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkPLUS4ROMS()) {
                        if (!copyPLUS4ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (MACHINE_TYPE == MACHINE_XVIC || MACHINE_TYPE == MACHINE_ALL) {
                    if (!checkVIC20ROMS()) {
                        if (!copyVIC20ROMS()) {
                            should_enable = false;
                        }
                    }
                }

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (should_enable) {
                    pref.setEnabled(true);
                    pref.setSummary("");
                } else {
                    pref.setSummary("Some needed files cannot be copied.");
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartX64() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_X64);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkC64ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkC64ROMS()) {
                    if (!copyC64ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartX64SC() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_X64SC);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkC64ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkC64ROMS()) {
                    if (!copyC64ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartXSCPU64() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_XSCPU64);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkSCPU64ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkSCPU64ROMS()) {
                    if (!copySCPU64ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartX64DTV() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_X64DTV);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkC64DTVROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkC64DTVROMS()) {
                    if (!copyC64DTVROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartX128() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_X128);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkC128ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkC128ROMS()) {
                    if (!copyC128ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartXCBM2() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_XCBM2);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkCBM2ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkCBM2ROMS()) {
                    if (!copyCBM2ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartXCBM5X0() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_XCBM5X0);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkCBM5X0ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkCBM5X0ROMS()) {
                    if (!copyCBM5X0ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartXPET() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_XPET);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkPETROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkPETROMS()) {
                    if (!copyPETROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartXPLUS4() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_XPLUS4);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkPLUS4ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkPLUS4ROMS()) {
                    if (!copyPLUS4ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setEnableStartXVIC() {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_START_XVIC);
        if (pref != null) {

            /* pushed roms, check for presence of roms in any of the hardcoded locations */
            if (ROMS_TYPE == ROMS_PUSHED) {
                boolean should_enable = true;

                if (!checkVIC20ROMS()) {
                    pref.setSummary("Some needed files are missing.");
                } else {
                    pref.setEnabled(true);
                    pref.setSummary("");
                }
            }


            /* asset roms, check for presence of roms and when missing copy roms to correct location */
            if (ROMS_TYPE == ROMS_ASSET) {
                boolean should_enable = true;

                if (!checkDRIVEROMS()) {
                    copyDRIVEROMS();
                }

                if (!checkVIC20ROMS()) {
                    if (!copyVIC20ROMS()) {
                        pref.setSummary("Some needed files cannot be copied.");
                    } else {
                        pref.setEnabled(true);
                        pref.setSummary("");
                    }
                }
            }

            /* embedded roms, no need to check, just enable */
            if (ROMS_TYPE == ROMS_EMBEDDED) {
                pref.setEnabled(true);
            }
        }
    }

    void setRomHandler(String key, final int requestCode) {
        setFileHandler(this, key, requestCode, true);
    }

    void setFloppyHandler(String key, final int requestCode) {
        setFileHandler(this, key, requestCode, false);
    }

    void setFileHandler(final Context context, String key, final int requestCode, final boolean rom) {
        Preference pref = (Preference)getPreferenceScreen().findPreference(key);
        if (pref != null) {
            pref.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                public boolean onPreferenceClick(Preference preference) {
                    Intent settingsIntent = new Intent();
                    settingsIntent.setClass(context, ImportView.class);
                    settingsIntent.putExtra("import", (rom) ? (new RomImportView()) : (new FloppyImportView()));
                    startActivityForResult(settingsIntent, requestCode);

                    return true;
                }
            });
        }
    }

    boolean checkC64ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(C64_PATH, C64_KERNAL_NAME);
        present += checkFileExistanceInPath(C64_PATH, C64_BASIC_NAME);
        present += checkFileExistanceInPath(C64_PATH, C64_CHARGEN_NAME);
        present += checkFileExistanceInPath(C64_PATH, C64_SDL_SYM_NAME);

        return (present == 4) ? true : false;
    }

    boolean checkC64DTVROMS() {
        int present = 0;

        present += checkFileExistanceInPath(C64DTV_PATH, C64DTV_KERNAL_NAME);
        present += checkFileExistanceInPath(C64DTV_PATH, C64DTV_BASIC_NAME);
        present += checkFileExistanceInPath(C64DTV_PATH, C64DTV_CHARGEN_NAME);
        present += checkFileExistanceInPath(C64DTV_PATH, C64DTV_ROM_NAME);
        present += checkFileExistanceInPath(C64DTV_PATH, C64_SDL_SYM_NAME);

        return (present == 5) ? true : false;
    }

    boolean checkSCPU64ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(SCPU64_PATH, SCPU64_CHARGEN_NAME);
        present += checkFileExistanceInPath(SCPU64_PATH, SCPU64_ROM_NAME);
        present += checkFileExistanceInPath(SCPU64_PATH, SCPU64_SDL_SYM_NAME);

        return (present == 3) ? true : false;
    }

    boolean checkCBM5X0ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(CBM2_PATH, CBM5X0_KERNAL_NAME);
        present += checkFileExistanceInPath(CBM2_PATH, CBM5X0_BASIC_NAME);
        present += checkFileExistanceInPath(CBM2_PATH, CBM5X0_CHARGEN_NAME);
        present += checkFileExistanceInPath(CBM2_PATH, CBM5X0_SDL_SYM_NAME);

        return (present == 4) ? true : false;
    }

    boolean checkCBM2ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(CBM2_PATH, CBM2_KERNAL_NAME);
        present += checkFileExistanceInPath(CBM2_PATH, CBM2_BASIC_NAME);
        present += checkFileExistanceInPath(CBM2_PATH, CBM2_CHARGEN_NAME);
        present += checkFileExistanceInPath(CBM2_PATH, CBM2_SDL_SYM_NAME);

        return (present == 4) ? true : false;
    }

    boolean checkC128ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(C128_PATH, C128_CHARGCH_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_CHARGDE_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_CHARGEN_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_CHARGFR_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_CHARGSE_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNAL_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALCH_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALDE_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALFI_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALFR_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALIT_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALNO_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNALSE_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_BASICLO_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_BASICHI_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_BASIC64_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_KERNAL64_NAME);
        present += checkFileExistanceInPath(C128_PATH, C128_SDL_SYM_NAME);

        return (present == 7) ? true : false;
    }

    boolean checkPETROMS() {
        int present = 0;

        present += checkFileExistanceInPath(PET_PATH, PET_KERNAL_NAME);
        present += checkFileExistanceInPath(PET_PATH, PET_BASIC_NAME);
        present += checkFileExistanceInPath(PET_PATH, PET_CHARGEN_NAME);
        present += checkFileExistanceInPath(PET_PATH, PET_EDITOR_NAME);
        present += checkFileExistanceInPath(PET_PATH, PET_SDL_SYM_NAME);

        return (present == 5) ? true : false;
    }

    boolean checkPLUS4ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(PLUS4_PATH, PLUS4_KERNAL_NAME);
        present += checkFileExistanceInPath(PLUS4_PATH, PLUS4_BASIC_NAME);
        present += checkFileExistanceInPath(PLUS4_PATH, PLUS4_3PLUS1LO_NAME);
        present += checkFileExistanceInPath(PLUS4_PATH, PLUS4_3PLUS1HI_NAME);
        present += checkFileExistanceInPath(PLUS4_PATH, PLUS4_SDL_SYM_NAME);

        return (present == 5) ? true : false;
    }

    boolean checkVIC20ROMS() {
        int present = 0;

        present += checkFileExistanceInPath(VIC20_PATH, VIC20_KERNAL_NAME);
        present += checkFileExistanceInPath(VIC20_PATH, VIC20_BASIC_NAME);
        present += checkFileExistanceInPath(VIC20_PATH, VIC20_CHARGEN_NAME);
        present += checkFileExistanceInPath(VIC20_PATH, VIC20_SDL_SYM_NAME);

        return (present == 4) ? true : false;
    }

    boolean checkDRIVEROMS() {
        int present = 0;

        present += checkFileExistanceInPath(DRIVE_PATH, "d1541II");
        present += checkFileExistanceInPath(DRIVE_PATH, "d1571cr");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos1001");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos1540");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos1541");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos1551");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos1570");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos1571");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos2000");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos2031");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos2040");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos3040");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos4000");
        present += checkFileExistanceInPath(DRIVE_PATH, "dos4040");

        return (present == 13) ? true : false;
    }

    boolean copyC64ROMS() {
        if (!copyAssetFileToPath(C64_PATH, C64_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64_PATH, C64_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64_PATH, C64_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64_PATH, C64_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyC64DTVROMS() {
        if (!copyAssetFileToPath(C64DTV_PATH, C64DTV_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64DTV_PATH, C64DTV_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64DTV_PATH, C64DTV_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64DTV_PATH, C64DTV_ROM_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C64DTV_PATH, C64DTV_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copySCPU64ROMS() {
        if (!copyAssetFileToPath(SCPU64_PATH, SCPU64_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(SCPU64_PATH, SCPU64_ROM_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(SCPU64_PATH, SCPU64_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyCBM5X0ROMS() {
        if (!copyAssetFileToPath(CBM2_PATH, CBM5X0_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(CBM2_PATH, CBM5X0_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(CBM2_PATH, CBM5X0_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(CBM2_PATH, CBM5X0_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyCBM2ROMS() {
        if (!copyAssetFileToPath(CBM2_PATH, CBM2_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(CBM2_PATH, CBM2_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(CBM2_PATH, CBM2_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(CBM2_PATH, CBM2_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyC128ROMS() {
        if (!copyAssetFileToPath(C128_PATH, C128_CHARGCH_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_CHARGDE_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_CHARGFR_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_CHARGSE_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALCH_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALDE_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALFI_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALFR_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALIT_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALNO_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNALSE_NAME)) {
            return false;
        }        
        if (!copyAssetFileToPath(C128_PATH, C128_BASICLO_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_BASICHI_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_BASIC64_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_KERNAL64_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(C128_PATH, C128_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyPETROMS() {
        if (!copyAssetFileToPath(PET_PATH, PET_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PET_PATH, PET_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PET_PATH, PET_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PET_PATH, PET_EDITOR_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PET_PATH, PET_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyPLUS4ROMS() {
        if (!copyAssetFileToPath(PLUS4_PATH, PLUS4_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PLUS4_PATH, PLUS4_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PLUS4_PATH, PLUS4_3PLUS1LO_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PLUS4_PATH, PLUS4_3PLUS1HI_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(PLUS4_PATH, PLUS4_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyVIC20ROMS() {
        if (!copyAssetFileToPath(VIC20_PATH, VIC20_KERNAL_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(VIC20_PATH, VIC20_BASIC_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(VIC20_PATH, VIC20_CHARGEN_NAME)) {
            return false;
        }
        if (!copyAssetFileToPath(VIC20_PATH, VIC20_SDL_SYM_NAME)) {
            return false;
        }
        return true;
    }

    boolean copyDRIVEROMS() {
        if (!copyAssetFileToPath(DRIVE_PATH, "d1541II")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "d1571cr")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos1001")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos1540")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos1541")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos1551")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos1570")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos1571")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos2000")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos2031")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos2040")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos3040")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos4000")) {
            return false;
        }
        if (!copyAssetFileToPath(DRIVE_PATH, "dos4040")) {
            return false;
        }
        return true;
    }

    boolean copyAssetFileToPath(String path, String file) {
        DosBoxMenuUtility.copyAssetFile(this, path + "_" + file, file, "/sdcard/vice/" + path);
        if (checkFileExistance("/sdcard/vice/" + path, file) == 0) {
            DosBoxMenuUtility.copyAssetFile(this, path + "_" + file, file, "/sd-ext/vice/" + path);
            if (checkFileExistance("/sd-ext/vice/" + path, file) == 0) {
                DosBoxMenuUtility.copyAssetFile(this, path + "_" + file, file, "/emmc/vice/" + path);
                if (checkFileExistance("/emmc/vice/" + path, file) == 0) {
                    DosBoxMenuUtility.copyAssetFile(this, path + "_" + file, file, "/vice/" + path);
                    if (checkFileExistance("/vice/" + path, file) == 0) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    int checkFileExistanceInPath(String path, String file) {
        int result = 0;

        result = checkFileExistance("/vice/" + path, file);
        result += checkFileExistance("/sdcard/vice/" + path, file);
        result += checkFileExistance("/sd-ext/vice/" + path, file);
        result += checkFileExistance("/emmc/vice/" + path, file);

        return (result != 0) ? 1 : 0;
    }

    int checkFileExistance(String path, String file) {
        int result = 1;

        try {
            File temp = new File(path, file);
            if (!temp.exists()) {
                result = 0;
            }
            temp = null;
        }
        catch (SecurityException e) {
        }

        return result;
    }

    String getWarning(String path, String file) {
        String result = "";

        try {
            File temp = new File(path, file);
            if (!temp.exists()) {
                result = " " + file;
            }
            temp = null;
        }
        catch (SecurityException e) {
        }

        return result;
    }

    void setTrueDriveSummary(String path) {
        Preference pref = (Preference)getPreferenceScreen().findPreference(PREF_KEY_TRUE_DRIVE_ON);
        if (pref != null) {
            String summary = getString(R.string.pref_true_drive_on_summary);
            boolean found = false;

            /* external roms, check if the drive rom exists */
            if (ROMS_TYPE == ROMS_EXTERNAL) {
                if ((path != null) && (path.length() > 0)) {
                    File temp = new File(path, DRIVE_NAME);
                    try {
                        if (temp.exists()) {
                            found = true;
                        }
                    }
                    catch (SecurityException e) {
                    }
                }
                if (!found) {
                    summary += "\nWarning: missing " + DRIVE_NAME + " ROMs";
                }
            }

            /* pushed roms, check for existance */
            if (ROMS_TYPE == ROMS_PUSHED) {
                if (!checkDRIVEROMS()) {
                    summary += "\nWarning: some drive roms are missing";
                }
            }

            /* asset roms, check for existance, if not present copy them */
            if (ROMS_TYPE == ROMS_ASSET) {
            }

            pref.setSummary(summary);
        }
    }

    void setFileSummary(String key, String file) {
        Preference pref = (Preference)getPreferenceScreen().findPreference(key);
        if (pref != null) {

            /* external roms, check for location and deal with the dialog item */
            if (ROMS_TYPE == ROMS_EXTERNAL) {
                if (PREF_KEY_ROM.equals(key) && (file != null)) {
                    String path = new File(file).getParent();
                    String warning = "";

                    if (MACHINE_TYPE == MACHINE_X64 || MACHINE_TYPE == MACHINE_X64SC) {
                        if (path.endsWith("c64") || (path.endsWith("C64"))) {
                            warning += getWarning(path, C64_KERNAL_NAME);
                            warning += getWarning(path, C64_BASIC_NAME);
                            warning += getWarning(path, C64_CHARGEN_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"c64\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_X64DTV) {
                        if (path.endsWith("c64dtv") || (path.endsWith("C64DTV"))) {
                            warning += getWarning(path, C64_KERNAL_NAME);
                            warning += getWarning(path, C64_BASIC_NAME);
                            warning += getWarning(path, C64_CHARGEN_NAME);
                            warning += getWarning(path, C64DTV_ROM_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"c64dtv\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_XSCPU64) {
                        if (path.endsWith("scpu64") || (path.endsWith("SCPU64"))) {
                            warning += getWarning(path, SCPU64_ROM_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"scpu64\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_XVIC) {
                        if (path.endsWith("vic20") || (path.endsWith("VIC20"))) {
                            warning += getWarning(path, C64_KERNAL_NAME);
                            warning += getWarning(path, C64_BASIC_NAME);
                            warning += getWarning(path, C64_CHARGEN_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"vic20\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_XCBM5X0) {
                        if (path.endsWith("cbm-ii") || (path.endsWith("CBM-II"))) {
                            warning += getWarning(path, CBM5X0_KERNAL_NAME);
                            warning += getWarning(path, CBM5X0_BASIC_NAME);
                            warning += getWarning(path, CBM5X0_CHARGEN_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"cbm-ii\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_X128) {
                        if (path.endsWith("c128") || (path.endsWith("C128"))) {
                            warning += getWarning(path, C64_CHARGEN_NAME);
                            warning += getWarning(path, C64_KERNAL_NAME);
                            warning += getWarning(path, C128_BASICLO_NAME);
                            warning += getWarning(path, C128_BASICHI_NAME);
                            warning += getWarning(path, C128_BASIC64_NAME);
                            warning += getWarning(path, C128_KERNAL64_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"c128\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_XCBM2) {
                        if (path.endsWith("cbm-ii") || (path.endsWith("CBM-II"))) {
                            warning += getWarning(path, CBM2_KERNAL_NAME);
                            warning += getWarning(path, CBM2_BASIC_NAME);
                            warning += getWarning(path, CBM2_CHARGEN_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"cbm-ii\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_XPET) {
                        if (path.endsWith("pet") || (path.endsWith("PET"))) {
                            warning += getWarning(path, PET_KERNAL_NAME);
                            warning += getWarning(path, PET_BASIC_NAME);
                            warning += getWarning(path, PET_CHARGEN_NAME);
                            warning += getWarning(path, PET_EDITOR_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"pet\"";
                        }
                        pref.setSummary(file + warning);
                    }

                    if (MACHINE_TYPE == MACHINE_XPLUS4) {
                        if (path.endsWith("plus4") || (path.endsWith("PLUS4"))) {
                            warning += getWarning(path, C64_KERNAL_NAME);
                            warning += getWarning(path, C64_BASIC_NAME);
                            warning += getWarning(path, PLUS4_3PLUS1LO_NAME);
                            warning += getWarning(path, PLUS4_3PLUS1HI_NAME);

                            if (warning.length() > 0) {
                                warning = "\nWarning: missing" + warning;
                            }
                            setTrueDriveSummary(new File(path).getParent());
                        } else {
                            warning = "\nWarning: ROM must be in a folder named \"plus4\"";
                        }
                        pref.setSummary(file + warning);
                    }
                } else {
                    pref.setSummary((file != null) ? file : "(empty)");
                }
            } else {
                pref.setSummary((file != null) ? file : "(empty)");
            }
        }
    }

    void enableFloppy(String key, boolean enabled) {
        Preference pref = (Preference)getPreferenceScreen().findPreference(key);
        if (pref != null) {
            pref.setEnabled(enabled);
        }
    }

    @Override
    protected void onActivityResult(final int requestCode, final int resultCode, final Intent extras) {
        if (resultCode == RESULT_OK) {
            String prefKey = null;
            String path = null;
            if (requestCode == Globals.PREFKEY_ROM_INT) {
                prefKey = Globals.PREFKEY_ROM;
                path = extras.getStringExtra("currentFile");
                setFileSummary(PREF_KEY_ROM, path);
            } else if (requestCode == Globals.PREFKEY_F1_INT) {
                prefKey = Globals.PREFKEY_F1;
                path = extras.getStringExtra("currentFile");
                setFileSummary(PREF_KEY_FLOPPY1, path);
            } else if (requestCode == Globals.PREFKEY_F2_INT) {
                prefKey = Globals.PREFKEY_F2;
                path = extras.getStringExtra("currentFile");
                setFileSummary(PREF_KEY_FLOPPY2, path);
            } else if (requestCode == Globals.PREFKEY_F3_INT) {
                prefKey = Globals.PREFKEY_F3;
                path = extras.getStringExtra("currentFile");
                setFileSummary(PREF_KEY_FLOPPY3, path);
            } else if (requestCode == Globals.PREFKEY_F4_INT) {
                prefKey = Globals.PREFKEY_F4;
                path = extras.getStringExtra("currentFile");
                setFileSummary(PREF_KEY_FLOPPY4, path);
            }
            if (prefKey != null) {
                SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
                Editor e = sp.edit();
                e.putString(prefKey, path);
                e.commit();
            }
        } else {
            String prefKey = null;
            if (requestCode == Globals.PREFKEY_F1_INT) {
                prefKey = Globals.PREFKEY_F1;
                setFileSummary(PREF_KEY_FLOPPY1, null);
            } else if (requestCode == Globals.PREFKEY_F2_INT) {
                prefKey = Globals.PREFKEY_F2;
                setFileSummary(PREF_KEY_FLOPPY2, null);
            } else if (requestCode == Globals.PREFKEY_F3_INT) {
                prefKey = Globals.PREFKEY_F3;
                setFileSummary(PREF_KEY_FLOPPY3, null);
            } else if (requestCode == Globals.PREFKEY_F4_INT) {
                prefKey = Globals.PREFKEY_F4;
                setFileSummary(PREF_KEY_FLOPPY4, null);
            }
            if (prefKey != null) {
                SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(this);
                Editor e = sp.edit();
                e.remove(prefKey);
                e.commit();
            }
        }
        setEnableStart();
    }

    public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
        if ((sharedPreferences != null) && (key != null)) {
        }
    }
}
