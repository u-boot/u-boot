/*
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

/*
 * Logic lh7a400-10 Card Engine CPLD interface
 */

#ifndef __LPD7A400_CPLD_H_
#define __LPD7A400_CPLD_H_


/*
 * IO Controller Address and Register Definitions
 *   - using LH7A400-10 Card Engine IO Controller Specification
 *     (logic PN: 70000079)
 */

/*------------------------------------------------------------------
 * Slow Peripherals (nCS6)
 */
#define LPD7A400_CPLD_CF		(0x60200000)
#define LPD7A400_CPLD_ISA		(0x60400000)

/*------------------------------------------------------------------
 * Fast Peripherals (nCS7)
 *
 * The CPLD directs access to 0x70000000-0x701fffff to the onboard
 * ethernet controller
 */
#define LPD7A400_CPLD_WLAN_BASE		(0x70000000)

/* All registers are 8 bit */
#define LPD7A400_CPLD_CECTL_REG		(0x70200000)
#define LPD7A400_CPLD_SPIDATA_REG	(0x70600000)
#define LPD7A400_CPLD_SPICTL_REG	(0x70800000)
#define LPD7A400_CPLD_EEPSPI_REG	(0x70a00000)
#define LPD7A400_CPLD_INTMASK_REG	(0x70c00000)
#define LPD7A400_CPLD_MODE_REG		(0x70e00000)
#define LPD7A400_CPLD_FLASH_REG		(0x71000000)
#define LPD7A400_CPLD_PWRMG_REG		(0x71200000)
#define LPD7A400_CPLD_REV_REG		(0x71400000)
#define LPD7A400_CPLD_EXTGPIO_REG	(0x71600000)
#define LPD7A400_CPLD_GPIODATA_REG	(0x71800000)
#define LPD7A400_CPLD_GPIODIR_REG	(0x71a00000)

#define LPD7A400_CPLD_REGPTR		(volatile u8*)

/* Card Engine Control Register (section 3.1.2) */
#define CECTL_SWINT	(0x80)	/* Software settable interrupt source
				   (routed to uP PF3)
				   0 = generate interrupt, 1 = do not */
#define CECTL_OCMSK	(0x40)	/* USB1 connection interrupt mask
				   0 = not masked, 1 = masked */
#define CECTL_PDRV	(0x20)	/* PCC_nDRV output
				   0 = active, 1 = inactive */
#define CECTL_USB1C	(0x10)	/* USB1 connection interrupt
				   0 = active, 1 = inactive */
#define CECTL_USB1P	(0x08)  /* USB1 Power enable
				   0 = enabled, 1 = disabled */
#define CECTL_AWKP	(0x04)  /* Auto-Wakeup enable
				   0 = enabled, 1 = disabled */
#define CECTL_LCDV	(0x02)  /* LCD VEE enable
				   0 = disabled, 1 = enabled */
#define CECTL_WLPE	(0x01)  /* Wired LAN power enable
				   0 = enabled, 1 = disabled */

/* SPI Control Register (section 3.1.5) */
#define SPICTL_SPLD	(0x20)	/* SPI load (R)
				   0 = data reg. has not been loaded, shift
				       count has not been reset
				   1 = data reg. loaded, shift count reset */
#define SPICTL_SPST	(0x10)  /* SPI start (RW)
				   0 = don't load data reg. and reset shift count
				   1 = ready to load data reg and reset shift count */
#define SPICTL_SPDN	(0x08)  /* SPI done (R)
				   0 = not done
				   1 = access done */
#define SPICTL_SPRW	(0x04)  /* SPI read/write (RW)
				   0 = SPI write access
				   1 = SPI read access */
#define SPICTL_STCS	(0x02)  /* SPI touch chip select (RW)
				   0 = not selected
				   1 = selected */
#define SPICTL_SCCS	(0x01)  /* SPI CODEC chip select (RW) {not used}
				   0 = not selected
				   1 = selected */

/* EEPROM SPI Interface Register (section 3.1.6) */
#define EEPSPI_EECS	(0x08)	/* EEPROM chip select (RW)
				   0 = not selected
				   1 = selected */
#define EEPSPI_EECK	(0x04)	/* EEPROM SPI clock (RW) */
#define EEPSPI_EETX	(0x02)  /* EEPROM SPI tx data (RW) */
#define EEPSPI_EERX	(0x01)  /* EEPROM SPI rx data (R) */

/* Interrupt/Mask Register (section 3.1.7) */
#define INTMASK_CMSK	(0x80)  /* CPLD_nIRQD interrupt mask (RW)
				   0 = not masked
				   1 = masked */
#define INTMASK_CIRQ	(0x40)	/* interrupt signal to CPLD (R)
				   0 = interrupt active
				   1 = no interrupt */
#define INTMASK_PIRQ	(0x10)	/* legacy, no effect */
#define INTMASK_TMSK	(0x08)	/* Touch chip interrupt mask (RW)
				   0 = not masked
				   1 = masked */
#define INTMASK_WMSK	(0x04)  /* Wired LAN interrupt mask (RW)
				   0 = not masked
				   1 = masked */
#define INTMASK_TIRQ	(0x02)  /* Touch chip interrupt request (R)
				   0 = interrupt active
				   1 = no interrupt */
#define INTMASK_WIRQ	(0x01)  /* Wired LAN interrupt request (R)
				   0 = interrupt active
				   1 = no interrupt */

/* Mode Register (section 3.1.8) */
#define MODE_VS1	(0x80)  /* PCMCIA Voltage Sense 1 input (PCC_VS1) (R)
				   0 = active slot VS1 pin is low
				   1 = active slot VS1 pin is high */
#define MODE_CD2	(0x40)  /* PCMCIA Card Detect 2 input (PCC_nCD2) (R)
				   0 = active slot CD2 is low
				   1 = active slot CD2 is high */
#define MODE_IOIS16	(0x20)  /* PCMCIA IOIS16 input (PCC_nIOIS16) (R)
				   0 = 16 bit access area
				   1 = 8 bit access area */
#define MODE_CD1	(0x10)  /* PCMCIA Card Detect 1 input (PCC_nCD1) (R)
				   0 = active slot CD1 is low
				   1 = active slot CD1 is high */
#define MODE_upMODE3	(0x08)  /* Mode Pin 3 (R)
				   0 = off-board boot device
				   1 = on-board boot device (flash) */
#define MODE_upMODE2	(0x04)  /* Mode Pin 2 (R) (LH7A400 Little Endian only)
				   0 = big endian
				   1 = little endian */
#define MODE_upMODE1	(0x02)  /* Mode Pin 1 and Mode Pin 2 (R) */
#define MODE_upMODE0	(0x01)  /*   - bus width at boot */


/* Flash Register (section 3.1.9) */
#define FLASH_FPOP	(0x08)	/* Flash populated (RW)
				   0 = populated, 1 = not */
#define FLASH_FST2	(0x04)  /* Flash status (R) (RY/BY# pin for upper 16 bit chip
				   0 = busy, 1 = ready */
#define FLASH_FST1	(0x02)  /* Flash status (R) (RY/BY# pin for lower 16 bit chip
				   0 = busy, 1 = ready */
#define FLASH_FPEN	(0x01)  /* Flash program enable (RW)
				   0 = flash write protected
				   1 = programming enabled */

/* Power Management Register (section 3.1.10)
 *    - when either of these is low an unmaskable interrupt to cpu
 *      is generated
 */
#define PWRMG_STBY	(0x10)  /* state of nSTANDBY signal to CPLD (R)
				   0 = low, 1 = high */
#define PWRMG_SPND	(0x04)  /* state of nSUSPEND signal to CPLD (R)
				   0 = low, 1 = high */


/* Extended GPIO Register (section 3.1.12) */
#define EXTGPIO_STATUS1	(0x04)  /* Status 1 output (RW) (uP_STATUS_1)
				   0 = set pin low, 1 = set pin high */
#define EXTGPIO_STATUS2 (0x02)  /* Status 2 output (RW) (uP_STATUS_2)
				   0 = set pin low, 1 = set pin high */
#define EXTGPIO_GPIO1	(0x01)  /* General purpose output (RW) (CPLD_GPIO_1)
				   0 = set pin low, 1 = set pin high */

/* GPIO Data Register (section 3.1.13) */
#define GPIODATA_GPIO2	(0x01)  /* General purpose input/output (RW) (CPLD_GPIO_2)
				   0 = set low (output) / read low (input)
				   1 = set high (output) / read high (input) */

/* GPIO Direction Register (section 3.1.14) */
#define GPIODIR_GPDR0	(0x01)  /* GPIO2 direction (RW)
				   0 = output, 1 = input */

#endif  /* __LH7A400_H__ */
