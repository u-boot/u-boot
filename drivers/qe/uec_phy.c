/*
 * Copyright (C) 2005 Freescale Semiconductor, Inc.
 *
 * Author: Shlomi Gridish
 *
 * Description: UCC GETH Driver -- PHY handling
 *		Driver for UEC on QE
 *		Based on 8260_io/fcc_enet.c
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#include "common.h"
#include "net.h"
#include "malloc.h"
#include "asm/errno.h"
#include "asm/immap_qe.h"
#include "asm/io.h"
#include "qe.h"
#include "uccf.h"
#include "uec.h"
#include "uec_phy.h"
#include "miiphy.h"

#if defined(CONFIG_QE)

#define ugphy_printk(format, arg...)  \
	printf(format "\n", ## arg)

#define ugphy_dbg(format, arg...)	     \
	ugphy_printk(format , ## arg)
#define ugphy_err(format, arg...)	     \
	ugphy_printk(format , ## arg)
#define ugphy_info(format, arg...)	     \
	ugphy_printk(format , ## arg)
#define ugphy_warn(format, arg...)	     \
	ugphy_printk(format , ## arg)

#ifdef UEC_VERBOSE_DEBUG
#define ugphy_vdbg ugphy_dbg
#else
#define ugphy_vdbg(ugeth, fmt, args...) do { } while (0)
#endif /* UEC_VERBOSE_DEBUG */

static void config_genmii_advert (struct uec_mii_info *mii_info);
static void genmii_setup_forced (struct uec_mii_info *mii_info);
static void genmii_restart_aneg (struct uec_mii_info *mii_info);
static int gbit_config_aneg (struct uec_mii_info *mii_info);
static int genmii_config_aneg (struct uec_mii_info *mii_info);
static int genmii_update_link (struct uec_mii_info *mii_info);
static int genmii_read_status (struct uec_mii_info *mii_info);
u16 phy_read (struct uec_mii_info *mii_info, u16 regnum);
void phy_write (struct uec_mii_info *mii_info, u16 regnum, u16 val);

/* Write value to the PHY for this device to the register at regnum, */
/* waiting until the write is done before it returns.  All PHY */
/* configuration has to be done through the TSEC1 MIIM regs */
void uec_write_phy_reg (struct eth_device *dev, int mii_id, int regnum, int value)
{
	uec_private_t *ugeth = (uec_private_t *) dev->priv;
	uec_mii_t *ug_regs;
	enet_tbi_mii_reg_e mii_reg = (enet_tbi_mii_reg_e) regnum;
	u32 tmp_reg;

	ug_regs = ugeth->uec_mii_regs;

	/* Stop the MII management read cycle */
	out_be32 (&ug_regs->miimcom, 0);
	/* Setting up the MII Mangement Address Register */
	tmp_reg = ((u32) mii_id << MIIMADD_PHY_ADDRESS_SHIFT) | mii_reg;
	out_be32 (&ug_regs->miimadd, tmp_reg);

	/* Setting up the MII Mangement Control Register with the value */
	out_be32 (&ug_regs->miimcon, (u32) value);
	sync();

	/* Wait till MII management write is complete */
	while ((in_be32 (&ug_regs->miimind)) & MIIMIND_BUSY);
}

/* Reads from register regnum in the PHY for device dev, */
/* returning the value.  Clears miimcom first.  All PHY */
/* configuration has to be done through the TSEC1 MIIM regs */
int uec_read_phy_reg (struct eth_device *dev, int mii_id, int regnum)
{
	uec_private_t *ugeth = (uec_private_t *) dev->priv;
	uec_mii_t *ug_regs;
	enet_tbi_mii_reg_e mii_reg = (enet_tbi_mii_reg_e) regnum;
	u32 tmp_reg;
	u16 value;

	ug_regs = ugeth->uec_mii_regs;

	/* Setting up the MII Mangement Address Register */
	tmp_reg = ((u32) mii_id << MIIMADD_PHY_ADDRESS_SHIFT) | mii_reg;
	out_be32 (&ug_regs->miimadd, tmp_reg);

	/* clear MII management command cycle */
	out_be32 (&ug_regs->miimcom, 0);
	sync();

	/* Perform an MII management read cycle */
	out_be32 (&ug_regs->miimcom, MIIMCOM_READ_CYCLE);

	/* Wait till MII management write is complete */
	while ((in_be32 (&ug_regs->miimind)) &
	       (MIIMIND_NOT_VALID | MIIMIND_BUSY));

	/* Read MII management status  */
	value = (u16) in_be32 (&ug_regs->miimstat);
	if (value == 0xffff)
		ugphy_vdbg
			("read wrong value : mii_id %d,mii_reg %d, base %08x",
			 mii_id, mii_reg, (u32) & (ug_regs->miimcfg));

	return (value);
}

void mii_clear_phy_interrupt (struct uec_mii_info *mii_info)
{
	if (mii_info->phyinfo->ack_interrupt)
		mii_info->phyinfo->ack_interrupt (mii_info);
}

void mii_configure_phy_interrupt (struct uec_mii_info *mii_info,
				  u32 interrupts)
{
	mii_info->interrupts = interrupts;
	if (mii_info->phyinfo->config_intr)
		mii_info->phyinfo->config_intr (mii_info);
}

/* Writes MII_ADVERTISE with the appropriate values, after
 * sanitizing advertise to make sure only supported features
 * are advertised
 */
static void config_genmii_advert (struct uec_mii_info *mii_info)
{
	u32 advertise;
	u16 adv;

	/* Only allow advertising what this PHY supports */
	mii_info->advertising &= mii_info->phyinfo->features;
	advertise = mii_info->advertising;

	/* Setup standard advertisement */
	adv = phy_read (mii_info, PHY_ANAR);
	adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4);
	if (advertise & ADVERTISED_10baseT_Half)
		adv |= ADVERTISE_10HALF;
	if (advertise & ADVERTISED_10baseT_Full)
		adv |= ADVERTISE_10FULL;
	if (advertise & ADVERTISED_100baseT_Half)
		adv |= ADVERTISE_100HALF;
	if (advertise & ADVERTISED_100baseT_Full)
		adv |= ADVERTISE_100FULL;
	phy_write (mii_info, PHY_ANAR, adv);
}

static void genmii_setup_forced (struct uec_mii_info *mii_info)
{
	u16 ctrl;
	u32 features = mii_info->phyinfo->features;

	ctrl = phy_read (mii_info, PHY_BMCR);

	ctrl &= ~(PHY_BMCR_DPLX | PHY_BMCR_100_MBPS |
		  PHY_BMCR_1000_MBPS | PHY_BMCR_AUTON);
	ctrl |= PHY_BMCR_RESET;

	switch (mii_info->speed) {
	case SPEED_1000:
		if (features & (SUPPORTED_1000baseT_Half
				| SUPPORTED_1000baseT_Full)) {
			ctrl |= PHY_BMCR_1000_MBPS;
			break;
		}
		mii_info->speed = SPEED_100;
	case SPEED_100:
		if (features & (SUPPORTED_100baseT_Half
				| SUPPORTED_100baseT_Full)) {
			ctrl |= PHY_BMCR_100_MBPS;
			break;
		}
		mii_info->speed = SPEED_10;
	case SPEED_10:
		if (features & (SUPPORTED_10baseT_Half
				| SUPPORTED_10baseT_Full))
			break;
	default:		/* Unsupported speed! */
		ugphy_err ("%s: Bad speed!", mii_info->dev->name);
		break;
	}

	phy_write (mii_info, PHY_BMCR, ctrl);
}

/* Enable and Restart Autonegotiation */
static void genmii_restart_aneg (struct uec_mii_info *mii_info)
{
	u16 ctl;

	ctl = phy_read (mii_info, PHY_BMCR);
	ctl |= (PHY_BMCR_AUTON | PHY_BMCR_RST_NEG);
	phy_write (mii_info, PHY_BMCR, ctl);
}

static int gbit_config_aneg (struct uec_mii_info *mii_info)
{
	u16 adv;
	u32 advertise;

	if (mii_info->autoneg) {
		/* Configure the ADVERTISE register */
		config_genmii_advert (mii_info);
		advertise = mii_info->advertising;

		adv = phy_read (mii_info, MII_1000BASETCONTROL);
		adv &= ~(MII_1000BASETCONTROL_FULLDUPLEXCAP |
			 MII_1000BASETCONTROL_HALFDUPLEXCAP);
		if (advertise & SUPPORTED_1000baseT_Half)
			adv |= MII_1000BASETCONTROL_HALFDUPLEXCAP;
		if (advertise & SUPPORTED_1000baseT_Full)
			adv |= MII_1000BASETCONTROL_FULLDUPLEXCAP;
		phy_write (mii_info, MII_1000BASETCONTROL, adv);

		/* Start/Restart aneg */
		genmii_restart_aneg (mii_info);
	} else
		genmii_setup_forced (mii_info);

	return 0;
}

static int marvell_config_aneg (struct uec_mii_info *mii_info)
{
	/* The Marvell PHY has an errata which requires
	 * that certain registers get written in order
	 * to restart autonegotiation */
	phy_write (mii_info, PHY_BMCR, PHY_BMCR_RESET);

	phy_write (mii_info, 0x1d, 0x1f);
	phy_write (mii_info, 0x1e, 0x200c);
	phy_write (mii_info, 0x1d, 0x5);
	phy_write (mii_info, 0x1e, 0);
	phy_write (mii_info, 0x1e, 0x100);

	gbit_config_aneg (mii_info);

	return 0;
}

static int genmii_config_aneg (struct uec_mii_info *mii_info)
{
	if (mii_info->autoneg) {
		config_genmii_advert (mii_info);
		genmii_restart_aneg (mii_info);
	} else
		genmii_setup_forced (mii_info);

	return 0;
}

static int genmii_update_link (struct uec_mii_info *mii_info)
{
	u16 status;

	/* Status is read once to clear old link state */
	phy_read (mii_info, PHY_BMSR);

	/*
	 * Wait if the link is up, and autonegotiation is in progress
	 * (ie - we're capable and it's not done)
	 */
	status = phy_read(mii_info, PHY_BMSR);
	if ((status & PHY_BMSR_LS) && (status & PHY_BMSR_AUTN_ABLE)
	    && !(status & PHY_BMSR_AUTN_COMP)) {
		int i = 0;

		while (!(status & PHY_BMSR_AUTN_COMP)) {
			/*
			 * Timeout reached ?
			 */
			if (i > UGETH_AN_TIMEOUT) {
				mii_info->link = 0;
				return 0;
			}

			udelay(1000);	/* 1 ms */
			status = phy_read(mii_info, PHY_BMSR);
		}
		mii_info->link = 1;
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (status & PHY_BMSR_LS)
			mii_info->link = 1;
		else
			mii_info->link = 0;
	}

	return 0;
}

static int genmii_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there
	 * was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;

	if (mii_info->autoneg) {
		status = phy_read (mii_info, PHY_ANLPAR);

		if (status & (PHY_ANLPAR_10FD | PHY_ANLPAR_TXFD))
			mii_info->duplex = DUPLEX_FULL;
		else
			mii_info->duplex = DUPLEX_HALF;
		if (status & (PHY_ANLPAR_TXFD | PHY_ANLPAR_TX))
			mii_info->speed = SPEED_100;
		else
			mii_info->speed = SPEED_10;
		mii_info->pause = 0;
	}
	/* On non-aneg, we assume what we put in BMCR is the speed,
	 * though magic-aneg shouldn't prevent this case from occurring
	 */

	return 0;
}

static int marvell_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there
	 * was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;

	/* If the link is up, read the speed and duplex */
	/* If we aren't autonegotiating, assume speeds
	 * are as set */
	if (mii_info->autoneg && mii_info->link) {
		int speed;

		status = phy_read (mii_info, MII_M1011_PHY_SPEC_STATUS);

		/* Get the duplexity */
		if (status & MII_M1011_PHY_SPEC_STATUS_FULLDUPLEX)
			mii_info->duplex = DUPLEX_FULL;
		else
			mii_info->duplex = DUPLEX_HALF;

		/* Get the speed */
		speed = status & MII_M1011_PHY_SPEC_STATUS_SPD_MASK;
		switch (speed) {
		case MII_M1011_PHY_SPEC_STATUS_1000:
			mii_info->speed = SPEED_1000;
			break;
		case MII_M1011_PHY_SPEC_STATUS_100:
			mii_info->speed = SPEED_100;
			break;
		default:
			mii_info->speed = SPEED_10;
			break;
		}
		mii_info->pause = 0;
	}

	return 0;
}

static int marvell_ack_interrupt (struct uec_mii_info *mii_info)
{
	/* Clear the interrupts by reading the reg */
	phy_read (mii_info, MII_M1011_IEVENT);

	return 0;
}

static int marvell_config_intr (struct uec_mii_info *mii_info)
{
	if (mii_info->interrupts == MII_INTERRUPT_ENABLED)
		phy_write (mii_info, MII_M1011_IMASK, MII_M1011_IMASK_INIT);
	else
		phy_write (mii_info, MII_M1011_IMASK, MII_M1011_IMASK_CLEAR);

	return 0;
}

static int dm9161_init (struct uec_mii_info *mii_info)
{
	/* Reset the PHY */
	phy_write (mii_info, PHY_BMCR, phy_read (mii_info, PHY_BMCR) |
		   PHY_BMCR_RESET);
	/* PHY and MAC connect */
	phy_write (mii_info, PHY_BMCR, phy_read (mii_info, PHY_BMCR) &
		   ~PHY_BMCR_ISO);

	phy_write (mii_info, MII_DM9161_SCR, MII_DM9161_SCR_INIT);

	config_genmii_advert (mii_info);
	/* Start/restart aneg */
	genmii_config_aneg (mii_info);

	return 0;
}

static int dm9161_config_aneg (struct uec_mii_info *mii_info)
{
	return 0;
}

static int dm9161_read_status (struct uec_mii_info *mii_info)
{
	u16 status;
	int err;

	/* Update the link, but return if there was an error */
	err = genmii_update_link (mii_info);
	if (err)
		return err;
	/* If the link is up, read the speed and duplex
	   If we aren't autonegotiating assume speeds are as set */
	if (mii_info->autoneg && mii_info->link) {
		status = phy_read (mii_info, MII_DM9161_SCSR);
		if (status & (MII_DM9161_SCSR_100F | MII_DM9161_SCSR_100H))
			mii_info->speed = SPEED_100;
		else
			mii_info->speed = SPEED_10;

		if (status & (MII_DM9161_SCSR_100F | MII_DM9161_SCSR_10F))
			mii_info->duplex = DUPLEX_FULL;
		else
			mii_info->duplex = DUPLEX_HALF;
	}

	return 0;
}

static int dm9161_ack_interrupt (struct uec_mii_info *mii_info)
{
	/* Clear the interrupt by reading the reg */
	phy_read (mii_info, MII_DM9161_INTR);

	return 0;
}

static int dm9161_config_intr (struct uec_mii_info *mii_info)
{
	if (mii_info->interrupts == MII_INTERRUPT_ENABLED)
		phy_write (mii_info, MII_DM9161_INTR, MII_DM9161_INTR_INIT);
	else
		phy_write (mii_info, MII_DM9161_INTR, MII_DM9161_INTR_STOP);

	return 0;
}

static void dm9161_close (struct uec_mii_info *mii_info)
{
}

static struct phy_info phy_info_dm9161 = {
	.phy_id = 0x0181b880,
	.phy_id_mask = 0x0ffffff0,
	.name = "Davicom DM9161E",
	.init = dm9161_init,
	.config_aneg = dm9161_config_aneg,
	.read_status = dm9161_read_status,
	.close = dm9161_close,
};

static struct phy_info phy_info_dm9161a = {
	.phy_id = 0x0181b8a0,
	.phy_id_mask = 0x0ffffff0,
	.name = "Davicom DM9161A",
	.features = MII_BASIC_FEATURES,
	.init = dm9161_init,
	.config_aneg = dm9161_config_aneg,
	.read_status = dm9161_read_status,
	.ack_interrupt = dm9161_ack_interrupt,
	.config_intr = dm9161_config_intr,
	.close = dm9161_close,
};

static struct phy_info phy_info_marvell = {
	.phy_id = 0x01410c00,
	.phy_id_mask = 0xffffff00,
	.name = "Marvell 88E11x1",
	.features = MII_GBIT_FEATURES,
	.config_aneg = &marvell_config_aneg,
	.read_status = &marvell_read_status,
	.ack_interrupt = &marvell_ack_interrupt,
	.config_intr = &marvell_config_intr,
};

static struct phy_info phy_info_genmii = {
	.phy_id = 0x00000000,
	.phy_id_mask = 0x00000000,
	.name = "Generic MII",
	.features = MII_BASIC_FEATURES,
	.config_aneg = genmii_config_aneg,
	.read_status = genmii_read_status,
};

static struct phy_info *phy_info[] = {
	&phy_info_dm9161,
	&phy_info_dm9161a,
	&phy_info_marvell,
	&phy_info_genmii,
	NULL
};

u16 phy_read (struct uec_mii_info *mii_info, u16 regnum)
{
	return mii_info->mdio_read (mii_info->dev, mii_info->mii_id, regnum);
}

void phy_write (struct uec_mii_info *mii_info, u16 regnum, u16 val)
{
	mii_info->mdio_write (mii_info->dev, mii_info->mii_id, regnum, val);
}

/* Use the PHY ID registers to determine what type of PHY is attached
 * to device dev.  return a struct phy_info structure describing that PHY
 */
struct phy_info *uec_get_phy_info (struct uec_mii_info *mii_info)
{
	u16 phy_reg;
	u32 phy_ID;
	int i;
	struct phy_info *theInfo = NULL;

	/* Grab the bits from PHYIR1, and put them in the upper half */
	phy_reg = phy_read (mii_info, PHY_PHYIDR1);
	phy_ID = (phy_reg & 0xffff) << 16;

	/* Grab the bits from PHYIR2, and put them in the lower half */
	phy_reg = phy_read (mii_info, PHY_PHYIDR2);
	phy_ID |= (phy_reg & 0xffff);

	/* loop through all the known PHY types, and find one that */
	/* matches the ID we read from the PHY. */
	for (i = 0; phy_info[i]; i++)
		if (phy_info[i]->phy_id ==
		    (phy_ID & phy_info[i]->phy_id_mask)) {
			theInfo = phy_info[i];
			break;
		}

	/* This shouldn't happen, as we have generic PHY support */
	if (theInfo == NULL) {
		ugphy_info ("UEC: PHY id %x is not supported!", phy_ID);
		return NULL;
	} else {
		ugphy_info ("UEC: PHY is %s (%x)", theInfo->name, phy_ID);
	}

	return theInfo;
}

void marvell_phy_interface_mode (struct eth_device *dev,
				 enet_interface_e mode)
{
	uec_private_t *uec = (uec_private_t *) dev->priv;
	struct uec_mii_info *mii_info;
	u16 status;

	if (!uec->mii_info) {
		printf ("%s: the PHY not intialized\n", __FUNCTION__);
		return;
	}
	mii_info = uec->mii_info;

	if (mode == ENET_100_RGMII) {
		phy_write (mii_info, 0x00, 0x9140);
		phy_write (mii_info, 0x1d, 0x001f);
		phy_write (mii_info, 0x1e, 0x200c);
		phy_write (mii_info, 0x1d, 0x0005);
		phy_write (mii_info, 0x1e, 0x0000);
		phy_write (mii_info, 0x1e, 0x0100);
		phy_write (mii_info, 0x09, 0x0e00);
		phy_write (mii_info, 0x04, 0x01e1);
		phy_write (mii_info, 0x00, 0x9140);
		phy_write (mii_info, 0x00, 0x1000);
		udelay (100000);
		phy_write (mii_info, 0x00, 0x2900);
		phy_write (mii_info, 0x14, 0x0cd2);
		phy_write (mii_info, 0x00, 0xa100);
		phy_write (mii_info, 0x09, 0x0000);
		phy_write (mii_info, 0x1b, 0x800b);
		phy_write (mii_info, 0x04, 0x05e1);
		phy_write (mii_info, 0x00, 0xa100);
		phy_write (mii_info, 0x00, 0x2100);
		udelay (1000000);
	} else if (mode == ENET_10_RGMII) {
		phy_write (mii_info, 0x14, 0x8e40);
		phy_write (mii_info, 0x1b, 0x800b);
		phy_write (mii_info, 0x14, 0x0c82);
		phy_write (mii_info, 0x00, 0x8100);
		udelay (1000000);
	}

	/* handle 88e1111 rev.B2 erratum 5.6 */
	if (mii_info->autoneg) {
		status = phy_read (mii_info, PHY_BMCR);
		phy_write (mii_info, PHY_BMCR, status | PHY_BMCR_AUTON);
	}
	/* now the B2 will correctly report autoneg completion status */
}

void change_phy_interface_mode (struct eth_device *dev, enet_interface_e mode)
{
#ifdef CONFIG_PHY_MODE_NEED_CHANGE
	marvell_phy_interface_mode (dev, mode);
#endif
}
#endif /* CONFIG_QE */
