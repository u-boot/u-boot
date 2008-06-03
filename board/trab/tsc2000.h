/*
 * Functions to access the TSC2000 controller on TRAB board (used for scanning
 * thermo sensors)
 *
 * Copyright (C) 2003 Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * Copyright (C) 2002 DENX Software Engineering, Wolfgang Denk, wd@denx.de
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

#ifndef _TSC2000_H_
#define _TSC2000_H_

/* temperature channel multiplexer definitions */
#define CON_MUX0		(gpio->PCCON = (gpio->PCCON & 0x0FFFFFCFF) | 0x00000100)
#define CLR_MUX0		(gpio->PCDAT &= 0x0FFEF)
#define SET_MUX0		(gpio->PCDAT |= 0x00010)

#define CON_MUX1		(gpio->PCCON = (gpio->PCCON & 0x0FFFFF3FF) | 0x00000400)
#define CLR_MUX1		(gpio->PCDAT &= 0x0FFDF)
#define SET_MUX1		(gpio->PCDAT |= 0x00020)

#define CON_MUX1_ENABLE		(gpio->PCCON = (gpio->PCCON & 0x0FFFFCFFF) | 0x00001000)
#define CLR_MUX1_ENABLE		(gpio->PCDAT |= 0x00040)
#define SET_MUX1_ENABLE		(gpio->PCDAT &= 0x0FFBF)

#define CON_MUX2_ENABLE		(gpio->PCCON = (gpio->PCCON & 0x0FFFF3FFF) | 0x00004000)
#define CLR_MUX2_ENABLE		(gpio->PCDAT |= 0x00080)
#define SET_MUX2_ENABLE		(gpio->PCDAT &= 0x0FF7F)

#define CON_MUX3_ENABLE		(gpio->PCCON = (gpio->PCCON & 0x0FFFCFFFF) | 0x00010000)
#define CLR_MUX3_ENABLE		(gpio->PCDAT |= 0x00100)
#define SET_MUX3_ENABLE		(gpio->PCDAT &= 0x0FEFF)

#define CON_MUX4_ENABLE		(gpio->PCCON = (gpio->PCCON & 0x0FFF3FFFF) | 0x00040000)
#define CLR_MUX4_ENABLE		(gpio->PCDAT |= 0x00200)
#define SET_MUX4_ENABLE		(gpio->PCDAT &= 0x0FDFF)

#define CON_SEL_TEMP_V_0	(gpio->PCCON = (gpio->PCCON & 0x0FFCFFFFF) | 0x00100000)
#define CLR_SEL_TEMP_V_0	(gpio->PCDAT &= 0x0FBFF)
#define SET_SEL_TEMP_V_0	(gpio->PCDAT |= 0x00400)

#define CON_SEL_TEMP_V_1	(gpio->PCCON = (gpio->PCCON & 0x0FF3FFFFF) | 0x00400000)
#define CLR_SEL_TEMP_V_1	(gpio->PCDAT &= 0x0F7FF)
#define SET_SEL_TEMP_V_1	(gpio->PCDAT |= 0x00800)

#define CON_SEL_TEMP_V_2	(gpio->PCCON = (gpio->PCCON & 0x0FCFFFFFF) | 0x01000000)
#define CLR_SEL_TEMP_V_2	(gpio->PCDAT &= 0x0EFFF)
#define SET_SEL_TEMP_V_2	(gpio->PCDAT |= 0x01000)

#define CON_SEL_TEMP_V_3	(gpio->PCCON = (gpio->PCCON & 0x0F3FFFFFF) | 0x04000000)
#define CLR_SEL_TEMP_V_3	(gpio->PCDAT &= 0x0DFFF)
#define SET_SEL_TEMP_V_3	(gpio->PCDAT |= 0x02000)

/* TSC2000 register definition */
#define TSC2000_REG_X		((0 << 11) | (0 << 5))
#define TSC2000_REG_Y		((0 << 11) | (1 << 5))
#define TSC2000_REG_Z1		((0 << 11) | (2 << 5))
#define TSC2000_REG_Z2		((0 << 11) | (3 << 5))
#define TSC2000_REG_BAT1	((0 << 11) | (5 << 5))
#define TSC2000_REG_BAT2	((0 << 11) | (6 << 5))
#define TSC2000_REG_AUX1	((0 << 11) | (7 << 5))
#define TSC2000_REG_AUX2	((0 << 11) | (8 << 5))
#define TSC2000_REG_TEMP1	((0 << 11) | (9 << 5))
#define TSC2000_REG_TEMP2	((0 << 11) | (0xA << 5))
#define TSC2000_REG_DAC		((0 << 11) | (0xB << 5))
#define TSC2000_REG_ZERO	((0 << 11) | (0x10 << 5))
#define TSC2000_REG_ADC		((1 << 11) | (0 << 5))
#define TSC2000_REG_DACCTL	((1 << 11) | (2 << 5))
#define TSC2000_REG_REF		((1 << 11) | (3 << 5))
#define TSC2000_REG_RESET	((1 << 11) | (4 << 5))
#define TSC2000_REG_CONFIG	((1 << 11) | (5 << 5))

/* bit definition of TSC2000 ADC register */
#define TC_PSM			(1 << 15)
#define TC_STS			(1 << 14)
#define TC_AD3			(1 << 13)
#define TC_AD2			(1 << 12)
#define TC_AD1			(1 << 11)
#define TC_AD0			(1 << 10)
#define TC_RS1			(1 << 9)
#define TC_RS0			(1 << 8)
#define TC_AV1			(1 << 7)
#define TC_AV0			(1 << 6)
#define TC_CL1			(1 << 5)
#define TC_CL0			(1 << 4)
#define TC_PV2			(1 << 3)
#define TC_PV1			(1 << 2)
#define TC_PV0			(1 << 1)

/* default value for TSC2000 ADC register for use with touch functions */
#define DEFAULT_ADC		(TC_PV1 | TC_AV0 | TC_AV1 | TC_RS0)

#define TSC2000_DELAY_BASE	500
#define TSC2000_NO_SENSOR	-0x10000

#define ERROR_BATTERY		220	/* must be adjusted, if R68 is changed on TRAB */

void tsc2000_write(unsigned short, unsigned short);
unsigned short tsc2000_read (unsigned short);
u16 tsc2000_read_channel (unsigned int);
void tsc2000_set_mux (unsigned int);
void tsc2000_set_range (unsigned int);
void tsc2000_reg_init (void);
s32 tsc2000_contact_temp (void);
void spi_wait_transmit_done (void);
void spi_init(void);
int tsc2000_interpolate(long value, long data[][2], long *result);
void adc_wait_conversion_done(void);


static inline void SET_CS_TOUCH(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	gpio->PDDAT &= 0x5FF;
}


static inline void CLR_CS_TOUCH(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	gpio->PDDAT |= 0x200;
}

#endif	/* _TSC2000_H_ */
