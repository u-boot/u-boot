/*
 * Copyright 1999 Egbert Eich
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/ioctl.h>
#include <sys/vt.h>
#include <sys/kd.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "debug.h"
#include "v86bios.h"

console
open_console(void)
{
    int fd;
    int VTno;
    char VTname[11];
    console Con = {-1,-1};
    struct vt_stat vts;

    if (NO_CONSOLE)
	    return Con;

    if ((fd = open("/dev/tty0",O_WRONLY,0)) < 0)
	return Con;

    if ((ioctl(fd, VT_OPENQRY, &VTno) < 0) || (VTno == -1)) {
	fprintf(stderr,"cannot get a vt\n");
	return Con;
    }

    close(fd);
    sprintf(VTname,"/dev/tty%i",VTno);

    if ((fd = open(VTname, O_RDWR|O_NDELAY, 0)) < 0) {
	fprintf(stderr,"cannot open console\n");
	return Con;
    }

    if (ioctl(fd, VT_GETSTATE, &vts) == 0)
	Con.vt = vts.v_active;

    if (ioctl(fd, VT_ACTIVATE, VTno) != 0) {
	fprintf(stderr,"cannot activate console\n");
	close(fd);
	return Con;
    }
    if (ioctl(fd, VT_WAITACTIVE, VTno) != 0) {
	fprintf(stderr,"wait for active console failed\n");
	close(fd);
	return Con;
    }
#if 0
    if (ioctl(fd, KDSETMODE, KD_GRAPHICS) < 0) {
	close(fd);
	return Con;
    }
#endif
    Con.fd = fd;
    return Con;
}

void
close_console(console Con)
{
    if (Con.fd == -1)
	return;

#if 0
    ioctl(Con.fd, KDSETMODE, KD_TEXT);
#endif
    if (Con.vt >=0)
	ioctl(Con.fd, VT_ACTIVATE, Con.vt);

    close(Con.fd);
}
