// SPDX-License-Identifier: GPL-2.0-only

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <pwrap/pwrap.h>
#include <power/pmic.h>

#define MT6328_HWCID                 0x0200
#define MT6328_DEW_READ_TEST         0x02d6
#define MT6328_DEW_WRITE_TEST        0x02d8

static const struct pmic_child_info pmic_regulators_info[] = {
	{ .prefix = "ldo", .driver = "mt6328_ldo" },
	{ .prefix = "buck", .driver = "mt6328_buck" },
    { .prefix = "fixed_ldo", .driver = "mt6328_fixed" },
	{ },
};

static const struct pmic_child_info pmic_devices_info[] = {
    { .prefix = "mt6328_keys", .driver = "mt6328_keys" },
    { },
};

static int mt6328_reg_count(struct udevice *dev)
{
	return 0xFFFF;
}

static int mt6328_read(struct udevice *dev, u32 adr, u32 *val)
{
	u32 rdata;
    
    pwrap_read(dev->parent, adr, &rdata);
	*val = rdata;
	return 0;
}

static int mt6328_write(struct udevice *dev, u32 adr, u32 val)
{
    return pwrap_write(dev->parent, adr, val);

}

static int mt6328_bind(struct udevice *dev)
{
	ofnode regulators_node, devices_node;
	int children;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		printf("%s: %s regulators subnode not found!", __func__,
		      dev->name);
		return -ENXIO;
	}

	printf("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	children = pmic_bind_children(dev, regulators_node, pmic_regulators_info);
	if (!children)
		printf("%s: %s - no child found\n", __func__, dev->name);

    devices_node = dev_read_subnode(dev, "devices");
    if (!ofnode_valid(devices_node)) {
        printf("%s: %s devices subnode not found!", __func__,
              dev->name);
        return -ENXIO;
    }

    printf("%s: '%s' - found devices subnode\n", __func__, dev->name);

    children = pmic_bind_children(dev, devices_node, pmic_devices_info);
    if (!children)
        printf("%s: %s - no child found\n", __func__, dev->name);
    
    return 0;
}

static int mt6328_probe(struct udevice *dev)
{
    u32 rdata =0;
    u32 sub_return = 0;
    u32 sub_return1 = 0;
	pmic_read_u32(dev, MT6328_HWCID, &rdata);
    if (rdata < 0) {
        printf("PMIC read ID fail");
        return -EIO;
    }
    printf("MediaTek MT6328 Chip ID 0x%x\n", rdata);

    pmic_read_u32(dev, MT6328_DEW_READ_TEST, &rdata);
	printf("PMIC read = 0x%x\n", rdata);
	if (rdata != 0x5aa5){
		printf("PMIC read test fail");
        return -EIO;
      }
    
	sub_return = pmic_write_u32(dev, MT6328_DEW_WRITE_TEST, 0x1234);
	sub_return1 = pmic_read_u32(dev, 0x02d8, &rdata);
	printf("PMIC Write = 0x%x\n", rdata);
	if ((rdata != 0x1234)||( sub_return != 0 )||( sub_return1 != 0 )) {
        printf("PMIC Write test fail\n");
        return -EIO;
    } else {
        printf("PMIC Write test pass\n");
        return 0;
    }

    return 0;
}

static struct dm_pmic_ops mt6328_ops = {
    .reg_count = mt6328_reg_count,
    .read_u32 = mt6328_read,
    .write_u32 = mt6328_write,
};
static const struct udevice_id mt6328_ids[] = {
	{ .compatible = "mediatek,mt6328" },
	{ }
};

U_BOOT_DRIVER(pmic_mt6328) = {
	.name = "mt6328_pmic",
	.id = UCLASS_PMIC,
	.of_match = mt6328_ids,
	.bind = mt6328_bind,
	.probe = mt6328_probe,
    .ops = &mt6328_ops,
};
