// SPDX-License-Identifier: GPL-2.0+
/*
 * Altera SoCFPGA common board code
 *
 * Copyright (C) 2015 Marek Vasut <marex@denx.de>
 */

#include <config.h>
#include <errno.h>
#include <env.h>
#include <fdtdec.h>
#include <log.h>
#include <init.h>
#include <hang.h>
#include <handoff.h>
#include <image.h>
#include <usb.h>
#include <usb/dwc2_udc.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/secure_vab.h>
#include <asm/arch/smc_api.h>
#include <bloblist.h>

DECLARE_GLOBAL_DATA_PTR;

#define DEFAULT_JTAG_USERCODE 0xFFFFFFFF

void s_init(void) {
#ifndef CONFIG_ARM64
	/*
	 * Preconfigure ACTLR and CPACR, make sure Write Full Line of Zeroes
	 * is disabled in ACTLR.
	 * This is optional on CycloneV / ArriaV.
	 * This is mandatory on Arria10, otherwise Linux refuses to boot.
	 */
	asm volatile(
		"mcr p15, 0, %0, c1, c0, 1\n"
		"mcr p15, 0, %0, c1, c0, 2\n"
		"isb\n"
		"dsb\n"
	::"r"(0x0));
#endif
}

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	/* Address of boot parameters for ATAG (if ATAG is used) */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init_banksize(void)
{
#if CONFIG_IS_ENABLED(HANDOFF) && IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)
#ifndef CONFIG_SPL_BUILD
	struct spl_handoff *ho;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, sizeof(*ho));
	if (!ho)
		return log_msg_ret("Missing SPL hand-off info", -ENOENT);
	handoff_load_dram_banks(ho);
#endif
#else
	fdtdec_setup_memory_banksize();
#endif /* HANDOFF && CONFIG_TARGET_SOCFPGA_AGILEX5 */

	return 0;
}

#ifdef CONFIG_USB_GADGET
struct dwc2_plat_otg_data socfpga_otg_data = {
	.usb_gusbcfg	= 0x1417,
};

int board_usb_init(int index, enum usb_init_type init)
{
	int node[2], count;
	fdt_addr_t addr;

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "udc",
					   COMPAT_ALTERA_SOCFPGA_DWC2USB,
					   node, 2);
	if (count <= 0)	/* No controller found. */
		return 0;

	addr = fdtdec_get_addr(gd->fdt_blob, node[0], "reg");
	if (addr == FDT_ADDR_T_NONE) {
		printf("UDC Controller has no 'reg' property!\n");
		return -EINVAL;
	}

	/* Patch the address from OF into the controller pdata. */
	socfpga_otg_data.regs_otg = addr;

	return dwc2_udc_probe(&socfpga_otg_data);
}

int g_dnl_board_usb_cable_connected(void)
{
	return 1;
}
#endif

u8 socfpga_get_board_id(void)
{
	u8 board_id = 0;
	u32 jtag_usercode;
	int err;

#if !IS_ENABLED(CONFIG_XPL_BUILD) && IS_ENABLED(CONFIG_SPL_ATF)
	err = smc_get_usercode(&jtag_usercode);
#else
	u32 resp_len = 1;

	err = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_GET_USERCODE, MBOX_CMD_DIRECT, 0,
			    NULL, 0, &resp_len, &jtag_usercode);
#endif

	if (err) {
		puts("Fail to read JTAG Usercode. Default Board ID to 0\n");
		return board_id;
	}

	debug("Valid JTAG Usercode: %u\n", jtag_usercode);

	if (jtag_usercode == DEFAULT_JTAG_USERCODE) {
		debug("JTAG Usercode is not set. Default Board ID to 0\n");
	} else if (jtag_usercode <= 255) {
		board_id = jtag_usercode;
		debug("Valid JTAG Usercode. Set Board ID to %u\n", board_id);
	} else {
		puts("Board ID is not in range 0 to 255\n");
	}

	return board_id;
}

#if IS_ENABLED(CONFIG_XPL_BUILD) && IS_ENABLED(CONFIG_TARGET_SOCFPGA_SOC64)
int board_fit_config_name_match(const char *name)
{
	char board_name[10];

	sprintf(board_name, "board_%u", socfpga_get_board_id());

	debug("Board name: %s\n", board_name);

	return strcmp(name, board_name);
}
#endif

#if IS_ENABLED(CONFIG_FIT_IMAGE_POST_PROCESS)
void board_fit_image_post_process(const void *fit, int node, void **p_image,
				  size_t *p_size)
{
	if (IS_ENABLED(CONFIG_SOCFPGA_SECURE_VAB_AUTH)) {
		if (socfpga_vendor_authentication(p_image, p_size))
			hang();
	}
}
#endif

#if !IS_ENABLED(CONFIG_XPL_BUILD) && IS_ENABLED(CONFIG_FIT)
void board_prep_linux(struct bootm_headers *images)
{
	bool use_fit = false;

	if (!images->fit_uname_cfg) {
		if (IS_ENABLED(CONFIG_SOCFPGA_SECURE_VAB_AUTH) &&
		    !IS_ENABLED(CONFIG_SOCFPGA_SECURE_VAB_AUTH_ALLOW_NON_FIT_IMAGE)) {
			/*
			 * Ensure the OS is always booted from FIT and with
			 * VAB signed certificate
			 */
			printf("Please use FIT with VAB signed images!\n");
			hang();
		}
	} else {
		use_fit = true;
		/* Update fdt_addr in enviroment variable */
		env_set_hex("fdt_addr", (ulong)images->ft_addr);
		debug("images->ft_addr = 0x%08lx\n", (ulong)images->ft_addr);
	}

	if (use_fit && IS_ENABLED(CONFIG_CADENCE_QSPI)) {
		if (env_get("linux_qspi_enable"))
			run_command(env_get("linux_qspi_enable"), 0);
	}
}
#endif

#if CONFIG_IS_ENABLED(LMB_ARCH_MEM_MAP)
void lmb_arch_add_memory(void)
{
	int i;
	struct bd_info *bd = gd->bd;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		if (bd->bi_dram[i].size)
			lmb_add(bd->bi_dram[i].start, bd->bi_dram[i].size);
	}
}
#endif
