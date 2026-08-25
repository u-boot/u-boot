// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Tenstorrent
 */

#include <dm.h>
#include <iommu.h>

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>

#define RISCV_IOMMU_REG_DDTP              0x10
#define RISCV_IOMMU_DDTP_BUSY             BIT_ULL(4)
#define RISCV_IOMMU_DDTP_TIMEOUT          30000000
#define RISCV_IOMMU_DDTP_IOMMU_MODE_MASK  GENMASK_ULL(3, 0)
#define RISCV_IOMMU_DDTP_IOMMU_MODE_OFF   0
#define RISCV_IOMMU_DDTP_IOMMU_MODE_BARE  1

static int riscv_iommu_probe(struct udevice *dev)
{
	void *base;
	int ret;
	u64 val;

	base = dev_read_addr_ptr(dev);
	if (!base)
		return -EINVAL;

	/* Read DDTP and check if busy bit is cleared */
	ret = readq_poll_timeout(base + RISCV_IOMMU_REG_DDTP, val,
				 !(val & RISCV_IOMMU_DDTP_BUSY),
				 RISCV_IOMMU_DDTP_TIMEOUT);
	if (ret)
		return ret;

	/*
	 * Transitions to Bare mode from any mode other
	 * than Off are undefined. Skip writing the register
	 * when already in Bare mode.
	 */
	switch (val & RISCV_IOMMU_DDTP_IOMMU_MODE_MASK) {
	case RISCV_IOMMU_DDTP_IOMMU_MODE_OFF:
		break;
	case RISCV_IOMMU_DDTP_IOMMU_MODE_BARE:
		return 0;
	default:
		return -EINVAL;
	}

	/* Enable Bare Mode */
	val &= ~RISCV_IOMMU_DDTP_IOMMU_MODE_MASK;
	val |= RISCV_IOMMU_DDTP_IOMMU_MODE_BARE;
	writeq(val, base + RISCV_IOMMU_REG_DDTP);

	/* Read DDTP to check if write was accepted */
	ret = readq_poll_timeout(base + RISCV_IOMMU_REG_DDTP, val,
				 !(val & RISCV_IOMMU_DDTP_BUSY),
				 RISCV_IOMMU_DDTP_TIMEOUT);
	if (ret)
		return ret;

	/* Check if DDTP value is Bare */
	if ((val & RISCV_IOMMU_DDTP_IOMMU_MODE_MASK) !=
	    RISCV_IOMMU_DDTP_IOMMU_MODE_BARE)
		return -EIO;

	return 0;
}

static const struct udevice_id riscv_iommu_ids[] = {
	{ .compatible = "riscv,iommu" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(riscv_iommu) = {
	.name = "riscv_iommu",
	.id = UCLASS_IOMMU,
	.of_match = riscv_iommu_ids,
	.probe = riscv_iommu_probe,
	.flags	= DM_FLAG_VITAL,
};
