#include "vice.h"

#ifdef USE_GNOMEUI
#include "x11/gnome/videoarch.h"
#else
#ifdef VMS
#include "x11xawvideoarch.h"
#else
#ifdef MACOSX_COCOA
#include "macosx/cocoa/videoarch.h"
#else
#include "x11/xaw/videoarch.h"
#endif
#endif
#endif
