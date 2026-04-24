// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Copyright 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 */

#include <bcb.h>
#include <command.h>
#include <env.h>
#include <fastboot.h>
#include <net.h>
#include <vsprintf.h>

/**
 * fastboot_buf_addr - base address of the fastboot download buffer
 */
void *fastboot_buf_addr;

/**
 * fastboot_buf_size - size of the fastboot download buffer
 */
u32 fastboot_buf_size;

/**
 * fastboot_progress_callback - callback executed during long operations
 */
void (*fastboot_progress_callback)(const char *msg);

/**
 * fastboot_response() - Writes a response of the form "$tag$reason".
 *
 * @tag: The first part of the response
 * @response: Pointer to fastboot response buffer
 * @format: printf style format string
 */
void fastboot_response(const char *tag, char *response,
		       const char *format, ...)
{
	va_list args;

	strlcpy(response, tag, FASTBOOT_RESPONSE_LEN);
	if (format) {
		va_start(args, format);
		vsnprintf(response + strlen(response),
			  FASTBOOT_RESPONSE_LEN - strlen(response) - 1,
			  format, args);
		va_end(args);
	}
}

/**
 * fastboot_fail() - Write a FAIL response of the form "FAIL$reason".
 *
 * @reason: Pointer to returned reason string
 * @response: Pointer to fastboot response buffer
 */
void fastboot_fail(const char *reason, char *response)
{
	fastboot_response("FAIL", response, "%s", reason);
}

/**
 * fastboot_okay() - Write an OKAY response of the form "OKAY$reason".
 *
 * @reason: Pointer to returned reason string, or NULL to send a bare "OKAY"
 * @response: Pointer to fastboot response buffer
 */
void fastboot_okay(const char *reason, char *response)
{
	if (reason)
		fastboot_response("OKAY", response, "%s", reason);
	else
		fastboot_response("OKAY", response, NULL);
}

/**
 * fastboot_set_reboot_flag() - Set flag to indicate reboot-bootloader
 *
 * Set flag which indicates that we should reboot into the bootloader
 * following the reboot that fastboot executes after this function.
 *
 * This function should be overridden in your board file with one
 * which sets whatever flag your board specific Android bootloader flow
 * requires in order to re-enter the bootloader.
 */
int __weak fastboot_set_reboot_flag(enum fastboot_reboot_reason reason)
{
	int ret;
	static const char * const boot_cmds[] = {
		[FASTBOOT_REBOOT_REASON_BOOTLOADER] = "bootonce-bootloader",
		[FASTBOOT_REBOOT_REASON_FASTBOOTD] = "boot-fastboot",
		[FASTBOOT_REBOOT_REASON_RECOVERY] = "boot-recovery"
	};

	int device = config_opt_enabled(CONFIG_FASTBOOT_FLASH_BLOCK,
					CONFIG_FASTBOOT_FLASH_BLOCK_DEVICE_ID, -1);
	if (device == -1) {
		device = config_opt_enabled(CONFIG_FASTBOOT_FLASH_MMC,
					    CONFIG_FASTBOOT_FLASH_MMC_DEV, -1);
	}
	const char *bcb_iface = config_opt_enabled(CONFIG_FASTBOOT_FLASH_BLOCK,
						   CONFIG_FASTBOOT_FLASH_BLOCK_INTERFACE_NAME,
						   "mmc");

	if (device == -1)
		return -EINVAL;

	if (reason >= FASTBOOT_REBOOT_REASONS_COUNT)
		return -EINVAL;

	ret = bcb_find_partition_and_load(bcb_iface, device, "misc");
	if (ret)
		goto out;

	ret = bcb_set(BCB_FIELD_COMMAND, boot_cmds[reason]);
	if (ret)
		goto out;

	ret = bcb_store();
out:
	bcb_reset();
	return ret;
}

/**
 * fastboot_get_progress_callback() - Return progress callback
 *
 * Return: Pointer to function called during long operations
 */
void (*fastboot_get_progress_callback(void))(const char *)
{
	return fastboot_progress_callback;
}

/**
 * fastboot_boot() - Execute fastboot boot command
 *
 * If ${fastboot_bootcmd} is set, run that command to execute the boot
 * process, if that returns, then exit the fastboot server and return
 * control to the caller.
 *
 * Otherwise execute "bootm <fastboot_buf_addr>", if that fails, reset
 * the board.
 */
void fastboot_boot(void)
{
	char *s;

	s = env_get("fastboot_bootcmd");
	if (s) {
		run_command(s, CMD_FLAG_ENV);
	} else if (IS_ENABLED(CONFIG_CMD_BOOTM)) {
		static char boot_addr_start[20];
		static char *const bootm_args[] = {
			"bootm", boot_addr_start, NULL
		};

		snprintf(boot_addr_start, sizeof(boot_addr_start) - 1,
			 "0x%p", fastboot_buf_addr);
		printf("Booting kernel at %s...\n\n\n", boot_addr_start);

		do_bootm(NULL, 0, 2, bootm_args);

		/*
		 * This only happens if image is somehow faulty so we start
		 * over. We deliberately leave this policy to the invocation
		 * of fastbootcmd if that's what's being run
		 */
		do_reset(NULL, 0, 0, NULL);
	}
}

/**
 * fastboot_handle_boot() - Shared implementation of system reaction to
 * fastboot commands
 *
 * Making desceisions about device boot state (stay in fastboot, reboot
 * to bootloader, reboot to OS, etc).
 */
void fastboot_handle_boot(int command, bool success)
{
	if (!success)
		return;

	switch (command) {
	case FASTBOOT_COMMAND_BOOT:
		fastboot_boot();
#if CONFIG_IS_ENABLED(NET)
		net_set_state(NETLOOP_SUCCESS);
#endif
		break;

	case FASTBOOT_COMMAND_CONTINUE:
#if CONFIG_IS_ENABLED(NET)
		net_set_state(NETLOOP_SUCCESS);
#endif
		break;

	case FASTBOOT_COMMAND_REBOOT:
	case FASTBOOT_COMMAND_REBOOT_BOOTLOADER:
	case FASTBOOT_COMMAND_REBOOT_FASTBOOTD:
	case FASTBOOT_COMMAND_REBOOT_RECOVERY:
		do_reset(NULL, 0, 0, NULL);
		break;
	}
}

/**
 * fastboot_set_progress_callback() - set progress callback
 *
 * @progress: Pointer to progress callback
 *
 * Set a callback which is invoked periodically during long running operations
 * (flash and erase). This can be used (for example) by the UDP transport to
 * send INFO responses to keep the client alive whilst those commands are
 * executing.
 */
void fastboot_set_progress_callback(void (*progress)(const char *msg))
{
	fastboot_progress_callback = progress;
}

/*
 * fastboot_init() - initialise new fastboot protocol session
 *
 * @buf_addr: Pointer to download buffer, or NULL for default
 * @buf_size: Size of download buffer, or zero for default
 */
void fastboot_init(void *buf_addr, u32 buf_size)
{
#if IS_ENABLED(CONFIG_FASTBOOT_FLASH_BLOCK)
	if (!strcmp(CONFIG_FASTBOOT_FLASH_BLOCK_INTERFACE_NAME, "mmc"))
		printf("Warning: the fastboot block backend features are limited, consider using the MMC backend\n");
#endif

	fastboot_buf_addr = buf_addr ? buf_addr :
				       (void *)CONFIG_FASTBOOT_BUF_ADDR;
	fastboot_buf_size = buf_size ? buf_size : CONFIG_FASTBOOT_BUF_SIZE;
	fastboot_set_progress_callback(NULL);

}

#if CONFIG_IS_ENABLED(EFI_PARTITION)
/**
 * fastboot_flash_gpt_partition_table() - Flash GPT partition table
 * @interface: Block interface name (e.g., "mmc", "scsi")
 * @device: Device number
 * @download_buffer: Buffer containing GPT data
 * @response: Fastboot response buffer
 */
void fastboot_flash_gpt_partition_table(const char *interface,
					int device,
					void *download_buffer,
					char *response)
{
	struct blk_desc *dev_desc;

	if (!interface || !strcmp(interface, "")) {
		fastboot_fail("block interface isn't provided", response);
		return;
	}

	dev_desc = blk_get_dev(interface, device);
	if (!dev_desc) {
		fastboot_fail("no such device", response);
		return;
	}

	printf("%s: updating MBR, Primary and Backup GPT(s) on %s device %d\n",
	       __func__, interface, dev_desc->devnum);

	if (is_valid_gpt_buf(dev_desc, download_buffer)) {
		printf("%s: invalid GPT - refusing to write to flash\n", __func__);
		fastboot_fail("invalid GPT partition", response);
		return;
	}

	if (write_mbr_and_gpt_partitions(dev_desc, download_buffer)) {
		printf("%s: writing GPT partitions failed\n", __func__);
		fastboot_fail("writing GPT partitions failed", response);
		return;
	}

	part_init(dev_desc);
	printf("........ success\n");
	fastboot_okay(NULL, response);
}
#endif

#if CONFIG_IS_ENABLED(DOS_PARTITION)
/**
 * fastboot_flash_mbr_partition_table() - Flash MBR partition table
 * @interface: Block interface name (e.g., "mmc", "scsi")
 * @device: Device number
 * @download_buffer: Buffer containing MBR data
 * @response: Fastboot response buffer
 */
void fastboot_flash_mbr_partition_table(const char *interface,
					int device,
					void *download_buffer,
					char *response)
{
	struct blk_desc *dev_desc;

	if (!interface || !strcmp(interface, "")) {
		fastboot_fail("block interface isn't provided", response);
		return;
	}

	dev_desc = blk_get_dev(interface, device);
	if (!dev_desc) {
		fastboot_fail("no such device", response);
		return;
	}

	printf("%s: updating MBR on %s device %d\n", __func__, interface,
	       dev_desc->devnum);

	if (is_valid_dos_buf(download_buffer)) {
		printf("%s: invalid MBR - refusing to write to flash\n", __func__);
		fastboot_fail("invalid MBR partition", response);
		return;
	}

	if (write_mbr_sector(dev_desc, download_buffer)) {
		printf("%s: writing MBR partition failed\n", __func__);
		fastboot_fail("writing MBR partition failed", response);
		return;
	}

	part_init(dev_desc);
	printf("........ success\n");
	fastboot_okay(NULL, response);
}
#endif
