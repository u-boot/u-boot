#include <common.h>
#include <mii_phy.h>
#include "rpxsuper.h"

#define MII_MDIO	0x01
#define MII_MDCK	0x02
#define MII_MDIR	0x04

void
mii_discover_phy(void)
{
    int known;
    unsigned short phy_reg;
    unsigned long phy_id;

    known = 0;
    printf("Discovering phy @ 0: ");
    phy_id = mii_phy_read(2) << 16;
    phy_id |= mii_phy_read(3);
    if ((phy_id & 0xFFFFFC00) == 0x00137800) {
	printf("Level One ");
	if ((phy_id & 0x000003F0) == 0xE0) {
	    printf("LXT971A Revision %d\n", (int)(phy_id & 0xF));
	    known = 1;
	}
	else printf("unknown type\n");
    }
    else printf("unknown OUI = 0x%08lX\n", phy_id);

    phy_reg = mii_phy_read(1);
    if (!(phy_reg & 0x0004)) printf("Link is down\n");
    if (!(phy_reg & 0x0020)) printf("Auto-negotiation not complete\n");
    if (phy_reg & 0x0002) printf("Jabber condition detected\n");
    if (phy_reg & 0x0010) printf("Remote fault condition detected \n");

    if (known) {
	phy_reg = mii_phy_read(17);
	if (phy_reg & 0x0400)
	    printf("Phy operating at %d MBit/s in %s-duplex mode\n",
		phy_reg & 0x4000 ? 100 : 10,
		phy_reg & 0x0200 ? "full" : "half");
	else
	    printf("bad link!!\n");
/*
left  off: no link, green 100MBit, yellow 10MBit
right off: no activity, green full-duplex, yellow half-duplex
*/
	mii_phy_write(20, 0x0452);
    }
}

unsigned short
mii_phy_read(unsigned short reg)
{
    int i;
    unsigned short tmp, val = 0, adr = 0;
    t_rpx_regs *regs = (t_rpx_regs*)CFG_REGS_BASE;

    tmp = 0x6002 | (adr << 7) | (reg << 2);
    regs->bcsr4 = 0xC3;
    for (i = 0; i < 64; i++) {
	regs->bcsr4 ^= MII_MDCK;
    }
    for (i = 0; i < 16; i++) {
	regs->bcsr4 &= ~MII_MDCK;
	if (tmp & 0x8000) regs->bcsr4 |= MII_MDIO;
	else regs->bcsr4 &= ~MII_MDIO;
	regs->bcsr4 |= MII_MDCK;
	tmp <<= 1;
    }
    regs->bcsr4 |= MII_MDIR;
    for (i = 0; i < 16; i++) {
	val <<= 1;
	regs->bcsr4 = MII_MDIO | (regs->bcsr4 | MII_MDCK);
	if (regs->bcsr4 & MII_MDIO) val |= 1;
	regs->bcsr4 = MII_MDIO | (regs->bcsr4 &= ~MII_MDCK);
    }
    return val;
}

void
mii_phy_write(unsigned short reg, unsigned short val)
{
    int i;
    unsigned short tmp, adr = 0;
    t_rpx_regs *regs = (t_rpx_regs*)CFG_REGS_BASE;

    tmp = 0x5002 | (adr << 7) | (reg << 2);
    regs->bcsr4 = 0xC3;
    for (i = 0; i < 64; i++) {
	regs->bcsr4 ^= MII_MDCK;
    }
    for (i = 0; i < 16; i++) {
	regs->bcsr4 &= ~MII_MDCK;
	if (tmp & 0x8000) regs->bcsr4 |= MII_MDIO;
	else regs->bcsr4 &= ~MII_MDIO;
	regs->bcsr4 |= MII_MDCK;
	tmp <<= 1;
    }
    for (i = 0; i < 16; i++) {
	regs->bcsr4 &= ~MII_MDCK;
	if (val & 0x8000) regs->bcsr4 |= MII_MDIO;
	else regs->bcsr4 &= ~MII_MDIO;
	regs->bcsr4 |= MII_MDCK;
	val <<= 1;
    }
}
