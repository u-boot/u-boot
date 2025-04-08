// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#include <dm/device.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <u-boot/crc.h>
#include <malloc.h>
#include <extension_board.h>

#include "phytec_som_detection.h"

struct phytec_eeprom_data eeprom_data;

#if IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION)

int phytec_eeprom_data_setup_fallback(struct phytec_eeprom_data *data,
				      int bus_num, int addr, int addr_fallback)
{
	int ret;

	ret = phytec_eeprom_data_init(data, bus_num, addr);
	if (ret) {
		pr_err("%s: init failed. Trying fall back address 0x%x\n",
		       __func__, addr_fallback);
		ret = phytec_eeprom_data_init(data, bus_num, addr_fallback);
	}

	if (ret)
		pr_err("%s: EEPROM data init failed\n", __func__);

	return ret;
}

int phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
			     int bus_num, int addr)
{
	int ret;

	ret = phytec_eeprom_data_init(data, bus_num, addr);
	if (ret)
		pr_err("%s: EEPROM data init failed\n", __func__);

	return ret;
}

int phytec_eeprom_read(u8 *data, int bus_num, int addr, int size, int offset)
{
	int ret;

#if CONFIG_IS_ENABLED(DM_I2C)
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, addr, 2, &dev);
	if (ret) {
		pr_err("%s: i2c EEPROM not found: %i.\n", __func__, ret);
		return ret;
	}

	ret = dm_i2c_read(dev, offset, (uint8_t *)data, size);
	if (ret) {
		pr_err("%s: Unable to read EEPROM data: %i\n", __func__, ret);
		return ret;
	}
#else
	i2c_set_bus_num(bus_num);
	ret = i2c_read(addr, offset, 2, (uint8_t *)data, size);
#endif
	return ret;
}

int phytec_eeprom_data_init_v2(struct phytec_eeprom_data *data)
{
	unsigned int crc;

	if (!data)
		return -1;

	crc = crc8(0, (const unsigned char *)&data->payload, PHYTEC_API2_DATA_LEN);
	debug("%s: crc: %x\n", __func__, crc);

	if (crc) {
		pr_err("%s: CRC mismatch. EEPROM data is not usable.\n",
		       __func__);
		return -EINVAL;
	}

	return 0;
}

#if IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION_BLOCKS)

int phytec_eeprom_data_init_v3_block(struct phytec_eeprom_data *data,
				     struct phytec_api3_block_header *header,
				     u8 *payload)
{
	struct phytec_api3_element *element = NULL;
	struct phytec_api3_element *list_iterator;

	if (!header)
		return -1;
	if (!payload)
		return -1;

	debug("%s: block type: %i\n", __func__, header->block_type);
	switch (header->block_type) {
	case PHYTEC_API3_BLOCK_MAC:
		element = phytec_blocks_init_mac(header, payload);
		break;
	default:
		debug("%s: Unknown block type %i\n", __func__,
		      header->block_type);
	}
	if (!element)
		return -1;

	if (!data->payload.block_head) {
		data->payload.block_head = element;
		return 0;
	}

	list_iterator = data->payload.block_head;
	while (list_iterator && list_iterator->next)
		list_iterator = list_iterator->next;
	list_iterator->next = element;

	return 0;
}

int phytec_eeprom_data_init_v3(struct phytec_eeprom_data *data,
			       int bus_num, int addr)
{
	int ret, i;
	struct phytec_api3_header header;
	unsigned int crc;
	u8 *payload;
	int block_addr;
	struct phytec_api3_block_header *block_header;

	if (!data)
		return -1;

	ret = phytec_eeprom_read((uint8_t *)&header, bus_num, addr,
				 PHYTEC_API3_DATA_HEADER_LEN,
				 PHYTEC_API2_DATA_LEN);
	if (ret) {
		pr_err("%s: Failed to read API v3 data header.\n", __func__);
		goto err;
	}

	crc = crc8(0, (const unsigned char *)&header,
		   PHYTEC_API3_DATA_HEADER_LEN);
	debug("%s: crc: %x\n", __func__, crc);
	if (crc) {
		pr_err("%s: CRC mismatch. API3 header is unusable.\n",
		       __func__);
		goto err;
	}

	debug("%s: data length: %i\n", __func__, header.data_length);
	payload = malloc(header.data_length);
	if (!payload) {
		pr_err("%s: Unable to allocate memory\n", __func__);
		goto err_payload;
	}

	ret = phytec_eeprom_read(payload, bus_num, addr, header.data_length,
				 PHYTEC_API3_DATA_HEADER_LEN +
				 PHYTEC_API2_DATA_LEN);
	if (ret) {
		pr_err("%s: Failed to read API v3 data payload.\n", __func__);
		goto err_payload;
	}

	block_addr = 0;
	debug("%s: block count: %i\n", __func__, header.block_count);
	for (i = 0; i < header.block_count; i++) {
		debug("%s: block_addr: %i\n", __func__, block_addr);
		block_header = (struct phytec_api3_block_header *)
			&payload[block_addr];
		crc = crc8(0, (const unsigned char *)block_header,
			   PHYTEC_API3_BLOCK_HEADER_LEN);

		debug("%s: crc: %x\n", __func__, crc);
		if (crc) {
			pr_err("%s: CRC mismatch. API3 block header is unusable\n",
			       __func__);
			goto err_payload;
		}

		ret = phytec_eeprom_data_init_v3_block(data, block_header,
			&payload[block_addr + PHYTEC_API3_BLOCK_HEADER_LEN]);
		/* Ignore failed block initialization and continue. */
		if (ret)
			debug("%s: Unable to create block with index %i.\n",
			      __func__, i);

		block_addr = block_header->next_block;
	}

	free(payload);
	return 0;
err_payload:
	free(payload);
err:
	return -1;
}

#else

inline int phytec_eeprom_data_init_v3(struct phytec_eeprom_data *data,
				      int bus_num, int addr)
{
	return 0;
}

#endif

int phytec_eeprom_data_init(struct phytec_eeprom_data *data,
			    int bus_num, int addr)
{
	int ret, i;
	u8 *ptr;

	if (!data)
		data = &eeprom_data;

	ret = phytec_eeprom_read((u8 *)data, bus_num, addr,
				 PHYTEC_API2_DATA_LEN, 0);
	if (ret)
		goto err;
	data->payload.block_head = NULL;

	if (data->payload.api_rev == 0xff) {
		pr_err("%s: EEPROM is not flashed. Prototype?\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	ptr = (u8 *)data;
	for (i = 0; i < PHYTEC_API2_DATA_LEN; ++i)
		if (ptr[i] != 0x0)
			break;

	if (i == PHYTEC_API2_DATA_LEN) {
		pr_err("%s: EEPROM data is all zero. Erased?\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	if (data->payload.api_rev >= PHYTEC_API_REV2) {
		ret = phytec_eeprom_data_init_v2(data);
		if (ret)
			goto err;
	}

	if (IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION_BLOCKS))
		if (data->payload.api_rev >= PHYTEC_API_REV3) {
			ret = phytec_eeprom_data_init_v3(data, bus_num, addr);
			if (ret)
				goto err;
		}

	data->valid = true;
	return 0;
err:
	data->valid = false;
	return ret;
}

static int phytec_get_product_name(struct phytec_eeprom_data *data,
				   char *product)
{
	struct phytec_api2_data *api2;
	unsigned int ksp_no, som_type;
	int len;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return -EINVAL;

	api2 = &data->payload.data.data_api2;

	if (api2->som_type > 1 && api2->som_type <= 3) {
		ksp_no = (api2->ksp_no << 8) | api2->som_no;
		len = snprintf(product, PHYTEC_PRODUCT_NAME_MAX_LEN + 1,
			       "%s-%04u", phytec_som_type_str[api2->som_type],
			       ksp_no);
		if (len != PHYTEC_PRODUCT_NAME_KSP_LEN)
			return -EINVAL;
		return 0;
	}

	switch (api2->som_type) {
	case 0:
	case 1:
	case 2:
	case 3:
		som_type = api2->som_type;
		break;
	case 4:
	case 5:
		som_type = 0;
		break;
	case 6:
	case 7:
		som_type = 1;
		break;
	default:
		pr_err("%s: Invalid SOM type: %i", __func__, api2->som_type);
		return -EINVAL;
	};

	len = snprintf(product, PHYTEC_PRODUCT_NAME_MAX_LEN + 1, "%s-%03u",
		       phytec_som_type_str[som_type], api2->som_no);
	if (len != PHYTEC_PRODUCT_NAME_STD_LEN)
		return -EINVAL;
	return 0;
}

static int phytec_get_part_number(struct phytec_eeprom_data *data,
				  char *part)
{
	char product_name[PHYTEC_PRODUCT_NAME_MAX_LEN + 1] = {'\0'};
	struct phytec_api2_data *api2;
	unsigned int ksp_type;
	int res, len;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return -EINVAL;

	api2 = &data->payload.data.data_api2;

	res = phytec_get_product_name(data, product_name);
	if (res)
		return res;

	if (api2->som_type <= 1) {
		len = snprintf(part, PHYTEC_PART_NUMBER_MAX_LEN + 1,
			       "%s-%s.%s", product_name, api2->opt,
			       api2->bom_rev);
		if (len < PHYTEC_PART_NUMBER_STD_LEN)
			return -EINVAL;
		return 0;
	}
	if (api2->som_type <= 3) {
		snprintf(part, PHYTEC_PART_NUMBER_MAX_LEN + 1, "%s.%s",
			 product_name, api2->bom_rev);
		if (len != PHYTEC_PART_NUMBER_KSP_LEN)
			return -EINVAL;
		return 0;
	}

	switch (api2->som_type) {
	case 4:
		ksp_type = 3;
		break;
	case 5:
		ksp_type = 2;
		break;
	case 6:
		ksp_type = 3;
		break;
	case 7:
		ksp_type = 2;
		break;
	default:
		pr_err("%s: Invalid SOM type: %i", __func__, api2->som_type);
		return -EINVAL;
	};

	len = snprintf(part, PHYTEC_PART_NUMBER_MAX_LEN + 1, "%s-%s%02u.%s",
		       product_name, phytec_som_type_str[ksp_type],
		       api2->ksp_no, api2->bom_rev);
	if (len < PHYTEC_PART_NUMBER_STD_KSP_LEN)
		return -EINVAL;

	return 0;
}

void __maybe_unused phytec_print_som_info(struct phytec_eeprom_data *data)
{
	char part_number[PHYTEC_PART_NUMBER_MAX_LEN + 1] = {'\0'};
	struct phytec_api2_data *api2;
	char pcb_sub_rev;
	int res;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return;

	api2 = &data->payload.data.data_api2;

	/* Calculate PCB subrevision */
	pcb_sub_rev = api2->pcb_sub_opt_rev & 0x0f;
	pcb_sub_rev = pcb_sub_rev ? ((pcb_sub_rev - 1) + 'a') : ' ';

	res = phytec_get_part_number(data, part_number);
	if (res)
		return;

	printf("SOM: %s\n", part_number);
	printf("PCB Rev.: %u%c\n", api2->pcb_rev, pcb_sub_rev);
	if (api2->som_type > 1)
		printf("Options: %s\n", api2->opt);
}

char * __maybe_unused phytec_get_opt(struct phytec_eeprom_data *data)
{
	char *opt;

	if (!data)
		data = &eeprom_data;

	if (!data->valid)
		return NULL;

	if (data->payload.api_rev < PHYTEC_API_REV2)
		opt = data->payload.data.data_api0.opt;
	else
		opt = data->payload.data.data_api2.opt;

	return opt;
}

u8 __maybe_unused phytec_get_rev(struct phytec_eeprom_data *data)
{
	struct phytec_api2_data *api2;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	api2 = &data->payload.data.data_api2;

	return api2->pcb_rev;
}

u8 __maybe_unused phytec_get_som_type(struct phytec_eeprom_data *data)
{
	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	return data->payload.data.data_api2.som_type;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT)
int phytec_ft_board_fixup(struct phytec_eeprom_data *data, void *blob)
{
	char product_name[PHYTEC_PRODUCT_NAME_MAX_LEN + 1] = {'\0'};
	char part_number[PHYTEC_PART_NUMBER_MAX_LEN + 1] = {'\0'};
	int res;

	if (!data)
		data = &eeprom_data;

	if (!data->valid || data->payload.api_rev < PHYTEC_API_REV2)
		return -EINVAL;

	res = phytec_get_product_name(data, product_name);
	if (res)
		return res;

	fdt_setprop(blob, 0, "phytec,som-product-name", product_name,
		    strlen(product_name) + 1);

	res = phytec_get_part_number(data, part_number);
	if (res)
		return res;

	fdt_setprop(blob, 0, "phytec,som-part-number", part_number,
		    strlen(part_number) + 1);

	return 0;
}
#endif /* IS_ENABLED(CONFIG_OF_LIBFDT) */

#if IS_ENABLED(CONFIG_CMD_EXTENSION)
struct extension *phytec_add_extension(const char *name, const char *overlay,
				       const char *other)
{
	struct extension *extension;

	if (strlen(overlay) > sizeof(extension->overlay)) {
		pr_err("Overlay name %s is longer than %lu.\n", overlay,
		       sizeof(extension->overlay));
		return NULL;
	}

	extension = calloc(1, sizeof(struct extension));
	snprintf(extension->name, sizeof(extension->name), name);
	snprintf(extension->overlay, sizeof(extension->overlay), overlay);
	snprintf(extension->other, sizeof(extension->other), other);
	snprintf(extension->owner, sizeof(extension->owner), "PHYTEC");

	return extension;
}
#endif /* IS_ENABLED(CONFIG_CMD_EXTENSION) */

struct phytec_api3_element *
	__maybe_unused phytec_get_block_head(struct phytec_eeprom_data *data)
{
	if (!data)
		data = &eeprom_data;
	if (!data->valid)
		return NULL;

	return data->payload.block_head;
}

#else

inline int phytec_eeprom_data_setup(struct phytec_eeprom_data *data,
				    int bus_num, int addr)
{
	return PHYTEC_EEPROM_INVAL;
}

inline int phytec_eeprom_data_setup_fallback(struct phytec_eeprom_data *data,
					     int bus_num, int addr,
					     int addr_fallback)
{
	return PHYTEC_EEPROM_INVAL;
}

inline int phytec_eeprom_data_init(struct phytec_eeprom_data *data,
				   int bus_num, int addr)
{
	return PHYTEC_EEPROM_INVAL;
}

inline void __maybe_unused phytec_print_som_info(struct phytec_eeprom_data *data)
{
}

inline char *__maybe_unused phytec_get_opt(struct phytec_eeprom_data *data)
{
	return NULL;
}

u8 __maybe_unused phytec_get_rev(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

u8 __maybe_unused phytec_get_som_type(struct phytec_eeprom_data *data)
{
	return PHYTEC_EEPROM_INVAL;
}

inline struct phytec_api3_element * __maybe_unused
	phytec_get_block_head(struct phytec_eeprom_data *data)
{
	return NULL;
}

#if IS_ENABLED(CONFIG_OF_LIBFDT)
inline int phytec_ft_board_fixup(struct phytec_eeprom_data *data, void *blob)
{
	return 0;
}
#endif /* IS_ENABLED(CONFIG_OF_LIBFDT) */
#if IS_ENABLED(CONFIG_CMD_EXTENSION)
inline struct extension *phytec_add_extension(const char *name,
					      const char *overlay,
					      const char *other)
{
	return NULL;
}
#endif /* IS_ENABLED(CONFIG_CMD_EXTENSION) */

#endif /* IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION) */
