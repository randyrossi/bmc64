if DUMMY_COMPILE

SUBDIRS = android

endif

if NATIVE_GTK3_COMPILE

SUBDIRS = gtk3 shared

else

if SDL_COMPILE

SUBDIRS = sdl shared

else

if WIN32_COMPILE

SUBDIRS = mingw32-pcap shared

else

if RASPI_COMPILE

SUBDIRS = raspi shared

endif
endif
endif
endif
