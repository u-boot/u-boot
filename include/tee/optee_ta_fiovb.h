/* SPDX-License-Identifier: BSD-2-Clause */
/* Copyright (c) 2018, Linaro Limited */
/* Copyright (c) 2019, Foundries.IO */

#ifndef __TA_FIOVB_H
#define __TA_FIOVB_H

#define TA_FIOVB_UUID {0x22250a54, 0x0bf1, 0x48fe, \
		      { 0x80, 0x02, 0x7b, 0x20, 0xf1, 0xc9, 0xc9, 0xb1 } }

#define PERSIST_VALUE_LIST {"bootcount", "upgrade_available", "rollback", \
			    "m4hash", "m4size"}

/*
 * Reads a persistent value corresponding to the given name.
 *
 * in	params[0].memref:	persistent value name
 * out	params[1].memref:	read persistent value buffer
 */
#define TA_FIOVB_CMD_READ_PERSIST_VALUE		0

/*
 * Writes a persistent value corresponding to the given name.
 *
 * in	params[0].memref:	persistent value name
 * in	params[1].memref:	persistent value buffer to write
 */
#define TA_FIOVB_CMD_WRITE_PERSIST_VALUE	1

/*
 * Delete a persistent object corresponding to the given name.
 *
 * in	params[0].memref:	persistent value name
 */
#define TA_FIOVB_CMD_DELETE_PERSIST_VALUE	2

#endif /*__TA_FIOVB_H*/
