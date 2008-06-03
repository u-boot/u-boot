/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
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
 *
 * Adapted for PATI
 */

#include <common.h>
#include <command.h>
#define PLX9056_LOC
#include "plx9056.h"
#include "pati.h"
#include "pci_eeprom.h"

extern void show_pld_regs(void);
extern int do_mplcommon(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

extern void user_led0(int led_on);
extern void user_led1(int led_on);

/* ------------------------------------------------------------------------- */
#if defined(CFG_PCI_CON_DEVICE)
extern void pci_con_disc(void);
extern void pci_con_connect(void);
#endif

/******************************************************************************
 * Eeprom Support
 ******************************************************************************/
unsigned long get32(unsigned long addr)
{
	unsigned long *p=(unsigned long *)addr;
	return *p;
}

void set32(unsigned long addr,unsigned long data)
{
	unsigned long *p=(unsigned long *)addr;
	*p=data;
}

#define PCICFG_GET_REG(x)	(get32((x) + PCI_CONFIG_BASE))
#define PCICFG_SET_REG(x,y)	(set32((x) + PCI_CONFIG_BASE,(y)))


/******************************************************************************
 * reload_pci_eeprom
 ******************************************************************************/

static void reload_pci_eeprom(void)
{
	unsigned long reg;
	/* Set Bit 29 and clear it again */
	reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
	udelay(1);
	/* set it*/
	reg|=(1<<29);
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
	/* EECLK @ 33MHz = 125kHz
	 * -> extra long load = 32 * 16bit = 512Bit @ 125kHz = 4.1msec
	 * use 20msec
	 */
	udelay(20000); /* wait 20ms */
	reg &= ~(1<<29); /* set it low */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
	udelay(1); /* wait some time */
}

/******************************************************************************
 * clock_pci_eeprom
 ******************************************************************************/

static void clock_pci_eeprom(void)
{
	unsigned long reg;
	/* clock is low, data is valid */
	reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
	udelay(1);
	/* set clck high */
	reg|=(1<<24);
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
	udelay(1); /* wait some time */
	reg &= ~(1<<24); /* set clock low */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
	udelay(1); /* wait some time */
}

/******************************************************************************
 * send_pci_eeprom_cmd
 ******************************************************************************/
static void send_pci_eeprom_cmd(unsigned long cmd, unsigned char len)
{
	unsigned long reg;
	int i;
	reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
	/* Clear all EEPROM bits */
	reg &= ~(0xF << 24);
	/* Toggle EEPROM's Chip select to get it out of Shift Register Mode */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
	udelay(1); /* wait some time */
	/* Enable EEPROM Chip Select */
	reg |= (1 << 25);
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
	/* Send EEPROM command - one bit at a time */
	for (i = (int)(len-1); i >= 0; i--) {
		/* Check if current bit is 0 or 1 */
		if (cmd & (1 << i))
			PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,(reg | (1<<26)));
		else
			PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg);
		clock_pci_eeprom();
	}
}

/******************************************************************************
 * write_pci_eeprom_offs
 ******************************************************************************/
static void write_pci_eeprom_offs(unsigned short offset, unsigned short value)
{
	unsigned long reg;
	int bitpos, cmdshft, cmdlen, timeout;
	/* we're using the Eeprom 93CS66 */
	cmdshft  = 2;
	cmdlen = EE66_CMD_LEN;
	/* Send Write_Enable command to EEPROM */
	send_pci_eeprom_cmd((EE_WREN << cmdshft),cmdlen);
	/* Send EEPROM Write command and offset to EEPROM */
	send_pci_eeprom_cmd((EE_WRITE << cmdshft) | (offset / 2),cmdlen);
	reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
	/* Clear all EEPROM bits */
	reg &= ~(0xF << 24);
	/* Make sure EEDO Input is disabled for some PLX chips */
	reg &= ~(1 << 31);
	/* Enable EEPROM Chip Select */
	reg |= (1 << 25);
	/* Write 16-bit value to EEPROM - one bit at a time */
	for (bitpos = 15; bitpos >= 0; bitpos--) {
		/* Get bit value and shift into result */
		if (value & (1 << bitpos))
			PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,(reg | (1<<26)));
		else
			PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg );
		clock_pci_eeprom();
	} /* for */
	/* Deselect Chip */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg & ~(1 << 25));
	/* Re-select Chip */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg | (1 << 25));
	/* A small delay is needed to let EEPROM complete */
	timeout = 0;
	do {
		udelay(10);
		reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
		timeout++;
	} while (((reg & (1 << 27)) == 0) && timeout < 20000);
	/* Send Write_Disable command to EEPROM */
	send_pci_eeprom_cmd((EE_WDS << cmdshft),cmdlen);
	/* Clear Chip Select and all other EEPROM bits */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg & ~(0xF << 24));
}


/******************************************************************************
 * read_pci_eeprom_offs
 ******************************************************************************/
static void read_pci_eeprom_offs(unsigned short offset, unsigned short *pvalue)
{
	unsigned long reg;
	int bitpos, cmdshft, cmdlen;
	/* we're using the Eeprom 93CS66 */
	cmdshft  = 2;
	cmdlen = EE66_CMD_LEN;
	/* Send EEPROM read command and offset to EEPROM */
	send_pci_eeprom_cmd((EE_READ << cmdshft) | (offset / 2),cmdlen);
	/* Set EEPROM write output bit */
	reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
	/* Set EEDO Input enable */
	reg |= (1 << 31);
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg | (1 << 26));
	/* Get 16-bit value from EEPROM - one bit at a time */
	for (bitpos = 0; bitpos < 16; bitpos++) {
		clock_pci_eeprom();
		udelay(10);
		reg=PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT);
		/* Get bit value and shift into result */
		if (reg & (1 << 27))
			*pvalue = (unsigned short)((*pvalue << 1) | 1);
		else
			*pvalue = (unsigned short)(*pvalue << 1);
	}
	/* Clear EEDO Input enable */
	reg &= ~(1 << 31);
	/* Clear Chip Select and all other EEPROM bits */
	PCICFG_SET_REG(PCI9056_EEPROM_CTRL_STAT,reg & ~(0xF << 24));
}


/******************************************************************************
 * EEPROM read/writes
******************************************************************************/

#undef EEPROM_DBG
static int pati_pci_eeprom_erase(void)
{
	int i;
	printf("Erasing EEPROM ");
	for( i=0; i < PATI_EEPROM_LAST_OFFSET; i+=2) {
		write_pci_eeprom_offs(i,0xffff);
		if((i%0x10))
			printf(".");
	}
	printf("\nDone\n");
	return 0;
}

static int pati_pci_eeprom_prg(void)
{
	int i;
	i=0;
	printf("Programming EEPROM ");
	while(pati_eeprom[i].offset<0xffff) {
		write_pci_eeprom_offs(pati_eeprom[i].offset,pati_eeprom[i].value);
		#ifdef EEPROM_DBG
		printf("0x%04X: 0x%04X\n",pati_eeprom[i].offset, pati_eeprom[i].value);
		#else
		if((i%0x10))
			printf(".");
		#endif
		i++;
	}
	printf("\nDone\n");
	return 0;
}

static int pati_pci_eeprom_write(unsigned short offset, unsigned long addr, unsigned short size)
{
	int i;
	unsigned short value;
	unsigned short *buffer =(unsigned short *)addr;
	if((offset + size) > PATI_EEPROM_LAST_OFFSET) {
		size = PATI_EEPROM_LAST_OFFSET - offset;
	}
	printf("Write To EEPROM from 0x%lX to 0x%X 0x%X words\n", addr, offset, size/2);
	for( i = offset; i< (offset + size); i+=2) {
		value = *buffer++;
		write_pci_eeprom_offs(i,value);
		#ifdef EEPROM_DBG
		printf("0x%04X: 0x%04X\n",i, value);
		#else
		if((i%0x10))
			printf(".");
		#endif
	}
	printf("\nDone\n");
	return 0;
}

static int pati_pci_eeprom_read(unsigned short offset, unsigned long addr, unsigned short size)
{
	int i;
	unsigned short value;
	unsigned short *buffer =(unsigned short *)addr;
	if((offset + size) > PATI_EEPROM_LAST_OFFSET) {
		size = PATI_EEPROM_LAST_OFFSET - offset;
	}
	printf("Read from EEPROM from 0x%X to 0x%lX 0x%X words\n", offset, addr, size/2);
	for( i = offset; i< (offset + size); i+=2) {
		read_pci_eeprom_offs(i,&value);
		*buffer++=value;
		#ifdef EEPROM_DBG
		printf("0x%04X: 0x%04X\n",i, value);
		#else
		if((i%0x10))
			printf(".");
		#endif
	}
	printf("\nDone\n");
	return 0;
}

/******************************************************************************
 * PCI Bridge Registers Dump
*******************************************************************************/
static void display_pci_regs(void)
{
	printf(" PCI9056_SPACE0_RANGE     %08lX\n",PCICFG_GET_REG(PCI9056_SPACE0_RANGE));
	printf(" PCI9056_SPACE0_REMAP     %08lX\n",PCICFG_GET_REG(PCI9056_SPACE0_REMAP));
	printf(" PCI9056_LOCAL_DMA_ARBIT  %08lX\n",PCICFG_GET_REG(PCI9056_LOCAL_DMA_ARBIT));
	printf(" PCI9056_ENDIAN_DESC      %08lX\n",PCICFG_GET_REG(PCI9056_ENDIAN_DESC));
	printf(" PCI9056_EXP_ROM_RANGE    %08lX\n",PCICFG_GET_REG(PCI9056_EXP_ROM_RANGE));
	printf(" PCI9056_EXP_ROM_REMAP    %08lX\n",PCICFG_GET_REG(PCI9056_EXP_ROM_REMAP));
	printf(" PCI9056_SPACE0_ROM_DESC  %08lX\n",PCICFG_GET_REG(PCI9056_SPACE0_ROM_DESC));
	printf(" PCI9056_DM_RANGE         %08lX\n",PCICFG_GET_REG(PCI9056_DM_RANGE));
	printf(" PCI9056_DM_MEM_BASE      %08lX\n",PCICFG_GET_REG(PCI9056_DM_MEM_BASE));
	printf(" PCI9056_DM_IO_BASE       %08lX\n",PCICFG_GET_REG(PCI9056_DM_IO_BASE));
	printf(" PCI9056_DM_PCI_MEM_REMAP %08lX\n",PCICFG_GET_REG(PCI9056_DM_PCI_MEM_REMAP));
	printf(" PCI9056_DM_PCI_IO_CONFIG %08lX\n",PCICFG_GET_REG(PCI9056_DM_PCI_IO_CONFIG));
	printf(" PCI9056_SPACE1_RANGE     %08lX\n",PCICFG_GET_REG(PCI9056_SPACE1_RANGE));
	printf(" PCI9056_SPACE1_REMAP     %08lX\n",PCICFG_GET_REG(PCI9056_SPACE1_REMAP));
	printf(" PCI9056_SPACE1_DESC      %08lX\n",PCICFG_GET_REG(PCI9056_SPACE1_DESC));
	printf(" PCI9056_DM_DAC           %08lX\n",PCICFG_GET_REG(PCI9056_DM_DAC));
	printf(" PCI9056_MAILBOX0         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX0));
	printf(" PCI9056_MAILBOX1         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX1));
	printf(" PCI9056_MAILBOX2         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX2));
	printf(" PCI9056_MAILBOX3         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX3));
	printf(" PCI9056_MAILBOX4         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX4));
	printf(" PCI9056_MAILBOX5         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX5));
	printf(" PCI9056_MAILBOX6         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX6));
	printf(" PCI9056_MAILBOX7         %08lX\n",PCICFG_GET_REG(PCI9056_MAILBOX7));
	printf(" PCI9056_PCI_TO_LOC_DBELL %08lX\n",PCICFG_GET_REG(PCI9056_PCI_TO_LOC_DBELL));
	printf(" PCI9056_LOC_TO_PCI_DBELL %08lX\n",PCICFG_GET_REG(PCI9056_LOC_TO_PCI_DBELL));
	printf(" PCI9056_INT_CTRL_STAT    %08lX\n",PCICFG_GET_REG(PCI9056_INT_CTRL_STAT));
	printf(" PCI9056_EEPROM_CTRL_STAT %08lX\n",PCICFG_GET_REG(PCI9056_EEPROM_CTRL_STAT));
	printf(" PCI9056_PERM_VENDOR_ID   %08lX\n",PCICFG_GET_REG(PCI9056_PERM_VENDOR_ID));
	printf(" PCI9056_REVISION_ID      %08lX\n",PCICFG_GET_REG(PCI9056_REVISION_ID));
	printf(" \n");
	printf(" PCI9056_VENDOR_ID        %08lX\n",PCICFG_GET_REG(PCI9056_VENDOR_ID));
	printf(" PCI9056_COMMAND          %08lX\n",PCICFG_GET_REG(PCI9056_COMMAND));
	printf(" PCI9056_REVISION         %08lX\n",PCICFG_GET_REG(PCI9056_REVISION));
	printf(" PCI9056_CACHE_SIZE       %08lX\n",PCICFG_GET_REG(PCI9056_CACHE_SIZE));
	printf(" PCI9056_RTR_BASE         %08lX\n",PCICFG_GET_REG(PCI9056_RTR_BASE));
	printf(" PCI9056_RTR_IO_BASE      %08lX\n",PCICFG_GET_REG(PCI9056_RTR_IO_BASE));
	printf(" PCI9056_LOCAL_BASE0      %08lX\n",PCICFG_GET_REG(PCI9056_LOCAL_BASE0));
	printf(" PCI9056_LOCAL_BASE1      %08lX\n",PCICFG_GET_REG(PCI9056_LOCAL_BASE1));
	printf(" PCI9056_UNUSED_BASE1     %08lX\n",PCICFG_GET_REG(PCI9056_UNUSED_BASE1));
	printf(" PCI9056_UNUSED_BASE2     %08lX\n",PCICFG_GET_REG(PCI9056_UNUSED_BASE2));
	printf(" PCI9056_CIS_PTR          %08lX\n",PCICFG_GET_REG(PCI9056_CIS_PTR));
	printf(" PCI9056_SUB_ID           %08lX\n",PCICFG_GET_REG(PCI9056_SUB_ID));
	printf(" PCI9056_EXP_ROM_BASE     %08lX\n",PCICFG_GET_REG(PCI9056_EXP_ROM_BASE));
	printf(" PCI9056_CAP_PTR          %08lX\n",PCICFG_GET_REG(PCI9056_CAP_PTR));
	printf(" PCI9056_INT_LINE         %08lX\n",PCICFG_GET_REG(PCI9056_INT_LINE));
	printf(" PCI9056_PM_CAP_ID        %08lX\n",PCICFG_GET_REG(PCI9056_PM_CAP_ID));
	printf(" PCI9056_PM_CSR           %08lX\n",PCICFG_GET_REG(PCI9056_PM_CSR));
	printf(" PCI9056_HS_CAP_ID        %08lX\n",PCICFG_GET_REG(PCI9056_HS_CAP_ID));
	printf(" PCI9056_VPD_CAP_ID       %08lX\n",PCICFG_GET_REG(PCI9056_VPD_CAP_ID));
	printf(" PCI9056_VPD_DATA         %08lX\n",PCICFG_GET_REG(PCI9056_VPD_DATA));
}


int do_pati(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (strcmp(argv[1], "info") == 0)
	{
		show_pld_regs();
		return 0;
	}
	if (strcmp(argv[1], "pci") == 0)
	{
		display_pci_regs();
		return 0;
	}
	if (strcmp(argv[1], "led") == 0)
	{
		int led_nr,led_on;
		led_nr = (int)simple_strtoul(argv[2], NULL, 10);
		led_on = (int)simple_strtoul(argv[3], NULL, 10);
		if(!led_nr)
			user_led0(led_on);
		else
			user_led1(led_on);
		return 0;
	}
#if defined(CFG_PCI_CON_DEVICE)
	if (strcmp(argv[1], "con") == 0) {
		pci_con_connect();
		return 0;
	}
	if (strcmp(argv[1], "disc") == 0) {
		pci_con_disc();
		return 0;
	}
#endif
	if (strcmp(argv[1], "eeprom") == 0) {
		unsigned long addr;
		int size, offset;
		offset = 0;
		size = PATI_EEPROM_LAST_OFFSET;
		if(argc>2) {
			if(argc>3) {
				addr = simple_strtoul(argv[3], NULL, 16);
				if(argc>4)
					offset = (int) simple_strtoul(argv[4], NULL, 16);
				if(argc>5)
					size = (int) simple_strtoul(argv[5], NULL, 16);
				if (strcmp(argv[2], "read") == 0) {
					return (pati_pci_eeprom_read(offset, addr, size));
				}
				if (strcmp(argv[2], "write") == 0) {
					return (pati_pci_eeprom_write(offset, addr, size));
				}
			}
			if (strcmp(argv[2], "prg") == 0) {
				return (pati_pci_eeprom_prg());
			}
			if (strcmp(argv[2], "era") == 0) {
				return (pati_pci_eeprom_erase());
			}
			if (strcmp(argv[2], "reload") == 0) {
				reload_pci_eeprom();
				return 0;
			}


		}
	}

	return (do_mplcommon(cmdtp, flag, argc, argv));
}

U_BOOT_CMD(
	pati,	8,	1,	do_pati,
	"pati    - PATI specific Cmds\n",
	"info - displays board information\n"
	"pati pci  - displays PCI registers\n"
	"pati led <nr> <on> \n"
	"          - switch LED <nr> <on>\n"
	"pati flash mem [SrcAddr]\n"
	"          - updates U-Boot with image in memory\n"
	"pati eeprom <cmd> - PCI EEPROM sub-system\n"
	"    read <addr> <offset> <size>\n"
	"          - read PCI EEPROM to <addr> from <offset> <size> words\n"
	"    write <addr> <offset> <size>\n"
	"          - write PCI EEPROM from <addr> to <offset> <size> words\n"
	"    prg   - programm PCI EEPROM with default values\n"
	"    era   - erase PCI EEPROM (write all word to 0xffff)\n"
	"    reload- Reload PCI Bridge with EEPROM Values\n"
	"    NOTE: <addr> must start on word boundary\n"
	"          <offset> and <size> must be even byte values\n"
);

/* ------------------------------------------------------------------------- */
