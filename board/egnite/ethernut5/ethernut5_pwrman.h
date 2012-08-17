/*
 * (C) Copyright 2011
 * egnite GmbH <info@egnite.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Ethernut 5 power management support
 *
 * For additional information visit the project home page at
 * http://www.ethernut.de/
 */

/* I2C address of the PMC */
#define PWRMAN_I2C_ADDR 0x22

/* PMC registers */
#define PWRMAN_REG_VERS		0	/* Version register */
#define PWRMAN_REG_STA		1	/* Feature status register */
#define PWRMAN_REG_ENA		2	/* Feature enable register */
#define PWRMAN_REG_DIS		3	/* Feature disable register */
#define PWRMAN_REG_TEMP		4	/* Board temperature */
#define PWRMAN_REG_VAUX		6	/* Auxiliary input voltage */
#define PWRMAN_REG_LEDCTL	8	/* LED blinking timer. */

/* Feature flags used in status, enable and disable registers */
#define PWRMAN_BOARD	0x01	/* 1.8V and 3.3V supply */
#define PWRMAN_VBIN	0x02	/* VBUS input at device connector */
#define PWRMAN_VBOUT	0x04	/* VBUS output at host connector */
#define PWRMAN_MMC	0x08	/* Memory card supply */
#define PWRMAN_RS232	0x10	/* RS-232 driver shutdown */
#define PWRMAN_ETHCLK	0x20	/* Ethernet clock enable */
#define PWRMAN_ETHRST	0x40	/* Ethernet PHY reset */
#define PWRMAN_WAKEUP	0x80	/* RTC wake-up */

/* Features, which are not essential to keep u-boot alive */
#define PWRMAN_DISPENSIBLE	(PWRMAN_VBOUT | PWRMAN_MMC | PWRMAN_ETHCLK)

/* Enable Ethernut 5 power management. */
extern void ethernut5_power_init(void);

/* Reset Ethernet PHY. */
extern void ethernut5_phy_reset(void);

extern void ethernut5_print_version(void);

#ifdef CONFIG_CMD_BSP
extern void ethernut5_print_power(void);
extern void ethernut5_print_celsius(void);
extern void ethernut5_print_voltage(void);
#endif
