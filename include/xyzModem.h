/*
 *==========================================================================
 *
 *      xyzModem.h
 *
 *      RedBoot stream handler for xyzModem protocol
 *
 *==========================================================================
 *####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating System.
 * Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
 * Copyright (C) 2002 Gary Thomas
 *
 * eCos is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 or (at your option) any later version.
 *
 * eCos is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with eCos; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.
 *
 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 *
 * Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
 * at http: *sources.redhat.com/ecos/ecos-license/
 * -------------------------------------------
 *####ECOSGPLCOPYRIGHTEND####
 *==========================================================================
 *#####DESCRIPTIONBEGIN####
 *
 * Author(s):    gthomas
 * Contributors: gthomas
 * Date:         2000-07-14
 * Purpose:
 * Description:
 *
 * This code is part of RedBoot (tm).
 *
 *####DESCRIPTIONEND####
 *
 *==========================================================================
 */

#ifndef _XYZMODEM_H_
#define _XYZMODEM_H_

#define xyzModem_xmodem 1
#define xyzModem_ymodem 2
/* Don't define this until the protocol support is in place */
/*#define xyzModem_zmodem 3 */

#define xyzModem_access   -1
#define xyzModem_noZmodem -2
#define xyzModem_timeout  -3
#define xyzModem_eof      -4
#define xyzModem_cancel   -5
#define xyzModem_frame    -6
#define xyzModem_cksum    -7
#define xyzModem_sequence -8

#define xyzModem_close 1
#define xyzModem_abort 2


#ifdef REDBOOT
extern getc_io_funcs_t xyzModem_io;
#else
#define CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT
#define CYGACC_CALL_IF_SET_CONSOLE_COMM(x)

#define diag_vprintf vprintf
#define diag_printf printf
#define diag_vsprintf vsprintf

#define CYGACC_CALL_IF_DELAY_US(x) udelay(x)

typedef struct {
    char *filename;
    int   mode;
    int   chan;
#ifdef CYGPKG_REDBOOT_NETWORKING
    struct sockaddr_in *server;
#endif
} connection_info_t;

#ifndef	BOOL_WAS_DEFINED
#define BOOL_WAS_DEFINED
typedef unsigned int bool;
#endif

#define false 0
#define true 1

#endif


int   xyzModem_stream_open(connection_info_t *info, int *err);
void  xyzModem_stream_close(int *err);
void  xyzModem_stream_terminate(bool method, int (*getc)(void));
int   xyzModem_stream_read(char *buf, int size, int *err);
char *xyzModem_error(int err);

#endif /* _XYZMODEM_H_ */
