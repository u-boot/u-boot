/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <mmc.h>

static int curr_device = -1;
#ifndef CONFIG_GENERIC_MMC
int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int dev;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "init") == 0) {
		if (argc == 2) {
			if (curr_device < 0)
				dev = 1;
			else
				dev = curr_device;
		} else if (argc == 3) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);
		} else {
			return CMD_RET_USAGE;
		}

		if (mmc_legacy_init(dev) != 0) {
			puts("No MMC card found\n");
			return 1;
		}

		curr_device = dev;
		printf("mmc%d is available\n", curr_device);
	} else if (strcmp(argv[1], "device") == 0) {
		if (argc == 2) {
			if (curr_device < 0) {
				puts("No MMC device available\n");
				return 1;
			}
		} else if (argc == 3) {
			dev = (int)simple_strtoul(argv[2], NULL, 10);

#ifdef CONFIG_SYS_MMC_SET_DEV
			if (mmc_set_dev(dev) != 0)
				return 1;
#endif
			curr_device = dev;
		} else {
			return CMD_RET_USAGE;
		}

		printf("mmc%d is current device\n", curr_device);
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	mmc, 3, 1, do_mmc,
	"MMC sub-system",
	"init [dev] - init MMC sub system\n"
	"mmc device [dev] - show or set current device"
);
#else /* !CONFIG_GENERIC_MMC */

static void print_mmcinfo(struct mmc *mmc)
{
	printf("Device: %s\n", mmc->cfg->name);
	printf("Manufacturer ID: %x\n", mmc->cid[0] >> 24);
	printf("OEM: %x\n", (mmc->cid[0] >> 8) & 0xffff);
	printf("Name: %c%c%c%c%c \n", mmc->cid[0] & 0xff,
			(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
			(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff);

	printf("Tran Speed: %d\n", mmc->tran_speed);
	printf("Rd Block Len: %d\n", mmc->read_bl_len);

	printf("%s version %d.%d\n", IS_SD(mmc) ? "SD" : "MMC",
			(mmc->version >> 8) & 0xf, mmc->version & 0xff);

	printf("High Capacity: %s\n", mmc->high_capacity ? "Yes" : "No");
	puts("Capacity: ");
	print_size(mmc->capacity, "\n");

	printf("Bus Width: %d-bit\n", mmc->bus_width);
}
static struct mmc *init_mmc_device(int dev, bool force_init)
{
	struct mmc *mmc;
	mmc = find_mmc_device(dev);
	if (!mmc) {
		printf("no mmc device at slot %x\n", dev);
		return NULL;
	}
	if (force_init)
		mmc->has_init = 0;
	if (mmc_init(mmc))
		return NULL;
	return mmc;
}
static int do_mmcinfo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct mmc *mmc;

	if (curr_device < 0) {
		if (get_mmc_num() > 0)
			curr_device = 0;
		else {
			puts("No MMC device available\n");
			return 1;
		}
	}

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	print_mmcinfo(mmc);
	return CMD_RET_SUCCESS;
}

#ifdef CONFIG_SUPPORT_EMMC_RPMB
static int confirm_key_prog(void)
{
	puts("Warning: Programming authentication key can be done only once !\n"
	     "         Use this command only if you are sure of what you are doing,\n"
	     "Really perform the key programming? <y/N> ");
	if (confirm_yesno())
		return 1;

	puts("Authentication key programming aborted\n");
	return 0;
}
static int do_mmcrpmb_key(cmd_tbl_t *cmdtp, int flag,
			  int argc, char * const argv[])
{
	void *key_addr;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (argc != 2)
		return CMD_RET_USAGE;

	key_addr = (void *)simple_strtoul(argv[1], NULL, 16);
	if (!confirm_key_prog())
		return CMD_RET_FAILURE;
	if (mmc_rpmb_set_key(mmc, key_addr)) {
		printf("ERROR - Key already programmed ?\n");
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}
static int do_mmcrpmb_read(cmd_tbl_t *cmdtp, int flag,
			   int argc, char * const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr = NULL;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (argc < 4)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);

	if (argc == 5)
		key_addr = (void *)simple_strtoul(argv[4], NULL, 16);

	printf("\nMMC RPMB read: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);
	n =  mmc_rpmb_read(mmc, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}
static int do_mmcrpmb_write(cmd_tbl_t *cmdtp, int flag,
			    int argc, char * const argv[])
{
	u16 blk, cnt;
	void *addr;
	int n;
	void *key_addr;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (argc != 5)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);
	key_addr = (void *)simple_strtoul(argv[4], NULL, 16);

	printf("\nMMC RPMB write: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);
	n =  mmc_rpmb_write(mmc, addr, blk, cnt, key_addr);

	printf("%d RPMB blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");
	if (n != cnt)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}
static int do_mmcrpmb_counter(cmd_tbl_t *cmdtp, int flag,
			      int argc, char * const argv[])
{
	unsigned long counter;
	struct mmc *mmc = find_mmc_device(curr_device);

	if (mmc_rpmb_get_counter(mmc, &counter))
		return CMD_RET_FAILURE;
	printf("RPMB Write counter= %lx\n", counter);
	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_rpmb[] = {
	U_BOOT_CMD_MKENT(key, 2, 0, do_mmcrpmb_key, "", ""),
	U_BOOT_CMD_MKENT(read, 5, 1, do_mmcrpmb_read, "", ""),
	U_BOOT_CMD_MKENT(write, 5, 0, do_mmcrpmb_write, "", ""),
	U_BOOT_CMD_MKENT(counter, 1, 1, do_mmcrpmb_counter, "", ""),
};

static int do_mmcrpmb(cmd_tbl_t *cmdtp, int flag,
		      int argc, char * const argv[])
{
	cmd_tbl_t *cp;
	struct mmc *mmc;
	char original_part;
	int ret;

	cp = find_cmd_tbl(argv[1], cmd_rpmb, ARRAY_SIZE(cmd_rpmb));

	/* Drop the rpmb subcommand */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cp->repeatable)
		return CMD_RET_SUCCESS;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (!(mmc->version & MMC_VERSION_MMC)) {
		printf("It is not a EMMC device\n");
		return CMD_RET_FAILURE;
	}
	if (mmc->version < MMC_VERSION_4_41) {
		printf("RPMB not supported before version 4.41\n");
		return CMD_RET_FAILURE;
	}
	/* Switch to the RPMB partition */
	original_part = mmc->part_num;
	if (mmc->part_num != MMC_PART_RPMB) {
		if (mmc_switch_part(curr_device, MMC_PART_RPMB) != 0)
			return CMD_RET_FAILURE;
		mmc->part_num = MMC_PART_RPMB;
	}
	ret = cp->cmd(cmdtp, flag, argc, argv);

	/* Return to original partition */
	if (mmc->part_num != original_part) {
		if (mmc_switch_part(curr_device, original_part) != 0)
			return CMD_RET_FAILURE;
		mmc->part_num = original_part;
	}
	return ret;
}
#endif

static int do_mmc_read(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	void *addr;

	if (argc != 4)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC read: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	n = mmc->block_dev.block_read(curr_device, blk, cnt, addr);
	/* flush cache after read */
	flush_cache((ulong)addr, cnt * 512); /* FIXME */
	printf("%d blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}
static int do_mmc_write(cmd_tbl_t *cmdtp, int flag,
			int argc, char * const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;
	void *addr;

	if (argc != 4)
		return CMD_RET_USAGE;

	addr = (void *)simple_strtoul(argv[1], NULL, 16);
	blk = simple_strtoul(argv[2], NULL, 16);
	cnt = simple_strtoul(argv[3], NULL, 16);

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC write: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}
	n = mmc->block_dev.block_write(curr_device, blk, cnt, addr);
	printf("%d blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}
static int do_mmc_erase(cmd_tbl_t *cmdtp, int flag,
			int argc, char * const argv[])
{
	struct mmc *mmc;
	u32 blk, cnt, n;

	if (argc != 3)
		return CMD_RET_USAGE;

	blk = simple_strtoul(argv[1], NULL, 16);
	cnt = simple_strtoul(argv[2], NULL, 16);

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	printf("\nMMC erase: dev # %d, block # %d, count %d ... ",
	       curr_device, blk, cnt);

	if (mmc_getwp(mmc) == 1) {
		printf("Error: card is write protected!\n");
		return CMD_RET_FAILURE;
	}
	n = mmc->block_dev.block_erase(curr_device, blk, cnt);
	printf("%d blocks erased: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}
static int do_mmc_rescan(cmd_tbl_t *cmdtp, int flag,
			 int argc, char * const argv[])
{
	struct mmc *mmc;

	mmc = init_mmc_device(curr_device, true);
	if (!mmc)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}
static int do_mmc_part(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	block_dev_desc_t *mmc_dev;
	struct mmc *mmc;

	mmc = init_mmc_device(curr_device, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	mmc_dev = mmc_get_dev(curr_device);
	if (mmc_dev != NULL && mmc_dev->type != DEV_TYPE_UNKNOWN) {
		print_part(mmc_dev);
		return CMD_RET_SUCCESS;
	}

	puts("get mmc type error!\n");
	return CMD_RET_FAILURE;
}
static int do_mmc_dev(cmd_tbl_t *cmdtp, int flag,
		      int argc, char * const argv[])
{
	int dev, part = 0, ret;
	struct mmc *mmc;

	if (argc == 1) {
		dev = curr_device;
	} else if (argc == 2) {
		dev = simple_strtoul(argv[1], NULL, 10);
	} else if (argc == 3) {
		dev = (int)simple_strtoul(argv[1], NULL, 10);
		part = (int)simple_strtoul(argv[2], NULL, 10);
		if (part > PART_ACCESS_MASK) {
			printf("#part_num shouldn't be larger than %d\n",
			       PART_ACCESS_MASK);
			return CMD_RET_FAILURE;
		}
	} else {
		return CMD_RET_USAGE;
	}

	mmc = init_mmc_device(dev, true);
	if (!mmc)
		return CMD_RET_FAILURE;

	ret = mmc_select_hwpart(dev, part);
	printf("switch to partitions #%d, %s\n",
	       part, (!ret) ? "OK" : "ERROR");
	if (ret)
		return 1;

	curr_device = dev;
	if (mmc->part_config == MMCPART_NOAVAILABLE)
		printf("mmc%d is current device\n", curr_device);
	else
		printf("mmc%d(part %d) is current device\n",
		       curr_device, mmc->part_num);

	return CMD_RET_SUCCESS;
}
static int do_mmc_list(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	print_mmc_devices('\n');
	return CMD_RET_SUCCESS;
}
#ifdef CONFIG_SUPPORT_EMMC_BOOT
static int do_mmc_bootbus(cmd_tbl_t *cmdtp, int flag,
			  int argc, char * const argv[])
{
	int dev;
	struct mmc *mmc;
	u8 width, reset, mode;

	if (argc != 5)
		return CMD_RET_USAGE;
	dev = simple_strtoul(argv[1], NULL, 10);
	width = simple_strtoul(argv[2], NULL, 10);
	reset = simple_strtoul(argv[3], NULL, 10);
	mode = simple_strtoul(argv[4], NULL, 10);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("BOOT_BUS_WIDTH only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	/* acknowledge to be sent during boot operation */
	return mmc_set_boot_bus_width(mmc, width, reset, mode);
}
static int do_mmc_boot_resize(cmd_tbl_t *cmdtp, int flag,
			      int argc, char * const argv[])
{
	int dev;
	struct mmc *mmc;
	u32 bootsize, rpmbsize;

	if (argc != 4)
		return CMD_RET_USAGE;
	dev = simple_strtoul(argv[1], NULL, 10);
	bootsize = simple_strtoul(argv[2], NULL, 10);
	rpmbsize = simple_strtoul(argv[3], NULL, 10);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		printf("It is not a EMMC device\n");
		return CMD_RET_FAILURE;
	}

	if (mmc_boot_partition_size_change(mmc, bootsize, rpmbsize)) {
		printf("EMMC boot partition Size change Failed.\n");
		return CMD_RET_FAILURE;
	}

	printf("EMMC boot partition Size %d MB\n", bootsize);
	printf("EMMC RPMB partition Size %d MB\n", rpmbsize);
	return CMD_RET_SUCCESS;
}
static int do_mmc_partconf(cmd_tbl_t *cmdtp, int flag,
			   int argc, char * const argv[])
{
	int dev;
	struct mmc *mmc;
	u8 ack, part_num, access;

	if (argc != 5)
		return CMD_RET_USAGE;

	dev = simple_strtoul(argv[1], NULL, 10);
	ack = simple_strtoul(argv[2], NULL, 10);
	part_num = simple_strtoul(argv[3], NULL, 10);
	access = simple_strtoul(argv[4], NULL, 10);

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("PARTITION_CONFIG only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	/* acknowledge to be sent during boot operation */
	return mmc_set_part_conf(mmc, ack, part_num, access);
}
static int do_mmc_rst_func(cmd_tbl_t *cmdtp, int flag,
			   int argc, char * const argv[])
{
	int dev;
	struct mmc *mmc;
	u8 enable;

	/*
	 * Set the RST_n_ENABLE bit of RST_n_FUNCTION
	 * The only valid values are 0x0, 0x1 and 0x2 and writing
	 * a value of 0x1 or 0x2 sets the value permanently.
	 */
	if (argc != 3)
		return CMD_RET_USAGE;

	dev = simple_strtoul(argv[1], NULL, 10);
	enable = simple_strtoul(argv[2], NULL, 10);

	if (enable > 2 || enable < 0) {
		puts("Invalid RST_n_ENABLE value\n");
		return CMD_RET_USAGE;
	}

	mmc = init_mmc_device(dev, false);
	if (!mmc)
		return CMD_RET_FAILURE;

	if (IS_SD(mmc)) {
		puts("RST_n_FUNCTION only exists on eMMC\n");
		return CMD_RET_FAILURE;
	}

	return mmc_set_rst_n_function(mmc, enable);
}
#endif
static int do_mmc_setdsr(cmd_tbl_t *cmdtp, int flag,
			 int argc, char * const argv[])
{
	struct mmc *mmc;
	u32 val;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;
	val = simple_strtoul(argv[2], NULL, 16);

	mmc = find_mmc_device(curr_device);
	if (!mmc) {
		printf("no mmc device at slot %x\n", curr_device);
		return CMD_RET_FAILURE;
	}
	ret = mmc_set_dsr(mmc, val);
	printf("set dsr %s\n", (!ret) ? "OK, force rescan" : "ERROR");
	if (!ret) {
		mmc->has_init = 0;
		if (mmc_init(mmc))
			return CMD_RET_FAILURE;
		else
			return CMD_RET_SUCCESS;
	}
	return ret;
}

static cmd_tbl_t cmd_mmc[] = {
	U_BOOT_CMD_MKENT(info, 1, 0, do_mmcinfo, "", ""),
	U_BOOT_CMD_MKENT(read, 4, 1, do_mmc_read, "", ""),
	U_BOOT_CMD_MKENT(write, 4, 0, do_mmc_write, "", ""),
	U_BOOT_CMD_MKENT(erase, 3, 0, do_mmc_erase, "", ""),
	U_BOOT_CMD_MKENT(rescan, 1, 1, do_mmc_rescan, "", ""),
	U_BOOT_CMD_MKENT(part, 1, 1, do_mmc_part, "", ""),
	U_BOOT_CMD_MKENT(dev, 3, 0, do_mmc_dev, "", ""),
	U_BOOT_CMD_MKENT(list, 1, 1, do_mmc_list, "", ""),
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	U_BOOT_CMD_MKENT(bootbus, 5, 0, do_mmc_bootbus, "", ""),
	U_BOOT_CMD_MKENT(bootpart-resize, 3, 0, do_mmc_boot_resize, "", ""),
	U_BOOT_CMD_MKENT(partconf, 5, 0, do_mmc_partconf, "", ""),
	U_BOOT_CMD_MKENT(rst-function, 3, 0, do_mmc_rst_func, "", ""),
#endif
#ifdef CONFIG_SUPPORT_EMMC_RPMB
	U_BOOT_CMD_MKENT(rpmb, CONFIG_SYS_MAXARGS, 1, do_mmcrpmb, "", ""),
#endif
	U_BOOT_CMD_MKENT(setdsr, 2, 0, do_mmc_setdsr, "", ""),
};

static int do_mmcops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *cp;

	cp = find_cmd_tbl(argv[1], cmd_mmc, ARRAY_SIZE(cmd_mmc));

	/* Drop the mmc command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cp->repeatable)
		return CMD_RET_SUCCESS;

	if (curr_device < 0) {
		if (get_mmc_num() > 0) {
			curr_device = 0;
		} else {
			puts("No MMC device available\n");
			return CMD_RET_FAILURE;
		}
	}
	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	mmc, 7, 1, do_mmcops,
	"MMC sub system",
	"info - display info of the current MMC device\n"
	"mmc read addr blk# cnt\n"
	"mmc write addr blk# cnt\n"
	"mmc erase blk# cnt\n"
	"mmc rescan\n"
	"mmc part - lists available partition on current mmc device\n"
	"mmc dev [dev] [part] - show or set current mmc device [partition]\n"
	"mmc list - lists available devices\n"
#ifdef CONFIG_SUPPORT_EMMC_BOOT
	"mmc bootbus dev boot_bus_width reset_boot_bus_width boot_mode\n"
	" - Set the BOOT_BUS_WIDTH field of the specified device\n"
	"mmc bootpart-resize <dev> <boot part size MB> <RPMB part size MB>\n"
	" - Change sizes of boot and RPMB partitions of specified device\n"
	"mmc partconf dev boot_ack boot_partition partition_access\n"
	" - Change the bits of the PARTITION_CONFIG field of the specified device\n"
	"mmc rst-function dev value\n"
	" - Change the RST_n_FUNCTION field of the specified device\n"
	"   WARNING: This is a write-once field and 0 / 1 / 2 are the only valid values.\n"
#endif
#ifdef CONFIG_SUPPORT_EMMC_RPMB
	"mmc rpmb read addr blk# cnt [address of auth-key] - block size is 256 bytes\n"
	"mmc rpmb write addr blk# cnt <address of auth-key> - block size is 256 bytes\n"
	"mmc rpmb key <address of auth-key> - program the RPMB authentication key.\n"
	"mmc rpmb counter - read the value of the write counter\n"
#endif
	"mmc setdsr <value> - set DSR register value\n"
	);

/* Old command kept for compatibility. Same as 'mmc info' */
U_BOOT_CMD(
	mmcinfo, 1, 0, do_mmcinfo,
	"display MMC info",
	"- display info of the current MMC device"
);

#endif /* !CONFIG_GENERIC_MMC */
