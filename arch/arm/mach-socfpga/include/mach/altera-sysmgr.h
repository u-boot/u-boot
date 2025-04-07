/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

struct altr_sysmgr_ops {
	int (*read)(struct udevice *dev, u32 *addr, u32 *value);
	int (*write)(struct udevice *dev, u32 *addr, u32 value);
};

struct altr_sysmgr_priv {
	void __iomem *regs;
};

#define altr_sysmgr_get_ops(dev)        ((struct altr_sysmgr_ops *)(dev)->driver->ops)
#define altr_sysmgr_get_priv(dev)	((struct altr_sysmgr_priv *)(dev_get_priv(dev)))
