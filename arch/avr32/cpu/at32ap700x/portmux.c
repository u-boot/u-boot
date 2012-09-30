/*
 * Copyright (C) 2006, 2008 Atmel Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>

#include <asm/io.h>

#include <asm/arch/chip-features.h>
#include <asm/arch/hardware.h>
#include <asm/arch/portmux.h>

/*
 * Lots of small functions here. We depend on --gc-sections getting
 * rid of the ones we don't need.
 */
void portmux_enable_ebi(unsigned int bus_width, unsigned int addr_width,
		unsigned long flags, unsigned long drive_strength)
{
	unsigned long porte_mask = 0;

	if (bus_width > 16)
		portmux_select_peripheral(PORTMUX_PORT_E, 0xffff,
				PORTMUX_FUNC_A, PORTMUX_BUSKEEPER);
	if (addr_width > 23)
		porte_mask |= (((1 << (addr_width - 23)) - 1) & 7) << 16;
	if (flags & PORTMUX_EBI_CS(2))
		porte_mask |= 1 << 25;
	if (flags & PORTMUX_EBI_CS(4))
		porte_mask |= 1 << 21;
	if (flags & PORTMUX_EBI_CS(5))
		porte_mask |= 1 << 22;
	if (flags & (PORTMUX_EBI_CF(0) | PORTMUX_EBI_CF(1)))
		porte_mask |= (1 << 19) | (1 << 20) | (1 << 23);

	portmux_select_peripheral(PORTMUX_PORT_E, porte_mask,
			PORTMUX_FUNC_A, 0);

	if (flags & PORTMUX_EBI_NWAIT)
		portmux_select_peripheral(PORTMUX_PORT_E, 1 << 24,
				PORTMUX_FUNC_A, PORTMUX_PULL_UP);
}

#ifdef AT32AP700x_CHIP_HAS_MACB
void portmux_enable_macb0(unsigned long flags, unsigned long drive_strength)
{
	unsigned long portc_mask;

	portc_mask = (1 << 3)	/* TXD0	*/
		| (1 << 4)	/* TXD1	*/
		| (1 << 7)	/* TXEN	*/
		| (1 << 8)	/* TXCK */
		| (1 << 9)	/* RXD0	*/
		| (1 << 10)	/* RXD1	*/
		| (1 << 13)	/* RXER	*/
		| (1 << 15)	/* RXDV	*/
		| (1 << 16)	/* MDC	*/
		| (1 << 17);	/* MDIO	*/

	if (flags & PORTMUX_MACB_MII)
		portc_mask |= (1 << 0)	/* COL	*/
			| (1 << 1)	/* CRS	*/
			| (1 << 2)	/* TXER	*/
			| (1 << 5)	/* TXD2	*/
			| (1 << 6)	/* TXD3 */
			| (1 << 11)	/* RXD2	*/
			| (1 << 12)	/* RXD3	*/
			| (1 << 14);	/* RXCK	*/

	if (flags & PORTMUX_MACB_SPEED)
		portc_mask |= (1 << 18);/* SPD	*/

	/* REVISIT: Some pins are probably pure outputs */
	portmux_select_peripheral(PORTMUX_PORT_C, portc_mask,
			PORTMUX_FUNC_A, PORTMUX_BUSKEEPER);
}

void portmux_enable_macb1(unsigned long flags, unsigned long drive_strength)
{
	unsigned long portc_mask = 0;
	unsigned long portd_mask;

	portd_mask = (1 << 13)	/* TXD0	*/
		| (1 << 14)	/* TXD1	*/
		| (1 << 11)	/* TXEN	*/
		| (1 << 12)	/* TXCK */
		| (1 << 10)	/* RXD0	*/
		| (1 << 6)	/* RXD1	*/
		| (1 << 5)	/* RXER	*/
		| (1 << 4)	/* RXDV	*/
		| (1 << 3)	/* MDC	*/
		| (1 << 2);	/* MDIO	*/

	if (flags & PORTMUX_MACB_MII)
		portc_mask = (1 << 19)	/* COL	*/
			| (1 << 23)	/* CRS	*/
			| (1 << 26)	/* TXER	*/
			| (1 << 27)	/* TXD2	*/
			| (1 << 28)	/* TXD3 */
			| (1 << 29)	/* RXD2	*/
			| (1 << 30)	/* RXD3	*/
			| (1 << 24);	/* RXCK	*/

	if (flags & PORTMUX_MACB_SPEED)
		portd_mask |= (1 << 15);/* SPD	*/

	/* REVISIT: Some pins are probably pure outputs */
	portmux_select_peripheral(PORTMUX_PORT_D, portd_mask,
			PORTMUX_FUNC_B, PORTMUX_BUSKEEPER);
	portmux_select_peripheral(PORTMUX_PORT_C, portc_mask,
			PORTMUX_FUNC_B, PORTMUX_BUSKEEPER);
}
#endif

#ifdef AT32AP700x_CHIP_HAS_MMCI
void portmux_enable_mmci(unsigned int slot, unsigned long flags,
		unsigned long drive_strength)
{
	unsigned long mask;
	unsigned long portmux_flags = PORTMUX_PULL_UP;

	/* First, the common CLK signal. It doesn't need a pull-up */
	portmux_select_peripheral(PORTMUX_PORT_A, 1 << 10,
			PORTMUX_FUNC_A, 0);

	if (flags & PORTMUX_MMCI_EXT_PULLUP)
		portmux_flags = 0;

	/* Then, the per-slot signals */
	switch (slot) {
	case 0:
		mask = (1 << 11) | (1 << 12);	/* CMD and DATA0 */
		if (flags & PORTMUX_MMCI_4BIT)
			/* DATA1..DATA3 */
			mask |= (1 << 13) | (1 << 14) | (1 << 15);
		portmux_select_peripheral(PORTMUX_PORT_A, mask,
				PORTMUX_FUNC_A, portmux_flags);
		break;
	case 1:
		mask = (1 << 6) | (1 << 7);	/* CMD and DATA0 */
		if (flags & PORTMUX_MMCI_4BIT)
			/* DATA1..DATA3 */
			mask |= (1 << 8) | (1 << 9) | (1 << 10);
		portmux_select_peripheral(PORTMUX_PORT_B, mask,
				PORTMUX_FUNC_B, portmux_flags);
		break;
	}
}
#endif

#ifdef AT32AP700x_CHIP_HAS_SPI
void portmux_enable_spi0(unsigned long cs_mask, unsigned long drive_strength)
{
	unsigned long pin_mask;

	/* MOSI and SCK */
	portmux_select_peripheral(PORTMUX_PORT_A, (1 << 1) | (1 << 2),
			PORTMUX_FUNC_A, 0);
	/* MISO may float */
	portmux_select_peripheral(PORTMUX_PORT_A, 1 << 0,
			PORTMUX_FUNC_A, PORTMUX_BUSKEEPER);

	/* Set up NPCSx as GPIO outputs, initially high */
	pin_mask = (cs_mask & 7) << 3;
	if (cs_mask & (1 << 3))
		pin_mask |= 1 << 20;

	portmux_select_gpio(PORTMUX_PORT_A, pin_mask,
			PORTMUX_DIR_OUTPUT | PORTMUX_INIT_HIGH);
}

void portmux_enable_spi1(unsigned long cs_mask, unsigned long drive_strength)
{
	/* MOSI and SCK */
	portmux_select_peripheral(PORTMUX_PORT_B, (1 << 1) | (1 << 5),
			PORTMUX_FUNC_B, 0);
	/* MISO may float */
	portmux_select_peripheral(PORTMUX_PORT_B, 1 << 0,
			PORTMUX_FUNC_B, PORTMUX_BUSKEEPER);

	/* Set up NPCSx as GPIO outputs, initially high */
	portmux_select_gpio(PORTMUX_PORT_B, (cs_mask & 7) << 2,
			PORTMUX_DIR_OUTPUT | PORTMUX_INIT_HIGH);
	portmux_select_gpio(PORTMUX_PORT_A, (cs_mask & 8) << (27 - 3),
			PORTMUX_DIR_OUTPUT | PORTMUX_INIT_HIGH);
}
#endif

#ifdef AT32AP700x_CHIP_HAS_LCDC
void portmux_enable_lcdc(int pin_config)
{
	unsigned long portc_mask = 0;
	unsigned long portd_mask = 0;
	unsigned long porte_mask = 0;

	switch (pin_config) {
	case 0:
		portc_mask = (1 << 19)	/* CC     */
			| (1 << 20)	/* HSYNC  */
			| (1 << 21)	/* PCLK   */
			| (1 << 22)	/* VSYNC  */
			| (1 << 23)	/* DVAL   */
			| (1 << 24)	/* MODE   */
			| (1 << 25)	/* PWR    */
			| (1 << 26)	/* DATA0  */
			| (1 << 27)	/* DATA1  */
			| (1 << 28)	/* DATA2  */
			| (1 << 29)	/* DATA3  */
			| (1 << 30)	/* DATA4  */
			| (1 << 31);	/* DATA5  */

		portd_mask = (1 << 0)	/* DATA6  */
			| (1 << 1)	/* DATA7  */
			| (1 << 2)	/* DATA8  */
			| (1 << 3)	/* DATA9  */
			| (1 << 4)	/* DATA10 */
			| (1 << 5)	/* DATA11 */
			| (1 << 6)	/* DATA12 */
			| (1 << 7)	/* DATA13 */
			| (1 << 8)	/* DATA14 */
			| (1 << 9)	/* DATA15 */
			| (1 << 10)	/* DATA16 */
			| (1 << 11)	/* DATA17 */
			| (1 << 12)	/* DATA18 */
			| (1 << 13)	/* DATA19 */
			| (1 << 14)	/* DATA20 */
			| (1 << 15)	/* DATA21 */
			| (1 << 16)	/* DATA22 */
			| (1 << 17);	/* DATA23 */
		break;

	case 1:
		portc_mask = (1 << 20)	/* HSYNC  */
			| (1 << 21)	/* PCLK   */
			| (1 << 22)	/* VSYNC  */
			| (1 << 25)	/* PWR    */
			| (1 << 31);	/* DATA5  */

		portd_mask = (1 << 0)	/* DATA6  */
			| (1 << 1)	/* DATA7  */
			| (1 << 7)	/* DATA13 */
			| (1 << 8)	/* DATA14 */
			| (1 << 9)	/* DATA15 */
			| (1 << 16)	/* DATA22 */
			| (1 << 17);	/* DATA23 */

		porte_mask = (1 << 0)	/* CC     */
			| (1 << 1)	/* DVAL   */
			| (1 << 2)	/* MODE   */
			| (1 << 3)	/* DATA0  */
			| (1 << 4)	/* DATA1  */
			| (1 << 5)	/* DATA2  */
			| (1 << 6)	/* DATA3  */
			| (1 << 7)	/* DATA4  */
			| (1 << 8)	/* DATA8  */
			| (1 << 9)	/* DATA9  */
			| (1 << 10)	/* DATA10 */
			| (1 << 11)	/* DATA11 */
			| (1 << 12)	/* DATA12 */
			| (1 << 13)	/* DATA16 */
			| (1 << 14)	/* DATA17 */
			| (1 << 15)	/* DATA18 */
			| (1 << 16)	/* DATA19 */
			| (1 << 17)	/* DATA20 */
			| (1 << 18);	/* DATA21 */
		break;
	}

	/* REVISIT: Some pins are probably pure outputs */
	portmux_select_peripheral(PORTMUX_PORT_C, portc_mask,
			PORTMUX_FUNC_A, PORTMUX_BUSKEEPER);
	portmux_select_peripheral(PORTMUX_PORT_D, portd_mask,
			PORTMUX_FUNC_A, PORTMUX_BUSKEEPER);
	portmux_select_peripheral(PORTMUX_PORT_E, porte_mask,
			PORTMUX_FUNC_B, PORTMUX_BUSKEEPER);
}
#endif
