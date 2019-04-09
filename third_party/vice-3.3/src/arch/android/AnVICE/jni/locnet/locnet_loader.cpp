/*
 * locnet_loader.cpp
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

#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

extern "C" {

#include "machine.h"
#include "ui.h"
#include "autostart.h"
#include "attach.h"
#include "mouse.h"
#include "resources.h"
#include "joy.h"
#include "datasette.h"

int loader_get_drive_true_emulation();

}

#ifdef __X64__
char execute_file[256] = "/sdcard/x64";
#endif

#ifdef __X64SC__
char execute_file[256] = "/sdcard/x64sc";
#endif

#ifdef __X64DTV__
char execute_file[256] = "/sdcard/x64dtv";
#endif

#ifdef __XSCPU64__
char execute_file[256] = "/sdcard/xscpu64";
#endif

#ifdef __X128__
char execute_file[256] = "/sdcard/x128";
#endif

#ifdef __XCBM2__
char execute_file[256] = "/sdcard/xcbm2";
#endif

#ifdef __XCBM5X0__
char execute_file[256] = "/sdcard/xcbm5x0";
#endif

#ifdef __XPET__
char execute_file[256] = "/sdcard/xpet";
#endif

#ifdef __XPLUS4__
char execute_file[256] = "/sdcard/xplus4";
#endif

#ifdef __XVIC__
char execute_file[256] = "/sdcard/xvic";
#endif

char autostart_filename[256] = "";
char execute_path[256] = "/sdcard/";
char savestate_filename[256];
int loader_loadstate = 0;
int loader_savestate = 0;
int loader_showinfo = 0;
int loader_turbo = 0;
int loader_true_drive = 0;
int loader_ntsc = 0;
int loader_border = 0;
int loader_rom_handling = 0;

#define main SDL_main

extern struct loader_config myLoader;

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeStart(JNIEnv *env, jobject obj, jobject bitmap, jint width, jint height)
{
    Android_Init(env, obj, bitmap, width, height);
    int i = 0;
    const char *argv[20];

    argv[i++] = execute_file;
    argv[i++] = "-chdir";
    argv[i++] = execute_path;
    argv[i++] = "-config";
    argv[i++] = "/sdcard/sdl-vicerc";
    if (loader_true_drive) {
        if (loader_true_drive == -1) {
            argv[i++] = "+truedrive";
        } else {
            argv[i++] = "-truedrive";
        }
    }
    if (loader_ntsc) {
        argv[i++] = "-ntsc";
    }
    if (loader_border == 1) {
        argv[i++] = "-sdllimitmode";
        argv[i++] = "0";
    } else {
        argv[i++] = "-sdllimitmode";
        argv[i++] = "1";
    }
    argv[i++] = "-autostart";
    argv[i++] = autostart_filename;

#if defined(__X64__) || defined(__X64SC__) || defined(__X64DTV__) || defined(__XSCPU64__) || defined(__XCBM5X0__)
    argv[i++] = "+VICIIdsize";
    argv[i++] = "+VICIIhwscale";
    argv[i++] = "-VICIIfilter";
    argv[i++] = "0";
#endif

#ifdef __XVIC__
    argv[i++] = "+VICdsize";
    argv[i++] = "+VIChwscale";
    argv[i++] = "-VICfilter";
    argv[i++] = "0";
#endif

#ifdef __XPLUS4__
    argv[i++] = "+TEDdsize";
    argv[i++] = "+TEDhwscale";
    argv[i++] = "-TEDfilter";
    argv[i++] = "0";
#endif

#ifdef __X128__
    argv[i++] = "+VICIIdsize";
    argv[i++] = "+VICIIhwscale";
    argv[i++] = "-VICIIfilter";
    argv[i++] = "0";
/*
    argv[i++] = "+VDCdsize";
    argv[i++] = "+VDChwscale";
    argv[i++] = "-VDCfilter";
    argv[i++] = "0";
*/
#endif

#if defined(__XCBM2__) || defined(__XPET__)
    argv[i++] = "+Crtcdsize";
    argv[i++] = "+Crtchwscale";
    argv[i++] = "-Crtcfilter";
    argv[i++] = "0";
#endif

    argv[i++] = "-directory";

    if (loader_rom_handling) {
#if defined(__X64__) || defined(__X64SC__)
        argv[i++] = "/vice/c64:/sdcard/vice/c64:/sd-ext/vice/c64:/emmc/vice/c64:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#ifdef __XSCPU64__
        argv[i++] = "/vice/scpu64:/sdcard/vice/scpu64:/sd-ext/vice/scpu64:/emmc/vice/scpu64:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#ifdef __X64DTV__
        argv[i++] = "/vice/c64dtv:/sdcard/vice/c64dtv:/sd-ext/vice/c64dtv:/emmc/vice/c64dtv:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#ifdef __X128__
        argv[i++] = "/vice/c128:/sdcard/vice/c128:/sd-ext/vice/c128:/emmc/vice/c128:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#if defined(__XCBM2__) || defined(__XCBM5X0__)
        argv[i++] = "/vice/cbm-ii:/sdcard/vice/cbm-ii:/sd-ext/vice/cbm-ii:/emmc/vice/cbm-ii:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#ifdef __XPET__
        argv[i++] = "/vice/pet:/sdcard/vice/pet:/sd-ext/vice/pet:/emmc/vice/pet:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#ifdef __XPLUS4__
        argv[i++] = "/vice/plus4:/sdcard/vice/plus4:/sd-ext/vice/plus4:/emmc/vice/plus4:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif

#ifdef __XVIC__
        argv[i++] = "/vice/vic20:/sdcard/vice/vic20:/sd-ext/vice/vic20:/emmc/vice/vic20:/vice/drives:/sdcard/vice/drives:/sd-ext/vice/drives:/emmc/vice/drives";
#endif
    }

    loader_true_drive = 0;

    main(i, (char **)argv);

    __android_log_print(ANDROID_LOG_INFO, LOCNET_LOG_TAG, "before Android_ShutDown()");
    Android_ShutDown();
}

void setDisk(JNIEnv *env, jint id, jstring filename, jint change_disk)
{
    if (filename && ((env)->GetStringLength(filename) > 0)) {
        const char *sfloppy = (env)->GetStringUTFChars(filename, 0);

        if (change_disk) {
            file_system_attach_disk(id + 8, sfloppy);
        } else {
            if (id == 0) {
                strcpy(autostart_filename, sfloppy);
            }
        }
        (env)->ReleaseStringUTFChars(filename, sfloppy);
    } else {
        if (change_disk) {
            file_system_detach_disk(id + 8);
        } else {
            file_system_detach_disk(id + 8);
        }
    }
}

extern "C" jint Java_com_locnet_vice_DosBoxLauncher_nativeSetOption(JNIEnv *env, jobject obj, jint option, jint value, jobject value2)
{
    jint result = 0;

    switch (option) {
        case 11:
            myLoader.frameskip = (value > 0) ? value : -1;
            break;
        case 14:
            loader_turbo = (value) ? 1 : -1;
            break;
        case 17:
            loader_showinfo = (value) ? 1 : -1;
            break;
        case 51:
            if (value2) {
                const char *srom = (env)->GetStringUTFChars((jstring)value2, 0);
                strcpy(execute_path, srom);
                strcpy(execute_file, srom);

#ifdef __X64__
                strcat(execute_file, "x64");
#endif

#ifdef __X64SC__
                strcat(execute_file, "x64sc");
#endif

#ifdef __X64DTV__
                strcat(execute_file, "x64dtv");
#endif

#ifdef __XSCPU64__
                strcat(execute_file, "xscpu64");
#endif

#ifdef __X128__
                strcat(execute_file, "x128");
#endif

#ifdef __XCBM2__
                strcat(execute_file, "xcbm2");
#endif

#ifdef __XCBM5X0__
                strcat(execute_file, "xcbm5x0");
#endif

#ifdef __XPET__
                strcat(execute_file, "xpet");
#endif

#ifdef __XPLUS4__
                strcat(execute_file, "xplus4");
#endif

#ifdef __XVIC__
                strcat(execute_file, "xvic");
#endif

                (env)->ReleaseStringUTFChars((jstring)value2, srom);
            }
            break;
        case 52:
            setDisk(env, abs(value) - 1, (jstring)value2, (value > 0) ? 1 : 0);
            break;
        case 53:
            loader_ntsc = value;
            break;
        case 56:
            if (value) {
                loader_true_drive = value;
            } else {
                result = loader_get_drive_true_emulation();
            }
            break;
        case 57:
            sdljoy_swap_ports();
            break;
        case 58:
            loader_border = value;
            break;
        case 59:
            datasette_control(value);
            break;
    }

    return result;
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeInit(JNIEnv *env, jobject obj)
{
    loadf = 0;
    myLoader.memsize = 2;
    myLoader.bmph = 0;
    myLoader.videoBuffer = 0;

    myLoader.abort = 0;
    myLoader.pause = 0;

    myLoader.frameskip = 0;
    myLoader.cycles = 1500;
    myLoader.soundEnable = 1;
    myLoader.cycleHack = 0;
    myLoader.refreshHack = 0;
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativePause(JNIEnv *env, jobject obj, jint state)
{
    if ((state == 0) || (state == 1)) {
        myLoader.pause = state;
    } else {
        myLoader.pause = (myLoader.pause) ? 0 : 1;
    }
    ui_pause_emulation(myLoader.pause);
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeSetRomHandling(jint state)
{
    loader_rom_handling = state;
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeStop(JNIEnv *env, jobject obj)
{
    myLoader.abort = 1;
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeReset(JNIEnv *env, jobject obj)
{
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeShutDown(JNIEnv *env, jobject obj)
{
    myLoader.bmph = 0;
    myLoader.videoBuffer = 0;
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeSaveState(JNIEnv *env, jobject obj, jstring filename, jint num)
{
    const char *srom = (env)->GetStringUTFChars(filename, 0);
    strcpy(savestate_filename, srom);

    switch (num) {
        case 1:
            strcat(savestate_filename, "-1.vsf");
            break;
        case 2:
            strcat(savestate_filename, "-2.vsf");
            break;
        case 3:
            strcat(savestate_filename, "-3.vsf");
            break;
        default:
            strcat(savestate_filename, ".vsf");
            break;
    }

    (env)->ReleaseStringUTFChars(filename, srom);

    ui_pause_emulation(1);
    loader_savestate = 1;
}

extern "C" void Java_com_locnet_vice_DosBoxLauncher_nativeLoadState(JNIEnv *env, jobject obj, jstring filename, jint num)
{
    const char *srom = (env)->GetStringUTFChars(filename, 0);
    strcpy(savestate_filename, srom);

    switch (num) {
        case 1:
            strcat(savestate_filename, "-1.vsf");
            break;
        case 2:
            strcat(savestate_filename, "-2.vsf");
            break;
        case 3:
            strcat(savestate_filename, "-3.vsf");
            break;
        default:
            strcat(savestate_filename, ".vsf");
            break;
    }

    (env)->ReleaseStringUTFChars(filename, srom);

    ui_pause_emulation(1);
    loader_loadstate = 1;
}
