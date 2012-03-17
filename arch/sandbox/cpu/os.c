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

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/types.h>

#include <asm/getopt.h>
#include <asm/sections.h>
#include <asm/state.h>
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

off_t os_lseek(int fd, off_t offset, int whence)
{
	if (whence == OS_SEEK_SET)
		whence = SEEK_SET;
	else if (whence == OS_SEEK_CUR)
		whence = SEEK_CUR;
	else if (whence == OS_SEEK_END)
		whence = SEEK_END;
	else
		os_exit(1);
	return lseek(fd, offset, whence);
}

int os_open(const char *pathname, int os_flags)
{
	int flags;

	switch (os_flags & OS_O_MASK) {
	case OS_O_RDONLY:
	default:
		flags = O_RDONLY;
		break;

	case OS_O_WRONLY:
		flags = O_WRONLY;
		break;

	case OS_O_RDWR:
		flags = O_RDWR;
		break;
	}

	if (os_flags & OS_O_CREAT)
		flags |= O_CREAT;

	return open(pathname, flags, 0777);
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

void os_usleep(unsigned long usec)
{
	usleep(usec);
}

u64 os_get_nsec(void)
{
#if defined(CLOCK_MONOTONIC) && defined(_POSIX_MONOTONIC_CLOCK)
	struct timespec tp;
	if (EINVAL == clock_gettime(CLOCK_MONOTONIC, &tp)) {
		struct timeval tv;

		gettimeofday(&tv, NULL);
		tp.tv_sec = tv.tv_sec;
		tp.tv_nsec = tv.tv_usec * 1000;
	}
	return tp.tv_sec * 1000000000ULL + tp.tv_nsec;
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000000ULL + tv.tv_usec * 1000;
#endif
}

static char *short_opts;
static struct option *long_opts;

int os_parse_args(struct sandbox_state *state, int argc, char *argv[])
{
	struct sb_cmdline_option **sb_opt = __u_boot_sandbox_option_start;
	size_t num_options = __u_boot_sandbox_option_count();
	size_t i;

	int hidden_short_opt;
	size_t si;

	int c;

	if (short_opts || long_opts)
		return 1;

	state->argc = argc;
	state->argv = argv;

	/* dynamically construct the arguments to the system getopt_long */
	short_opts = os_malloc(sizeof(*short_opts) * num_options * 2 + 1);
	long_opts = os_malloc(sizeof(*long_opts) * num_options);
	if (!short_opts || !long_opts)
		return 1;

	/*
	 * getopt_long requires "val" to be unique (since that is what the
	 * func returns), so generate unique values automatically for flags
	 * that don't have a short option.  pick 0x100 as that is above the
	 * single byte range (where ASCII/ISO-XXXX-X charsets live).
	 */
	hidden_short_opt = 0x100;
	si = 0;
	for (i = 0; i < num_options; ++i) {
		long_opts[i].name = sb_opt[i]->flag;
		long_opts[i].has_arg = sb_opt[i]->has_arg ?
			required_argument : no_argument;
		long_opts[i].flag = NULL;

		if (sb_opt[i]->flag_short) {
			short_opts[si++] = long_opts[i].val = sb_opt[i]->flag_short;
			if (long_opts[i].has_arg == required_argument)
				short_opts[si++] = ':';
		} else
			long_opts[i].val = sb_opt[i]->flag_short = hidden_short_opt++;
	}
	short_opts[si] = '\0';

	/* we need to handle output ourselves since u-boot provides printf */
	opterr = 0;

	/*
	 * walk all of the options the user gave us on the command line,
	 * figure out what u-boot option structure they belong to (via
	 * the unique short val key), and call the appropriate callback.
	 */
	while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
		for (i = 0; i < num_options; ++i) {
			if (sb_opt[i]->flag_short == c) {
				if (sb_opt[i]->callback(state, optarg)) {
					state->parse_err = sb_opt[i]->flag;
					return 0;
				}
				break;
			}
		}
		if (i == num_options) {
			/*
			 * store the faulting flag for later display.  we have to
			 * store the flag itself as the getopt parsing itself is
			 * tricky: need to handle the following flags (assume all
			 * of the below are unknown):
			 *   -a        optopt='a' optind=<next>
			 *   -abbbb    optopt='a' optind=<this>
			 *   -aaaaa    optopt='a' optind=<this>
			 *   --a       optopt=0   optind=<this>
			 * as you can see, it is impossible to determine the exact
			 * faulting flag without doing the parsing ourselves, so
			 * we just report the specific flag that failed.
			 */
			if (optopt) {
				static char parse_err[3] = { '-', 0, '\0', };
				parse_err[1] = optopt;
				state->parse_err = parse_err;
			} else
				state->parse_err = argv[optind - 1];
			break;
		}
	}

	return 0;
}
