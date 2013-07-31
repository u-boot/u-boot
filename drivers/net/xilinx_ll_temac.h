/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * LL_TEMAC interface
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */
#ifndef _XILINX_LL_TEMAC_
#define _XILINX_LL_TEMAC_

#include <config.h>
#include <net.h>
#include <phy.h>
#include <miiphy.h>

#include <asm/types.h>
#include <asm/byteorder.h>

#include "xilinx_ll_temac_sdma.h"

#if !defined(__BIG_ENDIAN)
# error LL_TEMAC requires big endianess
#endif

/*
 * TEMAC Memory and Register Definition
 *
 * [1]:	[0]/ip_documentation/xps_ll_temac.pdf
 *	page 19, Memory and Register Descriptions
 */
struct temac_reg {
	/* direct soft registers (low part) */
	u32 raf;	/* Reset and Address Filter */
	u32 tpf;	/* Transmit Pause Frame */
	u32 ifgp;	/* Transmit Inter Frame Gap Adjustment */
	u32 is;		/* Interrupt Status */
	u32 ip;		/* Interrupt Pending */
	u32 ie;		/* Interrupt Enable */
	u32 ttag;	/* Transmit VLAN Tag */
	u32 rtag;	/* Receive VLAN Tag */
	/* hard TEMAC registers */
	u32 msw;	/* Most Significant Word Data */
	u32 lsw;	/* Least Significant Word Data */
	u32 ctl;	/* Control */
	u32 rdy;	/* Ready Status */
	/* direct soft registers (high part) */
	u32 uawl;	/* Unicast Address Word Lower */
	u32 uawu;	/* Unicast Address Word Upper */
	u32 tpid0;	/* VLAN TPID Word 0 */
	u32 tpid1;	/* VLAN TPID Word 1 */
};

/* Reset and Address Filter Registers (raf), [1] p25 */
#define RAF_SR			(1 << 13)
#define RAF_EMFE		(1 << 12)
#define RAF_NFE			(1 << 11)
#define RAF_RVSTM_POS		9
#define RAF_RVSTM_MASK		(3 << RAF_RVSTM_POS)
#define RAF_TVSTM_POS		7
#define RAF_TVSTM_MASK		(3 << RAF_TVSTM_POS)
#define RAF_RVTM_POS		5
#define RAF_RVTM_MASK		(3 << RAF_RVTM_POS)
#define RAF_TVTM_POS		3
#define RAF_TVTM_MASK		(3 << RAF_TVTM_POS)
#define RAF_BCREJ		(1 << 2)
#define RAF_MCREJ		(1 << 1)
#define RAF_HTRST		(1 << 0)

/* Transmit Pause Frame Registers (tpf), [1] p28 */
#define TPF_TPFV_POS		0
#define TPF_TPFV_MASK		(0xFFFF << TPF_TPFV_POS)

/* Transmit Inter Frame Gap Adjustment Registers (ifgp), [1] p28 */
#define IFGP_POS		0
#define IFGP_MASK		(0xFF << IFGP_POS)

/* Interrupt Status, Pending, Enable Registers (is, ip, ie), [1] p29-33 */
#define ISPE_MR			(1 << 7)
#define ISPE_RDL		(1 << 6)
#define ISPE_TC			(1 << 5)
#define ISPE_RFO		(1 << 4)
#define ISPE_RR			(1 << 3)
#define ISPE_RC			(1 << 2)
#define ISPE_AN			(1 << 1)
#define ISPE_HAC		(1 << 0)

/* Transmit, Receive VLAN Tag Registers (ttag, rtag), [1] p34-35 */
#define TRTAG_TPID_POS		16
#define TRTAG_TPID_MASK		(0xFFFF << TRTAG_TPID_POS)
#define TRTAG_PRIO_POS		13
#define TRTAG_PRIO_MASK		(7 << TRTAG_PRIO_POS)
#define TRTAG_CFI		(1 << 12)
#define TRTAG_VID_POS		0
#define TRTAG_VID_MASK		(0xFFF << TRTAG_VID_POS)

/* Most, Least Significant Word Data Register (msw, lsw), [1] p46 */
#define MLSW_POS		0
#define MLSW_MASK		(~0UL << MLSW_POS)

/* LSW Data Register for PHY addresses (lsw), [1] p66 */
#define LSW_REGAD_POS		0
#define LSW_REGAD_MASK		(0x1F << LSW_REGAD_POS)
#define LSW_PHYAD_POS		5
#define LSW_PHYAD_MASK		(0x1F << LSW_PHYAD_POS)

/* LSW Data Register for PHY data (lsw), [1] p66 */
#define LSW_REGDAT_POS		0
#define LSW_REGDAT_MASK		(0xFFFF << LSW_REGDAT_POS)

/* Control Register (ctl), [1] p47 */
#define CTL_WEN			(1 << 15)
#define CTL_ADDR_POS		0
#define CTL_ADDR_MASK		(0x3FF << CTL_ADDR_POS)

/* Ready Status Register Ethernet (rdy), [1] p48 */
#define RSE_HACS_RDY		(1 << 14)
#define RSE_CFG_WR		(1 << 6)
#define RSE_CFG_RR		(1 << 5)
#define RSE_AF_WR		(1 << 4)
#define RSE_AF_RR		(1 << 3)
#define RSE_MIIM_WR		(1 << 2)
#define RSE_MIIM_RR		(1 << 1)
#define RSE_FABR_RR		(1 << 0)

/* Unicast Address Word Lower, Upper Registers (uawl, uawu), [1] p35-36 */
#define UAWL_UADDR_POS		0
#define UAWL_UADDR_MASK		(~0UL << UAWL_UADDR_POS)
#define UAWU_UADDR_POS		0
#define UAWU_UADDR_MASK		(0xFFFF << UAWU_UADDR_POS)

/* VLAN TPID Word 0, 1 Registers (tpid0, tpid1), [1] p37 */
#define TPID0_V0_POS		0
#define TPID0_V0_MASK		(0xFFFF << TPID0_V0_POS)
#define TPID0_V1_POS		16
#define TPID0_V1_MASK		(0xFFFF << TPID0_V1_POS)
#define TPID1_V2_POS		0
#define TPID1_V2_MASK		(0xFFFF << TPID1_V2_POS)
#define TPID1_V3_POS		16
#define TPID1_V3_MASK		(0xFFFF << TPID1_V3_POS)

/*
 * TEMAC Indirectly Addressable Register Index Enumeration
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [1]:	[0]/ip_documentation/xps_ll_temac.pdf
 *	page 23, PLB Indirectly Addressable TEMAC Registers
 */
enum temac_ctrl {
	TEMAC_RCW0	= 0x200,
	TEMAC_RCW1	= 0x240,
	TEMAC_TC	= 0x280,
	TEMAC_FCC	= 0x2C0,
	TEMAC_EMMC	= 0x300,
	TEMAC_PHYC	= 0x320,
	TEMAC_MC	= 0x340,
	TEMAC_UAW0	= 0x380,
	TEMAC_UAW1	= 0x384,
	TEMAC_MAW0	= 0x388,
	TEMAC_MAW1	= 0x38C,
	TEMAC_AFM	= 0x390,
	TEMAC_TIS	= 0x3A0,
	TEMAC_TIE	= 0x3A4,
	TEMAC_MIIMWD	= 0x3B0,
	TEMAC_MIIMAI	= 0x3B4
};

/* Receive Configuration Word 0, 1 Registers (RCW0, RCW1), [1] p50-51 */
#define RCW0_PADDR_POS		0
#define RCW0_PADDR_MASK		(~0UL << RCW_PADDR_POS)
#define RCW1_RST		(1 << 31)
#define RCW1_JUM		(1 << 30)
#define RCW1_FCS		(1 << 29)
#define RCW1_RX			(1 << 28)
#define RCW1_VLAN		(1 << 27)
#define RCW1_HD			(1 << 26)
#define RCW1_LT_DIS		(1 << 25)
#define RCW1_PADDR_POS		0
#define RCW1_PADDR_MASK		(0xFFFF << RCW_PADDR_POS)

/* Transmit Configuration Registers (TC), [1] p52 */
#define TC_RST			(1 << 31)
#define TC_JUM			(1 << 30)
#define TC_FCS			(1 << 29)
#define TC_TX			(1 << 28)
#define TC_VLAN			(1 << 27)
#define TC_HD			(1 << 26)
#define TC_IFG			(1 << 25)

/* Flow Control Configuration Registers (FCC), [1] p54 */
#define FCC_FCTX		(1 << 30)
#define FCC_FCRX		(1 << 29)

/* Ethernet MAC Mode Configuration Registers (EMMC), [1] p54 */
#define EMMC_LSPD_POS		30
#define EMMC_LSPD_MASK		(3 << EMMC_LSPD_POS)
#define EMMC_LSPD_1000		(2 << EMMC_LSPD_POS)
#define EMMC_LSPD_100		(1 << EMMC_LSPD_POS)
#define EMMC_LSPD_10		0
#define EMMC_RGMII		(1 << 29)
#define EMMC_SGMII		(1 << 28)
#define EMMC_GPCS		(1 << 27)
#define EMMC_HOST		(1 << 26)
#define EMMC_TX16		(1 << 25)
#define EMMC_RX16		(1 << 24)

/* RGMII/SGMII Configuration Registers (PHYC), [1] p56 */
#define PHYC_SLSPD_POS		30
#define PHYC_SLSPD_MASK		(3 << EMMC_SLSPD_POS)
#define PHYC_SLSPD_1000		(2 << EMMC_SLSPD_POS)
#define PHYC_SLSPD_100		(1 << EMMC_SLSPD_POS)
#define PHYC_SLSPD_10		0
#define PHYC_RLSPD_POS		2
#define PHYC_RLSPD_MASK		(3 << EMMC_RLSPD_POS)
#define PHYC_RLSPD_1000		(2 << EMMC_RLSPD_POS)
#define PHYC_RLSPD_100		(1 << EMMC_RLSPD_POS)
#define PHYC_RLSPD_10		0
#define PHYC_RGMII_HD		(1 << 1)
#define PHYC_RGMII_LINK		(1 << 0)

/* Management Configuration Registers (MC), [1] p57 */
#define MC_MDIOEN		(1 << 6)
#define MC_CLKDIV_POS		0
#define MC_CLKDIV_MASK		(0x3F << MC_CLKDIV_POS)

/*
 *             fHOSTCLK          fMDC =                  fHOSTCLK
 * fMDC = -------------------   --------->   MC_CLKDIV = -------- - 1
 *        (1 + MC_CLKDIV) * 2    2.5 MHz                   5MHz
 */
#define MC_CLKDIV(f, m)		((f / (2 * m)) - 1)
#define MC_CLKDIV_25(f)		MC_CLKDIV(f, 2500000)
#define MC_CLKDIV_20(f)		MC_CLKDIV(f, 2000000)
#define MC_CLKDIV_15(f)		MC_CLKDIV(f, 1500000)
#define MC_CLKDIV_10(f)		MC_CLKDIV(f, 1000000)

/* Unicast Address Word 0, 1 Registers (UAW0, UAW1), [1] p58-59 */
#define UAW0_UADDR_POS		0
#define UAW0_UADDR_MASK		(~0UL << UAW0_UADDR_POS)
#define UAW1_UADDR_POS		0
#define UAW1_UADDR_MASK		(0xFFFF << UAW1_UADDR_POS)

/* Multicast Address Word 0, 1 Registers (MAW0, MAW1), [1] p60 */
#define MAW0_MADDR_POS		0
#define MAW0_MADDR_MASK		(~0UL << MAW0_MADDR_POS)
#define MAW1_RNW		(1 << 23)
#define MAW1_MAIDX_POS		16
#define MAW1_MAIDX_MASK		(3 << MAW1_MAIDX_POS)
#define MAW1_MADDR_POS		0
#define MAW1_MADDR_MASK		(0xFFFF << MAW1_MADDR_POS)

/* Address Filter Mode Registers (AFM), [1] p63 */
#define AFM_PM			(1 << 31)

/* Interrupt Status, Enable Registers (TIS, TIE), [1] p63-65 */
#define TISE_CFG_W		(1 << 6)
#define TISE_CFG_R		(1 << 5)
#define TISE_AF_W		(1 << 4)
#define TISE_AF_R		(1 << 3)
#define TISE_MIIM_W		(1 << 2)
#define TISE_MIIM_R		(1 << 1)
#define TISE_FABR_R		(1 << 0)

/* MII Management Write Data Registers (MIIMWD), [1] p66 */
#define MIIMWD_DATA_POS		0
#define MIIMWD_DATA_MASK	(0xFFFF << MIIMWD_DATA_POS)

/* Ethernet interface ready status */
int ll_temac_check_status(struct temac_reg *regs, u32 mask);

/* Indirect write to ll_temac. */
int ll_temac_indirect_set(struct temac_reg *regs, u16 regn, u32 reg_data);

/* Indirect read from ll_temac. */
int ll_temac_indirect_get(struct temac_reg *regs, u16 regn, u32* reg_data);

struct ll_temac {
	phys_addr_t		ctrladdr;
	phys_addr_t		sdma_reg_addr[SDMA_CTRL_REGNUMS];

	unsigned		(*in32)(phys_addr_t);
	void			(*out32)(phys_addr_t, unsigned);

	int			(*ctrlinit) (struct eth_device *);
	int			(*ctrlhalt) (struct eth_device *);
	int			(*ctrlreset) (struct eth_device *);

	int			phyaddr;
	struct phy_device	*phydev;
	struct mii_dev		*bus;
	char			mdio_busname[MDIO_NAME_LEN];
};

#endif /* _XILINX_LL_TEMAC_ */
