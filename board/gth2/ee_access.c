/* Module for handling DALLAS DS2438, smart battery monitor
   Chip can store up to 40 bytes of user data in EEPROM,
   perform temp, voltage and current measurements.
   Chip also contains a unique serial number.

   Always read/write LSb first

   For documentaion, see data sheet for DS2438, 2438.pdf

   By Thomas.Lange@corelatus.com 001025

   Copyright (C) 2000-2005 Corelatus AB */

/* This program is free software; you can redistribute it and/or
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
#include <command.h>
#include <asm/au1x00.h>
#include <asm/io.h>
#include "ee_dev.h"
#include "ee_access.h"

/* static int Debug = 1; */
#undef E_DEBUG
#define E_DEBUG(fmt,args...) /* */
/* #define E_DEBUG(fmt,args...) printk("EEA:"fmt,##args); */

/* We dont have kernel functions */
#define printk printf
#define KERN_DEBUG
#define KERN_ERR
#define EIO 1

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/* lookup table ripped from DS app note 17, understanding and using cyclic redundancy checks... */

static u8 crc_lookup[256] = {
	0,	94,	188,	226,	97,	63,	221,	131,
	194,	156,	126,	32,	163,	253,	31,	65,
	157,	195,	33,	127,	252,	162,	64,	30,
	95,	1,	227,	189,	62,	96,	130,	220,
	35,	125,	159,	193,	66,	28,	254,	160,
	225,	191,	93,	3,	128,	222,	60,	98,
	190,	224,	2,	92,	223,	129,	99,	61,
	124,	34,	192,	158,	29,	67,	161,	255,
	70,	24,	250,	164,	39,	121,	155,	197,
	132,	218,	56,	102,	229,	187,	89,	7,
	219,	133,	103,	57,	186,	228,	6,	88,
	25,	71,	165,	251,	120,	38,	196,	154,
	101,	59,	217,	135,	4,	90,	184,	230,
	167,	249,	27,	69,	198,	152,	122,	36,
	248,	166,	68,	26,	153,	199,	37,	123,
	58,	100,	134,	216,	91,	5,	231,	185,
	140,	210,	48,	110,	237,	179,	81,	15,
	78,	16,	242,	172,	47,	113,	147,	205,
	17,	79,	173,	243,	112,	46,	204,	146,
	211,	141,	111,	49,	178,	236,	14,	80,
	175,	241,	19,	77,	206,	144,	114,	44,
	109,	51,	209,	143,	12,	82,	176,	238,
	50,	108,	142,	208,	83,	13,	239,	177,
	240,	174,	76,	18,	145,	207,	45,	115,
	202,	148,	118,	40,	171,	245,	23,	73,
	8,	86,	180,	234,	105,	55,	213,	139,
	87,	9,	235,	181,	54,	104,	138,	212,
	149,	203,	41,	119,	244,	170,	72,	22,
	233,	183,	85,	11,	136,	214,	52,	106,
	43,	117,	151,	201,	74,	20,	246,	168,
	116,	42,	200,	150,	21,	75,	169,	247,
	182,	232,	10,	84,	215,	137,	107,	53
};

static void
write_gpio_data(int value ){
	if(value){
		/* Tristate */
		gpio_tristate(GPIO_EEDQ);
	}
	else{
		/* Drive 0 */
		gpio_clear(GPIO_EEDQ);
	}
}

static u8 make_new_crc( u8 Old_crc, u8 New_value ){
	/* Compute a new checksum with new byte, using previous checksum as input
	   See DS app note 17, understanding and using cyclic redundancy checks...
	   Also see DS2438, page 11 */
	return( crc_lookup[Old_crc ^ New_value ]);
}

int ee_crc_ok( u8 *Buffer, int Len, u8 Crc ){
	/* Check if the checksum for this buffer is correct */
	u8 Curr_crc=0;
	int i;
	u8 *Curr_byte = Buffer;

	for(i=0;i<Len;i++){
		Curr_crc = make_new_crc( Curr_crc, *Curr_byte);
		Curr_byte++;
	}
	E_DEBUG("Calculated CRC = 0x%x, read = 0x%x\n", Curr_crc, Crc);

	if(Curr_crc == Crc){
		/* Good */
		return(TRUE);
	}
	printk(KERN_ERR"EE checksum error, Calculated CRC = 0x%x, read = 0x%x\n", Curr_crc, Crc);
	return(FALSE);
}

static void
set_idle(void){
	/* Send idle and keep start time
	   Continous 1 is idle */
	WRITE_PORT(1);
}


static int
do_cpu_reset(void){
	/* Release reset and verify that chip responds with presence pulse */
	int Retries=0;
	while(Retries<15){
		udelay(RESET_LOW_TIME);

		/* Send reset */
		WRITE_PORT(0);
		udelay(RESET_LOW_TIME);

		/* Release reset */
		WRITE_PORT(1);

		/* Wait for EEPROM to drive output */
		udelay(PRESENCE_TIMEOUT);
		if(!READ_PORT){
			/* Ok, EEPROM is driving a 0 */
			E_DEBUG("Presence detected\n");
			if(Retries){
				E_DEBUG("Retries %d\n",Retries);
			}
			/* Make sure chip releases pin */
			udelay(PRESENCE_LOW_TIME);
			return 0;
		}
		Retries++;
	}

	printk(KERN_ERR"eeprom did not respond when releasing reset\n");

	/* Make sure chip releases pin */
	udelay(PRESENCE_LOW_TIME);

	/* Set to idle again */
	set_idle();

	return(-EIO);
}

static u8
read_cpu_byte(void){
	/* Read a single byte from EEPROM
	   Read LSb first */
	int i;
	int Value;
	u8 Result=0;
	u32 Flags;

	E_DEBUG("Reading byte\n");

	for(i=0;i<8;i++){
		/* Small delay between pulses */
		udelay(1);

#ifdef __KERNEL__
		/* Disable irq */
		save_flags(Flags);
		cli();
#endif

		/* Pull down pin short time to start read
		   See page 26 in data sheet */

		WRITE_PORT(0);
		udelay(READ_LOW);
		WRITE_PORT(1);

		/* Wait for chip to drive pin */
		udelay(READ_TIMEOUT);

		Value = READ_PORT;
		if(Value)
			Value=1;

#ifdef __KERNEL__
		/* Enable irq */
		restore_flags(Flags);
#endif

		/* Wait for chip to release pin */
		udelay(TOTAL_READ_LOW-READ_TIMEOUT);

		/* LSb first */
		Result|=Value<<i;
		/* E_DEBUG("Read %d\n",Value); */

	}

	E_DEBUG("Read byte 0x%x\n",Result);

	return(Result);
}

static void
write_cpu_byte(u8 Byte){
	/* Write a single byte to EEPROM
	   Write LSb first */
	int i;
	int Value;
	u32 Flags;

	E_DEBUG("Writing byte 0x%x\n",Byte);

	for(i=0;i<8;i++){
		/* Small delay between pulses */
		udelay(1);
		Value = Byte&1;

#ifdef __KERNEL__
		/* Disable irq */
		save_flags(Flags);
		cli();
#endif

		/* Pull down pin short time for a 1, long time for a 0
		   See page 26 in data sheet */

		WRITE_PORT(0);
		if(Value){
			/* Write a 1 */
			udelay(WRITE_1_LOW);
		}
		else{
			/* Write a 0 */
			udelay(WRITE_0_LOW);
		}

		WRITE_PORT(1);

#ifdef __KERNEL__
		/* Enable irq */
		restore_flags(Flags);
#endif

		if(Value)
			/* Wait for chip to read the 1 */
			udelay(TOTAL_WRITE_LOW-WRITE_1_LOW);

		/* E_DEBUG("Wrote %d\n",Value); */
		Byte>>=1;
	}
}

int ee_do_cpu_command( u8 *Tx, int Tx_len, u8 *Rx, int Rx_len, int Send_skip ){
	/* Execute this command string, including
	   giving reset and setting to idle after command
	   if Rx_len is set, we read out data from EEPROM */
	int i;

	E_DEBUG("Command, Tx_len %d, Rx_len %d\n", Tx_len, Rx_len );

	if(do_cpu_reset()){
		/* Failed! */
		return(-EIO);
	}

	if(Send_skip)
		/* Always send SKIP_ROM first to tell chip we are sending a command,
		   except when we read out rom data for chip */
		write_cpu_byte(SKIP_ROM);

	/* Always have Tx data */
	for(i=0;i<Tx_len;i++){
		write_cpu_byte(Tx[i]);
	}

	if(Rx_len){
		for(i=0;i<Rx_len;i++){
			Rx[i]=read_cpu_byte();
		}
	}

	set_idle();

	E_DEBUG("Command done\n");

	return(0);
}

int ee_init_cpu_data(void){
	int i;
	u8 Tx[10];

	/* Leave it floting since altera is driving the same pin */
	set_idle();

	/* Copy all User EEPROM data to scratchpad */
	for(i=0;i<USER_PAGES;i++){
		Tx[0]=RECALL_MEMORY;
		Tx[1]=EE_USER_PAGE_0+i;
		if(ee_do_cpu_command(Tx,2,NULL,0,TRUE)) return(-EIO);
	}

	/* Make sure chip doesnt store measurements in NVRAM */
	Tx[0]=WRITE_SCRATCHPAD;
	Tx[1]=0; /* Page */
	Tx[2]=9;
	if(ee_do_cpu_command(Tx,3,NULL,0,TRUE)) return(-EIO);

	Tx[0]=COPY_SCRATCHPAD;
	if(ee_do_cpu_command(Tx,2,NULL,0,TRUE)) return(-EIO);

	for(i=0;i<10;i++){
		udelay(1000);
	}

	return(0);
}
