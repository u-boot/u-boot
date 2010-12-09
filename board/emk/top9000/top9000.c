/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2010
 * Reinhard Meyer, EMK Elektronik, reinhard.meyer@emk-elektronik.de
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <net.h>
#include <netdev.h>
#include <mmc.h>
#include <i2c.h>
#include <spi.h>
#include <asm/arch/at91sam9260.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_shdwn.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NAND
static void nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	at91_sys_write(AT91_MATRIX_EBICSA,
		csa | AT91_MATRIX_CS3A_SMC_SMARTMEDIA);

	/* Configure SMC CS3 for NAND/SmartMedia */
	at91_sys_write(AT91_SMC_SETUP(3),
		AT91_SMC_NWESETUP_(1) | AT91_SMC_NCS_WRSETUP_(0) |
		AT91_SMC_NRDSETUP_(1) | AT91_SMC_NCS_RDSETUP_(0));
	at91_sys_write(AT91_SMC_PULSE(3),
		AT91_SMC_NWEPULSE_(3) | AT91_SMC_NCS_WRPULSE_(3) |
		AT91_SMC_NRDPULSE_(3) | AT91_SMC_NCS_RDPULSE_(3));
	at91_sys_write(AT91_SMC_CYCLE(3),
		AT91_SMC_NWECYCLE_(5) | AT91_SMC_NRDCYCLE_(5));
	at91_sys_write(AT91_SMC_MODE(3),
		AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		AT91_SMC_EXNWMODE_DISABLE |
		AT91_SMC_DBW_8 |
		AT91_SMC_TDF_(2));

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void macb_hw_init(void)
{
	/* Enable EMAC clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_EMAC);

	/* Initialize EMAC=MACB hardware */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	/* Enable MCI clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_MCI);

	/* Initialize MCI hardware */
	at91_mci_hw_init();

	/* This calls the atmel_mmc_init in gen_atmel_mci.c */
	return atmel_mci_init((void *)AT91_BASE_MCI);
}

/* this is a weak define that we are overriding */
int board_mmc_getcd(u8 *cd, struct mmc *mmc)
{
	/*
	 * the only currently existing use of this function
	 * (fsl_esdhc.c) suggests this function must return
	 * *cs = TRUE if a card is NOT detected -> in most
	 * cases the value of the pin when the detect switch
	 * closes to GND
	 */
	*cd = at91_get_gpio_value(CONFIG_SYS_MMC_CD_PIN) ? 1 : 0;
	return 0;
}

#endif

int board_early_init_f(void)
{
	struct at91_shdwn *shdwn = (struct at91_shdwn *)AT91_SHDWN_BASE;

	/*
	 * make sure the board can be powered on by
	 * any transition on WKUP
	 */
	writel(AT91_SHDW_MR_WKMODE0H2L | AT91_SHDW_MR_WKMODE0L2H,
		&shdwn->mr);

	/* Enable clocks for all PIOs */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_PIOA);
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_PIOB);
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9260_ID_PIOC);

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

	at91_serial_hw_init();
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
		(void *)AT91_EMAC_BASE,
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
#ifdef CONFIG_SOFT_I2C
void iic_init(void)
{
	/* ports are now initialized in board_early_init_f() */
}

int iic_read(void)
{
	switch ((gd->flags & GD_FLG_RELOC) ? i2c_get_bus_num() : 0) {
	case 0:
		return at91_get_pio_value(I2C0_PORT, SDA0_PIN);
	case 1:
		return at91_get_pio_value(I2C1_PORT, SDA1_PIN);
	}
	return 1;
}

void iic_sda(int bit)
{
	switch ((gd->flags & GD_FLG_RELOC) ? i2c_get_bus_num() : 0) {
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
	switch ((gd->flags & GD_FLG_RELOC) ? i2c_get_bus_num() : 0) {
	case 0:
		at91_set_pio_value(I2C0_PORT, SCL0_PIN, bit);
		break;
	case 1:
		at91_set_pio_value(I2C1_PORT, SCL1_PIN, bit);
		break;
	}
}

#endif
