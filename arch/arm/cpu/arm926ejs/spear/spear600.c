/*
 * (C) Copyright 2000-2009
 * Viresh Kumar, ST Microelectronics, viresh.kumar@st.com
 * Vipin Kumar, ST Microelectronics, vipin.kumar@st.com
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

#include <common.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/arch/spr_misc.h>
#include <asm/arch/spr_defs.h>

#define FALSE				0
#define TRUE				(!FALSE)

static void sel_1v8(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 ddr1v8, ddr2v5;

	ddr2v5 = readl(&misc_p->ddr_2v5_compensation);
	ddr2v5 &= 0x8080ffc0;
	ddr2v5 |= 0x78000003;
	writel(ddr2v5, &misc_p->ddr_2v5_compensation);

	ddr1v8 = readl(&misc_p->ddr_1v8_compensation);
	ddr1v8 &= 0x8080ffc0;
	ddr1v8 |= 0x78000010;
	writel(ddr1v8, &misc_p->ddr_1v8_compensation);

	while (!(readl(&misc_p->ddr_1v8_compensation) & DDR_COMP_ACCURATE))
		;
}

static void sel_2v5(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 ddr1v8, ddr2v5;

	ddr1v8 = readl(&misc_p->ddr_1v8_compensation);
	ddr1v8 &= 0x8080ffc0;
	ddr1v8 |= 0x78000003;
	writel(ddr1v8, &misc_p->ddr_1v8_compensation);

	ddr2v5 = readl(&misc_p->ddr_2v5_compensation);
	ddr2v5 &= 0x8080ffc0;
	ddr2v5 |= 0x78000010;
	writel(ddr2v5, &misc_p->ddr_2v5_compensation);

	while (!(readl(&misc_p->ddr_2v5_compensation) & DDR_COMP_ACCURATE))
		;
}

/*
 * plat_ddr_init:
 */
void plat_ddr_init(void)
{
	struct misc_regs *misc_p = (struct misc_regs *)CONFIG_SPEAR_MISCBASE;
	u32 ddrpad;
	u32 core3v3, ddr1v8, ddr2v5;

	/* DDR pad register configurations */
	ddrpad = readl(&misc_p->ddr_pad);
	ddrpad &= ~DDR_PAD_CNF_MSK;

#if (CONFIG_DDR_HCLK)
	ddrpad |= 0xEAAB;
#elif (CONFIG_DDR_2HCLK)
	ddrpad |= 0xEAAD;
#elif (CONFIG_DDR_PLL2)
	ddrpad |= 0xEAAD;
#endif
	writel(ddrpad, &misc_p->ddr_pad);

	/* Compensation register configurations */
	core3v3 = readl(&misc_p->core_3v3_compensation);
	core3v3 &= 0x8080ffe0;
	core3v3 |= 0x78000002;
	writel(core3v3, &misc_p->core_3v3_compensation);

	ddr1v8 = readl(&misc_p->ddr_1v8_compensation);
	ddr1v8 &= 0x8080ffc0;
	ddr1v8 |= 0x78000004;
	writel(ddr1v8, &misc_p->ddr_1v8_compensation);

	ddr2v5 = readl(&misc_p->ddr_2v5_compensation);
	ddr2v5 &= 0x8080ffc0;
	ddr2v5 |= 0x78000004;
	writel(ddr2v5, &misc_p->ddr_2v5_compensation);

	if ((readl(&misc_p->ddr_pad) & DDR_PAD_SW_CONF) == DDR_PAD_SW_CONF) {
		/* Software memory configuration */
		if (readl(&misc_p->ddr_pad) & DDR_PAD_SSTL_SEL)
			sel_1v8();
		else
			sel_2v5();
	} else {
		/* Hardware memory configuration */
		if (readl(&misc_p->ddr_pad) & DDR_PAD_DRAM_TYPE)
			sel_1v8();
		else
			sel_2v5();
	}
}

/*
 * soc_init:
 */
void soc_init(void)
{
	/* Nothing to be done for SPEAr600 */
}

/*
 * xxx_boot_selected:
 *
 * return TRUE if the particular booting option is selected
 * return FALSE otherwise
 */
static u32 read_bootstrap(void)
{
	return (readl(CONFIG_SPEAR_BOOTSTRAPCFG) >> CONFIG_SPEAR_BOOTSTRAPSHFT)
		& CONFIG_SPEAR_BOOTSTRAPMASK;
}

int snor_boot_selected(void)
{
	u32 bootstrap = read_bootstrap();

	if (SNOR_BOOT_SUPPORTED) {
		/* Check whether SNOR boot is selected */
		if ((bootstrap & CONFIG_SPEAR_ONLYSNORBOOT) ==
			CONFIG_SPEAR_ONLYSNORBOOT)
			return TRUE;

		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND8BOOT)
			return TRUE;

		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND16BOOT)
			return TRUE;
	}

	return FALSE;
}

int nand_boot_selected(void)
{
	u32 bootstrap = read_bootstrap();

	if (NAND_BOOT_SUPPORTED) {
		/* Check whether NAND boot is selected */
		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND8BOOT)
			return TRUE;

		if ((bootstrap & CONFIG_SPEAR_NORNANDBOOT) ==
			CONFIG_SPEAR_NORNAND16BOOT)
			return TRUE;
	}

	return FALSE;
}

int pnor_boot_selected(void)
{
	/* Parallel NOR boot is not selected in any SPEAr600 revision */
	return FALSE;
}

int usb_boot_selected(void)
{
	u32 bootstrap = read_bootstrap();

	if (USB_BOOT_SUPPORTED) {
		/* Check whether USB boot is selected */
		if (!(bootstrap & CONFIG_SPEAR_USBBOOT))
			return TRUE;
	}

	return FALSE;
}

int tftp_boot_selected(void)
{
	/* TFTP boot is not selected in any SPEAr600 revision */
	return FALSE;
}

int uart_boot_selected(void)
{
	/* UART boot is not selected in any SPEAr600 revision */
	return FALSE;
}

int spi_boot_selected(void)
{
	/* SPI boot is not selected in any SPEAr600 revision */
	return FALSE;
}

int i2c_boot_selected(void)
{
	/* I2C boot is not selected in any SPEAr600 revision */
	return FALSE;
}

int mmc_boot_selected(void)
{
	return FALSE;
}

void plat_late_init(void)
{
	spear_late_init();
}
