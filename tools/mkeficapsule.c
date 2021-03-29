// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 Linaro Limited
 *		Author: AKASHI Takahiro
 */

#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "fdt_host.h"

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;
typedef __u64 u64;
typedef __s16 s16;
typedef __s32 s32;

#define aligned_u64 __aligned_u64

#define SIGNATURE_NODENAME	"signature"
#define OVERLAY_NODENAME	"__overlay__"

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
	{"dtb", required_argument, NULL, 'D'},
	{"public key", required_argument, NULL, 'K'},
	{"overlay", no_argument, NULL, 'O'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

static void print_usage(void)
{
	printf("Usage: %s [options] <output file>\n"
	       "Options:\n"

	       "\t--fit <fit image>       new FIT image file\n"
	       "\t--raw <raw image>       new raw image file\n"
	       "\t--index <index>         update image index\n"
	       "\t--instance <instance>   update hardware instance\n"
	       "\t--public-key <key file> public key esl file\n"
	       "\t--dtb <dtb file>        dtb file\n"
	       "\t--overlay               the dtb file is an overlay\n"
	       "\t--help                  print a help message\n",
	       tool_name);
}

static int fdt_add_pub_key_data(void *sptr, void *dptr, size_t key_size,
				bool overlay)
{
	int parent;
	int ov_node;
	int frag_node;
	int ret = 0;

	if (overlay) {
		/*
		 * The signature would be stored in the
		 * first fragment node of the overlay
		 */
		frag_node = fdt_first_subnode(dptr, 0);
		if (frag_node == -FDT_ERR_NOTFOUND) {
			fprintf(stderr,
				"Couldn't find the fragment node: %s\n",
				fdt_strerror(frag_node));
			goto done;
		}

		ov_node = fdt_subnode_offset(dptr, frag_node, OVERLAY_NODENAME);
		if (ov_node == -FDT_ERR_NOTFOUND) {
			fprintf(stderr,
				"Couldn't find the __overlay__ node: %s\n",
				fdt_strerror(ov_node));
			goto done;
		}
	} else {
		ov_node = 0;
	}

	parent = fdt_subnode_offset(dptr, ov_node, SIGNATURE_NODENAME);
	if (parent == -FDT_ERR_NOTFOUND) {
		parent = fdt_add_subnode(dptr, ov_node, SIGNATURE_NODENAME);
		if (parent < 0) {
			ret = parent;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr,
					"Couldn't create signature node: %s\n",
					fdt_strerror(parent));
			}
		}
	}
	if (ret)
		goto done;

	/* Write the key to the FDT node */
	ret = fdt_setprop(dptr, parent, "capsule-key",
			  sptr, key_size);

done:
	if (ret)
		ret = ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;

	return ret;
}

static int add_public_key(const char *pkey_file, const char *dtb_file,
			  bool overlay)
{
	int ret;
	int srcfd = -1;
	int destfd = -1;
	void *sptr = NULL;
	void *dptr = NULL;
	off_t src_size;
	struct stat pub_key;
	struct stat dtb;

	/* Find out the size of the public key */
	srcfd = open(pkey_file, O_RDONLY);
	if (srcfd == -1) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			__func__, pkey_file, strerror(errno));
		ret = -1;
		goto err;
	}

	ret = fstat(srcfd, &pub_key);
	if (ret == -1) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			__func__, pkey_file, strerror(errno));
		ret = -1;
		goto err;
	}

	src_size = pub_key.st_size;

	/* mmap the public key esl file */
	sptr = mmap(0, src_size, PROT_READ, MAP_SHARED, srcfd, 0);
	if (sptr == MAP_FAILED) {
		fprintf(stderr, "%s: Failed to mmap %s:%s\n",
			__func__, pkey_file, strerror(errno));
		ret = -1;
		goto err;
	}

	/* Open the dest FDT */
	destfd = open(dtb_file, O_RDWR);
	if (destfd == -1) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			__func__, dtb_file, strerror(errno));
		ret = -1;
		goto err;
	}

	ret = fstat(destfd, &dtb);
	if (ret == -1) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			__func__, dtb_file, strerror(errno));
		goto err;
	}

	dtb.st_size += src_size + 0x30;
	if (ftruncate(destfd, dtb.st_size)) {
		fprintf(stderr, "%s: Can't expand %s: %s\n",
			__func__, dtb_file, strerror(errno));
		ret = -1;
		goto err;
	}

	errno = 0;
	/* mmap the dtb file */
	dptr = mmap(0, dtb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		    destfd, 0);
	if (dptr == MAP_FAILED) {
		fprintf(stderr, "%s: Failed to mmap %s:%s\n",
			__func__, dtb_file, strerror(errno));
		ret = -1;
		goto err;
	}

	if (fdt_check_header(dptr)) {
		fprintf(stderr, "%s: Invalid FDT header\n", __func__);
		ret = -1;
		goto err;
	}

	ret = fdt_open_into(dptr, dptr, dtb.st_size);
	if (ret) {
		fprintf(stderr, "%s: Cannot expand FDT: %s\n",
			__func__, fdt_strerror(ret));
		ret = -1;
		goto err;
	}

	/* Copy the esl file to the expanded FDT */
	ret = fdt_add_pub_key_data(sptr, dptr, src_size, overlay);
	if (ret < 0) {
		fprintf(stderr, "%s: Unable to add public key to the FDT\n",
			__func__);
		ret = -1;
		goto err;
	}

	ret = 0;

err:
	if (sptr)
		munmap(sptr, src_size);

	if (dptr)
		munmap(dptr, dtb.st_size);

	if (srcfd != -1)
		close(srcfd);

	if (destfd != -1)
		close(destfd);

	return ret;
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
	char *pkey_file;
	char *dtb_file;
	efi_guid_t *guid;
	unsigned long index, instance;
	int c, idx;
	int ret;
	bool overlay = false;

	file = NULL;
	pkey_file = NULL;
	dtb_file = NULL;
	guid = NULL;
	index = 0;
	instance = 0;
	for (;;) {
		c = getopt_long(argc, argv, "f:r:i:I:v:D:K:Oh", options, &idx);
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
		case 'K':
			if (pkey_file) {
				printf("Public Key already specified\n");
				return -1;
			}
			pkey_file = optarg;
			break;
		case 'D':
			if (dtb_file) {
				printf("DTB file already specified\n");
				return -1;
			}
			dtb_file = optarg;
			break;
		case 'O':
			overlay = true;
			break;
		case 'h':
			print_usage();
			return 0;
		}
	}

	/* need a fit image file or raw image file */
	if (!file && !pkey_file && !dtb_file) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (pkey_file && dtb_file) {
		ret = add_public_key(pkey_file, dtb_file, overlay);
		if (ret == -1) {
			printf("Adding public key to the dtb failed\n");
			exit(EXIT_FAILURE);
		} else {
			exit(EXIT_SUCCESS);
		}
	}

	if (create_fwbin(argv[optind], file, guid, index, instance)
			< 0) {
		printf("Creating firmware capsule failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
