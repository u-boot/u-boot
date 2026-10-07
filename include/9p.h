/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#ifndef __9P_H__
#define __9P_H__

#include <linux/types.h>

#define P9_MSIZE            65536
#define P9_NOFID            (~0U)
#define P9_NOTAG            0xFFFF
#define P9_REQ_TAG          1

#define P9_RLERROR          7
#define P9_TLOPEN           12
#define P9_RLOPEN           13
#define P9_TGETATTR         24
#define P9_RGETATTR         25
#define P9_TREADDIR         40
#define P9_RREADDIR         41
#define P9_TVERSION         100
#define P9_RVERSION         101
#define P9_TATTACH          104
#define P9_RATTACH          105
#define P9_TWALK            110
#define P9_RWALK            111
#define P9_TREAD            116
#define P9_RREAD            117
#define P9_TCLUNK           120
#define P9_RCLUNK           121

#define P9_O_RDONLY         00000000
#define P9_GETATTR_SIZE     0x00000010ULL
#define P9_GETATTR_SIZE_OFFSET  41
#define P9_DT_DIR           4

/**
 * struct p9_dirent_hdr - 9P2000.L directory entry wire header
 * @qid: File QID (13 bytes)
 * @offset: Offset cookie for next readdir (8 bytes)
 * @type: Directory entry type (1 byte)
 * @name_len: Length of filename (2 bytes)
 */
struct p9_dirent_hdr {
	u8  qid[13];
	u64 offset;
	u8  type;
	u16 name_len;
} __packed;

/**
 * struct p9_header - 9P message header
 * @size: Total message size in bytes, including this header
 * @id: Message type / opcode (e.g. P9_TVERSION)
 * @tag: Message tag to match request and reply
 */
struct p9_header {
	u32 size;
	u8  id;
	u16 tag;
} __packed;

struct udevice;

/**
 * struct dm_p9_ops - Driver model operations for 9P transports
 * @request: Execute a transmit/receive transaction
 * @get_mount_tag: (optional) Retrieve the mount tag string
 */
struct dm_p9_ops {
	int (*request)(struct udevice *dev, void *tx, int tx_len, void *rx, int rx_len);
	const char *(*get_mount_tag)(struct udevice *dev);
};

/**
 * struct p9_client - 9P client state
 * @dev: Underlying transport udevice
 * @tx: Transmit buffer
 * @rx: Receive buffer
 * @next_fid: Next FID to allocate
 * @msize: Negotiated maximum message size
 */
struct p9_client {
	struct udevice *dev;
	u8 *tx;
	u8 *rx;
	u32 next_fid;
	u32 msize;
};

/**
 * p9_get_device() - Find a 9P transport device by mount tag or default
 * @tag: Mount tag to match, or NULL/"-" for default/first device
 * @devp: Output pointer to the found udevice
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_get_device(const char *tag, struct udevice **devp);

/**
 * p9_get_client() - Get 9P client state from a transport udevice
 * @dev: 9P transport udevice
 *
 * Return: Pointer to struct p9_client
 */
struct p9_client *p9_get_client(struct udevice *dev);

/**
 * p9_client_alloc_fid() - Allocate a unique FID
 * @c: Pointer to 9P client
 *
 * Return: Newly allocated FID
 */
u32 p9_client_alloc_fid(struct p9_client *c);

/**
 * p9_client_version() - Negotiate 9P2000.L protocol version and msize
 * @c: Pointer to 9P client
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_version(struct p9_client *c);

/**
 * p9_client_attach() - Attach to the root directory
 * @c: Pointer to 9P client
 * @fid: FID to assign to root
 * @uname: User name
 * @aname: File tree / mount point name
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_attach(struct p9_client *c, u32 fid, const char *uname, const char *aname);

/**
 * p9_client_walk() - Walk a path and associate with a new FID
 * @c: Pointer to 9P client
 * @base_fid: Starting directory FID
 * @new_fid: New FID to associate with target path
 * @path: Relative path to walk
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_walk(struct p9_client *c, u32 base_fid, u32 new_fid, const char *path);

/**
 * p9_client_lopen() - Open a file (9P2000.L)
 * @c: Pointer to 9P client
 * @fid: FID of file to open
 * @flags: Open flags (e.g. P9_O_RDONLY)
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_lopen(struct p9_client *c, u32 fid, u32 flags);

/**
 * p9_client_clunk() - Forget / close a FID
 * @c: Pointer to 9P client
 * @fid: FID to clunk
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_clunk(struct p9_client *c, u32 fid);

/**
 * p9_client_stat() - Get file size attribute via Tgetattr
 * @c: Pointer to 9P client
 * @fid: FID of file
 * @size: Output pointer for file size
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_stat(struct p9_client *c, u32 fid, loff_t *size);

/**
 * p9_client_read() - Read data from a file
 * @c: Pointer to 9P client
 * @fid: FID of file
 * @offset: Byte offset in file to read from
 * @count: Maximum number of bytes to read
 * @buf: Destination buffer
 * @actread: Output pointer for actual bytes read
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_read(struct p9_client *c, u32 fid, u64 offset, u32 count, void *buf, u32 *actread);

/**
 * p9_client_readdir() - Read directory entries
 * @c: Pointer to 9P client
 * @fid: FID of directory
 * @offset: Directory offset / cookie
 * @count: Maximum bytes of dirents to read
 * @buf: Destination buffer
 * @actread: Output pointer for actual bytes returned
 *
 * Return: 0 on success, negative error code on failure
 */
int p9_client_readdir(struct p9_client *c, u32 fid, u64 offset, u32 count, void *buf, u32 *actread);

#endif /* __9P_H__ */
