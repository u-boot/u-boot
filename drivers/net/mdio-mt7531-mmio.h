/* SPDX-License-Identifier: GPL-2.0+ */

struct mt7531_mdio_mmio_priv {
	phys_addr_t switch_regs;
};

int mt7531_mdio_mmio_read(struct mii_dev *bus, int addr, int devad, int reg);
int mt7531_mdio_mmio_write(struct mii_dev *bus, int addr, int devad,
			   int reg, u16 value);
