/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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

/*
 * SRom I/O routines.
 */

#include <common.h>
#include <pci.h>
#include "srom.h"

#define SROM_RD         0x00004000    /* Read from Boot ROM */
#define SROM_WR         0x00002000    /* Write to Boot ROM */
#define SROM_SR         0x00000800    /* Select Serial ROM when set */

#define DT_IN           0x00000004    /* Serial Data In */
#define DT_CLK          0x00000002    /* Serial ROM Clock */
#define DT_CS           0x00000001    /* Serial ROM Chip Select */

static u_int         dc_srom_iobase;

/*----------------------------------------------------------------------------*/

static int inl(u_long addr)
{
    return le32_to_cpu(*(volatile u_long *)(addr));
}

/*----------------------------------------------------------------------------*/

static void outl (int command, u_long addr)
{
    *(volatile u_long *)(addr) = cpu_to_le32(command);
}

/*----------------------------------------------------------------------------*/

static void sendto_srom(u_int command, u_long addr)
{
    outl(command, addr);
    udelay(1);

    return;
}

/*----------------------------------------------------------------------------*/

static int getfrom_srom(u_long addr)
{
    s32 tmp;

    tmp = inl(addr);
    udelay(1);

    return tmp;
}

/*----------------------------------------------------------------------------*/

static void srom_latch (u_int command, u_long addr)
{
    sendto_srom (command, addr);
    sendto_srom (command | DT_CLK, addr);
    sendto_srom (command, addr);

    return;
}

/*----------------------------------------------------------------------------*/

static void srom_command_rd (u_int command, u_long addr)
{
    srom_latch (command, addr);
    srom_latch (command, addr);
    srom_latch ((command & 0x0000ff00) | DT_CS, addr);

    return;
}

/*----------------------------------------------------------------------------*/

static void srom_command_wr (u_int command, u_long addr)
{
    srom_latch (command, addr);
    srom_latch ((command & 0x0000ff00) | DT_CS, addr);
    srom_latch (command, addr);

    return;
}

/*----------------------------------------------------------------------------*/

static void srom_address(u_int command, u_long addr, u_char offset)
{
    int i;
    signed char a;

    a = (char)(offset << 2);
    for (i=0; i<6; i++, a <<= 1)
    {
	srom_latch(command | ((a < 0) ? DT_IN : 0), addr);
    }
    udelay(1);

    i = (getfrom_srom(addr) >> 3) & 0x01;

    return;
}
/*----------------------------------------------------------------------------*/

static short srom_data_rd (u_int command, u_long addr)
{
    int i;
    short word = 0;
    s32 tmp;

    for (i=0; i<16; i++)
    {
	sendto_srom(command  | DT_CLK, addr);
	tmp = getfrom_srom(addr);
	sendto_srom(command, addr);

	word = (word << 1) | ((tmp >> 3) & 0x01);
    }

    sendto_srom(command & 0x0000ff00, addr);

    return word;
}

/*----------------------------------------------------------------------------*/

static int srom_data_wr (u_int command, u_long addr, short val)
{
    int i;
    u_long longVal;
    s32 tmp;

    longVal = (u_long)(le16_to_cpu(val));

    for (i=0; i<16; i++)
    {
	tmp = (longVal & 0x8000)>>13;

	sendto_srom (tmp | command, addr);
	sendto_srom (tmp | command  | DT_CLK, addr);
	sendto_srom (tmp | command, addr);

	longVal = longVal<<1;
    }

    sendto_srom(command & 0x0000ff00, addr);
    sendto_srom(command, addr);

    tmp = 100;
    do
    {
	if ((getfrom_srom(dc_srom_iobase) & 0x8) == 0x8)
	    break;
	udelay(1000);
    } while (--tmp);

    if (tmp == 0)
    {
	printf("Write DEC21143 SRom timed out !\n");
	return (-1);
    }

    return 0;
}


/*----------------------------------------------------------------------------*/
static short srom_rd (u_long addr, u_char offset)
{
    sendto_srom (SROM_RD | SROM_SR, addr);
    srom_latch (SROM_RD | SROM_SR | DT_CS, addr);

    srom_command_rd (SROM_RD | SROM_SR | DT_IN | DT_CS, addr);

    srom_address (SROM_RD | SROM_SR | DT_CS, addr, offset);

    return srom_data_rd (SROM_RD | SROM_SR | DT_CS, addr);
}

/*----------------------------------------------------------------------------*/

static void srom_wr_enable (u_long addr)
{
    int i;

    sendto_srom (SROM_WR | SROM_SR, addr);
    srom_latch (SROM_WR | SROM_SR | DT_CS, addr);

    srom_latch (SROM_WR | SROM_SR | DT_IN | DT_CS, addr);
    srom_latch (SROM_WR | SROM_SR | DT_CS, addr);
    srom_latch (SROM_WR | SROM_SR | DT_CS, addr);

    for (i=0; i<6; i++)
    {
	srom_latch (SROM_WR | SROM_SR | DT_IN | DT_CS, addr);
    }
}

/*----------------------------------------------------------------------------*/

static int srom_wr (u_long addr, u_char offset, short val)
{
    srom_wr_enable (addr);

    sendto_srom (SROM_WR | SROM_SR, addr);
    srom_latch (SROM_WR | SROM_SR | DT_CS, addr);

    srom_command_wr (SROM_WR | SROM_SR | DT_IN | DT_CS, addr);

    srom_address (SROM_WR | SROM_SR | DT_CS, addr, offset);

    return srom_data_wr (SROM_WR | SROM_SR | DT_CS, addr, val);
}

/*----------------------------------------------------------------------------*/
/*
 * load data from the srom
 */
int dc_srom_load (u_short *dest)
{
    int offset;
    short tmp;

    /* get srom iobase from local network controller */
    pci_read_config_dword(PCI_BDF(0,14,0), PCI_BASE_ADDRESS_1, &dc_srom_iobase);
    dc_srom_iobase &= PCI_BASE_ADDRESS_MEM_MASK;
    dc_srom_iobase  = pci_mem_to_phys(PCI_BDF(0,14,0), dc_srom_iobase);
    dc_srom_iobase += 0x48;            /* io offset for srom access */

    memset (dest, 0, 128);
    for (offset=0; offset<64; offset++)
    {
	tmp = srom_rd (dc_srom_iobase, offset);
	*dest++ = le16_to_cpu(tmp);
    }

    return (0);
}

/*----------------------------------------------------------------------------*/

/*
 * store data into the srom
 */
int dc_srom_store (u_short *src)
{
    int offset;

    /* get srom iobase from local network controller */
    pci_read_config_dword(PCI_BDF(0,14,0), PCI_BASE_ADDRESS_1, &dc_srom_iobase);
    dc_srom_iobase &= PCI_BASE_ADDRESS_MEM_MASK;
    dc_srom_iobase  = pci_mem_to_phys(PCI_BDF(0,14,0), dc_srom_iobase);
    dc_srom_iobase += 0x48;            /* io offset for srom access */

    for (offset=0; offset<64; offset++)
    {
	if (srom_wr (dc_srom_iobase, offset, *src) == -1)
		return (-1);
	src++;
    }

    return (0);
}

/*----------------------------------------------------------------------------*/
