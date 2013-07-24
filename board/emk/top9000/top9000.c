/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>
#include <netdev.h>
#include <mmc.h>
#include <atmel_mci.h>
#include <i2c.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_shdwn.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NAND
static void nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Assign CS3 to NAND/SmartMedia Interface */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
		AT91_SMC_MODE_DBW_8 |
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void macb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable EMAC clock */
	writel(1 << ATMEL_ID_EMAC0, &pmc->pcer);

	/* Initialize EMAC=MACB hardware */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable MCI clock */
	writel(1 << ATMEL_ID_MCI, &pmc->pcer);

	/* Initialize MCI hardware */
	at91_mci_hw_init();

	/* This calls the atmel_mmc_init in gen_atmel_mci.c */
	return atmel_mci_init((void *)ATMEL_BASE_MCI);
}

/* this is a weak define that we are overriding */
int board_mmc_getcd(struct mmc *mmc)
{
	return !at91_get_gpio_value(CONFIG_SYS_MMC_CD_PIN);
}

#endif

int board_early_init_f(void)
{
	struct at91_shdwn *shdwn = (struct at91_shdwn *)ATMEL_BASE_SHDWN;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/*
	 * make sure the board can be powered on by
	 * any transition on WKUP
	 */
	writel(AT91_SHDW_MR_WKMODE0H2L | AT91_SHDW_MR_WKMODE0L2H,
		&shdwn->mr);

	/* Enable clocks for all PIOs */
	writel((1 << ATMEL_ID_PIOA) | (1 << ATMEL_ID_PIOB) |
		(1 << ATMEL_ID_PIOC),
		&pmc->pcer);

	/* set SCL0 and SDA0 to open drain */
	at91_set_pio_output(I2C0_PORT, SCL0_PIN, 1);
	at91_set_pio_multi_drive(I2C0_PORT, SCL0_PIN, 1);
	at91_set_pio_pullup(I2C0_PORT, SCL0_PIN, 1);
	at91_set_pio_output(I2C0_PORT, SDA0_PIN, 1);
	at91_set_pio_multi_drive(I2C0_PORT, SDA0_PIN, 1);
	at91_set_pio_pullup(I2C0_PORT, SDA0_PIN, 1);

	/* set SCL1 and SDA1 to open drain */
	at91_set_pio_output(I2C1_PORT, SCL1_PIN, 1);
	at91_set_pio_multi_drive(I2C1_PORT, SCL1_PIN, 1);
	at91_set_pio_pullup(I2C1_PORT, SCL1_PIN, 1);
	at91_set_pio_output(I2C1_PORT, SDA1_PIN, 1);
	at91_set_pio_multi_drive(I2C1_PORT, SDA1_PIN, 1);
	at91_set_pio_pullup(I2C1_PORT, SDA1_PIN, 1);
	return 0;
}

int board_init(void)
{
	/* arch number of TOP9000 Board */
	gd->bd->bi_arch_number = MACH_TYPE_TOP9000;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	at91_seriald_hw_init();
#ifdef CONFIG_CMD_NAND
	nand_hw_init();
#endif
#ifdef CONFIG_MACB
	macb_hw_init();
#endif
#ifdef CONFIG_ATMEL_SPI0
	/* (n+4) denotes to use nSPISEL(0) in GPIO mode! */
	at91_spi0_hw_init(1 << (FRAM_CS_NUM + 4));
#endif
#ifdef CONFIG_ATMEL_SPI1
	at91_spi1_hw_init(1 << (ENC_CS_NUM + 4));
#endif
	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	/* read 'factory' part of EEPROM */
	read_factory_r();
	return 0;
}
#endif

int dram_init(void)
{
	gd->ram_size = get_ram_size(
		(void *)CONFIG_SYS_SDRAM_BASE,
		CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
	/*
	 * Initialize ethernet HW addresses prior to starting Linux,
	 * needed for nfsroot.
	 * TODO: We need to investigate if that is really necessary.
	 */
	eth_init(gd->bd);
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
	int num = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0,
		(void *)ATMEL_BASE_EMAC0,
		CONFIG_SYS_PHY_ID);
	if (!rc)
		num++;
#endif
#ifdef CONFIG_ENC28J60
	rc = enc28j60_initialize(ENC_SPI_BUS, ENC_CS_NUM,
		ENC_SPI_CLOCK, SPI_MODE_0);
	if (!rc)
		num++;
# ifdef CONFIG_ENC28J60_2
	rc = enc28j60_initialize(ENC_SPI_BUS, ENC_CS_NUM+1,
		ENC_SPI_CLOCK, SPI_MODE_0);
	if (!rc)
		num++;
#  ifdef CONFIG_ENC28J60_3
	rc = enc28j60_initialize(ENC_SPI_BUS, ENC_CS_NUM+2,
		ENC_SPI_CLOCK, SPI_MODE_0);
	if (!rc)
		num++;
#  endif
# endif
#endif
	return num;
}

/*
 * I2C access functions
 *
 * Note:
 * We need to access Bus 0 before relocation to access the
 * environment settings.
 * However i2c_get_bus_num() cannot be called before
 * relocation.
 */
#ifdef CONFIG_SYS_I2C_SOFT
void iic_init(void)
{
	/* ports are now initialized in board_early_init_f() */
}

int iic_read(void)
{
	switch (I2C_ADAP_HWNR) {
	case 0:
		return at91_get_pio_value(I2C0_PORT, SDA0_PIN);
	case 1:
		return at91_get_pio_value(I2C1_PORT, SDA1_PIN);
	}
	return 1;
}

void iic_sda(int bit)
{
	switch (I2C_ADAP_HWNR) {
	case 0:
		at91_set_pio_value(I2C0_PORT, SDA0_PIN, bit);
		break;
	case 1:
		at91_set_pio_value(I2C1_PORT, SDA1_PIN, bit);
		break;
	}
}

void iic_scl(int bit)
{
	switch (I2C_ADAP_HWNR) {
	case 0:
		at91_set_pio_value(I2C0_PORT, SCL0_PIN, bit);
		break;
	case 1:
		at91_set_pio_value(I2C1_PORT, SCL1_PIN, bit);
		break;
	}
}

#endif
