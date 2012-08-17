/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>
#include <malloc.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/fsl_dtsec.h>

#include "../common/qixis.h"
#include "../common/fman.h"

#include "p3060qds_qixis.h"

#define EMI_NONE       0xffffffff
#define EMI1_RGMII1    0
#define EMI1_SLOT1     1
#define EMI1_SLOT2     2
#define EMI1_SLOT3     3
#define EMI1_RGMII2    4

static int mdio_mux[NUM_FM_PORTS];

static char *mdio_names[5] = {
	"P3060QDS_MDIO0",
	"P3060QDS_MDIO1",
	"P3060QDS_MDIO2",
	"P3060QDS_MDIO3",
	"P3060QDS_MDIO4",
};

/*
 * Mapping of all 18 SERDES lanes to board slots.
 * A value of '0' here means that the mapping must be determined
 * dynamically, Lane 8/9/16/17 map to Slot1 or Aurora debug
 */
static u8 lane_to_slot[] = {
	4, 4, 4, 4, 3, 3, 3, 3, 0, 0, 2, 2, 2, 2, 1, 1, 0, 0
};

static char *p3060qds_mdio_name_for_muxval(u32 muxval)
{
	return mdio_names[muxval];
}

struct mii_dev *mii_dev_for_muxval(u32 muxval)
{
	struct mii_dev *bus;
	char *name = p3060qds_mdio_name_for_muxval(muxval);

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

struct p3060qds_mdio {
	u32 muxval;
	struct mii_dev *realbus;
};

static void p3060qds_mux_mdio(u32 muxval)
{
	u8 brdcfg4;

	brdcfg4 = QIXIS_READ(brdcfg[4]);
	brdcfg4 &= ~BRDCFG4_EMISEL_MASK;
	brdcfg4 |= (muxval << 4);
	QIXIS_WRITE(brdcfg[4], brdcfg4);
}

static int p3060qds_mdio_read(struct mii_dev *bus, int addr, int devad,
				int regnum)
{
	struct p3060qds_mdio *priv = bus->priv;

	p3060qds_mux_mdio(priv->muxval);

	return priv->realbus->read(priv->realbus, addr, devad, regnum);
}

static int p3060qds_mdio_write(struct mii_dev *bus, int addr, int devad,
				int regnum, u16 value)
{
	struct p3060qds_mdio *priv = bus->priv;

	p3060qds_mux_mdio(priv->muxval);

	return priv->realbus->write(priv->realbus, addr, devad, regnum, value);
}

static int p3060qds_mdio_reset(struct mii_dev *bus)
{
	struct p3060qds_mdio *priv = bus->priv;

	return priv->realbus->reset(priv->realbus);
}

static int p3060qds_mdio_init(char *realbusname, u32 muxval)
{
	struct p3060qds_mdio *pmdio;
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate P3060QDS MDIO bus\n");
		return -1;
	}

	pmdio = malloc(sizeof(*pmdio));
	if (!pmdio) {
		printf("Failed to allocate P3060QDS private data\n");
		free(bus);
		return -1;
	}

	bus->read = p3060qds_mdio_read;
	bus->write = p3060qds_mdio_write;
	bus->reset = p3060qds_mdio_reset;
	sprintf(bus->name, p3060qds_mdio_name_for_muxval(muxval));

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

void board_ft_fman_fixup_port(void *blob, char * prop, phys_addr_t pa,
				enum fm_port port, int offset)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int srds_prtcl = (in_be32(&gur->rcwsr[4]) &
			  FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;

	if (mdio_mux[port] == EMI1_RGMII1)
		fdt_set_phy_handle(blob, prop, pa, "phy_rgmii1");

	if (mdio_mux[port] == EMI1_RGMII2)
		fdt_set_phy_handle(blob, prop, pa, "phy_rgmii2");

	if ((mdio_mux[port] == EMI1_SLOT1) && ((srds_prtcl == 0x3)
		|| (srds_prtcl == 0x6))) {
		switch (port) {
		case FM2_DTSEC4:
			fdt_set_phy_handle(blob, prop, pa, "phy2_slot1");
			break;
		case FM1_DTSEC4:
			fdt_set_phy_handle(blob, prop, pa, "phy3_slot1");
			break;
		default:
			break;
		}
	}

	if (mdio_mux[port] == EMI1_SLOT3) {
		switch (port) {
		case FM2_DTSEC3:
			fdt_set_phy_handle(blob, prop, pa, "phy0_slot3");
			break;
		case FM1_DTSEC3:
			fdt_set_phy_handle(blob, prop, pa, "phy1_slot3");
			break;
		default:
			break;
		}
	}
}

void fdt_fixup_board_enet(void *fdt)
{
	int i, lane, idx;

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		idx = i - FM1_DTSEC1;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + idx);
			if (lane < 0)
				break;

			switch (mdio_mux[i]) {
			case EMI1_SLOT1:
				if (lane >= 14) {
					fdt_status_okay_by_alias(fdt,
						"emi1_slot1");
					fdt_status_disabled_by_alias(fdt,
						"emi1_slot1_bk1");
				} else {
					fdt_status_disabled_by_alias(fdt,
						"emi1_slot1");
					fdt_status_okay_by_alias(fdt,
						"emi1_slot1_bk1");
				}
				break;
			case EMI1_SLOT2:
				fdt_status_okay_by_alias(fdt, "emi1_slot2");
				break;
			case EMI1_SLOT3:
				fdt_status_okay_by_alias(fdt, "emi1_slot3");
				break;
			}
		break;
		case PHY_INTERFACE_MODE_RGMII:
			if (i == FM1_DTSEC1)
				fdt_status_okay_by_alias(fdt, "emi1_rgmii1");

			if (i == FM1_DTSEC2)
				fdt_status_okay_by_alias(fdt, "emi1_rgmii2");
			break;
		default:
			break;
		}
	}
#if (CONFIG_SYS_NUM_FMAN == 2)
	for (i = FM2_DTSEC1; i < FM2_DTSEC1 + CONFIG_SYS_NUM_FM2_DTSEC; i++) {
		idx = i - FM2_DTSEC1;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM2_DTSEC1 + idx);
			if (lane >= 0) {
				switch (mdio_mux[i]) {
				case EMI1_SLOT1:
					if (lane >= 14)
						fdt_status_okay_by_alias(fdt,
							"emi1_slot1");
					else
						fdt_status_okay_by_alias(fdt,
							"emi1_slot1_bk1");
					break;
				case EMI1_SLOT2:
					fdt_status_okay_by_alias(fdt,
						"emi1_slot2");
					break;
				case EMI1_SLOT3:
					fdt_status_okay_by_alias(fdt,
						"emi1_slot3");
					break;
				}
			}
			break;
		default:
			break;
		}
	}
#endif
}

static void initialize_lane_to_slot(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int sdprtl = (in_be32(&gur->rcwsr[4]) &
				FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;

	switch (sdprtl) {
	case 0x03:
	case 0x06:
		lane_to_slot[8] = 1;
		lane_to_slot[9] = lane_to_slot[8];
		lane_to_slot[16] = 5;
		lane_to_slot[17] = lane_to_slot[16];
		break;
	case 0x16:
	case 0x19:
	case 0x1C:
		lane_to_slot[8] = 5;
		lane_to_slot[9] = lane_to_slot[8];
		lane_to_slot[16] = 1;
		lane_to_slot[17] = lane_to_slot[16];
		break;
	default:
		puts("Invalid SerDes protocol for P3060QDS\n");
		break;
	}
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_FMAN_ENET
	struct dtsec *tsec = (void *)CONFIG_SYS_FSL_FM1_DTSEC1_ADDR;
	int i;
	struct fsl_pq_mdio_info dtsec_mdio_info;
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	int srds_cfg = (in_be32(&gur->rcwsr[4]) &
				FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26;

	initialize_lane_to_slot();

	/*
	 * Set TBIPA on FM1@DTSEC1.  This is needed for configurations
	 * where FM1@DTSEC1 isn't used directly, since it provides
	 * MDIO for other ports.
	 */
	out_be32(&tsec->tbipa, CONFIG_SYS_TBIPA_VALUE);

	/* Initialize the mdio_mux array so we can recognize empty elements */
	for (i = 0; i < NUM_FM_PORTS; i++)
		mdio_mux[i] = EMI_NONE;

	dtsec_mdio_info.regs =
		(struct tsec_mii_mng *)CONFIG_SYS_FM1_DTSEC1_MDIO_ADDR;
	dtsec_mdio_info.name = DEFAULT_FM_MDIO_NAME;

	/* Register the 1G MDIO bus */
	fsl_pq_mdio_init(bis, &dtsec_mdio_info);

	/* Register the 5 muxing front-ends to the MDIO buses */
	if (fm_info_get_enet_if(FM1_DTSEC1) == PHY_INTERFACE_MODE_RGMII)
		p3060qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII1);

	if (fm_info_get_enet_if(FM1_DTSEC2) == PHY_INTERFACE_MODE_RGMII)
		p3060qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_RGMII2);
	p3060qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT1);
	p3060qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT2);
	p3060qds_mdio_init(DEFAULT_FM_MDIO_NAME, EMI1_SLOT3);

	if (fm_info_get_enet_if(FM1_DTSEC1) == PHY_INTERFACE_MODE_RGMII)
		fm_info_set_phy_address(FM1_DTSEC1, 1); /* RGMII1 */
	else if (fm_info_get_enet_if(FM1_DTSEC1) == PHY_INTERFACE_MODE_SGMII)
		fm_info_set_phy_address(FM1_DTSEC1, SGMII_CARD_PORT2_PHY_ADDR);

	if (fm_info_get_enet_if(FM1_DTSEC2) == PHY_INTERFACE_MODE_RGMII)
		fm_info_set_phy_address(FM1_DTSEC2, 2); /* RGMII2 */
	else if (fm_info_get_enet_if(FM1_DTSEC2) == PHY_INTERFACE_MODE_SGMII)
		fm_info_set_phy_address(FM1_DTSEC2, SGMII_CARD_PORT4_PHY_ADDR);

	fm_info_set_phy_address(FM2_DTSEC1, SGMII_CARD_PORT1_PHY_ADDR);
	fm_info_set_phy_address(FM2_DTSEC2, SGMII_CARD_PORT3_PHY_ADDR);

	switch (srds_cfg) {
	case 0x03:
	case 0x06:
		fm_info_set_phy_address(FM2_DTSEC3, SGMII_CARD_PORT3_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_PORT4_PHY_ADDR);
		fm_info_set_phy_address(FM2_DTSEC4, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC4, SGMII_CARD_PORT2_PHY_ADDR);
		break;
	case 0x16:
	case 0x19:
	case 0x1C:
		fm_info_set_phy_address(FM2_DTSEC3, SGMII_CARD_PORT1_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC3, SGMII_CARD_PORT2_PHY_ADDR);
		fm_info_set_phy_address(FM2_DTSEC4, SGMII_CARD_PORT3_PHY_ADDR);
		fm_info_set_phy_address(FM1_DTSEC4, SGMII_CARD_PORT4_PHY_ADDR);
		break;
	default:
		puts("Invalid SerDes protocol for P3060QDS\n");
		break;
	}

	for (i = FM1_DTSEC1; i < FM1_DTSEC1 + CONFIG_SYS_NUM_FM1_DTSEC; i++) {
		int idx = i - FM1_DTSEC1, lane, slot;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM1_DTSEC1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			if (QIXIS_READ(present) & (1 << (slot - 1)))
				fm_disable_port(i);
			switch (slot) {
			case 1:
				mdio_mux[i] = EMI1_SLOT1;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 2:
				mdio_mux[i] = EMI1_SLOT2;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 3:
				mdio_mux[i] = EMI1_SLOT3;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			};
			break;
		case PHY_INTERFACE_MODE_RGMII:
			if (i == FM1_DTSEC1) {
				mdio_mux[i] = EMI1_RGMII1;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
			} else if (i == FM1_DTSEC2) {
				mdio_mux[i] = EMI1_RGMII2;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
			}
			break;
		default:
			break;
		}
	}

#if (CONFIG_SYS_NUM_FMAN == 2)
	for (i = FM2_DTSEC1; i < FM2_DTSEC1 + CONFIG_SYS_NUM_FM2_DTSEC; i++) {
		int idx = i - FM2_DTSEC1, lane, slot;
		switch (fm_info_get_enet_if(i)) {
		case PHY_INTERFACE_MODE_SGMII:
			lane = serdes_get_first_lane(SGMII_FM2_DTSEC1 + idx);
			if (lane < 0)
				break;
			slot = lane_to_slot[lane];
			if (QIXIS_READ(present) & (1 << (slot - 1)))
				fm_disable_port(i);
			switch (slot) {
			case 1:
				mdio_mux[i] = EMI1_SLOT1;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 2:
				mdio_mux[i] = EMI1_SLOT2;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			case 3:
				mdio_mux[i] = EMI1_SLOT3;
				fm_info_set_mdio(i,
					mii_dev_for_muxval(mdio_mux[i]));
				break;
			};
			break;
		default:
			break;
		}
	}
#endif /* CONFIG_SYS_NUM_FMAN */

	cpu_eth_init(bis);
#endif /* CONFIG_FMAN_ENET */

	return pci_eth_init(bis);
}
