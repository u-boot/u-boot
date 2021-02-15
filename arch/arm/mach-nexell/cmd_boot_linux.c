// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 nexell
 * jhkim <jhkim@nexell.co.kr>
 */

#include <common.h>
#include <bootm.h>
#include <command.h>
#include <environment.h>
#include <errno.h>
#include <image.h>
#include <fdt_support.h>
#include <asm/global_data.h>

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_CLI_FRAMEWORK)

DECLARE_GLOBAL_DATA_PTR;

static bootm_headers_t linux_images;

static void boot_go_set_os(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[],
			   bootm_headers_t *images)
{
	char * const img_addr = argv[0];

	images->os.type = IH_TYPE_KERNEL;
	images->os.comp = IH_COMP_NONE;
	images->os.os = IH_OS_LINUX;
	images->os.load = simple_strtoul(img_addr, NULL, 16);
	images->ep = images->os.load;
#if defined(CONFIG_ARM)
	images->os.arch = IH_ARCH_ARM;
#elif defined(CONFIG_ARM64)
	images->os.arch = IH_ARCH_ARM64;
#else
	#error "Not support architecture ..."
#endif
	if (!IS_ENABLED(CONFIG_OF_LIBFDT) && !IS_ENABLED(CONFIG_SPL_BUILD)) {
		/* set DTB address for linux kernel */
		if (argc > 2) {
			unsigned long ft_addr;

			ft_addr = simple_strtol(argv[2], NULL, 16);
			images->ft_addr = (char *)ft_addr;

			/*
			 * if not defined IMAGE_ENABLE_OF_LIBFDT,
			 * must be set to fdt address
			 */
			if (!IMAGE_ENABLE_OF_LIBFDT)
				gd->bd->bi_boot_params = ft_addr;

			debug("## set ft:%08lx and boot params:%08lx [control of:%s]"
			      "...\n", ft_addr, gd->bd->bi_boot_params,
			      IMAGE_ENABLE_OF_LIBFDT ? "on" : "off");
		}
	}
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_LMB)
static void boot_start_lmb(bootm_headers_t *images)
{
	ulong		mem_start;
	phys_size_t	mem_size;

	lmb_init(&images->lmb);

	mem_start = getenv_bootm_low();
	mem_size = getenv_bootm_size();

	lmb_add(&images->lmb, (phys_addr_t)mem_start, mem_size);

	arch_lmb_reserve(&images->lmb);
	board_lmb_reserve(&images->lmb);
}
#else
#define lmb_reserve(lmb, base, size)
static inline void boot_start_lmb(bootm_headers_t *images) { }
#endif

int do_boot_linux(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	boot_os_fn *boot_fn;
	bootm_headers_t *images = &linux_images;
	int flags;
	int ret;

	boot_start_lmb(images);

	flags  = BOOTM_STATE_START;

	argc--; argv++;
	boot_go_set_os(cmdtp, flag, argc, argv, images);

	if (IS_ENABLED(CONFIG_OF_LIBFDT)) {
		/* find flattened device tree */
		ret = boot_get_fdt(flag, argc, argv, IH_ARCH_DEFAULT, images,
				   &images->ft_addr, &images->ft_len);
		if (ret) {
			puts("Could not find a valid device tree\n");
			return 1;
		}
		set_working_fdt_addr((ulong)images->ft_addr);
	}

	if (!IS_ENABLED(CONFIG_OF_LIBFDT))
		flags |= BOOTM_STATE_OS_GO;

	boot_fn = do_bootm_linux;
	ret = boot_fn(flags, argc, argv, images);

	if (ret == BOOTM_ERR_UNIMPLEMENTED)
		show_boot_progress(BOOTSTAGE_ID_DECOMP_UNIMPL);
	else if (ret == BOOTM_ERR_RESET)
		do_reset(cmdtp, flag, argc, argv);

	return ret;
}

U_BOOT_CMD(bootl, CONFIG_SYS_MAXARGS, 1, do_boot_linux,
	   "boot linux image from memory",
	   "[addr [arg ...]]\n    - boot linux image stored in memory\n"
	   "\tuse a '-' for the DTB address\n"
);
#endif

#if defined(CONFIG_CMD_BOOTD) && !defined(CONFIG_CMD_BOOTM)
int do_bootd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return run_command(env_get("bootcmd"), flag);
}

U_BOOT_CMD(boot, 1, 1, do_bootd,
	   "boot default, i.e., run 'bootcmd'",
	   ""
);

/* keep old command name "bootd" for backward compatibility */
U_BOOT_CMD(bootd, 1,	1,	do_bootd,
	   "boot default, i.e., run 'bootcmd'",
	   ""
);
#endif
