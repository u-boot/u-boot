/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
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

#ifndef _INCLUDE_TPM_H_
#define _INCLUDE_TPM_H_

#include <common.h>

/*
 * tis_init()
 *
 * Initialize the TPM device. Returns 0 on success or -1 on
 * failure (in case device probing did not succeed).
 */
int tis_init(void);

/*
 * tis_open()
 *
 * Requests access to locality 0 for the caller. After all commands have been
 * completed the caller is supposed to call tis_close().
 *
 * Returns 0 on success, -1 on failure.
 */
int tis_open(void);

/*
 * tis_close()
 *
 * terminate the currect session with the TPM by releasing the locked
 * locality. Returns 0 on success of -1 on failure (in case lock
 * removal did not succeed).
 */
int tis_close(void);

/*
 * tis_sendrecv()
 *
 * Send the requested data to the TPM and then try to get its response
 *
 * @sendbuf - buffer of the data to send
 * @send_size size of the data to send
 * @recvbuf - memory to save the response to
 * @recv_len - pointer to the size of the response buffer
 *
 * Returns 0 on success (and places the number of response bytes at recv_len)
 * or -1 on failure.
 */
int tis_sendrecv(const uint8_t *sendbuf, size_t send_size, uint8_t *recvbuf,
			size_t *recv_len);

#endif /* _INCLUDE_TPM_H_ */
