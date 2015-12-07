/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __TIS_H
#define __TIS_H

#ifndef CONFIG_DM_TPM

#include <common.h>

/* Low-level interface to access TPM */

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
#endif

#endif /* __TIS_H */
