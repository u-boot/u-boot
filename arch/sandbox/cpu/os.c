/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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

struct os_mem_hdr {
	size_t length;		/* number of bytes in the block */
};

ssize_t os_read(int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

ssize_t os_read_no_block(int fd, void *buf, size_t count)
{
	const int flags = fcntl(fd, F_GETFL, 0);

	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	return os_read(fd, buf, count);
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

int os_unlink(const char *pathname)
{
	return unlink(pathname);
}

void os_exit(int exit_code)
{
	exit(exit_code);
}

/* Restore tty state when we exit */
static struct termios orig_term;
static bool term_setup;

static void os_fd_restore(void)
{
	if (term_setup)
		tcsetattr(0, TCSANOW, &orig_term);
}

/* Put tty into raw mode so <tab> and <ctrl+c> work */
void os_tty_raw(int fd, bool allow_sigs)
{
	struct termios term;

	if (term_setup)
		return;
	term_setup = true;

	/* If not a tty, don't complain */
	if (tcgetattr(fd, &orig_term))
		return;

	term = orig_term;
	term.c_iflag = IGNBRK | IGNPAR;
	term.c_oflag = OPOST | ONLCR;
	term.c_cflag = CS8 | CREAD | CLOCAL;
	term.c_lflag = allow_sigs ? ISIG : 0;
	if (tcsetattr(fd, TCSANOW, &term))
		return;

	atexit(os_fd_restore);
}

void *os_malloc(size_t length)
{
	struct os_mem_hdr *hdr;

	hdr = mmap(NULL, length + sizeof(*hdr), PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (hdr == MAP_FAILED)
		return NULL;
	hdr->length = length;

	return hdr + 1;
}

void os_free(void *ptr)
{
	struct os_mem_hdr *hdr = ptr;

	hdr--;
	if (ptr)
		munmap(hdr, hdr->length + sizeof(*hdr));
}

void *os_realloc(void *ptr, size_t length)
{
	struct os_mem_hdr *hdr = ptr;
	void *buf = NULL;

	hdr--;
	if (length != 0) {
		buf = os_malloc(length);
		if (!buf)
			return buf;
		if (ptr) {
			if (length > hdr->length)
				length = hdr->length;
			memcpy(buf, ptr, length);
		}
	}
	os_free(ptr);

	return buf;
}

void os_usleep(unsigned long usec)
{
	usleep(usec);
}

uint64_t __attribute__((no_instrument_function)) os_get_nsec(void)
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
	struct sandbox_cmdline_option **sb_opt = __u_boot_sandbox_option_start;
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

void os_dirent_free(struct os_dirent_node *node)
{
	struct os_dirent_node *next;

	while (node) {
		next = node->next;
		free(node);
		node = next;
	}
}

int os_dirent_ls(const char *dirname, struct os_dirent_node **headp)
{
	struct dirent entry, *result;
	struct os_dirent_node *head, *node, *next;
	struct stat buf;
	DIR *dir;
	int ret;
	char *fname;
	int len;

	*headp = NULL;
	dir = opendir(dirname);
	if (!dir)
		return -1;

	/* Create a buffer for the maximum filename length */
	len = sizeof(entry.d_name) + strlen(dirname) + 2;
	fname = malloc(len);
	if (!fname) {
		ret = -ENOMEM;
		goto done;
	}

	for (node = head = NULL;; node = next) {
		ret = readdir_r(dir, &entry, &result);
		if (ret || !result)
			break;
		next = malloc(sizeof(*node) + strlen(entry.d_name) + 1);
		if (!next) {
			os_dirent_free(head);
			ret = -ENOMEM;
			goto done;
		}
		next->next = NULL;
		strcpy(next->name, entry.d_name);
		switch (entry.d_type) {
		case DT_REG:
			next->type = OS_FILET_REG;
			break;
		case DT_DIR:
			next->type = OS_FILET_DIR;
			break;
		case DT_LNK:
			next->type = OS_FILET_LNK;
			break;
		}
		next->size = 0;
		snprintf(fname, len, "%s/%s", dirname, next->name);
		if (!stat(fname, &buf))
			next->size = buf.st_size;
		if (node)
			node->next = next;
		if (!head)
			head = node;
	}
	*headp = head;

done:
	closedir(dir);
	free(fname);
	return ret;
}

const char *os_dirent_typename[OS_FILET_COUNT] = {
	"   ",
	"SYM",
	"DIR",
	"???",
};

const char *os_dirent_get_typename(enum os_dirent_t type)
{
	if (type >= 0 && type < OS_FILET_COUNT)
		return os_dirent_typename[type];

	return os_dirent_typename[OS_FILET_UNKNOWN];
}

int os_get_filesize(const char *fname, loff_t *size)
{
	struct stat buf;
	int ret;

	ret = stat(fname, &buf);
	if (ret)
		return ret;
	*size = buf.st_size;
	return 0;
}

void os_putc(int ch)
{
	putchar(ch);
}

void os_puts(const char *str)
{
	while (*str)
		os_putc(*str++);
}

int os_write_ram_buf(const char *fname)
{
	struct sandbox_state *state = state_get_current();
	int fd, ret;

	fd = open(fname, O_CREAT | O_WRONLY, 0777);
	if (fd < 0)
		return -ENOENT;
	ret = write(fd, state->ram_buf, state->ram_size);
	close(fd);
	if (ret != state->ram_size)
		return -EIO;

	return 0;
}

int os_read_ram_buf(const char *fname)
{
	struct sandbox_state *state = state_get_current();
	int fd, ret;
	loff_t size;

	ret = os_get_filesize(fname, &size);
	if (ret < 0)
		return ret;
	if (size != state->ram_size)
		return -ENOSPC;
	fd = open(fname, O_RDONLY);
	if (fd < 0)
		return -ENOENT;

	ret = read(fd, state->ram_buf, state->ram_size);
	close(fd);
	if (ret != state->ram_size)
		return -EIO;

	return 0;
}

static int make_exec(char *fname, const void *data, int size)
{
	int fd;

	strcpy(fname, "/tmp/u-boot.jump.XXXXXX");
	fd = mkstemp(fname);
	if (fd < 0)
		return -ENOENT;
	if (write(fd, data, size) < 0)
		return -EIO;
	close(fd);
	if (chmod(fname, 0777))
		return -ENOEXEC;

	return 0;
}

static int add_args(char ***argvp, const char *add_args[], int count)
{
	char **argv;
	int argc;

	for (argv = *argvp, argc = 0; (*argvp)[argc]; argc++)
		;

	argv = malloc((argc + count + 1) * sizeof(char *));
	if (!argv) {
		printf("Out of memory for %d argv\n", count);
		return -ENOMEM;
	}
	memcpy(argv, *argvp, argc * sizeof(char *));
	memcpy(argv + argc, add_args, count * sizeof(char *));
	argv[argc + count] = NULL;

	*argvp = argv;
	return 0;
}

int os_jump_to_image(const void *dest, int size)
{
	struct sandbox_state *state = state_get_current();
	char fname[30], mem_fname[30];
	int fd, err;
	const char *extra_args[5];
	char **argv = state->argv;
#ifdef DEBUG
	int argc, i;
#endif

	err = make_exec(fname, dest, size);
	if (err)
		return err;

	strcpy(mem_fname, "/tmp/u-boot.mem.XXXXXX");
	fd = mkstemp(mem_fname);
	if (fd < 0)
		return -ENOENT;
	close(fd);
	err = os_write_ram_buf(mem_fname);
	if (err)
		return err;

	os_fd_restore();

	extra_args[0] = "-j";
	extra_args[1] = fname;
	extra_args[2] = "-m";
	extra_args[3] = mem_fname;
	extra_args[4] = "--rm_memory";
	err = add_args(&argv, extra_args,
		       sizeof(extra_args) / sizeof(extra_args[0]));
	if (err)
		return err;

#ifdef DEBUG
	for (i = 0; argv[i]; i++)
		printf("%d %s\n", i, argv[i]);
#endif

	if (state_uninit())
		os_exit(2);

	err = execv(fname, argv);
	free(argv);
	if (err)
		return err;

	return unlink(fname);
}
