/*
 * Palm Treo 680 Support
 *
 * Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 *
 */

#include <common.h>
#include <command.h>
#include <serial.h>
#include <nand.h>
#include <malloc.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch-pxa/pxa.h>
#include <asm/arch-pxa/regs-mmc.h>
#include <asm/io.h>
#include <asm/global_data.h>
#include <u-boot/crc.h>
#include <linux/mtd/docg4.h>

DECLARE_GLOBAL_DATA_PTR;

static struct nand_chip docg4_nand_chip;

int board_init(void)
{
	/* We have RAM, disable cache */
	dcache_disable();
	icache_disable();

	gd->bd->bi_arch_number = CONFIG_MACH_TYPE;
	gd->bd->bi_boot_params = CONFIG_SYS_DRAM_BASE + 0x100;

	return 0;
}

int dram_init(void)
{
	/* IPL initializes SDRAM (we're already running from it) */
	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

#ifdef CONFIG_LCD
void lcd_enable(void)
{
	/*
	 * Undo the L_BIAS / gpio77 pin configuration performed by the pxa lcd
	 * driver code.  We need it as an output gpio.
	 */
	writel((readl(GAFR2_L) & ~(0xc << 24)), GAFR2_L);

	/* power-up and enable the lcd */
	writel(0x00400000, GPSR(86)); /* enable; drive high */
	writel(0x00002000, GPSR(77)); /* power; drive high */
	writel(0x02000000, GPCR(25)); /* enable_n; drive low */

	/* turn on LCD backlight and configure PWM for reasonable brightness */
	writel(0x00, PWM_CTRL0);
	writel(0x1b1, PWM_PERVAL0);
	writel(0xfd, PWM_PWDUTY0);
	writel(0x00000040, GPSR(38)); /*  backlight power on */
}
#endif

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	writel(1 << 10, GPSR(42)); /* power on */
	return pxa_mmc_register(0);
}
#endif

void board_nand_init(void)
{
	/* we have one 128M diskonchip G4  */

	struct mtd_info *mtd = &nand_info[0];
	struct nand_chip *nand = &docg4_nand_chip;
	if (docg4_nand_init(mtd, nand, 0))
		hang();
}

#ifdef CONFIG_SPL_BUILD
void nand_boot(void)
{
	__attribute__((noreturn)) void (*uboot)(void);

	extern const void *_start, *_end;   /* boundaries of spl in memory */

	/* size of spl; ipl loads this, and then a portion of u-boot */
	const size_t spl_image_size = ((size_t)&_end - (size_t)&_start);

	/* the flash offset of the blocks that are loaded by the spl */
	const uint32_t spl_load_offset = CONFIG_SYS_NAND_U_BOOT_OFFS +
		DOCG4_IPL_LOAD_BLOCK_COUNT * DOCG4_BLOCK_SIZE;

	/* total number of bytes loaded by IPL */
	const size_t ipl_load_size =
		DOCG4_IPL_LOAD_BLOCK_COUNT * DOCG4_BLOCK_CAPACITY_SPL;

	/* number of bytes of u-boot proper that was loaded by the IPL */
	const size_t ipl_uboot_load_size = ipl_load_size - spl_image_size;

	/* number of remaining bytes of u-boot that the SPL must load */
	const size_t spl_load_size =
		CONFIG_SYS_NAND_U_BOOT_SIZE - ipl_load_size;

	/* memory address where we resume loading u-boot */
	void *const load_addr =
		(void *)(CONFIG_SYS_NAND_U_BOOT_DST + ipl_uboot_load_size);

	/*
	 * Copy the portion of u-boot already read from flash by the IPL to its
	 * correct load address.
	 */
	memcpy((void *)CONFIG_SYS_NAND_U_BOOT_DST, &_end, ipl_uboot_load_size);

	/*
	 * Resume loading u-boot where the IPL left off.
	 */
	nand_spl_load_image(spl_load_offset, spl_load_size, load_addr);

#ifdef CONFIG_NAND_ENV_DST
	nand_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			    (void *)CONFIG_NAND_ENV_DST);

#ifdef CONFIG_ENV_OFFSET_REDUND
	nand_spl_load_image(CONFIG_ENV_OFFSET_REDUND, CONFIG_ENV_SIZE,
			    (void *)CONFIG_NAND_ENV_DST + CONFIG_ENV_SIZE);
#endif
#endif
	/*
	 * Jump to U-Boot image
	 */
	uboot = (void *)CONFIG_SYS_NAND_U_BOOT_START;
	(*uboot)();
}

void board_init_f(ulong bootflag)
{
	nand_boot();
}

#endif  /* CONFIG_SPL_BUILD */
