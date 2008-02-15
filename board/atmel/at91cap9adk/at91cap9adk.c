/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop <at> leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
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
#include <asm/arch/AT91CAP9.h>

#define MP_BLOCK_3_BASE	0xFDF00000

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

static void at91cap9_serial_hw_init(void)
{
#ifdef CONFIG_USART0
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA22_TXD0 | AT91C_PA23_RXD0;
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_US0;
#endif

#ifdef CONFIG_USART1
	AT91C_BASE_PIOD->PIO_PDR = AT91C_PD0_TXD1 | AT91C_PD1_RXD1;
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_US1;
#endif

#ifdef CONFIG_USART2
	AT91C_BASE_PIOD->PIO_PDR = AT91C_PD2_TXD2 | AT91C_PD3_RXD2;
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_US2;
#endif

#ifdef CONFIG_USART3	/* DBGU */
	AT91C_BASE_PIOC->PIO_PDR = AT91C_PC31_DTXD | AT91C_PC30_DRXD;
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SYS;
#endif


}

static void at91cap9_nor_hw_init(void)
{
	/* Ensure EBI supply is 3.3V */
	AT91C_BASE_CCFG->CCFG_EBICSA |= AT91C_EBI_SUP_3V3;

	/* Configure SMC CS0 for parallel flash */
	AT91C_BASE_SMC->SMC_SETUP0 = AT91C_FLASH_NWE_SETUP |
				     AT91C_FLASH_NCS_WR_SETUP |
				     AT91C_FLASH_NRD_SETUP |
				     AT91C_FLASH_NCS_RD_SETUP;

	AT91C_BASE_SMC->SMC_PULSE0 = AT91C_FLASH_NWE_PULSE |
				     AT91C_FLASH_NCS_WR_PULSE |
				     AT91C_FLASH_NRD_PULSE |
				     AT91C_FLASH_NCS_RD_PULSE;

	AT91C_BASE_SMC->SMC_CYCLE0 = AT91C_FLASH_NWE_CYCLE |
				     AT91C_FLASH_NRD_CYCLE;

	AT91C_BASE_SMC->SMC_CTRL0 =  AT91C_SMC_READMODE |
				     AT91C_SMC_WRITEMODE |
				     AT91C_SMC_NWAITM_NWAIT_DISABLE |
				     AT91C_SMC_BAT_BYTE_WRITE |
				     AT91C_SMC_DBW_WIDTH_SIXTEEN_BITS |
				     (AT91C_SMC_TDF & (1 << 16));
}

#ifdef CONFIG_CMD_NAND
static void at91cap9_nand_hw_init(void)
{
	/* Enable CS3 */
	AT91C_BASE_CCFG->CCFG_EBICSA |= AT91C_EBI_CS3A_SM | AT91C_EBI_SUP_3V3;

	/* Configure SMC CS3 for NAND/SmartMedia */
	AT91C_BASE_SMC->SMC_SETUP3 = AT91C_SM_NWE_SETUP |
				     AT91C_SM_NCS_WR_SETUP |
				     AT91C_SM_NRD_SETUP |
				     AT91C_SM_NCS_RD_SETUP;

	AT91C_BASE_SMC->SMC_PULSE3 = AT91C_SM_NWE_PULSE |
				     AT91C_SM_NCS_WR_PULSE |
				     AT91C_SM_NRD_PULSE |
				     AT91C_SM_NCS_RD_PULSE;

	AT91C_BASE_SMC->SMC_CYCLE3 = AT91C_SM_NWE_CYCLE |
				     AT91C_SM_NRD_CYCLE;

	AT91C_BASE_SMC->SMC_CTRL3 =  AT91C_SMC_READMODE |
				     AT91C_SMC_WRITEMODE |
				     AT91C_SMC_NWAITM_NWAIT_DISABLE |
				     AT91C_SMC_DBW_WIDTH_EIGTH_BITS |
				     AT91C_SM_TDF;

	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOABCD;

	/* RDY/BSY is not connected */

	/* Enable NandFlash */
	AT91C_BASE_PIOD->PIO_PER = AT91C_PIO_PD15;
	AT91C_BASE_PIOD->PIO_OER = AT91C_PIO_PD15;
}
#endif

#ifdef CONFIG_HAS_DATAFLASH
static void at91cap9_spi_hw_init(void)
{
	AT91C_BASE_PIOD->PIO_BSR = AT91C_PD0_SPI0_NPCS2D |
				   AT91C_PD1_SPI0_NPCS3D;
	AT91C_BASE_PIOD->PIO_PDR = AT91C_PD0_SPI0_NPCS2D |
				   AT91C_PD1_SPI0_NPCS3D;

	AT91C_BASE_PIOA->PIO_ASR = AT91C_PA28_SPI0_NPCS3A;
	AT91C_BASE_PIOA->PIO_BSR = AT91C_PA4_SPI0_NPCS2A |
				   AT91C_PA1_SPI0_MOSI |
				   AT91C_PA0_SPI0_MISO |
				   AT91C_PA3_SPI0_NPCS1 |
				   AT91C_PA5_SPI0_NPCS0 |
				   AT91C_PA2_SPI0_SPCK;
	AT91C_BASE_PIOA->PIO_PDR = AT91C_PA28_SPI0_NPCS3A |
				   AT91C_PA4_SPI0_NPCS2A |
				   AT91C_PA1_SPI0_MOSI |
				   AT91C_PA0_SPI0_MISO |
				   AT91C_PA3_SPI0_NPCS1 |
				   AT91C_PA5_SPI0_NPCS0 |
				   AT91C_PA2_SPI0_SPCK;

	/* Enable Clock */
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI0;
}
#endif

#ifdef CONFIG_MACB
static void at91cap9_macb_hw_init(void)
{
	unsigned int gpio;

	/* Enable clock */
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_EMAC;

	/*
	 * Disable pull-up on:
	 *	RXDV (PB22) => PHY normal mode (not Test mode)
	 *	ERX0 (PB25) => PHY ADDR0
	 *	ERX1 (PB26) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	AT91C_BASE_PIOB->PIO_PPUDR = AT91C_PB22_E_RXDV |
				     AT91C_PB25_E_RX0 |
				     AT91C_PB26_E_RX1;

	/* Need to reset PHY -> 500ms reset */
	AT91C_BASE_RSTC->RSTC_RMR = (AT91C_RSTC_KEY & (0xA5 << 24)) |
				    (AT91C_RSTC_ERSTL & (0x0D << 8)) |
				    AT91C_RSTC_URSTEN;
	AT91C_BASE_RSTC->RSTC_RCR = (AT91C_RSTC_KEY & (0xA5 << 24)) |
				    AT91C_RSTC_EXTRST;

	/* Wait for end hardware reset */
	while (!(AT91C_BASE_RSTC->RSTC_RSR & AT91C_RSTC_NRSTL));

	/* Re-enable pull-up */
	AT91C_BASE_PIOB->PIO_PPUER = AT91C_PB22_E_RXDV |
				     AT91C_PB25_E_RX0 |
				     AT91C_PB26_E_RX1;

#ifdef CONFIG_RMII
	gpio =	AT91C_PB30_E_MDIO |
		AT91C_PB29_E_MDC  |
		AT91C_PB21_E_TXCK |
		AT91C_PB27_E_RXER |
		AT91C_PB25_E_RX0  |
		AT91C_PB22_E_RXDV |
		AT91C_PB26_E_RX1  |
		AT91C_PB28_E_TXEN |
		AT91C_PB23_E_TX0  |
		AT91C_PB24_E_TX1;
	AT91C_BASE_PIOB->PIO_ASR = gpio;
	AT91C_BASE_PIOB->PIO_BSR = 0;
	AT91C_BASE_PIOB->PIO_PDR = gpio;
#else
#error AT91CAP9A-DK works only in RMII mode
#endif

	/* Unlock EMAC, 3 0 2 1 sequence */
#define MP_MAC_KEY0	0x5969cb2a
#define MP_MAC_KEY1	0xb4a1872e
#define MP_MAC_KEY2	0x05683fbc
#define MP_MAC_KEY3	0x3634fba4
#define UNLOCK_MAC	0x00000008
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x3c)) = MP_MAC_KEY3;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x30)) = MP_MAC_KEY0;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x38)) = MP_MAC_KEY2;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x34)) = MP_MAC_KEY1;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x40)) = UNLOCK_MAC;
}
#endif

#ifdef CONFIG_USB_OHCI_NEW
static void at91cap9_uhp_hw_init(void)
{
	/* Unlock USB OHCI, 3 2 0 1 sequence */
#define MP_OHCI_KEY0	0x896c11ca
#define MP_OHCI_KEY1	0x68ebca21
#define MP_OHCI_KEY2	0x4823efbc
#define MP_OHCI_KEY3	0x8651aae4
#define UNLOCK_OHCI	0x00000010
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x3c)) = MP_OHCI_KEY3;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x38)) = MP_OHCI_KEY2;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x30)) = MP_OHCI_KEY0;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x34)) = MP_OHCI_KEY1;
	*((AT91_REG *)((AT91_REG) MP_BLOCK_3_BASE + 0x40)) = UNLOCK_OHCI;
}
#endif

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91CAP9ADK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91CAP9ADK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91cap9_serial_hw_init();
	at91cap9_nor_hw_init();
#ifdef CONFIG_CMD_NAND
	at91cap9_nand_hw_init();
#endif
#ifdef CONFIG_HAS_DATAFLASH
	at91cap9_spi_hw_init();
#endif
#ifdef CONFIG_MACB
	at91cap9_macb_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91cap9_uhp_hw_init();
#endif

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
#ifdef CONFIG_MACB
	/*
	 * Initialize ethernet HW addr prior to starting Linux,
	 * needed for nfsroot
	 */
	eth_init(gd->bd);
#endif
}
#endif
