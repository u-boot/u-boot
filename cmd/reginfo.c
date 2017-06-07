/*
 * (C) Copyright 2000
 * Subodh Nijsure, SkyStream Networks, snijsure@skystream.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#if defined (CONFIG_4xx)
extern void ppc4xx_reginfo(void);
#elif defined (CONFIG_MPC5200)
#include <mpc5xxx.h>
#elif defined (CONFIG_MPC86xx)
extern void mpc86xx_reginfo(void);
#elif defined(CONFIG_MPC85xx)
extern void mpc85xx_reginfo(void);
#endif

static int do_reginfo(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
#if defined (CONFIG_4xx)
	ppc4xx_reginfo();
#elif defined(CONFIG_MPC5200)
	puts ("\nMPC5200 registers\n");
	printf ("MBAR=%08x\n", CONFIG_SYS_MBAR);
	puts ("Memory map registers\n");
	printf ("\tCS0: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS0_START,
		*(volatile ulong*)MPC5XXX_CS0_STOP,
		*(volatile ulong*)MPC5XXX_CS0_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x00010000) ? 1 : 0);
	printf ("\tCS1: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS1_START,
		*(volatile ulong*)MPC5XXX_CS1_STOP,
		*(volatile ulong*)MPC5XXX_CS1_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x00020000) ? 1 : 0);
	printf ("\tCS2: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS2_START,
		*(volatile ulong*)MPC5XXX_CS2_STOP,
		*(volatile ulong*)MPC5XXX_CS2_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x00040000) ? 1 : 0);
	printf ("\tCS3: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS3_START,
		*(volatile ulong*)MPC5XXX_CS3_STOP,
		*(volatile ulong*)MPC5XXX_CS3_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x00080000) ? 1 : 0);
	printf ("\tCS4: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS4_START,
		*(volatile ulong*)MPC5XXX_CS4_STOP,
		*(volatile ulong*)MPC5XXX_CS4_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x00100000) ? 1 : 0);
	printf ("\tCS5: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS5_START,
		*(volatile ulong*)MPC5XXX_CS5_STOP,
		*(volatile ulong*)MPC5XXX_CS5_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x00200000) ? 1 : 0);
	printf ("\tCS6: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS6_START,
		*(volatile ulong*)MPC5XXX_CS6_STOP,
		*(volatile ulong*)MPC5XXX_CS6_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x04000000) ? 1 : 0);
	printf ("\tCS7: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_CS7_START,
		*(volatile ulong*)MPC5XXX_CS7_STOP,
		*(volatile ulong*)MPC5XXX_CS7_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x08000000) ? 1 : 0);
	printf ("\tBOOTCS: start %08lX\tstop %08lX\tconfig %08lX\ten %d\n",
		*(volatile ulong*)MPC5XXX_BOOTCS_START,
		*(volatile ulong*)MPC5XXX_BOOTCS_STOP,
		*(volatile ulong*)MPC5XXX_BOOTCS_CFG,
		(*(volatile ulong*)MPC5XXX_ADDECR & 0x02000000) ? 1 : 0);
	printf ("\tSDRAMCS0: %08lX\n",
		*(volatile ulong*)MPC5XXX_SDRAM_CS0CFG);
	printf ("\tSDRAMCS1: %08lX\n",
		*(volatile ulong*)MPC5XXX_SDRAM_CS1CFG);
#elif defined(CONFIG_MPC86xx)
	mpc86xx_reginfo();

#elif defined(CONFIG_MPC85xx)
	mpc85xx_reginfo();
#endif

	return 0;
}

 /**************************************************/

#if defined(CONFIG_CMD_REGINFO)
U_BOOT_CMD(
	reginfo,	2,	1,	do_reginfo,
	"print register information",
	""
);
#endif
