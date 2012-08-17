/*
 * Copyright (C) 2007 Freescale Semiconductor, Inc.
 *
 * Copyright (C) 2011 Matrix Vision GmbH
 * Andre Schwarz <andre.schwarz@matrix-vision.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>
#include <hwconfig.h>
#include <i2c.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/fsl_mpc83xx_serdes.h>
#include <fdt_support.h>
#include <spd_sdram.h>
#include "mergerbox.h"
#include "fpga.h"
#include "../common/mv_common.h"

static void setup_serdes(void)
{
	fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_SATA,
		FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
	fsl_setup_serdes(CONFIG_FSL_SERDES2, FSL_SERDES_PROTO_PEX,
		FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);
}

#if defined(CONFIG_SYS_DRAM_TEST)
int testdram(void)
{
	uint *pstart = (uint *) CONFIG_SYS_MEMTEST_START;
	uint *pend = (uint *) CONFIG_SYS_MEMTEST_END;
	uint *p;

	printf("Testing DRAM from 0x%08x to 0x%08x\n",
		CONFIG_SYS_MEMTEST_START, CONFIG_SYS_MEMTEST_END);

	printf("DRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf("DRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("DRAM test passed.\n");
	return 0;
}
#endif

phys_size_t initdram(int board_type)
{
	u32 msize;

	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile clk83xx_t *clk = (clk83xx_t *)&immr->clk;

	/* Enable PCI_CLK[0:1] */
	clk->occr |= 0xc0000000;
	udelay(2000);

#if defined(CONFIG_SPD_EEPROM)
	msize = spd_sdram();
#else
	immap_t *im = (immap_t *) CONFIG_SYS_IMMR;
	u32 msize_log2;

	msize = CONFIG_SYS_DDR_SIZE;
	msize_log2 = __ilog2(msize);

	im->sysconf.ddrlaw[0].bar = CONFIG_SYS_DDR_SDRAM_BASE & 0xfffff000;
	im->sysconf.ddrlaw[0].ar = LBLAWAR_EN | (msize_log2 - 1);

	im->sysconf.ddrcdr = CONFIG_SYS_DDRCDR_VALUE;
	udelay(50000);

	im->ddr.sdram_clk_cntl = CONFIG_SYS_DDR_SDRAM_CLK_CNTL;
	udelay(1000);

	im->ddr.csbnds[0].csbnds = CONFIG_SYS_DDR_CS0_BNDS;
	im->ddr.cs_config[0] = CONFIG_SYS_DDR_CS0_CONFIG;
	udelay(1000);

	im->ddr.timing_cfg_0 = CONFIG_SYS_DDR_TIMING_0;
	im->ddr.timing_cfg_1 = CONFIG_SYS_DDR_TIMING_1;
	im->ddr.timing_cfg_2 = CONFIG_SYS_DDR_TIMING_2;
	im->ddr.timing_cfg_3 = CONFIG_SYS_DDR_TIMING_3;
	im->ddr.sdram_cfg = CONFIG_SYS_DDR_SDRAM_CFG;
	im->ddr.sdram_cfg2 = CONFIG_SYS_DDR_SDRAM_CFG2;
	im->ddr.sdram_mode = CONFIG_SYS_DDR_MODE;
	im->ddr.sdram_mode2 = CONFIG_SYS_DDR_MODE2;
	im->ddr.sdram_interval = CONFIG_SYS_DDR_INTERVAL;
	__asm__ __volatile__("sync");
	udelay(1000);

	im->ddr.sdram_cfg |= SDRAM_CFG_MEM_EN;
	udelay(2000);
#endif
	setup_serdes();

	return msize << 20;
}

int checkboard(void)
{
	puts("Board: Matrix Vision MergerBox\n");

	return 0;
}

int misc_init_r(void)
{
	u16 dim;
	int result;
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	volatile gpio83xx_t *gpio = (gpio83xx_t *)&immr->gpio[1];
	unsigned char mac[6], mac_verify[6];
	char *s = getenv("reset_env");

	for (dim = 10; dim < 180; dim += 5) {
		mergerbox_tft_dim(dim);
		udelay(100000);
	}

	if (s)
		mv_reset_environment();

	i2c_read(SPD_EEPROM_ADDRESS, 0x80, 2, mac, sizeof(mac));

	/* check if Matrix Vision prefix present and export to env */
	if (mac[0] == 0x00 && mac[1] == 0x0c && mac[2] == 0x8d) {
		printf("valid MAC found in eeprom: %pM\n", mac);
		eth_setenv_enetaddr("ethaddr", mac);
	} else {
		printf("no valid MAC found in eeprom.\n");

		/* no: check the env */
		if (!eth_getenv_enetaddr("ethaddr", mac)) {
			printf("no valid MAC found in env either.\n");
			/* TODO: ask for valid MAC */
		} else {
			printf("valid MAC found in env: %pM\n", mac);
			printf("updating MAC in eeprom.\n");

			do {
				result = test_and_clear_bit(20, &gpio->dat);
				if (result)
					printf("unprotect EEPROM failed !\n");
				udelay(20000);
			} while(result);

			i2c_write(SPD_EEPROM_ADDRESS, 0x80, 2, mac, 6);
			udelay(20000);

			do {
				result = test_and_set_bit(20, &gpio->dat);
				if (result)
					printf("protect EEPROM failed !\n");
				udelay(20000);
			} while(result);

			printf("verify MAC %pM ... ", mac);
			i2c_read(SPD_EEPROM_ADDRESS, 0x80, 2, mac_verify, 6);

			if (!strncmp((char *)mac, (char *)mac_verify, 6))
				printf("ok.\n");
			else
				/* TODO: retry or do something useful */
				printf("FAILED (got %pM) !\n", mac_verify);
		}
	}

	return 0;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	volatile gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];

	iopd->dat &= ~TFT_SPI_CPLD_CS;
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	volatile gpio83xx_t *iopd = &((immap_t *)CONFIG_SYS_IMMR)->gpio[0];

	iopd->dat |= TFT_SPI_CPLD_CS;
}

/* control backlight pwm (display brightness).
 * allow values 0-250 with 0 = turn off and 250 = max brightness
 */
void mergerbox_tft_dim(u16 value)
{
	struct spi_slave *slave;
	u16 din;
	u16 dout = 0;

	if (value > 0 && value < 250)
		dout = 0x4000 | value;

	slave = spi_setup_slave(0, 0, 1000000, SPI_MODE_0 | SPI_CS_HIGH);
	spi_claim_bus(slave);
	spi_xfer(slave, 16, &dout, &din, SPI_XFER_BEGIN | SPI_XFER_END);
	spi_release_bus(slave);
	spi_free_slave(slave);
}

void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_dr_usb(blob, bd);
	ft_pci_setup(blob, bd);
}
