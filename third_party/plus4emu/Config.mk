# Compile with HOST_BUILD=Y make plus4emu for a host executable

ifeq ($(HOST_BUILD),Y)
	CC_FOR_TARGET = gcc
	CXX_FOR_TARGET = g++
	CFLAGS_FOR_TARGET =
	CPPFLAGS_FOR_TARGET =
	CIRCLE_STDLIB_HOME =
	AR_FOR_TARGET = ar
	RANLIB_FOR_TARGET = ranlib
endif
