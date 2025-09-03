// SPDX-License-Identifier: GPL-2.0+

struct mt7531_mdio_priv {
	phys_addr_t switch_regs;
};

int mt7531_mdio_read(struct udevice *dev, int addr, int devad, int reg);
int mt7531_mdio_write(struct udevice *dev, int addr, int devad,
		      int reg, u16 value);
