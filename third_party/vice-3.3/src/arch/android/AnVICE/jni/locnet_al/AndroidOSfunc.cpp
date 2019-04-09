/*
 * AndroidOSfunc.cpp
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

#define USE_JNIGRAPHIC 0

#include <jni.h>

#if USE_JNIGRAPHIC
#include <android/bitmap.h>
#endif

#include "AndroidOSfunc.h"
#include "string.h"
#include "loader.h"
#include "SDL.h"
#include "locnet_vk.h"
#include "keycodes.h"
#include <deque>

std::deque<struct locnet_al_event> eventQueue;

struct loader_config myLoader;
struct loader_config *loadf;
bool enableSound = true;
bool enableCycleHack = true;
bool enableRefreshHack = true;

extern void (SDLCALL *mixerCallBack)(void *userdata, Uint8 *stream, int len);

void Android_Exit();
char *Android_VideoGetBuffer();

JNIEnv *gEnv = NULL;
jobject JavaCallbackThread = NULL;
jmethodID JavaVideoRedraw = NULL;
jmethodID JavaVideoSetMode = NULL;
jmethodID JavaAudioWriteBuffer = NULL;
jmethodID JavaAudioGetBuffer = NULL;
jmethodID JavaAudioInit = NULL;
jmethodID JavaExit = NULL;
jclass JavaCallbackThreadClass = NULL;

//locnet, 2011-04-28, support 2.1 or below
jmethodID JavaVideoGetBuffer = NULL;

void Android_Init(JNIEnv *env, jobject obj, jobject bitmap, jint width, jint height)
{
    gEnv = env;
    JavaCallbackThread = env->NewGlobalRef(obj);
    JavaCallbackThreadClass = env->GetObjectClass(JavaCallbackThread);
    JavaVideoRedraw = env->GetMethodID(JavaCallbackThreadClass, "callbackVideoRedraw", "(IIII)V");
    JavaVideoSetMode = env->GetMethodID(JavaCallbackThreadClass, "callbackVideoSetMode", "(II)Landroid/graphics/Bitmap;");
    JavaAudioInit = env->GetMethodID(JavaCallbackThreadClass, "callbackAudioInit", "(IIII)I");
    JavaAudioWriteBuffer = env->GetMethodID(JavaCallbackThreadClass, "callbackAudioWriteBuffer", "(I)V");
    JavaAudioGetBuffer = env->GetMethodID(JavaCallbackThreadClass, "callbackAudioGetBuffer", "()[S");
    JavaExit = env->GetMethodID(JavaCallbackThreadClass, "callbackExit", "()V");

    //locnet, 2011-04-28, support 2.1 or below
    JavaVideoGetBuffer = env->GetMethodID(JavaCallbackThreadClass, "callbackVideoGetBuffer", "()Ljava/nio/Buffer;");

    myLoader.bmph = bitmap;
    myLoader.width = width;
    myLoader.height = height;
    myLoader.rowbytes = myLoader.width * 2;

    loadf = &myLoader;
    enableSound = myLoader.soundEnable;
    enableCycleHack = myLoader.cycleHack;
    enableRefreshHack = myLoader.refreshHack;
}

void Android_ShutDown()
{
    //locnet, 2011-04-28, support 2.1 or below
    JavaVideoGetBuffer = NULL;

    JavaAudioGetBuffer = NULL;
    JavaAudioWriteBuffer = NULL;
    JavaAudioInit = NULL;
    JavaVideoRedraw = NULL;
    JavaCallbackThreadClass = NULL;

    while (eventQueue.size()) {
        eventQueue.pop_front();
    }

    atexit(Android_Exit);
    exit(0);
}

//addition key not defined in keycodes.h
#define AKEYCODE_ESCAPE            111
#define AKEYCODE_FORWARD_DEL       112
#define AKEYCODE_CTRL_LEFT         113
#define AKEYCODE_HOME              122
#define AKEYCODE_END               123
#define AKEYCODE_INSERT            124

#define AKEYCODE_F1                131
#define AKEYCODE_F2                132
#define AKEYCODE_F3                133
#define AKEYCODE_F4                134
#define AKEYCODE_F5                135
#define AKEYCODE_F6                136
#define AKEYCODE_F7                137
#define AKEYCODE_F8                138
#define AKEYCODE_F9                139
#define AKEYCODE_F10               140
#define AKEYCODE_F11               141
#define AKEYCODE_F12               142

//locnet, 2011-05-30, add more key support
#define AKEYCODE_NUM_LOCK          143
#define AKEYCODE_NUMPAD_0          144
#define AKEYCODE_NUMPAD_1          145
#define AKEYCODE_NUMPAD_2          146
#define AKEYCODE_NUMPAD_3          147
#define AKEYCODE_NUMPAD_4          148
#define AKEYCODE_NUMPAD_5          149
#define AKEYCODE_NUMPAD_6          150
#define AKEYCODE_NUMPAD_7          151
#define AKEYCODE_NUMPAD_8          152
#define AKEYCODE_NUMPAD_9          153
#define AKEYCODE_NUMPAD_DIVIDE     154
#define AKEYCODE_NUMPAD_MULTIPLY   155
#define AKEYCODE_NUMPAD_SUBTRACT   156
#define AKEYCODE_NUMPAD_ADD        157
#define AKEYCODE_NUMPAD_DOT        158
#define AKEYCODE_NUMPAD_ENTER      160

void getKeyFromUnicode(int unicode, struct locnet_al_event *event);
void getKeyFromKeyCode(int keyCode, struct locnet_al_event *event);

extern "C" jint Java_com_locnet_vice_DosBoxControl_nativeKey(JNIEnv *env, jobject obj, jint keyCode, jint down, jint ctrl, jint alt, jint shift)
{
    int unicode = (keyCode >> 8) & 0xFF;
    keyCode = keyCode & 0xFF;

    struct locnet_al_event event;
    event.keycode = KEYBOARD_NONE;
    event.modifier = 0;
    event.unicode = unicode;

    if (unicode != 0) {
        getKeyFromUnicode(unicode, &event);
        if ((event.keycode != KEYBOARD_NONE) && (event.modifier == KEYBOARD_SHIFT_FLAG)) {
            shift = 1;
        }
    }

    if (event.keycode == KEYBOARD_NONE) {
        int dosboxKeycode = KEYBOARD_NONE;

        switch (keyCode) {
            case AKEYCODE_CTRL_LEFT:
                dosboxKeycode = KBD_leftctrl;
                break;
            case AKEYCODE_ALT_LEFT:
                dosboxKeycode = KBD_cbm;
                break;
            case AKEYCODE_SHIFT_LEFT:
                dosboxKeycode = KBD_leftshift;
                break;
            case AKEYCODE_INSERT:
                dosboxKeycode = KBD_pound;
                break;
            case AKEYCODE_HOME:
                dosboxKeycode = KBD_home;
                break;
            case AKEYCODE_FORWARD_DEL:
                dosboxKeycode = KBD_up_arrow;
                break;
            case AKEYCODE_END:
                dosboxKeycode = KBD_at;
                break;
            case AKEYCODE_AT:
                dosboxKeycode = KBD_at;
                break;
            case AKEYCODE_POUND:
                dosboxKeycode = KBD_pound;
                break;
            case AKEYCODE_STAR:
                dosboxKeycode = KBD_star;
                break;
            case AKEYCODE_PLUS:
                dosboxKeycode = KBD_plus;
                break;
            case AKEYCODE_ESCAPE:
                dosboxKeycode = KBD_esc;
                break;
            case AKEYCODE_TAB:
                dosboxKeycode = KBD_cbm;
                break;
            case AKEYCODE_DEL:
                dosboxKeycode = KBD_backspace;
                break;
            case AKEYCODE_ENTER:
                dosboxKeycode = KBD_enter;
                break;
            case AKEYCODE_SPACE:
                dosboxKeycode = KBD_space;
                break;
            case AKEYCODE_DPAD_LEFT:
                dosboxKeycode = KBD_left;
                break;
            case AKEYCODE_DPAD_UP:
                dosboxKeycode = KBD_up;
                break;
            case AKEYCODE_DPAD_DOWN:
                dosboxKeycode = KBD_down;
                break;
            case AKEYCODE_DPAD_RIGHT:
                dosboxKeycode = KBD_right;
                break;
            case AKEYCODE_GRAVE:
                dosboxKeycode = KBD_left_arrow;
                break;
            case AKEYCODE_MINUS:
                dosboxKeycode = KBD_plus;
                break;
            case AKEYCODE_EQUALS:
                dosboxKeycode = KBD_minus;
                break;
            case AKEYCODE_BACKSLASH:
                dosboxKeycode = KBD_colon;
                break;
            case AKEYCODE_LEFT_BRACKET:
                dosboxKeycode = KBD_at;
                break;
            case AKEYCODE_RIGHT_BRACKET:
                dosboxKeycode = KBD_star;
                break;
            case AKEYCODE_SEMICOLON:
                dosboxKeycode = KBD_colon;
                break;
            case AKEYCODE_APOSTROPHE:
                dosboxKeycode = KBD_semicolon;
                break;
            case AKEYCODE_PERIOD:
                dosboxKeycode = KBD_period;
                break;
            case AKEYCODE_COMMA:
                dosboxKeycode = KBD_comma;
                break;
            case AKEYCODE_SLASH:
                dosboxKeycode = KBD_slash;
                break;
            case AKEYCODE_PAGE_UP:
                dosboxKeycode = KBD_restore;
                break;
            case AKEYCODE_PAGE_DOWN:
                dosboxKeycode = KBD_star;
                break;
            case AKEYCODE_A:
                dosboxKeycode = KBD_a;
                break;
            case AKEYCODE_B:
                dosboxKeycode = KBD_b;
                break;
            case AKEYCODE_C:
                dosboxKeycode = KBD_c;
                break;
            case AKEYCODE_D:
                dosboxKeycode = KBD_d;
                break;
            case AKEYCODE_E:
                dosboxKeycode = KBD_e;
                break;
            case AKEYCODE_F:
                dosboxKeycode = KBD_f;
                break;
            case AKEYCODE_G:
                dosboxKeycode = KBD_g;
                break;
            case AKEYCODE_H:
                dosboxKeycode = KBD_h;
                break;
            case AKEYCODE_I:
                dosboxKeycode = KBD_i;
                break;
            case AKEYCODE_J:
                dosboxKeycode = KBD_j;
                break;
            case AKEYCODE_K:
                dosboxKeycode = KBD_k;
                break;
            case AKEYCODE_L:
                dosboxKeycode = KBD_l;
                break;
            case AKEYCODE_M:
                dosboxKeycode = KBD_m;
                break;
            case AKEYCODE_N:
                dosboxKeycode = KBD_n;
                break;
            case AKEYCODE_O:
                dosboxKeycode = KBD_o;
                break;
            case AKEYCODE_P:
                dosboxKeycode = KBD_p;
                break;
            case AKEYCODE_Q:
                dosboxKeycode = KBD_q;
                break;
            case AKEYCODE_R:
                dosboxKeycode = KBD_r;
                break;
            case AKEYCODE_S:
                dosboxKeycode = KBD_s;
                break;
            case AKEYCODE_T:
                dosboxKeycode = KBD_t;
                break;
            case AKEYCODE_U:
                dosboxKeycode = KBD_u;
                break;
            case AKEYCODE_V:
                dosboxKeycode = KBD_v;
                break;
            case AKEYCODE_W:
                dosboxKeycode = KBD_w;
                break;
            case AKEYCODE_X:
                dosboxKeycode = KBD_x;
                break;
            case AKEYCODE_Y:
                dosboxKeycode = KBD_y;
                break;
            case AKEYCODE_Z:
                dosboxKeycode = KBD_z;
                break;
            case AKEYCODE_0:
                dosboxKeycode = KBD_0;
                break;
            case AKEYCODE_1:
                dosboxKeycode = KBD_1;
                break;
            case AKEYCODE_2:
                dosboxKeycode = KBD_2;
                break;
            case AKEYCODE_3:
                dosboxKeycode = KBD_3;
                break;
            case AKEYCODE_4:
                dosboxKeycode = KBD_4;
                break;
            case AKEYCODE_5:
                dosboxKeycode = KBD_5;
                break;
            case AKEYCODE_6:
                dosboxKeycode = KBD_6;
                break;
            case AKEYCODE_7:
                dosboxKeycode = KBD_7;
                break;
            case AKEYCODE_8:
                dosboxKeycode = KBD_8;
                break;
            case AKEYCODE_9:
                dosboxKeycode = KBD_9;
                break;
            case AKEYCODE_F1:
                dosboxKeycode = KBD_f1;
                break;
            case AKEYCODE_F2:
                dosboxKeycode = KBD_f2;
                break;
            case AKEYCODE_F3:
                dosboxKeycode = KBD_f3;
                break;
            case AKEYCODE_F4:
                dosboxKeycode = KBD_f4;
                break;
            case AKEYCODE_F5:
                dosboxKeycode = KBD_f5;
                break;
            case AKEYCODE_F6:
                dosboxKeycode = KBD_f6;
                break;
            case AKEYCODE_F7:
                dosboxKeycode = KBD_f7;
                break;
            case AKEYCODE_F8:
                dosboxKeycode = KBD_f8;
                break;
            case AKEYCODE_F12:
                dosboxKeycode = KBD_f12;
                break;
            default:
                break;
        }

        event.keycode = dosboxKeycode;
    }

    if (event.keycode != KEYBOARD_NONE) {
        int modifier = 0;

        if (ctrl) {
            modifier |= KEYBOARD_CTRL_FLAG;
        }
        if (alt) {
            modifier |= KEYBOARD_ALT_FLAG;
        }
        if (shift) {
            modifier |= KEYBOARD_SHIFT_FLAG;
        }

        event.eventType = (down) ? SDL_KEYDOWN : SDL_KEYUP;
        event.modifier = modifier;

        eventQueue.push_back(event);

        return 1;
    } else {
        return 0;
    }
}

extern "C" void Java_com_locnet_vice_DosBoxControl_nativeJoystick(JNIEnv *env, jobject obj, jint x, jint y, jint action, jint button)
{
    struct locnet_al_event event;

    event.eventType = SDL_NOEVENT;

    switch (action) {
        case 0:
            event.eventType = SDL_JOYBUTTONDOWN;
            event.keycode = button;
            break;
        case 1:
            event.eventType = SDL_JOYBUTTONUP;
            event.keycode = button;
            break;
        case 2:
            event.eventType = SDL_JOYAXISMOTION;
            event.x = x;
            event.y = y;
            break;
    }

    if (event.eventType != SDL_NOEVENT) {
        eventQueue.push_back(event);
    }
}

extern "C" void Java_com_locnet_vice_DosBoxControl_nativeMouse(JNIEnv *env, jobject obj, jint x, jint y, jint down_x, jint down_y, jint action, jint button)
{
    struct locnet_al_event event;

    event.eventType = SDL_NOEVENT;

    switch (action) {
        case 0:
            event.eventType = SDL_MOUSEBUTTONDOWN;
            event.down_x = down_x;
            event.down_y = down_y;
            event.keycode = button;
            break;
        case 1:
            event.eventType = SDL_MOUSEBUTTONUP;
            event.keycode = button;
            break;
        case 2:
            event.eventType = SDL_MOUSEMOTION;
            event.down_x = down_x;
            event.down_y = down_y;
            event.x = x;
            event.y = y;

            break;
    }

    if (event.eventType != SDL_NOEVENT) {
        eventQueue.push_back(event);
    }
}

void Android_AudioGetBuffer()
{
    if ((loadf != 0) && (loadf->abort == 0) && (gEnv != 0)) {
        loadf->audioBuffer = (jshortArray)gEnv->CallObjectMethod( JavaCallbackThread, JavaAudioGetBuffer );
    }
}

extern "C" void Android_AudioWriteBuffer()
{
    short size = 0;

    if ((loadf != 0) && (loadf->abort == 0) && (gEnv != 0)) {
        if ((mixerCallBack != 0) && (loadf->audioBuffer != 0)) {
            jboolean isCopy = JNI_TRUE;
            jsize len = gEnv->GetArrayLength(loadf->audioBuffer);
            jshort *audioBuffer = gEnv->GetShortArrayElements(loadf->audioBuffer, &isCopy);

            size = 0;

            (*mixerCallBack)(&size, (unsigned char *)audioBuffer, (len << 1));

            gEnv->ReleaseShortArrayElements(loadf->audioBuffer, audioBuffer, 0);

            if (size > 0) {
                gEnv->CallVoidMethod(JavaCallbackThread, JavaAudioWriteBuffer, (int)size);
            }
        }
    }
}

extern "C" int Java_com_locnet_vice_DosBoxLauncher_nativeAudioWriteBuffer(JNIEnv *env, jobject obj, jshortArray audioBufferArray)
{
    short size = 0;

    if ((loadf != 0) && (loadf->abort == 0) && (env != 0)) {
        if ((mixerCallBack != 0) && (audioBufferArray != 0)) {
            jboolean isCopy = JNI_TRUE;
            jsize len = env->GetArrayLength(audioBufferArray);
            jshort *audioBuffer = env->GetShortArrayElements(audioBufferArray, &isCopy);

            size = 0;

            (*mixerCallBack)(&size, (unsigned char *)audioBuffer, (len << 1));

            env->ReleaseShortArrayElements(audioBufferArray, audioBuffer, 0);
        }
    }

    return size;
}

int Android_OpenAudio(int rate, int channels, int encoding, int bufSize)
{
    if ((loadf != 0) && (loadf->abort == 0) && (gEnv != 0)) {
        return gEnv->CallIntMethod(JavaCallbackThread, JavaAudioInit, rate, channels, encoding, bufSize);
    } else {
        return 0;
    }
}

extern "C" int Android_PollEvent(locnet_al_event_struct *event)
{
    if (eventQueue.size() > 0) {
        if (event) {
            *event = eventQueue.front();
            eventQueue.pop_front();
        }
        return 1;
    }
    return 0;
}

extern "C" int Android_HasRepeatEvent(int eventType, int value)
{
    if (eventQueue.size() > 2) {
        locnet_al_event_struct event1 = eventQueue.front();
        eventQueue.pop_front();
        locnet_al_event_struct event2 = eventQueue.front();
        eventQueue.pop_front();
        switch (eventType) {
            case SDL_JOYBUTTONDOWN:
                if ((event1.eventType == SDL_JOYBUTTONUP) &&
                    (event1.keycode == value) &&
                    (event2.eventType == SDL_JOYBUTTONDOWN) &&
                    (event2.keycode == value)) {
                    return 1;
                }
                break;
            case SDL_JOYBUTTONUP:
                if ((event1.eventType == SDL_JOYBUTTONDOWN) &&
                    (event1.keycode == value) &&
                    (event2.eventType == SDL_JOYBUTTONUP) &&
                    (event2.keycode == value)) {
                    return 1;
                }
                break;
        }
        eventQueue.push_front(event2);
        eventQueue.push_front(event1);
    }
    return 0;
}

void Android_Exit()
{
    if (gEnv != 0) {
        JNIEnv *env = gEnv;

        gEnv = NULL;

        env->CallVoidMethod( JavaCallbackThread, JavaExit);

        env->DeleteGlobalRef(JavaCallbackThread);
        JavaCallbackThread = NULL;
    }
}

void Android_SetVideoMode(int width, int height, int depth)
{
    if ((loadf != 0) && (gEnv != 0)) {
        if ((width != loadf->width) || (height != loadf->height)) {
            jobject bmph = gEnv->CallObjectMethod( JavaCallbackThread, JavaVideoSetMode, width, height );

            if (bmph) {
                loadf->bmph = bmph;
                loadf->width = width;
                loadf->height = height;
                loadf->rowbytes = width * 2;

#if !USE_JNIGRAPHIC
                //locnet, 2011-04-28, support 2.1 or below
                loadf->videoBuffer = Android_VideoGetBuffer();
#endif
            }
        }
    }
}

char *Android_VideoGetBuffer()
{
    char * result = 0;

    jobject videoBuffer = gEnv->CallObjectMethod(JavaCallbackThread, JavaVideoGetBuffer);
    if (videoBuffer != 0) {
        result = (char *)gEnv->GetDirectBufferAddress(videoBuffer);
        gEnv->DeleteLocalRef(videoBuffer);
        videoBuffer = 0;
    }

    return result;
}

void    Android_LockSurface()
{
    if ((gEnv != 0) && (loadf != 0) && (loadf->bmph != 0)) {
#if USE_JNIGRAPHIC
        void* pixels = 0;

        AndroidBitmap_lockPixels(gEnv, loadf->bmph, &pixels);
        loadf->videoBuffer = (char *)pixels;
#else
        //locnet, 2011-04-28, support 2.1 or below
        if (loadf->videoBuffer == 0) {
            loadf->videoBuffer = Android_VideoGetBuffer();
        }
#endif
    }
}

void    Android_UnlockSurface(int startLine, int endLine)
{
    if ((gEnv != 0) && (loadf != 0) && (loadf->bmph != 0) && (loadf->videoBuffer != 0)) {
#if USE_JNIGRAPHIC
        AndroidBitmap_unlockPixels(gEnv, loadf->bmph);
        loadf->videoBuffer = 0;
#endif
    }

    if ((loadf != 0) && (loadf->abort == 0) && (gEnv != 0) && (endLine > startLine)) {
        gEnv->CallVoidMethod( JavaCallbackThread, JavaVideoRedraw, loadf->width, loadf->height, startLine, endLine );
    }
}

void    Android_ResetScreen()
{
    if ((gEnv != 0) && (loadf != 0) && (loadf->bmph != 0)) {
        void* pixels = 0;

#if USE_JNIGRAPHIC
        AndroidBitmap_lockPixels(gEnv, loadf->bmph, &pixels);
#else
        //locnet, 2011-04-28, support 2.1 or below
        pixels = Android_VideoGetBuffer();
#endif

        if (pixels != 0) {
            memset(pixels, 0, loadf->width * loadf->height * 2);
        }

#if USE_JNIGRAPHIC
        AndroidBitmap_unlockPixels(gEnv, loadf->bmph);
#endif
    }
}


void getKeyFromUnicode(int unicode, struct locnet_al_event *event)
{
    switch (unicode) {
        case '!':
        case '#':
        case '$':
        case '%':
        case '\'':
        case '&':
        case '(':
        case ')':
        case '?':
        case '"':
        case '<':
        case '>':
        case '[':
        case ']':
            event->modifier = KEYBOARD_SHIFT_FLAG;
            break;
        default:
            if ((unicode >= 'A') && (unicode <= 'Z')) {
                event->modifier = KEYBOARD_SHIFT_FLAG;
            }
            break;
    }
    int dosboxKeycode = KEYBOARD_NONE;
    switch (unicode) {
        case '!':
        case '1':
            dosboxKeycode = KBD_1;
            break;
        case '"':
        case '2':
            dosboxKeycode = KBD_2;
            break;
        case '#':
        case '3':
            dosboxKeycode = KBD_3;
            break;
        case '$':
        case '4':
            dosboxKeycode = KBD_4;
            break;
        case '%':
        case '5':
            dosboxKeycode = KBD_5;
            break;
        case '&':
        case '6':
            dosboxKeycode = KBD_6;
            break;
        case '\'':
        case '7':
            dosboxKeycode = KBD_7;
            break;
        case '(':
        case '8':
            dosboxKeycode = KBD_8;
            break;
        case ')':
        case '9':
            dosboxKeycode = KBD_9;
            break;
        case '0':
            dosboxKeycode = KBD_0;
            break;
        case 'a':
        case 'A':
            dosboxKeycode = KBD_a;
            break;
        case 'b':
        case 'B':
            dosboxKeycode = KBD_b;
            break;
        case 'c':
        case 'C':
            dosboxKeycode = KBD_c;
            break;
        case 'd':
        case 'D':
            dosboxKeycode = KBD_d;
            break;
        case 'e':
        case 'E':
            dosboxKeycode = KBD_e;
            break;
        case 'f':
        case 'F':
            dosboxKeycode = KBD_f;
            break;
        case 'g':
        case 'G':
            dosboxKeycode = KBD_g;
            break;
        case 'h':
        case 'H':
            dosboxKeycode = KBD_h;
            break;
        case 'i':
        case 'I':
            dosboxKeycode = KBD_i;
            break;
        case 'j':
        case 'J':
            dosboxKeycode = KBD_j;
            break;
        case 'k':
        case 'K':
            dosboxKeycode = KBD_k;
            break;
        case 'l':
        case 'L':
            dosboxKeycode = KBD_l;
            break;
        case 'm':
        case 'M':
            dosboxKeycode = KBD_m;
            break;
        case 'n':
        case 'N':
            dosboxKeycode = KBD_n;
            break;
        case 'o':
        case 'O':
            dosboxKeycode = KBD_o;
            break;
        case 'p':
        case 'P':
            dosboxKeycode = KBD_p;
            break;
        case 'q':
        case 'Q':
            dosboxKeycode = KBD_q;
            break;
        case 'r':
        case 'R':
            dosboxKeycode = KBD_r;
            break;
        case 's':
        case 'S':
            dosboxKeycode = KBD_s;
            break;
        case 't':
        case 'T':
            dosboxKeycode = KBD_t;
            break;
        case 'u':
        case 'U':
            dosboxKeycode = KBD_u;
            break;
        case 'v':
        case 'V':
            dosboxKeycode = KBD_v;
            break;
        case 'w':
        case 'W':
            dosboxKeycode = KBD_w;
            break;
        case 'x':
        case 'X':
            dosboxKeycode = KBD_x;
            break;
        case 'y':
        case 'Y':
            dosboxKeycode = KBD_y;
            break;
        case 'z':
        case 'Z':
            dosboxKeycode = KBD_z;
            break;
        case 0x08:
            dosboxKeycode = KBD_backspace;
            break;
        case 0x09:
            dosboxKeycode = KBD_cbm;
            break;
        case 0x20:
            dosboxKeycode = KBD_space;
            break;
        case 0x0A:
            dosboxKeycode = KBD_enter;
            break;
        case '+':
            dosboxKeycode = KBD_plus;
            break;
        case '-':
            dosboxKeycode = KBD_minus;
            break;
        case '@':
            dosboxKeycode = KBD_at;
            break;
        case '*':
            dosboxKeycode = KBD_star;
            break;
        case '[':
        case ':':
            dosboxKeycode = KBD_colon;
            break;
        case ']':
        case ';':
            dosboxKeycode = KBD_semicolon;
            break;
        case '=':
            dosboxKeycode = KBD_equals;
            break;
        case '?':
        case '/':
            dosboxKeycode = KBD_slash;
            break;
        case '<':
        case ',':
            dosboxKeycode = KBD_comma;
            break;
        case '>':
        case '.':
            dosboxKeycode = KBD_period;
            break;
        case '\\':
            dosboxKeycode = KBD_colon;
            break;
        case 0x1B:
            dosboxKeycode = KBD_esc;
            break;
        case 0x1C:
            dosboxKeycode = KBD_left;
            break;
        case 0x1D:
            dosboxKeycode = KBD_right;
            break;
        case 0x1E:
            dosboxKeycode = KBD_up;
            break;
        case 0x1F:
            dosboxKeycode = KBD_down;
            break;
        default:
            dosboxKeycode = KEYBOARD_NONE;
            break;
    }
    event->keycode = dosboxKeycode;
}
