/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2006
 * MicroSys GmbH
 *
 * (C) Copyright 2009
 * Wolfgang Grandegger, DENX Software Engineering, wg@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <netdev.h>
#include <miiphy.h>
#include <libfdt.h>
#include <mb862xx.h>
#include <video_fb.h>
#include <asm/processor.h>
#include <asm/io.h>

#ifdef CONFIG_OF_LIBFDT
#include <fdt_support.h>
#endif /* CONFIG_OF_LIBFDT */

/* mt46v16m16-75 */
#ifdef CONFIG_MPC5200_DDR
/* Settings for XLB = 132 MHz */
#define SDRAM_MODE	0x018D0000
#define SDRAM_EMODE	0x40090000
#define SDRAM_CONTROL	0x714f0f00
#define SDRAM_CONFIG1	0x73722930
#define SDRAM_CONFIG2	0x47770000
#define SDRAM_TAPDELAY	0x10000000
#else
#error SDRAM is not supported on this board
#endif

DECLARE_GLOBAL_DATA_PTR;

static void sdram_start (int hi_addr)
{
	struct mpc5xxx_sdram *sdram = (struct mpc5xxx_sdram *)MPC5XXX_SDRAM;
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	out_be32 (&sdram->ctrl, SDRAM_CONTROL | 0x80000000 | hi_addr_bit);

	/* precharge all banks */
	out_be32 (&sdram->ctrl, SDRAM_CONTROL | 0x80000002 | hi_addr_bit);

	/* set mode register: extended mode */
	out_be32 (&sdram->mode, SDRAM_EMODE);

	/* set mode register: reset DLL */
	out_be32 (&sdram->mode, SDRAM_MODE | 0x04000000);

	/* precharge all banks */
	out_be32 (&sdram->ctrl, SDRAM_CONTROL | 0x80000002 | hi_addr_bit);

	/* auto refresh */
	out_be32 (&sdram->ctrl, SDRAM_CONTROL | 0x80000004 | hi_addr_bit);

	/* set mode register */
	out_be32 (&sdram->mode, SDRAM_MODE);

	/* normal operation */
	out_be32 (&sdram->ctrl, SDRAM_CONTROL | hi_addr_bit);
}

/*
 * ATTENTION: Although partially referenced dram_init does NOT make real
 *	      use of CONFIG_SYS_SDRAM_BASE. The code does not work if
 *	      CONFIG_SYS_SDRAM_BASE is something else than 0x00000000.
 */

int dram_init(void)
{
	struct mpc5xxx_mmap_ctl *mmap_ctl =
		(struct mpc5xxx_mmap_ctl *)CONFIG_SYS_MBAR;
	struct mpc5xxx_sdram *sdram = (struct mpc5xxx_sdram *)MPC5XXX_SDRAM;
	struct mpc5xxx_cdm *cdm = (struct mpc5xxx_cdm *)MPC5XXX_CDM;
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	ulong test1, test2;

	/* setup SDRAM chip selects */
	out_be32 (&mmap_ctl->sdram0, 0x0000001e);	/* 2G at 0x0 */
	out_be32 (&mmap_ctl->sdram1, 0x00000000);	/* disabled */

	/* setup config registers */
	out_be32 (&sdram->config1, SDRAM_CONFIG1);
	out_be32 (&sdram->config2, SDRAM_CONFIG2);

	/* set tap delay */
	out_be32 (&cdm->porcfg, SDRAM_TAPDELAY);

	/* find RAM size using SDRAM CS0 only */
	sdram_start (0);
	test1 = get_ram_size ((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	sdram_start (1);
	test2 = get_ram_size ((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start (0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0)
		out_be32 (&mmap_ctl->sdram0,
			  0x13 + __builtin_ffs (dramsize >> 20) - 1);
	else
		out_be32 (&mmap_ctl->sdram1, 0);	/* disabled */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller. Please refer to Freescale's AN3221 "MPC5200B SDRAM
	 * Initialization and Configuration", 3.3.1 SDelay--MBAR + 0x0190:
	 *
	 * "The SDelay should be written to a value of 0x00000004. It is
	 * required to account for changes caused by normal wafer processing
	 * parameters."
	 */
	out_be32 (&sdram->sdelay, 0x04);

	gd->ram_size = dramsize + dramsize2;

	return 0;
}

int checkboard (void)
{
	puts ("Board: IPEK01 \n");
	return 0;
}

void flash_preinit (void)
{
	struct mpc5xxx_lpb *lpb = (struct mpc5xxx_lpb *)MPC5XXX_LPB;

	/*
	 * Now, when we are in RAM, enable flash write
	 * access for detection process.
	 * Note that CS_BOOT cannot be cleared when
	 * executing in flash.
	 */
	clrbits_be32 (&lpb->cs0_cfg, 0x1);	/* clear RO */
}

void flash_afterinit (ulong start, ulong size)
{
	struct mpc5xxx_mmap_ctl *mmap_ctl =
		(struct mpc5xxx_mmap_ctl *)CONFIG_SYS_MBAR;

#if defined(CONFIG_BOOT_ROM)
	/* adjust mapping */
	out_be32 (&mmap_ctl->cs1_start, START_REG (start));
	out_be32 (&mmap_ctl->cs1_stop, STOP_REG (start, size));
#else
	/* adjust mapping */
	out_be32 (&mmap_ctl->boot_start, START_REG (start));
	out_be32 (&mmap_ctl->cs0_start, START_REG (start));
	out_be32 (&mmap_ctl->boot_stop, STOP_REG (start, size));
	out_be32 (&mmap_ctl->cs0_stop, STOP_REG (start, size));
#endif
}

extern flash_info_t flash_info[];	/* info for FLASH chips */

int misc_init_r (void)
{
	/* adjust flash start */
	gd->bd->bi_flashstart = flash_info[0].start[0];
	return (0);
}

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init (struct pci_controller *);

void pci_init_board (void)
{
	pci_mpc5xxx_init (&hose);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup (blob, bd);
	fdt_fixup_memory (blob, (u64) bd->bi_memstart, (u64) bd->bi_memsize);

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in FEC comes first */
	return pci_eth_init(bis);
}

#ifdef CONFIG_VIDEO
extern GraphicDevice mb862xx;

static const gdc_regs init_regs[] = {
	{0x0100, 0x00000900},
	{0x0020, 0x80190257},
	{0x0024, 0x00000000},
	{0x0028, 0x00000000},
	{0x002c, 0x00000000},
	{0x0110, 0x00000000},
	{0x0114, 0x00000000},
	{0x0118, 0x02570320},
	{0x0004, 0x041f0000},
	{0x0008, 0x031f031f},
	{0x000c, 0x067f0347},
	{0x0010, 0x02780000},
	{0x0014, 0x0257025c},
	{0x0018, 0x00000000},
	{0x001c, 0x02570320},
	{0x0100, 0x80010900},
	{0x0, 0x0}
};

const gdc_regs *board_get_regs (void)
{
	return init_regs;
}

/* Returns Lime base address */
unsigned int board_video_init (void)
{
	if (mb862xx_probe (CONFIG_SYS_LIME_BASE) != MB862XX_TYPE_LIME)
		return 0;

	mb862xx.winSizeX = 800;
	mb862xx.winSizeY = 600;
	mb862xx.gdfIndex = GDF_15BIT_555RGB;
	mb862xx.gdfBytesPP = 2;

	return CONFIG_SYS_LIME_BASE;
}

#if defined(CONFIG_CONSOLE_EXTRA_INFO)
/*
 * Return text to be printed besides the logo.
 */
void video_get_info_str (int line_number, char *info)
{
	if (line_number == 1)
		strcpy (info, " Board: IPEK01");
	else
		info[0] = '\0';
}
#endif
#endif /* CONFIG_VIDEO */
