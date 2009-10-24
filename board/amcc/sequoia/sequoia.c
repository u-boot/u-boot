/*
 * (C) Copyright 2006-2009
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
#include <ppc4xx.h>
#include <asm/gpio.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SYS_NO_FLASH)
extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */
#endif

extern void __ft_board_setup(void *blob, bd_t *bd);
ulong flash_get_size(ulong base, int banknum);

static inline u32 get_async_pci_freq(void)
{
	if (in_8((void *)(CONFIG_SYS_BCSR_BASE + 5)) &
		CONFIG_SYS_BCSR5_PCI66EN)
		return 66666666;
	else
		return 33333333;
}

int board_early_init_f(void)
{
	u32 sdr0_cust0;
	u32 sdr0_pfc1, sdr0_pfc2;
	u32 reg;

	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	mtdcr(EBC0_CFGDATA, 0xb8400000);

	/*
	 * Setup the interrupt controller polarities, triggers, etc.
	 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000005);	/* ATI & UIC1 crit are critical */
	mtdcr(UIC0PR, 0xfffff7ff);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	mtdcr(UIC2SR, 0xffffffff);	/* clear all */
	mtdcr(UIC2ER, 0x00000000);	/* disable all */
	mtdcr(UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC2PR, 0xffffffff);	/* per ref-board manual */
	mtdcr(UIC2TR, 0x00000000);	/* per ref-board manual */
	mtdcr(UIC2VR, 0x00000000);	/* int31 highest, base=0x000 */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all */

	/* Check and reconfigure the PCI sync clock if necessary */
	ppc4xx_pci_sync_clock_config(get_async_pci_freq());

	/* 50MHz tmrclk */
	out_8((u8 *) CONFIG_SYS_BCSR_BASE + 0x04, 0x00);

	/* clear write protects */
	out_8((u8 *) CONFIG_SYS_BCSR_BASE + 0x07, 0x00);

	/* enable Ethernet */
	out_8((u8 *) CONFIG_SYS_BCSR_BASE + 0x08, 0x00);

	/* enable USB device */
	out_8((u8 *) CONFIG_SYS_BCSR_BASE + 0x09, 0x20);

	/* select Ethernet (and optionally IIC1) pins */
	mfsdr(SDR0_PFC1, sdr0_pfc1);
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_SELECT_MASK) |
		SDR0_PFC1_SELECT_CONFIG_4;
#ifdef CONFIG_I2C_MULTI_BUS
	sdr0_pfc1 |= ((sdr0_pfc1 & ~SDR0_PFC1_SIS_MASK) | SDR0_PFC1_SIS_IIC1_SEL);
#endif
	/* Two UARTs, so we need 4-pin mode.  Also, we want CTS/RTS mode. */
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0IM_MASK) | SDR0_PFC1_U0IM_4PINS;
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U0ME_MASK) | SDR0_PFC1_U0ME_CTS_RTS;
	sdr0_pfc1 = (sdr0_pfc1 & ~SDR0_PFC1_U1ME_MASK) | SDR0_PFC1_U1ME_CTS_RTS;

	mfsdr(SDR0_PFC2, sdr0_pfc2);
	sdr0_pfc2 = (sdr0_pfc2 & ~SDR0_PFC2_SELECT_MASK) |
		SDR0_PFC2_SELECT_CONFIG_4;
	mtsdr(SDR0_PFC2, sdr0_pfc2);
	mtsdr(SDR0_PFC1, sdr0_pfc1);

	/* PCI arbiter enabled */
	mfsdr(SDR0_PCI0, reg);
	mtsdr(SDR0_PCI0, 0x80000000 | reg);

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

int misc_init_r(void)
{
#if !defined(CONFIG_SYS_NO_FLASH)
	uint pbcr;
	int size_val = 0;
#endif
#ifdef CONFIG_440EPX
	unsigned long usb2d0cr = 0;
	unsigned long usb2phy0cr, usb2h0cr = 0;
	unsigned long sdr0_pfc1;
	char *act = getenv("usbact");
#endif
	u32 reg;

#if !defined(CONFIG_SYS_NO_FLASH)
	/* Re-do flash sizing to get full correct info */

	/* adjust flash start and offset */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtdcr(EBC0_CFGADDR, PB3CR);
#else
	mtdcr(EBC0_CFGADDR, PB0CR);
#endif
	pbcr = mfdcr(EBC0_CFGDATA);
	size_val = ffs(gd->bd->bi_flashsize) - 21;
	pbcr = (pbcr & 0x0001ffff) | gd->bd->bi_flashstart | (size_val << 17);
#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL)
	mtdcr(EBC0_CFGADDR, PB3CR);
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
#endif /* CONFIG_SYS_NO_FLASH */

	/*
	 * USB suff...
	 */
#ifdef CONFIG_440EPX
	if (act == NULL || strcmp(act, "hostdev") == 0) {
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

		/*
		 * To enable the USB 2.0 Device function
		 * through the UTMI interface
		 */
		usb2d0cr = usb2d0cr &~SDR0_USB2D0CR_USB2DEV_EBC_SEL_MASK;
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_USB2DEV_SELECTION;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_USB2D_SEL;

		mtsdr(SDR0_PFC1, sdr0_pfc1);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2H0CR, usb2h0cr);

		/*clear resets*/
		udelay (1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Host(int phy) Device(ext phy)\n");

	} else if (strcmp(act, "dev") == 0) {
		/*-------------------PATCH-------------------------------*/
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
		/*-------------------PATCH-------------------------------*/

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
		usb2d0cr = usb2d0cr | SDR0_USB2D0CR_EBC_SELECTION;

		sdr0_pfc1 = sdr0_pfc1 &~SDR0_PFC1_UES_MASK;
		sdr0_pfc1 = sdr0_pfc1 | SDR0_PFC1_UES_EBCHR_SEL;

		mtsdr(SDR0_USB2H0CR, usb2h0cr);
		mtsdr(SDR0_USB2PHY0CR, usb2phy0cr);
		mtsdr(SDR0_USB2D0CR, usb2d0cr);
		mtsdr(SDR0_PFC1, sdr0_pfc1);

		/* clear resets */
		udelay (1000);
		mtsdr(SDR0_SRST1, 0x00000000);
		udelay (1000);
		mtsdr(SDR0_SRST0, 0x00000000);

		printf("USB:   Device(int phy)\n");
	}
#endif /* CONFIG_440EPX */

	mfsdr(SDR0_SRST1, reg);		/* enable security/kasumi engines */
	reg &= ~(SDR0_SRST1_CRYP0 | SDR0_SRST1_KASU0);
	mtsdr(SDR0_SRST1, reg);

	/*
	 * Clear PLB4A0_ACR[WRP]
	 * This fix will make the MAL burst disabling patch for the Linux
	 * EMAC driver obsolete.
	 */
	reg = mfdcr(PLB4_ACR) & ~PLB4_ACR_WRP;
	mtdcr(PLB4_ACR, reg);

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");
	u8 rev;
	u32 clock = get_async_pci_freq();

#ifdef CONFIG_440EPX
	printf("Board: Sequoia - AMCC PPC440EPx Evaluation Board");
#else
	printf("Board: Rainier - AMCC PPC440GRx Evaluation Board");
#endif

	rev = in_8((void *)(CONFIG_SYS_BCSR_BASE + 0));
	printf(", Rev. %X, PCI-Async=%d MHz", rev, clock / 1000000);

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	/*
	 * Reconfiguration of the PCI sync clock is already done,
	 * now check again if everything is in range:
	 */
	if (ppc4xx_pci_sync_clock_config(clock)) {
		printf("ERROR: PCI clocking incorrect (async=%d "
		       "sync=%ld)!\n", clock, get_PCI_freq());
	}

	return (0);
}

#if defined(CONFIG_PCI) && defined(CONFIG_PCI_PNP)
/*
 * Assign interrupts to PCI devices.
 */
void sequoia_pci_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, VECNUM_EIRQ2);
}
#endif

/*
 * pci_pre_init
 *
 * This routine is called just prior to registering the hose and gives
 * the board the opportunity to check things. Returning a value of zero
 * indicates that things are bad & PCI initialization should be aborted.
 *
 * Different boards may wish to customize the pci controller structure
 * (add regions, override default access routines, etc) or perform
 * certain pre-initialization actions.
 */
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long addr;

	/*
	 * Set priority for all PLB3 devices to 0.
	 * Set PLB3 arbiter to fair mode.
	 */
	mfsdr(SD0_AMP1, addr);
	mtsdr(SD0_AMP1, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(PLB3_ACR);
	mtdcr(PLB3_ACR, addr | 0x80000000);

	/*
	 * Set priority for all PLB4 devices to 0.
	 */
	mfsdr(SD0_AMP0, addr);
	mtsdr(SD0_AMP0, (addr & 0x000000FF) | 0x0000FF00);
	addr = mfdcr(PLB4_ACR) | 0xa0000000;	/* Was 0x8---- */
	mtdcr(PLB4_ACR, addr);

	/*
	 * Set Nebula PLB4 arbiter to fair mode.
	 */
	/* Segment0 */
	addr = (mfdcr(PLB0_ACR) & ~PLB0_ACR_PPM_MASK) | PLB0_ACR_PPM_FAIR;
	addr = (addr & ~PLB0_ACR_HBU_MASK) | PLB0_ACR_HBU_ENABLED;
	addr = (addr & ~PLB0_ACR_RDP_MASK) | PLB0_ACR_RDP_4DEEP;
	addr = (addr & ~PLB0_ACR_WRP_MASK) | PLB0_ACR_WRP_2DEEP;
	mtdcr(PLB0_ACR, addr);

	/* Segment1 */
	addr = (mfdcr(PLB1_ACR) & ~PLB1_ACR_PPM_MASK) | PLB1_ACR_PPM_FAIR;
	addr = (addr & ~PLB1_ACR_HBU_MASK) | PLB1_ACR_HBU_ENABLED;
	addr = (addr & ~PLB1_ACR_RDP_MASK) | PLB1_ACR_RDP_4DEEP;
	addr = (addr & ~PLB1_ACR_WRP_MASK) | PLB1_ACR_WRP_2DEEP;
	mtdcr(PLB1_ACR, addr);

#ifdef CONFIG_PCI_PNP
	hose->fixup_irq = sequoia_pci_fixup_irq;
#endif
	return 1;
}
#endif /* defined(CONFIG_PCI) */

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
	/*
	 * Set up Direct MMIO registers
	 */
	/*
	 * PowerPC440EPX PCI Master configuration.
	 * Map one 1Gig range of PLB/processor addresses to PCI memory space.
	 * PLB address 0xA0000000-0xDFFFFFFF
	 *     ==> PCI address 0xA0000000-0xDFFFFFFF
	 * Use byte reversed out routines to handle endianess.
	 * Make this region non-prefetchable.
	 */
	out32r(PCIL0_PMM0MA, 0x00000000);	/* PMM0 Mask/Attribute */
						/* - disabled b4 setting */
	out32r(PCIL0_PMM0LA, CONFIG_SYS_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIL0_PMM0PCILA, CONFIG_SYS_PCI_MEMBASE); /* PMM0 PCI Low Address */
	out32r(PCIL0_PMM0PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_PMM0MA, 0xE0000001);	/* 512M + No prefetching, */
						/* and enable region */

	out32r(PCIL0_PMM1MA, 0x00000000);	/* PMM0 Mask/Attribute */
						/* - disabled b4 setting */
	out32r(PCIL0_PMM1LA, CONFIG_SYS_PCI_MEMBASE2); /* PMM0 Local Address */
	out32r(PCIL0_PMM1PCILA, CONFIG_SYS_PCI_MEMBASE2); /* PMM0 PCI Low Address */
	out32r(PCIL0_PMM1PCIHA, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_PMM1MA, 0xE0000001);	/* 512M + No prefetching, */
						/* and enable region */

	out32r(PCIL0_PTM1MS, 0x00000001);	/* Memory Size/Attribute */
	out32r(PCIL0_PTM1LA, 0);		/* Local Addr. Reg */
	out32r(PCIL0_PTM2MS, 0);		/* Memory Size/Attribute */
	out32r(PCIL0_PTM2LA, 0);		/* Local Addr. Reg */

	/*
	 * Set up Configuration registers
	 */

	/* Program the board's subsystem id/vendor id */
	pci_write_config_word(0, PCI_SUBSYSTEM_VENDOR_ID,
			      CONFIG_SYS_PCI_SUBSYS_VENDORID);
	pci_write_config_word(0, PCI_SUBSYSTEM_ID, CONFIG_SYS_PCI_SUBSYS_ID);

	/* Configure command register as bus master */
	pci_write_config_word(0, PCI_COMMAND, PCI_COMMAND_MASTER);

	/* 240nS PCI clock */
	pci_write_config_word(0, PCI_LATENCY_TIMER, 1);

	/* No error reporting */
	pci_write_config_word(0, PCI_ERREN, 0);

	pci_write_config_dword(0, PCI_BRDGOPT2, 0x00000101);

}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	unsigned short temp_short;

	/*
	 * Write the PowerPC440 EP PCI Configuration regs.
	 * Enable PowerPC440 EP to be a master on the PCI bus (PMM).
	 * Enable PowerPC440 EP to act as a PCI memory target (PTM).
	 */
	pci_read_config_word(0, PCI_COMMAND, &temp_short);
	pci_write_config_word(0, PCI_COMMAND,
			      temp_short | PCI_COMMAND_MASTER |
			      PCI_COMMAND_MEMORY);
}
#endif /* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT) */

/*
 * is_pci_host
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
	/* Cactus is always configured as host. */
	return (1);
}
#endif /* defined(CONFIG_PCI) */

#if defined(CONFIG_POST)
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return 0;	/* No hotkeys supported */
}
#endif /* CONFIG_POST */

#if defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_SYS_RAMBOOT)
/*
 * On NAND-booting sequoia, we need to patch the chips select numbers
 * in the dtb (CS0 - NAND, CS3 - NOR)
 */
void ft_board_setup(void *blob, bd_t *bd)
{
	int rc;
	int len;
	int nodeoffset;
	struct fdt_property *prop;
	u32 *reg;
	char path[32];

	/* First do common fdt setup */
	__ft_board_setup(blob, bd);

	/* And now configure NOR chip select to 3 instead of 0 */
	strcpy(path, "/plb/opb/ebc/nor_flash@0,0");
	nodeoffset = fdt_path_offset(blob, path);
	prop = fdt_get_property_w(blob, nodeoffset, "reg", &len);
	if (prop == NULL) {
		printf("Unable to update NOR chip select for NAND booting\n");
		return;
	}
	reg = (u32 *)&prop->data[0];
	reg[0] = 3;
	rc = fdt_find_and_setprop(blob, path, "reg", reg, 3 * sizeof(u32), 1);
	if (rc) {
		printf("Unable to update property NOR mappings, err=%s\n",
		       fdt_strerror(rc));
		return;
	}

	/* And now configure NAND chip select to 0 instead of 3 */
	strcpy(path, "/plb/opb/ebc/ndfc@3,0");
	nodeoffset = fdt_path_offset(blob, path);
	prop = fdt_get_property_w(blob, nodeoffset, "reg", &len);
	if (prop == NULL) {
		printf("Unable to update NDFC chip select for NAND booting\n");
		return;
	}
	reg = (u32 *)&prop->data[0];
	reg[0] = 0;
	rc = fdt_find_and_setprop(blob, path, "reg", reg, 3 * sizeof(u32), 1);
	if (rc) {
		printf("Unable to update property NDFC mappings, err=%s\n",
		       fdt_strerror(rc));
		return;
	}
}
#endif /* CONFIG_NAND_U_BOOT */
