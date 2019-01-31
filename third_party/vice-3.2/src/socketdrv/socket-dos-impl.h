/*! \file socket-dos-impl.h \n
 *  \author Marco van den Heuvel\n
 *  \brief  Abstraction from network sockets.
 *
 * socket-dos-impl.h - Abstraction from network sockets. DOS implementation.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * based on code from network.c written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef VICE_SOCKET_DOS_IMPL_H
#define VICE_SOCKET_DOS_IMPL_H

#ifdef HAVE_NETWORK
 
#if !defined(HAVE_GETDTABLESIZE) && defined(HAVE_GETRLIMIT)
#include <sys/resource.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

typedef int SOCKET;
typedef struct timeval TIMEVAL;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#define in_addr_t unsigned long

#endif /* #ifdef HAVE_NETWORK */

#endif /* #ifndef VICE_SOCKET_DOS_IMPL_H */
