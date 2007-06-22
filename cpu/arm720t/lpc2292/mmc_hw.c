/*
    This code was original written by Ulrich Radig and modified by
    Embedded Artists AB (www.embeddedartists.com).

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <config.h>
#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spi.h>

#define MMC_Enable() PUT32(IO1CLR, 1l << 22)
#define MMC_Disable() PUT32(IO1SET, 1l << 22)
#define mmc_spi_cfg() spi_set_clock(8); spi_set_cfg(0, 1, 0);

static unsigned char Write_Command_MMC (unsigned char *CMD);
static void MMC_Read_Block(unsigned char *CMD, unsigned char *Buffer,
		    unsigned short int Bytes);

/* initialize the hardware */
int mmc_hw_init(void)
{
	unsigned long a;
	unsigned short int Timeout = 0;
	unsigned char b;
	unsigned char CMD[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};

	/* set-up GPIO and SPI */
	(*((volatile unsigned long *)PINSEL2)) &= ~(1l << 3); /* clear bit 3 */
	(*((volatile unsigned long *)IO1DIR)) |= (1l << 22); /* set bit 22 (output) */

	MMC_Disable();

	spi_lock();
	spi_set_clock(248);
	spi_set_cfg(0, 1, 0);
	MMC_Enable();

	/* waste some time */
	for(a=0; a < 20000; a++)
		asm("nop");

	/* Put the MMC/SD-card into SPI-mode */
	for (b = 0; b < 10; b++) /* Sends min 74+ clocks to the MMC/SD-card */
		spi_write(0xff);

	/* Sends command CMD0 to MMC/SD-card */
	while (Write_Command_MMC(CMD) != 1) {
		if (Timeout++ > 200) {
			MMC_Disable();
			spi_unlock();
			return(1); /* Abort with command 1 (return 1) */
		}
	}
	/* Sends Command CMD1 an MMC/SD-card */
	Timeout = 0;
	CMD[0] = 0x41;/* Command 1 */
	CMD[5] = 0xFF;

	while (Write_Command_MMC(CMD) != 0) {
		if (Timeout++ > 200) {
			MMC_Disable();
			spi_unlock();
			return (2); /* Abort with command 2 (return 2) */
		}
	}

	MMC_Disable();
	spi_unlock();

	return 0;
}

/* ############################################################################
   Sends a command to the MMC/SD-card
   ######################################################################### */
static unsigned char Write_Command_MMC (unsigned char *CMD)
{
	unsigned char a, tmp = 0xff;
	unsigned short int Timeout = 0;

	MMC_Disable();
	spi_write(0xFF);
	MMC_Enable();

	for (a = 0; a < 0x06; a++)
		spi_write(*CMD++);

	while (tmp == 0xff) {
		tmp = spi_read();
		if (Timeout++ > 5000)
		  break;
	}

	return (tmp);
}

/* ############################################################################
   Routine to read the CID register from the MMC/SD-card (16 bytes)
   ######################################################################### */
void MMC_Read_Block(unsigned char *CMD, unsigned char *Buffer, unsigned short
	int Bytes)
{
	unsigned short int a;

	spi_lock();
	mmc_spi_cfg();
	MMC_Enable();

	if (Write_Command_MMC(CMD) != 0) {
		MMC_Disable();
		spi_unlock();
		return;
	}

	while (spi_read() != 0xfe) {};
	for (a = 0; a < Bytes; a++)
		*Buffer++ = spi_read();

	/* Read the CRC-byte */
	spi_read(); /* CRC - byte is discarded */
	spi_read(); /* CRC - byte is discarded */
	/* set MMC_Chip_Select to high (MMC/SD-card Inaktiv) */
	MMC_Disable();
	spi_unlock();

	return;
}

/* ############################################################################
   Routine to read a block (512 bytes) from the MMC/SD-card
   ######################################################################### */
unsigned char mmc_read_sector (unsigned long addr,unsigned char *Buffer)
{
	/* Command 16 to read aBlocks from the MMC/SD - caed */
	unsigned char CMD[] = {0x51,0x00,0x00,0x00,0x00,0xFF};

	/* The addres on the MMC/SD-card is in bytes,
	addr is transformed from blocks to bytes and the result is
	placed into the command */

	addr = addr << 9; /* addr = addr * 512 */

	CMD[1] = ((addr & 0xFF000000) >> 24);
	CMD[2] = ((addr & 0x00FF0000) >> 16);
	CMD[3] = ((addr & 0x0000FF00) >> 8 );

	MMC_Read_Block(CMD, Buffer, 512);

	return (0);
}

/* ############################################################################
   Routine to write a block (512 byte) to the MMC/SD-card
   ######################################################################### */
unsigned char mmc_write_sector (unsigned long addr,unsigned char *Buffer)
{
	unsigned char tmp, a;
	unsigned short int b;
	/* Command 24 to write a block to the MMC/SD - card */
	unsigned char CMD[] = {0x58, 0x00, 0x00, 0x00, 0x00, 0xFF};

	/* The addres on the MMC/SD-card is in bytes,
	addr is transformed from blocks to bytes and the result is
	placed into the command */

	addr = addr << 9; /* addr = addr * 512 */

	CMD[1] = ((addr & 0xFF000000) >> 24);
	CMD[2] = ((addr & 0x00FF0000) >> 16);
	CMD[3] = ((addr & 0x0000FF00) >> 8 );

	spi_lock();
	mmc_spi_cfg();
	MMC_Enable();

	/* Send command CMD24 to the MMC/SD-card (Write 1 Block/512 Bytes) */
	tmp = Write_Command_MMC(CMD);
	if (tmp != 0) {
		MMC_Disable();
		spi_unlock();
		return(tmp);
	}

	/* Do a short delay and send a clock-pulse to the MMC/SD-card */
	for (a = 0; a < 100; a++)
		spi_read();

	/* Send a start byte to the MMC/SD-card */
	spi_write(0xFE);

	/* Write the block (512 bytes) to the MMC/SD-card */
	for (b = 0; b < 512; b++)
		spi_write(*Buffer++);

	/* write the CRC-Byte */
	spi_write(0xFF); /* write a dummy CRC */
	spi_write(0xFF); /* CRC code is not used */

	/* Wait for MMC/SD-card busy */
	while (spi_read() != 0xff) {};

	/* set MMC_Chip_Select to high (MMC/SD-card inactive) */
	MMC_Disable();
	spi_unlock();
	return (0);
}

/* #########################################################################
   Routine to read the CSD register from the MMC/SD-card (16 bytes)
   ######################################################################### */
unsigned char mmc_read_csd (unsigned char *Buffer)
{
	/* Command to read the CSD register */
	unsigned char CMD[] = {0x49, 0x00, 0x00, 0x00, 0x00, 0xFF};

	MMC_Read_Block(CMD, Buffer, 16);

	return (0);
}
