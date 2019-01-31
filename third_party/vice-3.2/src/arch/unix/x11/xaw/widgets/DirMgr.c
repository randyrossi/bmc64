/****************************************************************************

	DirMgr.c

	This file contains the C code to implement the DirectoryMgr system.

	This system is intended to manage filtered and sorted directory
	lists.

 ****************************************************************************/

/*
 * Author:
 * 	Brian Totty
 * 	Department of Computer Science
 * 	University Of Illinois at Urbana-Champaign
 *	1304 West Springfield Avenue
 * 	Urbana, IL 61801
 *
 * 	totty@cs.uiuc.edu
 *
 * (Sep 5, 1995: bugfix by Bert Bos <bert@let.rug.nl>, search for [BB])
 * (Apr 7, 1997: changed NO_REGEXP into !HAVE_REGEXP_H and autoconf.h added
 *               by Ettore Perazzoli <ettore@comm2000.it>)
 * (Apr 7, 1997: fixed behavior when regexp.h is not available by Ettore
 *		 Perazzoli <ettore@comm2000.it>, search for [EP])
 * (May 4, 1997: miscellaneous fixes by Ettore Perazzoli <ettore@comm2000.it>,
 *               search for [EP])
 * (Mar 19, 1998: disabled usage of regexp.h on {Free,Net}BSD.  Ettore
 *               Perazzoli <ettore@comm2000.it>, search for [EP])
 */

#include "vice.h"

#include <stdlib.h>		/* [EP] 05/04/97 */

#include "lib.h"

#include "DirMgr.h"

#include "RegExp.h"

/*---------------------------------------------------------------------------*

                   S I M P L E    I N T E R F A C E

 *---------------------------------------------------------------------------*/

DirectoryMgr *DirectoryMgrSimpleOpen(char *path, int sort_type, char *pattern)
{
    DirectoryMgr *dm;
    PFI f_func;
    PFIcomp s_func;
    fwf_regex_t f_data;

    if (pattern == NULL) {
        pattern = "*";
    }
    if (!DirectoryMgrSimpleFilterFunc(pattern, &f_func, &f_data)) {
        return NULL;
    }
    if (!DirectoryMgrSimpleSortingFunc(sort_type, &s_func)) {
        RegExpFree(&f_data);
        return NULL;
    }
    dm = DirectoryMgrOpen(path, s_func, f_func, &f_data, TRUE);
    return dm;
} /* End DirectoryMgrSimpleOpen */

int DirectoryMgrSimpleRefilter(DirectoryMgr *dm, char *pattern)
{
    PFI f_func;
    fwf_regex_t f_data;

    if (!DirectoryMgrSimpleFilterFunc(pattern, &f_func, &f_data)) {
        return FALSE;
    }
    DirectoryMgrRefilter(dm, f_func, &f_data, TRUE);
    return TRUE;
} /* End DirectoryMgrSimpleRefilter */

int DirectoryMgrSimpleResort(DirectoryMgr *dm, int sort_type)
{
    PFIcomp c_func;

    if (!DirectoryMgrSimpleSortingFunc(sort_type, &c_func)) {
        return FALSE;
    }
    DirectoryMgrResort(dm, c_func);
    return TRUE;
} /* End DirectoryMgrSimpleResort */

/*---------------------------------------------------------------------------*

                    N O R M A L    I N T E R F A C E

 *---------------------------------------------------------------------------*/

int DirectoryMgrCanOpen(char *path)
{
    int status;
    Directory dir;

    status = DirectoryOpen(path, &dir);
    if (status == TRUE) {
        DirectoryClose(&dir);
    }
    return(status);
} /* End DirectoryMgrCanOpen */

DirectoryMgr *DirectoryMgrOpen(char *path, PFIcomp c_func, PFI f_func, fwf_regex_t *f_data, int free_data)
{
    DirectoryMgr *dm;

    dm = lib_malloc(sizeof(DirectoryMgr));
    memset(dm, 0, sizeof(DirectoryMgr));

    if (DirectoryOpen(path, DirectoryMgrDir(dm)) == FALSE) {
        fprintf(stderr, "DirectoryMgrOpen: can't open dir '%s'\n", DirectoryMgrDir(dm)->path); /* [EP] 05/04/97 */
        lib_free(dm);
        RegExpFree(f_data);
        return NULL;
    }
    DirectoryMgrCompFunc(dm) = c_func;
    DirectoryMgrRefilter(dm, f_func, f_data, free_data);
    return dm;
} /* End DirectoryMgrOpen */

void DirectoryMgrClose(DirectoryMgr *dm)
{
    lib_free(DirectoryMgrData(dm));
    lib_free(DirectoryMgrSortedPtrs(dm));
    if (DirectoryMgrFreeFilterData(dm)) {
        RegExpFree(&DirectoryMgrFilterData(dm));
    }
    DirectoryClose(DirectoryMgrDir(dm));
    lib_free(dm);
} /* End DirectoryMgrClose */

int DirectoryMgrRefilter(DirectoryMgr *dm, PFI f_func, fwf_regex_t *f_data, int f_free)
{
    if (DirectoryMgrFreeFilterData(dm)) {
        RegExpFree(&DirectoryMgrFilterData(dm));
    }
    DirectoryMgrFilterFunc(dm) = f_func;
    DirectoryMgrFilterData(dm) = *f_data;
    DirectoryMgrFreeFilterData(dm) = f_free;
    DirectoryMgrRefresh(dm);
    return 0;		/* [EP] 05/04/97 */
} /* End DirectoryMgrRefilter */

int DirectoryMgrRefresh(DirectoryMgr *dm)
{
    int err, data_size, ptrs_size, i;
    DirEntryCons *head, *tail, *cons;
    DirEntry *dm_data, **dm_ptrs;
    PFI f_func;
    fwf_regex_t *f_data;

    tail = NULL;		/* make compiler happy [EP] 05/04/97 */
    DirectoryMgrTotalCount(dm) = 0;
    DirectoryMgrFilteredCount(dm) = 0;
    DirectoryRestart(DirectoryMgrDir(dm));
    lib_free(DirectoryMgrData(dm));
    lib_free(DirectoryMgrSortedPtrs(dm));
    head = NULL;
    f_func = DirectoryMgrFilterFunc(dm);
    f_data = &DirectoryMgrFilterData(dm);
    while (1) {
        cons = lib_malloc(sizeof(DirEntryCons));
        memset(cons, 0, sizeof(DirEntryCons));

        err = DirectoryReadNextEntry(DirectoryMgrDir(dm), &(cons->dir_entry));
        if (err == FALSE) {
            lib_free(cons);
            break;
        }
        ++DirectoryMgrTotalCount(dm);
        if ((f_func == NULL) || /* The next line make directories always
                                   appear, even if pattern is applied, AF 23jun98 */
             (DirEntryType(&cons->dir_entry) == F_TYPE_DIR) ||
             (f_func && f_func(&(cons->dir_entry), f_data))) {
            cons->next = NULL;
            if (head == NULL) {
                head = cons;
            } else {
                tail->next = cons;
            }
            tail = cons;
            ++DirectoryMgrFilteredCount(dm);
        } else {
            lib_free(cons);
        }
    }

    data_size = sizeof(DirEntry) * DirectoryMgrFilteredCount(dm);
    ptrs_size = sizeof(DirEntry *) * DirectoryMgrFilteredCount(dm);
    dm_data = lib_malloc(data_size);
    dm_ptrs = lib_malloc(ptrs_size);

    DirectoryMgrData(dm) = dm_data;
    DirectoryMgrSortedPtrs(dm) = dm_ptrs;

    for (i = 0; i < DirectoryMgrFilteredCount(dm); i++) {
        DirectoryMgrData(dm)[i] = head->dir_entry;
        DirectoryMgrSortedPtrs(dm)[i] = &(DirectoryMgrData(dm)[i]);
        cons = head->next;
        lib_free(head);
        head = cons;
    }

    DirectoryMgrResort(dm,DirectoryMgrCompFunc(dm));
    DirectoryMgrRestart(dm);
    return TRUE;
} /* End DirectoryMgrRefresh */

void DirectoryMgrResort(DirectoryMgr *dm, PFIcomp c_func)
{
    DirectoryMgrCompFunc(dm) = c_func;
    if (c_func != NULL) {
        qsort(DirectoryMgrSortedPtrs(dm), DirectoryMgrFilteredCount(dm), sizeof(DirEntry *), (int (*)(const void *, const void *))DirectoryMgrCompFunc(dm));
    }
    DirectoryMgrRestart(dm);
} /* End DirectoryMgrResort */

/*---------------------------------------------------------------------------*

                  I T E R A T I O N    C O M M A N D S

 *---------------------------------------------------------------------------*/

int DirectoryMgrGotoItem(DirectoryMgr *dm, int i)
{
    if (i < 0 || i >= DirectoryMgrFilteredCount(dm)) {
        return FALSE;
    }
    DirectoryMgrCurrentIndex(dm) = i;
    return TRUE;
} /* End DirectoryMgrGotoItem */

int DirectoryMgrGotoNamedItem(DirectoryMgr *dm, char *name)
{
    int i;
    DirEntry *entry;

    for (i = 0; i < DirectoryMgrFilteredCount(dm); i++) {
        entry = DirectoryMgrSortedPtrs(dm)[i];
        if (strcmp(DirEntryFileName(entry),name) == 0) {
            DirectoryMgrCurrentIndex(dm) = i;
            return TRUE;
        }
    }
    return FALSE;
} /* End DirectoryMgrGotoNamedItem */

void DirectoryMgrRestart(DirectoryMgr *dm)
{
    DirectoryMgrCurrentIndex(dm) = 0;
} /* End DirectoryMgrRestart */

DirEntry *DirectoryMgrCurrentEntry(DirectoryMgr *dm)
{
    int index;

    index = DirectoryMgrCurrentIndex(dm);
    if (index < 0 || index >= DirectoryMgrFilteredCount(dm)) {
        return NULL;
    }
    return DirectoryMgrSortedPtrs(dm)[index];
} /* End DirectoryMgrCurrentEntry */

DirEntry *DirectoryMgrNextEntry(DirectoryMgr *dm)
{
    int index;

    index = DirectoryMgrCurrentIndex(dm);
    if (index >= DirectoryMgrFilteredCount(dm)) {
        return NULL;
    }
    ++DirectoryMgrCurrentIndex(dm);
    return DirectoryMgrSortedPtrs(dm)[index];
} /* End DirectoryMgrNextEntry */

DirEntry *DirectoryMgrPrevEntry(DirectoryMgr *dm)
{
    int index;

    index = DirectoryMgrCurrentIndex(dm) - 1;
    if (index < 0) {
        return NULL;
    }
    --DirectoryMgrCurrentIndex(dm);
    return DirectoryMgrSortedPtrs(dm)[index];
} /* End DirectoryMgrPrevEntry */

/*---------------------------------------------------------------------------*

                   U T I L I T Y    F U N C T I O N S

 *---------------------------------------------------------------------------*/

int DirectoryMgrSimpleFilterFunc(char *pattern, PFI *ff_ptr, fwf_regex_t *fd_ptr)
{
    char regexp[2048];

    if (pattern[0] == '/') {
        *ff_ptr = DirectoryMgrFilterDirectories;
    } else {
        *ff_ptr = DirectoryMgrFilterName;
    }
    RegExpInit(fd_ptr);
    ShellPatternToRegExp(pattern, regexp, sizeof(regexp));
    if (RegExpCompile(regexp, fd_ptr)) {
        (void)RegExpCompile(".*", fd_ptr);
    }

    return TRUE;
} /* End DirectoryMgrSimpleFilterFunc */

int DirectoryMgrSimpleSortingFunc(int sort_type, PFIcomp *sf_ptr)
{
    *sf_ptr = NULL;
    switch (sort_type) {
        case DIR_MGR_SORT_NONE:
            break;
        case DIR_MGR_SORT_NAME:
            *sf_ptr = DirectoryMgrCompareName;
            break;
        case DIR_MGR_SORT_SIZE_ASCENDING:
            *sf_ptr = DirectoryMgrCompareSizeAscending;
            break;
        case DIR_MGR_SORT_SIZE_DESCENDING:
            *sf_ptr = DirectoryMgrCompareSizeDescending;
            break;
        case DIR_MGR_SORT_NAME_DIRS_FIRST:
            *sf_ptr = DirectoryMgrCompareNameDirsFirst;
            break;
        case DIR_MGR_SORT_ACCESS_ASCENDING:
            *sf_ptr = DirectoryMgrCompareLastAccessAscending;
            break;
        case DIR_MGR_SORT_ACCESS_DESCENDING:
            *sf_ptr = DirectoryMgrCompareLastAccessDescending;
            break;
        default:
            fprintf(stderr, "Bad sort type %d\n", sort_type);
            return(FALSE);
    }
    return TRUE;
} /* End DirectoryMgrSimpleSortingFunc */

/*---------------------------------------------------------------------------*

                    S O R T I N G    R O U T I N E S

 *---------------------------------------------------------------------------*/

int DirectoryMgrCompareName(DirEntry **e1p, DirEntry **e2p)
{
    return strcmp(DirEntryFileName(*e1p), DirEntryFileName(*e2p));
} /* End DirectoryMgrCompareName */

int DirectoryMgrCompareNameDirsFirst(DirEntry **e1p, DirEntry **e2p)
{
    if (DirEntryLeadsToDir(*e1p)) {
        if (!DirEntryLeadsToDir(*e2p)) {
            return -1;
        }
    } else if (DirEntryLeadsToDir(*e2p)) {
        return 1;
    }
    return strcmp(DirEntryFileName(*e1p), DirEntryFileName(*e2p));
} /* End DirectoryMgrCompareNameDirsFirst */

int DirectoryMgrCompareSizeAscending(DirEntry **e1p, DirEntry **e2p)
{
    if (DirEntryFileSize(*e1p) < DirEntryFileSize(*e2p)) {
        return -1;
    } else if (DirEntryFileSize(*e1p) == DirEntryFileSize(*e2p)) {
        return 0;
    } else {
        return 1;
    }
} /* End DirectoryMgrCompareSizeAscending */

int DirectoryMgrCompareSizeDescending(DirEntry **e1p, DirEntry **e2p)
{
    if (DirEntryFileSize(*e1p) > DirEntryFileSize(*e2p)) {
        return -1;
    } else if (DirEntryFileSize(*e1p) == DirEntryFileSize(*e2p)) {
        return 0;
    } else {
        return 1;
    }
} /* End DirectoryMgrCompareSizeDescending */

int DirectoryMgrCompareLastAccessAscending(DirEntry **e1p, DirEntry **e2p)
{
    return ((long)DirEntryLastAccess(*e1p) > (long)DirEntryLastAccess(*e2p));
} /* End DirectoryMgrCompareLastAccessAscending */

int DirectoryMgrCompareLastAccessDescending(DirEntry **e1p, DirEntry **e2p)
{
    return ((long)DirEntryLastAccess(*e1p) < (long)DirEntryLastAccess(*e2p));
} /* End DirectoryMgrCompareLastAccessDescending */

/*---------------------------------------------------------------------------*

                     F I L T E R    R O U T I N E S

 *---------------------------------------------------------------------------*/

int DirectoryMgrFilterName(DirEntry *de, fwf_regex_t *fsm)
{
    return RegExpMatch(DirEntryFileName(de), fsm);
} /* End DirectoryMgrFilterName */

int DirectoryMgrFilterDirectories(DirEntry *de, fwf_regex_t *fsm)
{
    return DirEntryIsDir(de);
} /* End DirectoryMgrFilterName */
