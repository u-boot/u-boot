// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */

#include <command.h>
#include <console.h>
#include <log.h>
#include <misc.h>
#include <asm/arch/bsec.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <linux/printk.h>

/*
 * Closed device: OTP0
 * STM32MP15x: bit 6 of OTP0
 * STM32MP13x: 0b111111 = 0x3F for OTP_SECURED closed device
 * STM32MP25x: bit 0 of OTP18
 */
#define STM32MP1_OTP_CLOSE_ID				0
#define STM32_OTP_STM32MP13X_CLOSE_MASK		GENMASK(5, 0)
#define STM32_OTP_STM32MP15X_CLOSE_MASK		BIT(6)
#define STM32MP25_OTP_WORD8				8
#define STM32_OTP_STM32MP25X_BOOTROM_CLOSE_MASK	GENMASK(7, 0)
#define STM32MP25_OTP_CLOSE_ID			18
#define STM32_OTP_STM32MP25X_CLOSE_MASK		GENMASK(3, 0)
#define STM32_OTP_STM32MP25X_PROVISIONING_DONE_MASK	GENMASK(7, 4)
#define STM32MP25_OTP_HWCONFIG			124
#define STM32_OTP_STM32MP25X_DISABLE_SCAN_MASK	BIT(20)

#define STM32MP25_OTP_BOOTROM_CONF8	17
#define STM32_OTP_STM32MP25X_OEM_KEY2_EN	BIT(8)

/* PKH is the first element of the key list */
#define STM32KEY_PKH 0

struct stm32key {
	char *name;
	char *desc;
	u16 start;
	u8 size;
	int (*post_process)(struct udevice *dev);
};

const struct stm32key stm32mp13_list[] = {
	[STM32KEY_PKH] = {
		.name = "PKHTH",
		.desc = "Hash of the 8 ECC Public Keys Hashes Table (ECDSA is the authentication algorithm)",
		.start = 24,
		.size = 8,
	},
	{
		.name = "EDMK",
		.desc = "Encryption/Decryption Master Key",
		.start = 92,
		.size = 4,
	}
};

const struct stm32key stm32mp15_list[] = {
	[STM32KEY_PKH] = {
		.name = "PKH",
		.desc = "Hash of the ECC Public Key (ECDSA is the authentication algorithm)",
		.start = 24,
		.size = 8,
	}
};

static int post_process_oem_key2(struct udevice *dev);

const struct stm32key stm32mp25_list[] = {
	[STM32KEY_PKH] = {
		.name = "OEM-KEY1",
		.desc = "Hash of the 8 ECC Public Keys Hashes Table (ECDSA is the authentication algorithm) for FSBLA or M",
		.start = 144,
		.size = 8,
	},
	{
		.name = "OEM-KEY2",
		.desc = "Hash of the 8 ECC Public Keys Hashes Table (ECDSA is the authentication algorithm) for FSBLM",
		.start = 152,
		.size = 8,
		.post_process = post_process_oem_key2,
	},
	{
		.name = "FIP-EDMK",
		.desc = "Encryption/Decryption Master Key for FIP",
		.start = 260,
		.size = 8,
	},
	{
		.name = "EDMK1",
		.desc = "Encryption/Decryption Master Key for FSBLA or M",
		.start = 364,
		.size = 4,
	},
	{
		.name = "EDMK2",
		.desc = "Encryption/Decryption Master Key for FSBLM",
		.start = 360,
		.size = 4,
	}
};

struct otp_close {
	u32 word;
	u32 mask_wr;
	u32 mask_rd;
	bool (*close_status_ops)(u32 value, u32 mask);
};

static bool compare_mask_exact(u32 value, u32 mask)
{
	return ((value & mask) == mask);
}

static bool compare_any_bits(u32 value, u32 mask)
{
	return ((value & mask) != 0);
}

const struct otp_close stm32mp13_close_state_otp[] = {
	{
		.word = STM32MP1_OTP_CLOSE_ID,
		.mask_wr = STM32_OTP_STM32MP13X_CLOSE_MASK,
		.mask_rd = STM32_OTP_STM32MP13X_CLOSE_MASK,
		.close_status_ops = compare_mask_exact,
	}
};

const struct otp_close stm32mp15_close_state_otp[] = {
	{
		.word = STM32MP1_OTP_CLOSE_ID,
		.mask_wr = STM32_OTP_STM32MP15X_CLOSE_MASK,
		.mask_rd = STM32_OTP_STM32MP15X_CLOSE_MASK,
		.close_status_ops = compare_mask_exact,
	}
};

const struct otp_close stm32mp25_close_state_otp[] = {
	{
		.word = STM32MP25_OTP_WORD8,
		.mask_wr = STM32_OTP_STM32MP25X_BOOTROM_CLOSE_MASK,
		.mask_rd = 0,
		.close_status_ops = NULL
	},
	{
		.word = STM32MP25_OTP_CLOSE_ID,
		.mask_wr = STM32_OTP_STM32MP25X_CLOSE_MASK |
			   STM32_OTP_STM32MP25X_PROVISIONING_DONE_MASK,
		.mask_rd = STM32_OTP_STM32MP25X_CLOSE_MASK,
		.close_status_ops = compare_any_bits
	},
	{
		.word = STM32MP25_OTP_HWCONFIG,
		.mask_wr = STM32_OTP_STM32MP25X_DISABLE_SCAN_MASK,
		.mask_rd = 0,
		.close_status_ops = NULL
	},
};

/* index of current selected key in stm32key list, 0 = PKH by default */
static u8 stm32key_index;

static u8 get_key_nb(void)
{
	if (IS_ENABLED(CONFIG_STM32MP13X))
		return ARRAY_SIZE(stm32mp13_list);

	if (IS_ENABLED(CONFIG_STM32MP15X))
		return ARRAY_SIZE(stm32mp15_list);

	if (IS_ENABLED(CONFIG_STM32MP23X) || IS_ENABLED(CONFIG_STM32MP25X))
		return ARRAY_SIZE(stm32mp25_list);
}

static const struct stm32key *get_key(u8 index)
{
	if (IS_ENABLED(CONFIG_STM32MP13X))
		return &stm32mp13_list[index];

	if (IS_ENABLED(CONFIG_STM32MP15X))
		return &stm32mp15_list[index];

	if (IS_ENABLED(CONFIG_STM32MP23X) || IS_ENABLED(CONFIG_STM32MP25X))
		return &stm32mp25_list[index];
}

static u8 get_otp_close_state_nb(void)
{
	if (IS_ENABLED(CONFIG_STM32MP13X))
		return ARRAY_SIZE(stm32mp13_close_state_otp);

	if (IS_ENABLED(CONFIG_STM32MP15X))
		return ARRAY_SIZE(stm32mp15_close_state_otp);

	if (IS_ENABLED(CONFIG_STM32MP23X) || IS_ENABLED(CONFIG_STM32MP25X))
		return ARRAY_SIZE(stm32mp25_close_state_otp);
}

static const struct otp_close *get_otp_close_state(u8 index)
{
	if (IS_ENABLED(CONFIG_STM32MP13X))
		return &stm32mp13_close_state_otp[index];

	if (IS_ENABLED(CONFIG_STM32MP15X))
		return &stm32mp15_close_state_otp[index];

	if (IS_ENABLED(CONFIG_STM32MP23X) || IS_ENABLED(CONFIG_STM32MP25X))
		return &stm32mp25_close_state_otp[index];
}

static int get_misc_dev(struct udevice **dev)
{
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(stm32mp_bsec), dev);
	if (ret)
		log_err("Can't find stm32mp_bsec driver\n");

	return ret;
}

static void read_key_value(const struct stm32key *key, unsigned long addr)
{
	int i;

	for (i = 0; i < key->size; i++) {
		printf("%s OTP %i: [%08x] %08x\n", key->name, key->start + i,
		       (u32)addr, __be32_to_cpu(*(u32 *)addr));
		addr += 4;
	}
}

static int read_key_otp(struct udevice *dev, const struct stm32key *key, bool print, bool *locked)
{
	int i, word, ret;
	int nb_invalid = 0, nb_zero = 0, nb_lock = 0, nb_lock_err = 0;
	u32 val, lock;
	bool status;

	for (i = 0, word = key->start; i < key->size; i++, word++) {
		ret = misc_read(dev, STM32_BSEC_OTP(word), &val, 4);
		if (ret != 4)
			val = ~0x0;
		ret = misc_read(dev, STM32_BSEC_LOCK(word), &lock, 4);
		if (ret != 4)
			lock = BSEC_LOCK_ERROR;
		if (print)
			printf("%s OTP %i: %08x lock : %08x\n", key->name, word, val, lock);
		if (val == ~0x0)
			nb_invalid++;
		else if (val == 0x0)
			nb_zero++;
		if (lock & BSEC_LOCK_PERM)
			nb_lock++;
		if (lock & BSEC_LOCK_ERROR)
			nb_lock_err++;
	}

	status = nb_lock_err || (nb_lock == key->size);
	if (locked)
		*locked = status;
	if (nb_lock_err && print)
		printf("%s lock is invalid!\n", key->name);
	else if (!status && print)
		printf("%s is not locked!\n", key->name);

	if (nb_invalid == key->size) {
		if (print)
			printf("%s is invalid!\n", key->name);
		return -EINVAL;
	}
	if (nb_zero == key->size) {
		if (print)
			printf("%s is free!\n", key->name);
		return -ENOENT;
	}

	return 0;
}

static int read_close_status(struct udevice *dev, bool print, bool *closed)
{
	int ret, result, i;
	const struct otp_close *otp_close = NULL;
	u32 otp_close_nb = get_otp_close_state_nb();
	u32 val, lock, mask, word = 0;
	bool status = true;
	bool tested_once = false;

	result = 0;
	for (i = 0; status && (i < otp_close_nb); i++) {
		otp_close = get_otp_close_state(i);

		if (!otp_close->close_status_ops)
			continue;

		mask = otp_close->mask_rd;
		word = otp_close->word;

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

		status = otp_close->close_status_ops(val, mask);
		tested_once = true;
	}

	if (!tested_once)
		status = false;

	if (closed)
		*closed = status;
	if (print)
		printf("OTP %d: closed status: %d lock : %08x\n", word, status, lock);

	return result;
}

static int write_close_status(struct udevice *dev)
{
	int i;
	u32 val, word, ret;
	const struct otp_close *otp_close = NULL;
	u32 otp_num = get_otp_close_state_nb();

	for (i = 0; i < otp_num; i++) {
		otp_close = get_otp_close_state(i);
		val = otp_close->mask_wr;
		word = otp_close->word;
		ret = misc_write(dev, STM32_BSEC_OTP(word), &val, 4);
		if (ret != 4) {
			log_err("Error: can't update OTP %d\n", word);
			return ret;
		}
	}
	return 0;
}

static int post_process_oem_key2(struct udevice *dev)
{
	int ret;
	u32 val;

	ret = misc_read(dev, STM32_BSEC_OTP(STM32MP25_OTP_BOOTROM_CONF8), &val, 4);
	if (ret != 4) {
		log_err("Error %d failed to read STM32MP25_OTP_BOOTROM_CONF8\n", ret);
		return -EIO;
	}

	val |= STM32_OTP_STM32MP25X_OEM_KEY2_EN;
	ret = misc_write(dev, STM32_BSEC_OTP(STM32MP25_OTP_BOOTROM_CONF8), &val, 4);
	if (ret != 4) {
		log_err("Error %d failed to write OEM_KEY2_ENABLE\n", ret);
		return -EIO;
	}

	return 0;
}

static int fuse_key_value(struct udevice *dev, const struct stm32key *key, unsigned long addr,
			  bool print)
{
	u32 word, val;
	int i, ret;

	for (i = 0, word = key->start; i < key->size; i++, word++, addr += 4) {
		val = __be32_to_cpu(*(u32 *)addr);
		if (print)
			printf("Fuse %s OTP %i : %08x\n", key->name, word, val);

		ret = misc_write(dev, STM32_BSEC_OTP(word), &val, 4);
		if (ret != 4) {
			log_err("Fuse %s OTP %i failed\n", key->name, word);
			return ret;
		}
		/* on success, lock the OTP for the key */
		val = BSEC_LOCK_PERM;
		ret = misc_write(dev, STM32_BSEC_LOCK(word), &val, 4);
		if (ret != 4) {
			log_err("Lock %s OTP %i failed\n", key->name, word);
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

static void display_key_info(const struct stm32key *key)
{
	printf("%s : %s\n", key->name, key->desc);
	printf("\tOTP%d..%d\n", key->start, key->start + key->size - 1);
}

static int do_stm32key_list(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int i;

	for (i = 0; i < get_key_nb(); i++)
		display_key_info(get_key(i));

	return CMD_RET_SUCCESS;
}

static int do_stm32key_select(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct stm32key *key;
	int i;

	if (argc == 1) {
		printf("Selected key:\n");
		key = get_key(stm32key_index);
		display_key_info(key);
		return CMD_RET_SUCCESS;
	}

	for (i = 0; i < get_key_nb(); i++) {
		key = get_key(i);
		if (!strcmp(key->name, argv[1])) {
			printf("%s selected\n", key->name);
			stm32key_index = i;
			return CMD_RET_SUCCESS;
		}
	}

	printf("Unknown key %s\n", argv[1]);

	return CMD_RET_FAILURE;
}

static int do_stm32key_read(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct stm32key *key;
	struct udevice *dev;
	unsigned long addr;
	int ret, i;
	int result;

	ret = get_misc_dev(&dev);

	if (argc == 1) {
		if (ret)
			return CMD_RET_FAILURE;
		key = get_key(stm32key_index);
		ret = read_key_otp(dev, key, true, NULL);
		if (ret != -ENOENT)
			return CMD_RET_FAILURE;
		return CMD_RET_SUCCESS;
	}

	if (!strcmp("-a", argv[1])) {
		if (ret)
			return CMD_RET_FAILURE;
		result = CMD_RET_SUCCESS;
		for (i = 0; i < get_key_nb(); i++) {
			key = get_key(i);
			ret = read_key_otp(dev, key, true, NULL);
			if (ret != -ENOENT)
				result = CMD_RET_FAILURE;
		}
		ret = read_close_status(dev, true, NULL);
		if (ret)
			result = CMD_RET_FAILURE;

		return result;
	}

	addr = hextoul(argv[1], NULL);
	if (!addr)
		return CMD_RET_USAGE;

	key = get_key(stm32key_index);
	printf("Read %s at 0x%08x\n", key->name, (u32)addr);
	read_key_value(key, addr);

	return CMD_RET_SUCCESS;
}

static int do_stm32key_fuse(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct stm32key *key = get_key(stm32key_index);
	struct udevice *dev;
	unsigned long addr;
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

	if (read_key_otp(dev, key, !yes, &lock) != -ENOENT) {
		printf("Error: can't fuse again the OTP\n");
		return CMD_RET_FAILURE;
	}
	if (lock) {
		printf("Error: %s is locked\n", key->name);
		return CMD_RET_FAILURE;
	}

	if (!yes) {
		printf("Writing %s with\n", key->name);
		read_key_value(key, addr);
	}

	if (!yes && !confirm_prog())
		return CMD_RET_FAILURE;

	if (fuse_key_value(dev, key, addr, !yes))
		return CMD_RET_FAILURE;

	if (key->post_process) {
		if (key->post_process(dev)) {
			printf("Error: %s for post process\n", key->name);
			return CMD_RET_FAILURE;
		}
	}

	printf("%s updated !\n", key->name);

	return CMD_RET_SUCCESS;
}

static int do_stm32key_close(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct stm32key *key;
	bool yes, lock, closed;
	struct udevice *dev;
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
	key = get_key(STM32KEY_PKH);
	ret = read_key_otp(dev, key, !yes, &lock);
	if (ret) {
		if (ret == -ENOENT)
			printf("Error: %s not programmed!\n", key->name);
		return CMD_RET_FAILURE;
	}
	if (!lock)
		printf("Warning: %s not locked!\n", key->name);

	if (!yes && !confirm_prog())
		return CMD_RET_FAILURE;

	if (write_close_status(dev))
		return CMD_RET_FAILURE;

	printf("Device is closed !\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(stm32key,
	"list : list the supported key with description\n"
	"stm32key select [<key>] : Select the key identified by <key> or display the key used for read/fuse command\n"
	"stm32key read [<addr> | -a ] : Read the curent key at <addr> or current / all (-a) key in OTP\n"
	"stm32key fuse [-y] <addr> : Fuse the current key at addr in OTP\n"
	"stm32key close [-y] : Close the device\n");

U_BOOT_CMD_WITH_SUBCMDS(stm32key, "Manage key on STM32", stm32key_help_text,
	U_BOOT_SUBCMD_MKENT(list, 1, 0, do_stm32key_list),
	U_BOOT_SUBCMD_MKENT(select, 2, 0, do_stm32key_select),
	U_BOOT_SUBCMD_MKENT(read, 2, 0, do_stm32key_read),
	U_BOOT_SUBCMD_MKENT(fuse, 3, 0, do_stm32key_fuse),
	U_BOOT_SUBCMD_MKENT(close, 2, 0, do_stm32key_close));

/*
 * Check the "closed" state in product life cycle, when product secrets have
 * been provisioned into the device, by SSP tools for example.
 * On closed devices, authentication is mandatory.
 */
bool stm32mp_is_closed(void)
{
	struct udevice *dev;
	bool closed;
	int ret;

	ret = get_misc_dev(&dev);
	if (ret)
		return false;

	ret = read_close_status(dev, false, &closed);
	if (ret)
		return false;

	return closed;
}
