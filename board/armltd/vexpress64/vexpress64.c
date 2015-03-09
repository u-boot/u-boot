/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 * Sharma Bhupesh <bhupesh.sharma@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <asm/semihosting.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_SEMIHOSTING
	/*
	 * Please refer to doc/README.semihosting for a more complete
	 * description.
	 *
	 * We require that the board include file defines these env variables:
	 * - kernel_name
	 * - kernel_addr_r
	 * - initrd_name
	 * - initrd_addr_r
	 * - fdt_name
	 * - fdt_addr_r
	 *
	 * For the "fdt chosen" startup macro, this code will then define:
	 * - initrd_end (based on initrd_addr_r plus actual initrd_size)
	 *
	 * We will then load the kernel, initrd, and fdt into the specified
	 * locations in memory in a similar way that the ATF fastmodel code
	 * uses semihosting calls to load other boot stages and u-boot itself.
	 */

	/* Env variable strings */
	char *kernel_name = getenv("kernel_name");
	char *kernel_addr_str = getenv("kernel_addr_r");
	char *initrd_name = getenv("initrd_name");
	char *initrd_addr_str = getenv("initrd_addr_r");
	char *fdt_name = getenv("fdt_name");
	char *fdt_addr_str = getenv("fdt_addr_r");
	char initrd_end_str[64];

	/* Actual addresses converted from env variables */
	void *kernel_addr_r;
	void *initrd_addr_r;
	void *fdt_addr_r;

	/* Actual initrd base and size */
	unsigned long initrd_base;
	unsigned long initrd_size;

	/* Space available */
	int avail;

	/* Make sure the environment variables needed are set */
	if (!(kernel_addr_str && initrd_addr_str && fdt_addr_str)) {
		printf("%s: Define {kernel/initrd/fdt}_addr_r\n", __func__);
		return -1;
	}
	if (!(kernel_name && initrd_name && fdt_name)) {
		printf("%s: Define {kernel/initrd/fdt}_name\n", __func__);
		return -1;
	}

	/* Get exact initrd_size */
	initrd_size = smh_len(initrd_name);
	if (initrd_size == -1) {
		printf("%s: Can't get file size for \'%s\'\n", __func__,
		       initrd_name);
		return -1;
	}

	/* Set initrd_end */
	initrd_base = simple_strtoul(initrd_addr_str, NULL, 16);
	initrd_addr_r = (void *)initrd_base;
	sprintf(initrd_end_str, "0x%lx", initrd_base + initrd_size - 1);
	setenv("initrd_end", initrd_end_str);

	/* Load kernel to memory */
	fdt_addr_r = (void *)simple_strtoul(fdt_addr_str, NULL, 16);
	kernel_addr_r = (void *)simple_strtoul(kernel_addr_str, NULL, 16);

	/*
	 * The kernel must be lower in memory than fdt and loading the
	 * kernel must not trample the fdt or vice versa.
	 */
	avail = fdt_addr_r - kernel_addr_r;
	if (avail < 0) {
		printf("%s: fdt must be after kernel\n", __func__);
		return -1;
	}
	smh_load(kernel_name, kernel_addr_r, avail, 1);

	/* Load fdt to memory */
	smh_load(fdt_name, fdt_addr_r, 0x20000, 1);

	/* Load initrd to memory */
	smh_load(initrd_name, initrd_addr_r, initrd_size, 1);

#endif				/* CONFIG_SEMIHOSTING */
	return 0;
}
#endif				/* CONFIG_BOARD_LATE_INIT */

/*
 * Board specific ethernet initialization routine.
 */
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC91111
	rc = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}
