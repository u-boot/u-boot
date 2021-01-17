// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'sbi' command displays information about the SBI implementation.
 *
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>
#include <asm/sbi.h>

struct sbi_imp {
	const long id;
	const char *name;
};

struct sbi_ext {
	const u32 id;
	const char *name;
};

static struct sbi_imp implementations[] = {
	{ 0, "Berkeley Boot Loader (BBL)" },
	{ 1, "OpenSBI" },
	{ 2, "Xvisor" },
	{ 3, "KVM" },
	{ 4, "RustSBI" },
	{ 5, "Diosix" },
};

static struct sbi_ext extensions[] = {
	{ 0x00000000, "sbi_set_timer" },
	{ 0x00000001, "sbi_console_putchar" },
	{ 0x00000002, "sbi_console_getchar" },
	{ 0x00000003, "sbi_clear_ipi" },
	{ 0x00000004, "sbi_send_ipi" },
	{ 0x00000005, "sbi_remote_fence_i" },
	{ 0x00000006, "sbi_remote_sfence_vma" },
	{ 0x00000007, "sbi_remote_sfence_vma_asid" },
	{ 0x00000008, "sbi_shutdown" },
	{ 0x00000010, "SBI Base Functionality" },
	{ 0x54494D45, "Timer Extension" },
	{ 0x00735049, "IPI Extension" },
	{ 0x52464E43, "RFENCE Extension" },
	{ 0x0048534D, "Hart State Management Extension" },
	{ 0x53525354, "System Reset Extension" },
};

static int do_sbi(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	int i;
	long ret;

	ret = sbi_get_spec_version();
	if (ret >= 0)
		printf("SBI %ld.%ld\n", ret >> 24, ret & 0xffffff);
	ret = sbi_get_impl_id();
	if (ret >= 0) {
		for (i = 0; i < ARRAY_SIZE(implementations); ++i) {
			if (ret == implementations[i].id) {
				printf("%s\n", implementations[i].name);
				break;
			}
		}
		if (i == ARRAY_SIZE(implementations))
			printf("Unknown implementation ID %ld\n", ret);
	}
	printf("Extensions:\n");
	for (i = 0; i < ARRAY_SIZE(extensions); ++i) {
		ret = sbi_probe_extension(extensions[i].id);
		if (ret > 0)
			printf("  %s\n", extensions[i].name);
	}
	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char sbi_help_text[] =
	"- display SBI spec version, implementation, and available extensions";

#endif

U_BOOT_CMD_COMPLETE(
	sbi, 1, 0, do_sbi,
	"display SBI information",
	sbi_help_text, NULL
);
