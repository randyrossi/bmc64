/*
 * AndroidOSfunc.h
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

#ifndef ANDROID_OS_FUNC
#define ANDROID_OS_FUNC

#include <jni.h>
#include <android/log.h>

#ifdef __X64__
#define LOCNET_LOG_TAG "c64"
#endif

#ifdef __X64SC__
#define LOCNET_LOG_TAG "c64sc"
#endif

#ifdef __X64DTV__
#define LOCNET_LOG_TAG "c64dtv"
#endif

#ifdef __XSCPU64__
#define LOCNET_LOG_TAG "scpu64"
#endif

#ifdef __X128__
#define LOCNET_LOG_TAG "c128"
#endif

#ifdef __XCBM2__
#define LOCNET_LOG_TAG "cbm2"
#endif

#ifdef __XCBM5X0__
#define LOCNET_LOG_TAG "cbm5x0"
#endif

#ifdef __XPET__
#define LOCNET_LOG_TAG "pet"
#endif

#ifdef __XPLUS4__
#define LOCNET_LOG_TAG "plus4"
#endif

#ifdef __XVIC__
#define LOCNET_LOG_TAG "vic20"
#endif

#define DEAD_ZONE 0.3

typedef struct locnet_al_event {
    int eventType;
    int keycode;
    int unicode;
    int modifier;
    float x;
    float y;
    float down_x;
    float down_y;
} locnet_al_event_struct;

void Android_Init(JNIEnv *env, jobject obj, jobject bitmap, jint width, jint height);
void Android_ShutDown();
void Android_SetVideoMode(int width, int height, int depth);
void Android_LockSurface();
void Android_UnlockSurface(int startLine, int endLine);
void Android_ResetScreen();
void Android_AudioGetBuffer();
int Android_OpenAudio(int rate, int channels, int encoding, int bufSize);

#ifdef __cplusplus
extern "C" {
#endif

void Android_AudioWriteBuffer();
int Android_PollEvent(locnet_al_event_struct *event);
int Android_HasRepeatEvent(int eventType, int value);

#ifdef __cplusplus
}
#endif


#endif
