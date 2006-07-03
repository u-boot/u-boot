/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/processor.h>
#include <spd_sdram.h>
#include <ppc440.h>
#include "bamboo.h"

void ext_bus_cntlr_init(void);
void configure_ppc440ep_pins(void);
int is_nand_selected(void);

unsigned char cfg_simulate_spd_eeprom[128];

gpio_param_s gpio_tab[GPIO_GROUP_MAX][GPIO_MAX];
#if 0
{	   /* GPIO   Alternate1	      Alternate2	Alternate3 */
    {
	/* GPIO Core 0 */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_0	-> EBC_ADDR(7)	    DMA_REQ(2) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_1	-> EBC_ADDR(6)	    DMA_ACK(2) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_2	-> EBC_ADDR(5)	    DMA_EOT/TC(2) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_3	-> EBC_ADDR(4)	    DMA_REQ(3) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_4	-> EBC_ADDR(3)	    DMA_ACK(3) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_5 ................. */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_6	-> EBC_CS_N(1) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_7	-> EBC_CS_N(2) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_8	-> EBC_CS_N(3) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_ALT1 }, /* GPIO0_9	 -> EBC_CS_N(4) */
	{ GPIO0_BASE, GPIO_OUT, GPIO_ALT1 }, /* GPIO0_10 -> EBC_CS_N(5) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_11 -> EBC_BUS_ERR */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_12 -> ZII_p0Rxd(0) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_13 -> ZII_p0Rxd(1) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_14 -> ZII_p0Rxd(2) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_15 -> ZII_p0Rxd(3) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_16 -> ZII_p0Txd(0) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_17 -> ZII_p0Txd(1) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_18 -> ZII_p0Txd(2) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_19 -> ZII_p0Txd(3) */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_20 -> ZII_p0Rx_er */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_21 -> ZII_p0Rx_dv */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_22 -> ZII_p0RxCrs */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_23 -> ZII_p0Tx_er */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_24 -> ZII_p0Tx_en */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_25 -> ZII_p0Col */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_26 ->		    USB2D_RXVALID */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_27 -> EXT_EBC_REQ	    USB2D_RXERROR */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_28 ->		    USB2D_TXVALID */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_29 -> EBC_EXT_HDLA	    USB2D_PAD_SUSPNDM */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_30 -> EBC_EXT_ACK	    USB2D_XCVRSELECT */
	{ GPIO0_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO0_31 -> EBC_EXR_BUSREQ   USB2D_TERMSELECT */
    },
    {
	/* GPIO Core 1 */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_0	-> USB2D_OPMODE0 */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_1	-> USB2D_OPMODE1 */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_2	-> UART0_DCD_N	    UART1_DSR_CTS_N   UART2_SOUT */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_3	-> UART0_8PIN_DSR_N UART1_RTS_DTR_N   UART2_SIN */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_4	-> UART0_8PIN_CTS_N		      UART3_SIN */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_5	-> UART0_RTS_N */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_6	-> UART0_DTR_N	    UART1_SOUT */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_7	-> UART0_RI_N	    UART1_SIN */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_8	-> UIC_IRQ(0) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_9	-> UIC_IRQ(1) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_10 -> UIC_IRQ(2) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_11 -> UIC_IRQ(3) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_12 -> UIC_IRQ(4)	    DMA_ACK(1) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_13 -> UIC_IRQ(6)	    DMA_EOT/TC(1) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_14 -> UIC_IRQ(7)	    DMA_REQ(0) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_15 -> UIC_IRQ(8)	    DMA_ACK(0) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_16 -> UIC_IRQ(9)	    DMA_EOT/TC(0) */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_17 -> - */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_18 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_19 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_20 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_21 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_22 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_23 ->   \	   Can be unselected thru TraceSelect Bit */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_24 ->   /	      in PowerPC440EP Chip */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_25 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_26 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_27 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_28 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_29 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_30 ->  | */
	{ GPIO1_BASE, GPIO_DIS, GPIO_SEL }, /* GPIO1_31 -> - */
    }
};
#endif

/*----------------------------------------------------------------------------+
  | EBC Devices Characteristics
  |   Peripheral Bank Access Parameters	      -	  EBC0_BnAP
  |   Peripheral Bank Configuration Register  -	  EBC0_BnCR
  +----------------------------------------------------------------------------*/
/* Small Flash */
#define EBC0_BNAP_SMALL_FLASH				\
	EBC0_BNAP_BME_DISABLED			|	\
	EBC0_BNAP_TWT_ENCODE(6)			|	\
	EBC0_BNAP_CSN_ENCODE(0)	    		|	\
	EBC0_BNAP_OEN_ENCODE(1)	    		|	\
	EBC0_BNAP_WBN_ENCODE(1)	    		|	\
	EBC0_BNAP_WBF_ENCODE(3)	    		|	\
	EBC0_BNAP_TH_ENCODE(1)	    		|	\
	EBC0_BNAP_RE_ENABLED	    		|	\
	EBC0_BNAP_SOR_DELAYED	    		|	\
	EBC0_BNAP_BEM_WRITEONLY	    		|	\
	EBC0_BNAP_PEN_DISABLED

#define EBC0_BNCR_SMALL_FLASH_CS0			\
	EBC0_BNCR_BAS_ENCODE(0xFFF00000)    	| 	\
	EBC0_BNCR_BS_1MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_8BIT

#define EBC0_BNCR_SMALL_FLASH_CS4			\
	EBC0_BNCR_BAS_ENCODE(0x87F00000)    	| 	\
	EBC0_BNCR_BS_1MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_8BIT

/* Large Flash or SRAM */
#define EBC0_BNAP_LARGE_FLASH_OR_SRAM			\
	EBC0_BNAP_BME_DISABLED	    		|	\
	EBC0_BNAP_TWT_ENCODE(8)	    		|	\
	EBC0_BNAP_CSN_ENCODE(0)	    		|	\
	EBC0_BNAP_OEN_ENCODE(1)	    		|	\
	EBC0_BNAP_WBN_ENCODE(1)	    		|	\
	EBC0_BNAP_WBF_ENCODE(1)	    		|	\
	EBC0_BNAP_TH_ENCODE(2)	    		|	\
	EBC0_BNAP_SOR_DELAYED	    		|	\
	EBC0_BNAP_BEM_RW	    		|	\
	EBC0_BNAP_PEN_DISABLED

#define EBC0_BNCR_LARGE_FLASH_OR_SRAM_CS0   		\
	EBC0_BNCR_BAS_ENCODE(0xFF800000)	| 	\
	EBC0_BNCR_BS_8MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_16BIT


#define EBC0_BNCR_LARGE_FLASH_OR_SRAM_CS4   		\
	EBC0_BNCR_BAS_ENCODE(0x87800000)	| 	\
	EBC0_BNCR_BS_8MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_16BIT

/* NVRAM - FPGA */
#define EBC0_BNAP_NVRAM_FPGA				\
	EBC0_BNAP_BME_DISABLED	    		|	\
	EBC0_BNAP_TWT_ENCODE(9)	    		|	\
	EBC0_BNAP_CSN_ENCODE(0)	    		|	\
	EBC0_BNAP_OEN_ENCODE(1)	    		|	\
	EBC0_BNAP_WBN_ENCODE(1)	    		|	\
	EBC0_BNAP_WBF_ENCODE(0)	    		|	\
	EBC0_BNAP_TH_ENCODE(2)	    		|	\
	EBC0_BNAP_RE_ENABLED	    		|	\
	EBC0_BNAP_SOR_DELAYED	    		|	\
	EBC0_BNAP_BEM_WRITEONLY	    		|	\
	EBC0_BNAP_PEN_DISABLED

#define EBC0_BNCR_NVRAM_FPGA_CS5			\
	EBC0_BNCR_BAS_ENCODE(0x80000000)    	| 	\
	EBC0_BNCR_BS_1MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_8BIT

/* Nand Flash */
#define EBC0_BNAP_NAND_FLASH				\
	EBC0_BNAP_BME_DISABLED	    		|	\
	EBC0_BNAP_TWT_ENCODE(3)	    		|	\
	EBC0_BNAP_CSN_ENCODE(0)	    		|	\
	EBC0_BNAP_OEN_ENCODE(0)	    		|	\
	EBC0_BNAP_WBN_ENCODE(0)	    		|	\
	EBC0_BNAP_WBF_ENCODE(0)	    		|	\
	EBC0_BNAP_TH_ENCODE(1)	    		|	\
	EBC0_BNAP_RE_ENABLED	    		|	\
	EBC0_BNAP_SOR_NOT_DELAYED   		|	\
	EBC0_BNAP_BEM_RW	    		|	\
	EBC0_BNAP_PEN_DISABLED


#define EBC0_BNCR_NAND_FLASH_CS0	0xB8400000

/* NAND0 */
#define EBC0_BNCR_NAND_FLASH_CS1			\
	EBC0_BNCR_BAS_ENCODE(0x90000000)    	| 	\
	EBC0_BNCR_BS_1MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_32BIT
/* NAND1 - Bank2 */
#define EBC0_BNCR_NAND_FLASH_CS2			\
	EBC0_BNCR_BAS_ENCODE(0x94000000)    	| 	\
	EBC0_BNCR_BS_1MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_32BIT

/* NAND1 - Bank3 */
#define EBC0_BNCR_NAND_FLASH_CS3			\
	EBC0_BNCR_BAS_ENCODE(0x94000000)    	| 	\
	EBC0_BNCR_BS_1MB		    	|	\
	EBC0_BNCR_BU_RW			    	|	\
	EBC0_BNCR_BW_32BIT

int board_early_init_f(void)
{
	ext_bus_cntlr_init();

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(uic0sr, 0xffffffff);	/* clear all */
	mtdcr(uic0er, 0x00000000);	/* disable all */
	mtdcr(uic0cr, 0x00000009);	/* ATI & UIC1 crit are critical */
	mtdcr(uic0pr, 0xfffffe13);	/* per ref-board manual */
	mtdcr(uic0tr, 0x01c00008);	/* per ref-board manual */
	mtdcr(uic0vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic0sr, 0xffffffff);	/* clear all */

	mtdcr(uic1sr, 0xffffffff);	/* clear all */
	mtdcr(uic1er, 0x00000000);	/* disable all */
	mtdcr(uic1cr, 0x00000000);	/* all non-critical */
	mtdcr(uic1pr, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(uic1tr, 0x00ffc000);	/* per ref-board manual */
	mtdcr(uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(uic1sr, 0xffffffff);	/* clear all */

	/*--------------------------------------------------------------------
	 * Setup the GPIO pins
	 *-------------------------------------------------------------------*/
	out32(GPIO0_OSRL,  0x00000400);
	out32(GPIO0_OSRH,  0x00000000);
	out32(GPIO0_TSRL,  0x00000400);
	out32(GPIO0_TSRH,  0x00000000);
	out32(GPIO0_ISR1L, 0x00000000);
	out32(GPIO0_ISR1H, 0x00000000);
	out32(GPIO0_ISR2L, 0x00000000);
	out32(GPIO0_ISR2H, 0x00000000);
	out32(GPIO0_ISR3L, 0x00000000);
	out32(GPIO0_ISR3H, 0x00000000);

	out32(GPIO1_OSRL,  0x0C380000);
	out32(GPIO1_OSRH,  0x00000000);
	out32(GPIO1_TSRL,  0x0C380000);
	out32(GPIO1_TSRH,  0x00000000);
	out32(GPIO1_ISR1L, 0x0FC30000);
	out32(GPIO1_ISR1H, 0x00000000);
	out32(GPIO1_ISR2L, 0x0C010000);
	out32(GPIO1_ISR2H, 0x00000000);
	out32(GPIO1_ISR3L, 0x01400000);
	out32(GPIO1_ISR3H, 0x00000000);

	configure_ppc440ep_pins();

	return 0;
}

#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <linux/mtd/nand_legacy.h>
extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];

/*----------------------------------------------------------------------------+
  | nand_reset.
  |   Reset Nand flash
  |   This routine will abort previous cmd
  +----------------------------------------------------------------------------*/
int nand_reset(ulong addr)
{
	int wait=0, stat=0;

	out8(addr + NAND_CMD_REG, NAND0_CMD_RESET);
	out8(addr + NAND_CMD_REG, NAND0_CMD_READ_STATUS);

	while ((stat != 0xc0) && (wait != 0xffff)) {
		stat = in8(addr + NAND_DATA_REG);
		wait++;
	}

	if (stat == 0xc0) {
		return 0;
	} else {
		printf("NAND Reset timeout.\n");
		return -1;
	}
}

void board_nand_set_device(int cs, ulong addr)
{
	/* Set NandFlash Core Configuration Register */
	out32(addr + NAND_CCR_REG, 0x00001000 | (cs << 24));

	switch (cs) {
	case 1:
		/* -------
		 *  NAND0
		 * -------
		 * K9F1208U0A : 4 addr cyc, 1 col + 3 Row
		 * Set NDF1CR - Enable External CS1 in NAND FLASH controller
		 */
		out32(addr + NAND_CR1_REG, 0x80002222);
		break;

	case 2:
		/* -------
		 *  NAND1
		 * -------
		 * K9K2G0B : 5 addr cyc, 2 col + 3 Row
		 * Set NDF2CR : Enable External CS2 in NAND FLASH controller
		 */
		out32(addr + NAND_CR2_REG, 0xC0007777);
		break;
	}

	/* Perform Reset Command */
	if (nand_reset(addr) != 0)
		return;
}

void nand_init(void)
{
	board_nand_set_device(1, CFG_NAND_ADDR);

	nand_probe(CFG_NAND_ADDR);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}

#if 0 /* NAND1 not supported yet */
	board_nand_set_device(2, CFG_NAND2_ADDR);

	nand_probe(CFG_NAND2_ADDR);
	if (nand_dev_desc[0].ChipID != NAND_ChipID_UNKNOWN) {
		print_size(nand_dev_desc[0].totlen, "\n");
	}
#endif
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */

int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: Bamboo - AMCC PPC440EP Evaluation Board");
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

/*************************************************************************
 *
 * init_spd_array -- Bamboo has one bank onboard sdram (plus DIMM)
 *
 * Fixed memory is composed of :
 *	MT46V16M16TG-75 from Micron (x 2), 256Mb, 16 M x16, DDR266,
 *	13 row add bits, 10 column add bits (but 12 row used only).
 *	ECC device: MT46V16M8TG-75 from Micron (x 1), 128Mb, x8, DDR266,
 *	12 row add bits, 10 column add bits.
 *	Prepare a subset (only the used ones) of SPD data
 *
 *	Note : if the ECC is enabled (SDRAM_ECC_ENABLE) the size of
 *	the corresponding bank is divided by 2 due to number of Row addresses
 *	12 in the ECC module
 *
 *  Assumes:	64 MB, ECC, non-registered
 *		PLB @ 133 MHz
 *
 ************************************************************************/
static void init_spd_array(void)
{
	cfg_simulate_spd_eeprom[8]     = 0x04;    /* 2.5 Volt */
	cfg_simulate_spd_eeprom[2]     = 0x07;    /* DDR ram */

#ifdef CONFIG_DDR_ECC
	cfg_simulate_spd_eeprom[11]    = 0x02;    /* ECC ON : 02 OFF : 00 */
	cfg_simulate_spd_eeprom[31]    = 0x08;    /* bankSizeID: 32MB */
	cfg_simulate_spd_eeprom[3]     = 0x0C;    /* num Row Addr: 12 */
#else
	cfg_simulate_spd_eeprom[11]    = 0x00;    /* ECC ON : 02 OFF : 00 */
	cfg_simulate_spd_eeprom[31]    = 0x10;    /* bankSizeID: 64MB */
	cfg_simulate_spd_eeprom[3]     = 0x0D;    /* num Row Addr: 13 */
#endif

	cfg_simulate_spd_eeprom[4]     = 0x09;    /* numColAddr: 9  */
	cfg_simulate_spd_eeprom[5]     = 0x01;    /* numBanks: 1 */
	cfg_simulate_spd_eeprom[0]     = 0x80;    /* number of SPD bytes used: 128 */
	cfg_simulate_spd_eeprom[1]     = 0x08;    /*  total number bytes in SPD device = 256 */
	cfg_simulate_spd_eeprom[21]    = 0x00;    /* not registered: 0  registered : 0x02*/
	cfg_simulate_spd_eeprom[6]     = 0x20;    /* Module data width: 32 bits */
	cfg_simulate_spd_eeprom[7]     = 0x00;    /* Module data width continued: +0 */
	cfg_simulate_spd_eeprom[15]    = 0x01;    /* wcsbc = 1 */
	cfg_simulate_spd_eeprom[27]    = 0x50;    /* tRpNs = 20 ns  */
	cfg_simulate_spd_eeprom[29]    = 0x50;    /* tRcdNs = 20 ns */

	cfg_simulate_spd_eeprom[30]    = 45;      /* tRasNs */

	cfg_simulate_spd_eeprom[18]    = 0x0C;    /* casBit (2,2.5) */

	cfg_simulate_spd_eeprom[9]     = 0x75;    /* SDRAM Cycle Time (cas latency 2.5) = 7.5 ns */
	cfg_simulate_spd_eeprom[23]    = 0xA0;    /* SDRAM Cycle Time (cas latency 2) = 10 ns */
	cfg_simulate_spd_eeprom[25]    = 0x00;    /* SDRAM Cycle Time (cas latency 1.5) = N.A */
	cfg_simulate_spd_eeprom[12]    = 0x82;    /* refresh Rate Type: Normal (15.625us) + Self refresh */
}

long int initdram (int board_type)
{
	long dram_size = 0;

	/*
	 * First write simulated values in eeprom array for onboard bank 0
	 */
	init_spd_array();

	dram_size = spd_sdram();

	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int testdram(void)
{
	unsigned long *mem = (unsigned long *)0;
	const unsigned long kend = (1024 / sizeof(unsigned long));
	unsigned long k, n;

	mtmsr(0);

	for (k = 0; k < CFG_KBYTES_SDRAM;
	     ++k, mem += (1024 / sizeof(unsigned long))) {
		if ((k & 1023) == 0) {
			printf("%3d MB\r", k / 1024);
		}

		memset(mem, 0xaaaaaaaa, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0xaaaaaaaa) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}

		memset(mem, 0x55555555, 1024);
		for (n = 0; n < kend; ++n) {
			if (mem[n] != 0x55555555) {
				printf("SDRAM test fails at: %08x\n",
				       (uint) & mem[n]);
				return 1;
			}
		}
	}
	printf("SDRAM test passes\n");
	return 0;
}
#endif

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB3 devices to 0.
	  | Set PLB3 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	mfsdr(sdr_amp1, addr);
	mtsdr(sdr_amp1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb3_acr);
	mtdcr(plb3_acr, addr | 0x80000000);

	/*-------------------------------------------------------------------------+
	  | Set priority for all PLB4 devices to 0.
	  +-------------------------------------------------------------------------*/
	mfsdr(sdr_amp0, addr);
	mtsdr(sdr_amp0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(plb4_acr) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(plb4_acr, addr);

	/*-------------------------------------------------------------------------+
	  | Set Nebula PLB4 arbiter to fair mode.
	  +-------------------------------------------------------------------------*/
	/* Segment0 */
	addr = (mfdcr(plb0_acr) & ~plb0_acr_ppm_mask) | plb0_acr_ppm_fair;
	addr = (addr & ~plb0_acr_hbu_mask) | plb0_acr_hbu_enabled;
	addr = (addr & ~plb0_acr_rdp_mask) | plb0_acr_rdp_4deep;
	addr = (addr & ~plb0_acr_wrp_mask) | plb0_acr_wrp_2deep;
	mtdcr(plb0_acr, addr);

	/* Segment1 */
	addr = (mfdcr(plb1_acr) & ~plb1_acr_ppm_mask) | plb1_acr_ppm_fair;
	addr = (addr & ~plb1_acr_hbu_mask) | plb1_acr_hbu_enabled;
	addr = (addr & ~plb1_acr_rdp_mask) | plb1_acr_rdp_4deep;
	addr = (addr & ~plb1_acr_wrp_mask) | plb1_acr_wrp_2deep;
	mtdcr(plb1_acr, addr);

	return 1;
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_PRE_INIT) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Set up Direct MMIO registers
	 *--------------------------------------------------------------------------*/
	/*--------------------------------------------------------------------------+
	  | PowerPC440 EP PCI Master configuration.
	  | Map one 1Gig range of PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xDFFFFFFF ==> PCI address 0xA0000000-0xDFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +--------------------------------------------------------------------------*/
	out32r(PCIX0_PMM0MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM0LA, CFG_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIX0_PMM0PCILA, CFG_PCI_MEMBASE);	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_PMM0MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIX0_PMM1MA, 0x00000000);	/* PMM0 Mask/Attribute - disabled b4 setting */
	out32r(PCIX0_PMM1LA, CFG_PCI_MEMBASE2); /* PMM0 Local Address */
	out32r(PCIX0_PMM1PCILA, CFG_PCI_MEMBASE2);	/* PMM0 PCI Low Address */
	out32r(PCIX0_PMM1PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_PMM1MA, 0xE0000001);	/* 512M + No prefetching, and enable region */

	out32r(PCIX0_PTM1MS, 0x00000001);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM1LA, 0);	/* Local Addr. Reg */
	out32r(PCIX0_PTM2MS, 0);	/* Memory Size/Attribute */
	out32r(PCIX0_PTM2LA, 0);	/* Local Addr. Reg */

	/*--------------------------------------------------------------------------+
	 * Set up Configuration registers
	 *--------------------------------------------------------------------------*/

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CFG_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CFG_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

/*************************************************************************
 *  pci_master_init
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*--------------------------------------------------------------------------+
	  | Write the PowerPC440 EP PCI Configuration regs.
	  |   Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	  |   Enable PowerPC440 EP to act as a PCI memory target (PTM).
	  +--------------------------------------------------------------------------*/
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT) */

/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	/* Bamboo is always configured as host. */
	return (1);
}
#endif				/* defined(CONFIG_PCI) */

/*----------------------------------------------------------------------------+
  | is_powerpc440ep_pass1.
  +----------------------------------------------------------------------------*/
int is_powerpc440ep_pass1(void)
{
	unsigned long pvr;

	pvr = get_pvr();

	if (pvr == PVR_POWERPC_440EP_PASS1)
		return TRUE;
	else if (pvr == PVR_POWERPC_440EP_PASS2)
		return FALSE;
	else {
		printf("brdutil error 3\n");
		for (;;)
			;
	}

	return(FALSE);
}

/*----------------------------------------------------------------------------+
  | is_nand_selected.
  +----------------------------------------------------------------------------*/
int is_nand_selected(void)
{
#ifdef CONFIG_BAMBOO_NAND
	return TRUE;
#else
	return FALSE;
#endif
}

/*----------------------------------------------------------------------------+
  | config_on_ebc_cs4_is_small_flash => from EPLD
  +----------------------------------------------------------------------------*/
unsigned char config_on_ebc_cs4_is_small_flash(void)
{
	/* Not implemented yet => returns constant value */
	return TRUE;
}

/*----------------------------------------------------------------------------+
  | Ext_bus_cntlr_init.
  | Initialize the external bus controller
  +----------------------------------------------------------------------------*/
void ext_bus_cntlr_init(void)
{
	unsigned long sdr0_pstrp0, sdr0_sdstp1;
	unsigned long bootstrap_settings, boot_selection, ebc_boot_size;
	int	      computed_boot_device = BOOT_DEVICE_UNKNOWN;
	unsigned long ebc0_cs0_bnap_value = 0, ebc0_cs0_bncr_value = 0;
	unsigned long ebc0_cs1_bnap_value = 0, ebc0_cs1_bncr_value = 0;
	unsigned long ebc0_cs2_bnap_value = 0, ebc0_cs2_bncr_value = 0;
	unsigned long ebc0_cs3_bnap_value = 0, ebc0_cs3_bncr_value = 0;
	unsigned long ebc0_cs4_bnap_value = 0, ebc0_cs4_bncr_value = 0;


	/*-------------------------------------------------------------------------+
	  |
	  |  PART 1 : Initialize EBC Bank 5
	  |  ==============================
	  | Bank5 is always associated to the NVRAM/EPLD.
	  | It has to be initialized prior to other banks settings computation since
	  | some board registers values may be needed
	  |
	  +-------------------------------------------------------------------------*/
	/* NVRAM - FPGA */
	mtebc(pb5ap, EBC0_BNAP_NVRAM_FPGA);
	mtebc(pb5cr, EBC0_BNCR_NVRAM_FPGA_CS5);

	/*-------------------------------------------------------------------------+
	  |
	  |  PART 2 : Determine which boot device was selected
	  |  =========================================
	  |
	  |  Read Pin Strap Register in PPC440EP
	  |  In case of boot from IIC, read Serial Device Strap Register1
	  |
	  |  Result can either be :
	  |   - Boot from EBC 8bits    => SMALL FLASH
	  |   - Boot from EBC 16bits   => Large Flash or SRAM
	  |   - Boot from NAND Flash
	  |   - Boot from PCI
	  |
	  +-------------------------------------------------------------------------*/
	/* Read Pin Strap Register in PPC440EP */
	mfsdr(sdr_pstrp0, sdr0_pstrp0);
	bootstrap_settings = sdr0_pstrp0 & SDR0_PSTRP0_BOOTSTRAP_MASK;

	/*-------------------------------------------------------------------------+
	  |  PPC440EP Pass1
	  +-------------------------------------------------------------------------*/
	if (is_powerpc440ep_pass1() == TRUE) {
		switch(bootstrap_settings) {
		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS0:
			/* Default Strap Settings 0 : CPU 400 - PLB 133 - Boot EBC 8 bit 33MHz */
			/* Boot from Small Flash */
			computed_boot_device = BOOT_FROM_SMALL_FLASH;
			break;
		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS1:
			/* Default Strap Settings 1 : CPU 533 - PLB 133 - Boot PCI 66MHz */
			/* Boot from PCI */
			computed_boot_device = BOOT_FROM_PCI;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS2:
			/* Default Strap Settings 2 : CPU 500 - PLB 100 - Boot NDFC16 66MHz */
			/* Boot from Nand Flash */
			computed_boot_device = BOOT_FROM_NAND_FLASH0;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS3:
			/* Default Strap Settings 3 : CPU 333 - PLB 133 - Boot EBC 8 bit 66MHz */
			/* Boot from Small Flash */
			computed_boot_device = BOOT_FROM_SMALL_FLASH;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_IIC_A8_EN:
		case SDR0_PSTRP0_BOOTSTRAP_IIC_A4_EN:
			/* Boot Settings in IIC EEprom address 0xA8 or 0xA4 */
			/* Read Serial Device Strap Register1 in PPC440EP */
			mfsdr(sdr_sdstp1, sdr0_sdstp1);
			boot_selection	= sdr0_sdstp1 & SDR0_SDSTP1_BOOT_SEL_MASK;
			ebc_boot_size	= sdr0_sdstp1 & SDR0_SDSTP1_EBC_ROM_BS_MASK;

			switch(boot_selection) {
			case SDR0_SDSTP1_BOOT_SEL_EBC:
				switch(ebc_boot_size) {
				case SDR0_SDSTP1_EBC_ROM_BS_16BIT:
					computed_boot_device = BOOT_FROM_LARGE_FLASH_OR_SRAM;
					break;
				case SDR0_SDSTP1_EBC_ROM_BS_8BIT:
					computed_boot_device = BOOT_FROM_SMALL_FLASH;
					break;
				}
				break;

			case SDR0_SDSTP1_BOOT_SEL_PCI:
				computed_boot_device = BOOT_FROM_PCI;
				break;

			case SDR0_SDSTP1_BOOT_SEL_NDFC:
				computed_boot_device = BOOT_FROM_NAND_FLASH0;
				break;
			}
			break;
		}
	}

	/*-------------------------------------------------------------------------+
	  |  PPC440EP Pass2
	  +-------------------------------------------------------------------------*/
	else {
		switch(bootstrap_settings) {
		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS0:
			/* Default Strap Settings 0 : CPU 400 - PLB 133 - Boot EBC 8 bit 33MHz */
			/* Boot from Small Flash */
			computed_boot_device = BOOT_FROM_SMALL_FLASH;
			break;
		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS1:
			/* Default Strap Settings 1 : CPU 333 - PLB 133 - Boot PCI 66MHz */
			/* Boot from PCI */
			computed_boot_device = BOOT_FROM_PCI;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS2:
			/* Default Strap Settings 2 : CPU 400 - PLB 100 - Boot NDFC16 33MHz */
			/* Boot from Nand Flash */
			computed_boot_device = BOOT_FROM_NAND_FLASH0;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS3:
			/* Default Strap Settings 3 : CPU 400 - PLB 100 - Boot EBC 16 bit 33MHz */
			/* Boot from Large Flash or SRAM */
			computed_boot_device = BOOT_FROM_LARGE_FLASH_OR_SRAM;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS4:
			/* Default Strap Settings 4 : CPU 333 - PLB 133 - Boot EBC 16 bit 66MHz */
			/* Boot from Large Flash or SRAM */
			computed_boot_device = BOOT_FROM_LARGE_FLASH_OR_SRAM;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_SETTINGS6:
			/* Default Strap Settings 6 : CPU 400 - PLB 100 - Boot PCI 33MHz */
			/* Boot from PCI */
			computed_boot_device = BOOT_FROM_PCI;
			break;

		case SDR0_PSTRP0_BOOTSTRAP_IIC_A8_EN:
		case SDR0_PSTRP0_BOOTSTRAP_IIC_A4_EN:
			/* Default Strap Settings 5-7 */
			/* Boot Settings in IIC EEprom address 0xA8 or 0xA4 */
			/* Read Serial Device Strap Register1 in PPC440EP */
			mfsdr(sdr_sdstp1, sdr0_sdstp1);
			boot_selection	= sdr0_sdstp1 & SDR0_SDSTP1_BOOT_SEL_MASK;
			ebc_boot_size	= sdr0_sdstp1 & SDR0_SDSTP1_EBC_ROM_BS_MASK;

			switch(boot_selection) {
			case SDR0_SDSTP1_BOOT_SEL_EBC:
				switch(ebc_boot_size) {
				case SDR0_SDSTP1_EBC_ROM_BS_16BIT:
					computed_boot_device = BOOT_FROM_LARGE_FLASH_OR_SRAM;
					break;
				case SDR0_SDSTP1_EBC_ROM_BS_8BIT:
					computed_boot_device = BOOT_FROM_SMALL_FLASH;
					break;
				}
				break;

			case SDR0_SDSTP1_BOOT_SEL_PCI:
				computed_boot_device = BOOT_FROM_PCI;
				break;

			case SDR0_SDSTP1_BOOT_SEL_NDFC:
				computed_boot_device = BOOT_FROM_NAND_FLASH0;
				break;
			}
			break;
		}
	}

	/*-------------------------------------------------------------------------+
	  |
	  |  PART 3 : Compute EBC settings depending on selected boot device
	  |  ======   ======================================================
	  |
	  | Resulting EBC init will be among following configurations :
	  |
	  |  - Boot from EBC 8bits => boot from SMALL FLASH selected
	  |	       EBC-CS0	   = Small Flash
	  |	       EBC-CS1,2,3 = NAND Flash or
	  |			    Exp.Slot depending on Soft Config
	  |	       EBC-CS4	   = SRAM/Large Flash or
	  |			    Large Flash/SRAM depending on jumpers
	  |	       EBC-CS5	   = NVRAM / EPLD
	  |
	  |  - Boot from EBC 16bits => boot from Large Flash or SRAM selected
	  |	       EBC-CS0	   = SRAM/Large Flash or
	  |			     Large Flash/SRAM depending on jumpers
	  |	       EBC-CS1,2,3 = NAND Flash or
	  |			     Exp.Slot depending on Software Configuration
	  |	       EBC-CS4	   = Small Flash
	  |	       EBC-CS5	   = NVRAM / EPLD
	  |
	  |  - Boot from NAND Flash
	  |	       EBC-CS0	   = NAND Flash0
	  |	       EBC-CS1,2,3 = NAND Flash1
	  |	       EBC-CS4	   = SRAM/Large Flash or
	  |			     Large Flash/SRAM depending on jumpers
	  |	       EBC-CS5	   = NVRAM / EPLD
	  |
	  |    - Boot from PCI
	  |	       EBC-CS0	   = ...
	  |	       EBC-CS1,2,3 = NAND Flash or
	  |			     Exp.Slot depending on Software Configuration
	  |	       EBC-CS4	   = SRAM/Large Flash or
	  |			     Large Flash/SRAM or
	  |			     Small Flash depending on jumpers
	  |	       EBC-CS5	   = NVRAM / EPLD
	  |
	  +-------------------------------------------------------------------------*/

	switch(computed_boot_device) {
		/*------------------------------------------------------------------------- */
	case BOOT_FROM_SMALL_FLASH:
		/*------------------------------------------------------------------------- */
		ebc0_cs0_bnap_value = EBC0_BNAP_SMALL_FLASH;
		ebc0_cs0_bncr_value = EBC0_BNCR_SMALL_FLASH_CS0;
		if ((is_nand_selected()) == TRUE) {
			/* NAND Flash */
			ebc0_cs1_bnap_value = EBC0_BNAP_NAND_FLASH;
			ebc0_cs1_bncr_value = EBC0_BNCR_NAND_FLASH_CS1;
			ebc0_cs2_bnap_value = EBC0_BNAP_NAND_FLASH;
			ebc0_cs2_bncr_value = EBC0_BNCR_NAND_FLASH_CS2;
			ebc0_cs3_bnap_value = 0;
			ebc0_cs3_bncr_value = 0;
		} else {
			/* Expansion Slot */
			ebc0_cs1_bnap_value = 0;
			ebc0_cs1_bncr_value = 0;
			ebc0_cs2_bnap_value = 0;
			ebc0_cs2_bncr_value = 0;
			ebc0_cs3_bnap_value = 0;
			ebc0_cs3_bncr_value = 0;
		}
		ebc0_cs4_bnap_value = EBC0_BNAP_LARGE_FLASH_OR_SRAM;
		ebc0_cs4_bncr_value = EBC0_BNCR_LARGE_FLASH_OR_SRAM_CS4;

		break;

		/*------------------------------------------------------------------------- */
	case BOOT_FROM_LARGE_FLASH_OR_SRAM:
		/*------------------------------------------------------------------------- */
		ebc0_cs0_bnap_value = EBC0_BNAP_LARGE_FLASH_OR_SRAM;
		ebc0_cs0_bncr_value = EBC0_BNCR_LARGE_FLASH_OR_SRAM_CS0;
		if ((is_nand_selected()) == TRUE) {
			/* NAND Flash */
			ebc0_cs1_bnap_value = EBC0_BNAP_NAND_FLASH;
			ebc0_cs1_bncr_value = EBC0_BNCR_NAND_FLASH_CS1;
			ebc0_cs2_bnap_value = 0;
			ebc0_cs2_bncr_value = 0;
			ebc0_cs3_bnap_value = 0;
			ebc0_cs3_bncr_value = 0;
		} else {
			/* Expansion Slot */
			ebc0_cs1_bnap_value = 0;
			ebc0_cs1_bncr_value = 0;
			ebc0_cs2_bnap_value = 0;
			ebc0_cs2_bncr_value = 0;
			ebc0_cs3_bnap_value = 0;
			ebc0_cs3_bncr_value = 0;
		}
		ebc0_cs4_bnap_value = EBC0_BNAP_SMALL_FLASH;
		ebc0_cs4_bncr_value = EBC0_BNCR_SMALL_FLASH_CS4;

		break;

		/*------------------------------------------------------------------------- */
	case BOOT_FROM_NAND_FLASH0:
		/*------------------------------------------------------------------------- */
		ebc0_cs0_bnap_value = 0;
		ebc0_cs0_bncr_value = 0;

		ebc0_cs1_bnap_value = EBC0_BNAP_NAND_FLASH;
		ebc0_cs1_bncr_value = EBC0_BNCR_NAND_FLASH_CS1;
		ebc0_cs2_bnap_value = 0;
		ebc0_cs2_bncr_value = 0;
		ebc0_cs3_bnap_value = 0;
		ebc0_cs3_bncr_value = 0;

		/* Large Flash or SRAM */
		ebc0_cs4_bnap_value = EBC0_BNAP_LARGE_FLASH_OR_SRAM;
		ebc0_cs4_bncr_value = EBC0_BNCR_LARGE_FLASH_OR_SRAM_CS4;

		break;

		/*------------------------------------------------------------------------- */
	case BOOT_FROM_PCI:
		/*------------------------------------------------------------------------- */
		ebc0_cs0_bnap_value = 0;
		ebc0_cs0_bncr_value = 0;

		if ((is_nand_selected()) == TRUE) {
			/* NAND Flash */
			ebc0_cs1_bnap_value = EBC0_BNAP_NAND_FLASH;
			ebc0_cs1_bncr_value = EBC0_BNCR_NAND_FLASH_CS1;
			ebc0_cs2_bnap_value = 0;
			ebc0_cs2_bncr_value = 0;
			ebc0_cs3_bnap_value = 0;
			ebc0_cs3_bncr_value = 0;
		} else {
			/* Expansion Slot */
			ebc0_cs1_bnap_value = 0;
			ebc0_cs1_bncr_value = 0;
			ebc0_cs2_bnap_value = 0;
			ebc0_cs2_bncr_value = 0;
			ebc0_cs3_bnap_value = 0;
			ebc0_cs3_bncr_value = 0;
		}

		if ((config_on_ebc_cs4_is_small_flash()) == TRUE) {
			/* Small Flash */
			ebc0_cs4_bnap_value = EBC0_BNAP_SMALL_FLASH;
			ebc0_cs4_bncr_value = EBC0_BNCR_SMALL_FLASH_CS4;
		} else {
			/* Large Flash or SRAM */
			ebc0_cs4_bnap_value = EBC0_BNAP_LARGE_FLASH_OR_SRAM;
			ebc0_cs4_bncr_value = EBC0_BNCR_LARGE_FLASH_OR_SRAM_CS4;
		}

		break;

		/*------------------------------------------------------------------------- */
	case BOOT_DEVICE_UNKNOWN:
		/*------------------------------------------------------------------------- */
		/* Error */
		break;

	}


	/*-------------------------------------------------------------------------+
	  | Initialize EBC CONFIG
	  +-------------------------------------------------------------------------*/
	mtdcr(ebccfga, xbcfg);
	mtdcr(ebccfgd, EBC0_CFG_EBTC_DRIVEN	   |
	      EBC0_CFG_PTD_ENABLED	  |
	      EBC0_CFG_RTC_2048PERCLK	  |
	      EBC0_CFG_EMPL_LOW		  |
	      EBC0_CFG_EMPH_LOW		  |
	      EBC0_CFG_CSTC_DRIVEN	  |
	      EBC0_CFG_BPF_ONEDW	  |
	      EBC0_CFG_EMS_8BIT		  |
	      EBC0_CFG_PME_DISABLED	  |
	      EBC0_CFG_PMT_ENCODE(0)	  );

	/*-------------------------------------------------------------------------+
	  | Initialize EBC Bank 0-4
	  +-------------------------------------------------------------------------*/
	/* EBC Bank0 */
	mtebc(pb0ap, ebc0_cs0_bnap_value);
	mtebc(pb0cr, ebc0_cs0_bncr_value);
	/* EBC Bank1 */
	mtebc(pb1ap, ebc0_cs1_bnap_value);
	mtebc(pb1cr, ebc0_cs1_bncr_value);
	/* EBC Bank2 */
	mtebc(pb2ap, ebc0_cs2_bnap_value);
	mtebc(pb2cr, ebc0_cs2_bncr_value);
	/* EBC Bank3 */
	mtebc(pb3ap, ebc0_cs3_bnap_value);
	mtebc(pb3cr, ebc0_cs3_bncr_value);
	/* EBC Bank4 */
	mtebc(pb4ap, ebc0_cs4_bnap_value);
	mtebc(pb4cr, ebc0_cs4_bncr_value);

	return;
}


/*----------------------------------------------------------------------------+
  | get_uart_configuration.
  +----------------------------------------------------------------------------*/
uart_config_nb_t get_uart_configuration(void)
{
	return (L4);
}

/*----------------------------------------------------------------------------+
  | set_phy_configuration_through_fpga => to EPLD
  +----------------------------------------------------------------------------*/
void set_phy_configuration_through_fpga(zmii_config_t config)
{

	unsigned long fpga_selection_reg;

	fpga_selection_reg = in8(FPGA_SELECTION_1_REG) & ~FPGA_SEL_1_REG_PHY_MASK;

	switch(config)
	{
	case ZMII_CONFIGURATION_IS_MII:
		fpga_selection_reg = fpga_selection_reg | FPGA_SEL_1_REG_MII;
		break;
	case ZMII_CONFIGURATION_IS_RMII:
		fpga_selection_reg = fpga_selection_reg | FPGA_SEL_1_REG_RMII;
		break;
	case ZMII_CONFIGURATION_IS_SMII:
		fpga_selection_reg = fpga_selection_reg | FPGA_SEL_1_REG_SMII;
		break;
	case ZMII_CONFIGURATION_UNKNOWN:
	default:
		break;
	}
	out8(FPGA_SELECTION_1_REG,fpga_selection_reg);

}

/*----------------------------------------------------------------------------+
  | scp_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void scp_selection_in_fpga(void)
{
	unsigned long fpga_selection_2_reg;

	fpga_selection_2_reg = in8(FPGA_SELECTION_2_REG) & ~FPGA_SEL2_REG_IIC1_SCP_SEL_MASK;
	fpga_selection_2_reg |= FPGA_SEL2_REG_SEL_SCP;
	out8(FPGA_SELECTION_2_REG,fpga_selection_2_reg);
}

/*----------------------------------------------------------------------------+
  | iic1_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void iic1_selection_in_fpga(void)
{
	unsigned long fpga_selection_2_reg;

	fpga_selection_2_reg = in8(FPGA_SELECTION_2_REG) & ~FPGA_SEL2_REG_IIC1_SCP_SEL_MASK;
	fpga_selection_2_reg |= FPGA_SEL2_REG_SEL_IIC1;
	out8(FPGA_SELECTION_2_REG,fpga_selection_2_reg);
}

/*----------------------------------------------------------------------------+
  | dma_a_b_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void dma_a_b_selection_in_fpga(void)
{
	unsigned long fpga_selection_2_reg;

	fpga_selection_2_reg = in8(FPGA_SELECTION_2_REG) | FPGA_SEL2_REG_SEL_DMA_A_B;
	out8(FPGA_SELECTION_2_REG,fpga_selection_2_reg);
}

/*----------------------------------------------------------------------------+
  | dma_a_b_unselect_in_fpga.
  +----------------------------------------------------------------------------*/
void dma_a_b_unselect_in_fpga(void)
{
	unsigned long fpga_selection_2_reg;

	fpga_selection_2_reg = in8(FPGA_SELECTION_2_REG) & ~FPGA_SEL2_REG_SEL_DMA_A_B;
	out8(FPGA_SELECTION_2_REG,fpga_selection_2_reg);
}

/*----------------------------------------------------------------------------+
  | dma_c_d_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void dma_c_d_selection_in_fpga(void)
{
	unsigned long fpga_selection_2_reg;

	fpga_selection_2_reg = in8(FPGA_SELECTION_2_REG) | FPGA_SEL2_REG_SEL_DMA_C_D;
	out8(FPGA_SELECTION_2_REG,fpga_selection_2_reg);
}

/*----------------------------------------------------------------------------+
  | dma_c_d_unselect_in_fpga.
  +----------------------------------------------------------------------------*/
void dma_c_d_unselect_in_fpga(void)
{
	unsigned long fpga_selection_2_reg;

	fpga_selection_2_reg = in8(FPGA_SELECTION_2_REG) & ~FPGA_SEL2_REG_SEL_DMA_C_D;
	out8(FPGA_SELECTION_2_REG,fpga_selection_2_reg);
}

/*----------------------------------------------------------------------------+
  | usb2_device_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void usb2_device_selection_in_fpga(void)
{
	unsigned long fpga_selection_1_reg;

	fpga_selection_1_reg = in8(FPGA_SELECTION_1_REG) | FPGA_SEL_1_REG_USB2_DEV_SEL;
	out8(FPGA_SELECTION_1_REG,fpga_selection_1_reg);
}

/*----------------------------------------------------------------------------+
  | usb2_device_reset_through_fpga.
  +----------------------------------------------------------------------------*/
void usb2_device_reset_through_fpga(void)
{
	/* Perform soft Reset pulse */
	unsigned long fpga_reset_reg;
	int i;

	fpga_reset_reg = in8(FPGA_RESET_REG);
	out8(FPGA_RESET_REG,fpga_reset_reg | FPGA_RESET_REG_RESET_USB20_DEV);
	for (i=0; i<500; i++)
		udelay(1000);
	out8(FPGA_RESET_REG,fpga_reset_reg);
}

/*----------------------------------------------------------------------------+
  | usb2_host_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void usb2_host_selection_in_fpga(void)
{
	unsigned long fpga_selection_1_reg;

	fpga_selection_1_reg = in8(FPGA_SELECTION_1_REG) | FPGA_SEL_1_REG_USB2_HOST_SEL;
	out8(FPGA_SELECTION_1_REG,fpga_selection_1_reg);
}

/*----------------------------------------------------------------------------+
  | ndfc_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void ndfc_selection_in_fpga(void)
{
	unsigned long fpga_selection_1_reg;

	fpga_selection_1_reg  = in8(FPGA_SELECTION_1_REG) &~FPGA_SEL_1_REG_NF_SELEC_MASK;
	fpga_selection_1_reg |= FPGA_SEL_1_REG_NF0_SEL_BY_NFCS1;
	fpga_selection_1_reg |= FPGA_SEL_1_REG_NF1_SEL_BY_NFCS2;
	out8(FPGA_SELECTION_1_REG,fpga_selection_1_reg);
}

/*----------------------------------------------------------------------------+
  | uart_selection_in_fpga.
  +----------------------------------------------------------------------------*/
void uart_selection_in_fpga(uart_config_nb_t uart_config)
{
	/* FPGA register */
	unsigned char	fpga_selection_3_reg;

	/* Read FPGA Reagister */
	fpga_selection_3_reg = in8(FPGA_SELECTION_3_REG);

	switch (uart_config)
	{
	case L1:
		/* ----------------------------------------------------------------------- */
		/* L1 configuration:	UART0 = 8 pins */
		/* ----------------------------------------------------------------------- */
		/* Configure FPGA */
		fpga_selection_3_reg	= fpga_selection_3_reg & ~FPGA_SEL3_REG_SEL_UART_CONFIG_MASK;
		fpga_selection_3_reg	= fpga_selection_3_reg | FPGA_SEL3_REG_SEL_UART_CONFIG1;
		out8(FPGA_SELECTION_3_REG, fpga_selection_3_reg);

		break;

	case L2:
		/* ----------------------------------------------------------------------- */
		/* L2 configuration:	UART0 = 4 pins */
		/*			UART1 = 4 pins */
		/* ----------------------------------------------------------------------- */
		/* Configure FPGA */
		fpga_selection_3_reg	= fpga_selection_3_reg & ~FPGA_SEL3_REG_SEL_UART_CONFIG_MASK;
		fpga_selection_3_reg	= fpga_selection_3_reg | FPGA_SEL3_REG_SEL_UART_CONFIG2;
		out8(FPGA_SELECTION_3_REG, fpga_selection_3_reg);

		break;

	case L3:
		/* ----------------------------------------------------------------------- */
		/* L3 configuration:	UART0 = 4 pins */
		/*			UART1 = 2 pins */
		/*			UART2 = 2 pins */
		/* ----------------------------------------------------------------------- */
		/* Configure FPGA */
		fpga_selection_3_reg	= fpga_selection_3_reg & ~FPGA_SEL3_REG_SEL_UART_CONFIG_MASK;
		fpga_selection_3_reg	= fpga_selection_3_reg | FPGA_SEL3_REG_SEL_UART_CONFIG3;
		out8(FPGA_SELECTION_3_REG, fpga_selection_3_reg);
		break;

	case L4:
		/* Configure FPGA */
		fpga_selection_3_reg	= fpga_selection_3_reg & ~FPGA_SEL3_REG_SEL_UART_CONFIG_MASK;
		fpga_selection_3_reg	= fpga_selection_3_reg | FPGA_SEL3_REG_SEL_UART_CONFIG4;
		out8(FPGA_SELECTION_3_REG, fpga_selection_3_reg);

		break;

	default:
		/* Unsupported UART configuration number */
		for (;;)
			;
		break;

	}
}


/*----------------------------------------------------------------------------+
  | init_default_gpio
  +----------------------------------------------------------------------------*/
void init_default_gpio(void)
{
	int i;

	/* Init GPIO0 */
	for(i=0; i<GPIO_MAX; i++)
	{
		gpio_tab[GPIO0][i].add	  = GPIO0_BASE;
		gpio_tab[GPIO0][i].in_out = GPIO_DIS;
		gpio_tab[GPIO0][i].alt_nb = GPIO_SEL;
	}

	/* Init GPIO1 */
	for(i=0; i<GPIO_MAX; i++)
	{
		gpio_tab[GPIO1][i].add	  = GPIO1_BASE;
		gpio_tab[GPIO1][i].in_out = GPIO_DIS;
		gpio_tab[GPIO1][i].alt_nb = GPIO_SEL;
	}

	/* EBC_CS_N(5) - GPIO0_10 */
	gpio_tab[GPIO0][10].in_out    = GPIO_OUT;
	gpio_tab[GPIO0][10].alt_nb    = GPIO_ALT1;

	/* EBC_CS_N(4) - GPIO0_9 */
	gpio_tab[GPIO0][9].in_out    = GPIO_OUT;
	gpio_tab[GPIO0][9].alt_nb    = GPIO_ALT1;
}

/*----------------------------------------------------------------------------+
  | update_uart_ios
  +------------------------------------------------------------------------------
  |
  | Set UART Configuration in PowerPC440EP
  |
  | +---------------------------------------------------------------------+
  | | Configuartion   |	  Connector   | Nb of pins | Pins   | Associated  |
  | |	 Number	      |	  Port Name   |	 available | naming |	CORE	  |
  | +-----------------+---------------+------------+--------+-------------+
  | |	  L1	      |	  Port_A      |	    8	   | UART   | UART core 0 |
  | +-----------------+---------------+------------+--------+-------------+
  | |	  L2	      |	  Port_A      |	    4	   | UART1  | UART core 0 |
  | |	 (L2D)	      |	  Port_B      |	    4	   | UART2  | UART core 1 |
  | +-----------------+---------------+------------+--------+-------------+
  | |	  L3	      |	  Port_A      |	    4	   | UART1  | UART core 0 |
  | |	 (L3D)	      |	  Port_B      |	    2	   | UART2  | UART core 1 |
  | |		      |	  Port_C      |	    2	   | UART3  | UART core 2 |
  | +-----------------+---------------+------------+--------+-------------+
  | |		      |	  Port_A      |	    2	   | UART1  | UART core 0 |
  | |	  L4	      |	  Port_B      |	    2	   | UART2  | UART core 1 |
  | |	 (L4D)	      |	  Port_C      |	    2	   | UART3  | UART core 2 |
  | |		      |	  Port_D      |	    2	   | UART4  | UART core 3 |
  | +-----------------+---------------+------------+--------+-------------+
  |
  |  Involved GPIOs
  |
  | +------------------------------------------------------------------------------+
  | |  GPIO   |	  Aternate 1	 | I/O |  Alternate 2	 | I/O | Alternate 3 | I/O |
  | +---------+------------------+-----+-----------------+-----+-------------+-----+
  | | GPIO1_2 | UART0_DCD_N	 |  I  | UART1_DSR_CTS_N |  I  | UART2_SOUT  |	O  |
  | | GPIO1_3 | UART0_8PIN_DSR_N |  I  | UART1_RTS_DTR_N |  O  | UART2_SIN   |	I  |
  | | GPIO1_4 | UART0_8PIN_CTS_N |  I  | NA		 |  NA | UART3_SIN   |	I  |
  | | GPIO1_5 | UART0_RTS_N	 |  O  | NA		 |  NA | UART3_SOUT  |	O  |
  | | GPIO1_6 | UART0_DTR_N	 |  O  | UART1_SOUT	 |  O  | NA	     |	NA |
  | | GPIO1_7 | UART0_RI_N	 |  I  | UART1_SIN	 |  I  | NA	     |	NA |
  | +------------------------------------------------------------------------------+
  |
  |
  +----------------------------------------------------------------------------*/

void update_uart_ios(uart_config_nb_t uart_config)
{
	switch (uart_config)
	{
	case L1:
		/* ----------------------------------------------------------------------- */
		/* L1 configuration:	UART0 = 8 pins */
		/* ----------------------------------------------------------------------- */
		/* Update GPIO Configuration Table */
		gpio_tab[GPIO1][2].in_out = GPIO_IN;
		gpio_tab[GPIO1][2].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][3].in_out = GPIO_IN;
		gpio_tab[GPIO1][3].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][4].in_out = GPIO_IN;
		gpio_tab[GPIO1][4].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][5].in_out = GPIO_OUT;
		gpio_tab[GPIO1][5].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][6].in_out = GPIO_OUT;
		gpio_tab[GPIO1][6].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][7].in_out = GPIO_IN;
		gpio_tab[GPIO1][7].alt_nb = GPIO_ALT1;

		break;

	case L2:
		/* ----------------------------------------------------------------------- */
		/* L2 configuration:	UART0 = 4 pins */
		/*			UART1 = 4 pins */
		/* ----------------------------------------------------------------------- */
		/* Update GPIO Configuration Table */
		gpio_tab[GPIO1][2].in_out = GPIO_IN;
		gpio_tab[GPIO1][2].alt_nb = GPIO_ALT2;

		gpio_tab[GPIO1][3].in_out = GPIO_OUT;
		gpio_tab[GPIO1][3].alt_nb = GPIO_ALT2;

		gpio_tab[GPIO1][4].in_out = GPIO_IN;
		gpio_tab[GPIO1][4].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][5].in_out = GPIO_OUT;
		gpio_tab[GPIO1][5].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][6].in_out = GPIO_OUT;
		gpio_tab[GPIO1][6].alt_nb = GPIO_ALT2;

		gpio_tab[GPIO1][7].in_out = GPIO_IN;
		gpio_tab[GPIO1][7].alt_nb = GPIO_ALT2;

		break;

	case L3:
		/* ----------------------------------------------------------------------- */
		/* L3 configuration:	UART0 = 4 pins */
		/*			UART1 = 2 pins */
		/*			UART2 = 2 pins */
		/* ----------------------------------------------------------------------- */
		/* Update GPIO Configuration Table */
		gpio_tab[GPIO1][2].in_out = GPIO_OUT;
		gpio_tab[GPIO1][2].alt_nb = GPIO_ALT3;

		gpio_tab[GPIO1][3].in_out = GPIO_IN;
		gpio_tab[GPIO1][3].alt_nb = GPIO_ALT3;

		gpio_tab[GPIO1][4].in_out = GPIO_IN;
		gpio_tab[GPIO1][4].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][5].in_out = GPIO_OUT;
		gpio_tab[GPIO1][5].alt_nb = GPIO_ALT1;

		gpio_tab[GPIO1][6].in_out = GPIO_OUT;
		gpio_tab[GPIO1][6].alt_nb = GPIO_ALT2;

		gpio_tab[GPIO1][7].in_out = GPIO_IN;
		gpio_tab[GPIO1][7].alt_nb = GPIO_ALT2;

		break;

	case L4:
		/* ----------------------------------------------------------------------- */
		/* L4 configuration:	UART0 = 2 pins */
		/*			UART1 = 2 pins */
		/*			UART2 = 2 pins */
		/*			UART3 = 2 pins */
		/* ----------------------------------------------------------------------- */
		/* Update GPIO Configuration Table */
		gpio_tab[GPIO1][2].in_out = GPIO_OUT;
		gpio_tab[GPIO1][2].alt_nb = GPIO_ALT3;

		gpio_tab[GPIO1][3].in_out = GPIO_IN;
		gpio_tab[GPIO1][3].alt_nb = GPIO_ALT3;

		gpio_tab[GPIO1][4].in_out = GPIO_IN;
		gpio_tab[GPIO1][4].alt_nb = GPIO_ALT3;

		gpio_tab[GPIO1][5].in_out = GPIO_OUT;
		gpio_tab[GPIO1][5].alt_nb = GPIO_ALT3;

		gpio_tab[GPIO1][6].in_out = GPIO_OUT;
		gpio_tab[GPIO1][6].alt_nb = GPIO_ALT2;

		gpio_tab[GPIO1][7].in_out = GPIO_IN;
		gpio_tab[GPIO1][7].alt_nb = GPIO_ALT2;

		break;

	default:
		/* Unsupported UART configuration number */
		printf("ERROR - Unsupported UART configuration number.\n\n");
		for (;;)
			;
		break;

	}

	/* Set input Selection Register on Alt_Receive for UART Input Core */
	out32(GPIO1_IS1L, (in32(GPIO1_IS1L) | 0x0FC30000));
	out32(GPIO1_IS2L, (in32(GPIO1_IS2L) | 0x0C030000));
	out32(GPIO1_IS3L, (in32(GPIO1_IS3L) | 0x03C00000));
}

/*----------------------------------------------------------------------------+
  | update_ndfc_ios(void).
  +----------------------------------------------------------------------------*/
void update_ndfc_ios(void)
{
	/* Update GPIO Configuration Table */
	gpio_tab[GPIO0][6].in_out = GPIO_OUT;	    /* EBC_CS_N(1) */
	gpio_tab[GPIO0][6].alt_nb = GPIO_ALT1;

#if 0
	gpio_tab[GPIO0][7].in_out = GPIO_OUT;	    /* EBC_CS_N(2) */
	gpio_tab[GPIO0][7].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][7].in_out = GPIO_OUT;	    /* EBC_CS_N(3) */
	gpio_tab[GPIO0][7].alt_nb = GPIO_ALT1;
#endif
}

/*----------------------------------------------------------------------------+
  | update_zii_ios(void).
  +----------------------------------------------------------------------------*/
void update_zii_ios(void)
{
	/* Update GPIO Configuration Table */
	gpio_tab[GPIO0][12].in_out = GPIO_IN;	    /* ZII_p0Rxd(0) */
	gpio_tab[GPIO0][12].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][13].in_out = GPIO_IN;	    /* ZII_p0Rxd(1) */
	gpio_tab[GPIO0][13].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][14].in_out = GPIO_IN;	    /* ZII_p0Rxd(2) */
	gpio_tab[GPIO0][14].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][15].in_out = GPIO_IN;	    /* ZII_p0Rxd(3) */
	gpio_tab[GPIO0][15].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][16].in_out = GPIO_OUT;	    /* ZII_p0Txd(0) */
	gpio_tab[GPIO0][16].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][17].in_out = GPIO_OUT;	    /* ZII_p0Txd(1) */
	gpio_tab[GPIO0][17].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][18].in_out = GPIO_OUT;	    /* ZII_p0Txd(2) */
	gpio_tab[GPIO0][18].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][19].in_out = GPIO_OUT;	    /* ZII_p0Txd(3) */
	gpio_tab[GPIO0][19].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][20].in_out = GPIO_IN;	    /* ZII_p0Rx_er */
	gpio_tab[GPIO0][20].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][21].in_out = GPIO_IN;	    /* ZII_p0Rx_dv */
	gpio_tab[GPIO0][21].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][22].in_out = GPIO_IN;	    /* ZII_p0Crs */
	gpio_tab[GPIO0][22].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][23].in_out = GPIO_OUT;	    /* ZII_p0Tx_er */
	gpio_tab[GPIO0][23].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][24].in_out = GPIO_OUT;	    /* ZII_p0Tx_en */
	gpio_tab[GPIO0][24].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][25].in_out = GPIO_IN;	    /* ZII_p0Col */
	gpio_tab[GPIO0][25].alt_nb = GPIO_ALT1;

}

/*----------------------------------------------------------------------------+
  | update_uic_0_3_irq_ios().
  +----------------------------------------------------------------------------*/
void update_uic_0_3_irq_ios(void)
{
	gpio_tab[GPIO1][8].in_out = GPIO_IN;	    /* UIC_IRQ(0) */
	gpio_tab[GPIO1][8].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][9].in_out = GPIO_IN;	    /* UIC_IRQ(1) */
	gpio_tab[GPIO1][9].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][10].in_out = GPIO_IN;	    /* UIC_IRQ(2) */
	gpio_tab[GPIO1][10].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][11].in_out = GPIO_IN;	    /* UIC_IRQ(3) */
	gpio_tab[GPIO1][11].alt_nb = GPIO_ALT1;
}

/*----------------------------------------------------------------------------+
  | update_uic_4_9_irq_ios().
  +----------------------------------------------------------------------------*/
void update_uic_4_9_irq_ios(void)
{
	gpio_tab[GPIO1][12].in_out = GPIO_IN;	    /* UIC_IRQ(4) */
	gpio_tab[GPIO1][12].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][13].in_out = GPIO_IN;	    /* UIC_IRQ(6) */
	gpio_tab[GPIO1][13].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][14].in_out = GPIO_IN;	    /* UIC_IRQ(7) */
	gpio_tab[GPIO1][14].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][15].in_out = GPIO_IN;	    /* UIC_IRQ(8) */
	gpio_tab[GPIO1][15].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][16].in_out = GPIO_IN;	    /* UIC_IRQ(9) */
	gpio_tab[GPIO1][16].alt_nb = GPIO_ALT1;
}

/*----------------------------------------------------------------------------+
  | update_dma_a_b_ios().
  +----------------------------------------------------------------------------*/
void update_dma_a_b_ios(void)
{
	gpio_tab[GPIO1][12].in_out = GPIO_OUT;	    /* DMA_ACK(1) */
	gpio_tab[GPIO1][12].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO1][13].in_out = GPIO_BI;	    /* DMA_EOT/TC(1) */
	gpio_tab[GPIO1][13].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO1][14].in_out = GPIO_IN;	    /* DMA_REQ(0) */
	gpio_tab[GPIO1][14].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO1][15].in_out = GPIO_OUT;	    /* DMA_ACK(0) */
	gpio_tab[GPIO1][15].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO1][16].in_out = GPIO_BI;	    /* DMA_EOT/TC(0) */
	gpio_tab[GPIO1][16].alt_nb = GPIO_ALT2;
}

/*----------------------------------------------------------------------------+
  | update_dma_c_d_ios().
  +----------------------------------------------------------------------------*/
void update_dma_c_d_ios(void)
{
	gpio_tab[GPIO0][0].in_out = GPIO_IN;	    /* DMA_REQ(2) */
	gpio_tab[GPIO0][0].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][1].in_out = GPIO_OUT;	    /* DMA_ACK(2) */
	gpio_tab[GPIO0][1].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][2].in_out = GPIO_BI;	    /* DMA_EOT/TC(2) */
	gpio_tab[GPIO0][2].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][3].in_out = GPIO_IN;	    /* DMA_REQ(3) */
	gpio_tab[GPIO0][3].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][4].in_out = GPIO_OUT;	    /* DMA_ACK(3) */
	gpio_tab[GPIO0][4].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][5].in_out = GPIO_BI;	    /* DMA_EOT/TC(3) */
	gpio_tab[GPIO0][5].alt_nb = GPIO_ALT2;

}

/*----------------------------------------------------------------------------+
  | update_ebc_master_ios().
  +----------------------------------------------------------------------------*/
void update_ebc_master_ios(void)
{
	gpio_tab[GPIO0][27].in_out = GPIO_IN;	    /* EXT_EBC_REQ */
	gpio_tab[GPIO0][27].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][29].in_out = GPIO_OUT;	    /* EBC_EXT_HDLA */
	gpio_tab[GPIO0][29].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][30].in_out = GPIO_OUT;	    /* EBC_EXT_ACK */
	gpio_tab[GPIO0][30].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO0][31].in_out = GPIO_OUT;	    /* EBC_EXR_BUSREQ */
	gpio_tab[GPIO0][31].alt_nb = GPIO_ALT1;
}

/*----------------------------------------------------------------------------+
  | update_usb2_device_ios().
  +----------------------------------------------------------------------------*/
void update_usb2_device_ios(void)
{
	gpio_tab[GPIO0][26].in_out = GPIO_IN;	    /* USB2D_RXVALID */
	gpio_tab[GPIO0][26].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][27].in_out = GPIO_IN;	    /* USB2D_RXERROR */
	gpio_tab[GPIO0][27].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][28].in_out = GPIO_OUT;	    /* USB2D_TXVALID */
	gpio_tab[GPIO0][28].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][29].in_out = GPIO_OUT;	    /* USB2D_PAD_SUSPNDM */
	gpio_tab[GPIO0][29].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][30].in_out = GPIO_OUT;	    /* USB2D_XCVRSELECT */
	gpio_tab[GPIO0][30].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO0][31].in_out = GPIO_OUT;	    /* USB2D_TERMSELECT */
	gpio_tab[GPIO0][31].alt_nb = GPIO_ALT2;

	gpio_tab[GPIO1][0].in_out = GPIO_OUT;	    /* USB2D_OPMODE0 */
	gpio_tab[GPIO1][0].alt_nb = GPIO_ALT1;

	gpio_tab[GPIO1][1].in_out = GPIO_OUT;	    /* USB2D_OPMODE1 */
	gpio_tab[GPIO1][1].alt_nb = GPIO_ALT1;

}

/*----------------------------------------------------------------------------+
  | update_pci_patch_ios().
  +----------------------------------------------------------------------------*/
void update_pci_patch_ios(void)
{
	gpio_tab[GPIO0][29].in_out = GPIO_OUT;	    /* EBC_EXT_HDLA */
	gpio_tab[GPIO0][29].alt_nb = GPIO_ALT1;
}

/*----------------------------------------------------------------------------+
  |   set_chip_gpio_configuration(unsigned char gpio_core)
  |   Put the core impacted by clock modification and sharing in reset.
  |   Config the select registers to resolve the sharing depending of the config.
  |   Configure the GPIO registers.
  |
  +----------------------------------------------------------------------------*/
void set_chip_gpio_configuration(unsigned char gpio_core)
{
	unsigned char i=0, j=0, reg_offset = 0;
	unsigned long gpio_reg, gpio_core_add;

	/* GPIO config of the GPIOs 0 to 31 */
	for (i=0; i<GPIO_MAX; i++, j++)
	{
		if (i == GPIO_MAX/2)
		{
			reg_offset = 4;
			j = i-16;
		}

		gpio_core_add = gpio_tab[gpio_core][i].add;

		if ( (gpio_tab[gpio_core][i].in_out == GPIO_IN) ||
		     (gpio_tab[gpio_core][i].in_out == GPIO_BI ))
		{
			switch (gpio_tab[gpio_core][i].alt_nb)
			{
			case GPIO_SEL:
				break;

			case GPIO_ALT1:
				gpio_reg = in32(GPIO_IS1(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_IN_SEL >> (j*2));
				out32(GPIO_IS1(gpio_core_add+reg_offset), gpio_reg);
				break;

			case GPIO_ALT2:
				gpio_reg = in32(GPIO_IS2(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_IN_SEL >> (j*2));
				out32(GPIO_IS2(gpio_core_add+reg_offset), gpio_reg);
				break;

			case GPIO_ALT3:
				gpio_reg = in32(GPIO_IS3(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_IN_SEL >> (j*2));
				out32(GPIO_IS3(gpio_core_add+reg_offset), gpio_reg);
				break;
			}
		}
		if ( (gpio_tab[gpio_core][i].in_out == GPIO_OUT) ||
		     (gpio_tab[gpio_core][i].in_out == GPIO_BI ))
		{

			switch (gpio_tab[gpio_core][i].alt_nb)
			{
			case GPIO_SEL:
				break;
			case GPIO_ALT1:
				gpio_reg = in32(GPIO_OS(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_ALT1_SEL >> (j*2));
				out32(GPIO_OS(gpio_core_add+reg_offset), gpio_reg);
				gpio_reg = in32(GPIO_TS(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_ALT1_SEL >> (j*2));
				out32(GPIO_TS(gpio_core_add+reg_offset), gpio_reg);
				break;
			case GPIO_ALT2:
				gpio_reg = in32(GPIO_OS(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_ALT2_SEL >> (j*2));
				out32(GPIO_OS(gpio_core_add+reg_offset), gpio_reg);
				gpio_reg = in32(GPIO_TS(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_ALT2_SEL >> (j*2));
				out32(GPIO_TS(gpio_core_add+reg_offset), gpio_reg);
				break;
			case GPIO_ALT3:
				gpio_reg = in32(GPIO_OS(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_ALT3_SEL >> (j*2));
				out32(GPIO_OS(gpio_core_add+reg_offset), gpio_reg);
				gpio_reg = in32(GPIO_TS(gpio_core_add+reg_offset)) & ~(GPIO_MASK >> (j*2));
				gpio_reg = gpio_reg | (GPIO_ALT3_SEL >> (j*2));
				out32(GPIO_TS(gpio_core_add+reg_offset), gpio_reg);
				break;
			}
		}
	}
}

/*----------------------------------------------------------------------------+
  | force_bup_core_selection.
  +----------------------------------------------------------------------------*/
void force_bup_core_selection(core_selection_t *core_select_P, config_validity_t *config_val_P)
{
	/* Pointer invalid */
	if (core_select_P == NULL)
	{
		printf("Configuration invalid pointer 1\n");
		for (;;)
			;
	}

	/* L4 Selection */
	*(core_select_P+UART_CORE0)	       = CORE_SELECTED;
	*(core_select_P+UART_CORE1)	       = CORE_SELECTED;
	*(core_select_P+UART_CORE2)	       = CORE_SELECTED;
	*(core_select_P+UART_CORE3)	       = CORE_SELECTED;

	/* RMII Selection */
	*(core_select_P+RMII_SEL)		= CORE_SELECTED;

	/* External Interrupt 0-9 selection */
	*(core_select_P+UIC_0_3)		= CORE_SELECTED;
	*(core_select_P+UIC_4_9)		= CORE_SELECTED;

	*(core_select_P+SCP_CORE)	        = CORE_SELECTED;
	*(core_select_P+DMA_CHANNEL_CD)		= CORE_SELECTED;
	*(core_select_P+PACKET_REJ_FUNC_AVAIL)	= CORE_SELECTED;
	*(core_select_P+USB1_DEVICE)		= CORE_SELECTED;

	if (is_nand_selected()) {
		*(core_select_P+NAND_FLASH)	= CORE_SELECTED;
	}

	*config_val_P = CONFIG_IS_VALID;

}

/*----------------------------------------------------------------------------+
  | configure_ppc440ep_pins.
  +----------------------------------------------------------------------------*/
void configure_ppc440ep_pins(void)
{
	uart_config_nb_t uart_configuration;
	config_validity_t config_val = CONFIG_IS_INVALID;

	/* Create Core Selection Table */
	core_selection_t ppc440ep_core_selection[MAX_CORE_SELECT_NB] =
		{
			CORE_NOT_SELECTED,	/* IIC_CORE, */
			CORE_NOT_SELECTED,	/* SPC_CORE, */
			CORE_NOT_SELECTED,	/* DMA_CHANNEL_AB, */
			CORE_NOT_SELECTED,	/* UIC_4_9, */
			CORE_NOT_SELECTED,	/* USB2_HOST, */
			CORE_NOT_SELECTED,	/* DMA_CHANNEL_CD, */
			CORE_NOT_SELECTED,	/* USB2_DEVICE, */
			CORE_NOT_SELECTED,	/* PACKET_REJ_FUNC_AVAIL, */
			CORE_NOT_SELECTED,	/* USB1_DEVICE, */
			CORE_NOT_SELECTED,	/* EBC_MASTER, */
			CORE_NOT_SELECTED,	/* NAND_FLASH, */
			CORE_NOT_SELECTED,	/* UART_CORE0, */
			CORE_NOT_SELECTED,	/* UART_CORE1, */
			CORE_NOT_SELECTED,	/* UART_CORE2, */
			CORE_NOT_SELECTED,	/* UART_CORE3, */
			CORE_NOT_SELECTED,	/* MII_SEL, */
			CORE_NOT_SELECTED,	/* RMII_SEL, */
			CORE_NOT_SELECTED,	/* SMII_SEL, */
			CORE_NOT_SELECTED,	/* PACKET_REJ_FUNC_EN */
			CORE_NOT_SELECTED,	/* UIC_0_3 */
			CORE_NOT_SELECTED,	/* USB1_HOST */
			CORE_NOT_SELECTED	/* PCI_PATCH */
		};


	/* Table Default Initialisation + FPGA Access */
	init_default_gpio();
	set_chip_gpio_configuration(GPIO0);
	set_chip_gpio_configuration(GPIO1);

	/* Update Table */
	force_bup_core_selection(ppc440ep_core_selection, &config_val);
#if 0 /* test-only */
	/* If we are running PIBS 1, force known configuration */
	update_core_selection_table(ppc440ep_core_selection, &config_val);
#endif

	/*----------------------------------------------------------------------------+
	  | SDR + ios table update + fpga initialization
	  +----------------------------------------------------------------------------*/
	unsigned long sdr0_pfc1	    = 0;
	unsigned long sdr0_usb0	    = 0;
	unsigned long sdr0_mfr	    = 0;

	/* PCI Always selected */

	/* I2C Selection */
	if (ppc440ep_core_selection[IIC_CORE] == CORE_SELECTED)
	{
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_IIC1_SEL;
		iic1_selection_in_fpga();
	}

	/* SCP Selection */
	if (ppc440ep_core_selection[SCP_CORE] == CORE_SELECTED)
	{
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_SCP_SEL;
		scp_selection_in_fpga();
	}

	/* UIC 0:3 Selection */
	if (ppc440ep_core_selection[UIC_0_3] == CORE_SELECTED)
	{
		update_uic_0_3_irq_ios();
		dma_a_b_unselect_in_fpga();
	}

	/* UIC 4:9 Selection */
	if (ppc440ep_core_selection[UIC_4_9] == CORE_SELECTED)
	{
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_DIS_MASK) | SDR0_PFC1_DIS_UICIRQ5_SEL;
		update_uic_4_9_irq_ios();
	}

	/* DMA AB Selection */
	if (ppc440ep_core_selection[DMA_CHANNEL_AB] == CORE_SELECTED)
	{
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_DIS_MASK) | SDR0_PFC1_DIS_DMAR_SEL;
		update_dma_a_b_ios();
		dma_a_b_selection_in_fpga();
	}

	/* DMA CD Selection */
	if (ppc440ep_core_selection[DMA_CHANNEL_CD] == CORE_SELECTED)
	{
		update_dma_c_d_ios();
		dma_c_d_selection_in_fpga();
	}

	/* EBC Master Selection */
	if (ppc440ep_core_selection[EBC_MASTER] == CORE_SELECTED)
	{
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_ERE_MASK) | SDR0_PFC1_ERE_EXTR_SEL;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_UES_MASK) | SDR0_PFC1_UES_EBCHR_SEL;
		update_ebc_master_ios();
	}

	/* PCI Patch Enable */
	if (ppc440ep_core_selection[PCI_PATCH] == CORE_SELECTED)
	{
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_UES_MASK) | SDR0_PFC1_UES_EBCHR_SEL;
		update_pci_patch_ios();
	}

	/* USB2 Host Selection - Not Implemented in PowerPC 440EP Pass1 */
	if (ppc440ep_core_selection[USB2_HOST] == CORE_SELECTED)
	{
		/* Not Implemented in PowerPC 440EP Pass1-Pass2 */
		printf("Invalid configuration => USB2 Host selected\n");
		for (;;)
			;
		/*usb2_host_selection_in_fpga(); */
	}

	/* USB2.0 Device Selection */
	if (ppc440ep_core_selection[USB2_DEVICE] == CORE_SELECTED)
	{
		update_usb2_device_ios();
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_UES_MASK) | SDR0_PFC1_UES_USB2D_SEL;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_UPR_MASK) | SDR0_PFC1_UPR_DISABLE;

		mfsdr(sdr_usb0, sdr0_usb0);
		sdr0_usb0 = sdr0_usb0 &~SDR0_USB0_USB_DEVSEL_MASK;
		sdr0_usb0 = sdr0_usb0 | SDR0_USB0_USB20D_DEVSEL;
		mtsdr(sdr_usb0, sdr0_usb0);

		usb2_device_selection_in_fpga();
	}

	/* USB1.1 Device Selection */
	if (ppc440ep_core_selection[USB1_DEVICE] == CORE_SELECTED)
	{
		mfsdr(sdr_usb0, sdr0_usb0);
		sdr0_usb0 = sdr0_usb0 &~SDR0_USB0_USB_DEVSEL_MASK;
		sdr0_usb0 = sdr0_usb0 | SDR0_USB0_USB11D_DEVSEL;
		mtsdr(sdr_usb0, sdr0_usb0);
	}

	/* USB1.1 Host Selection */
	if (ppc440ep_core_selection[USB1_HOST] == CORE_SELECTED)
	{
		mfsdr(sdr_usb0, sdr0_usb0);
		sdr0_usb0 = sdr0_usb0 &~SDR0_USB0_LEEN_MASK;
		sdr0_usb0 = sdr0_usb0 | SDR0_USB0_LEEN_ENABLE;
		mtsdr(sdr_usb0, sdr0_usb0);
	}

	/* NAND Flash Selection */
	if (ppc440ep_core_selection[NAND_FLASH] == CORE_SELECTED)
	{
		update_ndfc_ios();

		mtsdr(sdr_cust0, SDR0_CUST0_MUX_NDFC_SEL   |
		      SDR0_CUST0_NDFC_ENABLE	|
		      SDR0_CUST0_NDFC_BW_8_BIT	|
		      SDR0_CUST0_NDFC_ARE_MASK	|
		      SDR0_CUST0_CHIPSELGAT_EN1 |
		      SDR0_CUST0_CHIPSELGAT_EN2);

		ndfc_selection_in_fpga();
	}
	else
	{
		/* Set Mux on EMAC */
		mtsdr(sdr_cust0, SDR0_CUST0_MUX_EMAC_SEL);
	}

	/* MII Selection */
	if (ppc440ep_core_selection[MII_SEL] == CORE_SELECTED)
	{
		update_zii_ios();
		mfsdr(sdr_mfr, sdr0_mfr);
		sdr0_mfr = (sdr0_mfr & ~SDR0_MFR_ZMII_MODE_MASK) | SDR0_MFR_ZMII_MODE_MII;
		mtsdr(sdr_mfr, sdr0_mfr);

		set_phy_configuration_through_fpga(ZMII_CONFIGURATION_IS_MII);
	}

	/* RMII Selection */
	if (ppc440ep_core_selection[RMII_SEL] == CORE_SELECTED)
	{
		update_zii_ios();
		mfsdr(sdr_mfr, sdr0_mfr);
		sdr0_mfr = (sdr0_mfr & ~SDR0_MFR_ZMII_MODE_MASK) | SDR0_MFR_ZMII_MODE_RMII_10M;
		mtsdr(sdr_mfr, sdr0_mfr);

		set_phy_configuration_through_fpga(ZMII_CONFIGURATION_IS_RMII);
	}

	/* SMII Selection */
	if (ppc440ep_core_selection[SMII_SEL] == CORE_SELECTED)
	{
		update_zii_ios();
		mfsdr(sdr_mfr, sdr0_mfr);
		sdr0_mfr = (sdr0_mfr & ~SDR0_MFR_ZMII_MODE_MASK) | SDR0_MFR_ZMII_MODE_SMII;
		mtsdr(sdr_mfr, sdr0_mfr);

		set_phy_configuration_through_fpga(ZMII_CONFIGURATION_IS_SMII);
	}

	/* UART Selection */
	uart_configuration = get_uart_configuration();
	switch (uart_configuration)
	{
	case L1:	 /* L1 Selection */
		/* UART0 8 pins Only */
		/*sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) | SDR0_PFC1_U0ME_DSR_DTR; */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) |SDR0_PFC1_U0ME_CTS_RTS;	  /* Chip Pb */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0IM_MASK) | SDR0_PFC1_U0IM_8PINS;
		break;
	case L2:	 /* L2 Selection */
		/* UART0 and UART1 4 pins */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) | SDR0_PFC1_U1ME_DSR_DTR;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0IM_MASK) | SDR0_PFC1_U0IM_4PINS;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U1ME_MASK) | SDR0_PFC1_U1ME_DSR_DTR;
		break;
	case L3:	 /* L3 Selection */
		/* UART0 4 pins, UART1 and UART2 2 pins */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) | SDR0_PFC1_U1ME_DSR_DTR;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0IM_MASK) | SDR0_PFC1_U0IM_4PINS;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U1ME_MASK) | SDR0_PFC1_U1ME_DSR_DTR;
		break;
	case L4:	 /* L4 Selection */
		/* UART0, UART1, UART2 and UART3 2 pins */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) | SDR0_PFC1_U0ME_DSR_DTR;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0IM_MASK) | SDR0_PFC1_U0IM_4PINS;
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U1ME_MASK) | SDR0_PFC1_U1ME_DSR_DTR;
		break;
	}
	update_uart_ios(uart_configuration);

	/* UART Selection in all cases */
	uart_selection_in_fpga(uart_configuration);

	/* Packet Reject Function Available */
	if (ppc440ep_core_selection[PACKET_REJ_FUNC_AVAIL] == CORE_SELECTED)
	{
		/* Set UPR Bit in SDR0_PFC1 Register */
		sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_UPR_MASK) | SDR0_PFC1_UPR_ENABLE;
	}

	/* Packet Reject Function Enable */
	if (ppc440ep_core_selection[PACKET_REJ_FUNC_EN] == CORE_SELECTED)
	{
		mfsdr(sdr_mfr, sdr0_mfr);
		sdr0_mfr = (sdr0_mfr & ~SDR0_MFR_PKT_REJ_MASK) | SDR0_MFR_PKT_REJ_EN;;
		mtsdr(sdr_mfr, sdr0_mfr);
	}

	/* Perform effective access to hardware */
	mtsdr(sdr_pfc1, sdr0_pfc1);
	set_chip_gpio_configuration(GPIO0);
	set_chip_gpio_configuration(GPIO1);

	/* USB2.0 Device Reset must be done after GPIO setting */
	if (ppc440ep_core_selection[USB2_DEVICE] == CORE_SELECTED)
		usb2_device_reset_through_fpga();

}
