Fl_Native_File_Chooser -- Access platform's native file choosers in FLTK
------------------------------------------------------------------------


WHAT IS "Fl_Native_File_Chooser"?
=================================

    Fl_Native_File_Chooser is a 'widget' wrapper to access the
    different platform's native file choosers. On platforms that
    don't have a 'native' file chooser (linux), we use FLTK's own.

    The purpose of this library is to make a consistent interface
    for accessing the different native file choosers.

    Tested under Linux, Mac OSX and Windows with fltk 1.1.6.
    Tested under Linux with fltk 2.x.


LICENSING
=========

    Fl_Native_File_Chooser comes with complete free source code. 
    Fl_Native_File_Chooser is available under the terms of the 
    GNU Library General Public License. See COPYING for more info.

    Yes, it can be used in commercial software! Free! Imagine that.


BUILD INSTRUCTIONS
==================

    1. Which version of FLTK?

    Fl_Native_File_Chooser now supports fltk1 and fltk2 as of 0.83e.

    Edit the Makefile and uncomment/modify the FLTKCONFIG and FLTK2CONFIG
    variables as needed.
    
    If both variables are uncommented, both versions of
    Fl_Native_File_Chooser will be built.

    For instance, if you have both fltk1 and fltk2 installed on your
    system, the top of your Makefile settings might look like:

FLTKCONFIG=/usr/local/src/fltk-1.1.x-svn/fltk-config
FLTK2CONFIG=/usr/local/src/fltk-2.0-svn/fltk2-config
    
    Then you can build both with just:

	make

   These test programs are created, depending on if you have
   configured the above for fltk1, fltk2, or both:

       	./test-browser            -- fltk1 exerciser demo
	./simple-app              -- fltk1 simple app

       	./test-browser-fltk2      -- fltk2 exerciser demo
	./simple-app-fltk2        -- fltk2 simple app

   Originally Fl_Native_File_Chooser was designed for FLTK1, 
   so there may be some left over FLTK1 specific references
   in the docs. Please report these as bugs (see below)


PLATFORM SPECIFIC NOTES
=======================

   For linux and osx the default compilers are used.

   For Windows, tested with VS Express 8 + make.bat / Makefile.MICROSOFT.
   Ian confirmed it compiled OK under Windows with mingw using the default
   unix Makefile.


WHERE'S THE DOCUMENTATION?
==========================

    ./documentation/index.html


HOW DO I LINK Fl_Native_File_Chooser INTO MY OWN APPLICATION?
=============================================================

    ./documentation/how-to-use.html


FILE LAYOUT
===========

    ./Makefile                 -- main Makefile for unix builds
    ./Makefile.MICROSOFT       -- main Makefile for native Microsoft builds

    ./documentation/index.html -- public documentation for fltk1 + fltk2

    ./FL                       \__ fltk1 include files and lib
    ./Fl_Native_File_Chooser.o /   (for your app to include and link)

    ./fltk                     \__ fltk2 include files and lib
    ./NativeFileChooser.o      /   (for your app to include and link)

    ./reference                 -- project's reference docs (internal use)

    *_FLTK.{cxx,H,h}            -- Platforms that don't have native choosers
    *_MAC.{cxx,H,h}             -- Mac platform specific source code
    *_WIN32.{cxx,H,h}           -- Windows platform specific source code


RELEASE NOTES/VERSION INFORMATION
=================================
    
    See ./CHANGES.


BUGS? FEATURE REQUESTS?
=======================
    
    Send bugs and RFE's to erco at seriss dot com

