/*
 * Copyright (C) 2013 Guntermann & Drunck, GmbH
 *
 * Written by Dirk Eibach <eibach@gdsys.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <tpm.h>
#include <i2c.h>
#include <asm/unaligned.h>

#define ATMEL_TPM_TIMEOUT_MS 5000 /* sufficient for anything but
				     generating/exporting keys */

/*
 * tis_init()
 *
 * Initialize the TPM device. Returns 0 on success or -1 on
 * failure (in case device probing did not succeed).
 */
int tis_init(void)
{
	return 0;
}

/*
 * tis_open()
 *
 * Requests access to locality 0 for the caller. After all commands have been
 * completed the caller is supposed to call tis_close().
 *
 * Returns 0 on success, -1 on failure.
 */
int tis_open(void)
{
	return 0;
}

/*
 * tis_close()
 *
 * terminate the currect session with the TPM by releasing the locked
 * locality. Returns 0 on success of -1 on failure (in case lock
 * removal did not succeed).
 */
int tis_close(void)
{
	return 0;
}

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
			size_t *recv_len)
{
	int res;
	unsigned long start;

#ifdef DEBUG
	memset(recvbuf, 0xcc, *recv_len);
	printf("send to TPM (%d bytes, recv_len=%d):\n", send_size, *recv_len);
	print_buffer(0, (void *)sendbuf, 1, send_size, 0);
#endif

	res = i2c_write(0x29, 0, 0, (uchar *)sendbuf, send_size);
	if (res) {
		printf("i2c_write returned %d\n", res);
		return -1;
	}

	start = get_timer(0);
	while ((res = i2c_read(0x29, 0, 0, recvbuf, 10))) {
		if (get_timer(start) > ATMEL_TPM_TIMEOUT_MS) {
			puts("tpm timed out\n");
			return -1;
		}
		udelay(100);
	}
	if (!res) {
		*recv_len = get_unaligned_be32(recvbuf + 2);
		if (*recv_len > 10)
			res = i2c_read(0x29, 0, 0, recvbuf, *recv_len);
	}
	if (res) {
		printf("i2c_read returned %d (rlen=%d)\n", res, *recv_len);
#ifdef DEBUG
		print_buffer(0, recvbuf, 1, *recv_len, 0);
#endif
	}

#ifdef DEBUG
	if (!res) {
		printf("read from TPM (%d bytes):\n", *recv_len);
		print_buffer(0, recvbuf, 1, *recv_len, 0);
	}
#endif

	return res;
}
