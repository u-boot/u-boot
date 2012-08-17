#ifndef _MICREL_H

#define MII_KSZ9021_EXT_COMMON_CTRL		0x100
#define MII_KSZ9021_EXT_STRAP_STATUS		0x101
#define MII_KSZ9021_EXT_OP_STRAP_OVERRIDE	0x102
#define MII_KSZ9021_EXT_OP_STRAP_STATUS		0x103
#define MII_KSZ9021_EXT_RGMII_CLOCK_SKEW	0x104
#define MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW	0x105
#define MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW	0x106
#define MII_KSZ9021_EXT_ANALOG_TEST		0x107

struct phy_device;
int ksz9021_phy_extended_write(struct phy_device *phydev, int regnum, u16 val);
int ksz9021_phy_extended_read(struct phy_device *phydev, int regnum);

#endif
