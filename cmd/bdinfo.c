// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <env.h>
#include <net.h>
#include <vsprintf.h>
#include <asm/cache.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

void bdinfo_print_num(const char *name, ulong value)
{
	printf("%-12s= 0x%0*lx\n", name, 2 * (int)sizeof(value), value);
}

static void print_eth(int idx)
{
	char name[10], *val;
	if (idx)
		sprintf(name, "eth%iaddr", idx);
	else
		strcpy(name, "ethaddr");
	val = env_get(name);
	if (!val)
		val = "(not set)";
	printf("%-12s= %s\n", name, val);
}

static void print_lnum(const char *name, unsigned long long value)
{
	printf("%-12s= 0x%.8llX\n", name, value);
}

void bdinfo_print_mhz(const char *name, unsigned long hz)
{
	char buf[32];

	printf("%-12s= %6s MHz\n", name, strmhz(buf, hz));
}

static void print_bi_dram(const bd_t *bd)
{
#ifdef CONFIG_NR_DRAM_BANKS
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			bdinfo_print_num("DRAM bank",	i);
			bdinfo_print_num("-> start",	bd->bi_dram[i].start);
			bdinfo_print_num("-> size",	bd->bi_dram[i].size);
		}
	}
#endif
}

void __weak board_detail(void)
{
	/* Please define board_detail() for your PPC platform */
}

__weak void arch_print_bdinfo(void)
{
}

int do_bdinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	bd_t *bd = gd->bd;

#ifdef DEBUG
	bdinfo_print_num("bd address", (ulong)bd);
#endif
	bdinfo_print_num("boot_params", (ulong)bd->bi_boot_params);
	print_bi_dram(bd);
	bdinfo_print_num("memstart", (ulong)bd->bi_memstart);
	print_lnum("memsize", (u64)bd->bi_memsize);
	bdinfo_print_num("flashstart", (ulong)bd->bi_flashstart);
	bdinfo_print_num("flashsize", (ulong)bd->bi_flashsize);
	bdinfo_print_num("flashoffset", (ulong)bd->bi_flashoffset);
	printf("baudrate    = %u bps\n", gd->baudrate);
	bdinfo_print_num("relocaddr", gd->relocaddr);
	bdinfo_print_num("reloc off", gd->reloc_off);
	printf("%-12s= %u-bit\n", "Build", (uint)sizeof(void *) * 8);
	if (IS_ENABLED(CONFIG_CMD_NET)) {
		printf("current eth = %s\n", eth_get_name());
		print_eth(0);
		printf("IP addr     = %s\n", env_get("ipaddr"));
	}
	bdinfo_print_num("fdt_blob", (ulong)gd->fdt_blob);
	bdinfo_print_num("new_fdt", (ulong)gd->new_fdt);
	bdinfo_print_num("fdt_size", (ulong)gd->fdt_size);
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO) || defined(CONFIG_DM_VIDEO)
	bdinfo_print_num("FB base  ", gd->fb_base);
#endif
#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
	bdinfo_print_num("multi_dtb_fit", (ulong)gd->multi_dtb_fit);
#endif

	arch_print_bdinfo();

	/* This section is used only by ppc */
#if defined(CONFIG_MPC8xx) || defined(CONFIG_E500)
	bdinfo_print_num("immr_base", bd->bi_immr_base);
#endif
	if (IS_ENABLED(CONFIG_PPC)) {
		bdinfo_print_num("bootflags", bd->bi_bootflags);
		bdinfo_print_mhz("intfreq", bd->bi_intfreq);
#ifdef CONFIG_ENABLE_36BIT_PHYS
		if (IS_ENABLED(CONFIG_PHYS_64BIT))
			puts("addressing  = 36-bit\n");
		else
			puts("addressing  = 32-bit\n");
#endif
		board_detail();
	}
#if defined(CONFIG_CPM2)
	bdinfo_print_mhz("cpmfreq", bd->bi_cpmfreq);
	bdinfo_print_mhz("vco", bd->bi_vco);
	bdinfo_print_mhz("sccfreq", bd->bi_sccfreq);
	bdinfo_print_mhz("brgfreq", bd->bi_brgfreq);
#endif

	/* This is used by m68k and ppc */
#if defined(CONFIG_SYS_INIT_RAM_ADDR)
	bdinfo_print_num("sramstart", (ulong)bd->bi_sramstart);
	bdinfo_print_num("sramsize", (ulong)bd->bi_sramsize);
#endif
	if (IS_ENABLED(CONFIG_PPC) || IS_ENABLED(CONFIG_M68K))
		bdinfo_print_mhz("busfreq", bd->bi_busfreq);

	/* The rest are used only by m68k */
#ifdef CONFIG_M68K
#if defined(CONFIG_SYS_MBAR)
	bdinfo_print_num("mbar", bd->bi_mbar_base);
#endif
	bdinfo_print_mhz("cpufreq", bd->bi_intfreq);
	if (IS_ENABLED(CONFIG_PCI))
		bdinfo_print_mhz("pcifreq", bd->bi_pcifreq);
#ifdef CONFIG_EXTRA_CLOCK
	bdinfo_print_mhz("flbfreq", bd->bi_flbfreq);
	bdinfo_print_mhz("inpfreq", bd->bi_inpfreq);
	bdinfo_print_mhz("vcofreq", bd->bi_vcofreq);
#endif
#endif

	return 0;
}

/* -------------------------------------------------------------------- */

U_BOOT_CMD(
	bdinfo,	1,	1,	do_bdinfo,
	"print Board Info structure",
	""
);
