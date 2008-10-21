/*
 * (C) Copyright 2004
 * Elmeg Communications Systems GmbH, Juergen Selent (j.selent@elmeg.de)
 *
 * Support for the Elmeg VoVPN Gateway Module
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
#include <ioports.h>
#include <mpc8260.h>
#include <asm/m8260_pci.h>
#include <miiphy.h>

#include "m88e6060.h"

/*
 * I/O Port configuration table
 *
 * if conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

const iop_conf_t iop_conf_tab[4][32] = {
    /* Port A configuration */
    {	/*	     conf ppar psor pdir podr pdat */
	/* PA31 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1252           */
	/* PA30 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    BP_RES           */
	/* PA29 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1253           */
	/* PA28 */ { 1,   1,   1,   1,   0,   0 }, /* FCC1   RMII TX_EN       */
	/* PA27 */ { 1,   1,   1,   0,   0,   0 }, /* FCC1   RMII CRS_DV      */
	/* PA26 */ { 1,   1,   1,   0,   0,   0 }, /* FCC1   RMII RX_ERR      */
	/* PA25 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    HWID             */
	/* PA24 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    HWID             */
	/* PA23 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    HWID             */
	/* PA22 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    HWID             */
	/* PA21 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    HWID             */
	/* PA20 */ { 1,   0,   0,   1,   0,   1 }, /* GPO    LED STATUS       */
	/* PA19 */ { 1,   1,   0,   1,   0,   0 }, /* FCC1   RMII TxD[1]      */
	/* PA18 */ { 1,   1,   0,   1,   0,   0 }, /* FCC1   RMII TxD[0]      */
	/* PA17 */ { 1,   1,   0,   0,   0,   0 }, /* FCC1   RMII RxD[0]      */
	/* PA16 */ { 1,   1,   0,   0,   0,   0 }, /* FCC1   RMII RxD[1]      */
	/* PA15 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1255           */
	/* PA14 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP????           */
	/* PA13 */ { 1,   0,   0,   1,   0,   1 }, /* GPO    EN_BCTL1 XXX jse */
	/* PA12 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    SWITCH RESET     */
	/* PA11 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    DSP SL1 RESET    */
	/* PA10 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    DSP SL2 RESET    */
	/* PA9  */ { 1,   1,   0,   1,   0,   0 }, /* SMC2   TXD              */
	/* PA8  */ { 1,   1,   0,   0,   0,   0 }, /* SMC2   RXD              */
	/* PA7  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA6  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA5  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA4  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA3  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA2  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA1  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exit       */
	/* PA0  */ { 0,   0,   0,   0,   0,   0 }  /* pin does not exit       */
    },

    /* Port B configuration */
    {   /*	     conf ppar psor pdir podr pdat */
	/* PB31 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1257           */
	/* PB30 */ { 1,   1,   0,   0,   0,   0 }, /* FCC2   RMII CRS_DV      */
	/* PB29 */ { 1,   1,   1,   1,   0,   0 }, /* FCC2   RMII TX_EN       */
	/* PB28 */ { 1,   1,   0,   0,   0,   0 }, /* FCC2   RMII RX_ERR      */
	/* PB27 */ { 1,   1,   1,   0,   1,   0 }, /* TDM_B2 L1TXD XXX val=0  */
	/* PB26 */ { 1,   1,   1,   0,   1,   0 }, /* TDM_B2 L1RXD XXX val,dr */
	/* PB25 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1259           */
	/* PB24 */ { 1,   1,   1,   0,   0,   0 }, /* TDM_B2 L1RSYNC          */
	/* PB23 */ { 1,   1,   0,   1,   0,   0 }, /* FCC2   RMII TxD[1]      */
	/* PB22 */ { 1,   1,   0,   1,   0,   0 }, /* FCC2   RMII TxD[0]      */
	/* PB21 */ { 1,   1,   0,   0,   0,   0 }, /* FCC2   RMII RxD[0]      */
	/* PB20 */ { 1,   1,   0,   0,   0,   0 }, /* FCC2   RMII RxD[1]      */
	/* PB19 */ { 1,   0,   0,   1,   0,   1 }, /* GPO    PHY MDC          */
	/* PB18 */ { 1,   0,   0,   0,   0,   0 }, /* GPIO   PHY MDIO         */
	/* PB17 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB16 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB15 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB14 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB13 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB12 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB11 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB10 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB9  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB8  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB7  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB6  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB5  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB4  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB3  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB2  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB1  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PB0  */ { 0,   0,   0,   0,   0,   0 }  /* pin does not exist      */
    },

    /* Port C */
    {   /*	     conf ppar psor pdir podr pdat */
	/* PC31 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PC30 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PC29 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1183           */
	/* PC28 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1184           */
	/* PC27 */ { 1,   1,   0,   0,   0,   0 }, /* CLK5   TDM_A1 RX        */
	/* PC26 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1185           */
	/* PC25 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1178           */
	/* PC24 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1186           */
	/* PC23 */ { 1,   1,   0,   0,   0,   0 }, /* CLK9   TDM_B2 RX        */
	/* PC22 */ { 1,   1,   0,   0,   0,   0 }, /* CLK10  FCC1 RMII REFCLK */
	/* PC21 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1187           */
	/* PC20 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1182           */
	/* PC19 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1188           */
	/* PC18 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    HW RESET         */
	/* PC17 */ { 1,   1,   0,   1,   0,   0 }, /* BRG8   SWITCH CLKIN     */
	/* PC16 */ { 1,   1,   0,   0,   0,   0 }, /* CLK16  FCC2 RMII REFCLK */
	/* PC15 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL1_MTYPE_3      */
	/* PC14 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL1_MTYPE_2      */
	/* PC13 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL1_MTYPE_1      */
	/* PC12 */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL1_MTYPE_0      */
	/* PC11 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1176           */
	/* PC10 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1177           */
	/* PC9  */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL2_MTYPE_3      */
	/* PC8  */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL2_MTYPE_2      */
	/* PC7  */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL2_MTYPE_1      */
	/* PC6  */ { 1,   0,   0,   0,   0,   0 }, /* GPI    SL2_MTYPE_0      */
	/* PC5  */ { 1,   1,   0,   1,   0,   0 }, /* SMC1   TXD              */
	/* PC4  */ { 1,   1,   0,   0,   0,   0 }, /* SMC1   RXD              */
	/* PC3  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PC2  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PC1  */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1192           */
	/* PC0  */ { 1,   0,   0,   0,   0,   0 }, /* GPI    RACK             */
    },

    /* Port D */
    {   /*	     conf ppar psor pdir podr pdat */
	/* PD31 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1193           */
	/* PD30 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1194           */
	/* PD29 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1195           */
	/* PD28 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD27 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD26 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD25 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1179           */
	/* PD24 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1180           */
	/* PD23 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1181           */
	/* PD22 */ { 1,   1,   1,   0,   1,   0 }, /* TDM_A2 L1TXD            */
	/* PD21 */ { 1,   1,   1,   0,   1,   0 }, /* TDM_A2 L1RXD            */
	/* PD20 */ { 1,   1,   1,   0,   0,   0 }, /* TDM_A2 L1RSYNC          */
	/* PD19 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1196           */
	/* PD18 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1197           */
	/* PD17 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1198           */
	/* PD16 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1199           */
	/* PD15 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1250           */
	/* PD14 */ { 1,   0,   0,   1,   0,   0 }, /* GPO    TP1251           */
	/* PD13 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD12 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD11 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD10 */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD9  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD8  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD7  */ { 0,   0,   0,   1,   0,   0 }, /* GPO    FL_BYTE          */
	/* PD6  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD5  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD4  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD3  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD2  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD1  */ { 0,   0,   0,   0,   0,   0 }, /* pin does not exist      */
	/* PD0  */ { 0,   0,   0,   0,   0,   0 }  /* pin does not exist      */
    }
};

void reset_phy (void)
{
	volatile ioport_t *iop;
#if defined(CONFIG_CMD_NET)
	int i;
	unsigned short val;
#endif

	iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, 0);

	/* Reset the PHY */
	iop->pdat &= 0xfff7ffff;	/* PA12 = |SWITCH_RESET */
#if defined(CONFIG_CMD_NET)
	udelay(20000);
	iop->pdat |= 0x00080000;
	for (i=0; i<100; i++) {
		udelay(20000);
		if (bb_miiphy_read("FCC1 ETHERNET", CONFIG_SYS_PHY_ADDR,2,&val ) == 0) {
			break;
		}
	}
	/* initialize switch */
	m88e6060_initialize( CONFIG_SYS_PHY_ADDR );
#endif
}

static unsigned long UPMATable[] = {
	0x8fffec00,  0x0ffcfc00,  0x0ffcfc00,  0x0ffcfc00, /* Words 0 to 3	*/
	0x0ffcfc04,  0x3ffdfc00,  0xfffffc01,  0xfffffc01, /* Words 4 to 7	*/
	0xfffffc00,  0xfffffc04,  0xfffffc01,  0xfffffc00, /* Words 8 to 11	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 12 to 15	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 16 to 19	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 20 to 23	*/
	0x8fffec00,  0x00fffc00,  0x00fffc00,  0x00fffc00, /* Words 24 to 27	*/
	0x0ffffc04,  0xfffffc01,  0xfffffc01,  0xfffffc01, /* Words 28 to 31	*/
	0xfffffc00,  0xfffffc01,  0xfffffc01,  0xfffffc00, /* Words 32 to 35	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 36 to 39	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 40 to 43	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 44 to 47	*/
	0xfffffc00,  0xfffffc04,  0xfffffc01,  0xfffffc00, /* Words 48 to 51	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, /* Words 52 to 55	*/
	0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, /* Words 56 to 59	*/
	0xffffec00,  0xffffec04,  0xffffec00,  0xfffffc01  /* Words 60 to 63	*/
};

int board_early_init_f (void)
{
	volatile immap_t *immap;
	volatile memctl8260_t *memctl;
	volatile unsigned char *dummy;
	int i;

	immap = (immap_t *) CONFIG_SYS_IMMR;
	memctl = &immap->im_memctl;

#if 0
	/* CS2-5 - DSP via UPMA */
	dummy = (volatile unsigned char *) (memctl->memc_br2 & BRx_BA_MSK);
	memctl->memc_mar = 0;
	memctl->memc_mamr = MxMR_OP_WARR;
	for (i = 0; i < 64; i++) {
		memctl->memc_mdr = UPMATable[i];
		*dummy = 0;
	}
	memctl->memc_mamr = 0x00044440;
#else
	/* CS7 - DPRAM via UPMA */
	dummy = (volatile unsigned char *) (memctl->memc_br7 & BRx_BA_MSK);
	memctl->memc_mar = 0;
	memctl->memc_mamr = MxMR_OP_WARR;
	for (i = 0; i < 64; i++) {
		memctl->memc_mdr = UPMATable[i];
		*dummy = 0;
	}
	memctl->memc_mamr = 0x00044440;
#endif
	return 0;
}

int misc_init_r (void)
{
	volatile ioport_t *iop;
	unsigned char temp;
#if 0
	/* DUMP UPMA RAM */
	volatile immap_t *immap;
	volatile memctl8260_t *memctl;
	volatile unsigned char *dummy;
	unsigned char c;
	int i;

	immap = (immap_t *) CONFIG_SYS_IMMR;
	memctl = &immap->im_memctl;


	dummy = (volatile unsigned char *) (memctl->memc_br7 & BRx_BA_MSK);
	memctl->memc_mar = 0;
	memctl->memc_mamr = MxMR_OP_RARR;
	for (i = 0; i < 64; i++) {
		c = *dummy;
		printf( "UPMA[%02d]: 0x%08lx,0x%08lx: 0x%08lx\n",i,
		        memctl->memc_mamr,
		        memctl->memc_mar,
		        memctl->memc_mdr );
	}
	memctl->memc_mamr = 0x00044440;
#endif
	/* enable buffers (DSP, DPRAM) */
	iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, 0);
	iop->pdat &= 0xfffbffff;	/* PA13 = |EN_M_BCTL1 */

	/* destroy DPRAM magic */
	*(volatile unsigned char *)0xf0500000 = 0x00;

	/* clear any pending DPRAM irq */
	temp = *(volatile unsigned char *)0xf05003ff;

	/* write module-id into DPRAM */
	*(volatile unsigned char *)0xf0500201 = 0x50;

	return 0;
}

#if defined(CONFIG_HAVE_OWN_RESET)
int
do_reset (void *cmdtp, int flag, int argc, char *argv[])
{
	volatile ioport_t *iop;

	iop = ioport_addr((immap_t *)CONFIG_SYS_IMMR, 2);
	iop->pdat |= 0x00002000;	/* PC18 = HW_RESET */
	return 1;
}
#endif	/* CONFIG_HAVE_OWN_RESET */

#define ns2clk(ns) (ns / (1000000000 / CONFIG_8260_CLKIN) + 1)

phys_size_t initdram (int board_type)
{
#ifndef CONFIG_SYS_RAMBOOT
	volatile immap_t *immap;
	volatile memctl8260_t *memctl;
	volatile uchar *ramaddr;
	int i;
	uchar c;

	immap = (immap_t *) CONFIG_SYS_IMMR;
	memctl = &immap->im_memctl;
	ramaddr = (uchar *) CONFIG_SYS_SDRAM_BASE;
	c = 0xff;

	immap->im_siu_conf.sc_ppc_acr  = 0x02;
	immap->im_siu_conf.sc_ppc_alrh = 0x01267893;
	immap->im_siu_conf.sc_ppc_alrl = 0x89abcdef;
	immap->im_siu_conf.sc_tescr1   = 0x00000000;
	immap->im_siu_conf.sc_tescr2   = 0x00000000;

	memctl->memc_mptpr = CONFIG_SYS_MPTPR;
	memctl->memc_psrt = CONFIG_SYS_PSRT;
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_SDRAM_BASE | CONFIG_SYS_BR1_PRELIM;

	/* Precharge all banks */
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | 0x28000000;
	*ramaddr = c;

	/* CBR refresh */
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | 0x08000000;
	for (i = 0; i < 8; i++)
		*ramaddr = c;

	/* Mode Register write */
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | 0x18000000;
	*ramaddr = c;

	/* Refresh enable */
	memctl->memc_psdmr = CONFIG_SYS_PSDMR | 0x40000000;
	*ramaddr = c;
#endif /* CONFIG_SYS_RAMBOOT */

	return (CONFIG_SYS_SDRAM_SIZE);
}

int checkboard (void)
{
#ifdef CONFIG_CLKIN_66MHz
	puts ("Board: Elmeg VoVPN Gateway Module (66MHz)\n");
#else
	puts ("Board: Elmeg VoVPN Gateway Module (100MHz)\n");
#endif
	return 0;
}
