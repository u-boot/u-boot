// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2018 - 2019 Xilinx, Inc.
 *
 * Michal Simek <michal.simek@xilinx.com>
 */

#define DEBUG

#include <common.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <fru.h>

#ifndef CONFIG_SPL_BUILD

#define FRU_DATA_START	0
#define FRU_DATA_SIZE	256

#if defined(CONFIG_DTB_RESELECT)
static int get_fru(uint8_t *fru_space)
{
	struct udevice *dev;
	int ret, i, count;
	u32 *phandles;
	ofnode chosen_node;

	chosen_node = ofnode_path("/chosen");

	count = ofnode_count_phandle_with_args(chosen_node, "xlnx,eeprom",
					       NULL);
	/* If there is no node or count is 0 fail */
	if (count < 0 || !count) {
		debug("%s: Incorrect or missing xlnx,eeprom property %d\n",
		      __func__, count);
		return -EINVAL;
	}

	debug("%s: Found %d eeprom phandles\n", __func__, count);

	phandles = calloc(count, sizeof(*phandles));
	if (!phandles)
		return -ENOMEM;

	ret = ofnode_read_u32_array(chosen_node, "xlnx,eeprom",
				    phandles, count);
	if (ret) {
		debug("%s: Reading phandles failed\n", __func__);
		return ret;
	}

	/* Interate over phandles and try to read FRU data */
	for (i = 0; i < count; i++) {
		debug("%s: %d: Reading device phandle %x\n",
		      __func__, i, phandles[i]);
		/* Get device from phandle_id */
		ret = uclass_get_device_by_phandle_id(UCLASS_I2C_EEPROM,
						      phandles[i], &dev);
		if (ret) {
			debug("%s: %d: Get device phandle %x failed %d\n",
			      __func__, i, phandles[i], ret);
			continue;
		}

		/* Setup alen to 2 */
		ret = i2c_set_chip_offset_len(dev, 2);
		if (ret)
			break;

		/* Read data to fru_space */
		ret = dm_i2c_read(dev, FRU_DATA_START, fru_space,
				  FRU_DATA_SIZE);
		if (!ret) {
			debug("%s: %i: I2C FRU location %p\n",
			      __func__, i, fru_space);
			break;
		}

		debug("%s: %d: I2C FRU read failed\n", __func__, i);
	}

	free(phandles);
	return ret;
}

static int do_board_detect(void)
{
	int ret;
	void *fru_space;

	fru_space = calloc(FRU_DATA_SIZE, sizeof(u8));
	if (!fru_space) {
		debug("%s: Malloc failed\n", __func__);
		return -ENOMEM;
	}

	ret = get_fru(fru_space);
	if (!ret) {
		ret = fru_capture((ulong)fru_space);
		if (ret)
			debug("%s: FRU decoding failed\n", __func__);
		else
			debug("%s: Board detected via FRU\n", __func__);

	} else {
		debug("%s: Cannot read EEPROM\n", __func__);
	}

	free(fru_space);
	return ret;
}

int embedded_dtb_select(void)
{
	int ret;

	debug("%s: Start board detect\n", __func__);

	ret = do_board_detect();
	if (ret < 0)
		debug("%s: Board detection failed\n", __func__);

	fdtdec_setup();

	return 0;
}
#endif

int board_fit_config_name_match(const char *name)
{
	if (!fru_data.captured) {
		debug("%s: FRU data not captured\n", __func__);
		return -EINVAL;
	}

	debug("%s: checking name %s with %s\n", __func__, name,
	      fru_data.brd.product_name);

	if (!strcmp(name, (char *)fru_data.brd.product_name)) {
		debug("Board found finally\n");
		return 0;
	}

	return -EINVAL;
}
#endif

