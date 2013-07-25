/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*************************************************************************
 * Altera Nios-II EPCS Controller Core interfaces
 ************************************************************************/

#ifndef __NIOS2_EPCS_H__
#define __NIOS2_EPCS_H__

typedef struct epcs_devinfo_t {
	const char	*name;		/* Device name */
	unsigned char	id;		/* Device silicon id */
	unsigned char	size;		/* Total size log2(bytes)*/
	unsigned char	num_sects;	/* Number of sectors */
	unsigned char	sz_sect;	/* Sector size log2(bytes) */
	unsigned char	sz_page;	/* Page size log2(bytes) */
	unsigned char   prot_mask;	/* Protection mask */
}epcs_devinfo_t;

/* Resets the epcs controller -- to prevent (potential) soft-reset
 * problems when booting from the epcs controller
 */
extern int epcs_reset (void);

/* Returns the devinfo struct if EPCS device is found;
 * NULL otherwise.
 */
extern epcs_devinfo_t *epcs_dev_find (void);

/* Returns the number of bytes used by config data.
 * Negative on error.
 */
extern int epcs_cfgsz (void);

/* Erase sectors 'start' to 'end' - return zero on success
 */
extern int epcs_erase (unsigned start, unsigned end);

/* Read 'cnt' bytes from device offset 'off' into buf at 'addr'
 * Zero return on success
 */
extern int epcs_read (ulong addr, ulong off, ulong cnt);

/* Write 'cnt' bytes to device offset 'off' from buf at 'addr'.
 * Zero return on success
 */
extern int epcs_write (ulong addr, ulong off, ulong cnt);

/* Verify 'cnt' bytes at device offset 'off' comparing with buf
 * at 'addr'. On failure, write first invalid offset to *err.
 * Zero return on success
 */
extern int epcs_verify (ulong addr, ulong off, ulong cnt, ulong *err);

#endif /* __NIOS2_EPCS_H__ */
