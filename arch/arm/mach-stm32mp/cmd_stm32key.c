// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <log.h>
#include <misc.h>
#include <dm/device.h>
#include <dm/uclass.h>

/* Closed device : bit 6 of OPT0*/
#define STM32_OTP_CLOSE_ID		0
#define STM32_OTP_CLOSE_MASK		BIT(6)

/* HASH of key: 8 OTPs, starting with OTP24) */
#define STM32_OTP_HASH_KEY_START	24
#define STM32_OTP_HASH_KEY_SIZE		8

#define BSEC_LOCK_ERROR			(-1)
#define BSEC_LOCK_PERM			BIT(0)

static int get_misc_dev(struct udevice **dev)
{
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(stm32mp_bsec), dev);
	if (ret)
		log_err("Can't find stm32mp_bsec driver\n");

	return ret;
}

static void read_hash_value(u32 addr)
{
	int i;

	printf("Read KEY at 0x%x\n", addr);
	for (i = 0; i < STM32_OTP_HASH_KEY_SIZE; i++) {
		printf("OTP value %i: %x\n", STM32_OTP_HASH_KEY_START + i,
		       __be32_to_cpu(*(u32 *)addr));
		addr += 4;
	}
}

static int read_hash_otp(struct udevice *dev, bool print, bool *locked)
{
	int i, word, ret;
	int nb_invalid = 0, nb_zero = 0, nb_lock = 0;
	u32 val, lock;
	bool status;

	for (i = 0, word = STM32_OTP_HASH_KEY_START; i < STM32_OTP_HASH_KEY_SIZE; i++, word++) {
		ret = misc_read(dev, STM32_BSEC_OTP(word), &val, 4);
		if (ret != 4)
			val = ~0x0;
		ret = misc_read(dev, STM32_BSEC_LOCK(word), &lock, 4);
		if (ret != 4)
			lock = BSEC_LOCK_ERROR;
		if (print)
			printf("OTP HASH %i: %x lock : %x\n", word, val, lock);
		if (val == ~0x0)
			nb_invalid++;
		else if (val == 0x0)
			nb_zero++;
		if (lock & BSEC_LOCK_PERM)
			nb_lock++;
	}

	status = (nb_lock == STM32_OTP_HASH_KEY_SIZE);
	if (locked)
		*locked = status;
	if (!status && print)
		printf("Hash of key is not locked!\n");

	if (nb_invalid == STM32_OTP_HASH_KEY_SIZE) {
		if (print)
			printf("Hash of key is invalid!\n");
		return -EINVAL;
	}
	if (nb_zero == STM32_OTP_HASH_KEY_SIZE) {
		if (print)
			printf("Hash of key is free!\n");
		return -ENOENT;
	}

	return 0;
}

static int read_close_status(struct udevice *dev, bool print, bool *closed)
{
	int word, ret, result;
	u32 val, lock;
	bool status;

	result = 0;
	word = STM32_OTP_CLOSE_ID;
	ret = misc_read(dev, STM32_BSEC_OTP(word), &val, 4);
	if (ret < 0)
		result = ret;
	if (ret != 4)
		val = 0x0;

	ret = misc_read(dev, STM32_BSEC_LOCK(word), &lock, 4);
	if (ret < 0)
		result = ret;
	if (ret != 4)
		lock = BSEC_LOCK_ERROR;

	status = (val & STM32_OTP_CLOSE_MASK) == STM32_OTP_CLOSE_MASK;
	if (closed)
		*closed = status;
	if (print)
		printf("OTP %d: closed status: %d lock : %x\n", word, status, lock);

	return result;
}

static int fuse_hash_value(struct udevice *dev, u32 addr, bool print)
{
	u32 word, val;
	int i, ret;

	for (i = 0, word = STM32_OTP_HASH_KEY_START;
	     i < STM32_OTP_HASH_KEY_SIZE;
	     i++, word++, addr += 4) {
		val = __be32_to_cpu(*(u32 *)addr);
		if (print)
			printf("Fuse OTP %i : %x\n", word, val);

		ret = misc_write(dev, STM32_BSEC_OTP(word), &val, 4);
		if (ret != 4) {
			log_err("Fuse OTP %i failed\n", word);
			return ret;
		}
		/* on success, lock the OTP for HASH key */
		val = BSEC_LOCK_PERM;
		ret = misc_write(dev, STM32_BSEC_LOCK(word), &val, 4);
		if (ret != 4) {
			log_err("Lock OTP %i failed\n", word);
			return ret;
		}
	}

	return 0;
}

static int confirm_prog(void)
{
	puts("Warning: Programming fuses is an irreversible operation!\n"
			"         This may brick your system.\n"
			"         Use this command only if you are sure of what you are doing!\n"
			"\nReally perform this fuse programming? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Fuse programming aborted\n");
	return 0;
}

static int do_stm32key_read(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	u32 addr;
	int ret;

	ret = get_misc_dev(&dev);

	if (argc == 1) {
		if (ret)
			return CMD_RET_FAILURE;
		read_hash_otp(dev, true, NULL);
		ret = read_close_status(dev, true, NULL);
		if (ret)
			return CMD_RET_FAILURE;

		return CMD_RET_SUCCESS;
	}

	addr = hextoul(argv[1], NULL);
	if (!addr)
		return CMD_RET_USAGE;

	read_hash_value(addr);

	return CMD_RET_SUCCESS;
}

static int do_stm32key_fuse(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct udevice *dev;
	u32 addr;
	int ret;
	bool yes = false, lock;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (argc == 3) {
		if (strcmp(argv[1], "-y"))
			return CMD_RET_USAGE;
		yes = true;
	}

	addr = hextoul(argv[argc - 1], NULL);
	if (!addr)
		return CMD_RET_USAGE;

	ret = get_misc_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;

	if (read_hash_otp(dev, !yes, &lock) != -ENOENT) {
		printf("Error: can't fuse again the OTP\n");
		return CMD_RET_FAILURE;
	}
	if (lock) {
		printf("Error: PKH is locked\n");
		return CMD_RET_FAILURE;
	}

	if (!yes && !confirm_prog())
		return CMD_RET_FAILURE;

	if (fuse_hash_value(dev, addr, !yes))
		return CMD_RET_FAILURE;

	printf("Hash key updated !\n");

	return CMD_RET_SUCCESS;
}

static int do_stm32key_close(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	bool yes, lock, closed;
	struct udevice *dev;
	u32 val;
	int ret;

	yes = false;
	if (argc == 2) {
		if (strcmp(argv[1], "-y"))
			return CMD_RET_USAGE;
		yes = true;
	}

	ret = get_misc_dev(&dev);
	if (ret)
		return CMD_RET_FAILURE;

	if (read_close_status(dev, !yes, &closed))
		return CMD_RET_FAILURE;

	if (closed) {
		printf("Error: already closed!\n");
		return CMD_RET_FAILURE;
	}

	/* check PKH status before to close */
	ret = read_hash_otp(dev, !yes, &lock);
	if (ret) {
		if (ret == -ENOENT)
			printf("Error: OTP not programmed!\n");
		return CMD_RET_FAILURE;
	}
	if (!lock)
		printf("Warning: OTP not locked!\n");

	if (!yes && !confirm_prog())
		return CMD_RET_FAILURE;

	val = STM32_OTP_CLOSE_MASK;
	ret = misc_write(dev, STM32_BSEC_OTP(STM32_OTP_CLOSE_ID), &val, 4);
	if (ret != 4) {
		printf("Error: can't update OTP\n");
		return CMD_RET_FAILURE;
	}

	printf("Device is closed !\n");

	return CMD_RET_SUCCESS;
}

static char stm32key_help_text[] =
	"read [<addr>]: Read the hash stored at addr in memory or in OTP\n"
	"stm32key fuse [-y] <addr> : Fuse hash stored at addr in OTP\n"
	"stm32key close [-y] : Close the device, the hash stored in OTP\n";

U_BOOT_CMD_WITH_SUBCMDS(stm32key, "Fuse ST Hash key", stm32key_help_text,
	U_BOOT_SUBCMD_MKENT(read, 2, 0, do_stm32key_read),
	U_BOOT_SUBCMD_MKENT(fuse, 3, 0, do_stm32key_fuse),
	U_BOOT_SUBCMD_MKENT(close, 2, 0, do_stm32key_close));
