/*
 * (C) Copyright 2003-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <asm/processor.h>

/* Two MT48LC8M32B2 for 32 MB */
/* #include "mt48lc8m32b2-6-7.h" */

/* One MT48LC16M32S2 for 64 MB */
/* #include "mt48lc16m32s2-75.h" */
#if defined (CONFIG_MCC200_SDRAM)
#include "mt48lc16m16a2-75.h"
#else
#include "mt46v16m16-75.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[];	/* FLASH chips info */

extern int do_auto_update(void);
ulong flash_get_size (ulong base, int banknum);

#ifndef CONFIG_SYS_RAMBOOT
static void sdram_start (int hi_addr)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000000 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set mode register: extended mode */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_EMODE;
	__asm__ volatile ("sync");

	/* set mode register: reset DLL */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE | 0x04000000;
	__asm__ volatile ("sync");
#endif

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000002 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | 0x80000004 | hi_addr_bit;
	__asm__ volatile ("sync");

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = SDRAM_MODE;
	__asm__ volatile ("sync");

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = SDRAM_CONTROL | hi_addr_bit;
	__asm__ volatile ("sync");

	udelay(10);
}
#endif

/*
 * ATTENTION: Although partially referenced initdram does NOT make real use
 *	      use of CONFIG_SYS_SDRAM_BASE. The code does not work if CONFIG_SYS_SDRAM_BASE
 *	      is something else than 0x00000000.
 */

phys_size_t initdram (int board_type)
{
	ulong dramsize = 0;
	ulong dramsize2 = 0;
	uint svr, pvr;
#ifndef CONFIG_SYS_RAMBOOT
	ulong test1, test2;

	/* setup SDRAM chip selects */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001e;/* 2G at 0x0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = 0x80000000;/* disabled */
	__asm__ volatile ("sync");

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = SDRAM_CONFIG1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = SDRAM_CONFIG2;
	__asm__ volatile ("sync");

#if SDRAM_DDR
	/* set tap delay */
	*(vu_long *)MPC5XXX_CDM_PORCFG = SDRAM_TAPDELAY;
	__asm__ volatile ("sync");
#endif

	/* find RAM size using SDRAM CS0 only */
	sdram_start(0);
	test1 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	sdram_start(1);
	test2 = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0);
		dramsize = test1;
	} else {
		dramsize = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20)) {
		dramsize = 0;
	}

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 + __builtin_ffs(dramsize >> 20) - 1;
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
	}

	/* let SDRAM CS1 start right after CS0 */
	*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize + 0x0000001e;/* 2G */

	/* find RAM size using SDRAM CS1 only */
	if (!dramsize)
		sdram_start(0);
	test2 = test1 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x80000000);
	if (!dramsize) {
		sdram_start(1);
		test2 = get_ram_size((long *)(CONFIG_SYS_SDRAM_BASE + dramsize), 0x80000000);
	}
	if (test1 > test2) {
		sdram_start(0);
		dramsize2 = test1;
	} else {
		dramsize2 = test2;
	}

	/* memory smaller than 1MB is impossible */
	if (dramsize2 < (1 << 20)) {
		dramsize2 = 0;
	}

	/* set SDRAM CS1 size according to the amount of RAM found */
	if (dramsize2 > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize
			| (0x13 + __builtin_ffs(dramsize2 >> 20) - 1);
	} else {
		*(vu_long *)MPC5XXX_SDRAM_CS1CFG = dramsize; /* disabled */
	}

#else /* CONFIG_SYS_RAMBOOT */

	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13) {
		dramsize = (1 << (dramsize - 0x13)) << 20;
	} else {
		dramsize = 0;
	}

	/* retrieve size of memory connected to SDRAM CS1 */
	dramsize2 = *(vu_long *)MPC5XXX_SDRAM_CS1CFG & 0xFF;
	if (dramsize2 >= 0x13) {
		dramsize2 = (1 << (dramsize2 - 0x13)) << 20;
	} else {
		dramsize2 = 0;
	}

#endif /* CONFIG_SYS_RAMBOOT */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller. Please refer to Freescale's AN3221 "MPC5200B SDRAM
	 * Initialization and Configuration", 3.3.1 SDelay--MBAR + 0x0190:
	 *
	 * "The SDelay should be written to a value of 0x00000004. It is
	 * required to account for changes caused by normal wafer processing
	 * parameters."
	 */
	svr = get_svr();
	pvr = get_pvr();
	if ((SVR_MJREV(svr) >= 2) && (PVR_MAJ(pvr) == 1) && (PVR_MIN(pvr) == 4)) {
		*(vu_long *)MPC5XXX_SDRAM_SDELAY = 0x04;
		__asm__ volatile ("sync");
	}

	return dramsize + dramsize2;
}

int checkboard (void)
{
#if defined(CONFIG_PRS200)
	puts ("Board: PRS200\n");
#else
	puts ("Board: MCC200\n");
#endif
	return 0;
}

int misc_init_r (void)
{
	ulong flash_sup_end, snum;

	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if boot FLASH isn't max size
	 */
	if (gd->bd->bi_flashsize < (0 - CONFIG_SYS_FLASH_BASE)) {
		/* adjust mapping */
		*(vu_long *)MPC5XXX_BOOTCS_START = *(vu_long *)MPC5XXX_CS0_START =
			START_REG(gd->bd->bi_flashstart);
		*(vu_long *)MPC5XXX_BOOTCS_STOP = *(vu_long *)MPC5XXX_CS0_STOP =
			STOP_REG(gd->bd->bi_flashstart, gd->bd->bi_flashsize);

		/*
		 * Re-check to get correct base address
		 */
		flash_get_size(gd->bd->bi_flashstart, CONFIG_SYS_MAX_FLASH_BANKS - 1);

		/*
		 * Re-do flash protection upon new addresses
		 */
		flash_protect (FLAG_PROTECT_CLEAR,
			       gd->bd->bi_flashstart, 0xffffffff,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Monitor protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_SYS_MONITOR_BASE, CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_ENV_ADDR,
			       CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);

		/* Redundant environment protection ON by default */
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_ENV_ADDR_REDUND,
			       CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
			       &flash_info[CONFIG_SYS_MAX_FLASH_BANKS - 1]);
	}

	if (gd->bd->bi_flashsize > (32 << 20)) {
		/* Unprotect the upper bank of the Flash */
		*(volatile int*)MPC5XXX_CS0_CFG |= (1 << 6);
		flash_protect (FLAG_PROTECT_CLEAR,
			       flash_info[0].start[0] + flash_info[0].size / 2,
			       (flash_info[0].start[0] - 1) + flash_info[0].size,
			       &flash_info[0]);
		*(volatile int*)MPC5XXX_CS0_CFG &= ~(1 << 6);
		printf ("Warning: Only 32 of 64 MB of Flash are accessible from U-Boot\n");
		flash_info[0].size = 32 << 20;
		for (snum = 0, flash_sup_end = gd->bd->bi_flashstart + (32<<20);
			flash_info[0].start[snum] < flash_sup_end;
			snum++);
		flash_info[0].sector_count = snum;
	}

#ifdef CONFIG_AUTO_UPDATE
	do_auto_update();
#endif
	return (0);
}

#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void pci_mpc5xxx_init(struct pci_controller *);

void pci_init_board(void)
{
	pci_mpc5xxx_init(&hose);
}
#endif

#if defined(CONFIG_CMD_IDE) && defined(CONFIG_IDE_RESET)

void init_ide_reset (void)
{
	debug ("init_ide_reset\n");

}

void ide_set_reset (int idereset)
{
	debug ("ide_reset(%d)\n", idereset);

}
#endif

#if defined(CONFIG_CMD_DOC)
void doc_init (void)
{
	doc_probe (CONFIG_SYS_DOC_BASE);
}
#endif
