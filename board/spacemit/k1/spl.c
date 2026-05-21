// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2025-2026, RISCstar Ltd.
 */

#include <asm/io.h>
#include <binman.h>
#include <binman_sym.h>
#include <clk.h>
#include <clk-uclass.h>
#include <cpu_func.h>
#include <configs/k1.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <i2c.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <log.h>
#include <power/regulator.h>
#include <spi_flash.h>
#include <spl.h>
#include <tlv_eeprom.h>
#include "tlv_codes.h"

/* boot mode configs */
#define BOOT_DEV_FLAG_REG	0xd4282d10
#define BOOT_PIN_SEL_REG	0xd4282c20

#define BOOT_STRAP_MODE_OFFSET	9
#define BOOT_STRAP_MODE_MASK	3
#define BOOT_STRAP_MODE_EMMC	0
#define BOOT_STRAP_MODE_SPI	1
#define BOOT_STRAP_MODE_NAND	2
#define BOOT_STRAP_MODE_SD	3

#define MUX_MODE4		4
#define EDGE_NONE		BIT(6)
#define PULL_UP			(6 << 13)       /* bit[15:13] 110 */
#define PAD_DS_MEDIUM		BIT(12)
#define PAD_1V8_DS2		PAD_DS_MEDIUM
#define I2C_PIN_CONFIG(x)       ((x) | EDGE_NONE | PULL_UP | PAD_1V8_DS2)
#define I2C_BUF_SIZE		64

#define MFP_GPIO_84		0xd401e154
#define MFP_GPIO_85		0xd401e158

#define DDR_FIRMWARE_BASE	0xc082d000

#define DDR_DEFAULT_CS_NUM      2
#define DDR_DEFAULT_TYPE        "LPDDR4X"
#define DDR_DEFAULT_TX_ODT      80
#define DDR_DEFAULT_DATA_RATE   2400

#define MAGIC_NUM		0xaa55aa55

typedef void (*puts_func_t)(const char *s);
typedef int (*ddr_init_func_t)(u64 ddr_base, u32 cs_num, u32 data_rate,
			       puts_func_t puts);

enum board_boot_mode {
	BOOT_MODE_NONE = 0,
	BOOT_MODE_USB = 0x55a,
	BOOT_MODE_EMMC,
	BOOT_MODE_NAND,
	BOOT_MODE_SPI,
	BOOT_MODE_SD,
	BOOT_MODE_SHELL = 0x55f,
	BOOT_MODE_BOOTSTRAP,
};

struct ddr_cfg {
	u32     data_rate;
	u32     cs_num;
	u32     tx_odt;
	u8      type[I2C_BUF_SIZE];
};

binman_sym_declare(ulong, ddr_fw, image_pos);
binman_sym_declare(ulong, ddr_fw, size);

char product_name[I2C_BUF_SIZE] = "k1";

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
		log_debug("clk: device is active\n");
	else
		log_debug("clk: device not active, probing...\n");
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
	ret = regulator_set_value(dev, 1800000);
	if (ret)
		log_warning("Fail to set vdd_1v8_mmc as 1800000 (%d)\n", ret);
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

/* Set default value for DDR chips */
static void ddr_cfg_init(struct ddr_cfg *cfg)
{
	memset(cfg, 0, sizeof(struct ddr_cfg));
	cfg->data_rate = DDR_DEFAULT_DATA_RATE;
	cfg->cs_num = DDR_DEFAULT_CS_NUM;
	cfg->tx_odt = DDR_DEFAULT_TX_ODT;
	strcpy(cfg->type, DDR_DEFAULT_TYPE);
}

int read_ddr_info(struct ddr_cfg *cfg)
{
	u8 eeprom_data[TLV_TOTAL_LEN_MAX], *p;
	struct tlvinfo_header *tlv_hdr;
	struct tlvinfo_tlv *tlv_entry;
	u32 size, entry_size;
	int ret, i;
	bool found = false;

	if (!cfg)
		return -EINVAL;
	ddr_cfg_init(cfg);
	ret = read_tlvinfo_tlv_eeprom(eeprom_data, &tlv_hdr,
				      &tlv_entry, i);
	if (ret)
		return ret;
	p = (u8 *)tlv_entry;
	for (i = 0; i < tlv_hdr->totallen; ) {
		switch (tlv_entry->type) {
		case TLV_CODE_DDR_CSNUM:
			memcpy(&cfg->cs_num, &tlv_entry->value[0], 1);
			found = true;
			break;
		case TLV_CODE_DDR_TYPE:
			size = min((u32)tlv_entry->length, (u32)I2C_BUF_SIZE);
			memcpy(&cfg->type[0], &tlv_entry->value[0], size);
			found = true;
			break;
		case TLV_CODE_DDR_DATARATE:
			memcpy(&cfg->data_rate, &tlv_entry->value[0], 2);
			found = true;
			break;
		case TLV_CODE_DDR_TX_ODT:
			memcpy(&cfg->tx_odt, &tlv_entry->value[0], 1);
			found = true;
			break;
		case TLV_CODE_CRC_32:
			if (!found)
				return -ENOENT;
			return 0;
		}
		entry_size = tlv_entry->length + sizeof(struct tlvinfo_tlv);
		i += entry_size;
		p += entry_size;
		tlv_entry = (struct tlvinfo_tlv *)p;
	}
	if (!found)
		return -ENOENT;
	return 0;
}

void ddr_early_init(void)
{
	void __iomem *src, *dst;
	ulong pos, size;
	struct ddr_cfg cfg;
	ddr_init_func_t ddr_init;

	pos = binman_sym(ulong, ddr_fw, image_pos);
	size = binman_sym(ulong, ddr_fw, size);
	src = (void __iomem *)pos;
	dst = (void __iomem *)(DDR_FIRMWARE_BASE);
	log_info("DDR firmware: [0x%lx]:0x%x, size:0x%lx\n", pos, readl(src), size);
	memcpy((u8 *)dst, (u8 *)src, size);
	size = round_up(size, 64);
	/*
	 * Ensure the just-written DDR firmware bytes are observable in the
	 * instruction stream. RISC-V's fence.i alone is sufficient for the
	 * single-hart SPL case; skip flush_dcache_range as cbo.flush may
	 * trap on our current privilege state.
	 */
	asm volatile ("fence.i" ::: "memory");

	read_ddr_info(&cfg);
	ddr_init = (ddr_init_func_t)DDR_FIRMWARE_BASE;
#ifdef DEBUG
	ddr_init(0xc0000000, cfg.cs_num, cfg.data_rate, puts);
#else
	ddr_init(0xc0000000, cfg.cs_num, cfg.data_rate, NULL);
#endif
	writel(MAGIC_NUM, (void __iomem *)0x00000000);
	flush_dcache_range(0, 64);
	invalidate_dcache_range(0, 64);
	if (readl((void __iomem *)0x00000000) == MAGIC_NUM)
		log_info("DDR is ready\n");
	else
		log_info("DDR is not ready\n");
}

void nor_early_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_SPI, 0, &dev);
	if (ret)
		panic("Fail to detect spi controller.\n");
	udelay(10);
	ret = uclass_get_device(UCLASS_SPI_FLASH, 0, &dev);
	if (ret)
		log_info("Fail to detect spi nor flash.\n");
	udelay(10);
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		panic("spl_early_init() failed:%d\n", ret);

	riscv_cpu_setup();

	clk_early_init();
	serial_early_init();

	preloader_console_init();

	i2c_early_init();
	ret = read_product_name(product_name, I2C_BUF_SIZE);
	if (ret)
		log_info("Fail to detect board:%d\n", ret);
	else
		log_info("Get board name:%s\n", product_name);
	pmic_init();

	ddr_early_init();
	nor_early_init();
}

u32 spl_boot_device(void)
{
	void __iomem *boot_dev = (void __iomem *)BOOT_DEV_FLAG_REG;
	void __iomem *boot_strap = (void __iomem *)BOOT_PIN_SEL_REG;
	u32 mode, sel, ret = 0;

	mode = readl(boot_dev);
	if (mode == BOOT_MODE_NONE || mode > BOOT_MODE_SD) {
		sel = readl(boot_strap);
		sel >>= BOOT_STRAP_MODE_OFFSET;
		sel &= BOOT_STRAP_MODE_MASK;
		switch (sel) {
		case BOOT_STRAP_MODE_EMMC:
			mode = BOOT_MODE_EMMC;
			break;
		case BOOT_STRAP_MODE_NAND:
			mode = BOOT_MODE_NAND;
			break;
		case BOOT_STRAP_MODE_SPI:
			mode = BOOT_MODE_SPI;
			break;
		case BOOT_STRAP_MODE_SD:
		default:
			mode = BOOT_MODE_SD;
			break;
		}
	}
	/* TODO:
	 *   The current upstream DTS file only contains the eMMC node. When
	 *   the SD node is added via an overlay, the eMMC device ends up as
	 *   MMC1 in SPL.
	 *   However, the SD device should be the first device (MMC1).
	 *   This sequence needs to be corrected once the SD node is merged
	 *   into the upstream U-Boot DTS file.
	 */
	switch (mode) {
	case BOOT_MODE_EMMC:
		ret = BOOT_DEVICE_MMC1;
		break;
	case BOOT_MODE_NAND:
		ret = BOOT_DEVICE_NAND;
		break;
	case BOOT_MODE_SPI:
		ret = BOOT_DEVICE_SPI;
		break;
	case BOOT_MODE_USB:
		ret = BOOT_DEVICE_USB;
		break;
	case BOOT_MODE_SD:
		ret = BOOT_DEVICE_MMC2;
		break;
	default:
		ret = BOOT_DEVICE_MMC1;
		break;
	}
	return ret;
}

void spl_board_init(void)
{
}

int board_fit_config_name_match(const char *name)
{
	char fdt_name[I2C_BUF_SIZE];
	int i;

	memset(fdt_name, 0, I2C_BUF_SIZE);
	if (!strncmp(product_name, "k1-x_MUSE-Pi-Pro", 16))
		snprintf(fdt_name, I2C_BUF_SIZE, "%s",
			 "spacemit/k1-musepi-pro");
	else if (!strncmp(product_name, "k1-x_deb1", 9))
		snprintf(fdt_name, I2C_BUF_SIZE, "%s",
			 "spacemit/k1-bananapi-f3");
	if (fdt_name[0] == '\0') {
		/* set default board name */
		sprintf(fdt_name, "spacemit/k1-musepi-pro");
	}
	for (i = 0; i < I2C_BUF_SIZE; i++) {
		if (fdt_name[i] == '\0')
			break;
		fdt_name[i] = tolower(fdt_name[i]);
	}
	if (!strcmp(name, fdt_name))
		return 0;
	return -ENOENT;
}

void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	return (void *)CONFIG_SPL_LOAD_FIT_ADDRESS;
}
