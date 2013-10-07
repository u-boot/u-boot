/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <pci.h>

void show_reset_reg(void)
{
	unsigned long reg;

	/* read clock regsiter */
	printf("===== Display reset and initialize register Start =========\n");
	mfcpr(CPR0_PLLC,reg);
	printf("cpr_pllc   = %#010lx\n",reg);

	mfcpr(CPR0_PLLD,reg);
	printf("cpr_plld   = %#010lx\n",reg);

	mfcpr(CPR0_PRIMAD0,reg);
	printf("cpr_primad = %#010lx\n",reg);

	mfcpr(CPR0_PRIMBD0,reg);
	printf("cpr_primbd = %#010lx\n",reg);

	mfcpr(CPR0_OPBD0,reg);
	printf("cpr_opbd   = %#010lx\n",reg);

	mfcpr(CPR0_PERD,reg);
	printf("cpr_perd   = %#010lx\n",reg);

	mfcpr(CPR0_MALD,reg);
	printf("cpr_mald   = %#010lx\n",reg);

	/* read sdr register */
	mfsdr(SDR0_EBC,reg);
	printf("SDR0_EBC    = %#010lx\n",reg);

	mfsdr(SDR0_CP440,reg);
	printf("SDR0_CP440  = %#010lx\n",reg);

	mfsdr(SDR0_XCR,reg);
	printf("SDR0_XCR    = %#010lx\n",reg);

	mfsdr(SDR0_XPLLC,reg);
	printf("SDR0_XPLLC  = %#010lx\n",reg);

	mfsdr(SDR0_XPLLD,reg);
	printf("SDR0_XPLLD  = %#010lx\n",reg);

	mfsdr(SDR0_PFC0,reg);
	printf("SDR0_PFC0   = %#010lx\n",reg);

	mfsdr(SDR0_PFC1,reg);
	printf("SDR0_PFC1   = %#010lx\n",reg);

	mfsdr(SDR0_CUST0,reg);
	printf("SDR0_CUST0  = %#010lx\n",reg);

	mfsdr(SDR0_CUST1,reg);
	printf("SDR0_CUST1  = %#010lx\n",reg);

	mfsdr(SDR0_UART0,reg);
	printf("SDR0_UART0  = %#010lx\n",reg);

	mfsdr(SDR0_UART1,reg);
	printf("SDR0_UART1  = %#010lx\n",reg);

	printf("===== Display reset and initialize register End   =========\n");
}

void show_xbridge_info(void)
{
	unsigned long reg;

	printf("PCI-X chip control registers\n");
	mfsdr(SDR0_XCR, reg);
	printf("SDR0_XCR    = %#010lx\n", reg);

	mfsdr(SDR0_XPLLC, reg);
	printf("SDR0_XPLLC  = %#010lx\n", reg);

	mfsdr(SDR0_XPLLD, reg);
	printf("SDR0_XPLLD  = %#010lx\n", reg);

	printf("PCI-X Bridge Configure registers\n");
	printf("PCIL0_VENDID            = %#06x\n", in16r(PCIL0_VENDID));
	printf("PCIL0_DEVID             = %#06x\n", in16r(PCIL0_DEVID));
	printf("PCIL0_CMD               = %#06x\n", in16r(PCIL0_CMD));
	printf("PCIL0_STATUS            = %#06x\n", in16r(PCIL0_STATUS));
	printf("PCIL0_REVID             = %#04x\n", in8(PCIL0_REVID));
	printf("PCIL0_CACHELS           = %#04x\n", in8(PCIL0_CACHELS));
	printf("PCIL0_LATTIM            = %#04x\n", in8(PCIL0_LATTIM));
	printf("PCIL0_HDTYPE            = %#04x\n", in8(PCIL0_HDTYPE));
	printf("PCIL0_BIST              = %#04x\n", in8(PCIL0_BIST));

	printf("PCIL0_BAR0              = %#010lx\n", in32r(PCIL0_BAR0));
	printf("PCIL0_BAR1              = %#010lx\n", in32r(PCIL0_BAR1));
	printf("PCIL0_BAR2              = %#010lx\n", in32r(PCIL0_BAR2));
	printf("PCIL0_BAR3              = %#010lx\n", in32r(PCIL0_BAR3));
	printf("PCIL0_BAR4              = %#010lx\n", in32r(PCIL0_BAR4));
	printf("PCIL0_BAR5              = %#010lx\n", in32r(PCIL0_BAR5));

	printf("PCIL0_CISPTR            = %#010lx\n", in32r(PCIL0_CISPTR));
	printf("PCIL0_SBSSYSVID         = %#010x\n", in16r(PCIL0_SBSYSVID));
	printf("PCIL0_SBSSYSID          = %#010x\n", in16r(PCIL0_SBSYSID));
	printf("PCIL0_EROMBA            = %#010lx\n", in32r(PCIL0_EROMBA));
	printf("PCIL0_CAP               = %#04x\n", in8(PCIL0_CAP));
	printf("PCIL0_INTLN             = %#04x\n", in8(PCIL0_INTLN));
	printf("PCIL0_INTPN             = %#04x\n", in8(PCIL0_INTPN));
	printf("PCIL0_MINGNT            = %#04x\n", in8(PCIL0_MINGNT));
	printf("PCIL0_MAXLTNCY          = %#04x\n", in8(PCIL0_MAXLTNCY));

	printf("PCIL0_BRDGOPT1          = %#010lx\n", in32r(PCIL0_BRDGOPT1));
	printf("PCIL0_BRDGOPT2          = %#010lx\n", in32r(PCIL0_BRDGOPT2));

	printf("PCIL0_POM0LAL           = %#010lx\n", in32r(PCIL0_POM0LAL));
	printf("PCIL0_POM0LAH           = %#010lx\n", in32r(PCIL0_POM0LAH));
	printf("PCIL0_POM0SA            = %#010lx\n", in32r(PCIL0_POM0SA));
	printf("PCIL0_POM0PCILAL        = %#010lx\n", in32r(PCIL0_POM0PCIAL));
	printf("PCIL0_POM0PCILAH        = %#010lx\n", in32r(PCIL0_POM0PCIAH));
	printf("PCIL0_POM1LAL           = %#010lx\n", in32r(PCIL0_POM1LAL));
	printf("PCIL0_POM1LAH           = %#010lx\n", in32r(PCIL0_POM1LAH));
	printf("PCIL0_POM1SA            = %#010lx\n", in32r(PCIL0_POM1SA));
	printf("PCIL0_POM1PCILAL        = %#010lx\n", in32r(PCIL0_POM1PCIAL));
	printf("PCIL0_POM1PCILAH        = %#010lx\n", in32r(PCIL0_POM1PCIAH));
	printf("PCIL0_POM2SA            = %#010lx\n", in32r(PCIL0_POM2SA));

	printf("PCIL0_PIM0SA            = %#010lx\n", in32r(PCIL0_PIM0SA));
	printf("PCIL0_PIM0LAL           = %#010lx\n", in32r(PCIL0_PIM0LAL));
	printf("PCIL0_PIM0LAH           = %#010lx\n", in32r(PCIL0_PIM0LAH));
	printf("PCIL0_PIM1SA            = %#010lx\n", in32r(PCIL0_PIM1SA));
	printf("PCIL0_PIM1LAL           = %#010lx\n", in32r(PCIL0_PIM1LAL));
	printf("PCIL0_PIM1LAH           = %#010lx\n", in32r(PCIL0_PIM1LAH));
	printf("PCIL0_PIM2SA            = %#010lx\n", in32r(PCIL0_PIM1SA));
	printf("PCIL0_PIM2LAL           = %#010lx\n", in32r(PCIL0_PIM1LAL));
	printf("PCIL0_PIM2LAH           = %#010lx\n", in32r(PCIL0_PIM1LAH));

	printf("PCIL0_XSTS              = %#010lx\n", in32r(PCIL0_STS));
}

int do_show_xbridge_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	show_xbridge_info();
	return 0;
}

U_BOOT_CMD(xbriinfo, 1, 1, do_show_xbridge_info,
	   "Show PCIX bridge info", "");

#define TAISHAN_PCI_DEV_ID0 0x800
#define TAISHAN_PCI_DEV_ID1 0x1000

void show_pcix_device_info(void)
{
	int ii;
	int dev;
	u8 capp;
	u8 xcapid;
	u16 status;
	u16 xcommand;
	u32 xstatus;

	for (ii = 0; ii < 2; ii++) {
		if (ii == 0)
			dev = TAISHAN_PCI_DEV_ID0;
		else
			dev = TAISHAN_PCI_DEV_ID1;

		pci_read_config_word(dev, PCI_STATUS, &status);
		if (status & PCI_STATUS_CAP_LIST) {
			pci_read_config_byte(dev, PCI_CAPABILITY_LIST, &capp);

			pci_read_config_byte(dev, (int)(capp), &xcapid);
			if (xcapid == 0x07) {
				pci_read_config_word(dev, (int)(capp + 2),
						     &xcommand);
				pci_read_config_dword(dev, (int)(capp + 4),
						      &xstatus);
				printf("BUS0 dev%d Xcommand=%#06x,Xstatus=%#010x\n",
				       (ii + 1), xcommand, xstatus);
			} else {
				printf("BUS0 dev%d PCI-X CAP ID error,"
				       "CAP=%#04x,XCAPID=%#04x\n",
				       (ii + 1), capp, xcapid);
			}
		} else {
			printf("BUS0 dev%d not found PCI_STATUS_CAP_LIST supporting\n",
			       ii + 1);
		}
	}

}

int do_show_pcix_device_info(cmd_tbl_t * cmdtp, int flag, int argc,
			     char * const argv[])
{
	show_pcix_device_info();
	return 0;
}

U_BOOT_CMD(xdevinfo, 1, 1, do_show_pcix_device_info,
	   "Show PCIX Device info", "");

extern void show_reset_reg(void);

int do_show_reset_reg_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	show_reset_reg();
	return 0;
}

U_BOOT_CMD(resetinfo, 1, 1, do_show_reset_reg_info,
	   "Show Reset REG info", "");
