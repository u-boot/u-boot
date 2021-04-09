// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 Intel Corporation <www.intel.com>
 *
 */

#include <asm/arch/handoff_soc64.h>
#include <asm/io.h>
#include <common.h>
#include <errno.h>
#include "log.h"

int socfpga_get_handoff_size(void *handoff_address, enum endianness endian)
{
	u32 size;

	size = readl(handoff_address + SOC64_HANDOFF_OFFSET_LENGTH);
	if (endian == BIG_ENDIAN)
		size = swab32(size);

	size = (size - SOC64_HANDOFF_OFFSET_DATA) / sizeof(u32);

	debug("%s: handoff address = 0x%p handoff size = 0x%08x\n", __func__,
	      (u32 *)handoff_address, size);

	return size;
}

int socfpga_handoff_read(void *handoff_address, void *table, u32 table_len,
			 enum endianness big_endian)
{
	u32 temp, i;
	u32 *table_x32 = table;

	debug("%s: handoff addr = 0x%p ", __func__, (u32 *)handoff_address);

	if (big_endian) {
		if (swab32(readl(SOC64_HANDOFF_BASE)) == SOC64_HANDOFF_MAGIC_BOOT) {
			debug("Handoff table address = 0x%p ", table_x32);
			debug("table length = 0x%x\n", table_len);
			debug("%s: handoff data =\n{\n", __func__);

			for (i = 0; i < table_len; i++) {
				temp = readl(handoff_address +
					     SOC64_HANDOFF_OFFSET_DATA +
					     (i * sizeof(u32)));
				*table_x32 = swab32(temp);

				if (!(i % 2))
					debug(" No.%d Addr 0x%08x: ", i,
					      *table_x32);
				else
					debug(" 0x%08x\n", *table_x32);

				table_x32++;
			}
			debug("\n}\n");
		} else {
			debug("%s: Cannot find SOC64_HANDOFF_MAGIC_BOOT ", __func__);
			debug("at addr  0x%p\n", (u32 *)handoff_address);
			return -EPERM;
		}
	}

	return 0;
}
