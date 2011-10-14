/*
 * Copyright 2009-2011 Freescale Semiconductor, Inc.
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

#ifndef _FSL_LIODN_H_
#define _FSL_LIODN_H_

#include <asm/types.h>

struct srio_liodn_id_table {
	u32 id[2];
	unsigned long reg_offset[2];
	u8 num_ids;
	u8 portid;
};
#define SET_SRIO_LIODN_1(port, idA) \
	{ .id = { idA }, .num_ids = 1, .portid = port, \
	  .reg_offset[0] = offsetof(ccsr_gur_t, rio##port##liodnr) \
		+ CONFIG_SYS_MPC85xx_GUTS_OFFSET + CONFIG_SYS_CCSRBAR, \
	}

#define SET_SRIO_LIODN_2(port, idA, idB) \
	{ .id = { idA, idB }, .num_ids = 2, .portid = port, \
	  .reg_offset[0] = offsetof(ccsr_gur_t, rio##port##liodnr) \
		+ CONFIG_SYS_MPC85xx_GUTS_OFFSET + CONFIG_SYS_CCSRBAR, \
	  .reg_offset[1] = offsetof(ccsr_gur_t, rio##port##maintliodnr) \
		+ CONFIG_SYS_MPC85xx_GUTS_OFFSET + CONFIG_SYS_CCSRBAR, \
	}

struct liodn_id_table {
	const char * compat;
	u32 id[2];
	u8 num_ids;
	phys_addr_t compat_offset;
	unsigned long reg_offset;
};

extern u32 get_ppid_liodn(int ppid_tbl_idx, int ppid);
extern void set_liodns(void);
extern void fdt_fixup_liodn(void *blob);

#define SET_LIODN_BASE_1(idA) \
	{ .id = { idA }, .num_ids = 1, }

#define SET_LIODN_BASE_2(idA, idB) \
	{ .id = { idA, idB }, .num_ids = 2 }

#define SET_LIODN_ENTRY_1(name, idA, off, compatoff) \
	{ .compat = name, \
	  .id = { idA }, .num_ids = 1, \
	  .reg_offset = off + CONFIG_SYS_CCSRBAR, \
	  .compat_offset = compatoff + CONFIG_SYS_CCSRBAR_PHYS, \
	}

#define SET_LIODN_ENTRY_2(name, idA, idB, off, compatoff) \
	{ .compat = name, \
	  .id = { idA, idB }, .num_ids = 2, \
	  .reg_offset = off + CONFIG_SYS_CCSRBAR, \
	  .compat_offset = compatoff + CONFIG_SYS_CCSRBAR_PHYS, \
	}

#define SET_GUTS_LIODN(compat, liodn, name, compatoff) \
	SET_LIODN_ENTRY_1(compat, liodn, \
		offsetof(ccsr_gur_t, name) + CONFIG_SYS_MPC85xx_GUTS_OFFSET, \
		compatoff)

#define SET_USB_LIODN(usbNum, compat, liodn) \
	SET_GUTS_LIODN(compat, liodn, usb##usbNum##liodnr,\
		CONFIG_SYS_MPC85xx_USB##usbNum##_OFFSET)

#define SET_SATA_LIODN(sataNum, liodn) \
	SET_GUTS_LIODN("fsl,pq-sata-v2", liodn, sata##sataNum##liodnr,\
		CONFIG_SYS_MPC85xx_SATA##sataNum##_OFFSET)

#define SET_PCI_LIODN(compat, pciNum, liodn) \
	SET_GUTS_LIODN(compat, liodn, pex##pciNum##liodnr,\
		CONFIG_SYS_MPC85xx_PCIE##pciNum##_OFFSET)

/* reg nodes for DMA start @ 0x300 */
#define SET_DMA_LIODN(dmaNum, liodn) \
	SET_GUTS_LIODN("fsl,eloplus-dma", liodn, dma##dmaNum##liodnr,\
		CONFIG_SYS_MPC85xx_DMA##dmaNum##_OFFSET + 0x300)

#define SET_SDHC_LIODN(sdhcNum, liodn) \
	SET_GUTS_LIODN("fsl,esdhc", liodn, sdmmc##sdhcNum##liodnr,\
		CONFIG_SYS_MPC85xx_ESDHC_OFFSET)

#define SET_QMAN_LIODN(liodn) \
	SET_LIODN_ENTRY_1("fsl,qman", liodn, offsetof(ccsr_qman_t, liodnr) + \
		CONFIG_SYS_FSL_QMAN_OFFSET, \
		CONFIG_SYS_FSL_QMAN_OFFSET)

#define SET_BMAN_LIODN(liodn) \
	SET_LIODN_ENTRY_1("fsl,bman", liodn, offsetof(ccsr_bman_t, liodnr) + \
		CONFIG_SYS_FSL_BMAN_OFFSET, \
		CONFIG_SYS_FSL_BMAN_OFFSET)

#define SET_PME_LIODN(liodn) \
	SET_LIODN_ENTRY_1("fsl,pme", liodn, offsetof(ccsr_pme_t, liodnr) + \
		CONFIG_SYS_FSL_CORENET_PME_OFFSET, \
		CONFIG_SYS_FSL_CORENET_PME_OFFSET)

/* -1 from portID due to how immap has the registers */
#define FM_PPID_RX_PORT_OFFSET(fmNum, portID) \
	CONFIG_SYS_FSL_FM##fmNum##_OFFSET + \
	offsetof(struct ccsr_fman, fm_bmi_common.fmbm_ppid[portID - 1])

/* enetNum is 0, 1, 2... so we + 8 for 1g to get to HW Port ID */
#define SET_FMAN_RX_1G_LIODN(fmNum, enetNum, liodn) \
	SET_LIODN_ENTRY_1("fsl,fman-port-1g-rx", liodn, \
		FM_PPID_RX_PORT_OFFSET(fmNum, enetNum + 8), \
		CONFIG_SYS_FSL_FM##fmNum##_RX##enetNum##_1G_OFFSET) \

/* enetNum is 0, 1, 2... so we + 16 for 10g to get to HW Port ID */
#define SET_FMAN_RX_10G_LIODN(fmNum, enetNum, liodn) \
	SET_LIODN_ENTRY_1("fsl,fman-port-10g-rx", liodn, \
		FM_PPID_RX_PORT_OFFSET(fmNum, enetNum + 16), \
		CONFIG_SYS_FSL_FM##fmNum##_RX##enetNum##_10G_OFFSET) \

/*
 * handle both old and new versioned SEC properties:
 * "fsl,secX.Y" became "fsl,sec-vX.Y" during development
 */
#define SET_SEC_JR_LIODN_ENTRY(jrNum, liodnA, liodnB) \
	SET_LIODN_ENTRY_2("fsl,sec4.0-job-ring", liodnA, liodnB,\
		offsetof(ccsr_sec_t, jrliodnr[jrNum].ls) + \
		CONFIG_SYS_FSL_SEC_OFFSET, \
		CONFIG_SYS_FSL_SEC_OFFSET + 0x1000 + 0x1000 * jrNum), \
	SET_LIODN_ENTRY_2("fsl,sec-v4.0-job-ring", liodnA, liodnB,\
		offsetof(ccsr_sec_t, jrliodnr[jrNum].ls) + \
		CONFIG_SYS_FSL_SEC_OFFSET, \
		CONFIG_SYS_FSL_SEC_OFFSET + 0x1000 + 0x1000 * jrNum)

/* This is a bit evil since we treat rtic param as both a string & hex value */
#define SET_SEC_RTIC_LIODN_ENTRY(rtic, liodnA) \
	SET_LIODN_ENTRY_1("fsl,sec4.0-rtic-memory", \
		liodnA,	\
		offsetof(ccsr_sec_t, rticliodnr[0x##rtic-0xa].ls) + \
		CONFIG_SYS_FSL_SEC_OFFSET, \
		CONFIG_SYS_FSL_SEC_OFFSET + 0x6100 + 0x20 * (0x##rtic-0xa)), \
	SET_LIODN_ENTRY_1("fsl,sec-v4.0-rtic-memory", \
		liodnA,	\
		offsetof(ccsr_sec_t, rticliodnr[0x##rtic-0xa].ls) + \
		CONFIG_SYS_FSL_SEC_OFFSET, \
		CONFIG_SYS_FSL_SEC_OFFSET + 0x6100 + 0x20 * (0x##rtic-0xa))

#define SET_SEC_DECO_LIODN_ENTRY(num, liodnA, liodnB) \
	SET_LIODN_ENTRY_2(NULL, liodnA, liodnB, \
		offsetof(ccsr_sec_t, decoliodnr[num].ls) + \
		CONFIG_SYS_FSL_SEC_OFFSET, 0)

#define SET_RAID_ENGINE_JQ_LIODN_ENTRY(jqNum, rNum, liodnA) \
	SET_LIODN_ENTRY_1("fsl,raideng-v1.0-job-ring", \
	liodnA, \
	offsetof(struct ccsr_raide, jq[jqNum].ring[rNum].cfg1) + \
	CONFIG_SYS_FSL_RAID_ENGINE_OFFSET, \
	offsetof(struct ccsr_raide, jq[jqNum].ring[rNum].cfg0) + \
	CONFIG_SYS_FSL_RAID_ENGINE_OFFSET)

#define SET_RMAN_LIODN(ibNum, liodn) \
	SET_LIODN_ENTRY_1("fsl,rman-inbound-block", liodn, \
		offsetof(struct ccsr_rman, mmitdr) + \
		CONFIG_SYS_FSL_CORENET_RMAN_OFFSET, \
		CONFIG_SYS_FSL_CORENET_RMAN_OFFSET + ibNum * 0x1000)

extern struct liodn_id_table liodn_tbl[], liodn_bases[], sec_liodn_tbl[];
extern struct liodn_id_table raide_liodn_tbl[];
extern struct liodn_id_table fman1_liodn_tbl[], fman2_liodn_tbl[];
extern struct srio_liodn_id_table srio_liodn_tbl[];
extern struct liodn_id_table rman_liodn_tbl[];
extern int liodn_tbl_sz, sec_liodn_tbl_sz, raide_liodn_tbl_sz;
extern int fman1_liodn_tbl_sz, fman2_liodn_tbl_sz;
extern int srio_liodn_tbl_sz;
extern int rman_liodn_tbl_sz;

#endif
