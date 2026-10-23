// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2016 The Android Open Source Project
 */

#include <command.h>
#include <console.h>
#include <dm.h>
#include <env.h>
#include <fastboot.h>
#include <fastboot-internal.h>
#include <fb_block.h>
#include <fb_mmc.h>
#include <fb_nand.h>
#include <fb_spi_flash.h>
#include <memalign.h>
#include <part.h>
#include <stdlib.h>
#include <video_console.h>
#include <vsprintf.h>
#include <linux/printk.h>
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_FLASHING)
#include <avb_verify.h>
#include <blk.h>
#include <u-boot/sha256.h>
#endif

#define FASTBOOT_DOT_LINE_WIDTH_DEFAULT	74

/**
 * image_size - final fastboot image size
 */
static u32 image_size;

/**
 * fastboot_bytes_received - number of bytes received in the current download
 */
static u32 fastboot_bytes_received;

/**
 * fastboot_bytes_expected - number of bytes expected in the current download
 */
static u32 fastboot_bytes_expected;

static void okay(char *, char *);
static void boot_image(char *, char *);
static void getvar(char *, char *);
static void download(char *, char *);
static void flash(char *, char *);
static void erase(char *, char *);
static void reboot_bootloader(char *, char *);
static void reboot_fastbootd(char *, char *);
static void reboot_recovery(char *, char *);
static void oem_format(char *, char *);
static void oem_partconf(char *, char *);
static void oem_bootbus(char *, char *);
static void oem_console(char *, char *);
static void oem_board(char *, char *);
static void run_ucmd(char *, char *);
static void run_acmd(char *, char *);
#if CONFIG_IS_ENABLED(FASTBOOT_CMD_FLASHING)
static void flashing_lock(char *, char *);
static void flashing_unlock(char *, char *);
static void flashing_get_unlock_ability(char *, char *);
static bool flashing_refuse_if_locked(char *response);
#else
static inline bool flashing_refuse_if_locked(char *response) { return false; }
#endif

struct fastboot_command {
	const char *command;
	void (*dispatch)(char *cmd_parameter, char *response);
};

#ifdef CONFIG_XPL_BUILD
static const struct fastboot_command commands[FASTBOOT_COMMAND_COUNT] = {
	[FASTBOOT_COMMAND_GETVAR] = {
		.command = "getvar",
		.dispatch = getvar
	},
	[FASTBOOT_COMMAND_DOWNLOAD] = {
		.command = "download",
		.dispatch = download
	},
	[FASTBOOT_COMMAND_FLASH] =  {
		.command = "flash",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_FLASH, (flash), (NULL))
	},
	[FASTBOOT_COMMAND_ERASE] =  {
		.command = "erase",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_FLASH, (erase), (NULL))
	},
	[FASTBOOT_COMMAND_CONTINUE] =  {
		.command = "continue",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT] =  {
		.command = "reboot",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_REBOOT, (okay), (NULL))
	},
	[FASTBOOT_COMMAND_REBOOT_BOOTLOADER] =  {
		.command = "reboot-bootloader",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_REBOOT,
					     (reboot_bootloader), (NULL))
	},
	[FASTBOOT_COMMAND_REBOOT_FASTBOOTD] =  {
		.command = "reboot-fastboot",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_REBOOT,
					     (reboot_fastbootd), (NULL))
	},
	[FASTBOOT_COMMAND_REBOOT_RECOVERY] =  {
		.command = "reboot-recovery",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_REBOOT,
					     (reboot_recovery), (NULL))
	},
	[FASTBOOT_COMMAND_SET_ACTIVE] =  {
		.command = "set_active",
		.dispatch = okay
	},
};
#else
static const struct fastboot_command commands[FASTBOOT_COMMAND_COUNT] = {
	[FASTBOOT_COMMAND_GETVAR] = {
		.command = "getvar",
		.dispatch = getvar
	},
	[FASTBOOT_COMMAND_DOWNLOAD] = {
		.command = "download",
		.dispatch = download
	},
	[FASTBOOT_COMMAND_FLASH] =  {
		.command = "flash",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_FLASH, (flash), (NULL))
	},
	[FASTBOOT_COMMAND_ERASE] =  {
		.command = "erase",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_FLASH, (erase), (NULL))
	},
	[FASTBOOT_COMMAND_BOOT] =  {
		.command = "boot",
		.dispatch = boot_image
	},
	[FASTBOOT_COMMAND_CONTINUE] =  {
		.command = "continue",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT] =  {
		.command = "reboot",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_REBOOT_BOOTLOADER] =  {
		.command = "reboot-bootloader",
		.dispatch = reboot_bootloader
	},
	[FASTBOOT_COMMAND_REBOOT_FASTBOOTD] =  {
		.command = "reboot-fastboot",
		.dispatch = reboot_fastbootd
	},
	[FASTBOOT_COMMAND_REBOOT_RECOVERY] =  {
		.command = "reboot-recovery",
		.dispatch = reboot_recovery
	},
	[FASTBOOT_COMMAND_SET_ACTIVE] =  {
		.command = "set_active",
		.dispatch = okay
	},
	[FASTBOOT_COMMAND_OEM_FORMAT] = {
		.command = "oem format",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_FORMAT, (oem_format), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_PARTCONF] = {
		.command = "oem partconf",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_PARTCONF, (oem_partconf), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_BOOTBUS] = {
		.command = "oem bootbus",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_BOOTBUS, (oem_bootbus), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_RUN] = {
		.command = "oem run",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_OEM_RUN, (run_ucmd), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_CONSOLE] = {
		.command = "oem console",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_CONSOLE, (oem_console), (NULL))
	},
	[FASTBOOT_COMMAND_OEM_BOARD] = {
		.command = "oem board",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_OEM_BOARD, (oem_board), (NULL))
	},
	[FASTBOOT_COMMAND_UCMD] = {
		.command = "UCmd",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT, (run_ucmd), (NULL))
	},
	[FASTBOOT_COMMAND_ACMD] = {
		.command = "ACmd",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_UUU_SUPPORT, (run_acmd), (NULL))
	},
	[FASTBOOT_COMMAND_FLASHING_LOCK] = {
		.command = "flashing lock",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_FLASHING, (flashing_lock), (NULL))
	},
	[FASTBOOT_COMMAND_FLASHING_UNLOCK] = {
		.command = "flashing unlock",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_FLASHING, (flashing_unlock), (NULL))
	},
	[FASTBOOT_COMMAND_FLASHING_GET_UNLOCK_ABILITY] = {
		.command = "flashing get_unlock_ability",
		.dispatch = CONFIG_IS_ENABLED(FASTBOOT_CMD_FLASHING,
					      (flashing_get_unlock_ability),
					      (NULL))
	},
};
#endif

/**
 * fastboot_handle_command - Handle fastboot command
 *
 * @cmd_string: Pointer to command string
 * @response: Pointer to fastboot response buffer
 *
 * Return: Executed command, or -1 if not recognized
 */
int fastboot_handle_command(char *cmd_string, char *response)
{
	int i;
	char *cmd_parameter;

	cmd_parameter = cmd_string;
	strsep(&cmd_parameter, ":");

	for (i = 0; i < FASTBOOT_COMMAND_COUNT; i++) {
		if (commands[i].command &&
		    !strcmp(commands[i].command, cmd_string)) {
			if (commands[i].dispatch) {
				commands[i].dispatch(cmd_parameter,
							response);
				return i;
			} else {
				pr_err("command %s not supported.\n", cmd_string);
				fastboot_fail("Unsupported command", response);
				return -1;
			}
		}
	}

	pr_err("command %s not recognized.\n", cmd_string);
	fastboot_fail("unrecognized command", response);
	return -1;
}

void fastboot_multiresponse(int cmd, char *response)
{
	switch (cmd) {
	case FASTBOOT_COMMAND_GETVAR:
		fastboot_getvar_all(response);
		break;
	case FASTBOOT_COMMAND_OEM_CONSOLE:
		if (CONFIG_IS_ENABLED(FASTBOOT_CMD_OEM_CONSOLE)) {
			char buf[FASTBOOT_RESPONSE_LEN] = { 0 };

			if (console_record_isempty()) {
				console_record_reset();
				fastboot_okay(NULL, response);
			} else {
				int ret = console_record_readline(buf, sizeof(buf) - 5);

				if (ret < 0)
					fastboot_fail("Error reading console", response);
				else
					fastboot_response("INFO", response, "%s", buf);
			}
			break;
		}
		fallthrough;
	default:
		fastboot_fail("Unknown multiresponse command", response);
		break;
	}
}

/**
 * okay() - Send bare OKAY response
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 *
 * Send a bare OKAY fastboot response. This is used where the command is
 * valid, but all the work is done after the response has been sent (e.g.
 * boot, reboot etc.)
 */
static void okay(char *cmd_parameter, char *response)
{
	fastboot_okay(NULL, response);
}

/**
 * boot_image() - accept "boot"; the downloaded image is started afterwards
 *
 * Refused on a locked device: nobody verified the image.
 */
static void __maybe_unused boot_image(char *cmd_parameter, char *response)
{
	if (flashing_refuse_if_locked(response))
		return;

	fastboot_okay(NULL, response);
}

/**
 * getvar() - Read a config/version variable
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void getvar(char *cmd_parameter, char *response)
{
	fastboot_getvar(cmd_parameter, response);
}

/**
 * fastboot_download() - Start a download transfer from the client
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void download(char *cmd_parameter, char *response)
{
	char *tmp;

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}
	fastboot_bytes_received = 0;
	fastboot_bytes_expected = hextoul(cmd_parameter, &tmp);
	if (fastboot_bytes_expected == 0) {
		fastboot_fail("Expected nonzero image size", response);
		return;
	}
	/*
	 * Nothing to download yet. Response is of the form:
	 * [DATA|FAIL]$cmd_parameter
	 *
	 * where cmd_parameter is an 8 digit hexadecimal number
	 */
	if (fastboot_bytes_expected > fastboot_buf_size) {
		fastboot_fail(cmd_parameter, response);
	} else {
		printf("Starting download of %d bytes\n",
		       fastboot_bytes_expected);
		fastboot_response("DATA", response, "%s", cmd_parameter);
	}
}

/**
 * fastboot_data_remaining() - return bytes remaining in current transfer
 *
 * Return: Number of bytes left in the current download
 */
u32 fastboot_data_remaining(void)
{
	return fastboot_bytes_expected - fastboot_bytes_received;
}

/**
 * fastboot_dot_line_width() - number of dot columns to print per line
 *
 * Return: number of dot columns per line
 */
static int fastboot_dot_line_width(void)
{
	struct udevice *dev;
	struct vidconsole_priv *priv;

	if (IS_ENABLED(CONFIG_VIDEO) &&
	    !uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev)) {
		priv = dev_get_uclass_priv(dev);
		if (priv->cols > 0)
			return priv->cols;
	}

	return FASTBOOT_DOT_LINE_WIDTH_DEFAULT;
}

/**
 * fastboot_data_download() - Copy image data to fastboot_buf_addr.
 *
 * @fastboot_data: Pointer to received fastboot data
 * @fastboot_data_len: Length of received fastboot data
 * @response: Pointer to fastboot response buffer
 *
 * Copies image data from fastboot_data to fastboot_buf_addr. Writes to
 * response. fastboot_bytes_received is updated to indicate the number
 * of bytes that have been transferred.
 *
 * On completion sets image_size and ${filesize} to the total size of the
 * downloaded image.
 */
void fastboot_data_download(const void *fastboot_data,
			    unsigned int fastboot_data_len,
			    char *response)
{
#define BYTES_PER_DOT	0x20000
	u32 pre_dot_num, now_dot_num;

	if (fastboot_data_len == 0 ||
	    (fastboot_bytes_received + fastboot_data_len) >
	    fastboot_bytes_expected) {
		fastboot_fail("Received invalid data length",
			      response);
		return;
	}
	/* Download data to fastboot_buf_addr */
	memcpy(fastboot_buf_addr + fastboot_bytes_received,
	       fastboot_data, fastboot_data_len);

	pre_dot_num = fastboot_bytes_received / BYTES_PER_DOT;
	fastboot_bytes_received += fastboot_data_len;
	now_dot_num = fastboot_bytes_received / BYTES_PER_DOT;

	if (pre_dot_num != now_dot_num) {
		putc('.');
		if (!(now_dot_num % fastboot_dot_line_width()))
			putc('\n');
	}
	*response = '\0';
}

/**
 * fastboot_data_complete() - Mark current transfer complete
 *
 * @response: Pointer to fastboot response buffer
 *
 * Set image_size and ${filesize} to the total size of the downloaded image.
 */
void fastboot_data_complete(char *response)
{
	/* Download complete. Respond with "OKAY" */
	fastboot_okay(NULL, response);
	printf("\ndownloading of %d bytes finished\n", fastboot_bytes_received);
	image_size = fastboot_bytes_received;
#if CONFIG_IS_ENABLED(ENV_SUPPORT)
	env_set_hex("filesize", image_size);
#endif
	fastboot_bytes_expected = 0;
	fastboot_bytes_received = 0;
}

/**
 * flash() - write the downloaded image to the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Writes the previously downloaded image to the partition indicated by
 * cmd_parameter. Writes to response.
 */
static void __maybe_unused flash(char *cmd_parameter, char *response)
{
	if (flashing_refuse_if_locked(response))
		return;

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_BLOCK))
		fastboot_block_flash_write(cmd_parameter, fastboot_buf_addr,
					   image_size, response);

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC))
		fastboot_mmc_flash_write(cmd_parameter, fastboot_buf_addr,
					 image_size, response);

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND))
		fastboot_nand_flash_write(cmd_parameter, fastboot_buf_addr,
					  image_size, response);

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_SPI))
		fastboot_spi_flash_write(cmd_parameter, fastboot_buf_addr,
					 image_size, response);
}

/**
 * erase() - erase the indicated partition.
 *
 * @cmd_parameter: Pointer to partition name
 * @response: Pointer to fastboot response buffer
 *
 * Erases the partition indicated by cmd_parameter (clear to 0x00s). Writes
 * to response.
 */
static void __maybe_unused erase_partition(const char *name, char *response)
{
	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_BLOCK))
		fastboot_block_erase(name, response);

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC))
		fastboot_mmc_erase(name, response);

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_NAND))
		fastboot_nand_erase(name, response);

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_SPI))
		fastboot_spi_flash_erase(name, response);
}

static void __maybe_unused erase(char *cmd_parameter, char *response)
{
	if (flashing_refuse_if_locked(response))
		return;

	erase_partition(cmd_parameter, response);
}

/**
 * run_ucmd() - Execute the UCmd command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused run_ucmd(char *cmd_parameter, char *response)
{
	if (flashing_refuse_if_locked(response))
		return;

	if (!cmd_parameter) {
		pr_err("missing slot suffix\n");
		fastboot_fail("missing command", response);
		return;
	}

	if (run_command(cmd_parameter, 0))
		fastboot_fail("", response);
	else
		fastboot_okay(NULL, response);
}

static char g_a_cmd_buff[64];

void fastboot_acmd_complete(void)
{
	run_command(g_a_cmd_buff, 0);
}

/**
 * run_acmd() - Execute the ACmd command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused run_acmd(char *cmd_parameter, char *response)
{
	if (flashing_refuse_if_locked(response))
		return;

	if (!cmd_parameter) {
		pr_err("missing slot suffix\n");
		fastboot_fail("missing command", response);
		return;
	}

	if (strlen(cmd_parameter) >= sizeof(g_a_cmd_buff)) {
		pr_err("too long command\n");
		fastboot_fail("too long command", response);
		return;
	}

	strcpy(g_a_cmd_buff, cmd_parameter);
	fastboot_okay(NULL, response);
}

/**
 * reboot_bootloader() - Sets reboot bootloader flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused reboot_bootloader(char *cmd_parameter,
					     char *response)
{
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_BOOTLOADER))
		fastboot_fail("Cannot set reboot flag", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * reboot_fastbootd() - Sets reboot fastboot flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused reboot_fastbootd(char *cmd_parameter,
					    char *response)
{
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_FASTBOOTD))
		fastboot_fail("Cannot set fastboot flag", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * reboot_recovery() - Sets reboot recovery flag.
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused reboot_recovery(char *cmd_parameter,
					   char *response)
{
	if (fastboot_set_reboot_flag(FASTBOOT_REBOOT_REASON_RECOVERY))
		fastboot_fail("Cannot set recovery flag", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * oem_format() - Execute the OEM format command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_format(char *cmd_parameter, char *response)
{
	char cmdbuf[32];
	const int mmc_dev = CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC,
					      (CONFIG_VAL(FASTBOOT_FLASH_MMC_DEV)),
					      (-1));

	if (flashing_refuse_if_locked(response))
		return;

	if (!env_get("partitions")) {
		fastboot_fail("partitions not set", response);
	} else {
		sprintf(cmdbuf, "gpt write mmc %x $partitions", mmc_dev);
		if (run_command(cmdbuf, 0))
			fastboot_fail("", response);
		else
			fastboot_okay(NULL, response);
	}
}

/**
 * oem_partconf() - Execute the OEM partconf command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_partconf(char *cmd_parameter, char *response)
{
	char cmdbuf[32];
	const int mmc_dev = CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC,
					      (CONFIG_VAL(FASTBOOT_FLASH_MMC_DEV)),
					      (-1));

	if (flashing_refuse_if_locked(response))
		return;

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}

	/* execute 'mmc partconfg' command with cmd_parameter arguments*/
	snprintf(cmdbuf, sizeof(cmdbuf), "mmc partconf %x %s 0", mmc_dev, cmd_parameter);
	printf("Execute: %s\n", cmdbuf);
	if (run_command(cmdbuf, 0))
		fastboot_fail("Cannot set oem partconf", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * oem_bootbus() - Execute the OEM bootbus command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_bootbus(char *cmd_parameter, char *response)
{
	char cmdbuf[32];
	const int mmc_dev = CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC,
					      (CONFIG_VAL(FASTBOOT_FLASH_MMC_DEV)),
					      (-1));

	if (flashing_refuse_if_locked(response))
		return;

	if (!cmd_parameter) {
		fastboot_fail("Expected command parameter", response);
		return;
	}

	/* execute 'mmc bootbus' command with cmd_parameter arguments*/
	snprintf(cmdbuf, sizeof(cmdbuf), "mmc bootbus %x %s", mmc_dev, cmd_parameter);
	printf("Execute: %s\n", cmdbuf);
	if (run_command(cmdbuf, 0))
		fastboot_fail("Cannot set oem bootbus", response);
	else
		fastboot_okay(NULL, response);
}

/**
 * oem_console() - Execute the OEM console command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_console(char *cmd_parameter, char *response)
{
	if (cmd_parameter)
		console_in_puts(cmd_parameter);

	if (console_record_isempty())
		fastboot_fail("Empty console", response);
	else
		fastboot_response(FASTBOOT_MULTIRESPONSE_START, response, NULL);
}

/**
 * fastboot_oem_board() - Execute the OEM board command. This is default
 * weak implementation, which may be overwritten in board/ files.
 *
 * @cmd_parameter: Pointer to command parameter
 * @data: Pointer to fastboot input buffer
 * @size: Size of the fastboot input buffer
 * @response: Pointer to fastboot response buffer
 */
void __weak fastboot_oem_board(char *cmd_parameter, void *data, u32 size, char *response)
{
	fastboot_fail("oem board function not defined", response);
}

/**
 * oem_board() - Execute the OEM board command
 *
 * @cmd_parameter: Pointer to command parameter
 * @response: Pointer to fastboot response buffer
 */
static void __maybe_unused oem_board(char *cmd_parameter, char *response)
{
	fastboot_oem_board(cmd_parameter, (void *)fastboot_buf_addr, image_size, response);
}

#if CONFIG_IS_ENABLED(FASTBOOT_CMD_FLASHING)
/**
 * flashing_flash_dev() - block device of the fastboot flash back end
 *
 * Return: the device of the mmc or block back end, NULL for back ends
 * without a block device (nand, spi)
 */
static struct blk_desc *flashing_flash_dev(void)
{
	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC))
		return blk_get_dev("mmc",
				   CONFIG_IS_ENABLED(FASTBOOT_FLASH_MMC,
						     (CONFIG_VAL(FASTBOOT_FLASH_MMC_DEV)),
						     (-1)));

	if (CONFIG_IS_ENABLED(FASTBOOT_FLASH_BLOCK))
		return blk_get_dev(CONFIG_IS_ENABLED(FASTBOOT_FLASH_BLOCK,
						     (CONFIG_VAL(FASTBOOT_FLASH_BLOCK_INTERFACE_NAME)),
						     ("")),
				   CONFIG_IS_ENABLED(FASTBOOT_FLASH_BLOCK,
						     (CONFIG_VAL(FASTBOOT_FLASH_BLOCK_DEVICE_ID)),
						     (-1)));

	return NULL;
}

/**
 * flashing_avb_ops() - AVB ops for the lock state calls
 *
 * The lock state is kept by the AVB TA and these ops never read a
 * partition, so the storage device is only informational: the fastboot
 * flash device when it is a block device, none otherwise.
 *
 * @response: fastboot response buffer, filled on failure
 * Return: the ops, to be released with avb_ops_free(), or NULL
 */
static AvbOps *flashing_avb_ops(char *response)
{
	struct blk_desc *desc = flashing_flash_dev();
	AvbOps *ops;

	ops = avb_ops_alloc(desc ? desc->uclass_id : UCLASS_INVALID,
			    desc ? desc->devnum : -1);
	if (!ops)
		fastboot_fail("cannot allocate AVB ops", response);

	return ops;
}

int fastboot_flashing_is_unlocked(bool *unlocked, char *response)
{
	AvbOps *ops;
	AvbIOResult rc;

	ops = flashing_avb_ops(response);
	if (!ops)
		return -1;

	rc = ops->read_is_device_unlocked(ops, unlocked);
	avb_ops_free(ops);
	if (rc != AVB_IO_RESULT_OK) {
		fastboot_fail("cannot read the device lock state", response);
		return -1;
	}

	return 0;
}

/**
 * flashing_refuse_if_locked() - fail a flash or erase on a locked device
 *
 * A locked device must not let fastboot change its partitions; only
 * "flashing unlock", which wipes userdata, may be used to leave that state.
 *
 * @response: fastboot response buffer, filled when the command is refused
 * Return: true if the command must not run
 */
static bool flashing_refuse_if_locked(char *response)
{
	bool unlocked;

	if (fastboot_flashing_is_unlocked(&unlocked, response))
		return true;
	if (!unlocked) {
		fastboot_fail("device is locked", response);
		return true;
	}

	return false;
}

/**
 * flashing_set_lock() - set the AVB device lock state through the AVB TA
 *
 * @locked: new state
 * @response: fastboot response buffer, filled on failure
 * Return: 0 on success, -1 otherwise
 */
static int flashing_set_lock(bool locked, char *response)
{
	AvbOps *ops;
	AvbIOResult rc;

	ops = flashing_avb_ops(response);
	if (!ops)
		return -1;

	rc = avb_write_lock_state(ops, locked);
	avb_ops_free(ops);
	if (rc != AVB_IO_RESULT_OK) {
		fastboot_fail("cannot change the device lock state", response);
		return -1;
	}

	return 0;
}

/**
 * flashing_wipe_userdata() - erase userdata and metadata
 *
 * AVB requires user data to be wiped on every change of the device state,
 * in either direction, so userdata (and metadata, which holds the file
 * based encryption keys) are erased before the state changes. The erase
 * goes through the configured flash back end, like the "erase" command,
 * but without the lock check, since this is how a locked device gets
 * unlocked.
 *
 * @response: fastboot response buffer, filled on failure
 * Return: 0 on success, -1 when a partition could not be erased
 */
static int flashing_wipe_userdata(char *response)
{
	struct disk_partition info;
	struct blk_desc *desc;

	erase_partition("userdata", response);
	if (!strncmp(response, "FAIL", 4))
		return -1;

	/*
	 * metadata is optional on older layouts: skip it when the flash
	 * device has a partition table without it, otherwise its erase has
	 * to succeed like the one of userdata.
	 */
	desc = flashing_flash_dev();
	if (desc && part_get_info_by_name(desc, "metadata", &info) < 0)
		return 0;

	erase_partition("metadata", response);
	if (!strncmp(response, "FAIL", 4))
		return -1;

	return 0;
}

/* blocks of the frp partition hashed per read */
#define FLASHING_FRP_CHUNK	128

/**
 * flashing_unlock_allowed() - evaluate Android's "OEM unlocking" setting
 *
 * Android's PersistentDataBlockService keeps the developer option "OEM
 * unlocking" in the last byte of the frp (persistent data block)
 * partition, non-zero when the user allowed the bootloader to be unlocked,
 * and guards the partition with a SHA-256 digest in its first 32 bytes,
 * computed over the whole partition with those bytes taken as zero; on a
 * mismatch it formats the partition, which clears the option. Apply the
 * same rules: no frp partition (or no block device behind the flash back
 * end), a read error, a bad digest or a zero byte all refuse unlocking.
 *
 * Return: true if "flashing unlock" may proceed
 */
static bool flashing_unlock_allowed(void)
{
	u8 stored[SHA256_SUM_LEN], digest[SHA256_SUM_LEN];
	struct disk_partition info;
	struct blk_desc *desc;
	sha256_context ctx;
	lbaint_t blk, cnt;
	bool allowed = false;
	u8 last = 0;
	u8 *buf;

	desc = flashing_flash_dev();
	if (!desc || part_get_info_by_name(desc, "frp", &info) < 0)
		return false;

	buf = malloc_cache_aligned(FLASHING_FRP_CHUNK * info.blksz);
	if (!buf)
		return false;

	sha256_starts(&ctx);
	for (blk = 0; blk < info.size; blk += cnt) {
		cnt = info.size - blk;
		if (cnt > FLASHING_FRP_CHUNK)
			cnt = FLASHING_FRP_CHUNK;
		if (blk_dread(desc, info.start + blk, cnt, buf) != cnt)
			goto out;
		if (!blk) {
			memcpy(stored, buf, SHA256_SUM_LEN);
			memset(buf, 0, SHA256_SUM_LEN);
		}
		sha256_update(&ctx, buf, cnt * info.blksz);
		last = buf[cnt * info.blksz - 1];
	}
	sha256_finish(&ctx, digest);
	allowed = !memcmp(stored, digest, SHA256_SUM_LEN) && last;
out:
	free(buf);

	return allowed;
}

/**
 * flashing_lock() - lock the device
 *
 * Verification errors stop the boot afterwards. The AVB TA resets the
 * rollback indexes on a state change. Locking needs no permission from
 * the OS: it only takes privileges away. A device that is already locked
 * is left alone, so userdata is not wiped for nothing.
 */
static void flashing_lock(char *cmd_parameter, char *response)
{
	bool unlocked;

	if (fastboot_flashing_is_unlocked(&unlocked, response))
		return;
	if (!unlocked) {
		fastboot_fail("device already locked", response);
		return;
	}

	if (flashing_wipe_userdata(response))
		return;

	if (flashing_set_lock(true, response))
		return;

	fastboot_okay(NULL, response);
}

/**
 * flashing_unlock() - unlock the device
 *
 * Refused on a device that is already unlocked, when Android's "OEM
 * unlocking" setting does not allow it, and when userdata cannot be
 * erased.
 */
static void flashing_unlock(char *cmd_parameter, char *response)
{
	bool unlocked;

	if (fastboot_flashing_is_unlocked(&unlocked, response))
		return;
	if (unlocked) {
		fastboot_fail("device already unlocked", response);
		return;
	}

	if (!flashing_unlock_allowed()) {
		fastboot_fail("unlocking is not allowed, enable OEM unlocking in Android",
			      response);
		return;
	}

	if (flashing_wipe_userdata(response))
		return;

	if (flashing_set_lock(false, response))
		return;

	fastboot_okay(NULL, response);
}

/**
 * flashing_get_unlock_ability() - report whether unlocking is allowed
 */
static void flashing_get_unlock_ability(char *cmd_parameter, char *response)
{
	fastboot_okay(flashing_unlock_allowed() ? "1" : "0", response);
}
#endif /* FASTBOOT_CMD_FLASHING */
