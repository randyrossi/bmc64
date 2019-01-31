
# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := org_viceteam_sdl

# Path to shared libraries - Android 1.6 cannot load them properly, thus we have to specify absolute path here
# SDL_SHARED_LIBRARIES_PATH := /data/data/de.schwardtnet.alienblaster/lib

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := org.viceteam.sdl

# Android Dev Phone G1 has trackball instead of cursor keys, and 
# sends trackball movement events as rapid KeyDown/KeyUp events,
# this will make Up/Down/Left/Right key up events with X frames delay,
# so if application expects you to press and hold button it will process the event correctly.
# TODO: create a libsdl config file for that option and for key mapping/on-screen keyboard
SDL_TRACKBALL_KEYUP_DELAY := 4

# If the application designed for higher screen resolution enable this to get the screen
# resized in HW-accelerated way, however it eats a tiny bit of CPU
SDL_VIDEO_RENDER_RESIZE := 1

COMPILED_LIBRARIES := jpeg png

APPLICATION_ADDITIONAL_CFLAGS := 

APPLICATION_ADDITIONAL_LDFLAGS := 

APPLICATION_SUBDIRS_BUILD :=

APPLICATION_CUSTOM_BUILD_SCRIPT := y

SDL_ADDITIONAL_CFLAGS := -DSDL_ANDROID_KEYCODE_MOUSE=UNKNOWN -DSDL_ANDROID_KEYCODE_0=LCTRL -DSDL_ANDROID_KEYCODE_1=RETURN -DSDL_ANDROID_KEYCODE_2=KP_PLUS -DSDL_ANDROID_KEYCODE_3=KP_MINUS -DSDL_ANDROID_KEYCODE_4=LCTRL

SDL_VERSION := 1.2

# If SDL_Mixer should link to libMAD
SDL_MIXER_USE_LIBMAD :=
ifneq ($(strip $(filter mad, $(COMPILED_LIBRARIES))),)
SDL_MIXER_USE_LIBMAD := 1
endif

ifneq ($(findstring -crystax,$(TARGET_CC)),)
$(info Building with CrystaX toolchain - RTTI and exceptions enabled, STLPort disabled)
CRYSTAX_TOOLCHAIN=1
endif

include $(call all-subdir-makefiles)
