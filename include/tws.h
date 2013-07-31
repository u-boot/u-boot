/*
 * (C) Copyright 2009
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _TWS_H_
#define _TWS_H_

/*
 * Read/Write interface:
 *   buffer:  Where to read/write the data
 *   len:     How many bits to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int tws_read(uchar *buffer, int len);
int tws_write(uchar *buffer, int len);

#endif	/* _TWS_H_ */
