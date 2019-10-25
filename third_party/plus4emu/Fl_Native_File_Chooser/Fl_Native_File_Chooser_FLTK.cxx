//
// Fl_Native_File_Chooser_FLTK.cxx -- FLTK native OS file chooser widget
//
// Copyright 2004 by Greg Ercolano.
// API changes + filter improvements by Nathan Vander Wilt 2005
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please keep code 80 column compliant.
//
//      10        20        30        40        50        60        70
//       |         |         |         |         |         |         |
// 4567890123456789012345678901234567890123456789012345678901234567890123456789
//

#ifdef FLTK1
//
// FLTK1
//
#include <FL/Fl_Native_File_Chooser.H>
#define FNFC_CLASS Fl_Native_File_Chooser
#define FNFC_CTOR  Fl_Native_File_Chooser
#define FLTK_CHOOSER_SINGLE    Fl_File_Chooser::SINGLE
#define FLTK_CHOOSER_DIRECTORY Fl_File_Chooser::DIRECTORY
#define FLTK_CHOOSER_MULTI     Fl_File_Chooser::MULTI
#define FLTK_CHOOSER_CREATE    Fl_File_Chooser::CREATE
#else
//
// FLTK2
//
#include <fltk/NativeFileChooser.h>
#include <fltk/run.h>
#define FNFC_CTOR  NativeFileChooser
#define FNFC_CLASS fltk::FNFC_CTOR
#define FLTK_CHOOSER_SINGLE    fltk::FileChooser::SINGLE
#define FLTK_CHOOSER_DIRECTORY fltk::FileChooser::DIRECTORY
#define FLTK_CHOOSER_MULTI     fltk::FileChooser::MULTI
#define FLTK_CHOOSER_CREATE    fltk::FileChooser::CREATE
#endif

#include "common.cxx"
#include <sys/stat.h>

// CTOR
FNFC_CLASS::FNFC_CTOR(int val) {
    static int init = 0;		// 'first time' initialize flag
    if ( init == 0 ) {
        // Initialize when instanced for first time
	load_system_icons();
	init = 1;
    }
    _btype       = val;
    _options     = NO_OPTIONS;
    _filter      = NULL;
    _filtvalue   = 0;
    _parsedfilt  = NULL;
    _preset_file = NULL;
    _prevvalue   = NULL;
    _directory   = NULL;
    _errmsg      = NULL;
#ifdef FLTK1
    file_chooser = new Fl_File_Chooser(NULL, NULL, 0, NULL);
#else
    file_chooser = new fltk::FileChooser(NULL, NULL, 0, NULL);
#endif
    type(val);		// do this after file_chooser created
    _nfilters    = 0;
}

// DTOR
FNFC_CLASS::~FNFC_CTOR() {
    delete file_chooser;
    _filter      = strfree(_filter);
    _parsedfilt  = strfree(_parsedfilt);
    _preset_file = strfree(_preset_file);
    _prevvalue   = strfree(_prevvalue);
    _directory   = strfree(_directory);
    _errmsg      = strfree(_errmsg);
}

// PRIVATE: SET ERROR MESSAGE
void FNFC_CLASS::errmsg(const char *msg) {
    _errmsg = strfree(_errmsg);
    _errmsg = strnew(msg);
}

// PRIVATE: translate Native types to Fl_File_Chooser types
int FNFC_CLASS::type_fl_file(int val) {
    switch (val) {
        case BROWSE_FILE:
	    return(FLTK_CHOOSER_SINGLE);
        case BROWSE_DIRECTORY:
	    return(FLTK_CHOOSER_SINGLE | FLTK_CHOOSER_DIRECTORY);
        case BROWSE_MULTI_FILE:
	    return(FLTK_CHOOSER_MULTI);
        case BROWSE_MULTI_DIRECTORY:
	    return(FLTK_CHOOSER_DIRECTORY | FLTK_CHOOSER_MULTI);
        case BROWSE_SAVE_FILE:
	    return(FLTK_CHOOSER_SINGLE | FLTK_CHOOSER_CREATE);
        case BROWSE_SAVE_DIRECTORY:
	    return(FLTK_CHOOSER_DIRECTORY | FLTK_CHOOSER_MULTI | FLTK_CHOOSER_CREATE);
        default:
	    return(FLTK_CHOOSER_SINGLE);
    }
}

void FNFC_CLASS::type(int val) {
    _btype = val;
    file_chooser->type(type_fl_file(val));
}

int FNFC_CLASS::type() const {
    return(_btype);
}

// SET OPTIONS
void FNFC_CLASS::options(int val) {
    _options = val;
}

// GET OPTIONS
int FNFC_CLASS::options() const {
    return(_options);
}

// Show chooser, blocks until done.
// RETURNS:
//    0 - user picked a file
//    1 - user cancelled
//   -1 - failed; errmsg() has reason
//
int FNFC_CLASS::show() {
    // FILTER
    if ( _parsedfilt ) {
        file_chooser->filter(_parsedfilt);
    }

    // FILTER VALUE
    //     Set this /after/ setting the filter
    //
    file_chooser->filter_value(_filtvalue);

    // DIRECTORY
    if ( _directory && _directory[0] ) {
        file_chooser->directory(_directory);
    } else {
        file_chooser->directory(_prevvalue);
    }

    // PRESET FILE
    if ( _preset_file ) {
        file_chooser->value(_preset_file);
    }

    // OPTIONS: PREVIEW
    file_chooser->preview( (options() & PREVIEW) ? 1 : 0);

    // OPTIONS: NEW FOLDER
    if ( options() & NEW_FOLDER )
        file_chooser->type(file_chooser->type() |
	                   FLTK_CHOOSER_CREATE);		// on

    // SHOW
    file_chooser->show();

#ifdef FLTK1
    // FLTK1: BLOCK WHILE BROWSER SHOWN
    while ( file_chooser->shown() ) {
        Fl::wait();
    }
#else
    // FLTK2: BLOCK WHILE BROWSER SHOWN
    while ( file_chooser->visible() ) {
        fltk::wait();
    }
#endif

    if ( file_chooser->value() && file_chooser->value()[0] ) {
        _prevvalue = strfree(_prevvalue);
	_prevvalue = strnew(file_chooser->value());
	_filtvalue = file_chooser->filter_value();	// update filter value

	// HANDLE SHOWING 'SaveAs' CONFIRM
	if ( options() & SAVEAS_CONFIRM && type() == BROWSE_SAVE_FILE ) {
	    struct stat buf;
	    if ( stat(file_chooser->value(), &buf) != -1 ) {
		if ( buf.st_mode & S_IFREG ) {		// Regular file + exists?
		     if ( exist_dialog() == 0 ) {
		         return(1);
		     }
		}
	    }
	}
    }

    if ( file_chooser->count() ) return(0);
    else return(1);
}

// RETURN ERROR MESSAGE
const char *FNFC_CLASS::errmsg() const {
    return(_errmsg ? _errmsg : "No error");
}

// GET FILENAME
const char* FNFC_CLASS::filename() const {
    if ( file_chooser->count() > 0 ) return(file_chooser->value());
    return("");
}

// GET FILENAME FROM LIST OF FILENAMES
const char* FNFC_CLASS::filename(int i) const {
    if ( i < file_chooser->count() )
        return(file_chooser->value(i+1));	// convert fltk 1 based to our 0 based
    return("");
}

// SET TITLE
//     Can be NULL if no title desired.
//
void FNFC_CLASS::title(const char *val) {
    file_chooser->label(val);
}

// GET TITLE
//    Can return NULL if none set.
//
const char *FNFC_CLASS::title() const {
    return(file_chooser->label());
}

// SET FILTER
//     Can be NULL if no filter needed
//
void FNFC_CLASS::filter(const char *val) {
    _filter = strfree(_filter);
    _filter = strnew(val);
    parse_filter();
}

// GET FILTER
const char *FNFC_CLASS::filter() const {
    return(_filter);
}

// SET SELECTED FILTER
void FNFC_CLASS::filter_value(int val) {
    _filtvalue = val;
}

// RETURN SELECTED FILTER
int FNFC_CLASS::filter_value() const {
    return(_filtvalue);
}

// GET TOTAL FILENAMES CHOSEN
int FNFC_CLASS::count() const {
    return(file_chooser->count());
}

// PRESET PATHNAME
//     Can be NULL if no preset is desired.
//
void FNFC_CLASS::directory(const char *val) {
    _directory = strfree(_directory);
    _directory = strnew(val);
}

// GET PRESET PATHNAME
//    Can return NULL if none set.
//
const char *FNFC_CLASS::directory() const {
    return(_directory);
}

// Convert our filter format to fltk's chooser format
//     FROM                                     TO (FLTK)
//     -------------------------                --------------------------
//     "*.cxx"                                  "*.cxx Files(*.cxx)"
//     "C Files\t*.{cxx,h}"                     "C Files(*.{cxx,h})"
//     "C Files\t*.{cxx,h}\nText Files\t*.txt"  "C Files(*.{cxx,h})\tText Files(*.txt)"
//
//     Returns a modified version of the filter that the caller is responsible
//     for freeing with strfree().
//
void FNFC_CLASS::parse_filter() {
    _parsedfilt = strfree(_parsedfilt);	// clear previous parsed filter (if any)
    _nfilters = 0;
    char *in = _filter;
    if ( !in ) return;

    int has_name = strchr(in, '\t') ? 1 : 0;

    char mode = has_name ? 'n' : 'w';	// parse mode: n=title, w=wildcard
    char wildcard[1024] = "";		// parsed wildcard
    char name[1024] = "";

    // Parse filter user specified
    for ( ; 1; in++ ) {

        /*** DEBUG
        printf("WORKING ON '%c': mode=<%c> name=<%s> wildcard=<%s>\n",
	                    *in, mode,     name,     wildcard);
	***/

        switch (*in) {
	    // FINISHED PARSING NAME?
	    case '\t':
	        if ( mode != 'n' ) goto regchar;
		mode = 'w';
		break;

	    // ESCAPE NEXT CHAR
	    case '\\':
	        ++in;
		goto regchar;

	    // FINISHED PARSING ONE OF POSSIBLY SEVERAL FILTERS?
	    case '\r':
	    case '\n':
	    case '\0':
		// APPEND NEW FILTER TO LIST
		if ( wildcard[0] ) {
		    // OUT: "name(wild)\tname(wild)"
		    char comp[2048];
		    sprintf(comp, "%s%.511s(%.511s)", ((_parsedfilt)?"\t":""),
		    				      name, wildcard);
		    _parsedfilt = strapp(_parsedfilt, comp);
		    _nfilters++;
		    //DEBUG printf("DEBUG: PARSED FILT NOW <%s>\n", _parsedfilt);
		}
		// RESET
		wildcard[0] = name[0] = '\0';
		mode = strchr(in, '\t') ? 'n' : 'w';
		// DONE?
		if ( *in == '\0' ) return;	// done
		else continue;			// not done yet, more filters

	    // Parse all other chars
	    default:				// handle all non-special chars
	    regchar:				// handle regular char
                switch ( mode ) {
	            case 'n': chrcat(name, *in);     continue;
	            case 'w': chrcat(wildcard, *in); continue;
	        }
		break;
	}
    }
    //NOTREACHED
}

// SET PRESET FILENAME
void FNFC_CLASS::preset_file(const char* val) {
    _preset_file = strfree(_preset_file);
    _preset_file = strnew(val);
}

// GET PRESET FILENAME
const char* FNFC_CLASS::preset_file() const {
    return(_preset_file);
}
