/*
 * (C) Copyright 2005
 * Thomas.Lange@corelatus.se
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
#include <command.h>
#include <asm/au1x00.h>
#include <asm/addrspace.h>
#include <asm/mipsregs.h>
#include <asm/io.h>
#include <watchdog.h>

#include "ee_access.h"

static int wdi_status = 0;

#define SDRAM_SIZE ((64*1024*1024)-(12*4096))


#define SERIAL_LOG_BUFFER KSEG1ADDR(SDRAM_SIZE + (8*4096))

void inline log_serial_char(char c){
	char *serial_log_buffer = (char*)SERIAL_LOG_BUFFER;
	int serial_log_offset;
	u32 *serial_log_offsetp = (u32*)SERIAL_LOG_BUFFER;

	serial_log_offset = *serial_log_offsetp;

	*(serial_log_buffer + serial_log_offset) = c;

	serial_log_offset++;

	if(serial_log_offset >= 4096){
		serial_log_offset = 4;
	}
	*serial_log_offsetp = serial_log_offset;
}

void init_log_serial(void){
	char *serial_log_buffer = (char*)SERIAL_LOG_BUFFER;
	u32 *serial_log_offsetp = (u32*)SERIAL_LOG_BUFFER;

	/* Copy buffer from last run */
	memcpy(serial_log_buffer + 4096,
	       serial_log_buffer,
	       4096);

	memset(serial_log_buffer, 0, 4096);

	*serial_log_offsetp = 4;
}


void hw_watchdog_reset(void){
	volatile u32 *sys_outputset = (volatile u32*)SYS_OUTPUTSET;
	volatile u32 *sys_outputclear = (volatile u32*)SYS_OUTPUTCLR;
	if(wdi_status){
		*sys_outputset = GPIO_CPU_LED|GPIO_WDI;
		wdi_status = 0;
	}
	else{
		*sys_outputclear = GPIO_CPU_LED|GPIO_WDI;
		wdi_status = 1;
	}
}

long int initdram(int board_type)
{
	/* Sdram is setup by assembler code */
	/* If memory could be changed, we should return the true value here */

	WATCHDOG_RESET();

	return (SDRAM_SIZE);
}

/* In cpu/mips/cpu.c */
void write_one_tlb( int index, u32 pagemask, u32 hi, u32 low0, u32 low1 );

void set_ledcard(u32 value){
	/* Clock 24 bits to led card */
	int i;
	volatile u32 *sys_outputset = (volatile u32*)SYS_OUTPUTSET;
	volatile u32 *sys_outputclr = (volatile u32*)SYS_OUTPUTCLR;

	/* Start with known values */
	*sys_outputclr = GPIO_LEDCLK|GPIO_LEDD;

	for(i=0;i<24;i++){
		if(value&0x00800000){
			*sys_outputset = GPIO_LEDD;
		}
		else{
			*sys_outputclr = GPIO_LEDD;
		}
		udelay(1);
		*sys_outputset = GPIO_LEDCLK;
		udelay(1);
		*sys_outputclr = GPIO_LEDCLK;
		udelay(1);

		value<<=1;
	}
	/* Data is enable output */
	*sys_outputset = GPIO_LEDD;
}

int checkboard (void)
{
	volatile u32 *sys_counter = (volatile u32*)SYS_COUNTER_CNTRL;
	volatile u32 *sys_outputset = (volatile u32*)SYS_OUTPUTSET;
	volatile u32 *sys_outputclr = (volatile u32*)SYS_OUTPUTCLR;
	u32 proc_id;

	WATCHDOG_RESET();

	*sys_counter = 0x100; /* Enable 32 kHz oscillator for RTC/TOY */

	proc_id = read_32bit_cp0_register(CP0_PRID);

	switch (proc_id >> 24) {
	case 0:
		puts ("Board: GTH2\n");
		printf ("CPU: Au1000 500 MHz, id: 0x%02x, rev: 0x%02x\n",
			(proc_id >> 8) & 0xFF, proc_id & 0xFF);
		break;
	default:
		printf ("Unsupported cpu %d, proc_id=0x%x\n", proc_id >> 24, proc_id);
	}

	set_io_port_base(0);

#ifdef CONFIG_IDE_PCMCIA
	/* PCMCIA is on a 36 bit physical address.
	   We need to map it into a 32 bit addresses */
	write_one_tlb(20,                 /* index */
		      0x01ffe000,         /* Pagemask, 16 MB pages */
		      CFG_PCMCIA_IO_BASE, /* Hi */
		      0x3C000017,         /* Lo0 */
		      0x3C200017);        /* Lo1 */

	write_one_tlb(21,                   /* index */
		      0x01ffe000,           /* Pagemask, 16 MB pages */
		      CFG_PCMCIA_ATTR_BASE, /* Hi */
		      0x3D000017,           /* Lo0 */
		      0x3D200017);          /* Lo1 */

	write_one_tlb(22,                   /* index */
		      0x01ffe000,           /* Pagemask, 16 MB pages */
		      CFG_PCMCIA_MEM_ADDR,  /* Hi */
		      0x3E000017,           /* Lo0 */
		      0x3E200017);          /* Lo1 */

#endif	/* CONFIG_IDE_PCMCIA */

	/* Wait for GPIO ports to become stable */
	udelay(5000); /* FIXME */

	/* Release reset of ethernet PHY chips */
	/* Always do this, because linux does not know about it */
	*sys_outputset = GPIO_ERESET;

	/* Kill FPGA:s */
	*sys_outputclr = GPIO_CACONFIG|GPIO_DPACONFIG;
	udelay(2);
	*sys_outputset = GPIO_CACONFIG|GPIO_DPACONFIG;

	/* Turn front led yellow */
	set_ledcard(0x00100000);

	return 0;
}

#define POWER_OFFSET    0xF0000
#define SW_WATCHDOG_REASON 13

#define BOOTDATA_OFFSET 0xF8000
#define MAX_ATTEMPTS 5

#define FAILSAFE_BOOT 1
#define SYSTEM_BOOT   2
#define SYSTEM2_BOOT  3

#define WRITE_FLASH16(a, d)      \
do                              \
{                               \
  *((volatile u16 *) (a)) = (d);\
 } while(0)

static void write_bootdata (volatile u16 * addr, u8 System, u8 Count)
{
	u16 data;
	volatile u16 *flash = (u16 *) (CFG_FLASH_BASE);

	switch(System){
	case FAILSAFE_BOOT:
		printf ("Setting failsafe boot in flash\n");
		break;
	case SYSTEM_BOOT:
		printf ("Setting system boot in flash\n");
		break;
	case SYSTEM2_BOOT:
		printf ("Setting system2 boot in flash\n");
		break;
	default:
		printf ("Invalid system data %u, setting failsafe\n", System);
		System = FAILSAFE_BOOT;
	}

	if ((Count < 1) | (Count > MAX_ATTEMPTS)) {
		printf ("Invalid boot count %u, setting 1\n", Count);
		Count = 1;
	}

	printf ("Boot attempt %d\n", Count);

	data = (System << 8) | Count;
	/* AMD 16 bit */
	WRITE_FLASH16 (&flash[0x555], 0xAAAA);
	WRITE_FLASH16 (&flash[0x2AA], 0x5555);
	WRITE_FLASH16 (&flash[0x555], 0xA0A0);

	WRITE_FLASH16 (addr, data);
}

static int random_system(void){
	/* EEPROM read failed. Just try to choose one
	   system release and hope it works */

	/* FIXME */
	return(SYSTEM_BOOT);
}

static int switch_system(int old_system){
	u8 Rx[10];
	u8 Tx[5];
	int valid_release;

	if(old_system==FAILSAFE_BOOT){
		/* Find out which system release to use */

		/* Copy from nvram to scratchpad */
		Tx[0] = RECALL_MEMORY;
		Tx[1] = 7; /* Page */
		if (ee_do_cpu_command (Tx, 2, NULL, 0, 1)) {
			printf ("EE user page 7 recall failed\n");
			return (random_system());
		}

		Tx[0] = READ_SCRATCHPAD;
		if (ee_do_cpu_command (Tx, 2, Rx, 9, 1)) {
			printf ("EE user page 7 read failed\n");
			return (random_system());
		}
		/* Crc in 9:th byte */
		if (!ee_crc_ok (Rx, 8, *(Rx + 8))) {
			printf ("EE read failed, page 7. CRC error\n");
			return (random_system());
		}

		valid_release = Rx[7];
		if((valid_release==0xFF)|
		   ((valid_release&1) == 0)){
			return(SYSTEM_BOOT);
		}
		else{
			return(SYSTEM2_BOOT);
		}
	}
	else{
		return(FAILSAFE_BOOT);
	}
}

static void check_boot_tries (void)
{
	/* Count the number of boot attemps
	   switch system if too many */

	int i;
	volatile u16 *addr;
	volatile u16 data;
	u8 system = FAILSAFE_BOOT;
	u8 count;

	addr = (u16 *) (CFG_FLASH_BASE + BOOTDATA_OFFSET);

	if (*addr == 0xFFFF) {
		printf ("*** No bootdata exists. ***\n");
		write_bootdata (addr, FAILSAFE_BOOT, 1);
	} else {
		/* Search for latest written bootdata */
		i = 0;
		while ((*(addr + 1) != 0xFFFF) & (i < 8000)) {
			addr++;
			i++;
		}
		if (i >= 8000) {
			/* Whoa, dont write any more */
			printf ("*** No bootdata found. Not updating flash***\n");
		} else {
			/* See how many times we have tried to boot real system */
			data = *addr;
			system = data >> 8;
			count = data & 0xFF;
			if ((system != SYSTEM_BOOT) &
			    (system != SYSTEM2_BOOT) &
			    (system != FAILSAFE_BOOT)) {
				printf ("*** Wrong system %d\n", system);
				system = FAILSAFE_BOOT;
				count = 1;
			} else {
				switch (count) {
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
					/* Try same system again if needed */
					count++;
					break;

				case 5:
					/* Switch system and reset tries */
					count = 1;
					system = switch_system(system);
					printf ("***Too many boot attempts, switching system***\n");
					break;
				default:
					/* Switch system, start over and hope it works */
					printf ("***Unexpected data on addr 0x%x, %u***\n",
						(u32) addr, data);
					count = 1;
					system = switch_system(system);
				}
			}
			write_bootdata (addr + 1, system, count);
		}
	}
	switch(system){
	case FAILSAFE_BOOT:
		printf ("Booting failsafe system\n");
		setenv ("bootargs", "panic=1 root=/dev/hda7");
		setenv ("bootcmd", "ide reset;disk 0x81000000 0:5;run addmisc;bootm");
		break;

	case SYSTEM_BOOT:
		printf ("Using normal system\n");
		setenv ("bootargs", "panic=1 root=/dev/hda4");
		setenv ("bootcmd", "ide reset;disk 0x81000000 0:2;run addmisc;bootm");
		break;

	case SYSTEM2_BOOT:
		printf ("Using normal system2\n");
		setenv ("bootargs", "panic=1 root=/dev/hda9");
		setenv ("bootcmd", "ide reset;disk 0x81000000 0:8;run addmisc;bootm");
		break;
	default:
		printf ("Invalid system %d\n", system);
		printf ("Hanging\n");
		while(1);
	}
}

int misc_init_r(void){
	u8 Rx[80];
	u8 Tx[5];
	int page;
	int read = 0;

	WATCHDOG_RESET();

	if (ee_init_cpu_data ()) {
		printf ("EEPROM init failed\n");
		return (0);
	}

	/* Check which release to boot */
	check_boot_tries ();

	/* Read the pages where ethernet address is stored */

	for (page = EE_USER_PAGE_0; page <= EE_USER_PAGE_0 + 2; page++) {
		/* Copy from nvram to scratchpad */
		Tx[0] = RECALL_MEMORY;
		Tx[1] = page;
		if (ee_do_cpu_command (Tx, 2, NULL, 0, 1)) {
			printf ("EE user page %d recall failed\n", page);
			return (0);
		}

		Tx[0] = READ_SCRATCHPAD;
		if (ee_do_cpu_command (Tx, 2, Rx + read, 9, 1)) {
			printf ("EE user page %d read failed\n", page);
			return (0);
		}
		/* Crc in 9:th byte */
		if (!ee_crc_ok (Rx + read, 8, *(Rx + read + 8))) {
			printf ("EE read failed, page %d. CRC error\n", page);
			return (0);
		}
		read += 8;
	}

	/* Add eos after eth addr */
	Rx[17] = 0;

	printf ("Ethernet addr read from eeprom: %s\n\n", Rx);

	if ((Rx[2] != ':') |
	    (Rx[5] != ':') |
	    (Rx[8] != ':') | (Rx[11] != ':') | (Rx[14] != ':')) {
		printf ("*** ethernet addr invalid, using default ***\n");
	} else {
		setenv ("ethaddr", (char *)Rx);
	}
	return (0);
}
