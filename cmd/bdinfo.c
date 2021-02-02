// SPDX-License-Identifier: GPL-2.0+
/*
 * Implements the 'bd' command to show board information
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <env.h>
#include <lmb.h>
#include <net.h>
#include <video.h>
#include <vsprintf.h>
#include <asm/cache.h>

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

static void print_phys_addr(const char *name, phys_addr_t value)
{
	printf("%-12s= 0x%.*llx\n", name, 2 * (int)sizeof(ulong),
	       (unsigned long long)value);
}

void bdinfo_print_mhz(const char *name, unsigned long hz)
{
	char buf[32];

	printf("%-12s= %6s MHz\n", name, strmhz(buf, hz));
}

static void print_bi_dram(const struct bd_info *bd)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			bdinfo_print_num("DRAM bank",	i);
			bdinfo_print_num("-> start",	bd->bi_dram[i].start);
			bdinfo_print_num("-> size",	bd->bi_dram[i].size);
		}
	}
}

__weak void arch_print_bdinfo(void)
{
}

static void show_video_info(void)
{
	const struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_VIDEO, dev, uc) {
		printf("%-12s= %s %sactive\n", "Video", dev->name,
		       device_active(dev) ? "" : "in");
		if (device_active(dev)) {
			struct video_priv *upriv = dev_get_uclass_priv(dev);

			print_phys_addr("FB base", (ulong)upriv->fb);
			if (upriv->copy_fb)
				print_phys_addr("FB copy", (ulong)upriv->copy_fb);
			printf("%-12s= %dx%dx%d\n", "FB size", upriv->xsize,
			       upriv->ysize, 1 << upriv->bpix);
		}
	}
}

int do_bdinfo(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct bd_info *bd = gd->bd;

#ifdef DEBUG
	bdinfo_print_num("bd address", (ulong)bd);
#endif
	bdinfo_print_num("boot_params", (ulong)bd->bi_boot_params);
	print_bi_dram(bd);
	if (IS_ENABLED(CONFIG_SYS_HAS_SRAM)) {
		bdinfo_print_num("sramstart", (ulong)bd->bi_sramstart);
		bdinfo_print_num("sramsize", (ulong)bd->bi_sramsize);
	}
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
	if (IS_ENABLED(CONFIG_DM_VIDEO))
		show_video_info();
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	bdinfo_print_num("FB base  ", gd->fb_base);
#endif
#if CONFIG_IS_ENABLED(MULTI_DTB_FIT)
	bdinfo_print_num("multi_dtb_fit", (ulong)gd->multi_dtb_fit);
#endif
	if (gd->fdt_blob) {
		struct lmb lmb;

		lmb_init_and_reserve(&lmb, gd->bd, (void *)gd->fdt_blob);
		lmb_dump_all_force(&lmb);
	}

	arch_print_bdinfo();

	return 0;
}

U_BOOT_CMD(
	bdinfo,	1,	1,	do_bdinfo,
	"print Board Info structure",
	""
);
