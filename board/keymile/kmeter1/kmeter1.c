/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * (C) Copyright 2008
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <ioports.h>
#include <mpc83xx.h>
#include <i2c.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <pci.h>
#include <libfdt.h>

#include "../common/common.h"

extern void disable_addr_trans (void);
extern void enable_addr_trans (void);
const qe_iop_conf_t qe_iop_conf_tab[] = {
	/* port pin dir open_drain assign */

	/* MDIO */
	{0,  1, 3, 0, 2}, /* MDIO */
	{0,  2, 1, 0, 1}, /* MDC */

	/* UCC4 - UEC */
	{1, 14, 1, 0, 1}, /* TxD0 */
	{1, 15, 1, 0, 1}, /* TxD1 */
	{1, 20, 2, 0, 1}, /* RxD0 */
	{1, 21, 2, 0, 1}, /* RxD1 */
	{1, 18, 1, 0, 1}, /* TX_EN */
	{1, 26, 2, 0, 1}, /* RX_DV */
	{1, 27, 2, 0, 1}, /* RX_ER */
	{1, 24, 2, 0, 1}, /* COL */
	{1, 25, 2, 0, 1}, /* CRS */
	{2, 15, 2, 0, 1}, /* TX_CLK - CLK16 */
	{2, 16, 2, 0, 1}, /* RX_CLK - CLK17 */

	/* DUART - UART2 */
	{5,  0, 1, 0, 2}, /* UART2_SOUT */
	{5,  2, 1, 0, 1}, /* UART2_RTS */
	{5,  3, 2, 0, 2}, /* UART2_SIN */
	{5,  1, 2, 0, 3}, /* UART2_CTS */

	/* END of table */
	{0,  0, 0, 0, QE_IOP_TAB_END},
};

static int board_init_i2c_busses (void)
{
	I2C_MUX_DEVICE *dev = NULL;
	uchar	*buf;

	/* Set up the Bus for the DTTs */
	buf = (unsigned char *) getenv ("dtt_bus");
	if (buf != NULL)
		dev = i2c_mux_ident_muxstring (buf);
	if (dev == NULL) {
		printf ("Error couldn't add Bus for DTT\n");
		printf ("please setup dtt_bus to where your\n");
		printf ("DTT is found.\n");
	}
	return 0;
}

int board_early_init_r (void)
{
	unsigned short	svid;

	/*
	 * Because of errata in the UCCs, we have to write to the reserved
	 * registers to slow the clocks down.
	 */
	svid =  SVR_REV(mfspr (SVR));
	switch (svid) {
	case 0x0020:
		setbits_be32((void *)(CONFIG_SYS_IMMR + 0x14a8), 0x0c003000);
		break;
	case 0x0021:
		clrsetbits_be32((void *)(CONFIG_SYS_IMMR + 0x14ac),
			0x00000050, 0x000000a0);
		break;
	}
	/* enable the PHY on the PIGGY */
	setbits (8, (void *)(CONFIG_SYS_PIGGY_BASE + 0x10003), 0x01);

	return 0;
}

int misc_init_r (void)
{
	/* add board specific i2c busses */
	board_init_i2c_busses ();
	return 0;
}

int fixed_sdram(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;
	u32 ddr_size;
	u32 ddr_size_log2;

	im->sysconf.ddrlaw[0].ar = LAWAR_EN | 0x1e;
	im->ddr.csbnds[0].csbnds = CONFIG_SYS_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CS0_CONFIG;
	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	im->ddr.sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CONFIG_SYS_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE2;
	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_CLK_CNTL;
	udelay (200);
	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;

	msize = CONFIG_SYS_DDR_SIZE << 20;
	disable_addr_trans ();
	msize = get_ram_size (CONFIG_SYS_DDR_BASE, msize);
	enable_addr_trans ();
	msize /= (1024 * 1024);
	if (CONFIG_SYS_DDR_SIZE != msize) {
		for (ddr_size = msize << 20, ddr_size_log2 = 0;
		     (ddr_size > 1); ddr_size = ddr_size >> 1, ddr_size_log2++)
			if (ddr_size & 1)
				return -1;
		im->sysconf.ddrlaw[0].ar =
		    LAWAR_EN | ((ddr_size_log2 - 1) & LAWAR_SIZE);
		im->ddr.csbnds[0].csbnds = (((msize / 16) - 1) & 0xff);
	}

	return msize;
}

phys_size_t initdram (int board_type)
{
#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	extern void ddr_enable_ecc (unsigned int dram_size);
#endif
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	u32 msize = 0;

	if ((im->sysconf.immrbar & IMMRBAR_BASE_ADDR) != (u32)im)
		return -1;

	/* DDR SDRAM - Main SODIMM */
	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_BASE & LAWBAR_BAR;
	msize = fixed_sdram ();

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
	/*
	 * Initialize DDR ECC byte
	 */
	ddr_enable_ecc (msize * 1024 * 1024);
#endif

	/* return total bus SDRAM size(bytes)  -- DDR */
	return (msize * 1024 * 1024);
}

int checkboard (void)
{
	puts ("Board: Keymile kmeter1");
	if (ethernet_present ())
		puts (" with PIGGY.");
	puts ("\n");
	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
/*
 * update "/localbus/ranges" property in the blob
 */
void ft_blob_update (void *blob, bd_t *bd)
{
	ulong	*flash_data = NULL;
	flash_info_t	*info;
	ulong	flash_reg[6] = {0};
	int	len;
	int	size = 0;
	int	i = 0;

	len = fdt_get_node_and_value (blob, "/localbus", "ranges",
					(void *)&flash_data);

	if (flash_data == NULL) {
		printf ("%s: error /localbus/ranges entry\n", __FUNCTION__);
		return;
	}

	/* update Flash addr, size */
	while ( i < (len / 4)) {
		switch (flash_data[i]) {
		case 0:
			info = flash_get_info(CONFIG_SYS_FLASH_BASE);
			size = info->size;
			info = flash_get_info(CONFIG_SYS_FLASH_BASE_1);
			size += info->size;
			flash_data[i + 1] = 0;
			flash_data[i + 2] = cpu_to_be32 (CONFIG_SYS_FLASH_BASE);
			flash_data[i + 3] = cpu_to_be32 (size);
			break;
		default:
			break;
		}
		i += 4;
	}
	fdt_set_node_and_value (blob, "/localbus", "ranges", flash_data,
				len);

	info = flash_get_info(CONFIG_SYS_FLASH_BASE);
	size = info->size;
	flash_reg[2] = cpu_to_be32 (size);
	flash_reg[4] = flash_reg[2];
	info = flash_get_info(CONFIG_SYS_FLASH_BASE_1);
	flash_reg[5] = cpu_to_be32 (info->size);
	fdt_set_node_and_value (blob, "/localbus/flash@f0000000,0", "reg", flash_reg,
				sizeof (flash_reg));
}


void ft_board_setup (void *blob, bd_t *bd)
{
	ft_cpu_setup (blob, bd);
	ft_blob_update (blob, bd);
}
#endif

#if defined(CONFIG_HUSH_INIT_VAR)
extern int ivm_read_eeprom (void);
int hush_init_var (void)
{
	ivm_read_eeprom ();
	return 0;
}
#endif
