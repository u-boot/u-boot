// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/mach-imx/s400_api.h>
#include <asm/mach-imx/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/image.h>
#include <console.h>
#include <cpu_func.h>
#include <asm/mach-imx/ahab.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#define IMG_CONTAINER_BASE             (0x22010000UL)
#define IMG_CONTAINER_END_BASE         (IMG_CONTAINER_BASE + 0xFFFFUL)

#define AHAB_NO_AUTHENTICATION_IND 0xee
#define AHAB_BAD_KEY_HASH_IND 0xfa
#define AHAB_INVALID_KEY_IND 0xf9
#define AHAB_BAD_SIGNATURE_IND 0xf0
#define AHAB_BAD_HASH_IND 0xf1

static void display_ahab_auth_ind(u32 event)
{
	u8 resp_ind = (event >> 8) & 0xff;

	switch (resp_ind) {
	case AHAB_NO_AUTHENTICATION_IND:
		printf("AHAB_NO_AUTHENTICATION_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_BAD_KEY_HASH_IND:
		printf("AHAB_BAD_KEY_HASH_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_INVALID_KEY_IND:
		printf("AHAB_INVALID_KEY_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_BAD_SIGNATURE_IND:
		printf("AHAB_BAD_SIGNATURE_IND (0x%02X)\n\n", resp_ind);
		break;
	case AHAB_BAD_HASH_IND:
		printf("AHAB_BAD_HASH_IND (0x%02X)\n\n", resp_ind);
		break;
	default:
		printf("Unknown Indicator (0x%02X)\n\n", resp_ind);
		break;
	}
}

int ahab_auth_cntr_hdr(struct container_hdr *container, u16 length)
{
	int err;
	u32 resp;

	memcpy((void *)IMG_CONTAINER_BASE, (const void *)container,
	       ALIGN(length, CONFIG_SYS_CACHELINE_SIZE));

	flush_dcache_range(IMG_CONTAINER_BASE,
			   IMG_CONTAINER_BASE + ALIGN(length, CONFIG_SYS_CACHELINE_SIZE) - 1);

	err = ahab_auth_oem_ctnr(IMG_CONTAINER_BASE, &resp);
	if (err) {
		printf("Authenticate container hdr failed, return %d, resp 0x%x\n",
		       err, resp);
		display_ahab_auth_ind(resp);
	}

	return err;
}

int ahab_auth_release(void)
{
	int err;
	u32 resp;

	err = ahab_release_container(&resp);
	if (err) {
		printf("Error: release container failed, resp 0x%x!\n", resp);
		display_ahab_auth_ind(resp);
	}

	return err;
}

int ahab_verify_cntr_image(struct boot_img_t *img, int image_index)
{
	int err;
	u32 resp;

	err = ahab_verify_image(image_index, &resp);
	if (err) {
		printf("Authenticate img %d failed, return %d, resp 0x%x\n",
		       image_index, err, resp);
		display_ahab_auth_ind(resp);
		return -EIO;
	}

	return 0;
}

static inline bool check_in_dram(ulong addr)
{
	int i;
	struct bd_info *bd = gd->bd;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; ++i) {
		if (bd->bi_dram[i].size) {
			if (addr >= bd->bi_dram[i].start &&
			    addr < (bd->bi_dram[i].start + bd->bi_dram[i].size))
				return true;
		}
	}

	return false;
}

int authenticate_os_container(ulong addr)
{
	struct container_hdr *phdr;
	int i, ret = 0;
	int err;
	u16 length;
	struct boot_img_t *img;
	unsigned long s, e;

	if (addr % 4) {
		puts("Error: Image's address is not 4 byte aligned\n");
		return -EINVAL;
	}

	if (!check_in_dram(addr)) {
		puts("Error: Image's address is invalid\n");
		return -EINVAL;
	}

	phdr = (struct container_hdr *)addr;
	if (phdr->tag != 0x87 || phdr->version != 0x0) {
		printf("Error: Wrong container header\n");
		return -EFAULT;
	}

	if (!phdr->num_images) {
		printf("Error: Wrong container, no image found\n");
		return -EFAULT;
	}

	length = phdr->length_lsb + (phdr->length_msb << 8);

	debug("container length %u\n", length);

	err = ahab_auth_cntr_hdr(phdr, length);
	if (err) {
		ret = -EIO;
		goto exit;
	}

	debug("Verify images\n");

	/* Copy images to dest address */
	for (i = 0; i < phdr->num_images; i++) {
		img = (struct boot_img_t *)(addr +
					    sizeof(struct container_hdr) +
					    i * sizeof(struct boot_img_t));

		debug("img %d, dst 0x%x, src 0x%lx, size 0x%x\n",
		      i, (uint32_t)img->dst, img->offset + addr, img->size);

		memcpy((void *)img->dst, (const void *)(img->offset + addr), img->size);

		s = img->dst & ~(CONFIG_SYS_CACHELINE_SIZE - 1);
		e = ALIGN(img->dst + img->size, CONFIG_SYS_CACHELINE_SIZE) - 1;

		flush_dcache_range(s, e);

		ret = ahab_verify_cntr_image(img, i);
		if (ret)
			goto exit;
	}

exit:
	debug("ahab_auth_release, 0x%x\n", ret);
	ahab_auth_release();

	return ret;
}

static int do_authenticate(struct cmd_tbl *cmdtp, int flag, int argc,
			   char *const argv[])
{
	ulong addr;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], NULL, 16);

	printf("Authenticate OS container at 0x%lx\n", addr);

	if (authenticate_os_container(addr))
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

static void display_life_cycle(u32 lc)
{
	printf("Lifecycle: 0x%08X, ", lc);
	switch (lc) {
	case 0x1:
		printf("BLANK\n\n");
		break;
	case 0x2:
		printf("FAB\n\n");
		break;
	case 0x4:
		printf("NXP Provisioned\n\n");
		break;
	case 0x8:
		printf("OEM Open\n\n");
		break;
	case 0x10:
		printf("OEM Secure World Closed\n\n");
		break;
	case 0x20:
		printf("OEM closed\n\n");
		break;
	case 0x40:
		printf("Field Return OEM\n\n");
		break;
	case 0x80:
		printf("Field Return NXP\n\n");
		break;
	case 0x100:
		printf("OEM Locked\n\n");
		break;
	case 0x200:
		printf("BRICKED\n\n");
		break;
	default:
		printf("Unknown\n\n");
		break;
	}
}

static int confirm_close(void)
{
	puts("Warning: Please ensure your sample is in NXP closed state, "
	     "OEM SRK hash has been fused, \n"
	     "         and you are able to boot a signed image successfully "
	     "without any SECO events reported.\n"
	     "         If not, your sample will be unrecoverable.\n"
	     "\nReally perform this operation? <y/N>\n");

	if (confirm_yesno())
		return 1;

	puts("Ahab close aborted\n");
	return 0;
}

static int do_ahab_close(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	int err;
	u32 resp;

	if (!confirm_close())
		return -EACCES;

	err = ahab_forward_lifecycle(8, &resp);
	if (err != 0) {
		printf("Error in forward lifecycle to OEM closed\n");
		return -EIO;
	}

	printf("Change to OEM closed successfully\n");

	return 0;
}

int ahab_dump(void)
{
	u32 buffer[32];
	int ret, i = 0;

	do {
		ret = ahab_dump_buffer(buffer, 32);
		if (ret < 0) {
			printf("Error in dump AHAB log\n");
			return -EIO;
		}

		if (ret == 1)
			break;

		for (i = 0; i < ret; i++)
			printf("0x%x\n", buffer[i]);
	} while (ret >= 21);

	return 0;
}

static int do_ahab_dump(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return ahab_dump();
}

static int do_ahab_status(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	u32 lc;

	lc = readl(FSB_BASE_ADDR + 0x41c);
	lc &= 0x3f;

	display_life_cycle(lc);
	return 0;
}

U_BOOT_CMD(auth_cntr, CONFIG_SYS_MAXARGS, 1, do_authenticate,
	   "autenticate OS container via AHAB",
	   "addr\n"
	   "addr - OS container hex address\n"
);

U_BOOT_CMD(ahab_close, CONFIG_SYS_MAXARGS, 1, do_ahab_close,
	   "Change AHAB lifecycle to OEM closed",
	   ""
);

U_BOOT_CMD(ahab_dump, CONFIG_SYS_MAXARGS, 1, do_ahab_dump,
	   "Dump AHAB log for debug",
	   ""
);

U_BOOT_CMD(ahab_status, CONFIG_SYS_MAXARGS, 1, do_ahab_status,
	   "display AHAB lifecycle only",
	   ""
);
