// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <dm.h>
#include <env.h>
#include <event.h>
#include <asm/io.h>
#include <asm/arch/gcr.h>
#include <asm/mach-types.h>
#include "../common/uart.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	struct npcm_gcr *gcr = (struct npcm_gcr *)NPCM_GCR_BA;

	int ramsize = (readl(&gcr->intcr3) >> 8) & 0x7;

	switch (ramsize) {
	case 0:
		gd->ram_size = 0x08000000; /* 128 MB. */
		break;
	case 1:
		gd->ram_size = 0x10000000; /* 256 MB. */
		break;
	case 2:
		gd->ram_size = 0x20000000; /* 512 MB. */
		break;
	case 3:
		gd->ram_size = 0x40000000; /* 1024 MB. */
		break;
	case 4:
		gd->ram_size = 0x80000000; /* 2048 MB. */
		break;

	default:
	break;
	}

	return 0;
}

static int last_stage_init(void)
{

	char value[32];
	struct udevice *dev = gd->cur_serial_dev;

	if (gd->ram_size > 0) {
		sprintf(value, "%ldM", (gd->ram_size / 0x100000));
		env_set("mem", value);
	}

	if (dev && (dev->seq_ >= 0)) {
		void *addr;
		addr = dev_read_addr_ptr(dev);
		if (addr) {
			sprintf(value, "uart8250,mmio32,0x%x", (u32)addr);
			env_set("earlycon", value);
		}
		sprintf(value, "ttyS%d,115200n8", dev->seq_);
		env_set("console", value);
		return board_set_console();
	}

	return 0;
}
EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, last_stage_init);

