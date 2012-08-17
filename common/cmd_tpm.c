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

#include <common.h>
#include <command.h>
#include <tpm.h>

#define MAX_TRANSACTION_SIZE 30

/*
 * tpm_write() expects a variable number of parameters: the internal address
 * followed by data to write, byte by byte.
 *
 * Returns 0 on success or -1 on errors (wrong arguments or TPM failure).
 */
static int tpm_process(int argc, char * const argv[], cmd_tbl_t *cmdtp)
{
	u8 tpm_buffer[MAX_TRANSACTION_SIZE];
	u32 write_size, read_size;
	char *p;
	int rv = -1;

	for (write_size = 0; write_size < argc; write_size++) {
		u32 datum = simple_strtoul(argv[write_size], &p, 0);
		if (*p || (datum > 0xff)) {
			printf("\n%s: bad data value\n\n", argv[write_size]);
			cmd_usage(cmdtp);
			return rv;
		}
		tpm_buffer[write_size] = (u8)datum;
	}

	read_size = sizeof(tpm_buffer);
	if (!tis_sendrecv(tpm_buffer, write_size, tpm_buffer, &read_size)) {
		int i;
		puts("Got TPM response:\n");
		for (i = 0; i < read_size; i++)
			printf(" %2.2x", tpm_buffer[i]);
		puts("\n");
		rv = 0;
	} else {
		puts("tpm command failed\n");
	}
	return rv;
}

static int do_tpm(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rv = 0;

	/*
	 * Verify that in case it is present, the first argument, it is
	 * exactly one character in size.
	 */
	if (argc < 7) {
		puts("command should be at least six bytes in size\n");
		return -1;
	}

	if (tis_init()) {
		puts("tis_init() failed!\n");
		return -1;
	}

	if (tis_open()) {
		puts("tis_open() failed!\n");
		return -1;
	}

	rv = tpm_process(argc - 1, argv + 1, cmdtp);

	if (tis_close()) {
		puts("tis_close() failed!\n");
		rv = -1;
	}

	return rv;
}

U_BOOT_CMD(tpm, MAX_TRANSACTION_SIZE, 1, do_tpm,
	   "<byte> [<byte> ...]   - write data and read response",
	   "send arbitrary data (at least 6 bytes) to the TPM "
	   "device and read the response"
);
