// SPDX-License-Identifier: GPL-2.0+
/*
 * FWU Metadata Read/Write Tool
 * Copyright (c) 2025, Kory Maincent <kory.maincent@bootlin.com>
 *
 * Tool to read, display, and modify FWU (Firmware Update) metadata
 * from Linux userspace. Similar to fw_printenv/fw_setenv for U-Boot
 * environment, but for FWU metadata.
 *
 * Usage:
 *   fwumdata                          - Print all metadata
 *   fwumdata -u                       - Print metadata and update it if CRC corrupted
 *   fwumdata -c <config>              - Use custom config file
 *   fwumdata -a <bank>                - Set active bank
 *   fwumdata -p <bank>                - Set previous bank
 *   fwumdata -s <bank> <state>        - Set bank state (V2 only)
 *   fwumdata -i <id> -b <bank> -A     - Accept image
 *   fwumdata -i <id> -b <bank> -C     - Clear image acceptance
 *   fwumdata -i <id> -b <bank>
 *            -B <num_banks>
 *            -I <num_images> -C       - Clear image acceptance (V1 only)
 *   fwumdata -l                       - List detailed info with GUIDs
 */

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <u-boot/crc.h>
#include "fwumdata.h"

/* Device configuration */
struct fwumdata_device {
	const char *devname;
	long long devoff;
	unsigned long mdata_size;
	unsigned long erase_size;
	int fd;
	bool is_mtd;
};

/* Global state */
static struct fwumdata_device devices[2];  /* Primary and secondary */
static struct fwu_mdata *mdata;
static int have_redundant;
static struct fwu_mdata *valid_mdata;
static bool mdata_mod;
static const char *config_file;
static int nbanks, nimages; /* For V1 only */
static const char * const default_config_files[] = {
	"./fwumdata.config",
	"/etc/fwumdata.config",
	NULL
};

/* GUID/UUID utilities */
static void guid_to_string(const struct efi_guid *guid, char *str)
{
	sprintf(str, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		guid->time_high, guid->time_low, guid->reserved,
		guid->family, guid->node[0],
		guid->node[1], guid->node[2], guid->node[3],
		guid->node[4], guid->node[5], guid->node[6]);
}

/* Config file parsing */
static int parse_config(const char *fname)
{
	size_t linesize = 0;
	char *line = NULL;
	char *devname;
	int i = 0;
	FILE *fp;
	int rc;

	fp = fopen(fname, "r");
	if (!fp)
		return -ENOENT;

	while (i < 2 && getline(&line, &linesize, fp) != -1) {
		/* Skip comments and empty lines */
		if (line[0] == '#' || line[0] == '\n')
			continue;

		rc = sscanf(line, "%ms %lli %lx %lx",
			    &devname,
			    &devices[i].devoff,
			    &devices[i].mdata_size,
			    &devices[i].erase_size);

		if (rc < 3) {
			free(devname);
			continue;
		}

		if (rc < 4)
			devices[i].erase_size = devices[i].mdata_size;

		devices[i].devname = devname;
		i++;
	}

	free(line);
	fclose(fp);

	if (i == 2) {
		have_redundant = true;
		if (devices[0].mdata_size != devices[1].mdata_size) {
			fprintf(stderr,
				"Size mismatch between the two metadata\n");
			return -EINVAL;
		}
	}

	if (!i) {
		fprintf(stderr,
			"Can't read config %s content\n", fname);
		return -EINVAL;
	}

	return 0;
}

static int find_parse_config(void)
{
	int i;

	if (config_file)
		return parse_config(config_file);

	for (i = 0; default_config_files[i]; i++) {
		int ret;

		ret = parse_config(default_config_files[i]);
		if (ret == -ENOENT)
			continue;
		if (ret)
			return ret;

		config_file = default_config_files[i];
		return 0;
	}

	fprintf(stderr, "Error: Cannot find config file\n");
	return -ENOENT;
}

static int open_device(struct fwumdata_device *dev)
{
	if (strstr(dev->devname, "/dev/mtd"))
		dev->is_mtd = true;

	dev->fd = open(dev->devname, O_RDWR | O_SYNC);
	if (dev->fd < 0) {
		fprintf(stderr, "Cannot open %s: %s\n", dev->devname,
			strerror(errno));
		return -ENODEV;
	}

	return 0;
}

static int mtd_erase(int fd, unsigned long offset, unsigned long size)
{
	struct erase_info_user erase;
	int ret;

	erase.start = offset;
	erase.length = size;

	ret = ioctl(fd, MEMERASE, &erase);
	if (ret < 0) {
		fprintf(stderr, "MTD erase failed: %s\n", strerror(errno));
		return -errno;
	}

	return 0;
}

static int read_device(struct fwumdata_device *dev, void *buf, size_t count)
{
	if (lseek(dev->fd, dev->devoff, SEEK_SET) < 0) {
		fprintf(stderr, "Seek failed: %s\n", strerror(errno));
		return -errno;
	}

	if (read(dev->fd, buf, count) < 0) {
		fprintf(stderr, "Read failed: %s\n", strerror(errno));
		return -errno;
	}

	return 0;
}

static int write_device(struct fwumdata_device *dev, const void *buf,
			size_t count)
{
	int ret;

	/* Erase if MTD device */
	if (dev->is_mtd) {
		ret = mtd_erase(dev->fd, dev->devoff, dev->erase_size);
		if (ret)
			return ret;
	}

	if (lseek(dev->fd, dev->devoff, SEEK_SET) < 0) {
		fprintf(stderr, "Seek failed: %s\n", strerror(errno));
		return -errno;
	}

	if (write(dev->fd, buf, count) < 0) {
		fprintf(stderr, "Write failed: %s\n", strerror(errno));
		return -errno;
	}

	return 0;
}

/* Metadata operations */
static int validate_crc(struct fwu_mdata *mdata, size_t size)
{
	u32 calc_crc, stored_crc;

	stored_crc = mdata->crc32;
	calc_crc = crc32(0, (const u8 *)&mdata->version, size - sizeof(u32));

	if (calc_crc != stored_crc) {
		fprintf(stderr,
			"CRC mismatch: calculated 0x%08x, stored 0x%08x\n",
			calc_crc, stored_crc);
		if (mdata->version == 1)
			fprintf(stderr,
				"Metadata is V1, this may be size description issue\n");
		return -1;
	}

	return 0;
}

static void update_crc(struct fwu_mdata *mdata, size_t size)
{
	mdata->crc32 = crc32(0, (const u8 *)&mdata->version, size - sizeof(u32));
}

static int read_one_metadata(int mdata_id, size_t size)
{
	int ret;

	ret = open_device(&devices[mdata_id]);
	if (ret)
		return ret;

	ret = read_device(&devices[mdata_id], &mdata[mdata_id], size);
	if (ret)
		return ret;

	if (mdata[mdata_id].version != 1 && mdata[mdata_id].version != 2) {
		fprintf(stderr, "Invalid metadata %d version: %u\n",
			mdata_id, mdata[mdata_id].version);
	}

	return 0;
}

static int read_metadata(bool update)
{
	size_t alloc_size;
	int ret;

	/* Allocate initial buffer */
	alloc_size = devices[0].mdata_size;
	mdata = calloc(have_redundant ? 2 : 1, alloc_size);
	if (!mdata) {
		fprintf(stderr, "Memory allocation failed\n");
		return -ENOMEM;
	}

	ret = read_one_metadata(0, alloc_size);
	if (ret)
		return ret;

	if (validate_crc(&mdata[0], alloc_size) < 0) {
		fprintf(stderr,
			"Warning: Primary metadata CRC validation failed\n");
		mdata_mod = update;
	} else {
		valid_mdata = &mdata[0];
	}

	if (have_redundant) {
		ret = read_one_metadata(1, alloc_size);
		if (ret)
			return ret;

		if (validate_crc(&mdata[1], alloc_size) < 0) {
			fprintf(stderr,
				"Warning: Secondary metadata CRC validation failed\n");
			mdata_mod = update;
		} else if (valid_mdata && mdata[0].crc32 != mdata[1].crc32) {
			fprintf(stderr,
				"Metadatas valid but not equal, use first one as default\n");
			mdata_mod = update;
		} else {
			valid_mdata = &mdata[1];
		}
	}

	if (!valid_mdata) {
		fprintf(stderr,
			"No metadata valid, use first one as default\n");
		mdata_mod = update;
		valid_mdata = &mdata[0];
	}

	if (valid_mdata->version == 2) {
		struct fwu_mdata_ext *mdata_ext;

		mdata_ext = fwu_get_fw_mdata_ext(valid_mdata);
		if (mdata_ext->metadata_size != alloc_size) {
			fprintf(stderr,
				"Metadata real size 0x%x mismatch with the config 0x%zx\n",
				mdata_ext->metadata_size, alloc_size);
				return -EINVAL;
		}
	}

	return 0;
}

static int write_metadata(void)
{
	size_t write_size = devices[0].mdata_size;
	int ret;

	if (!mdata_mod)
		return 0;

	/* Update CRC */
	update_crc(valid_mdata, write_size);

	/* Write primary */
	ret = write_device(&devices[0], valid_mdata, write_size);
	if (ret < 0) {
		fprintf(stderr, "Failed to write primary metadata\n");
		return ret;
	}

	/* Write secondary if redundant */
	if (have_redundant) {
		ret = write_device(&devices[1], valid_mdata, write_size);
		if (ret < 0) {
			fprintf(stderr, "Failed to write secondary metadata\n");
			return -1;
		}
	}

	printf("FWU metadata updated successfully\n");
	mdata_mod = 0;

	return 0;
}

/* Display functions */
static const char *bank_state_to_string(u8 state)
{
	switch (state) {
	case FWU_BANK_ACCEPTED:
		return "accepted";
	case FWU_BANK_VALID:
		return "valid";
	case FWU_BANK_INVALID:
		return "invalid";
	default:
		return "unknown";
	}
}

static void print_metadata_summary(void)
{
	int i;

	printf("FWU Metadata:\n");
	printf("\tVersion:            %u\n", valid_mdata->version);
	printf("\tActive Index:       %u\n", valid_mdata->active_index);
	printf("\tPrevious Index:     %u\n", valid_mdata->previous_active_index);
	printf("\tCRC32:              0x%08x\n", valid_mdata->crc32);

	if (valid_mdata->version == 2) {
		struct fwu_fw_store_desc *fw_desc;
		struct fwu_mdata_ext *mdata_ext;

		mdata_ext = fwu_get_fw_mdata_ext(valid_mdata);
		printf("\tMetadata Size:      %u bytes\n", mdata_ext->metadata_size);
		printf("\tDescriptor Offset:  %u\n", mdata_ext->desc_offset);
		printf("\tBank States:\n");

		fw_desc = fwu_get_fw_desc(valid_mdata);
		for (i = 0; i < fw_desc->num_banks && i < MAX_BANKS_V2; i++) {
			printf("\t\tBank %d: %s (0x%02x)\n", i,
			       bank_state_to_string(mdata_ext->bank_state[i]),
			       mdata_ext->bank_state[i]);
		}
	}
}

static void print_metadata_detailed(void)
{
	struct fwu_fw_store_desc *fw_desc = NULL;
	struct fwu_image_bank_info *bank_info;
	struct fwu_image_entry *img_entry;
	int num_images, num_banks;
	char guid_str[64];
	int i, j;

	print_metadata_summary();

	if (valid_mdata->version == 1) {
		num_images = nimages;
		num_banks = nbanks;
	} else {
		fw_desc = fwu_get_fw_desc(valid_mdata);
		num_images = fw_desc->num_images;
		num_banks = fw_desc->num_banks;
	}

	if (fw_desc) {
		printf("\n\tFirmware Store Descriptor:\n");
		printf("\t\tNumber of Banks:       %u\n", num_banks);
		printf("\t\tNumber of Images:      %u\n", num_images);
		printf("\t\tImage Entry Size:      %u\n", fw_desc->img_entry_size);
		printf("\t\tBank Info Entry Size:  %u\n", fw_desc->bank_info_entry_size);
	}

	printf("\n\tImages:\n");
	for (i = 0; i < num_images; i++) {
		img_entry = fwu_get_image_entry(valid_mdata, valid_mdata->version,
						num_banks, i);

		printf("\t\tImage %d:\n", i);

		guid_to_string(&img_entry->image_type_guid, guid_str);
		printf("\t\t\tImage Type GUID:  %s\n", guid_str);

		guid_to_string(&img_entry->location_guid, guid_str);
		printf("\t\t\tLocation GUID:    %s\n", guid_str);

		printf("\t\t\tBanks:\n");
		for (j = 0; j < num_banks; j++) {
			bank_info = fwu_get_bank_info(valid_mdata,
						      valid_mdata->version,
						      num_banks, i, j);

			guid_to_string(&bank_info->image_guid, guid_str);
			printf("\t\t\t\tBank %d:\n", j);
			printf("\t\t\t\t\tImage GUID:  %s\n", guid_str);
			printf("\t\t\t\t\tAccepted:    %s (%u)\n",
			       (bank_info->accepted & FWU_IMAGE_ACCEPTED) ? "yes" : "no",
			       bank_info->accepted);
		}
	}
}

/* Modification functions */
static int set_active_index(int bank)
{
	struct fwu_fw_store_desc *fw_desc;
	int num_banks;

	if (valid_mdata->version == 2) {
		fw_desc = fwu_get_fw_desc(valid_mdata);
		num_banks = fw_desc->num_banks;
	} else {
		num_banks = nbanks;
	}

	if (bank < 0 || bank >= num_banks) {
		fprintf(stderr, "Error: Invalid bank %d (must be 0-%d)\n",
			bank, num_banks - 1);
		return -EINVAL;
	}

	if (valid_mdata->active_index == bank)
		return 0;

	valid_mdata->active_index = bank;
	mdata_mod = 1;

	printf("Active bank set to %d\n", bank);
	return 0;
}

static int set_previous_index(int bank)
{
	struct fwu_fw_store_desc *fw_desc;
	int num_banks;

	if (valid_mdata->version == 2) {
		fw_desc = fwu_get_fw_desc(valid_mdata);
		num_banks = fw_desc->num_banks;
	} else {
		num_banks = nbanks;
	}

	if (bank < 0 || bank >= num_banks) {
		fprintf(stderr, "Error: Invalid bank %d (must be 0-%d)\n",
			bank, num_banks - 1);
		return -EINVAL;
	}

	if (valid_mdata->previous_active_index == bank)
		return 0;

	valid_mdata->previous_active_index = bank;
	mdata_mod = 1;

	printf("Previous bank set to %d\n", bank);
	return 0;
}

static int set_image_accepted(int image, int bank, int accept)
{
	struct fwu_image_bank_info *bank_info;
	int num_images, num_banks;

	if (valid_mdata->version == 1) {
		num_images = nimages;
		num_banks = nbanks;
	} else {
		struct fwu_fw_store_desc *fw_desc;

		fw_desc = fwu_get_fw_desc(valid_mdata);
		num_images = fw_desc->num_images;
		num_banks = fw_desc->num_banks;
	}

	if (bank < 0 || bank >= num_banks) {
		fprintf(stderr, "Error: Invalid bank %d (must be 0-%d)\n",
			bank, num_banks - 1);
		return -EINVAL;
	}

	if (image < 0 || image >= num_images) {
		fprintf(stderr, "Error: Invalid image %d (must be 0-%d)\n",
			image, num_images - 1);
		return -EINVAL;
	}

	bank_info = fwu_get_bank_info(valid_mdata, valid_mdata->version,
				      num_banks, image, bank);
	if (accept == bank_info->accepted)
		return 0;

	if (accept) {
		bank_info->accepted = FWU_IMAGE_ACCEPTED;
	} else {
		bank_info->accepted = 0;

		/* According to the spec: bank_state[index] have to be set
		 * to invalid before any content in the img_bank_info[index]
		 * is overwritten.
		 */
		if (valid_mdata->version == 2) {
			struct fwu_mdata_ext *mdata_ext;

			mdata_ext = fwu_get_fw_mdata_ext(valid_mdata);
			mdata_ext->bank_state[bank] = FWU_BANK_INVALID;
		}
	}

	mdata_mod = 1;
	printf("Image %d in bank %d: acceptance %s\n",
	       image, bank, accept ? "set" : "cleared");

	return 0;
}

static int set_bank_state(int bank, const char *state_str)
{
	struct fwu_fw_store_desc *fw_desc;
	struct fwu_mdata_ext *mdata_ext;
	u8 state;
	int i;

	if (valid_mdata->version != 2) {
		fprintf(stderr,
			"Error: Bank state is only supported in V2 metadata\n");
		return -EINVAL;
	}

	fw_desc = fwu_get_fw_desc(valid_mdata);
	mdata_ext = fwu_get_fw_mdata_ext(valid_mdata);

	if (bank < 0 || bank >= fw_desc->num_banks || bank >= MAX_BANKS_V2) {
		fprintf(stderr, "Error: Invalid bank %d (must be 0-%d)\n",
			bank, fw_desc->num_banks - 1);
		return -EINVAL;
	}

	/* Parse state string */
	if (!strcmp(state_str, "accepted")) {
		state = FWU_BANK_ACCEPTED;
	} else if (!strcmp(state_str, "valid")) {
		state = FWU_BANK_VALID;
	} else if (!strcmp(state_str, "invalid")) {
		state = FWU_BANK_INVALID;
	} else {
		fprintf(stderr,
			"Error: Invalid state '%s' (must be accepted/valid/invalid)\n",
			state_str);
		return -EINVAL;
	}

	if (mdata_ext->bank_state[bank] == state)
		return 0;

	/* If a bank is set in a accepted state all firmware images in
	 * that bank must be marked as accepted as described in the spec.
	 */
	if (state == FWU_BANK_ACCEPTED) {
		for (i = 0; i < fw_desc->num_images; i++) {
			int ret;

			ret = set_image_accepted(i, bank, true);
			if (ret)
				return ret;
		}
	}
	mdata_ext->bank_state[bank] = state;
	mdata_mod = 1;

	printf("Bank %d state set to %s (0x%02x)\n", bank, state_str, state);
	return 0;
}

static int metadata_v1_validate_size(void)
{
	int calc_size;

	calc_size = sizeof(struct fwu_mdata) +
		    (sizeof(struct fwu_image_entry) +
		     sizeof(struct fwu_image_bank_info) * nbanks) * nimages;

	if (devices[0].mdata_size != calc_size) {
		fprintf(stderr,
			"Metadata calculate size (-B and -I options) 0x%x mismatch with the config 0x%zx\n",
			calc_size, devices[0].mdata_size);
		return -EINVAL;
	}

	return 0;
}

/* Command-line interface */
static void print_usage(void)
{
	fprintf(stderr, "Usage: fwumdata [options]\n\n");
	fprintf(stderr, "Options:\n"
		"\t-c, --config <file>         Use custom config file, defaults:\n"
		"\t                              ./fwumdata.config or /etc/fwumdata.config\n"
		"\t-l, --list                  List detailed metadata with GUIDs\n"
		"\t-a, --active <bank>         Set active bank index\n"
		"\t-p, --previous <bank>       Set previous bank index\n"
		"\t-s, --state <bank> <state>  Set bank state (V2 only)\n"
		"\t                              state: accepted|valid|invalid\n"
		"\t-i, --image <id>            Image number (for -A/-C)\n"
		"\t-b, --bank <bank>           Bank number (for -A/-C)\n"
		"\t-A, --accept                Accept image (requires -i and -b)\n"
		"\t-C, --clear                 Clear image acceptance (requires -i and -b)\n"
		"\t-u, --update                Update metadata if there is a checksum issue\n"
		"\t-B, --nbanks <num_banks>    Number of banks (required for V1 metadata)\n"
		"\t-I, --nimages <num_images>  Number of images (required for V1 metadata)\n"
		"\t-h, --help                  Print this help\n\n");
	fprintf(stderr, "Config file format (fwumdata.config):\n"
		"\t# Device Name      Device Offset    Metadata Size    Erase Size\n"
		"\t/dev/mtd0          0x0              0x78             0x1000\n"
		"\t/dev/mtd1          0x0              0x78             0x1000\n\n");
	fprintf(stderr, "Examples:\n"
		"\tfwumdata                              # Print metadata summary\n"
		"\tfwumdata -l                           # Print detailed metadata\n"
		"\tfwumdata -a 1                         # Set active bank to 1\n"
		"\tfwumdata -s 1 accepted                # Set bank 1 to accepted state\n"
		"\tfwumdata -i 0 -b 0 -A                 # Accept image in bank 0\n"
		"\tfwumdata -B 2 -I 2 -i 1 -b 1 -A -l    # Accept image 1 in bank 1 with metadata V1\n");
}

int main(int argc, char *argv[])
{
	char *bank_state_str = NULL;
	bool list_detailed = false;
	int bank_state_num = -1;
	int active_index = -1;
	int bank_id = -1;
	int prev_index = -1;
	bool do_accept = 0;
	bool do_clear = 0;
	bool do_update = 0;
	int image_id = -1;
	int ret = 0;
	int opt;

	static struct option long_options[] = {
		{"config", required_argument, 0, 'c'},
		{"list", no_argument, 0, 'l'},
		{"active", required_argument, 0, 'a'},
		{"previous", required_argument, 0, 'p'},
		{"state", required_argument, 0, 's'},
		{"image", required_argument, 0, 'i'},
		{"bank", required_argument, 0, 'b'},
		{"accept", no_argument, 0, 'A'},
		{"clear", no_argument, 0, 'C'},
		{"update", no_argument, 0, 'u'},
		{"nbanks", required_argument, 0, 'B'},
		{"nimages", required_argument, 0, 'I'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	/* Parse arguments */
	while ((opt = getopt_long(argc, argv, "c:la:p:s:i:b:ACuB:I:h", long_options, NULL)) != -1) {
		switch (opt) {
		case 'c':
			config_file = optarg;
			break;
		case 'l':
			list_detailed = 1;
			break;
		case 'a':
			active_index = atoi(optarg);
			break;
		case 'p':
			prev_index = atoi(optarg);
			break;
		case 's':
			bank_state_num = atoi(optarg);
			if (optind < argc && argv[optind][0] != '-') {
				bank_state_str = argv[optind++];
			} else {
				fprintf(stderr,
					"Error: -s requires bank number and state\n");
				return 1;
			}
			break;
		case 'i':
			image_id = atoi(optarg);
			break;
		case 'b':
			bank_id = atoi(optarg);
			break;
		case 'A':
			do_accept = 1;
			break;
		case 'C':
			do_clear = 1;
			break;
		case 'u':
			do_update = 1;
			break;
		case 'B':
			nbanks = atoi(optarg);
			break;
		case 'I':
			nimages = atoi(optarg);
			break;
		case 'h':
			print_usage();
			return 0;
		default:
			print_usage();
			return 1;
		}
	}

	ret = find_parse_config();
	if (ret < 0) {
		fprintf(stderr, "Error: Cannot read configuration\n");
		return ret;
	}

	ret = read_metadata(do_update);
	if (ret < 0) {
		fprintf(stderr, "Error: Cannot read metadata\n");
		goto cleanup;
	}

	if (valid_mdata->version == 1) {
		ret = metadata_v1_validate_size();
		if (ret)
			goto cleanup;
	}

	/* Perform operations */
	if (active_index >= 0) {
		ret = set_active_index(active_index);
		if (ret < 0)
			goto cleanup;
	}

	if (prev_index >= 0) {
		ret = set_previous_index(prev_index);
		if (ret < 0)
			goto cleanup;
	}

	if (do_accept || do_clear) {
		if (image_id < 0 || bank_id < 0) {
			fprintf(stderr,
				"Error: -A/-C requires both -i <guid> and -b <bank>\n");
			ret = -EINVAL;
			goto cleanup;
		}

		ret = set_image_accepted(image_id, bank_id, do_accept);
		if (ret < 0)
			goto cleanup;
	}

	if (bank_state_num >= 0 && bank_state_str) {
		ret = set_bank_state(bank_state_num, bank_state_str);
		if (ret < 0)
			goto cleanup;
	}

	/* Write back if modified */
	if (mdata_mod) {
		ret = write_metadata();
		if (ret)
			goto cleanup;
	}

	/* Display metadata if no modifications or list requested */
	if (list_detailed)
		print_metadata_detailed();
	else
		print_metadata_summary();

cleanup:
	/* Close devices and free memory */
	if (devices[0].fd)
		close(devices[0].fd);
	if (devices[1].fd)
		close(devices[1].fd);

	free(mdata);

	for (int i = 0; i < 2; i++) {
		if (devices[i].devname)
			free((void *)devices[i].devname);
	}

	return ret;
}
