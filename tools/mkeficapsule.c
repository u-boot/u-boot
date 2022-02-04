// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 Linaro Limited
 *		Author: AKASHI Takahiro
 */

#include <getopt.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
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

/**
 * read_bin_file - read a firmware binary file
 * @bin:	Path to a firmware binary file
 * @data:	Pointer to pointer of allocated buffer
 * @bin_size:	Size of allocated buffer
 *
 * Read out a content of binary, @bin, into @data.
 * A caller should free @data.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int read_bin_file(char *bin, void **data, off_t *bin_size)
{
	FILE *g;
	struct stat bin_stat;
	void *buf;
	size_t size;
	int ret = 0;

	g = fopen(bin, "r");
	if (!g) {
		fprintf(stderr, "cannot open %s\n", bin);
		return -1;
	}
	if (stat(bin, &bin_stat) < 0) {
		fprintf(stderr, "cannot determine the size of %s\n", bin);
		ret = -1;
		goto err;
	}
	if (bin_stat.st_size > SIZE_MAX) {
		fprintf(stderr, "file size is too large for malloc: %s\n", bin);
		ret = -1;
		goto err;
	}
	buf = malloc(bin_stat.st_size);
	if (!buf) {
		fprintf(stderr, "cannot allocate memory: %zx\n",
			(size_t)bin_stat.st_size);
		ret = -1;
		goto err;
	}

	size = fread(buf, 1, bin_stat.st_size, g);
	if (size < bin_stat.st_size) {
		fprintf(stderr, "read failed (%zx)\n", size);
		ret = -1;
		goto err;
	}

	*data = buf;
	*bin_size = bin_stat.st_size;
err:
	fclose(g);

	return ret;
}

/**
 * write_capsule_file - write a capsule file
 * @bin:	FILE stream
 * @data:	Pointer to data
 * @bin_size:	Size of data
 *
 * Write out data, @data, with the size @bin_size.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int write_capsule_file(FILE *f, void *data, size_t size, const char *msg)
{
	size_t size_written;

	size_written = fwrite(data, 1, size, f);
	if (size_written < size) {
		fprintf(stderr, "%s: write failed (%zx != %zx)\n", msg,
			size_written, size);
		return -1;
	}

	return 0;
}

/**
 * create_fwbin - create an uefi capsule file
 * @path:	Path to a created capsule file
 * @bin:	Path to a firmware binary to encapsulate
 * @guid:	GUID of related FMP driver
 * @index:	Index number in capsule
 * @instance:	Instance number in capsule
 * @mcount:	Monotonic count in authentication information
 * @private_file:	Path to a private key file
 * @cert_file:	Path to a certificate file
 *
 * This function actually does the job of creating an uefi capsule file.
 * All the arguments must be supplied.
 * If either @private_file ror @cert_file is NULL, the capsule file
 * won't be signed.
 *
 * Return:
 * * 0  - on success
 * * -1 - on failure
 */
static int create_fwbin(char *path, char *bin, efi_guid_t *guid,
			unsigned long index, unsigned long instance)
{
	struct efi_capsule_header header;
	struct efi_firmware_management_capsule_header capsule;
	struct efi_firmware_management_capsule_image_header image;
	FILE *f;
	void *data;
	off_t bin_size;
	u64 offset;
	int ret;

#ifdef DEBUG
	printf("For output: %s\n", path);
	printf("\tbin: %s\n\ttype: %pUl\n", bin, guid);
	printf("\tindex: %ld\n\tinstance: %ld\n", index, instance);
#endif

	f = NULL;
	data = NULL;
	ret = -1;

	/*
	 * read a firmware binary
	 */
	if (read_bin_file(bin, &data, &bin_size))
		goto err;

	/*
	 * write a capsule file
	 */
	f = fopen(path, "w");
	if (!f) {
		fprintf(stderr, "cannot open %s\n", path);
		goto err;
	}

	/*
	 * capsule file header
	 */
	header.capsule_guid = efi_guid_fm_capsule;
	header.header_size = sizeof(header);
	/* TODO: The current implementation ignores flags */
	header.flags = CAPSULE_FLAGS_PERSIST_ACROSS_RESET;
	header.capsule_image_size = sizeof(header)
					+ sizeof(capsule) + sizeof(u64)
					+ sizeof(image)
					+ bin_size;
	if (write_capsule_file(f, &header, sizeof(header),
			       "Capsule header"))
		goto err;

	/*
	 * firmware capsule header
	 * This capsule has only one firmware capsule image.
	 */
	capsule.version = 0x00000001;
	capsule.embedded_driver_count = 0;
	capsule.payload_item_count = 1;
	if (write_capsule_file(f, &capsule, sizeof(capsule),
			       "Firmware capsule header"))
		goto err;

	offset = sizeof(capsule) + sizeof(u64);
	if (write_capsule_file(f, &offset, sizeof(offset),
			       "Offset to capsule image"))
		goto err;

	/*
	 * firmware capsule image header
	 */
	image.version = 0x00000003;
	memcpy(&image.update_image_type_id, guid, sizeof(*guid));
	image.update_image_index = index;
	image.reserved[0] = 0;
	image.reserved[1] = 0;
	image.reserved[2] = 0;
	image.update_image_size = bin_size;
	image.update_vendor_code_size = 0; /* none */
	image.update_hardware_instance = instance;
	image.image_capsule_support = 0;
	if (write_capsule_file(f, &image, sizeof(image),
			       "Firmware capsule image header"))
		goto err;

	/*
	 * firmware binary
	 */
	if (write_capsule_file(f, data, bin_size, "Firmware binary"))
		goto err;

	ret = 0;
err:
	if (f)
		fclose(f);
	free(data);

	return ret;
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
				fprintf(stderr, "Image already specified\n");
				return -1;
			}
			file = optarg;
			guid = &efi_guid_image_type_uboot_fit;
			break;
		case 'r':
			if (file) {
				fprintf(stderr, "Image already specified\n");
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
		fprintf(stderr, "Creating firmware capsule failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
