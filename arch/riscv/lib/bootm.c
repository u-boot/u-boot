// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/byteorder.h>
#include <asm/bootm.h>

DECLARE_GLOBAL_DATA_PTR;

__weak void board_quiesce_devices(void)
{
}

int arch_fixup_fdt(void *blob)
{
	return 0;
}

int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	bd_t	*bd = gd->bd;
	char	*s;
	int	machid = bd->bi_arch_number;
	void	(*theKernel)(int arch, uint params);

	/*
	 * allow the PREP bootm subcommand, it is required for bootm to work
	 */
	if (flag & BOOTM_STATE_OS_PREP)
		return 0;

	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	theKernel = (void (*)(int, uint))images->ep;

	s = env_get("machid");
	if (s) {
		machid = simple_strtoul(s, NULL, 16);
		printf("Using machid 0x%x from environment\n", machid);
	}

	bootstage_mark(BOOTSTAGE_ID_RUN_OS);

	debug("## Transferring control to Linux (at address %08lx) ...\n",
	       (ulong)theKernel);

	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len) {
#ifdef CONFIG_OF_LIBFDT
		debug("using: FDT\n");
		if (image_setup_linux(images)) {
			printf("FDT creation failed! hanging...");
			hang();
		}
#endif
	}

	/* we assume that the kernel is in place */
	printf("\nStarting kernel ...\n\n");

	cleanup_before_linux();
	if (IMAGE_ENABLE_OF_LIBFDT && images->ft_len)
		theKernel(machid, (unsigned long)images->ft_addr);

	/* does not return */

	return 1;
}
