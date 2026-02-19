// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025-2026, RISCStar Ltd.
 */

#include <asm/io.h>
#include <clk.h>
#include <clk-uclass.h>
#include <cpu_func.h>
#include <configs/k1.h>
#include <cpu_func.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <dt-bindings/pinctrl/k1-pinctrl.h>
#include <generated/ddr_fw_info.h>
#include <i2c.h>
#include <linux/delay.h>
#include <log.h>
#include <power/regulator.h>
#include <spl.h>
#include <tlv_eeprom.h>
#include "tlv_codes.h"

#define I2C_PIN_CONFIG(x)       ((x) | EDGE_NONE | PULL_UP | PAD_1V8_DS2)
#define I2C_BUF_SIZE		64

#define MFP_GPIO_84		0xd401e154
#define MFP_GPIO_85		0xd401e158

static void reset_early_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_RESET, 0, &dev);
	if (ret)
		panic("Fail to detect reset controller.\n");
}

static void i2c_early_init(void)
{
	struct udevice *bus;

	// eeprom: I2C2, pin group(GPIO_84, GPIO_85)
	writel(I2C_PIN_CONFIG(MUX_MODE4), (void __iomem *)MFP_GPIO_84);
	writel(I2C_PIN_CONFIG(MUX_MODE4), (void __iomem *)MFP_GPIO_85);
	udelay(100);
	uclass_first_device(UCLASS_I2C, &bus);
	while (bus) {
		uclass_next_device(&bus);
		if (!bus)
			break;
	}
}

int read_product_name(char *name, int size)
{
	u8 eeprom_data[TLV_TOTAL_LEN_MAX], *p;
	struct tlvinfo_header *tlv_hdr;
	struct tlvinfo_tlv *tlv_entry;
	int ret, i = 0;
	u32 entry_size;

	if (!name || size <= 0)
		return -EINVAL;
	ret = read_tlvinfo_tlv_eeprom(eeprom_data, &tlv_hdr,
				      &tlv_entry, i);
	if (ret)
		return ret;
	p = (u8 *)tlv_entry;
	for (i = 0; i < tlv_hdr->totallen; ) {
		if (tlv_entry->type == TLV_CODE_PRODUCT_NAME) {
			if (tlv_entry->length < size)
				size = tlv_entry->length;
			memset(name, 0, size);
			memcpy(name, &tlv_entry->value[0], size);
			return 0;
		}
		if (tlv_entry->type == TLV_CODE_CRC_32)
			return -ENOENT;
		entry_size = tlv_entry->length + sizeof(struct tlvinfo_tlv);
		i += entry_size;
		p += entry_size;
		tlv_entry = (struct tlvinfo_tlv *)p;
	}
	return -ENOENT;
}

static void clk_early_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_CLK, "clock-controller@d4090000", &dev);
	if (ret)
		panic("Fail to detect clock-controller@d4090000\n");
	ret = uclass_get_device_by_name(UCLASS_CLK, "system-controller@d4050000", &dev);
	if (ret)
		panic("Fail to detect system-controller@d4050000\n");
	ret = uclass_get_device_by_name(UCLASS_CLK, "system-controller@d4282800", &dev);
	if (ret)
		panic("Fail to detect system-controller@d4282800\n");
	ret = uclass_get_device_by_name(UCLASS_CLK, "system-controller@d4015000", &dev);
	if (ret)
		panic("Fail to detect system-controller@d4015000\n");

	if (device_active(dev))
		debug("clk: device is active\n");
	else
		debug("clk: device not active, probing...\n");
}

void serial_early_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_SERIAL, 0, &dev);
	if (ret)
		panic("Serial uclass init failed: %d\n", ret);
}

static void set_vdd_core(void)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname("vdd_core", &dev);
	if (ret)
		panic("Fail to detect vdd_core (%d)\n", ret);
	ret = regulator_set_enable(dev, true);
	if (ret)
		log_warning("Fail to enable vdd_core (%d)\n", ret);
	ret = regulator_get_value(dev);
	if (ret < 0)
		log_warning("Fail to read vdd_core (%d)\n", ret);
	log_info("vdd_core, value:%d\n", ret);
}

static void set_vdd_1v8(void)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname("vdd_1v8", &dev);
	if (ret)
		panic("Fail to detect vdd_1v8 (%d)\n", ret);
	ret = regulator_set_value(dev, 1800000);
	if (ret)
		log_warning("Fail to set vdd_1v8 as 1800000 (%d)\n", ret);
	ret = regulator_set_enable(dev, true);
	if (ret)
		log_warning("Fail to enable vdd_1v8 (%d)\n", ret);
	ret = regulator_get_value(dev);
	if (ret < 0)
		log_warning("Fail to read vdd_1v8 (%d)\n", ret);
	log_info("vdd_1v8, value:%d\n", ret);
}

static void set_vdd_mmc(void)
{
	struct udevice *dev;
	int ret;

	ret = regulator_get_by_platname("vdd_1v8_mmc", &dev);
	if (ret)
		panic("Fail to detect vdd_1v8_mmc (%d)\n", ret);
	ret = regulator_set_enable(dev, true);
	if (ret)
		log_warning("Fail to enable vdd_1v8_mmc (%d)\n", ret);
	ret = regulator_get_value(dev);
	if (ret < 0)
		log_warning("Fail to read vdd_1v8_mmc (%d)\n", ret);
	log_info("vdd_1v8_mmc, value:%d\n", ret);
}

void pmic_init(void)
{
	struct udevice *pmic_dev;
	int ret;

	ret = uclass_get_device(UCLASS_PMIC, 0, &pmic_dev);
	if (ret)
		panic("Fail to detect PMIC (%d)\n", ret);
	set_vdd_core();
	set_vdd_1v8();
	set_vdd_mmc();
}

/* Load DDR training firmware */
int init_ddr_firmware(void)
{
	void __iomem *src, *dst;
	unsigned long size;

	src = (void __iomem *)(CONFIG_SPL_TEXT_BASE +
			       CONFIG_SPL_DDR_FIRMWARE_OFFSET);
	dst = (void __iomem *)(DDR_TRAINING_DATA_BASE);
	memcpy(dst, src, DDR_FW_FILE_SIZE);
	size = round_up(DDR_FW_FILE_SIZE, 64);
	flush_dcache_range((u32)(u64)dst, (u32)(u64)dst + size);
	return 0;
}

void ddr_early_init(void)
{
	void __iomem *addr;

	init_ddr_firmware();
	addr = (void __iomem *)(CONFIG_SPL_TEXT_BASE +
				CONFIG_SPL_DDR_FIRMWARE_OFFSET);
	// verify DDR firmware header
	log_info("[0x%x]:0x%x, firmware size:%d\n",
		 (uint)(u64)addr, readl(addr), DDR_FW_FILE_SIZE);
}

void board_init_f(ulong dummy)
{
	u8 i2c_buf[I2C_BUF_SIZE];
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed:%d\n", ret);

	riscv_cpu_setup();

	reset_early_init();
	clk_early_init();
	serial_early_init();

	preloader_console_init();

	i2c_early_init();
	ret = read_product_name(i2c_buf, I2C_BUF_SIZE);
	if (ret)
		log_info("Fail to detect board:%d\n", ret);
	else
		log_info("Get board name:%s\n", (char *)i2c_buf);
	pmic_init();

	ddr_early_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NOR;
}

void spl_board_init(void)
{
}
