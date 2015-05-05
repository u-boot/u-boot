/*
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <spl.h>
#include <asm/io.h>
#include <nand.h>

#if defined(CONFIG_SPL_NAND_RAW_ONLY)
void spl_nand_load_image(void)
{
	nand_init();

	nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
			    CONFIG_SYS_NAND_U_BOOT_SIZE,
			    (void *)CONFIG_SYS_NAND_U_BOOT_DST);
	spl_set_header_raw_uboot();
	nand_deselect();
}
#else
void spl_nand_load_image(void)
{
	struct image_header *header;
	int *src __attribute__((unused));
	int *dst __attribute__((unused));

	debug("spl: nand - using hw ecc\n");
	nand_init();

	/*use CONFIG_SYS_TEXT_BASE as temporary storage area */
	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE);
#ifdef CONFIG_SPL_OS_BOOT
	if (!spl_start_uboot()) {
		/*
		 * load parameter image
		 * load to temp position since nand_spl_load_image reads
		 * a whole block which is typically larger than
		 * CONFIG_CMD_SPL_WRITE_SIZE therefore may overwrite
		 * following sections like BSS
		 */
		nand_spl_load_image(CONFIG_CMD_SPL_NAND_OFS,
			CONFIG_CMD_SPL_WRITE_SIZE,
			(void *)CONFIG_SYS_TEXT_BASE);
		/* copy to destintion */
		for (dst = (int *)CONFIG_SYS_SPL_ARGS_ADDR,
				src = (int *)CONFIG_SYS_TEXT_BASE;
				src < (int *)(CONFIG_SYS_TEXT_BASE +
				CONFIG_CMD_SPL_WRITE_SIZE);
				src++, dst++) {
			writel(readl(src), dst);
		}

		/* load linux */
		nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
			sizeof(*header), (void *)header);
		spl_parse_image_header(header);
		if (header->ih_os == IH_OS_LINUX) {
			/* happy - was a linux */
			nand_spl_load_image(CONFIG_SYS_NAND_SPL_KERNEL_OFFS,
				spl_image.size, (void *)spl_image.load_addr);
			nand_deselect();
			return;
		} else {
			puts("The Expected Linux image was not "
				"found. Please check your NAND "
				"configuration.\n");
			puts("Trying to start u-boot now...\n");
		}
	}
#endif
#ifdef CONFIG_NAND_ENV_DST
	nand_spl_load_image(CONFIG_ENV_OFFSET,
		sizeof(*header), (void *)header);
	spl_parse_image_header(header);
	nand_spl_load_image(CONFIG_ENV_OFFSET, spl_image.size,
		(void *)spl_image.load_addr);
#ifdef CONFIG_ENV_OFFSET_REDUND
	nand_spl_load_image(CONFIG_ENV_OFFSET_REDUND,
		sizeof(*header), (void *)header);
	spl_parse_image_header(header);
	nand_spl_load_image(CONFIG_ENV_OFFSET_REDUND, spl_image.size,
		(void *)spl_image.load_addr);
#endif
#endif
	/* Load u-boot */
	nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
		sizeof(*header), (void *)header);
	spl_parse_image_header(header);
	nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
		spl_image.size, (void *)(unsigned long)spl_image.load_addr);
	nand_deselect();
}
#endif
