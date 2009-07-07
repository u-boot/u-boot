/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2009
 * Daniel Gorsulowski <daniel.gorsulowski@esd.eu>
 * esd electronic system design gmbh <www.esd.eu>
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
#include <asm/arch/at91sam9263.h>
#include <asm/arch/at91sam9_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#include <asm/arch/io.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

static int hw_rev = -1;	/* hardware revision */

int get_hw_rev(void)
{
	if (hw_rev >= 0)
		return hw_rev;

	hw_rev = at91_get_gpio_value(AT91_PIN_PB19);
	hw_rev |= at91_get_gpio_value(AT91_PIN_PB20) << 1;
	hw_rev |= at91_get_gpio_value(AT91_PIN_PB21) << 2;
	hw_rev |= at91_get_gpio_value(AT91_PIN_PB22) << 3;

	if (hw_rev == 15)
		hw_rev = 0;

	return hw_rev;
}

#ifdef CONFIG_CMD_NAND
static void meesc_nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBI0CSA);
	at91_sys_write(AT91_MATRIX_EBI0CSA,
		csa | AT91_MATRIX_EBI0_CS3A_SMC_SMARTMEDIA);

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
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_DBW_8 |
#endif
		AT91_SMC_TDF_(2));

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif /* CONFIG_CMD_NAND */

#ifdef CONFIG_MACB
static void meesc_macb_hw_init(void)
{
	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9263_ID_EMAC);
	at91_macb_hw_init();
}
#endif

/*
 * Static memory controller initialization to enable Beckhoff ET1100 EtherCAT
 * controller debugging
 * The ET1100 is located at physical address 0x70000000
 * Its process memory is located at physical address 0x70001000
 */
static void meesc_ethercat_hw_init(void)
{
	/* Configure SMC EBI1_CS0 for EtherCAT */
	at91_sys_write(AT91_SMC1_SETUP(0),
		AT91_SMC_NWESETUP_(0) | AT91_SMC_NCS_WRSETUP_(0) |
		AT91_SMC_NRDSETUP_(0) | AT91_SMC_NCS_RDSETUP_(0));
	at91_sys_write(AT91_SMC1_PULSE(0),
		AT91_SMC_NWEPULSE_(4) | AT91_SMC_NCS_WRPULSE_(9) |
		AT91_SMC_NRDPULSE_(4) | AT91_SMC_NCS_RDPULSE_(9));
	at91_sys_write(AT91_SMC1_CYCLE(0),
		AT91_SMC_NWECYCLE_(10) | AT91_SMC_NRDCYCLE_(5));
	/* Configure behavior at external wait signal, byte-select mode, 16 bit
	data bus width, none data float wait states and TDF optimization */
	at91_sys_write(AT91_SMC1_MODE(0),
		AT91_SMC_READMODE | AT91_SMC_EXNWMODE_READY |
		AT91_SMC_BAT_SELECT | AT91_SMC_DBW_16 | AT91_SMC_TDF_(0) |
		AT91_SMC_TDFMODE);

	/* Configure RDY/BSY */
	at91_set_B_periph(AT91_PIN_PE20, 0);	/* EBI1_NWAIT */
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = get_ram_size((long *) PHYS_SDRAM, (1 << 27));
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)AT91SAM9263_BASE_EMAC, 0x00);
#endif
	return rc;
}

int checkboard(void)
{
	char str[32];

	puts("Board: esd CAN-EtherCAT Gateway");
	if (getenv_r("serial#", str, sizeof(str)) > 0) {
		puts(", serial# ");
		puts(str);
	}
	printf("\nHardware-revision: 1.%d\n", get_hw_rev());
	printf("Mach-type: %lu\n", gd->bd->bi_arch_number);
	return 0;
}

int board_init(void)
{
	/* Peripheral Clock Enable Register */
	at91_sys_write(AT91_PMC_PCER,	1 << AT91SAM9263_ID_PIOA |
					1 << AT91SAM9263_ID_PIOB |
					1 << AT91SAM9263_ID_PIOCDE);

	/* arch number of MEESC-Board */
	gd->bd->bi_arch_number = MACH_TYPE_MEESC;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91_serial_hw_init();
#ifdef CONFIG_CMD_NAND
	meesc_nand_hw_init();
#endif
	meesc_ethercat_hw_init();
#ifdef CONFIG_HAS_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_MACB
	meesc_macb_hw_init();
#endif
#ifdef CONFIG_AT91_CAN
	at91_can_hw_init();
#endif
	return 0;
}
