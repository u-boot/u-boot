/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts are shamelessly stolen from various TI sources, original copyright
 * follows:
 * -----------------------------------------------------------------
 *
 * Copyright (C) 2004 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emac_defs.h>

#define MACH_TYPE_SCHMOOGIE		1255

DECLARE_GLOBAL_DATA_PTR;

extern void	i2c_init(int speed, int slaveaddr);
extern void	timer_init(void);
extern int	eth_hw_init(void);
extern phy_t	phy;


/* Works on Always On power domain only (no PD argument) */
void lpsc_on(unsigned int id)
{
	dv_reg_p	mdstat, mdctl;

	if (id >= DAVINCI_LPSC_GEM)
		return;			/* Don't work on DSP Power Domain */

	mdstat = REG_P(PSC_MDSTAT_BASE + (id * 4));
	mdctl = REG_P(PSC_MDCTL_BASE + (id * 4));

	while (REG(PSC_PTSTAT) & 0x01) {;}

	if ((*mdstat & 0x1f) == 0x03)
		return;			/* Already on and enabled */

	*mdctl |= 0x03;

	/* Special treatment for some modules as for sprue14 p.7.4.2 */
	if (	(id == DAVINCI_LPSC_VPSSSLV) ||
		(id == DAVINCI_LPSC_EMAC) ||
		(id == DAVINCI_LPSC_EMAC_WRAPPER) ||
		(id == DAVINCI_LPSC_MDIO) ||
		(id == DAVINCI_LPSC_USB) ||
		(id == DAVINCI_LPSC_ATA) ||
		(id == DAVINCI_LPSC_VLYNQ) ||
		(id == DAVINCI_LPSC_UHPI) ||
		(id == DAVINCI_LPSC_DDR_EMIF) ||
		(id == DAVINCI_LPSC_AEMIF) ||
		(id == DAVINCI_LPSC_MMC_SD) ||
		(id == DAVINCI_LPSC_MEMSTICK) ||
		(id == DAVINCI_LPSC_McBSP) ||
		(id == DAVINCI_LPSC_GPIO)
	   )
	   	*mdctl |= 0x200;

	REG(PSC_PTCMD) = 0x01;

	while (REG(PSC_PTSTAT) & 0x03) {;}
	while ((*mdstat & 0x1f) != 0x03) {;}	/* Probably an overkill... */
}

void dsp_on(void)
{
	int	i;

	if (REG(PSC_PDSTAT1) & 0x1f)
		return;			/* Already on */

	REG(PSC_GBLCTL) |= 0x01;
	REG(PSC_PDCTL1) |= 0x01;
	REG(PSC_PDCTL1) &= ~0x100;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) &= 0xfffffeff;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) &= 0xfffffeff;
	REG(PSC_PTCMD) = 0x02;

	for (i = 0; i < 100; i++) {
		if (REG(PSC_EPCPR) & 0x02)
			break;
	}

	REG(PSC_CHP_SHRTSW) = 0x01;
	REG(PSC_PDCTL1) |= 0x100;
	REG(PSC_EPCCR) = 0x02;

	for (i = 0; i < 100; i++) {
		if (!(REG(PSC_PTSTAT) & 0x02))
			break;
	}

	REG(PSC_GBLCTL) &= ~0x1f;
}


int board_init(void)
{
	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_SCHMOOGIE;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* Workaround for TMS320DM6446 errata 1.3.22 */
	REG(PSC_SILVER_BULLET) = 0;

	/* Power on required peripherals */
	lpsc_on(DAVINCI_LPSC_EMAC);
	lpsc_on(DAVINCI_LPSC_EMAC_WRAPPER);
	lpsc_on(DAVINCI_LPSC_MDIO);
	lpsc_on(DAVINCI_LPSC_I2C);
	lpsc_on(DAVINCI_LPSC_UART0);
	lpsc_on(DAVINCI_LPSC_TIMER1);
	lpsc_on(DAVINCI_LPSC_GPIO);

	/* Powerup the DSP */
	dsp_on();

	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x0000e003;

	/* Enable GIO3.3V cells used for EMAC */
	REG(VDD3P3V_PWDN) = 0;

	/* Enable UART0 MUX lines */
	REG(PINMUX1) |= 1;

	/* Enable EMAC and AEMIF pins */
	REG(PINMUX0) = 0x80000c1f;

	/* Enable I2C pin Mux */
	REG(PINMUX1) |= (1 << 7);

	/* Set the Bus Priority Register to appropriate value */
	REG(VBPR) = 0x20;

	timer_init();

	return(0);
}

int misc_init_r (void)
{
	u_int8_t	tmp[20], buf[10];
	int		i = 0;
	int		clk = 0;

	/* Set serial number from UID chip */
	u_int8_t	crc_tbl[256] = {
			0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
			0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
			0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
			0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
			0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
			0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
			0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
			0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
			0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
			0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
			0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
			0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
			0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
			0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
			0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
			0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
			0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
			0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
			0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
			0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
			0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
			0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
			0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
			0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
			0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
			0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
			0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
			0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
			0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
			0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
			0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
			0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35
		};

	clk = ((REG(PLL2_PLLM) + 1) * 27) / ((REG(PLL2_DIV2) & 0x1f) + 1);

	printf ("ARM Clock : %dMHz\n", ((REG(PLL1_PLLM) + 1) * 27 ) / 2);
	printf ("DDR Clock : %dMHz\n", (clk / 2));

	/* Set serial number from UID chip */
	if (i2c_read(CFG_UID_ADDR, 0, 1, buf, 8)) {
		printf("\nUID @ 0x%02x read FAILED!!!\n", CFG_UID_ADDR);
		forceenv("serial#", "FAILED");
	} else {
		if (buf[0] != 0x70) {	/* Device Family Code */
			printf("\nUID @ 0x%02x read FAILED!!!\n", CFG_UID_ADDR);
			forceenv("serial#", "FAILED");
		}
	}
	/* Now check CRC */
	tmp[0] = 0;
	for (i = 0; i < 8; i++)
		tmp[0] = crc_tbl[tmp[0] ^ buf[i]];

	if (tmp[0] != 0) {
		printf("\nUID @ 0x%02x - BAD CRC!!!\n", CFG_UID_ADDR);
		forceenv("serial#", "FAILED");
	} else {
		/* CRC OK, set "serial" env variable */
		sprintf((char *)&tmp[0], "%02x%02x%02x%02x%02x%02x",
			buf[6], buf[5], buf[4], buf[3], buf[2], buf[1]);
		forceenv("serial#", (char *)&tmp[0]);
	}

	if (!eth_hw_init()) {
		printf("ethernet init failed!\n");
	} else {
		printf("ETH PHY   : %s\n", phy.name);
	}

	return(0);
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return(0);
}
