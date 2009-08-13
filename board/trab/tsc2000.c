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

#include <common.h>
#include <s3c2400.h>
#include <div64.h>
#include "tsc2000.h"

#include "Pt1000_temp_data.h"

/* helper function */
#define abs(value) (((value) < 0) ? ((value)*-1) : (value))

/*
 * Maximal allowed deviation between two immediate meassurments of an analog
 * thermo channel. 1 DIGIT = 0.0276 °C. This is used to filter sporadic
 * "jumps" in measurment.
 */
#define MAX_DEVIATION	18	/* unit: DIGITs of adc; 18 DIGIT = 0.5 °C */

void tsc2000_spi_init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	S3C24X0_SPI * const spi = S3C24X0_GetBase_SPI();
	int i;

	/* Configure I/O ports. */
	gpio->PDCON = (gpio->PDCON & 0xF3FFFF) | 0x040000;
	gpio->PGCON = (gpio->PGCON & 0x0F3FFF) | 0x008000;
	gpio->PGCON = (gpio->PGCON & 0x0CFFFF) | 0x020000;
	gpio->PGCON = (gpio->PGCON & 0x03FFFF) | 0x080000;

	CLR_CS_TOUCH();

	spi->ch[0].SPPRE = 0x1F; /* Baud-rate ca. 514kHz */
	spi->ch[0].SPPIN = 0x01; /* SPI-MOSI holds Level after last bit */
	spi->ch[0].SPCON = 0x1A; /* Polling, Prescaler, Master, CPOL=0,
				    CPHA=1 */

	/* Dummy byte ensures clock to be low. */
	for (i = 0; i < 10; i++) {
		spi->ch[0].SPTDAT = 0xFF;
	}
	spi_wait_transmit_done();
}


void spi_wait_transmit_done(void)
{
	S3C24X0_SPI * const spi = S3C24X0_GetBase_SPI();

	while (!(spi->ch[0].SPSTA & 0x01)); /* wait until transfer is done */
}


void tsc2000_write(unsigned short reg, unsigned short data)
{
	S3C24X0_SPI * const spi = S3C24X0_GetBase_SPI();
	unsigned int command;

	SET_CS_TOUCH();
	command = reg;
	spi->ch[0].SPTDAT = (command & 0xFF00) >> 8;
	spi_wait_transmit_done();
	spi->ch[0].SPTDAT = (command & 0x00FF);
	spi_wait_transmit_done();
	spi->ch[0].SPTDAT = (data & 0xFF00) >> 8;
	spi_wait_transmit_done();
	spi->ch[0].SPTDAT = (data & 0x00FF);
	spi_wait_transmit_done();

	CLR_CS_TOUCH();
}


unsigned short tsc2000_read (unsigned short reg)
{
	unsigned short command, data;
	S3C24X0_SPI * const spi = S3C24X0_GetBase_SPI();

	SET_CS_TOUCH();
	command = 0x8000 | reg;

	spi->ch[0].SPTDAT = (command & 0xFF00) >> 8;
	spi_wait_transmit_done();
	spi->ch[0].SPTDAT = (command & 0x00FF);
	spi_wait_transmit_done();

	spi->ch[0].SPTDAT = 0xFF;
	spi_wait_transmit_done();
	data = spi->ch[0].SPRDAT;
	spi->ch[0].SPTDAT = 0xFF;
	spi_wait_transmit_done();

	CLR_CS_TOUCH();
	return (spi->ch[0].SPRDAT & 0x0FF) | (data << 8);
}


void tsc2000_set_mux (unsigned int channel)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	CLR_MUX1_ENABLE; CLR_MUX2_ENABLE;
	CLR_MUX3_ENABLE; CLR_MUX4_ENABLE;
	switch (channel) {
	case 0:
		CLR_MUX0; CLR_MUX1;
		SET_MUX1_ENABLE;
		break;
	case 1:
		SET_MUX0; CLR_MUX1;
		SET_MUX1_ENABLE;
		break;
	case 2:
		CLR_MUX0; SET_MUX1;
		SET_MUX1_ENABLE;
		break;
	case 3:
		SET_MUX0; SET_MUX1;
		SET_MUX1_ENABLE;
		break;
	case 4:
		CLR_MUX0; CLR_MUX1;
		SET_MUX2_ENABLE;
		break;
	case 5:
		SET_MUX0; CLR_MUX1;
		SET_MUX2_ENABLE;
		break;
	case 6:
		CLR_MUX0; SET_MUX1;
		SET_MUX2_ENABLE;
		break;
	case 7:
		SET_MUX0; SET_MUX1;
		SET_MUX2_ENABLE;
		break;
	case 8:
		CLR_MUX0; CLR_MUX1;
		SET_MUX3_ENABLE;
		break;
	case 9:
		SET_MUX0; CLR_MUX1;
		SET_MUX3_ENABLE;
		break;
	case 10:
		CLR_MUX0; SET_MUX1;
		SET_MUX3_ENABLE;
		break;
	case 11:
		SET_MUX0; SET_MUX1;
		SET_MUX3_ENABLE;
		break;
	case 12:
		CLR_MUX0; CLR_MUX1;
		SET_MUX4_ENABLE;
		break;
	case 13:
		SET_MUX0; CLR_MUX1;
		SET_MUX4_ENABLE;
		break;
	case 14:
		CLR_MUX0; SET_MUX1;
		SET_MUX4_ENABLE;
		break;
	case 15:
		SET_MUX0; SET_MUX1;
		SET_MUX4_ENABLE;
		break;
	default:
		CLR_MUX0; CLR_MUX1;
	}
}


void tsc2000_set_range (unsigned int range)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	switch (range) {
	case 1:
		CLR_SEL_TEMP_V_0; SET_SEL_TEMP_V_1;
		CLR_SEL_TEMP_V_2; CLR_SEL_TEMP_V_3;
		break;
	case 2:
		CLR_SEL_TEMP_V_0; CLR_SEL_TEMP_V_1;
		CLR_SEL_TEMP_V_2; SET_SEL_TEMP_V_3;
		break;
	case 3:
		SET_SEL_TEMP_V_0; CLR_SEL_TEMP_V_1;
		SET_SEL_TEMP_V_2; CLR_SEL_TEMP_V_3;
		break;
	}
}


u16 tsc2000_read_channel (unsigned int channel)
{
	u16 res;

	tsc2000_set_mux(channel);
	udelay(20 * TSC2000_DELAY_BASE);

	tsc2000_write(TSC2000_REG_ADC, 0x2036);
	adc_wait_conversion_done ();
	res = tsc2000_read(TSC2000_REG_AUX1);
	return res;
}


s32 tsc2000_contact_temp (void)
{
	long adc_pt1000, offset;
	long u_pt1000;
	long contact_temp;
	long temp1, temp2;

	tsc2000_reg_init ();
	tsc2000_set_range (3);

	/*
	 * Because of sporadic "jumps" in the measured adc values every
	 * channel is read two times. If there is a significant difference
	 * between the two measurements, then print an error and do a third
	 * measurement, because it is very unlikely that a successive third
	 * measurement goes also wrong.
	 */
	temp1 = tsc2000_read_channel (14);
	temp2 = tsc2000_read_channel (14);
	if (abs(temp2 - temp1) < MAX_DEVIATION)
		adc_pt1000 = temp2;
	else {
		printf ("%s: read adc value (channel 14) exceeded max allowed "
			"deviation: %d * 0.0276 °C\n",
			__FUNCTION__, MAX_DEVIATION);
		printf ("adc value 1: %ld DIGITs\nadc value 2: %ld DIGITs\n",
			temp1, temp2);
		adc_pt1000 = tsc2000_read_channel (14);
		printf ("use (third read) adc value: adc_pt1000 = "
			"%ld DIGITs\n",	adc_pt1000);
	}
	debug ("read channel 14 (pt1000 adc value): %ld\n", adc_pt1000);

	temp1 = tsc2000_read_channel (15);
	temp2 = tsc2000_read_channel (15);
	if (abs(temp2 - temp1) < MAX_DEVIATION)
		offset = temp2;
	else {
		printf ("%s: read adc value (channel 15) exceeded max allowed "
			"deviation: %d * 0.0276 °C\n",
			__FUNCTION__, MAX_DEVIATION);
		printf ("adc value 1: %ld DIGITs\nadc value 2: %ld DIGITs\n",
			temp1, temp2);
		offset = tsc2000_read_channel (15);
		printf ("use (third read) adc value: offset = %ld DIGITs\n",
			offset);
	}
	debug ("read channel 15 (offset): %ld\n", offset);

	/*
	 * Formula for calculating voltage drop on PT1000 resistor: u_pt1000 =
	 * x_range3 * (adc_raw - offset) / 10. Formula to calculate x_range3:
	 * x_range3 = (2500 * (1000000 + err_vref + err_amp3)) / (4095*6). The
	 * error correction Values err_vref and err_amp3 are assumed as 0 in
	 * u-boot, because this could cause only a very small error (< 1%).
	 */
	u_pt1000 = (101750 * (adc_pt1000 - offset)) / 10;
	debug ("u_pt1000: %ld\n", u_pt1000);

	if (tsc2000_interpolate(u_pt1000, Pt1000_temp_table,
				&contact_temp) == -1) {
		printf ("%s: error interpolating PT1000 vlaue\n",
			 __FUNCTION__);
		return (-1000);
	}
	debug ("contact_temp: %ld\n", contact_temp);

	return contact_temp;
}


void tsc2000_reg_init (void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

	tsc2000_write(TSC2000_REG_ADC, 0x2036);
	tsc2000_write(TSC2000_REG_REF, 0x0011);
	tsc2000_write(TSC2000_REG_DACCTL, 0x0000);

	CON_MUX0;
	CON_MUX1;

	CON_MUX1_ENABLE;
	CON_MUX2_ENABLE;
	CON_MUX3_ENABLE;
	CON_MUX4_ENABLE;

	CON_SEL_TEMP_V_0;
	CON_SEL_TEMP_V_1;
	CON_SEL_TEMP_V_2;
	CON_SEL_TEMP_V_3;

	tsc2000_set_mux(0);
	tsc2000_set_range(0);
}


int tsc2000_interpolate(long value, long data[][2], long *result)
{
	int i;
	unsigned long long val;

	/* the data is sorted and the first element is upper
	 * limit so we can easily check for out-of-band values
	 */
	if (data[0][0] < value || data[1][0] > value)
		return -1;

	i = 1;
	while (data[i][0] < value)
		i++;

	/* To prevent overflow we have to store the intermediate
	   result in 'long long'.
	*/

	val = ((unsigned long long)(data[i][1] - data[i-1][1])
		   * (unsigned long long)(value - data[i-1][0]));
	do_div(val, (data[i][0] - data[i-1][0]));
	*result = data[i-1][1] + val;

	return 0;
}


void adc_wait_conversion_done(void)
{
	while (!(tsc2000_read(TSC2000_REG_ADC) & (1 << 14)));
}
