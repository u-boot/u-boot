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

#define MACH_TYPE_DAVINCI_EVM		901

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
	DECLARE_GLOBAL_DATA_PTR;

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_EVM;

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

	clk = ((REG(PLL2_PLLM) + 1) * 27) / ((REG(PLL2_DIV2) & 0x1f) + 1);

	printf ("ARM Clock : %dMHz\n", ((REG(PLL1_PLLM) + 1) * 27 ) / 2);
	printf ("DDR Clock : %dMHz\n", (clk / 2));

	/* Set Ethernet MAC address from EEPROM */
	if (i2c_read(CFG_I2C_EEPROM_ADDR, 0x7f00, CFG_I2C_EEPROM_ADDR_LEN, buf, 6)) {
		printf("\nEEPROM @ 0x%02x read FAILED!!!\n", CFG_I2C_EEPROM_ADDR);
	} else {
		tmp[0] = 0xff;
		for (i = 0; i < 6; i++)
			tmp[0] &= buf[i];

		if ((tmp[0] != 0xff) && (getenv("ethaddr") == NULL)) {
			sprintf((char *)&tmp[0], "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
				buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
			setenv("ethaddr", (char *)&tmp[0]);
		}
	}

	if (!eth_hw_init()) {
		printf("ethernet init failed!\n");
	} else {
		printf("ETH PHY   : %s\n", phy.name);
	}

	i2c_read (0x39, 0x00, 1, (u_int8_t *)&i, 1);

	setenv ("videostd", ((i  & 0x80) ? "pal" : "ntsc"));

	return(0);
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return(0);
}
