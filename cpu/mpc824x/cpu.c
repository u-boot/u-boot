/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <config.h>
#include <mpc824x.h>
#include <common.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

int checkcpu (void)
{
	unsigned int pvr = get_pvr ();
	unsigned int version = pvr >> 16;
	unsigned char revision;
	ulong clock = gd->cpu_clk;
	char buf[32];

	puts ("CPU:   ");

	switch (version) {
	case CPU_TYPE_8240:
		puts ("MPC8240");
		break;

	case CPU_TYPE_8245:
		puts ("MPC8245");
		break;

	default:
		return -1;		/*not valid for this source */
	}

	CONFIG_READ_BYTE (REVID, revision);

	if (revision) {
		printf (" Revision %d.%d",
			(revision & 0xf0) >> 4,
			(revision & 0x0f));
	} else {
		return -1;		/* no valid CPU revision info */
	}

	printf (" at %s MHz:", strmhz (buf, clock));

	printf (" %u kB I-Cache", checkicache () >> 10);
	printf (" %u kB D-Cache", checkdcache () >> 10);

	puts ("\n");

	return 0;
}

/* ------------------------------------------------------------------------- */
/* L1 i-cache                                                                */

int checkicache (void)
{
	 /*TODO*/
	 return 128 * 4 * 32;
};

/* ------------------------------------------------------------------------- */
/* L1 d-cache                                                                */

int checkdcache (void)
{
	 /*TODO*/
	 return 128 * 4 * 32;

};

/*------------------------------------------------------------------- */

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong msr, addr;

	/* Interrupts and MMU off */
	__asm__ ("mtspr    81, 0");

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~0x1030;
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
#ifdef CONFIG_SYS_RESET_ADDRESS
	addr = CONFIG_SYS_RESET_ADDRESS;
#else
	/*
	 * note: when CONFIG_SYS_MONITOR_BASE points to a RAM address,
	 * CONFIG_SYS_MONITOR_BASE - sizeof (ulong) is usually a valid
	 * address. Better pick an address known to be invalid on
	 * your system and assign it to CONFIG_SYS_RESET_ADDRESS.
	 * "(ulong)-1" used to be a good choice for many systems...
	 */
	addr = CONFIG_SYS_MONITOR_BASE - sizeof (ulong);
#endif
	((void (*)(void)) addr) ();
	return 1;

}

/* ------------------------------------------------------------------------- */

/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 * This is the sys_logic_clk (memory bus) divided by 4
 */
unsigned long get_tbclk (void)
{
	return ((get_bus_freq (0) + 2L) / 4L);
}

/* ------------------------------------------------------------------------- */

/*
 * The MPC824x has an integrated PCI controller known as the MPC107.
 * The following are MPC107 Bridge Controller and PCI Support functions
 *
 */

/*
 *  This procedure reads a 32-bit address MPC107 register, and returns
 *  a 32 bit value.  It swaps the address to little endian before
 *  writing it to config address, and swaps the value to big endian
 *  before returning to the caller.
 */
unsigned int mpc824x_mpc107_getreg (unsigned int regNum)
{
	unsigned int temp;

	/* swap the addr. to little endian */
	*(volatile unsigned int *) CHRP_REG_ADDR = PCISWAP (regNum);
	temp = *(volatile unsigned int *) CHRP_REG_DATA;
	return PCISWAP (temp);		/* swap the data upon return */
}

/*
 *  This procedure writes a 32-bit address MPC107 register.  It swaps
 *  the address to little endian before writing it to config address.
 */

void mpc824x_mpc107_setreg (unsigned int regNum, unsigned int regVal)
{
	/* swap the addr. to little endian */
	*(volatile unsigned int *) CHRP_REG_ADDR = PCISWAP (regNum);
	*(volatile unsigned int *) CHRP_REG_DATA = PCISWAP (regVal);
	return;
}


/*
 *  Write a byte (8 bits) to a memory location.
 */
void mpc824x_mpc107_write8 (unsigned int addr, unsigned char data)
{
	*(unsigned char *) addr = data;
	__asm__ ("sync");
}

/*
 *  Write a word (16 bits) to a memory location after the value
 *  has been byte swapped (big to little endian or vice versa)
 */

void mpc824x_mpc107_write16 (unsigned int address, unsigned short data)
{
	*(volatile unsigned short *) address = BYTE_SWAP_16_BIT (data);
	__asm__ ("sync");
}

/*
 *  Write a long word (32 bits) to a memory location after the value
 *  has been byte swapped (big to little endian or vice versa)
 */

void mpc824x_mpc107_write32 (unsigned int address, unsigned int data)
{
	*(volatile unsigned int *) address = LONGSWAP (data);
	__asm__ ("sync");
}

/*
 *  Read a byte (8 bits) from a memory location.
 */
unsigned char mpc824x_mpc107_read8 (unsigned int addr)
{
	return *(volatile unsigned char *) addr;
}


/*
 *  Read a word (16 bits) from a memory location, and byte swap the
 *  value before returning to the caller.
 */
unsigned short mpc824x_mpc107_read16 (unsigned int address)
{
	unsigned short retVal;

	retVal = BYTE_SWAP_16_BIT (*(unsigned short *) address);
	return retVal;
}


/*
 *  Read a long word (32 bits) from a memory location, and byte
 *  swap the value before returning to the caller.
 */
unsigned int mpc824x_mpc107_read32 (unsigned int address)
{
	unsigned int retVal;

	retVal = LONGSWAP (*(unsigned int *) address);
	return (retVal);
}


/*
 *  Read a register in the Embedded Utilities Memory Block address
 *  space.
 *  Input: regNum - register number + utility base address.  Example,
 *         the base address of EPIC is 0x40000, the register number
 *	   being passed is 0x40000+the address of the target register.
 *	   (See epic.h for register addresses).
 *  Output:  The 32 bit little endian value of the register.
 */

unsigned int mpc824x_eummbar_read (unsigned int regNum)
{
	unsigned int temp;

	temp = *(volatile unsigned int *) (EUMBBAR_VAL + regNum);
	temp = PCISWAP (temp);
	return temp;
}


/*
 *  Write a value to a register in the Embedded Utilities Memory
 *  Block address space.
 *  Input: regNum - register number + utility base address.  Example,
 *                  the base address of EPIC is 0x40000, the register
 *	            number is 0x40000+the address of the target register.
 *	            (See epic.h for register addresses).
 *         regVal - value to be written to the register.
 */

void mpc824x_eummbar_write (unsigned int regNum, unsigned int regVal)
{
	*(volatile unsigned int *) (EUMBBAR_VAL + regNum) = PCISWAP (regVal);
	return;
}

/* ------------------------------------------------------------------------- */
