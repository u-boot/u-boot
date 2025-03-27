// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * Secure monitor calls.
 */

#include <dm.h>
#include <log.h>
#include <regmap.h>
#include <sm.h>
#include <syscon.h>
#include <asm/arch/boot.h>
#include <asm/arch/sm.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <linux/bitops.h>
#include <linux/compiler_attributes.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/bitfield.h>
#include <meson/sm.h>

static inline struct udevice *meson_get_sm_device(void)
{
	struct udevice *dev;
	int err;

	err = uclass_first_device_err(UCLASS_SM, &dev);
	if (err) {
		pr_err("Mesom SM device not found\n");
		return ERR_PTR(err);
	}

	return dev;
}

ssize_t meson_sm_read_efuse(uintptr_t offset, void *buffer, size_t size)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	regs.regs[1] = offset;
	regs.regs[2] = size;

	err = sm_call_read(dev, buffer, size,
			   MESON_SMC_CMD_EFUSE_READ, &regs);
	if (err < 0)
		pr_err("Failed to read efuse memory (%d)\n", err);

	return err;
}

ssize_t meson_sm_write_efuse(uintptr_t offset, void *buffer, size_t size)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	regs.regs[1] = offset;
	regs.regs[2] = size;

	err = sm_call_write(dev, buffer, size,
			    MESON_SMC_CMD_EFUSE_WRITE, &regs);
	if (err < 0)
		pr_err("Failed to write efuse memory (%d)\n", err);

	return err;
}

/*
 * Helps to handle two flavors of cpu_id layouts:
 *
 * - in-register view (value read from cpu_id reg, a.k.a. socinfo):
 *   +-----------+------------+------------+------------+
 *   | family_id | package_id |  chip_rev  | layout_rev |
 *   +-----------+------------+------------+------------+
 *   | 31     24 | 23      16 | 15       8 | 7        0 |
 *   +-----------+------------+------------+------------+
 *
 * - in-efuse view (value, residing inside efuse/shmem data usually for
 *   chip_id v2. Chip_id v1 does not contain cpu_id value inside efuse
 *   data (i.e. in chip_id_efuse)):
 *   +-----------+------------+------------+------------+
 *   | family_id |  chip_rev  | package_id | layout_rev |
 *   +-----------+------------+------------+------------+
 *   | 31     24 | 23      16 | 15       8 | 7        0 |
 *   +-----------+------------+------------+------------+
 */
enum {
	/* In-register view of cpu_id */
	CPU_ID_REG_MAJOR,	/* 31-24 bits */
	CPU_ID_REG_PACK,	/* 23-16 bits */
	CPU_ID_REG_MINOR,	/* 15-8 bits */
	CPU_ID_REG_MISC,	/* 7-0 bits */

	/* In-efuse view of cpu_id */
	CPU_ID_MAJOR = CPU_ID_REG_MAJOR,
	CPU_ID_PACK  = CPU_ID_REG_MINOR,
	CPU_ID_MINOR = CPU_ID_REG_PACK,
	CPU_ID_MISC  = CPU_ID_REG_MISC,
};

/*
 * This is a beginning chunk of the whole efuse storage area, containing
 * data related to chip_id only
 */
struct chip_id_efuse {
	u32 version;
	u8 raw[MESON_CHIP_ID_SZ]; /* payload */
} __packed;

static void meson_sm_serial_reverse(u8 serial[SM_SERIAL_SIZE])
{
	for (int i = 0; i < SM_SERIAL_SIZE / 2; i++) {
		int k = SM_SERIAL_SIZE - 1 - i;

		swap(serial[i], serial[k]);
	}
}

int meson_sm_get_chip_id(struct meson_sm_chip_id *chip_id)
{
	struct udevice *dev;
	union meson_cpu_id socinfo;
	struct pt_regs regs = { 0 };
	struct chip_id_efuse chip_id_efuse;
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	/*
	 * Request v2. If not supported by secure monitor, then v1 should be
	 * returned.
	 */
	regs.regs[1] = 2;

	err = sm_call_read(dev, &chip_id_efuse, sizeof(chip_id_efuse),
			   MESON_SMC_CMD_CHIP_ID_GET, &regs);
	if (err < 0) {
		pr_err("Failed to read chip_id (%d)\n", err);
		return err;
	}

	if (chip_id_efuse.version == 2) {
		memcpy((u8 *)chip_id, chip_id_efuse.raw,
		       sizeof(struct meson_sm_chip_id));
		return 0;
	}

	/*
	 * Legacy chip_id (v1) read out, transform data
	 * to expected order format (little-endian)
	 */
	memcpy(chip_id->serial, chip_id_efuse.raw, sizeof(chip_id->serial));
	meson_sm_serial_reverse(chip_id->serial);

	socinfo.val = meson_get_socinfo();
	if (!socinfo.val)
		return -ENODEV;

	chip_id->cpu_id = (union meson_cpu_id){
		.raw[CPU_ID_MAJOR] = socinfo.raw[CPU_ID_REG_MAJOR],
		.raw[CPU_ID_PACK]  = socinfo.raw[CPU_ID_REG_PACK],
		.raw[CPU_ID_MINOR] = socinfo.raw[CPU_ID_REG_MINOR],
		.raw[CPU_ID_MISC]  = socinfo.raw[CPU_ID_REG_MISC],
	};

	return 0;
}

int meson_sm_get_serial(void *buffer, size_t size)
{
	struct meson_sm_chip_id chip_id;
	int ret;

	if (size < SM_SERIAL_SIZE)
		return -EINVAL;

	ret = meson_sm_get_chip_id(&chip_id);
	if (ret)
		return ret;

	/*
	 * The order of serial inside chip_id and serial which function must
	 * return does not match: stick here to big-endian for backward
	 * compatibility.
	 */
	meson_sm_serial_reverse(chip_id.serial);
	memcpy(buffer, chip_id.serial, sizeof(chip_id.serial));
	return ret;
}

#define AO_SEC_SD_CFG15		0xfc
#define REBOOT_REASON_MASK	GENMASK(15, 12)

int meson_sm_get_reboot_reason(void)
{
	struct regmap *regmap;
	int nodeoffset;
	ofnode node;
	unsigned int reason;

	/* find the offset of compatible node */
	nodeoffset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						   "amlogic,meson-gx-ao-secure");
	if (nodeoffset < 0) {
		printf("%s: failed to get amlogic,meson-gx-ao-secure\n",
		       __func__);
		return -ENODEV;
	}

	/* get regmap from the syscon node */
	node = offset_to_ofnode(nodeoffset);
	regmap = syscon_node_to_regmap(node);
	if (IS_ERR(regmap)) {
		printf("%s: failed to get regmap\n", __func__);
		return -EINVAL;
	}

	regmap_read(regmap, AO_SEC_SD_CFG15, &reason);

	/* The SMC call is not used, we directly use AO_SEC_SD_CFG15 */
	return FIELD_GET(REBOOT_REASON_MASK, reason);
}

int meson_sm_pwrdm_set(size_t index, int cmd)
{
	struct udevice *dev;
	struct pt_regs regs = { 0 };
	int err;

	dev = meson_get_sm_device();
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	regs.regs[1] = index;
	regs.regs[2] = cmd;

	err = sm_call(dev, MESON_SMC_CMD_PWRDM_SET, NULL, &regs);
	if (err)
		pr_err("Failed to %s power domain ind=%zu (%d)\n", cmd == PWRDM_ON ?
				"enable" : "disable", index, err);

	return err;
}
