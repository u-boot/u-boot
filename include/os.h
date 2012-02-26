/*
 * Operating System Interface
 *
 * This provides access to useful OS routines for the sandbox architecture.
 * They are kept in a separate file so we can include system headers.
 *
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

#ifndef __OS_H__
#define __OS_H__

struct sandbox_state;

/**
 * Access to the OS read() system call
 *
 * \param fd	File descriptor as returned by os_open()
 * \param buf	Buffer to place data
 * \param count	Number of bytes to read
 * \return number of bytes read, or -1 on error
 */
ssize_t os_read(int fd, void *buf, size_t count);

/**
 * Access to the OS write() system call
 *
 * \param fd	File descriptor as returned by os_open()
 * \param buf	Buffer containing data to write
 * \param count	Number of bytes to write
 * \return number of bytes written, or -1 on error
 */
ssize_t os_write(int fd, const void *buf, size_t count);

/**
 * Access to the OS lseek() system call
 *
 * \param fd	File descriptor as returned by os_open()
 * \param offset	File offset (based on whence)
 * \param whence	Position offset is relative to (see below)
 * \return new file offset
 */
off_t os_lseek(int fd, off_t offset, int whence);

/* Defines for "whence" in os_lseek() */
#define OS_SEEK_SET	0
#define OS_SEEK_CUR	1
#define OS_SEEK_END	2

/**
 * Access to the OS open() system call
 *
 * \param pathname	Pathname of file to open
 * \param flags		Flags, like O_RDONLY, O_RDWR
 * \return file descriptor, or -1 on error
 */
int os_open(const char *pathname, int flags);

#define OS_O_RDONLY	0
#define OS_O_WRONLY	1
#define OS_O_RDWR	2
#define OS_O_MASK	3	/* Mask for read/write flags */
#define OS_O_CREAT	0100

/**
 * Access to the OS close() system call
 *
 * \param fd	File descriptor to close
 * \return 0 on success, -1 on error
 */
int os_close(int fd);

/**
 * Access to the OS exit() system call
 *
 * This exits with the supplied return code, which should be 0 to indicate
 * success.
 *
 * @param exit_code	exit code for U-Boot
 */
void os_exit(int exit_code) __attribute__((noreturn));

/**
 * Put tty into raw mode to mimic serial console better
 */
void os_tty_raw(int fd);

/**
 * Acquires some memory from the underlying os.
 *
 * \param length	Number of bytes to be allocated
 * \return Pointer to length bytes or NULL on error
 */
void *os_malloc(size_t length);

/**
 * Access to the usleep function of the os
 *
 * \param usec Time to sleep in micro seconds
 */
void os_usleep(unsigned long usec);

/**
 * Gets a monotonic increasing number of nano seconds from the OS
 *
 * \return A monotonic increasing time scaled in nano seconds
 */
u64 os_get_nsec(void);

/**
 * Parse arguments and update sandbox state.
 *
 * @param state		Sandbox state to update
 * @param argc		Argument count
 * @param argv		Argument vector
 * @return 0 if ok, and program should continue;
 *	1 if ok, but program should stop;
 *	-1 on error: program should terminate.
 */
int os_parse_args(struct sandbox_state *state, int argc, char *argv[]);

#endif
