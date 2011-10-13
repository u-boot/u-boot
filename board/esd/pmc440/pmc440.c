/*
 * (Cg) Copyright 2007-2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
 * Based on board/amcc/sequoia/sequoia.c
 *
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2006
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Alain Saurel,	    AMCC/IBM, alain.saurel@fr.ibm.com
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
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/ppc440.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <command.h>
#include <i2c.h>
#ifdef CONFIG_RESET_PHY_R
#include <miiphy.h>
#endif
#include <serial.h>
#include <asm/4xx_pci.h>

#include "fpga.h"
#include "pmc440.h"

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */
extern void __ft_board_setup(void *blob, bd_t *bd);

ulong flash_get_size(ulong base, int banknum);
int pci_is_66mhz(void);
int is_monarch(void);
int bootstrap_eeprom_read(unsigned dev_addr, unsigned offset,
			  uchar *buffer, unsigned cnt);

struct serial_device *default_serial_console(void)
{
	uchar buf[4];
	ulong delay;
	int i;
	ulong val;

	/*
	 * Use default console on P4 when strapping jumper
	 * is installed (bootstrap option != 'H').
	 */
	mfsdr(SDR0_PINSTP, val);
	if (((val & 0xf0000000) >> 29) != 7)
		return &eserial2_device;

	ulong scratchreg = in_be32((void*)GPIO0_ISR3L);
	if (!(scratchreg & 0x80)) {
		/* mark scratchreg valid */
		scratchreg = (scratchreg & 0xffffff00) | 0x80;

		i = bootstrap_eeprom_read(CONFIG_SYS_I2C_BOOT_EEPROM_ADDR,
					  0x10, buf, 4);
		if ((i != -1) && (buf[0] == 0x19) && (buf[1] == 0x75)) {
			scratchreg |= buf[2];

			/* bringup delay for console */
			for (delay=0; delay<(1000 * (ulong)buf[3]); delay++) {
				udelay(1000);
			}
		} else
			scratchreg |= 0x01;
		out_be32((void*)GPIO0_ISR3L, scratchreg);
	}

	if (scratchreg & 0x01)
		return &eserial2_device;
	else
		return &eserial1_device;
}

int board_early_init_f(void)
{
	u32 sdr0_cust0;
	u32 sdr0_pfc1, sdr0_pfc2;
	u32 reg;

	/* general EBC configuration (disable EBC timeouts) */
	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	mtdcr(EBC0_CFGDATA, 0xf8400000);

	/*
	 * Setup the GPIO pins
	 * TODO: setup GPIOs via CONFIG_SYS_4xx_GPIO_TABLE in board's config file
	 */
	out_be32((void *)GPIO0_OR,    0x40000102);
	out_be32((void *)GPIO0_TCR,   0x4c90011f);
	out_be32((void *)GPIO0_OSRL,  0x28051400);
	out_be32((void *)GPIO0_OSRH,  0x55005000);
	out_be32((void *)GPIO0_TSRL,  0x08051400);
	out_be32((void *)GPIO0_TSRH,  0x55005000);
	out_be32((void *)GPIO0_ISR1L, 0x54000000);
	out_be32((void *)GPIO0_ISR1H, 0x00000000);
	out_be32((void *)GPIO0_ISR2L, 0x44000000);
	out_be32((void *)GPIO0_ISR2H, 0x00000100);
	out_be32((void *)GPIO0_ISR3L, 0x00000000);
	out_be32((void *)GPIO0_ISR3H, 0x00000000);

	out_be32((void *)GPIO1_OR,    0x80002408);
	out_be32((void *)GPIO1_TCR,   0xd6003c08);
	out_be32((void *)GPIO1_OSRL,  0x0a5a0000);
	out_be32((void *)GPIO1_OSRH,  0x00000000);
	out_be32((void *)GPIO1_TSRL,  0x00000000);
	out_be32((void *)GPIO1_TSRH,  0x00000000);
	out_be32((void *)GPIO1_ISR1L, 0x00005555);
	out_be32((void *)GPIO1_ISR1H, 0x40000000);
	out_be32((void *)GPIO1_ISR2L, 0x04010000);
	out_be32((void *)GPIO1_ISR2H, 0x00000000);
	out_be32((void *)GPIO1_ISR3L, 0x01400000);
	out_be32((void *)GPIO1_ISR3H, 0x00000000);

	/* patch PLB:PCI divider for 66MHz PCI */
	mfcpr(CPR0_SPCID, reg);
	if (pci_is_66mhz() && (reg != 0x02000000)) {
		mtcpr(CPR0_SPCID, 0x02000000); /* 133MHZ : 2 for 66MHz PCI */

		mfcpr(CPR0_ICFG, reg);
		reg |= CPR0_ICFG_RLI_MASK;
		mtcpr(CPR0_ICFG, reg);

		mtspr(SPRN_DBCR0, 0x20000000); /* do chip reset */
	}

	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffff7ef);
	mtdcr(UIC0TR, 0x00000000);
	mtdcr(UIC0VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffc7f5);
	mtdcr(UIC1TR, 0x00000000);
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	mtdcr(UIC2SR, 0xffffffff);	/* clear all */
	mtdcr(UIC2ER, 0x00000000);	/* disable all */
	mtdcr(UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC2PR, 0x27ffffff);
	mtdcr(UIC2TR, 0x00000000);
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */

	/* select Ethernet pins */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
		SDR0_PFC1_SELECT_CONFIG_4;
	mfsdr(SDR0_PFC2, sdr0_pfc2);
	sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
		SDR0_PFC2_SELECT_CONFIG_4;

	/* enable 2nd IIC */
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_IIC1_SEL;

	mtsdr(SDR0_PFC2, sdr0_pfc2);
	mtsdr(SDR0_PFC1, sdr0_pfc1);

	/* setup NAND FLASH */
	mfsdr(SDR0_CUST0, sdr0_cust0);
	sdr0_cust0 = SDR0_CUST0_MUX_NDFC_SEL	|
		SDR0_CUST0_NDFC_ENABLE		|
		SDR0_CUST0_NDFC_BW_8_BIT	|
		SDR0_CUST0_NDFC_ARE_MASK	|
		(0x80000000 >> (28 + CONFIG_SYS_NAND_CS));
	mtsdr(SDR0_CUST0, sdr0_cust0);

	return 0;
}

#if defined(CONFIG_MISC_INIT_F)
int misc_init_f(void)
{
	struct pci_controller hose;
	hose.first_busno = 0;
	hose.last_busno = 0;
	hose.region_count = 0;

	if (getenv("pciearly") && (!is_monarch())) {
		printf("PCI:   early target init\n");
		pci_setup_indirect(&hose, PCIL0_CFGADR, PCIL0_CFGDATA);
		pci_target_init(&hose);
	}
	return 0;
}
#endif

/*
 * misc_init_r.
 */
int misc_init_r(void)
{
	uint pbcr;
	int size_val = 0;
	u32 reg;
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;
	unsigned long sdr0_srst0, sdr0_srst1;
	char *act = getenv("usbact");

	/*
	 * FLASH stuff...
	 */

	/* Re-do sizing to get full correct info */

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtdcr(EBC0_CFGADDR, PB2CR);
#else
	mtdcr(EBC0_CFGADDR, PB0CR);
#endif
	pbcr = mfdcr(EBC0_CFGDATA);
	size_val = ffs(gd->bd->bi_flashsize) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtdcr(EBC0_CFGADDR, PB2CR);
#else
	mtdcr(EBC0_CFGADDR, PB0CR);
#endif
	mtdcr(EBC0_CFGDATA, pbcr);

	/*
	 * Re-check to get correct base address
	 */
	flash_get_size(gd->bd->bi_flashstart, 0);

#ifdef CONFIG_ENV_IS_IN_FLASH
	/* Monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    -CONFIG_SYS_MONITOR_LEN,
			    0xffffffff,
			    &flash_info[0]);

	/* Env protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET,
			    CONFIG_ENV_ADDR_REDUND,
			    CONFIG_ENV_ADDR_REDUND + 2*CONFIG_ENV_SECT_SIZE - 1,
			    &flash_info[0]);
#endif

	/*
	 * USB suff...
	 */
	if ((act == NULL || strcmp(act, "host") == 0) &&
	    !(in_be32((void*)GPIO0_IR) & GPIO0_USB_PRSNT)){
		/* SDR Setting */
		mfsdr(SDR0_PFC1, sdr0_pfc1);
		mfsdr(SDR0_USB2D0CR, usb2d0cr);
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mfsdr(SDR0_USB2H0CR, usb2h0cr);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_16BIT_30MHZ;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;

		/*
		 * An 8-bit/60MHz interface is the only possible alternative
		 * when connecting the Device to the PHY
		 */
		usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_16BIT_30MHZ;

		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;

		mtsdr(SDR0_PFC1, sdr0_pfc1);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2H0CR, usb2h0cr);

		/*
		 * Take USB out of reset:
		 * -Initial status = all cores are in reset
		 * -deassert reset to OPB1, P4OPB0, OPB2, PLB42OPB1 OPB2PLB40 cores
		 * -wait 1 ms
		 * -deassert reset to PHY
		 * -wait 1 ms
		 * -deassert  reset to HOST
		 * -wait 4 ms
		 * -deassert all other resets
		 */
		mfsdr(SDR0_SRST1, sdr0_srst1);
		sdr0_srst1 &= ~(SDR0_SRST1_OPBA1 |	\
				SDR0_SRST1_P4OPB0 |	\
				SDR0_SRST1_OPBA2 |	\
				SDR0_SRST1_PLB42OPB1 |	\
				SDR0_SRST1_OPB2PLB40);
		mtsdr(SDR0_SRST1, sdr0_srst1);
		udelay(1000);

		mfsdr(SDR0_SRST1, sdr0_srst1);
		sdr0_srst1 &= ~SDR0_SRST1_USB20PHY;
		mtsdr(SDR0_SRST1, sdr0_srst1);
		udelay(1000);

		mfsdr(SDR0_SRST0, sdr0_srst0);
		sdr0_srst0 &= ~SDR0_SRST0_USB2H;
		mtsdr(SDR0_SRST0, sdr0_srst0);
		udelay(4000);

		/* finally all the other resets */
		mtsdr(SDR0_SRST1, 0x00000000);
		mtsdr(SDR0_SRST0, 0x00000000);

		if (!(in_be32((void*)GPIO0_IR) & GPIO0_USB_PRSNT)) {
			/* enable power on USB socket */
			out_be32((void*)GPIO1_OR,
				 in_be32((void*)GPIO1_OR) & ~GPIO1_USB_PWR_N);
		}

		printf("USB:   Host\n");

	} else if ((strcmp(act, "dev") == 0) ||
		   (in_be32((void*)GPIO0_IR) & GPIO0_USB_PRSNT)) {
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PURDIS;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_HOST;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_HOST;
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x672c6000);

		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000080);

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x60206000);

		*(unsigned int *)(0xe0000350) = 0x00000001;

		udelay (1000);
		mtsdr(SDR0_SRST1, 0x60306000);

		/* SDR Setting */
		mfsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mfsdr(SDR0_USB2H0CR, usb2h0cr);
		mfsdr(SDR0_USB2D0CR, usb2d0cr);
		mfsdr(SDR0_PFC1, sdr0_pfc1);

		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_XOCLK_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_XOCLK_EXTERNAL;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_WDINT_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_WDINT_8BIT_60MHZ;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DVBUS_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DVBUS_PUREN;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_DWNSTR_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_DWNSTR_DEV;
		usb2phy0cr = usb2phy0cr &~SDR0_USB2PHY0CR_UTMICN_MASK;
		usb2phy0cr = usb2phy0cr | SDR0_USB2PHY0CR_UTMICN_DEV;

		usb2h0cr   = usb2h0cr &~SDR0_USB2H0CR_WDINT_MASK;
		usb2h0cr   = usb2h0cr | SDR0_USB2H0CR_WDINT_8BIT_60MHZ;

		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;

		mtsdr(SDR0_USB2H0CR, usb2h0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_PFC1, sdr0_pfc1);

		/*clear resets*/
		udelay(1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay(1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Device\n");
	}

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(PLB4A0_ACR) & ~PLB4Ax_ACR_WRP_MASK;
	mtdcr(PLB4A0_ACR, reg);

#ifdef CONFIG_FPGA
	pmc440_init_fpga();
#endif

	/* turn off POST LED */
	out_be32((void*)GPIO1_OR,  in_be32((void*)GPIO1_OR) & ~GPIO1_POST_N);
	/* turn on RUN LED */
	out_be32((void*)GPIO0_OR,  in_be32((void*)GPIO0_OR) & ~GPIO0_LED_RUN_N);
	return 0;
}

int is_monarch(void)
{
	if (in_be32((void*)GPIO1_IR) & GPIO1_NONMONARCH)
		return 0;

	return 1;
}

int pci_is_66mhz(void)
{
	if (in_be32((void*)GPIO1_IR) & GPIO1_M66EN)
		return 1;
	return 0;
}

int board_revision(void)
{
	return (int)((in_be32((void*)GPIO1_IR) & GPIO1_HWID_MASK) >> 4);
}

int checkboard(void)
{
	puts("Board: esd GmbH - PMC440");

	gd->board_type = board_revision();
	printf(", Rev 1.%ld, ", gd->board_type);

	if (!is_monarch()) {
		puts("non-");
	}

	printf("monarch, PCI=%s MHz\n", pci_is_66mhz() ? "66" : "33");
	return (0);
}


#if defined(CONFIG_PCI) && defined(CONFIG_PCI_PNP)
/*
 * Assign interrupts to PCI devices. Some OSs rely on this.
 */
void board_pci_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char int_line[] = {IRQ_PCIC, IRQ_PCID, IRQ_PCIA, IRQ_PCIB};

	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE,
				   int_line[PCI_DEV(dev) & 0x03]);
}
#endif

/*
 * pci_target_init
 *
 * The bootstrap configuration provides default settings for the pci
 * inbound map (PIM). But the bootstrap config choices are limited and
 * may not be sufficient for a given board.
 */
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	char *ptmla_str, *ptmms_str;

	/*
	 * Set up Direct MMIO registers
	 */
	/*
	 * PowerPC440EPX PCI Master configuration.
	 * Map one 1Gig range of PLB/processor addresses to PCI memory space.
	 * PLB address 0x80000000-0xBFFFFFFF
	 *     ==> PCI address 0x80000000-0xBFFFFFFF
	 * Use byte reversed out routines to handle endianess.
	 * Make this region non-prefetchable.
	 */
	out32r(PCIL0_PMM0MA, 0x00000000);	/* PMM0 Mask/Attribute */
						/* - disabled b4 setting */
	out32r(PCIL0_PMM0LA, CONFIG_SYS_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIL0_PMM0PCILA, CONFIG_SYS_PCI_MEMBASE); /* PMM0 PCI Low Address */
	out32r(PCIL0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_PMM0MA, 0xc0000001);	/* 1G + No prefetching, */
						/* and enable region */

	if (!is_monarch()) {
		ptmla_str = getenv("ptm1la");
		ptmms_str = getenv("ptm1ms");
		if(NULL != ptmla_str && NULL != ptmms_str ) {
			out32r(PCIL0_PTM1MS,
			       simple_strtoul(ptmms_str, NULL, 16));
			out32r(PCIL0_PTM1LA,
			       simple_strtoul(ptmla_str, NULL, 16));
		} else {
			/* BAR1: default top 64MB of RAM */
			out32r(PCIL0_PTM1MS, 0xfc000001);
			out32r(PCIL0_PTM1LA, 0x0c000000);
		}
	} else {
		/* BAR1: default: complete 256MB RAM */
		out32r(PCIL0_PTM1MS, 0xf0000001);
		out32r(PCIL0_PTM1LA, 0x00000000);
	}

	ptmla_str = getenv("ptm2la");		/* Local Addr. Reg */
	ptmms_str = getenv("ptm2ms");		/* Memory Size/Attribute */
	if(NULL != ptmla_str && NULL != ptmms_str ) {
		out32r(PCIL0_PTM2MS, simple_strtoul(ptmms_str, NULL, 16));
		out32r(PCIL0_PTM2LA, simple_strtoul(ptmla_str, NULL, 16));
	} else {
		/* BAR2: default: 4MB FPGA */
		out32r(PCIL0_PTM2MS, 0xffc00001); /* Memory Size/Attribute */
		out32r(PCIL0_PTM2LA, 0xef000000); /* Local Addr. Reg */
	}

	if (is_monarch()) {
		/* BAR2: map FPGA registers behind system memory at 1GB */
		pci_hose_write_config_dword(hose, 0, PCI_BASE_ADDRESS_2, 0x40000008);
	}

	/*
	 * Set up Configuration registers
	 */

	/* Program the board's vendor id */
	pci_hose_write_config_word(hose, 0, PCI_SUBSYSTEM_VENDOR_ID,
				   CONFIG_SYS_PCI_SUBSYS_VENDORID);

	/* disabled for PMC405 backward compatibility */
	/* Configure command register as bus master */
	/* pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER); */


	/* 240nS PCI clock */
	pci_hose_write_config_word(hose, 0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_hose_write_config_word(hose, 0, PCI_ERREN, 0);

	if (!is_monarch()) {
		/* Program the board's subsystem id/classcode */
		pci_hose_write_config_word(hose, 0, PCI_SUBSYSTEM_ID,
					   CONFIG_SYS_PCI_SUBSYS_ID_NONMONARCH);
		pci_hose_write_config_word(hose, 0, PCI_CLASS_SUB_CODE,
					   CONFIG_SYS_PCI_CLASSCODE_NONMONARCH);

		/* PCI configuration done: release ERREADY */
		out_be32((void*)GPIO1_OR,
			 in_be32((void*)GPIO1_OR) | GPIO1_PPC_EREADY);
		out_be32((void*)GPIO1_TCR,
			 in_be32((void*)GPIO1_TCR) | GPIO1_PPC_EREADY);
	} else {
		/* Program the board's subsystem id/classcode */
		pci_hose_write_config_word(hose, 0, PCI_SUBSYSTEM_ID,
					   CONFIG_SYS_PCI_SUBSYS_ID_MONARCH);
		pci_hose_write_config_word(hose, 0, PCI_CLASS_SUB_CODE,
					   CONFIG_SYS_PCI_CLASSCODE_MONARCH);
	}

	/* enable host configuration */
	pci_hose_write_config_dword(hose, 0, PCI_BRDGOPT2, 0x00000101);
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

/*
 * Override weak default pci_master_init()
 */
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	/*
	 * Only configure the master in monach mode
	 */
	if (is_monarch())
		__pci_master_init(hose);
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT) */

static void wait_for_pci_ready(void)
{
	if (!(in_be32((void*)GPIO1_IR) & GPIO1_PPC_EREADY)) {
		printf("PCI:   Waiting for EREADY (CTRL-C to skip) ... ");
		while (1) {
			if (ctrlc()) {
				puts("abort\n");
				break;
			}
			if (in_be32((void*)GPIO1_IR) & GPIO1_PPC_EREADY) {
				printf("done\n");
				break;
			}
		}
	}
}

/*
 * Override weak is_pci_host()
 *
 * This routine is called to determine if a pci scan should be
 * performed. With various hardware environments (especially cPCI and
 * PPMC) it's insufficient to depend on the state of the arbiter enable
 * bit in the strap register, or generic host/adapter assumptions.
 *
 * Rather than hard-code a bad assumption in the general 440 code, the
 * 440 pci code requires the board to decide at runtime.
 *
 * Return 0 for adapter mode, non-zero for host (monarch) mode.
 */
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	char *s = getenv("pciscan");
	if (s == NULL)
		if (is_monarch()) {
			wait_for_pci_ready();
			return 1;
		} else
			return 0;
	else if (!strcmp(s, "yes"))
		return 1;

	return 0;
}
#endif /* defined(CONFIG_PCI) */

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
	char *s;
	unsigned short val_method, val_behavior;

	/* special LED setup for NGCC/CANDES */
	if ((s = getenv("bd_type")) &&
	    ((!strcmp(s, "ngcc")) || (!strcmp(s, "candes")))) {
		val_method   = 0x0e0a;
		val_behavior = 0x0cf2;
	} else {
		/* PMC440 standard type */
		val_method   = 0x0e10;
		val_behavior = 0x0cf0;
	}

	if (miiphy_write("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x1f, 0x0001) == 0) {
		miiphy_write("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x11, 0x0010);
		miiphy_write("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x11, val_behavior);
		miiphy_write("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x10, val_method);
		miiphy_write("ppc_4xx_eth0", CONFIG_PHY_ADDR, 0x1f, 0x0000);
	}

	if (miiphy_write("ppc_4xx_eth1", CONFIG_PHY1_ADDR, 0x1f, 0x0001) == 0) {
		miiphy_write("ppc_4xx_eth1", CONFIG_PHY1_ADDR, 0x11, 0x0010);
		miiphy_write("ppc_4xx_eth1", CONFIG_PHY1_ADDR, 0x11, val_behavior);
		miiphy_write("ppc_4xx_eth1", CONFIG_PHY1_ADDR, 0x10, val_method);
		miiphy_write("ppc_4xx_eth1", CONFIG_PHY1_ADDR, 0x1f, 0x0000);
	}
}
#endif

#if defined(CONFIG_SYS_EEPROM_WREN)
/*
 *  Input: <dev_addr> I2C address of EEPROM device to enable.
 *         <state>    -1: deliver current state
 *	               0: disable write
 *		       1: enable write
 *  Returns:          -1: wrong device address
 *                     0: dis-/en- able done
 *		     0/1: current state if <state> was -1.
 */
int eeprom_write_enable(unsigned dev_addr, int state)
{
	if ((CONFIG_SYS_I2C_EEPROM_ADDR != dev_addr) &&
	    (CONFIG_SYS_I2C_BOOT_EEPROM_ADDR != dev_addr)) {
		return -1;
	} else {
		switch (state) {
		case 1:
			/* Enable write access, clear bit GPIO_SINT2. */
			out_be32((void *)GPIO0_OR,
			      in_be32((void *)GPIO0_OR) & ~GPIO0_EP_EEP);
			state = 0;
			break;
		case 0:
			/* Disable write access, set bit GPIO_SINT2. */
			out_be32((void *)GPIO0_OR,
				 in_be32((void *)GPIO0_OR) | GPIO0_EP_EEP);
			state = 0;
			break;
		default:
			/* Read current status back. */
			state = (0 == (in_be32((void *)GPIO0_OR)
				       & GPIO0_EP_EEP));
			break;
		}
	}
	return state;
}
#endif /* #if defined(CONFIG_SYS_EEPROM_WREN) */

#define CONFIG_SYS_BOOT_EEPROM_PAGE_WRITE_BITS 3
int bootstrap_eeprom_write(unsigned dev_addr, unsigned offset,
			   uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

#if defined(CONFIG_SYS_EEPROM_WREN)
	eeprom_write_enable(dev_addr, 1);
#endif
	/*
	 * Write data until done or would cross a write page boundary.
	 * We must write the address again when changing pages
	 * because the address counter only increments within a page.
	 */

	while (offset < end) {
		unsigned alen, len;
		unsigned maxlen;
		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;		/* block number */
		addr[1] = blk_off;		/* block offset */
		alen	= 2;
		addr[0] |= dev_addr;		/* insert device address */

		len = end - offset;

#define	BOOT_EEPROM_PAGE_SIZE	   (1 << CONFIG_SYS_BOOT_EEPROM_PAGE_WRITE_BITS)
#define	BOOT_EEPROM_PAGE_OFFSET(x) ((x) & (BOOT_EEPROM_PAGE_SIZE - 1))

		maxlen = BOOT_EEPROM_PAGE_SIZE -
			BOOT_EEPROM_PAGE_OFFSET(blk_off);
		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;

		if (len > maxlen)
			len = maxlen;

		if (i2c_write (addr[0], offset, alen-1, buffer, len) != 0)
			rcode = 1;

		buffer += len;
		offset += len;

#if defined(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS)
		udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	}
#if defined(CONFIG_SYS_EEPROM_WREN)
	eeprom_write_enable(dev_addr, 0);
#endif
	return rcode;
}

int bootstrap_eeprom_read (unsigned dev_addr, unsigned offset,
			   uchar *buffer, unsigned cnt)
{
	unsigned end = offset + cnt;
	unsigned blk_off;
	int rcode = 0;

	/*
	 * Read data until done or would cross a page boundary.
	 * We must write the address again when changing pages
	 * because the next page may be in a different device.
	 */
	while (offset < end) {
		unsigned alen, len;
		unsigned maxlen;
		uchar addr[2];

		blk_off = offset & 0xFF;	/* block offset */

		addr[0] = offset >> 8;		/* block number */
		addr[1] = blk_off;		/* block offset */
		alen	= 2;

		addr[0] |= dev_addr;		/* insert device address */

		len = end - offset;

		maxlen = 0x100 - blk_off;
		if (maxlen > I2C_RXTX_LEN)
			maxlen = I2C_RXTX_LEN;
		if (len > maxlen)
			len = maxlen;

		if (i2c_read (addr[0], offset, alen-1, buffer, len) != 0)
			rcode = 1;
		buffer += len;
		offset += len;
	}

	return rcode;
}

#if defined(CONFIG_USB_OHCI_NEW) && defined(CONFIG_SYS_USB_OHCI_BOARD_INIT)
int usb_board_init(void)
{
	char *act = getenv("usbact");
	int i;

	if ((act == NULL || strcmp(act, "host") == 0) &&
	    !(in_be32((void*)GPIO0_IR) & GPIO0_USB_PRSNT))
		/* enable power on USB socket */
		out_be32((void*)GPIO1_OR,
			 in_be32((void*)GPIO1_OR) & ~GPIO1_USB_PWR_N);

	for (i=0; i<1000; i++)
		udelay(1000);

	return 0;
}

int usb_board_stop(void)
{
	/* disable power on USB socket */
	out_be32((void*)GPIO1_OR, in_be32((void*)GPIO1_OR) | GPIO1_USB_PWR_N);
	return 0;
}

int usb_board_init_fail(void)
{
	usb_board_stop();
	return 0;
}
#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_SYS_USB_OHCI_BOARD_INIT) */

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	int rc;

	__ft_board_setup(blob, bd);

	/*
	 * Disable PCI in non-monarch mode.
	 */
	if (!is_monarch()) {
		rc = fdt_find_and_setprop(blob, "/plb/pci@1ec000000", "status",
					  "disabled", sizeof("disabled"), 1);
		if (rc) {
			printf("Unable to update property status in PCI node, err=%s\n",
			       fdt_strerror(rc));
		}
	}
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
