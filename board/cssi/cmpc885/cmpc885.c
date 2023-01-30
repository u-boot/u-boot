// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010-2020 CS Group
 * Charles Frey <charles.frey@c-s.fr>
 * Florent Trinh Thai <florent.trinh-thai@c-s.fr>
 * Christophe Leroy <christophe.leroy@c-s.fr>
 *
 * Board specific routines for the CMPC885 board
 */

#include <env.h>
#include <common.h>
#include <mpc8xx.h>
#include <asm/io.h>
#include <dm.h>
#include <stdio.h>
#include <stdarg.h>
#include <watchdog.h>
#include <serial.h>
#include <hang.h>
#include <flash.h>
#include <init.h>
#include <fdt_support.h>
#include <linux/delay.h>

#include <spi.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOARD_CMPC885		"cmpc885"
#define BOARD_MCR3000_2G	"mcr3k_2g"
#define BOARD_VGOIP		"vgoip"
#define BOARD_MIAE		"miae"

#define TYPE_MCR	0x22
#define TYPE_MIAE	0x23

#define FAR_CASRSA     2
#define FAR_VGOIP      4
#define FAV_CLA        7
#define FAV_SRSA       8

#define ADDR_CPLD_R_RESET		((unsigned short __iomem *)CONFIG_CPLD_BASE)
#define ADDR_CPLD_R_ETAT		((unsigned short __iomem *)(CONFIG_CPLD_BASE + 2))
#define ADDR_CPLD_R_TYPE		((unsigned char  __iomem *)(CONFIG_CPLD_BASE + 3))

#define ADDR_FPGA_R_BASE		((unsigned char  __iomem *)CONFIG_FPGA_BASE)
#define ADDR_FPGA_R_ALARMES_IN		((unsigned char  __iomem *)CONFIG_FPGA_BASE + 0x31)
#define ADDR_FPGA_R_FAV			((unsigned char  __iomem *)CONFIG_FPGA_BASE + 0x44)

#define PATH_PHY2			"/soc@ff000000/mdio@e00/ethernet-phy@2"
#define PATH_PHY3			"/soc@ff000000/mdio@e00/ethernet-phy@3"
#define PATH_ETH1			"/soc@ff000000/ethernet@1e00"
#define FIBER_PHY PATH_PHY2

#define FPGA_R_ACQ_AL_FAV	0x04
#define R_ETAT_PRES_BASE	0x0040

#define R_RESET_STATUS		0x0400
#define R_RST_STATUS		0x0004

static int fdt_set_node_and_value(void *blob, char *node, const char *prop,
				  void *var, int size)
{
	int ret, off;

	off = fdt_path_offset(blob, node);

	if (off < 0) {
		printf("Cannot find %s node err:%s\n", node, fdt_strerror(off));

		return off;
	}

	ret = fdt_setprop(blob, off, prop, var, size);

	if (ret < 0)
		printf("Cannot set %s/%s prop err: %s\n", node, prop, fdt_strerror(ret));

	return ret;
}

/* Checks front/rear id and remove unneeded nodes from the blob */
static void ft_cleanup(void *blob, uint32_t id, const char *prop, const char *compatible)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, -1, compatible);

	while (off != -FDT_ERR_NOTFOUND) {
		const struct fdt_property *ids;
		int nb_ids, idx;
		int tmp = -1;

		ids = fdt_get_property(blob, off, prop, &nb_ids);

		for (idx = 0; idx < nb_ids; idx += 4) {
			if (*((uint32_t *)&ids->data[idx]) == id)
				break;
		}

		if (idx >= nb_ids)
			fdt_del_node(blob, off);
		else
			tmp = off;

		off = fdt_node_offset_by_compatible(blob, tmp, compatible);
	}

	fdt_set_node_and_value(blob, "/", prop, &id, sizeof(uint32_t));
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	u8 fav_id, far_id;

	const char *sync = "receive";

	ft_cpu_setup(blob, bd);

	/* BRG */
	do_fixup_by_path_u32(blob, "/soc/cpm", "brg-frequency", bd->bi_busfreq, 1);

	/* MAC addr */
	fdt_fixup_ethernet(blob);

	/* Bus Frequency for CPM */
	do_fixup_by_path_u32(blob, "/soc", "bus-frequency", bd->bi_busfreq, 1);

	/* E1 interface - Set data rate */
	do_fixup_by_path_u32(blob, "/localbus/e1", "data-rate", 2, 1);

	/* E1 interface - Set channel phase to 0 */
	do_fixup_by_path_u32(blob, "/localbus/e1", "channel-phase", 0, 1);

	/* E1 interface - rising edge sync pulse transmit */
	do_fixup_by_path(blob, "/localbus/e1", "rising-edge-sync-pulse", sync, strlen(sync), 1);

	/* MIAE only */
	if (!(in_be16(ADDR_CPLD_R_ETAT) & R_ETAT_PRES_BASE) || in_8(ADDR_FPGA_R_BASE) != TYPE_MIAE)
		return 0;

	far_id = in_8(ADDR_FPGA_R_BASE + 0x43) >> 5;
	ft_cleanup(blob, (u32)far_id, "far-id", "cs,mia-far");

	/*
	 * special case, with CASRSA (far_id: 2)
	 * FAV-SRSA register itself as FAV-CLA
	 */
	fav_id = in_8(ADDR_FPGA_R_BASE + 0x44) >> 5;

	if (far_id == FAR_CASRSA && fav_id == FAV_CLA)
		fav_id = FAV_SRSA;

	ft_cleanup(blob, (u32)fav_id, "fav-id", "cs,mia-fav");

	if (far_id == FAR_CASRSA) {
		/* switch to phy3 with gpio, we'll only use phy3 */
		immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
		cpm8xx_t __iomem *cp = (cpm8xx_t __iomem *)&immr->im_cpm;

		setbits_be32(&cp->cp_pedat, 0x00002000);
	}

	return 0;
}

int checkboard(void)
{
	serial_puts("Board: ");

	/* Is a motherboard present ? */
	if (in_be16(ADDR_CPLD_R_ETAT) & R_ETAT_PRES_BASE) {
		switch (in_8(ADDR_FPGA_R_BASE)) {
			int far_id;
		case TYPE_MCR:
			printf("MCR3000_2G (CS GROUP)\n");
			break;
		case TYPE_MIAE:
			far_id = in_8(ADDR_FPGA_R_BASE + 0x43) >> 5;

			if (far_id == FAR_VGOIP)
				printf("VGoIP (CS GROUP)\n");
			else
				printf("MIAE (CS GROUP)\n");

			break;
		default:
			printf("Unknown\n");
			for (;;)
				;
			break;
		}
	} else {
		printf("CMPC885 (CS GROUP)\n");
	}
	return 0;
}

#define SPI_EEPROM_READ	0x03
#define MAX_SPI_BYTES	0x20

#define EE_OFF_MAC1	0x13
#define EE_OFF_MAC2	0x19

/* Reads MAC addresses from SPI EEPROM */
static int setup_mac(void)
{
	struct udevice *eeprom;
	struct spi_slave *slave;
	char name[30], *str;
	uchar din[MAX_SPI_BYTES];
	uchar dout[MAX_SPI_BYTES] = {SPI_EEPROM_READ, 0, 0};
	int bitlen = 256, cs = 0, mode = 0, bus = 0, ret;
	unsigned long ident = 0x08005120;

	snprintf(name, sizeof(name), "generic_%d:%d", bus, cs);

	str = strdup(name);
	if (!str)
		return -1;

	ret = uclass_get_device(UCLASS_SPI, 0, &eeprom);
	if (ret) {
		printf("Could not enable Serial Peripheral Interface (SPI).\n");
		return -1;
	}

	ret = _spi_get_bus_and_cs(bus, cs, 1000000, mode, "spi_generic_drv", str, &eeprom, &slave);
	if (ret)
		return ret;

	ret = spi_claim_bus(slave);

	ret = spi_xfer(slave, bitlen, dout, din, SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret) {
		printf("Error %d during SPI transaction\n", ret);
		return ret;
	}

	if (memcmp(din + EE_OFF_MAC1, &ident, sizeof(ident)) == 0)
		eth_env_set_enetaddr("ethaddr", din + EE_OFF_MAC1);

	if (memcmp(din + EE_OFF_MAC2, &ident, sizeof(ident)) == 0)
		eth_env_set_enetaddr("eth1addr", din + EE_OFF_MAC2);

	spi_release_bus(slave);

	return 0;
}

int misc_init_r(void)
{
	u8 val, tmp, far_id;
	int count = 3;

	val = in_8(ADDR_FPGA_R_BASE);

	/* Verify mother board presence */
	if (in_be16(ADDR_CPLD_R_ETAT) & R_ETAT_PRES_BASE) {
		/* identify the type of mother board */
		switch (val) {
		case TYPE_MCR:
			/* if at boot alarm button is pressed, delay boot */
			if ((in_8(ADDR_FPGA_R_ALARMES_IN) & FPGA_R_ACQ_AL_FAV) == 0)
				env_set("bootdelay", "60");

			env_set("config", BOARD_MCR3000_2G);
			env_set("hostname", BOARD_MCR3000_2G);
			break;

		case TYPE_MIAE:
			do {
				tmp = in_8(ADDR_FPGA_R_BASE + 0x41);
				count--;
				mdelay(10); /* 10msec wait */
			} while (count && tmp != in_8(ADDR_FPGA_R_BASE + 0x41));

			if (!count) {
				printf("Cannot read the reset factory switch position\n");
				hang();
			}

			if (tmp & 0x1)
				env_set_default("Factory settings switch ON", 0);

			env_set("config", BOARD_MIAE);
			far_id = in_8(ADDR_FPGA_R_BASE + 0x43) >> 5;

			if (far_id == FAR_VGOIP)
				env_set("hostname", BOARD_VGOIP);
			else
				env_set("hostname", BOARD_MIAE);
			break;

		default:
			env_set("config", BOARD_CMPC885);
			env_set("hostname", BOARD_CMPC885);
			break;
		}
	} else {
		printf("no mother board detected");
		env_set("config", BOARD_CMPC885);
		env_set("hostname", BOARD_CMPC885);
	}

	if (setup_mac())
		printf("Error retrieving mac addresses\n");

	/* Protection ON by default */
	flash_protect(FLAG_PROTECT_SET, CFG_SYS_FLASH_BASE, 0xffffffff, &flash_info[0]);

	return 0;
}

static void iop_setup_mcr(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	iop8xx_t __iomem *iop = &immr->im_ioport;
	cpm8xx_t __iomem *cp = &immr->im_cpm;

	/* Wait reset on FPGA_F */
	udelay(100);

	/* We must initialize data before changing direction */
	setbits_be16(&iop->iop_pcdat, 0x088E);
	setbits_be16(&iop->iop_pddat, 0x0001);
	setbits_be32(&cp->cp_pbdat, 0x00029510);
	setbits_be32(&cp->cp_pedat, 0x00000002);

	/*
	 * PAPAR[13] = 0 [0x0004] -> GPIO: ()
	 * PAPAR[12] = 0 [0x0008] -> GPIO: ()
	 * PAPAR[9]  = 1 [0x0040] -> GPIO: (PCM_IN_12_MPC)
	 * PAPAR[8]  = 1 [0x0080] -> GPIO: (PCM_OUT_12_MPC)
	 * PAPAR[7]  = 1 [0x0100] -> GPIO: (TDM_BCLK_MPC)
	 * PAPAR[6]  = 1 [0x0200] -> GPIO: (CLK2)
	 */
	clrsetbits_be16(&iop->iop_papar, 0x03CC, 0x03C0);

	/*
	 * PBODR[16] = 1 [0x00008000] -> GPIO: (PROG_FPGA_MEZZ)
	 */
	setbits_be16(&cp->cp_pbodr, 0x00008000);

	/*
	 * PBDIR[27] = 1 [0x00000010] -> GPIO: (WR_TEMP2)
	 * PBDIR[26] = 1 [0x00000020] -> GPIO: (BRG02)
	 * PBDIR[23] = 1 [0x00000100] -> GPIO: (CS_TEMP2)
	 * PBDIR[18] = 1 [0x00002000] -> GPIO: (RTS2)
	 * PBDIR[16] = 1 [0x00008000] -> GPIO: (PROG_FPGA_MEZZ)
	 * PBDIR[15] = 1 [0x00010000] -> GPIO: (BRG03)
	 * PBDIR[14] = 1 [0x00020000] -> GPIO: (CS_TEMP)
	 */
	setbits_be32(&cp->cp_pbdir, 0x0003A130);

	/*
	 * PBPAR[20] = 1 [0x00000800] -> GPIO: (SMRXD2)
	 * PBPAR[17] = 0 [0x00004000] -> GPIO: (DONE_FPGA_MEZZ)
	 * PBPAR[16] = 0 [0x00008000] -> GPIO: (PROG_FPGA_MEZZ)
	 */
	clrsetbits_be32(&cp->cp_pbpar, 0x0000C800, 0x00000800);

	/*
	 * PCPAR[14] = 0 [0x0002] -> GPIO: (CS_POT2)
	 */
	clrbits_be16(&iop->iop_pcpar, 0x0002);

	/*
	 * PDPAR[14] = 1 [0x0002] -> GPIO: (TDM_FS_MPC)
	 * PDPAR[11] = 1 [0x0010] -> GPIO: (RXD3)
	 * PDPAR[10] = 1 [0x0020] -> GPIO: (TXD3)
	 * PDPAR[9]  = 1 [0x0040] -> GPIO: (TXD4)
	 * PDPAR[7]  = 1 [0x0100] -> GPIO: (RTS3)
	 * PDPAR[5]  = 1 [0x0400] -> GPIO: (CLK8)
	 * PDPAR[3]  = 1 [0x1000] -> GPIO: (CLK7)
	 */
	setbits_be16(&iop->iop_pdpar, 0x1572);

	/*
	 * PEPAR[27] = 1 [0x00000010] -> GPIO: (R2_RXER)
	 * PEPAR[26] = 1 [0x00000020] -> GPIO: (R2_CRS_DV)
	 * PEPAR[25] = 1 [0x00000040] -> GPIO: (RXD4)
	 * PEPAR[24] = 1 [0x00000080] -> GPIO: (BRG01)
	 * PEPAR[23] = 0 [0x00000100] -> GPIO: (DONE_FPGA_RADIO)
	 * PEPAR[22] = 1 [0x00000200] -> GPIO: (R2_RXD1)
	 * PEPAR[21] = 1 [0x00000400] -> GPIO: (R2_RXD0)
	 * PEPAR[20] = 1 [0x00000800] -> GPIO: (SMTXD2)
	 * PEPAR[19] = 1 [0x00001000] -> GPIO: (R2_TXEN)
	 * PEPAR[17] = 1 [0x00004000] -> GPIO: (CLK5)
	 * PEPAR[16] = 1 [0x00008000] -> GPIO: (R2_REF_CLK)
	 * PEPAR[15] = 1 [0x00010000] -> GPIO: (R2_TXD1)
	 * PEPAR[14] = 1 [0x00020000] -> GPIO: (R2_TXD0)
	 */
	clrsetbits_be32(&cp->cp_pepar, 0x0003DFF0, 0x0003DEF0);

	/*
	 * PADIR[9]  = 1 [0x0040] -> GPIO: (PCM_IN_12_MPC)
	 * PADIR[8]  = 1 [0x0080] -> GPIO: (PCM_OUT_12_MPC)
	 * PADIR[5]  = 1 [0x0400] -> GPIO: ()
	 */
	setbits_be16(&iop->iop_padir, 0x04C0);

	/*
	 * PCDIR[15] = 1 [0x0001] -> GPIO: (WP_EEPROM2)
	 * PCDIR[14] = 1 [0x0002] -> GPIO: (CS_POT2)
	 * PCDIR[13] = 1 [0x0004] -> GPIO: (CS_POT1)
	 * PCDIR[12] = 1 [0x0008] -> GPIO: (CS_EEPROM2)
	 * PCDIR[8]  = 1 [0x0080] -> GPIO: (CS_CODEC_FAV)
	 * PCDIR[4]  = 1 [0x0800] -> GPIO: (CS_CODEC_RADIO)
	 */
	setbits_be16(&iop->iop_pcdir, 0x088F);

	/*
	 * PDDIR[9]  = 1 [0x0040] -> GPIO: (TXD4)
	 * PDDIR[6]  = 0 [0x0200] -> GPIO: (INIT_FPGA_RADIO)
	 * PDDIR[2]  = x [0x2000] -> Reserved
	 * PDDIR[1]  = 0 [0x4000] -> ODR for PD10 : (TXD3)
	 * PDDIR[0]  = 0 [0x8000] -> ODR for PD8  : (R_MDC)
	 */
	clrsetbits_be16(&iop->iop_pddir, 0xC240, 0x0040);

	/*
	 * PEDIR[30] = 1 [0x00000002] -> GPIO: (FPGA_FIRMWARE)
	 * PEDIR[27] = 1 [0x00000010] -> GPIO: (R2_RXER)
	 * PEDIR[26] = 1 [0x00000020] -> GPIO: (R2_CRS_DV)
	 * PEDIR[23] = 0 [0x00000100] -> GPIO: (DONE_FPGA_RADIO)
	 * PEDIR[22] = 1 [0x00000200] -> GPIO: (R2_RXD1)
	 * PEDIR[21] = 1 [0x00000400] -> GPIO: (R2_RXD0)
	 * PEDIR[19] = 1 [0x00001000] -> GPIO: (R2_TXEN)
	 * PEDIR[18] = 1 [0x00002000] -> GPIO: (PROG_FPGA_RADIO)
	 * PEDIR[16] = 1 [0x00008000] -> GPIO: (R2_REF_CLK)
	 * PEDIR[15] = 1 [0x00010000] -> GPIO: (R2_TXD1)
	 * PEDIR[14] = 1 [0x00020000] -> GPIO: (R2_TXD0)
	 */
	clrsetbits_be32(&cp->cp_pedir, 0x0003B732, 0x0003B632);

	/*
	 * PAODR[10] = 1 [0x0020] -> GPIO: (INIT_FPGA_F)
	 */
	setbits_be16(&iop->iop_paodr, 0x0020); // set_bit

	/*
	 * PEODR[30] = 1 [0x00000002] -> GPIO: (FPGA_FIRMWARE)
	 * PEODR[18] = 1 [0x00002000] -> GPIO: (FPGA_RADIO)
	 */
	setbits_be32(&cp->cp_peodr, 0x00002002);

	/*
	 * PESO[24] = 1 [0x00000080] -> GPIO: (BRG01)
	 * PESO[23] = 0 [0x00000100] -> GPIO: (DONE_FPGA_RADIO)
	 * PESO[20] = 1 [0x00000800] -> GPIO: (SMTXD2)
	 * PESO[19] = 1 [0x00001000] -> GPIO: (R2_TXEN)
	 * PESO[15] = 1 [0x00010000] -> GPIO: (R2_TXD1)
	 * PESO[14] = 1 [0x00020000] -> GPIO: (R2_TXD0)
	 */
	clrsetbits_be32(&cp->cp_peso, 0x00031980, 0x00031880);

	/* Disable CS for device */
	/* PROGFPGA down */
	clrbits_be32(&cp->cp_pbdat, 0x00008000);

	/* PROGFPGA down */
	clrbits_be32(&cp->cp_pedat, 0x00002000);
	udelay(1);	/* Wait more than 300ns */

	/*
	 * We do not set the PROG signal of the C4E1 because
	 * there is a conflic with the CS of the EEPROM.
	 * I don't know why there is not the same problem
	 * with the FPGA_R
	 */

	/* PROGFPGA up	*/
	setbits_be32(&cp->cp_pedat, 0x00002000);
}

static void iop_setup_cmpc885(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	iop8xx_t __iomem *iop = &immr->im_ioport;
	cpm8xx_t __iomem *cp = &immr->im_cpm;

	/* We must initialize data before changing direction */
	out_be16(&iop->iop_pcdat, 0x0000);
	out_be16(&iop->iop_pddat, 0x0001);

	out_be32(&cp->cp_pbdat, 0x00021400);
	out_be32(&cp->cp_pedat, 0x00000000);

	/*
	 * PAPAR[13] = 0 [0x0004] -> GPIO: ()
	 * PAPAR[12] = 0 [0x0008] -> GPIO: ()
	 * PAPAR[9]  = 0 [0x0040] -> GPIO: ()
	 * PAPAR[8]  = 0 [0x0080] -> GPIO: ()
	 * PAPAR[7]  = 0 [0x0100] -> GPIO: ()
	 * PAPAR[6]  = 0 [0x0200] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_papar, 0x03CC);

	/*
	 * PBPAR[20] = 0 [0x00000800] -> GPIO: ()
	 * PBPAR[17] = 0 [0x00004000] -> GPIO: ()
	 * PBPAR[16] = 0 [0x00008000] -> GPIO: ()
	 */
	clrbits_be32(&cp->cp_pbpar, 0x0000C800);

	/*
	 * PCPAR[14] = 0 [0x0002] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_pcpar, 0x0002);

	/*
	 * PDPAR[14] = 0 [0x0002] -> GPIO: ()
	 * PDPAR[11] = 0 [0x0010] -> GPIO: ()
	 * PDPAR[10] = 0 [0x0020] -> GPIO: ()
	 * PDPAR[9]  = 0 [0x0040] -> GPIO: ()
	 * PDPAR[7]  = 0 [0x0100] -> GPIO: ()
	 * PDPAR[5]  = 0 [0x0400] -> GPIO: ()
	 * PDPAR[3]  = 0 [0x1000] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_pdpar, 0x1572);

	/*
	 * PEPAR[27] = 0 [0x00000010] -> GPIO: ()
	 * PEPAR[26] = 0 [0x00000020] -> GPIO: ()
	 * PEPAR[25] = 0 [0x00000040] -> GPIO: ()
	 * PEPAR[24] = 0 [0x00000080] -> GPIO: ()
	 * PEPAR[23] = 0 [0x00000100] -> GPIO: ()
	 * PEPAR[22] = 0 [0x00000200] -> GPIO: ()
	 * PEPAR[21] = 0 [0x00000400] -> GPIO: ()
	 * PEPAR[20] = 0 [0x00000800] -> GPIO: ()
	 * PEPAR[19] = 0 [0x00001000] -> GPIO: ()
	 * PEPAR[17] = 0 [0x00004000] -> GPIO: ()
	 * PEPAR[16] = 0 [0x00008000] -> GPIO: ()
	 * PEPAR[15] = 0 [0x00010000] -> GPIO: ()
	 * PEPAR[14] = 0 [0x00020000] -> GPIO: ()
	 */
	clrbits_be32(&cp->cp_pepar, 0x0003DFF0);

	/*
	 * PADIR[9]  = 0 [0x0040] -> GPIO: ()
	 * PADIR[8]  = 0 [0x0080] -> GPIO: ()
	 * PADIR[5]  = 0 [0x0400] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_padir, 0x04C0);

	/*
	 * In/Out or per. Function 0/1
	 * PBDIR[27] = 0 [0x00000010] -> GPIO: ()
	 * PBDIR[26] = 0 [0x00000020] -> GPIO: ()
	 * PBDIR[23] = 0 [0x00000100] -> GPIO: ()
	 * PBDIR[17] = 0 [0x00004000] -> GPIO: ()
	 * PBDIR[16] = 0 [0x00008000] -> GPIO: ()
	 */
	clrbits_be32(&cp->cp_pbdir, 0x0000C130);

	/*
	 * PCDIR[15] = 0 [0x0001] -> GPIO: ()
	 * PCDIR[14] = 0 [0x0002] -> GPIO: ()
	 * PCDIR[13] = 0 [0x0004] -> GPIO: ()
	 * PCDIR[12] = 0 [0x0008] -> GPIO: ()
	 * PCDIR[8]  = 0 [0x0080] -> GPIO: ()
	 * PCDIR[4]  = 0 [0x0800] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_pcdir, 0x088F);

	/*
	 * PDDIR[9]  = 0 [0x0040] -> GPIO: ()
	 * PDDIR[6]  = 0 [0x0200] -> GPIO: ()
	 * PDDIR[2]  = x [0x2000] -> Reserved
	 * PDDIR[1]  = 0 [0x4000] -> ODR for PD10 : ()
	 * PDDIR[0]  = 0 [0x8000] -> ODR for PD8  : (R_MDC)
	 */
	clrbits_be16(&iop->iop_pddir, 0xC240);

	/*
	 * PEDIR[30] = 0 [0x00000002] -> GPIO: ()
	 * PEDIR[27] = 0 [0x00000010] -> GPIO: ()
	 * PEDIR[26] = 0 [0x00000020] -> GPIO: ()
	 * PEDIR[23] = 0 [0x00000100] -> GPIO: ()
	 * PEDIR[22] = 0 [0x00000200] -> GPIO: ()
	 * PEDIR[21] = 0 [0x00000400] -> GPIO: ()
	 * PEDIR[19] = 0 [0x00001000] -> GPIO: ()
	 * PEDIR[18] = 0 [0x00002000] -> GPIO: ()
	 * PEDIR[16] = 0 [0x00008000] -> GPIO: ()
	 * PEDIR[15] = 0 [0x00010000] -> GPIO: ()
	 * PEDIR[14] = 0 [0x00020000] -> GPIO: ()
	 */
	clrbits_be32(&cp->cp_pedir, 0x0003B732);

	/*
	 * PAODR[10] = 0 [0x0020] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_paodr, 0x0020);

	/*
	 * PBODR[16] = 0 [0x00008000] -> GPIO: ()
	 */
	clrbits_be16(&cp->cp_pbodr, 0x00008000);

	/*
	 * PEODR[30] = 0 [0x00000002] -> GPIO: ()
	 * PEODR[18] = 0 [0x00002000] -> GPIO: ()
	 */
	clrbits_be32(&cp->cp_peodr, 0x00002002);

	/*
	 * PESO[24] = 0 [0x00000080] -> GPIO: ()
	 * PESO[23] = 0 [0x00000100] -> GPIO: ()
	 * PESO[20] = 0 [0x00000800] -> GPIO: ()
	 * PESO[19] = 0 [0x00001000] -> GPIO: ()
	 * PESO[15] = 0 [0x00010000] -> GPIO: ()
	 * PESO[14] = 0 [0x00020000] -> GPIO: ()
	 */
	clrbits_be32(&cp->cp_peso, 0x00031980);
}

static void iop_setup_miae(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	iop8xx_t __iomem *iop = &immr->im_ioport;
	cpm8xx_t __iomem *cp = &immr->im_cpm;

	/* Wait reset on FPGA_F */
	udelay(100);

	/* Set the front panel LED color to red */
	clrbits_8(ADDR_FPGA_R_FAV, 0x02);

	/* We must initialize data before changing direction */
	setbits_be16(&iop->iop_pcdat, 0x0888);
	setbits_be16(&iop->iop_pddat, 0x0201);
	setbits_be32(&cp->cp_pbdat, 0x00021510);
	setbits_be32(&cp->cp_pedat, 0x00000002);

	/*
	 * PAPAR[13] = 1 [0x0004] -> GPIO: (RXD2)
	 * PAPAR[12] = 1 [0x0008] -> GPIO: (TXD2)
	 * PAPAR[9]  = 1 [0x0040] -> GPIO: (TDM1O)
	 * PAPAR[8]  = 1 [0x0080] -> GPIO: (TDM1I)
	 * PAPAR[7]  = 1 [0x0100] -> GPIO: (TDM_BCLK_MPC)
	 * PAPAR[6]  = 1 [0x0200] -> GPIO: (CLK2)
	 */
	setbits_be16(&iop->iop_papar, 0x03CC);

	/*
	 * PBODR[16] = 0 [0x00008000] -> GPIO: (L1ST4)
	 */
	clrbits_be16(&cp->cp_pbodr, 0x00008000);

	/*
	 * PBDIR[27] = 1 [0x00000010] -> GPIO: (WR_TEMP2)
	 * PBDIR[26] = 1 [0x00000020] -> GPIO: (BRG02)
	 * PBDIR[23] = 1 [0x00000100] -> GPIO: (CS_TEMP2)
	 * PBDIR[18] = 1 [0x00002000] -> GPIO: (RTS2)
	 * PBDIR[16] = 0 [0x00008000] -> GPIO: (L1ST4)
	 * PBDIR[15] = 1 [0x00010000] -> GPIO: (BRG03)
	 * PBDIR[14] = 1 [0x00020000] -> GPIO: (CS_TEMP)
	 */
	clrsetbits_be32(&cp->cp_pbdir, 0x0003A130, 0x00032130);

	/*
	 * PBPAR[20] = 1 [0x00000800] -> GPIO: (SMRXD2)
	 * PBPAR[17] = 1 [0x00004000] -> GPIO: (L1ST3)
	 * PBPAR[16] = 1 [0x00008000] -> GPIO: (L1ST4)
	 */
	setbits_be32(&cp->cp_pbpar, 0x0000C800);

	/*
	 * PCPAR[14] = 1 [0x0002] -> GPIO: (L1ST2)
	 */
	setbits_be16(&iop->iop_pcpar, 0x0002);

	/*
	 * PDPAR[14] = 1 [0x0002] -> GPIO: (TDM_FS_MPC)
	 * PDPAR[11] = 1 [0x0010] -> GPIO: (RXD3)
	 * PDPAR[10] = 1 [0x0020] -> GPIO: (TXD3)
	 * PDPAR[9]  = 1 [0x0040] -> GPIO: (TXD4)
	 * PDPAR[7]  = 1 [0x0100] -> GPIO: (RTS3)
	 * PDPAR[5]  = 1 [0x0400] -> GPIO: (CLK8)
	 * PDPAR[3]  = 1 [0x1000] -> GPIO: (CLK7)
	 */
	setbits_be16(&iop->iop_pdpar, 0x1572);

	/*
	 * PEPAR[27] = 1 [0x00000010] -> GPIO: (R2_RXER)
	 * PEPAR[26] = 1 [0x00000020] -> GPIO: (R2_CRS_DV)
	 * PEPAR[25] = 1 [0x00000040] -> GPIO: (RXD4)
	 * PEPAR[24] = 1 [0x00000080] -> GPIO: (BRG01)
	 * PEPAR[23] = 1 [0x00000100] -> GPIO: (L1ST1)
	 * PEPAR[22] = 1 [0x00000200] -> GPIO: (R2_RXD1)
	 * PEPAR[21] = 1 [0x00000400] -> GPIO: (R2_RXD0)
	 * PEPAR[20] = 1 [0x00000800] -> GPIO: (SMTXD2)
	 * PEPAR[19] = 1 [0x00001000] -> GPIO: (R2_TXEN)
	 * PEPAR[17] = 1 [0x00004000] -> GPIO: (CLK5)
	 * PEPAR[16] = 1 [0x00008000] -> GPIO: (R2_REF_CLK)
	 * PEPAR[15] = 1 [0x00010000] -> GPIO: (R2_TXD1)
	 * PEPAR[14] = 1 [0x00020000] -> GPIO: (R2_TXD0)
	 */
	setbits_be32(&cp->cp_pepar, 0x0003DFF0);

	/*
	 * PADIR[9]  = 1 [0x0040] -> GPIO: (TDM1O)
	 * PADIR[8]  = 1 [0x0080] -> GPIO: (TDM1I)
	 * PADIR[5]  = 0 [0x0400] -> GPIO: ()
	 */
	clrsetbits_be16(&iop->iop_padir, 0x04C0, 0x00C0);

	/*
	 * PCDIR[15] = 1 [0x0001] -> GPIO: (WP_EEPROM2)
	 * PCDIR[14] = 1 [0x0002] -> GPIO: (L1ST2)
	 * PCDIR[13] = 0 [0x0004] -> GPIO: ()
	 * PCDIR[12] = 1 [0x0008] -> GPIO: (CS_EEPROM2)
	 * PCDIR[8]  = 1 [0x0080] -> GPIO: (CS_CODEC_2)
	 * PCDIR[4]  = 1 [0x0800] -> GPIO: (CS_CODEC_1)
	 */
	clrsetbits_be16(&iop->iop_pcdir, 0x088F, 0x088B);

	/*
	 * PDDIR[9]  = 1 [0x0040] -> GPIO: (TXD4)
	 * PDDIR[6]  = 1 [0x0200] -> GPIO: (CS_CODEC_3)
	 */
	setbits_be16(&iop->iop_pddir, 0x0240);

	/*
	 * PEDIR[30] = 1 [0x00000002] -> GPIO: (FPGA_FIRMWARE)
	 * PEDIR[27] = 1 [0x00000010] -> GPIO: (R2_RXER)
	 * PEDIR[26] = 1 [0x00000020] -> GPIO: (R2_CRS_DV)
	 * PEDIR[23] = 1 [0x00000100] -> GPIO: (L1ST1)
	 * PEDIR[22] = 1 [0x00000200] -> GPIO: (R2_RXD1)
	 * PEDIR[21] = 1 [0x00000400] -> GPIO: (R2_RXD0)
	 * PEDIR[19] = 1 [0x00001000] -> GPIO: (R2_TXEN)
	 * PEDIR[18] = 1 [0x00002000] -> GPIO: (PE18)
	 * PEDIR[16] = 1 [0x00008000] -> GPIO: (R2_REF_CLK)
	 * PEDIR[15] = 1 [0x00010000] -> GPIO: (R2_TXD1)
	 * PEDIR[14] = 1 [0x00020000] -> GPIO: (R2_TXD0)
	 */
	setbits_be32(&cp->cp_pedir, 0x0003B732);

	/*
	 * PAODR[10] = 1 [0x0020] -> GPIO: (INIT_FPGA_F)
	 */
	setbits_be16(&iop->iop_paodr, 0x0020);

	/*
	 * PEODR[30] = 1 [0x00000002] -> GPIO: (FPGA_FIRMWARE)
	 * PEODR[18] = 0 [0x00002000] -> GPIO: (PE18)
	 */
	clrsetbits_be32(&cp->cp_peodr, 0x00002002, 0x00000002);

	/*
	 * PESO[24] = 1 [0x00000080] -> GPIO: (BRG01)
	 * PESO[23] = 1 [0x00000100] -> GPIO: (L1ST1)
	 * PESO[20] = 1 [0x00000800] -> GPIO: (SMTXD2)
	 * PESO[19] = 1 [0x00001000] -> GPIO: (R2_TXEN)
	 * PESO[15] = 1 [0x00010000] -> GPIO: (R2_TXD1)
	 * PESO[14] = 1 [0x00020000] -> GPIO: (R2_TXD0)
	 */
	setbits_be32(&cp->cp_peso, 0x00031980);
}

int board_early_init_f(void)
{
	return 0;
}

/* Specific board initialization */
int board_early_init_r(void)
{
	immap_t __iomem *immr = (immap_t __iomem *)CONFIG_SYS_IMMR;
	iop8xx_t __iomem *iop = &immr->im_ioport;
	cpm8xx_t __iomem *cp = &immr->im_cpm;

	/* MPC885 Port settings common to all boards */
	setbits_be16(&iop->iop_padat, 0x0000);

	/* Port A (MPC885 reference manual - 34.2) */
	/*
	 * In/Out or per. Function 0/1
	 * PADIR[15] = 0 [0x0001] -> GPIO: (USB_RXD)
	 * PADIR[14] = 0 [0x0002] -> GPIO: (USB_OE)
	 * PADIR[13] = 0 [0x0004] -> GPIO: ()
	 * PADIR[12] = 0 [0x0008] -> GPIO: ()
	 * PADIR[11] = 1 [0x0010] -> GPIO: (R1_TXD0)
	 * PADIR[10] = 0 [0x0020] -> GPIO: ()
	 * PADIR[7]  = 0 [0x0100] -> GPIO: ()
	 * PADIR[6]  = 0 [0x0200] -> GPIO: ()
	 * PADIR[4]  = 1 [0x0800] -> GPIO: (R1_TXD1)
	 * PADIR[3]  = 0 [0x1000] -> GPIO: (R1_RXER)
	 * PADIR[2]  = 0 [0x2000] -> GPIO: (R1_CRS_DV)
	 * PADIR[1]  = 0 [0x4000] -> GPIO: (R1_RXD0)
	 * PADIR[0]  = 0 [0x8000] -> GPIO: (R1_RXD1)
	 */
	clrsetbits_be16(&iop->iop_padir, 0xFB3F, 0x0810);

	/*
	 * Open drain or active output
	 * PAODR[15] = x [0x0001]
	 * PAODR[14] = 0 [0x0002]
	 * PAODR[13] = x [0x0004]
	 * PAODR[12] = 0 [0x0008]
	 * PAODR[11] = 0 [0x0010]
	 * PAODR[9]  = 0 [0x0040]
	 * PAODR[8]  = 0 [0x0080]
	 * PAODR[7]  = 0 [0x0100]
	 */
	clrbits_be16(&iop->iop_paodr, 0x01DF);

	/*
	 * GPIO or per. Function
	 * PAPAR[15] = 1 [0x0001] -> GPIO: (USB_RXD)
	 * PAPAR[14] = 1 [0x0002] -> GPIO: (USB_OE)
	 * PAPAR[11] = 1 [0x0010] -> GPIO: (R1_TXD0)
	 * PAPAR[10] = 0 [0x0020] -> GPIO: (INIT_FPGA_F)
	 * PAPAR[5]  = 0 [0x0400] -> GPIO: ()
	 * PAPAR[4]  = 1 [0x0800] -> GPIO: (R1_TXD1)
	 * PAPAR[3]  = 1 [0x1000] -> GPIO: (R1_RXER)
	 * PAPAR[2]  = 1 [0x2000] -> GPIO: (R1_CRS_DV)
	 * PAPAR[1]  = 1 [0x4000] -> GPIO: (R1_RXD0)
	 * PAPAR[0]  = 1 [0x8000] -> GPIO: (R1_RXD1)
	 */
	clrsetbits_be16(&iop->iop_papar, 0xFC33, 0xF813);

	/* Port B (MPC885 reference manual - 34.3) */
	/*
	 * In/Out or per. Function 0/1
	 * PBDIR[31] = 0 [0x00000001] -> GPIO: (R1_REF_CLK)
	 * PBDIR[30] = 1 [0x00000002] -> GPIO: (CLK)
	 * PBDIR[29] = 1 [0x00000004] -> GPIO: (MOSI)
	 * PBDIR[28] = 1 [0x00000008] -> GPIO: (MISO)
	 * PBDIR[25] = 0 [0x00000040] -> GPIO: (SMTXD1)
	 * PBDIR[24] = 0 [0x00000080] -> GPIO: (SMRXD1)
	 * PBDIR[22] = 0 [0x00000200] -> GPIO: (INIT_FPGA_MEZZ)
	 * PBDIR[21] = 1 [0x00000400] -> GPIO: (CS_EEPROM)
	 * PBDIR[20] = 0 [0x00000800] -> GPIO: (SMRXD2)
	 * PBDIR[19] = 1 [0x00001000] -> GPIO: (WR_TEMP)
	 * PBDIR[17] = 0 [0x00004000] -> GPIO: (DONE_FPGA_MEZZ)
	 */
	clrsetbits_be32(&cp->cp_pbdir, 0x00005ECF, 0x0000140E);

	/*
	 * Open drain or active output
	 * PBODR[31] = 0 [0x00000001] -> GPIO: (R1_REF_CLK)
	 * PBODR[30] = 0 [0x00000002] -> GPIO: (CLK)
	 * PBODR[29] = 0 [0x00000004] -> GPIO: (MOSI)
	 * PBODR[28] = 0 [0x00000008] -> GPIO: (MISO)
	 * PBODR[27] = 0 [0x00000010] -> GPIO: (WR_TEMP2)
	 * PBODR[26] = 0 [0x00000020] -> GPIO: (BRG02)
	 * PBODR[25] = 0 [0x00000040] -> GPIO: (SMTXD1)
	 * PBODR[24] = 0 [0x00000080] -> GPIO: (SMRXD1)
	 * PBODR[23] = 0 [0x00000100] -> GPIO: (CS_TEMP2)
	 * PBODR[22] = 0 [0x00000200] -> GPIO: (INIT_FPGA_MEZZ)
	 * PBODR[21] = 0 [0x00000400] -> GPIO: (CS_EEPROM)
	 * PBODR[20] = 0 [0x00000800] -> GPIO: (SMRXD2)
	 * PBODR[19] = 0 [0x00001000] -> GPIO: (WR_TEMP)
	 * PBODR[18] = 0 [0x00002000] -> GPIO: (RTS2)
	 * PBODR[17] = 0 [0x00004000] -> GPIO: (DONE_FPGA_MEZZ)
	 */
	clrbits_be16(&cp->cp_pbodr, 0x00007FFF);

	/*
	 * GPIO or per. Function
	 * PBPAR[31] = 1 [0x00000001] -> GPIO: (R1_REF_CLK)
	 * PBPAR[30] = 1 [0x00000002] -> GPIO: (CLK)
	 * PBPAR[29] = 1 [0x00000004] -> GPIO: (MOSI)
	 * PBPAR[28] = 1 [0x00000008] -> GPIO: (MISO)
	 * PBPAR[27] = 0 [0x00000010] -> GPIO: (WR_TEMP2)
	 * PBPAR[26] = 0 [0x00000020] -> GPIO: (BRG02)
	 * PBPAR[25] = 1 [0x00000040] -> GPIO: (SMTXD1)
	 * PBPAR[24] = 1 [0x00000080] -> GPIO: (SMRXD1)
	 * PBPAR[23] = 0 [0x00000100] -> GPIO: (CS_TEMP2)
	 * PBPAR[22] = 0 [0x00000200] -> GPIO: (INIT_FPGA_MEZZ)
	 * PBPAR[21] = 0 [0x00000400] -> GPIO: (CS_EEPROM)
	 * PBPAR[19] = 0 [0x00001000] -> GPIO: (WR_TEMP)
	 * PBPAR[18] = 0 [0x00002000] -> GPIO: (RTS2)
	 * PBPAR[15] = 0 [0x00010000] -> GPIO: (BRG03)
	 * PBPAR[14] = 0 [0x00020000] -> GPIO: (CS_TEMP)
	 */
	clrsetbits_be32(&cp->cp_pbpar, 0x000337FF, 0x000000CF);

	/* Port C (MPC885 Reference Manual - 34.4) */
	/*
	 * In/Out or per. Function 0/1
	 * PCDIR[11] = 0 [0x0010] -> GPIO: (USB_RXP)
	 * PCDIR[10] = 0 [0x0020] -> GPIO: (USB_RXN)
	 * PCDIR[9]  = 0 [0x0040] -> GPIO: (CTS2)
	 * PCDIR[7]  = 1 [0x0100] -> GPIO: (USB_TXP)
	 * PCDIR[6]  = 1 [0x0200] -> GPIO: (USB_TXN)
	 * PCDIR[5]  = 0 [0x0400] -> GPIO: (CTS3)
	 */
	clrsetbits_be16(&iop->iop_pcdir, 0x0770, 0x0300);

	/*
	 * GPIO or per. Function
	 * PCPAR[15] = 0 [0x0001] -> GPIO: (WP_EEPROM2)
	 * PCPAR[13] = 0 [0x0004] -> GPIO: (CS_POT1)
	 * PCPAR[12] = 0 [0x0008] -> GPIO: (CS_EEPROM2)
	 * PCPAR[11] = 0 [0x0010] -> GPIO: (USB_RXP)
	 * PCPAR[10] = 0 [0x0020] -> GPIO: (USB_RXN)
	 * PCPAR[9]  = 0 [0x0040] -> GPIO: (CTS2)
	 * PCPAR[8]  = 0 [0x0080] -> GPIO: (CS_CODEC_FAV)
	 * PCPAR[7]  = 1 [0x0100] -> GPIO: (USB_TXP)
	 * PCPAR[6]  = 1 [0x0200] -> GPIO: (USB_TXN)
	 * PCPAR[5]  = 0 [0x0400] -> GPIO: (CTS3)
	 * PCPAR[4]  = 0 [0x0800] -> GPIO: (CS_CODEC_RADIO)
	 */
	clrsetbits_be16(&iop->iop_pcpar, 0x0FFD, 0x0300);

	/*
	 * Special Option register
	 * PCSO[15] = 0 [0x0001] -> GPIO: (WP_EEPROM2)
	 * PCSO[14] = 0 [0x0002] -> GPIO: (CS_POT2)
	 * PCSO[13] = x [0x0004] -> Reserved
	 * PCSO[12] = x [0x0008] -> Reserved
	 * PCSO[11] = 1 [0x0010] -> GPIO: (USB_RXP)
	 * PCSO[10] = 1 [0x0020] -> GPIO: (USB_RXN)
	 * PCSO[9]  = 1 [0x0040] -> GPIO: (CTS2)
	 * PCSO[8]  = 0 [0x0080] -> GPIO: (CS_CODEC_FAV)
	 * PCSO[7]  = 0 [0x0100] -> GPIO: (USB_TXP)
	 * PCSO[6]  = 0 [0x0200] -> GPIO: (USB_TXN)
	 * PCSO[5]  = 1 [0x0400] -> GPIO: (CTS3)
	 * PCSO[4]  = 0 [0x0800] -> GPIO: (CS_CODEC_RADIO)
	 */
	clrsetbits_be16(&iop->iop_pcso, 0x0FF3, 0x0470);

	/*
	 * Interrupt or IO
	 * PCINT[15] = 0 [0x0001] -> GPIO: ()
	 * PCINT[14] = 0 [0x0002] -> GPIO: ()
	 * PCINT[13] = 0 [0x0004] -> GPIO: ()
	 * PCINT[12] = 0 [0x0008] -> GPIO: ()
	 * PCINT[11] = 0 [0x0010] -> GPIO: (USB_RXP)
	 * PCINT[10] = 0 [0x0020] -> GPIO: (USB_RXN)
	 * PCINT[9]  = 0 [0x0040] -> GPIO: ()
	 * PCINT[8]  = 0 [0x0080] -> GPIO: ()
	 * PCINT[7]  = 0 [0x0100] -> GPIO: (USB_TXP)
	 * PCINT[6]  = 0 [0x0200] -> GPIO: (USB_TXN)
	 * PCINT[5]  = 0 [0x0400] -> GPIO: ()
	 * PCINT[4]  = 0 [0x0800] -> GPIO: ()
	 */
	clrbits_be16(&iop->iop_pcint, 0x0FFF);

	/* Port D (MPC885 Reference Manual - 34.5) */
	/*
	 * In/Out or per. Function 0/1
	 * PDDIR[15] = 1 [0x0001] -> GPIO: (CS_NAND)
	 * PDDIR[14] = 0 [0x0002] -> GPIO: (TDM_FS_MPC)
	 * PDDIR[13] = 1 [0x0004] -> GPIO: (ALE_NAND)
	 * PDDIR[12] = 1 [0x0008] -> GPIO: (CLE_NAND)
	 * PDDIR[11] = 0 [0x0010] -> GPIO: (RXD3)
	 * PDDIR[10] = 0 [0x0020] -> GPIO: (TXD3)
	 * PDDIR[9]  = 1 [0x0040] -> GPIO: (TXD4)
	 * PDDIR[8]  = 0 [0x0080] -> GPIO: (R_MDC)
	 * PDDIR[7]  = 0 [0x0100] -> GPIO: (RTS3)
	 * PDDIR[5]  = 0 [0x0400] -> GPIO: (CLK8)
	 * PDDIR[4]  = 0 [0x0800] -> GPIO: (CLK4)
	 * PDDIR[3]  = 0 [0x1000] -> GPIO: (CLK7)
	 */
	clrsetbits_be16(&iop->iop_pddir, 0x1DFF, 0x004D);

	/*
	 * GPIO or per. Function
	 * PDPAR[15] = 0 [0x0001] -> GPIO: (CS_NAND)
	 * PDPAR[13] = 0 [0x0004] -> GPIO: (ALE_NAND)
	 * PDPAR[12] = 0 [0x0008] -> GPIO: (CLE_NAND)
	 * PDPAR[8]  = 1 [0x0080] -> GPIO: (R_MDC)
	 * PDPAR[6]  = 0 [0x0200] -> GPIO: (INIT_FPGA_RADIO)
	 * PDPAR[4]  = 1 [0x0800] -> GPIO: (CLK4)
	 */
	clrsetbits_be16(&iop->iop_pdpar, 0x0A8D, 0x0880);

	/* Port E (MPC885 Reference Manual - 34.6) */
	/*
	 * In/Out or per. Function 0/1
	 * PEDIR[31] = 0 [0x00000001] -> GPIO: (DONE_FPGA_FIRMWARE)
	 * PEDIR[29] = 1 [0x00000004] -> GPIO: (USB_SPEED)
	 * PEDIR[28] = 1 [0x00000008] -> GPIO: (USB_SUSPEND)
	 * PEDIR[25] = 0 [0x00000040] -> GPIO: (RXD4)
	 * PEDIR[24] = 0 [0x00000080] -> GPIO: (BRG01)
	 * PEDIR[20] = 0 [0x00000800] -> GPIO: (SMTXD2)
	 * PEDIR[17] = 0 [0x00004000] -> GPIO: (CLK5)
	 */
	clrsetbits_be32(&cp->cp_pedir, 0x000048CD, 0x0000000C);

	/*
	 * open drain or active output
	 * PEODR[31] = 0 [0x00000001] -> GPIO: (DONE_FPGA_FIRMWARE)
	 * PEODR[29] = 0 [0x00000004] -> GPIO: (USB_SPEED)
	 * PEODR[28] = 1 [0x00000008] -> GPIO: (USB_SUSPEND)
	 * PEODR[27] = 0 [0x00000010] -> GPIO: (R2_RXER)
	 * PEODR[26] = 0 [0x00000020] -> GPIO: (R2_CRS_DV)
	 * PEODR[25] = 0 [0x00000040] -> GPIO: (RXD4)
	 * PEODR[24] = 0 [0x00000080] -> GPIO: (BRG01)
	 * PEODR[23] = 0 [0x00000100] -> GPIO: (DONE_FPGA_RADIO)
	 * PEODR[22] = 0 [0x00000200] -> GPIO: (R2_RXD1)
	 * PEODR[21] = 0 [0x00000400] -> GPIO: (R2_RXD0)
	 * PEODR[20] = 0 [0x00000800] -> GPIO: (SMTXD2)
	 * PEODR[19] = 0 [0x00001000] -> GPIO: (R2_TXEN)
	 * PEODR[17] = 0 [0x00004000] -> GPIO: (CLK5)
	 * PEODR[16] = 0 [0x00008000] -> GPIO: (R2_REF_CLK)
	 */
	clrsetbits_be32(&cp->cp_peodr, 0x0000DFFD, 0x00000008);

	/*
	 * GPIO or per. Function
	 * PEPAR[31] = 0 [0x00000001] -> GPIO: (DONE_FPGA_FIRMWARE)
	 * PEPAR[30] = 0 [0x00000002] -> GPIO: (PROG_FPGA_FIRMWARE)
	 * PEPAR[29] = 0 [0x00000004] -> GPIO: (USB_SPEED)
	 * PEPAR[28] = 0 [0x00000008] -> GPIO: (USB_SUSPEND)
	 * PEPAR[18] = 0 [0x00002000] -> GPIO: (PROG_FPGA_RADIO)
	 */
	clrbits_be32(&cp->cp_pepar, 0x0000200F);

	/*
	 * Special Option register
	 * PESO[31] = 0 [0x00000001] -> GPIO: (DONE_FPGA_FIRMWARE)
	 * PESO[30] = 0 [0x00000002] -> GPIO: (PROG_FPGA_FIRMWARE)
	 * PESO[29] = 0 [0x00000004] -> GPIO: (USB_SPEED)
	 * PESO[28] = 0 [0x00000008] -> GPIO: (USB_SUSPEND)
	 * PESO[27] = 0 [0x00000010] -> GPIO: (R2_RXER)
	 * PESO[26] = 0 [0x00000020] -> GPIO: (R2_CRS_DV)
	 * PESO[25] = 0 [0x00000040] -> GPIO: (RXD4)
	 * PESO[22] = 0 [0x00000200] -> GPIO: (R2_RXD1)
	 * PESO[21] = 0 [0x00000400] -> GPIO: (R2_RXD0)
	 * PESO[18] = 0 [0x00002000] -> GPIO: (PROG_FPGA_RADIO)
	 * PESO[17] = 0 [0x00004000] -> GPIO: (CLK5)
	 * PESO[16] = 0 [0x00008000] -> GPIO: (R2_REF_CLK)
	 */
	clrbits_be32(&cp->cp_peso, 0x0000E67F);

	/* Is a motherboard present ? */
	if (in_be16(ADDR_CPLD_R_ETAT) & R_ETAT_PRES_BASE) {
		/* Initialize signal PROG_FPGA_FIRMWARE */
		out_be32(&cp->cp_pedat, 0x00000002);
		out_be32(&cp->cp_peodr, 0x00000002);
		out_be32(&cp->cp_pedir, 0x00000002);

		/* Check if fpga firmware is loaded */
		if (!(in_be32(&cp->cp_pedat) & 0x00000001)) {
			printf("Reloading FPGA firmware.\n");

			/* Load fpga firmware */
			/* Activate PROG_FPGA_FIRMWARE for 1 usec */
			clrbits_be32(&cp->cp_pedat, 0x00000002);
			udelay(1);
			setbits_be32(&cp->cp_pedat, 0x00000002);

			/* Wait 200 msec and check DONE_FPGA_FIRMWARE */
			mdelay(200);
			if (!(in_be32(&cp->cp_pedat) & 0x00000001)) {
				for (;;) {
					printf("error loading firmware.\n");
					mdelay(500);
				}
			}

			/* Send a reset signal and wait for 20 msec */
			clrbits_be16(ADDR_CPLD_R_RESET, R_RST_STATUS);
			mdelay(20);
			setbits_be16(ADDR_CPLD_R_RESET, R_RST_STATUS);
		}

		/* Wait 300 msec and check the reset state */
		mdelay(300);
		if (!(in_be16(ADDR_CPLD_R_RESET) & R_RESET_STATUS)) {
			for (;;) {
				printf("Could not reset FPGA.\n");
				mdelay(500);
			}
		}

		/* is FPGA firmware loaded ? */
		if (!(in_be32(&cp->cp_pedat) & 0x00000001)) {
			printf("Reloading FPGA firmware\n");

			/* Load FPGA firmware */
			/* Activate PROG_FPGA_FIRMWARE for 1 usec */
			clrbits_be32(&cp->cp_pedat, 0x00000002);
			udelay(1);
			setbits_be32(&cp->cp_pedat, 0x00000002);

			/* Wait 200ms before checking DONE_FPGA_FIRMWARE */
			mdelay(200);
		}

		/* Identify the type of mother board */
		switch (in_8(ADDR_FPGA_R_BASE)) {
		case TYPE_MCR:
			iop_setup_mcr();
			break;

		case TYPE_MIAE:
			iop_setup_miae();
			break;

		default:
			break;
		}
	/* CMPC885 board alone */
	} else {
		iop_setup_cmpc885();
	}

	return 0;
}
