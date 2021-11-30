// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 Linaro Limited
 *		Author: AKASHI Takahiro
 */

#include <getopt.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

#include <sys/stat.h>
#include <sys/types.h>

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;
typedef __u64 u64;
typedef __s16 s16;
typedef __s32 s32;

#define aligned_u64 __aligned_u64

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include <efi.h>
#include <efi_api.h>

static const char *tool_name = "mkeficapsule";

efi_guid_t efi_guid_fm_capsule = EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;
efi_guid_t efi_guid_image_type_uboot_fit =
		EFI_FIRMWARE_IMAGE_TYPE_UBOOT_FIT_GUID;
efi_guid_t efi_guid_image_type_uboot_raw =
		EFI_FIRMWARE_IMAGE_TYPE_UBOOT_RAW_GUID;

static struct option options[] = {
	{"fit", required_argument, NULL, 'f'},
	{"raw", required_argument, NULL, 'r'},
	{"index", required_argument, NULL, 'i'},
	{"instance", required_argument, NULL, 'I'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

static void print_usage(void)
{
	printf("Usage: %s [options] <output file>\n"
	       "Options:\n"

	       "\t-f, --fit <fit image>       new FIT image file\n"
	       "\t-r, --raw <raw image>       new raw image file\n"
	       "\t-i, --index <index>         update image index\n"
	       "\t-I, --instance <instance>   update hardware instance\n"
	       "\t-h, --help                  print a help message\n",
	       tool_name);
}

static int create_fwbin(char *path, char *bin, efi_guid_t *guid,
			unsigned long index, unsigned long instance)
{
	struct efi_capsule_header header;
	struct efi_firmware_management_capsule_header capsule;
	struct efi_firmware_management_capsule_image_header image;
	FILE *f, *g;
	struct stat bin_stat;
	u8 *data;
	size_t size;
	u64 offset;

#ifdef DEBUG
	printf("For output: %s\n", path);
	printf("\tbin: %s\n\ttype: %pUl\n", bin, guid);
	printf("\tindex: %ld\n\tinstance: %ld\n", index, instance);
#endif

	g = fopen(bin, "r");
	if (!g) {
		printf("cannot open %s\n", bin);
		return -1;
	}
	if (stat(bin, &bin_stat) < 0) {
		printf("cannot determine the size of %s\n", bin);
		goto err_1;
	}
	data = malloc(bin_stat.st_size);
	if (!data) {
		printf("cannot allocate memory: %zx\n", (size_t)bin_stat.st_size);
		goto err_1;
	}
	f = fopen(path, "w");
	if (!f) {
		printf("cannot open %s\n", path);
		goto err_2;
	}
	header.capsule_guid = efi_guid_fm_capsule;
	header.header_size = sizeof(header);
	/* TODO: The current implementation ignores flags */
	header.flags = CAPSULE_FLAGS_PERSIST_ACROSS_RESET;
	header.capsule_image_size = sizeof(header)
					+ sizeof(capsule) + sizeof(u64)
					+ sizeof(image)
					+ bin_stat.st_size;

	size = fwrite(&header, 1, sizeof(header), f);
	if (size < sizeof(header)) {
		printf("write failed (%zx)\n", size);
		goto err_3;
	}

	capsule.version = 0x00000001;
	capsule.embedded_driver_count = 0;
	capsule.payload_item_count = 1;
	size = fwrite(&capsule, 1, sizeof(capsule), f);
	if (size < (sizeof(capsule))) {
		printf("write failed (%zx)\n", size);
		goto err_3;
	}
	offset = sizeof(capsule) + sizeof(u64);
	size = fwrite(&offset, 1, sizeof(offset), f);
	if (size < sizeof(offset)) {
		printf("write failed (%zx)\n", size);
		goto err_3;
	}

	image.version = 0x00000003;
	memcpy(&image.update_image_type_id, guid, sizeof(*guid));
	image.update_image_index = index;
	image.reserved[0] = 0;
	image.reserved[1] = 0;
	image.reserved[2] = 0;
	image.update_image_size = bin_stat.st_size;
	image.update_vendor_code_size = 0; /* none */
	image.update_hardware_instance = instance;
	image.image_capsule_support = 0;

	size = fwrite(&image, 1, sizeof(image), f);
	if (size < sizeof(image)) {
		printf("write failed (%zx)\n", size);
		goto err_3;
	}
	size = fread(data, 1, bin_stat.st_size, g);
	if (size < bin_stat.st_size) {
		printf("read failed (%zx)\n", size);
		goto err_3;
	}
	size = fwrite(data, 1, bin_stat.st_size, f);
	if (size < bin_stat.st_size) {
		printf("write failed (%zx)\n", size);
		goto err_3;
	}

	fclose(f);
	fclose(g);
	free(data);

	return 0;

err_3:
	fclose(f);
err_2:
	free(data);
err_1:
	fclose(g);

	return -1;
}

/*
 * Usage:
 *   $ mkeficapsule -f <firmware binary> <output file>
 */
int main(int argc, char **argv)
{
	char *file;
	efi_guid_t *guid;
	unsigned long index, instance;
	int c, idx;

	file = NULL;
	guid = NULL;
	index = 0;
	instance = 0;
	for (;;) {
		c = getopt_long(argc, argv, "f:r:i:I:v:h", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			if (file) {
				printf("Image already specified\n");
				return -1;
			}
			file = optarg;
			guid = &efi_guid_image_type_uboot_fit;
			break;
		case 'r':
			if (file) {
				printf("Image already specified\n");
				return -1;
			}
			file = optarg;
			guid = &efi_guid_image_type_uboot_raw;
			break;
		case 'i':
			index = strtoul(optarg, NULL, 0);
			break;
		case 'I':
			instance = strtoul(optarg, NULL, 0);
			break;
		case 'h':
			print_usage();
			return 0;
		}
	}

	/* need an output file */
	if (argc != optind + 1) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	/* need a fit image file or raw image file */
	if (!file) {
		print_usage();
		exit(EXIT_SUCCESS);
	}

	if (create_fwbin(argv[optind], file, guid, index, instance)
			< 0) {
		printf("Creating firmware capsule failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
