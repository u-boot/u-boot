/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
#include <pci.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale M54455 EVB\n");
	return 0;
};

phys_size_t initdram(int board_type)
{
	u32 dramsize;
#ifdef CONFIG_CF_SBF
	/*
	 * Serial Boot: The dram is already initialized in start.S
	 * only require to return DRAM size
	 */
	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000 >> 1;
#else
	volatile sdramc_t *sdram = (volatile sdramc_t *)(MMAP_SDRAM);
	volatile gpio_t *gpio = (volatile gpio_t *)(MMAP_GPIO);
	u32 i;

	dramsize = CONFIG_SYS_SDRAM_SIZE * 0x100000 >> 1;

	for (i = 0x13; i < 0x20; i++) {
		if (dramsize == (1 << i))
			break;
	}
	i--;

	gpio->mscr_sdram = CONFIG_SYS_SDRAM_DRV_STRENGTH;

	sdram->sdcs0 = (CONFIG_SYS_SDRAM_BASE | i);
	sdram->sdcs1 = (CONFIG_SYS_SDRAM_BASE1 | i);

	sdram->sdcfg1 = CONFIG_SYS_SDRAM_CFG1;
	sdram->sdcfg2 = CONFIG_SYS_SDRAM_CFG2;

	/* Issue PALL */
	sdram->sdcr = CONFIG_SYS_SDRAM_CTRL | 2;

	/* Issue LEMR */
	sdram->sdmr = CONFIG_SYS_SDRAM_EMOD | 0x408;
	sdram->sdmr = CONFIG_SYS_SDRAM_MODE | 0x300;

	udelay(500);

	/* Issue PALL */
	sdram->sdcr = CONFIG_SYS_SDRAM_CTRL | 2;

	/* Perform two refresh cycles */
	sdram->sdcr = CONFIG_SYS_SDRAM_CTRL | 4;
	sdram->sdcr = CONFIG_SYS_SDRAM_CTRL | 4;

	sdram->sdmr = CONFIG_SYS_SDRAM_MODE | 0x200;

	sdram->sdcr = (CONFIG_SYS_SDRAM_CTRL & ~0x80000000) | 0x10000c00;

	udelay(100);
#endif
	return (dramsize << 1);
};

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

#if defined(CONFIG_CMD_IDE)
#include <ata.h>

int ide_preinit(void)
{
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	gpio->par_fec |= (gpio->par_fec & GPIO_PAR_FEC_FEC1_MASK) | 0x10;
	gpio->par_feci2c |=
	    (gpio->par_feci2c & 0xF0FF) | (GPIO_PAR_FECI2C_MDC1_ATA_DIOR |
					   GPIO_PAR_FECI2C_MDIO1_ATA_DIOW);
	gpio->par_ata |=
	    (GPIO_PAR_ATA_BUFEN | GPIO_PAR_ATA_CS1 | GPIO_PAR_ATA_CS0 |
	     GPIO_PAR_ATA_DA2 | GPIO_PAR_ATA_DA1 | GPIO_PAR_ATA_DA0
	     | GPIO_PAR_ATA_RESET_RESET | GPIO_PAR_ATA_DMARQ_DMARQ |
	     GPIO_PAR_ATA_IORDY_IORDY);
	gpio->par_pci |=
	    (GPIO_PAR_PCI_GNT3_ATA_DMACK | GPIO_PAR_PCI_REQ3_ATA_INTRQ);

	return (0);
}

void ide_set_reset(int idereset)
{
	volatile atac_t *ata = (atac_t *) MMAP_ATA;
	long period;
	/*  t1,  t2,  t3,  t4,  t5,  t6,  t9, tRD,  tA */
	int piotms[5][9] = {
		{70, 165, 60, 30, 50, 5, 20, 0, 35},	/* PIO 0 */
		{50, 125, 45, 20, 35, 5, 15, 0, 35},	/* PIO 1 */
		{30, 100, 30, 15, 20, 5, 10, 0, 35},	/* PIO 2 */
		{30, 80, 30, 10, 20, 5, 10, 0, 35},	/* PIO 3 */
		{25, 70, 20, 10, 20, 5, 10, 0, 35}
	};			/* PIO 4 */

	if (idereset) {
		ata->cr = 0;	/* control reset */
		udelay(10000);
	} else {
#define CALC_TIMING(t) (t + period - 1) / period
		period = 1000000000 / gd->bus_clk;	/* period in ns */

		/*ata->ton = CALC_TIMING (180); */
		ata->t1 = CALC_TIMING(piotms[2][0]);
		ata->t2w = CALC_TIMING(piotms[2][1]);
		ata->t2r = CALC_TIMING(piotms[2][1]);
		ata->ta = CALC_TIMING(piotms[2][8]);
		ata->trd = CALC_TIMING(piotms[2][7]);
		ata->t4 = CALC_TIMING(piotms[2][3]);
		ata->t9 = CALC_TIMING(piotms[2][6]);

		ata->cr = 0x40;	/* IORDY enable */
		udelay(200000);
		ata->cr |= 0x01;	/* IORDY enable */
	}
}
#endif

#if defined(CONFIG_PCI)
/*
 * Initialize PCI devices, report devices found.
 */
static struct pci_controller hose;
extern void pci_mcf5445x_init(struct pci_controller *hose);

void pci_init_board(void)
{
	pci_mcf5445x_init(&hose);
}
#endif				/* CONFIG_PCI */

#if defined(CONFIG_FLASH_CFI_LEGACY)
#include <flash.h>
ulong board_flash_get_legacy (ulong base, int banknum, flash_info_t * info)
{
	int sect[] = CONFIG_SYS_ATMEL_SECT;
	int sectsz[] = CONFIG_SYS_ATMEL_SECTSZ;
	int i, j, k;

	if (base != CONFIG_SYS_ATMEL_BASE)
		return 0;

	info->flash_id          = 0x01000000;
	info->portwidth         = 1;
	info->chipwidth         = 1;
	info->buffer_size       = 32;
	info->erase_blk_tout    = 16384;
	info->write_tout        = 2;
	info->buffer_write_tout = 5;
	info->vendor            = 0xFFF0; /* CFI_CMDSET_AMD_LEGACY */
	info->cmd_reset         = 0x00F0;
	info->interface         = FLASH_CFI_X8;
	info->legacy_unlock     = 0;
	info->manufacturer_id   = (u16) ATM_MANUFACT;
	info->device_id         = ATM_ID_LV040;
	info->device_id2        = 0;

	info->ext_addr          = 0;
	info->cfi_version       = 0x3133;
	info->cfi_offset        = 0x0000;
	info->addr_unlock1      = 0x00000555;
	info->addr_unlock2      = 0x000002AA;
	info->name              = "CFI conformant";

	info->size              = 0;
	info->sector_count      = CONFIG_SYS_ATMEL_TOTALSECT;
	info->start[0] = base;
	for (k = 0, i = 0; i < CONFIG_SYS_ATMEL_REGION; i++) {
		info->size += sect[i] * sectsz[i];

		for (j = 0; j < sect[i]; j++, k++) {
			info->start[k + 1] = info->start[k] + sectsz[i];
			info->protect[k] = 0;
		}
	}

	return 1;
}
#endif				/* CONFIG_SYS_FLASH_CFI */
