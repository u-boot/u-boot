/*
 * Copyright (C) 2013 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <lcd.h>
#include <libtizen.h>
#include <samsung/misc.h>
#include <errno.h>
#include <version.h>
#include <linux/sizes.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <linux/input.h>
#include <power/pmic.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
void set_board_info(void)
{
	char info[64];

	snprintf(info, ARRAY_SIZE(info), "%d.%d", s5p_cpu_rev & 0x0f,
		 (s5p_cpu_rev & 0xf0) >> 0x04);
	setenv("soc_rev", info);

	snprintf(info, ARRAY_SIZE(info), "%x", s5p_cpu_id);
	setenv("soc_id", info);

#ifdef CONFIG_REVISION_TAG
	snprintf(info, ARRAY_SIZE(info), "%x", get_board_rev());
	setenv("board_rev", info);
#endif
#ifdef CONFIG_OF_LIBFDT
	snprintf(info, ARRAY_SIZE(info),  "%s%x-%s.dtb",
		 CONFIG_SYS_SOC, s5p_cpu_id, CONFIG_SYS_BOARD);
	setenv("fdtfile", info);
#endif
}
#endif /* CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG */

#ifdef CONFIG_LCD_MENU
static int power_key_pressed(u32 reg)
{
	struct pmic *pmic;
	u32 status;
	u32 mask;

	pmic = pmic_get(KEY_PWR_PMIC_NAME);
	if (!pmic) {
		printf("%s: Not found\n", KEY_PWR_PMIC_NAME);
		return 0;
	}

	if (pmic_probe(pmic))
		return 0;

	if (reg == KEY_PWR_STATUS_REG)
		mask = KEY_PWR_STATUS_MASK;
	else
		mask = KEY_PWR_INTERRUPT_MASK;

	if (pmic_reg_read(pmic, reg, &status))
		return 0;

	return !!(status & mask);
}

static int key_pressed(int key)
{
	int value;

	switch (key) {
	case KEY_POWER:
		value = power_key_pressed(KEY_PWR_INTERRUPT_REG);
		break;
	case KEY_VOLUMEUP:
		value = !gpio_get_value(KEY_VOL_UP_GPIO);
		break;
	case KEY_VOLUMEDOWN:
		value = !gpio_get_value(KEY_VOL_DOWN_GPIO);
		break;
	default:
		value = 0;
		break;
	}

	return value;
}

static int check_keys(void)
{
	int keys = 0;

	if (key_pressed(KEY_POWER))
		keys += KEY_POWER;
	if (key_pressed(KEY_VOLUMEUP))
		keys += KEY_VOLUMEUP;
	if (key_pressed(KEY_VOLUMEDOWN))
		keys += KEY_VOLUMEDOWN;

	return keys;
}

/*
 * 0 BOOT_MODE_INFO
 * 1 BOOT_MODE_THOR
 * 2 BOOT_MODE_UMS
 * 3 BOOT_MODE_DFU
 * 4 BOOT_MODE_EXIT
 */
static char *
mode_name[BOOT_MODE_EXIT + 1] = {
	"DEVICE",
	"THOR",
	"UMS",
	"DFU",
	"EXIT"
};

static char *
mode_info[BOOT_MODE_EXIT + 1] = {
	"info",
	"downloader",
	"mass storage",
	"firmware update",
	"and run normal boot"
};

#define MODE_CMD_ARGC	4

static char *
mode_cmd[BOOT_MODE_EXIT + 1][MODE_CMD_ARGC] = {
	{"", "", "", ""},
	{"thor", "0", "mmc", "0"},
	{"ums", "0", "mmc", "0"},
	{"dfu", "0", "mmc", "0"},
	{"", "", "", ""},
};

static void display_board_info(void)
{
#ifdef CONFIG_GENERIC_MMC
	struct mmc *mmc = find_mmc_device(0);
#endif
	vidinfo_t *vid = &panel_info;

	lcd_position_cursor(4, 4);

	lcd_printf("%s\n\t", U_BOOT_VERSION);
	lcd_puts("\n\t\tBoard Info:\n");
#ifdef CONFIG_SYS_BOARD
	lcd_printf("\tBoard name: %s\n", CONFIG_SYS_BOARD);
#endif
#ifdef CONFIG_REVISION_TAG
	lcd_printf("\tBoard rev: %u\n", get_board_rev());
#endif
	lcd_printf("\tDRAM banks: %u\n", CONFIG_NR_DRAM_BANKS);
	lcd_printf("\tDRAM size: %u MB\n", gd->ram_size / SZ_1M);

#ifdef CONFIG_GENERIC_MMC
	if (mmc) {
		if (!mmc->capacity)
			mmc_init(mmc);

		lcd_printf("\teMMC size: %llu MB\n", mmc->capacity / SZ_1M);
	}
#endif
	if (vid)
		lcd_printf("\tDisplay resolution: %u x % u\n",
			   vid->vl_col, vid->vl_row);

	lcd_printf("\tDisplay BPP: %u\n", 1 << vid->vl_bpix);
}

static int mode_leave_menu(int mode)
{
	char *exit_option;
	char *exit_boot = "boot";
	char *exit_back = "back";
	cmd_tbl_t *cmd;
	int cmd_result;
	int cmd_repeatable;
	int leave;

	lcd_clear();

	switch (mode) {
	case BOOT_MODE_EXIT:
		return 1;
	case BOOT_MODE_INFO:
		display_board_info();
		exit_option = exit_back;
		leave = 0;
		break;
	default:
		cmd = find_cmd(mode_cmd[mode][0]);
		if (cmd) {
			printf("Enter: %s %s\n", mode_name[mode],
						 mode_info[mode]);
			lcd_printf("\n\n\t%s %s\n", mode_name[mode],
						    mode_info[mode]);
			lcd_puts("\n\tDo not turn off device before finish!\n");

			cmd_result = cmd_process(0, MODE_CMD_ARGC,
						 *(mode_cmd + mode),
						 &cmd_repeatable, NULL);

			if (cmd_result == CMD_RET_SUCCESS) {
				printf("Command finished\n");
				lcd_clear();
				lcd_printf("\n\n\t%s finished\n",
					   mode_name[mode]);

				exit_option = exit_boot;
				leave = 1;
			} else {
				printf("Command error\n");
				lcd_clear();
				lcd_printf("\n\n\t%s command error\n",
					   mode_name[mode]);

				exit_option = exit_back;
				leave = 0;
			}
		} else {
			lcd_puts("\n\n\tThis mode is not supported.\n");
			exit_option = exit_back;
			leave = 0;
		}
	}

	lcd_printf("\n\n\tPress POWER KEY to %s\n", exit_option);

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	/* Wait for PWR key */
	while (!key_pressed(KEY_POWER))
		mdelay(1);

	lcd_clear();
	return leave;
}

static void display_download_menu(int mode)
{
	char *selection[BOOT_MODE_EXIT + 1];
	int i;

	for (i = 0; i <= BOOT_MODE_EXIT; i++)
		selection[i] = "[  ]";

	selection[mode] = "[=>]";

	lcd_clear();
	lcd_printf("\n\t\tDownload Mode Menu\n");

	for (i = 0; i <= BOOT_MODE_EXIT; i++)
		lcd_printf("\t%s  %s - %s\n\n", selection[i],
						mode_name[i],
						mode_info[i]);
}

static void download_menu(void)
{
	int mode = 0;
	int last_mode = 0;
	int run;
	int key;

	display_download_menu(mode);

	while (1) {
		run = 0;

		if (mode != last_mode)
			display_download_menu(mode);

		last_mode = mode;
		mdelay(100);

		key = check_keys();
		switch (key) {
		case KEY_POWER:
			run = 1;
			break;
		case KEY_VOLUMEUP:
			if (mode > 0)
				mode--;
			break;
		case KEY_VOLUMEDOWN:
			if (mode < BOOT_MODE_EXIT)
				mode++;
			break;
		default:
			break;
		}

		if (run) {
			if (mode_leave_menu(mode))
				break;

			display_download_menu(mode);
		}
	}

	lcd_clear();
}

static void display_mode_info(void)
{
	lcd_position_cursor(4, 4);
	lcd_printf("%s\n", U_BOOT_VERSION);
	lcd_puts("\nDownload Mode Menu\n");
#ifdef CONFIG_SYS_BOARD
	lcd_printf("Board name: %s\n", CONFIG_SYS_BOARD);
#endif
	lcd_printf("Press POWER KEY to display MENU options.");
}

static int boot_menu(void)
{
	int key = 0;
	int timeout = 10;

	display_mode_info();

	while (timeout--) {
		lcd_printf("\rNormal boot will start in: %d seconds.", timeout);
		mdelay(1000);

		key = key_pressed(KEY_POWER);
		if (key)
			break;
	}

	lcd_clear();

	/* If PWR pressed - show download menu */
	if (key) {
		printf("Power pressed - go to download menu\n");
		download_menu();
		printf("Download mode exit.\n");
	}

	return 0;
}

void check_boot_mode(void)
{
	int pwr_key;

	pwr_key = power_key_pressed(KEY_PWR_STATUS_REG);
	if (!pwr_key)
		return;

	/* Clear PWR button Rising edge interrupt status flag */
	power_key_pressed(KEY_PWR_INTERRUPT_REG);

	if (key_pressed(KEY_VOLUMEUP))
		boot_menu();
	else if (key_pressed(KEY_VOLUMEDOWN))
		mode_leave_menu(BOOT_MODE_THOR);
}

void keys_init(void)
{
	/* Set direction to input */
	gpio_direction_input(KEY_VOL_UP_GPIO);
	gpio_direction_input(KEY_VOL_DOWN_GPIO);
}
#endif /* CONFIG_LCD_MENU */

#ifdef CONFIG_CMD_BMP
void draw_logo(void)
{
	int x, y;
	ulong addr;

	addr = panel_info.logo_addr;
	if (!addr) {
		error("There is no logo data.");
		return;
	}

	if (panel_info.vl_width >= panel_info.logo_width) {
		x = ((panel_info.vl_width - panel_info.logo_width) >> 1);
		x += panel_info.logo_x_offset; /* For X center align */
	} else {
		x = 0;
		printf("Warning: image width is bigger than display width\n");
	}

	if (panel_info.vl_height >= panel_info.logo_height) {
		y = ((panel_info.vl_height - panel_info.logo_height) >> 1);
		y += panel_info.logo_y_offset; /* For Y center align */
	} else {
		y = 0;
		printf("Warning: image height is bigger than display height\n");
	}

	bmp_display(addr, x, y);
}
#endif /* CONFIG_CMD_BMP */
