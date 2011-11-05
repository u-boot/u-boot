/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <fcntl.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <os.h>

/* Operating System Interface */

ssize_t os_read(int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

ssize_t os_write(int fd, const void *buf, size_t count)
{
	return write(fd, buf, count);
}

int os_open(const char *pathname, int flags)
{
	return open(pathname, flags);
}

int os_close(int fd)
{
	return close(fd);
}

void os_exit(int exit_code)
{
	exit(exit_code);
}

/* Restore tty state when we exit */
static struct termios orig_term;

static void os_fd_restore(void)
{
	tcsetattr(0, TCSANOW, &orig_term);
}

/* Put tty into raw mode so <tab> and <ctrl+c> work */
void os_tty_raw(int fd)
{
	static int setup = 0;
	struct termios term;

	if (setup)
		return;
	setup = 1;

	/* If not a tty, don't complain */
	if (tcgetattr(fd, &orig_term))
		return;

	term = orig_term;
	term.c_iflag = IGNBRK | IGNPAR;
	term.c_oflag = OPOST | ONLCR;
	term.c_cflag = CS8 | CREAD | CLOCAL;
	term.c_lflag = 0;
	if (tcsetattr(fd, TCSANOW, &term))
		return;

	atexit(os_fd_restore);
}

void *os_malloc(size_t length)
{
	return mmap(NULL, length, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}
