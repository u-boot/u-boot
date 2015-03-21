/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch-fsl-lsch3/immap_lsch3.h>
#include <fsl_mdio.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl-mc/ldpaa_wriop.h>

#include "../common/qixis.h"

#include "ls2085aqds_qixis.h"


#ifdef CONFIG_FSL_MC_ENET
 /* - In LS2085A there are only 16 SERDES lanes, spread across 2 SERDES banks.
 *   Bank 1 -> Lanes A, B, C, D, E, F, G, H
 *   Bank 2 -> Lanes A,B, C, D, E, F, G, H
 */

 /* Mapping of 16 SERDES lanes to LS2085A QDS board slots. A value of '0' here
  * means that the mapping must be determined dynamically, or that the lane
  * maps to something other than a board slot.
  */

static u8 lane_to_slot_fsm2[] = {
	0, 0, 0, 0, 0, 0, 0, 0
};

/* On the Vitesse VSC8234XHG SGMII riser card there are 4 SGMII PHYs
 * housed.
 */
static int riser_phy_addr[] = {
	SGMII_CARD_PORT1_PHY_ADDR,
	SGMII_CARD_PORT2_PHY_ADDR,
	SGMII_CARD_PORT3_PHY_ADDR,
	SGMII_CARD_PORT4_PHY_ADDR,
};

/* Slot2 does not have EMI connections */
#define EMI_NONE	0xFFFFFFFF
#define EMI1_SLOT1	0
#define EMI1_SLOT2	1
#define EMI1_SLOT3	2
#define EMI1_SLOT4	3
#define EMI1_SLOT5	4
#define EMI1_SLOT6	5
#define EMI2		6
#define SFP_TX		1

static const char * const mdio_names[] = {
	"LS2085A_QDS_MDIO0",
	"LS2085A_QDS_MDIO1",
	"LS2085A_QDS_MDIO2",
	"LS2085A_QDS_MDIO3",
	"LS2085A_QDS_MDIO4",
	"LS2085A_QDS_MDIO5",
	DEFAULT_WRIOP_MDIO2_NAME,
};

struct ls2085a_qds_mdio {
	u8 muxval;
	struct mii_dev *realbus;
};

static const char *ls2085a_qds_mdio_name_for_muxval(u8 muxval)
{
	return mdio_names[muxval];
}

struct mii_dev *mii_dev_for_muxval(u8 muxval)
{
	struct mii_dev *bus;
	const char *name = ls2085a_qds_mdio_name_for_muxval(muxval);

	if (!name) {
		printf("No bus for muxval %x\n", muxval);
		return NULL;
	}

	bus = miiphy_get_dev_by_name(name);

	if (!bus) {
		printf("No bus by name %s\n", name);
		return NULL;
	}

	return bus;
}

static void ls2085a_qds_enable_SFP_TX(u8 muxval)
{
	u8 brdcfg9;

	brdcfg9 = QIXIS_READ(brdcfg[9]);
	brdcfg9 &= ~BRDCFG9_SFPTX_MASK;
	brdcfg9 |= (muxval << BRDCFG9_SFPTX_SHIFT);
	QIXIS_WRITE(brdcfg[9], brdcfg9);
}

static void ls2085a_qds_mux_mdio(u8 muxval)
{
	u8 brdcfg4;

	if (muxval <= 5) {
		brdcfg4 = QIXIS_READ(brdcfg[4]);
		brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
		brdcfg4 |= (muxval << BRDCFG4_EMISEL_SHIFT);
		QIXIS_WRITE(brdcfg[4], brdcfg4);
	}
}

static int ls2085a_qds_mdio_read(struct mii_dev *bus, int addr,
				 int devad, int regnum)
{
	struct ls2085a_qds_mdio *priv = bus->priv;

	ls2085a_qds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int ls2085a_qds_mdio_write(struct mii_dev *bus, int addr, int devad,
				  int regnum, u16 value)
{
	struct ls2085a_qds_mdio *priv = bus->priv;

	ls2085a_qds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int ls2085a_qds_mdio_reset(struct mii_dev *bus)
{
	struct ls2085a_qds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int ls2085a_qds_mdio_init(char *realbusname, u8 muxval)
{
	struct ls2085a_qds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate ls2085a_qds MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate ls2085a_qds private data\n");
		free(bus);
		return -1;
	}

	bus->read = ls2085a_qds_mdio_read;
	bus->write = ls2085a_qds_mdio_write;
	bus->reset = ls2085a_qds_mdio_reset;
	sprintf(bus->name, ls2085a_qds_mdio_name_for_muxval(muxval));

	pmdio->realbus = miiphy_get_dev_by_name(realbusname);

	if (!pmdio->realbus) {
		printf("No bus with name %s\n", realbusname);
		free(bus);
		free(pmdio);
		return -1;
	}

	pmdio->muxval = muxval;
	bus->priv = pmdio;

	return mdio_register(bus);
}

/*
 * Initialize the dpmac_info array.
 *
 */
static void initialize_dpmac_to_slot(void)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;
	int serdes2_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;


	switch (serdes1_prtcl) {
	case 0x2A:
		printf("qds: WRIOP: Supported SerDes Protocol 0x%02x\n",
		       serdes1_prtcl);
		break;
	default:
		printf("qds: WRIOP: Unsupported SerDes Protocol 0x%02x\n",
		       serdes1_prtcl);
		break;
	}

	switch (serdes2_prtcl) {
	case 0x07:
	case 0x08:
		printf("qds: WRIOP: Supported SerDes Protocol 0x%02x\n",
		       serdes2_prtcl);
		lane_to_slot_fsm2[0] = EMI1_SLOT4;
		lane_to_slot_fsm2[1] = EMI1_SLOT4;
		lane_to_slot_fsm2[2] = EMI1_SLOT4;
		lane_to_slot_fsm2[3] = EMI1_SLOT4;
		/* No MDIO physical connection */
		lane_to_slot_fsm2[4] = EMI1_SLOT6;
		lane_to_slot_fsm2[5] = EMI1_SLOT6;
		lane_to_slot_fsm2[6] = EMI1_SLOT6;
		lane_to_slot_fsm2[7] = EMI1_SLOT6;
		break;
	default:
		printf("qds: WRIOP: Unsupported SerDes Protocol 0x%02x\n",
		       serdes2_prtcl);
		break;
	}
}

void ls2085a_handle_phy_interface_sgmii(int dpmac_id)
{
	int lane, slot;
	struct mii_dev *bus;
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;
	int serdes2_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	switch (serdes1_prtcl) {
	}

	switch (serdes2_prtcl) {
	case 0x07:
	case 0x08:
		lane = serdes_get_first_lane(FSL_SRDS_2, SGMII9 +
							(dpmac_id - 9));
		slot = lane_to_slot_fsm2[lane];

		switch (++slot) {
		case 1:
			break;
		case 3:
			break;
		case 4:
			/* Slot housing a SGMII riser card? */
			wriop_set_phy_address(dpmac_id,
					      riser_phy_addr[dpmac_id - 9]);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT4;
			bus = mii_dev_for_muxval(EMI1_SLOT4);
			wriop_set_mdio(dpmac_id, bus);
			dpmac_info[dpmac_id].phydev = phy_connect(
						dpmac_info[dpmac_id].bus,
						dpmac_info[dpmac_id].phy_addr,
						NULL,
						dpmac_info[dpmac_id].enet_if);
			phy_config(dpmac_info[dpmac_id].phydev);
		break;
		case 5:
		break;
		case 6:
			/* Slot housing a SGMII riser card? */
			wriop_set_phy_address(dpmac_id,
					      riser_phy_addr[dpmac_id - 13]);
			dpmac_info[dpmac_id].board_mux = EMI1_SLOT6;
			bus = mii_dev_for_muxval(EMI1_SLOT6);
			wriop_set_mdio(dpmac_id, bus);
		break;
	}
	break;
	default:
		printf("qds: WRIOP: Unsupported SerDes Protocol 0x%02x\n",
		       serdes2_prtcl);
	break;
	}
}
void ls2085a_handle_phy_interface_xsgmii(int i)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	int serdes1_prtcl = (in_le32(&gur->rcwsr[28]) &
				FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK)
		>> FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	switch (serdes1_prtcl) {
	case 0x2A:
		/*
		 * XFI does not need a PHY to work, but to avoid U-boot use
		 * default PHY address which is zero to a MAC when it found
		 * a MAC has no PHY address, we give a PHY address to XFI
		 * MAC, and should not use a real XAUI PHY address, since
		 * MDIO can access it successfully, and then MDIO thinks
		 * the XAUI card is used for the XFI MAC, which will cause
		 * error.
		 */
		wriop_set_phy_address(i, i + 4);
		ls2085a_qds_enable_SFP_TX(SFP_TX);

		break;
	default:
		printf("qds: WRIOP: Unsupported SerDes Protocol 0x%02x\n",
		       serdes1_prtcl);
		break;
	}
}
#endif

int board_eth_init(bd_t *bis)
{
	int error;
#ifdef CONFIG_FSL_MC_ENET
	struct memac_mdio_info *memac_mdio0_info;
	struct memac_mdio_info *memac_mdio1_info;
	unsigned int i;

	initialize_dpmac_to_slot();

	memac_mdio0_info = (struct memac_mdio_info *)malloc(
					sizeof(struct memac_mdio_info));
	memac_mdio0_info->regs =
		(struct memac_mdio_controller *)
					CONFIG_SYS_FSL_WRIOP1_MDIO1;
	memac_mdio0_info->name = DEFAULT_WRIOP_MDIO1_NAME;

	/* Register the real MDIO1 bus */
	fm_memac_mdio_init(bis, memac_mdio0_info);

	memac_mdio1_info = (struct memac_mdio_info *)malloc(
					sizeof(struct memac_mdio_info));
	memac_mdio1_info->regs =
		(struct memac_mdio_controller *)
					CONFIG_SYS_FSL_WRIOP1_MDIO2;
	memac_mdio1_info->name = DEFAULT_WRIOP_MDIO2_NAME;

	/* Register the real MDIO2 bus */
	fm_memac_mdio_init(bis, memac_mdio1_info);

	/* Register the muxing front-ends to the MDIO buses */
	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT1);
	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT2);
	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT3);
	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT4);
	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT5);
	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO1_NAME, EMI1_SLOT6);

	ls2085a_qds_mdio_init(DEFAULT_WRIOP_MDIO2_NAME, EMI2);

	for (i = WRIOP1_DPMAC1; i < NUM_WRIOP_PORTS; i++) {
		switch (wriop_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_QSGMII:
			break;
		case PHY_INTERFACE_MODE_SGMII:
			ls2085a_handle_phy_interface_sgmii(i);
			break;
		case PHY_INTERFACE_MODE_XGMII:
			ls2085a_handle_phy_interface_xsgmii(i);
			break;
		default:
			break;
		}
	}

	error = cpu_eth_init(bis);
#endif
	error = pci_eth_init(bis);
	return error;
}
