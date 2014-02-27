/*
 * Operating System Interface
 *
 * This provides access to useful OS routines for the sandbox architecture.
 * They are kept in a separate file so we can include system headers.
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __OS_H__
#define __OS_H__

#include <linux/types.h>

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
 * Access to the OS read() system call with non-blocking access
 *
 * \param fd	File descriptor as returned by os_open()
 * \param buf	Buffer to place data
 * \param count	Number of bytes to read
 * \return number of bytes read, or -1 on error
 */
ssize_t os_read_no_block(int fd, void *buf, size_t count);

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
 * Access to the OS unlink() system call
 *
 * \param pathname Path of file to delete
 * \return 0 for success, other for error
 */
int os_unlink(const char *pathname);

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
 *
 * @param fd		File descriptor of stdin (normally 0)
 * @param allow_sigs	Allow Ctrl-C, Ctrl-Z to generate signals rather than
 *			be handled by U-Boot
 */
void os_tty_raw(int fd, bool allow_sigs);

/**
 * Acquires some memory from the underlying os.
 *
 * \param length	Number of bytes to be allocated
 * \return Pointer to length bytes or NULL on error
 */
void *os_malloc(size_t length);

/**
 * Free memory previous allocated with os_malloc()/os_realloc()
 *
 * This returns the memory to the OS.
 *
 * \param ptr		Pointer to memory block to free
 */
void os_free(void *ptr);

/**
 * Reallocate previously-allocated memory to increase/decrease space
 *
 * This works in a similar way to the C library realloc() function. If
 * length is 0, then ptr is freed. Otherwise the space used by ptr is
 * expanded or reduced depending on whether length is larger or smaller
 * than before.
 *
 * If ptr is NULL, then this is similar to calling os_malloc().
 *
 * This function may need to move the memory block to make room for any
 * extra space, in which case the new pointer is returned.
 *
 * \param ptr		Pointer to memory block to reallocate
 * \param length	New length for memory block
 * \return pointer to new memory block, or NULL on failure or if length
 *	is 0.
 */
void *os_realloc(void *ptr, size_t length);

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
uint64_t os_get_nsec(void);

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

/*
 * Types of directory entry that we support. See also os_dirent_typename in
 * the C file.
 */
enum os_dirent_t {
	OS_FILET_REG,		/* Regular file */
	OS_FILET_LNK,		/* Symbolic link */
	OS_FILET_DIR,		/* Directory */
	OS_FILET_UNKNOWN,	/* Something else */

	OS_FILET_COUNT,
};

/** A directory entry node, containing information about a single dirent */
struct os_dirent_node {
	struct os_dirent_node *next;	/* Pointer to next node, or NULL */
	ulong size;			/* Size of file in bytes */
	enum os_dirent_t type;		/* Type of entry */
	char name[0];			/* Name of entry */
};

/**
 * Get a directionry listing
 *
 * This allocates and returns a linked list containing the directory listing.
 *
 * @param dirname	Directory to examine
 * @param headp		Returns pointer to head of linked list, or NULL if none
 * @return 0 if ok, -ve on error
 */
int os_dirent_ls(const char *dirname, struct os_dirent_node **headp);

/**
 * Get the name of a directory entry type
 *
 * @param type		Type to cehck
 * @return string containing the name of that type, or "???" if none/invalid
 */
const char *os_dirent_get_typename(enum os_dirent_t type);

/**
 * Get the size of a file
 *
 * @param fname		Filename to check
 * @return size of file, or -1 if an error ocurred
 */
ssize_t os_get_filesize(const char *fname);

/**
 * Write a character to the controlling OS terminal
 *
 * This bypasses the U-Boot console support and writes directly to the OS
 * stdout file descriptor.
 *
 * @param ch	Character to write
 */
void os_putc(int ch);

/**
 * Write a string to the controlling OS terminal
 *
 * This bypasses the U-Boot console support and writes directly to the OS
 * stdout file descriptor.
 *
 * @param str	String to write (note that \n is not appended)
 */
void os_puts(const char *str);

/**
 * Write the sandbox RAM buffer to a existing file
 *
 * @param fname		Filename to write memory to (simple binary format)
 * @return 0 if OK, -ve on error
 */
int os_write_ram_buf(const char *fname);

/**
 * Read the sandbox RAM buffer from an existing file
 *
 * @param fname		Filename containing memory (simple binary format)
 * @return 0 if OK, -ve on error
 */
int os_read_ram_buf(const char *fname);

/**
 * Jump to a new executable image
 *
 * This uses exec() to run a new executable image, after putting it in a
 * temporary file. The same arguments and environment are passed to this
 * new image, with the addition of:
 *
 *	-j <filename>	Specifies the filename the image was written to. The
 *			calling image may want to delete this at some point.
 *	-m <filename>	Specifies the file containing the sandbox memory
 *			(ram_buf) from this image, so that the new image can
 *			have access to this. It also means that the original
 *			memory filename passed to U-Boot will be left intact.
 *
 * @param dest		Buffer containing executable image
 * @param size		Size of buffer
 */
int os_jump_to_image(const void *dest, int size);

#endif
