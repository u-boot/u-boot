/*
 * (C) Copyright 2007
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
	mfcpr(clk_pllc,reg);
	printf("cpr_pllc   = %#010x\n",reg);

	mfcpr(clk_plld,reg);
	printf("cpr_plld   = %#010x\n",reg);

	mfcpr(clk_primad,reg);
	printf("cpr_primad = %#010x\n",reg);

	mfcpr(clk_primbd,reg);
	printf("cpr_primbd = %#010x\n",reg);

	mfcpr(clk_opbd,reg);
	printf("cpr_opbd   = %#010x\n",reg);

	mfcpr(clk_perd,reg);
	printf("cpr_perd   = %#010x\n",reg);

	mfcpr(clk_mald,reg);
	printf("cpr_mald   = %#010x\n",reg);

	/* read sdr register */
	mfsdr(sdr_ebc,reg);
	printf("sdr_ebc    = %#010x\n",reg);

	mfsdr(sdr_cp440,reg);
	printf("sdr_cp440  = %#010x\n",reg);

	mfsdr(sdr_xcr,reg);
	printf("sdr_xcr    = %#010x\n",reg);

	mfsdr(sdr_xpllc,reg);
	printf("sdr_xpllc  = %#010x\n",reg);

	mfsdr(sdr_xplld,reg);
	printf("sdr_xplld  = %#010x\n",reg);

	mfsdr(sdr_pfc0,reg);
	printf("sdr_pfc0   = %#010x\n",reg);

	mfsdr(sdr_pfc1,reg);
	printf("sdr_pfc1   = %#010x\n",reg);

	mfsdr(sdr_cust0,reg);
	printf("sdr_cust0  = %#010x\n",reg);

	mfsdr(sdr_cust1,reg);
	printf("sdr_cust1  = %#010x\n",reg);

	mfsdr(sdr_uart0,reg);
	printf("sdr_uart0  = %#010x\n",reg);

	mfsdr(sdr_uart1,reg);
	printf("sdr_uart1  = %#010x\n",reg);

	printf("===== Display reset and initialize register End   =========\n");
}

void show_xbridge_info(void)
{
	unsigned long reg;

	printf("PCI-X chip control registers\n");
	mfsdr(sdr_xcr, reg);
	printf("sdr_xcr    = %#010x\n", reg);

	mfsdr(sdr_xpllc, reg);
	printf("sdr_xpllc  = %#010x\n", reg);

	mfsdr(sdr_xplld, reg);
	printf("sdr_xplld  = %#010x\n", reg);

	printf("PCI-X Bridge Configure registers\n");
	printf("PCIX0_VENDID            = %#06x\n", in16r(PCIX0_VENDID));
	printf("PCIX0_DEVID             = %#06x\n", in16r(PCIX0_DEVID));
	printf("PCIX0_CMD               = %#06x\n", in16r(PCIX0_CMD));
	printf("PCIX0_STATUS            = %#06x\n", in16r(PCIX0_STATUS));
	printf("PCIX0_REVID             = %#04x\n", in8(PCIX0_REVID));
	printf("PCIX0_CACHELS           = %#04x\n", in8(PCIX0_CACHELS));
	printf("PCIX0_LATTIM            = %#04x\n", in8(PCIX0_LATTIM));
	printf("PCIX0_HDTYPE            = %#04x\n", in8(PCIX0_HDTYPE));
	printf("PCIX0_BIST              = %#04x\n", in8(PCIX0_BIST));

	printf("PCIX0_BAR0              = %#010x\n", in32r(PCIX0_BAR0));
	printf("PCIX0_BAR1              = %#010x\n", in32r(PCIX0_BAR1));
	printf("PCIX0_BAR2              = %#010x\n", in32r(PCIX0_BAR2));
	printf("PCIX0_BAR3              = %#010x\n", in32r(PCIX0_BAR3));
	printf("PCIX0_BAR4              = %#010x\n", in32r(PCIX0_BAR4));
	printf("PCIX0_BAR5              = %#010x\n", in32r(PCIX0_BAR5));

	printf("PCIX0_CISPTR            = %#010x\n", in32r(PCIX0_CISPTR));
	printf("PCIX0_SBSSYSVID         = %#010x\n", in16r(PCIX0_SBSYSVID));
	printf("PCIX0_SBSSYSID          = %#010x\n", in16r(PCIX0_SBSYSID));
	printf("PCIX0_EROMBA            = %#010x\n", in32r(PCIX0_EROMBA));
	printf("PCIX0_CAP               = %#04x\n", in8(PCIX0_CAP));
	printf("PCIX0_INTLN             = %#04x\n", in8(PCIX0_INTLN));
	printf("PCIX0_INTPN             = %#04x\n", in8(PCIX0_INTPN));
	printf("PCIX0_MINGNT            = %#04x\n", in8(PCIX0_MINGNT));
	printf("PCIX0_MAXLTNCY          = %#04x\n", in8(PCIX0_MAXLTNCY));

	printf("PCIX0_BRDGOPT1          = %#010x\n", in32r(PCIX0_BRDGOPT1));
	printf("PCIX0_BRDGOPT2          = %#010x\n", in32r(PCIX0_BRDGOPT2));

	printf("PCIX0_POM0LAL           = %#010x\n", in32r(PCIX0_POM0LAL));
	printf("PCIX0_POM0LAH           = %#010x\n", in32r(PCIX0_POM0LAH));
	printf("PCIX0_POM0SA            = %#010x\n", in32r(PCIX0_POM0SA));
	printf("PCIX0_POM0PCILAL        = %#010x\n", in32r(PCIX0_POM0PCIAL));
	printf("PCIX0_POM0PCILAH        = %#010x\n", in32r(PCIX0_POM0PCIAH));
	printf("PCIX0_POM1LAL           = %#010x\n", in32r(PCIX0_POM1LAL));
	printf("PCIX0_POM1LAH           = %#010x\n", in32r(PCIX0_POM1LAH));
	printf("PCIX0_POM1SA            = %#010x\n", in32r(PCIX0_POM1SA));
	printf("PCIX0_POM1PCILAL        = %#010x\n", in32r(PCIX0_POM1PCIAL));
	printf("PCIX0_POM1PCILAH        = %#010x\n", in32r(PCIX0_POM1PCIAH));
	printf("PCIX0_POM2SA            = %#010x\n", in32r(PCIX0_POM2SA));

	printf("PCIX0_PIM0SA            = %#010x\n", in32r(PCIX0_PIM0SA));
	printf("PCIX0_PIM0LAL           = %#010x\n", in32r(PCIX0_PIM0LAL));
	printf("PCIX0_PIM0LAH           = %#010x\n", in32r(PCIX0_PIM0LAH));
	printf("PCIX0_PIM1SA            = %#010x\n", in32r(PCIX0_PIM1SA));
	printf("PCIX0_PIM1LAL           = %#010x\n", in32r(PCIX0_PIM1LAL));
	printf("PCIX0_PIM1LAH           = %#010x\n", in32r(PCIX0_PIM1LAH));
	printf("PCIX0_PIM2SA            = %#010x\n", in32r(PCIX0_PIM1SA));
	printf("PCIX0_PIM2LAL           = %#010x\n", in32r(PCIX0_PIM1LAL));
	printf("PCIX0_PIM2LAH           = %#010x\n", in32r(PCIX0_PIM1LAH));

	printf("PCIX0_XSTS              = %#010x\n", in32r(PCIX0_STS));
}

int do_show_xbridge_info(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	show_xbridge_info();
	return 0;
}

U_BOOT_CMD(xbriinfo, 1, 1, do_show_xbridge_info,
	   "xbriinfo  - Show PCIX bridge info\n", NULL);

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
			     char *argv[])
{
	show_pcix_device_info();
	return 0;
}

U_BOOT_CMD(xdevinfo, 1, 1, do_show_pcix_device_info,
	   "xdevinfo  - Show PCIX Device info\n", NULL);

extern void show_reset_reg(void);

int do_show_reset_reg_info(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	show_reset_reg();
	return 0;
}

U_BOOT_CMD(resetinfo, 1, 1, do_show_reset_reg_info,
	   "resetinfo  - Show Reset REG info\n", NULL);
