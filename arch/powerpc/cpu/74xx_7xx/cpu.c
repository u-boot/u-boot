/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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
 * cpu.c
 *
 * CPU specific code
 *
 * written or collected and sometimes rewritten by
 * Magnus Damm <damm@bitsmart.com>
 *
 * minor modifications by
 * Wolfgang Denk <wd@denx.de>
 *
 * more modifications by
 * Josh Huber <huber@mclx.com>
 * added support for the 74xx series of cpus
 * added support for the 7xx series of cpus
 * made the code a little less hard-coded, and more auto-detectish
 */

#include <common.h>
#include <command.h>
#include <74xx_7xx.h>
#include <asm/cache.h>

#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

cpu_t
get_cpu_type(void)
{
	uint pvr = get_pvr();
	cpu_t type;

	type = CPU_UNKNOWN;

	switch (PVR_VER(pvr)) {
	case 0x000c:
		type = CPU_7400;
		break;
	case 0x0008:
		type = CPU_750;

		if (((pvr >> 8) & 0xff) == 0x01) {
			type = CPU_750CX;	/* old CX (80100 and 8010x?)*/
		} else if (((pvr >> 8) & 0xff) == 0x22) {
			type = CPU_750CX;	/* CX (82201,82202) and CXe (82214) */
		} else if (((pvr >> 8) & 0xff) == 0x33) {
			type = CPU_750CX;	/* CXe (83311) */
		} else if (((pvr >> 12) & 0xF) == 0x3) {
			type = CPU_755;
		}
		break;

	case 0x7000:
		type = CPU_750FX;
		break;

	case 0x7002:
		type = CPU_750GX;
		break;

	case 0x800C:
		type = CPU_7410;
		break;

	case 0x8000:
		type = CPU_7450;
		break;

	case 0x8001:
		type = CPU_7455;
		break;

	case 0x8002:
		type = CPU_7457;
		break;

	case 0x8003:
		type = CPU_7447A;
		break;

	case 0x8004:
		type = CPU_7448;
		break;

	default:
		break;
	}

	return type;
}

/* ------------------------------------------------------------------------- */

#if !defined(CONFIG_BAB7xx)
int checkcpu (void)
{
	uint type   = get_cpu_type();
	uint pvr    = get_pvr();
	ulong clock = gd->cpu_clk;
	char buf[32];
	char *str;

	puts ("CPU:   ");

	switch (type) {
	case CPU_750CX:
		printf ("750CX%s v%d.%d", (pvr&0xf0)?"e":"",
			(pvr>>8) & 0xf,
			pvr & 0xf);
		goto	PR_CLK;

	case CPU_750:
		str = "750";
		break;

	case CPU_750FX:
		str = "750FX";
		break;

	case CPU_750GX:
		str = "750GX";
		break;

	case CPU_755:
		str = "755";
		break;

	case CPU_7400:
		str = "MPC7400";
		break;

	case CPU_7410:
		str = "MPC7410";
		break;

	case CPU_7447A:
		str = "MPC7447A";
		break;

	case CPU_7448:
		str = "MPC7448";
		break;

	case CPU_7450:
		str = "MPC7450";
		break;

	case CPU_7455:
		str = "MPC7455";
		break;

	case CPU_7457:
		str = "MPC7457";
		break;

	default:
		printf("Unknown CPU -- PVR: 0x%08x\n", pvr);
		return -1;
	}

	printf ("%s v%d.%d", str, (pvr >> 8) & 0xFF, pvr & 0xFF);
PR_CLK:
	printf (" @ %s MHz\n", strmhz(buf, clock));

	return (0);
}
#endif
/* these two functions are unimplemented currently [josh] */

/* -------------------------------------------------------------------- */
/* L1 i-cache								*/

int
checkicache(void)
{
	return 0; /* XXX */
}

/* -------------------------------------------------------------------- */
/* L1 d-cache								*/

int
checkdcache(void)
{
	return 0; /* XXX */
}

/* -------------------------------------------------------------------- */

static inline void
soft_restart(unsigned long addr)
{
	/* SRR0 has system reset vector, SRR1 has default MSR value */
	/* rfi restores MSR from SRR1 and sets the PC to the SRR0 value */

	__asm__ __volatile__ ("mtspr	26, %0"		:: "r" (addr));
	__asm__ __volatile__ ("li	4, (1 << 6)"	::: "r4");
	__asm__ __volatile__ ("mtspr	27, 4");
	__asm__ __volatile__ ("rfi");

	while(1);	/* not reached */
}


#if !defined(CONFIG_PCIPPC2) && \
    !defined(CONFIG_BAB7xx)  && \
    !defined(CONFIG_ELPPC)   && \
    !defined(CONFIG_PPMC7XX)
/* no generic way to do board reset. simply call soft_reset. */
void
do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;
	/* flush and disable I/D cache */
	__asm__ __volatile__ ("mfspr	3, 1008"	::: "r3");
	__asm__ __volatile__ ("ori	5, 5, 0xcc00"	::: "r5");
	__asm__ __volatile__ ("ori	4, 3, 0xc00"	::: "r4");
	__asm__ __volatile__ ("andc	5, 3, 5"	::: "r5");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 4");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("mtspr	1008, 5");
	__asm__ __volatile__ ("isync");
	__asm__ __volatile__ ("sync");

#ifdef CONFIG_SYS_RESET_ADDRESS
	addr = CONFIG_SYS_RESET_ADDRESS;
#else
	/*
	 * note: when CONFIG_SYS_MONITOR_BASE points to a RAM address,
	 * CONFIG_SYS_MONITOR_BASE - sizeof (ulong) is usually a valid
	 * address. Better pick an address known to be invalid on your
	 * system and assign it to CONFIG_SYS_RESET_ADDRESS.
	 */
	addr = CONFIG_SYS_MONITOR_BASE - sizeof (ulong);
#endif
	soft_restart(addr);
	while(1);	/* not reached */
}
#endif

/* ------------------------------------------------------------------------- */

/*
 * For the 7400 the TB clock runs at 1/4 the cpu bus speed.
 */
#ifndef CONFIG_SYS_BUS_CLK
#define	CONFIG_SYS_BUS_CLK gd->bus_clk
#endif

unsigned long get_tbclk(void)
{
	return CONFIG_SYS_BUS_CLK / 4;
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_WATCHDOG)
#if !defined(CONFIG_PCIPPC2) && !defined(CONFIG_BAB7xx)
void
watchdog_reset(void)
{

}
#endif  /* !CONFIG_PCIPPC2 && !CONFIG_BAB7xx */
#endif	/* CONFIG_WATCHDOG */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_OF_LIBFDT
void ft_cpu_setup(void *blob, bd_t *bd)
{
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "timebase-frequency", bd->bi_busfreq / 4, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "bus-frequency", bd->bi_busfreq, 1);
	do_fixup_by_prop_u32(blob, "device_type", "cpu", 4,
			     "clock-frequency", bd->bi_intfreq, 1);

	fdt_fixup_memory(blob, (u64)bd->bi_memstart, (u64)bd->bi_memsize);

	fdt_fixup_ethernet(blob);
}
#endif
/* ------------------------------------------------------------------------- */
