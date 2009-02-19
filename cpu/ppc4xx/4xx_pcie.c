/*
 * (C) Copyright 2006 - 2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (c) 2005 Cisco Systems.  All rights reserved.
 * Roland Dreier <rolandd@cisco.com>
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
 */

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <pci.h>
#include <ppc4xx.h>
#include <asm/processor.h>
#include <asm-ppc/io.h>

#if (defined(CONFIG_440SPE) || defined(CONFIG_405EX) ||	\
    defined(CONFIG_460EX) || defined(CONFIG_460GT)) && \
    defined(CONFIG_PCI) && !defined(CONFIG_PCI_DISABLE_PCIE)

#include <asm/4xx_pcie.h>

enum {
	PTYPE_ENDPOINT		= 0x0,
	PTYPE_LEGACY_ENDPOINT	= 0x1,
	PTYPE_ROOT_PORT		= 0x4,

	LNKW_X1			= 0x1,
	LNKW_X4			= 0x4,
	LNKW_X8			= 0x8
};

static int validate_endpoint(struct pci_controller *hose)
{
	if (hose->cfg_data == (u8 *)CONFIG_SYS_PCIE0_CFGBASE)
		return (is_end_point(0));
	else if (hose->cfg_data == (u8 *)CONFIG_SYS_PCIE1_CFGBASE)
		return (is_end_point(1));
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	else if (hose->cfg_data == (u8 *)CONFIG_SYS_PCIE2_CFGBASE)
		return (is_end_point(2));
#endif

	return 0;
}

static u8* pcie_get_base(struct pci_controller *hose, unsigned int devfn)
{
	u8 *base = (u8*)hose->cfg_data;

	/* use local configuration space for the first bus */
	if (PCI_BUS(devfn) == 0) {
		if (hose->cfg_data == (u8*)CONFIG_SYS_PCIE0_CFGBASE)
			base = (u8*)CONFIG_SYS_PCIE0_XCFGBASE;
		if (hose->cfg_data == (u8*)CONFIG_SYS_PCIE1_CFGBASE)
			base = (u8*)CONFIG_SYS_PCIE1_XCFGBASE;
#if CONFIG_SYS_PCIE_NR_PORTS > 2
		if (hose->cfg_data == (u8*)CONFIG_SYS_PCIE2_CFGBASE)
			base = (u8*)CONFIG_SYS_PCIE2_XCFGBASE;
#endif
	}

	return base;
}

static void pcie_dmer_disable(void)
{
	mtdcr (DCRN_PEGPL_CFG(DCRN_PCIE0_BASE),
		mfdcr (DCRN_PEGPL_CFG(DCRN_PCIE0_BASE)) | GPL_DMER_MASK_DISA);
	mtdcr (DCRN_PEGPL_CFG(DCRN_PCIE1_BASE),
		mfdcr (DCRN_PEGPL_CFG(DCRN_PCIE1_BASE)) | GPL_DMER_MASK_DISA);
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	mtdcr (DCRN_PEGPL_CFG(DCRN_PCIE2_BASE),
		mfdcr (DCRN_PEGPL_CFG(DCRN_PCIE2_BASE)) | GPL_DMER_MASK_DISA);
#endif
}

static void pcie_dmer_enable(void)
{
	mtdcr (DCRN_PEGPL_CFG (DCRN_PCIE0_BASE),
		mfdcr (DCRN_PEGPL_CFG(DCRN_PCIE0_BASE)) & ~GPL_DMER_MASK_DISA);
	mtdcr (DCRN_PEGPL_CFG (DCRN_PCIE1_BASE),
		mfdcr (DCRN_PEGPL_CFG(DCRN_PCIE1_BASE)) & ~GPL_DMER_MASK_DISA);
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	mtdcr (DCRN_PEGPL_CFG (DCRN_PCIE2_BASE),
		mfdcr (DCRN_PEGPL_CFG(DCRN_PCIE2_BASE)) & ~GPL_DMER_MASK_DISA);
#endif
}

static int pcie_read_config(struct pci_controller *hose, unsigned int devfn,
	int offset, int len, u32 *val) {

	u8 *address;
	*val = 0;

	if (validate_endpoint(hose))
		return 0;		/* No upstream config access */

	/*
	 * Bus numbers are relative to hose->first_busno
	 */
	devfn -= PCI_BDF(hose->first_busno, 0, 0);

	/*
	 * NOTICE: configuration space ranges are currenlty mapped only for
	 * the first 16 buses, so such limit must be imposed. In case more
	 * buses are required the TLB settings in board/amcc/<board>/init.S
	 * need to be altered accordingly (one bus takes 1 MB of memory space).
	 */
	if (PCI_BUS(devfn) >= 16)
		return 0;

	/*
	 * Only single device/single function is supported for the primary and
	 * secondary buses of the 440SPe host bridge.
	 */
	if ((!((PCI_FUNC(devfn) == 0) && (PCI_DEV(devfn) == 0))) &&
		((PCI_BUS(devfn) == 0) || (PCI_BUS(devfn) == 1)))
		return 0;

	address = pcie_get_base(hose, devfn);
	offset += devfn << 4;

	/*
	 * Reading from configuration space of non-existing device can
	 * generate transaction errors. For the read duration we suppress
	 * assertion of machine check exceptions to avoid those.
	 */
	pcie_dmer_disable ();

	debug("%s: cfg_data=%08x offset=%08x\n", __func__, hose->cfg_data, offset);
	switch (len) {
	case 1:
		*val = in_8(hose->cfg_data + offset);
		break;
	case 2:
		*val = in_le16((u16 *)(hose->cfg_data + offset));
		break;
	default:
		*val = in_le32((u32*)(hose->cfg_data + offset));
		break;
	}

	pcie_dmer_enable ();

	return 0;
}

static int pcie_write_config(struct pci_controller *hose, unsigned int devfn,
	int offset, int len, u32 val) {

	u8 *address;

	if (validate_endpoint(hose))
		return 0;		/* No upstream config access */

	/*
	 * Bus numbers are relative to hose->first_busno
	 */
	devfn -= PCI_BDF(hose->first_busno, 0, 0);

	/*
	 * Same constraints as in pcie_read_config().
	 */
	if (PCI_BUS(devfn) >= 16)
		return 0;

	if ((!((PCI_FUNC(devfn) == 0) && (PCI_DEV(devfn) == 0))) &&
		((PCI_BUS(devfn) == 0) || (PCI_BUS(devfn) == 1)))
		return 0;

	address = pcie_get_base(hose, devfn);
	offset += devfn << 4;

	/*
	 * Suppress MCK exceptions, similar to pcie_read_config()
	 */
	pcie_dmer_disable ();

	switch (len) {
	case 1:
		out_8(hose->cfg_data + offset, val);
		break;
	case 2:
		out_le16((u16 *)(hose->cfg_data + offset), val);
		break;
	default:
		out_le32((u32 *)(hose->cfg_data + offset), val);
		break;
	}

	pcie_dmer_enable ();

	return 0;
}

int pcie_read_config_byte(struct pci_controller *hose,pci_dev_t dev,int offset,u8 *val)
{
	u32 v;
	int rv;

	rv = pcie_read_config(hose, dev, offset, 1, &v);
	*val = (u8)v;
	return rv;
}

int pcie_read_config_word(struct pci_controller *hose,pci_dev_t dev,int offset,u16 *val)
{
	u32 v;
	int rv;

	rv = pcie_read_config(hose, dev, offset, 2, &v);
	*val = (u16)v;
	return rv;
}

int pcie_read_config_dword(struct pci_controller *hose,pci_dev_t dev,int offset,u32 *val)
{
	u32 v;
	int rv;

	rv = pcie_read_config(hose, dev, offset, 3, &v);
	*val = (u32)v;
	return rv;
}

int pcie_write_config_byte(struct pci_controller *hose,pci_dev_t dev,int offset,u8 val)
{
	return pcie_write_config(hose,(u32)dev,offset,1,val);
}

int pcie_write_config_word(struct pci_controller *hose,pci_dev_t dev,int offset,u16 val)
{
	return pcie_write_config(hose,(u32)dev,offset,2,(u32 )val);
}

int pcie_write_config_dword(struct pci_controller *hose,pci_dev_t dev,int offset,u32 val)
{
	return pcie_write_config(hose,(u32)dev,offset,3,(u32 )val);
}

#if defined(CONFIG_440SPE)
static void ppc4xx_setup_utl(u32 port) {

	volatile void *utl_base = NULL;

	/*
	 * Map UTL registers
	 */
	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE0), 0x0000000c);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE0), 0x20000000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE0), 0x00007001);
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE0), 0x68782800);
		break;

	case 1:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE1), 0x0000000c);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE1), 0x20001000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE1), 0x00007001);
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE1), 0x68782800);
		break;

	case 2:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE2), 0x0000000c);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE2), 0x20002000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE2), 0x00007001);
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE2), 0x68782800);
		break;
	}
	utl_base = (unsigned int *)(CONFIG_SYS_PCIE_BASE + 0x1000 * port);

	/*
	 * Set buffer allocations and then assert VRB and TXE.
	 */
	out_be32(utl_base + PEUTL_OUTTR,   0x08000000);
	out_be32(utl_base + PEUTL_INTR,    0x02000000);
	out_be32(utl_base + PEUTL_OPDBSZ,  0x10000000);
	out_be32(utl_base + PEUTL_PBBSZ,   0x53000000);
	out_be32(utl_base + PEUTL_IPHBSZ,  0x08000000);
	out_be32(utl_base + PEUTL_IPDBSZ,  0x10000000);
	out_be32(utl_base + PEUTL_RCIRQEN, 0x00f00000);
	out_be32(utl_base + PEUTL_PCTL,    0x80800066);
}

static int check_error(void)
{
	u32 valPE0, valPE1, valPE2;
	int err = 0;

	/* SDR0_PEGPLLLCT1 reset */
	if (!(valPE0 = SDR_READ(PESDR0_PLLLCT1) & 0x01000000))
		printf("PCIE: SDR0_PEGPLLLCT1 reset error 0x%x\n", valPE0);

	valPE0 = SDR_READ(PESDR0_RCSSET);
	valPE1 = SDR_READ(PESDR1_RCSSET);
	valPE2 = SDR_READ(PESDR2_RCSSET);

	/* SDR0_PExRCSSET rstgu */
	if (!(valPE0 & 0x01000000) ||
	    !(valPE1 & 0x01000000) ||
	    !(valPE2 & 0x01000000)) {
		printf("PCIE:  SDR0_PExRCSSET rstgu error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rstdl */
	if (!(valPE0 & 0x00010000) ||
	    !(valPE1 & 0x00010000) ||
	    !(valPE2 & 0x00010000)) {
		printf("PCIE:  SDR0_PExRCSSET rstdl error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rstpyn */
	if ((valPE0 & 0x00001000) ||
	    (valPE1 & 0x00001000) ||
	    (valPE2 & 0x00001000)) {
		printf("PCIE:  SDR0_PExRCSSET rstpyn error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET hldplb */
	if ((valPE0 & 0x10000000) ||
	    (valPE1 & 0x10000000) ||
	    (valPE2 & 0x10000000)) {
		printf("PCIE:  SDR0_PExRCSSET hldplb error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rdy */
	if ((valPE0 & 0x00100000) ||
	    (valPE1 & 0x00100000) ||
	    (valPE2 & 0x00100000)) {
		printf("PCIE:  SDR0_PExRCSSET rdy error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET shutdown */
	if ((valPE0 & 0x00000100) ||
	    (valPE1 & 0x00000100) ||
	    (valPE2 & 0x00000100)) {
		printf("PCIE:  SDR0_PExRCSSET shutdown error\n");
		err = -1;
	}
	return err;
}

/*
 * Initialize PCI Express core
 */
int ppc4xx_init_pcie(void)
{
	int time_out = 20;

	/* Set PLL clock receiver to LVPECL */
	SDR_WRITE(PESDR0_PLLLCT1, SDR_READ(PESDR0_PLLLCT1) | 1 << 28);

	if (check_error())
		return -1;

	if (!(SDR_READ(PESDR0_PLLLCT2) & 0x10000))
	{
		printf("PCIE: PESDR_PLLCT2 resistance calibration failed (0x%08x)\n",
		       SDR_READ(PESDR0_PLLLCT2));
		return -1;
	}
	/* De-assert reset of PCIe PLL, wait for lock */
	SDR_WRITE(PESDR0_PLLLCT1, SDR_READ(PESDR0_PLLLCT1) & ~(1 << 24));
	udelay(3);

	while (time_out) {
		if (!(SDR_READ(PESDR0_PLLLCT3) & 0x10000000)) {
			time_out--;
			udelay(1);
		} else
			break;
	}
	if (!time_out) {
		printf("PCIE: VCO output not locked\n");
		return -1;
	}
	return 0;
}
#endif

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
static void ppc4xx_setup_utl(u32 port)
{
	volatile void *utl_base = NULL;

	/*
	 * Map UTL registers at 0x0801_n000 (4K 0xfff mask) PEGPLn_REGMSK
	 */
	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE0), U64_TO_U32_HIGH(CONFIG_SYS_PCIE0_UTLBASE));
		mtdcr(DCRN_PEGPL_REGBAL(PCIE0), U64_TO_U32_LOW(CONFIG_SYS_PCIE0_UTLBASE));
		mtdcr(DCRN_PEGPL_REGMSK(PCIE0), 0x00007001);	/* BAM 11100000=4KB */
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE0), 0);
		break;

	case 1:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE1), U64_TO_U32_HIGH(CONFIG_SYS_PCIE0_UTLBASE));
		mtdcr(DCRN_PEGPL_REGBAL(PCIE1), U64_TO_U32_LOW(CONFIG_SYS_PCIE0_UTLBASE)
			+ 0x1000);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE1), 0x00007001);	/* BAM 11100000=4KB */
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE1), 0);
		break;
	}
	utl_base = (unsigned int *)(CONFIG_SYS_PCIE_BASE + 0x1000 * port);

	/*
	 * Set buffer allocations and then assert VRB and TXE.
	 */
	out_be32(utl_base + PEUTL_PBCTL, 0x0800000c);	/* PLBME, CRRE */
	out_be32(utl_base + PEUTL_OUTTR, 0x08000000);
	out_be32(utl_base + PEUTL_INTR, 0x02000000);
	out_be32(utl_base + PEUTL_OPDBSZ, 0x04000000);	/* OPD = 512 Bytes */
	out_be32(utl_base + PEUTL_PBBSZ, 0x00000000);	/* Max 512 Bytes */
	out_be32(utl_base + PEUTL_IPHBSZ, 0x02000000);
	out_be32(utl_base + PEUTL_IPDBSZ, 0x04000000);	/* IPD = 512 Bytes */
	out_be32(utl_base + PEUTL_RCIRQEN, 0x00f00000);
	out_be32(utl_base + PEUTL_PCTL, 0x80800066);	/* VRB,TXE,timeout=default */
}

/*
 * TODO: double check PCI express SDR based on the latest user manual
 *		 Some registers specified here no longer exist.. has to be
 *		 updated based on the final EAS spec.
 */
static int check_error(void)
{
	u32 valPE0, valPE1;
	int err = 0;

	valPE0 = SDR_READ(SDRN_PESDR_RCSSET(0));
	valPE1 = SDR_READ(SDRN_PESDR_RCSSET(1));

	/* SDR0_PExRCSSET rstgu */
	if (!(valPE0 & PESDRx_RCSSET_RSTGU) || !(valPE1 & PESDRx_RCSSET_RSTGU)) {
		printf("PCIE:  SDR0_PExRCSSET rstgu error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rstdl */
	if (!(valPE0 & PESDRx_RCSSET_RSTDL) || !(valPE1 & PESDRx_RCSSET_RSTDL)) {
		printf("PCIE:  SDR0_PExRCSSET rstdl error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rstpyn */
	if ((valPE0 & PESDRx_RCSSET_RSTPYN) || (valPE1 & PESDRx_RCSSET_RSTPYN)) {
		printf("PCIE:  SDR0_PExRCSSET rstpyn error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET hldplb */
	if ((valPE0 & PESDRx_RCSSET_HLDPLB) || (valPE1 & PESDRx_RCSSET_HLDPLB)) {
		printf("PCIE:  SDR0_PExRCSSET hldplb error\n");
		err = -1;
	}

	/* SDR0_PExRCSSET rdy */
	if ((valPE0 & PESDRx_RCSSET_RDY) || (valPE1 & PESDRx_RCSSET_RDY)) {
		printf("PCIE:  SDR0_PExRCSSET rdy error\n");
		err = -1;
	}

	return err;
}

/*
 * Initialize PCI Express core as described in User Manual
 * TODO: double check PE SDR PLL Register with the updated user manual.
 */
int ppc4xx_init_pcie(void)
{
	if (check_error())
		return -1;

	return 0;
}
#endif /* CONFIG_460EX */

#if defined(CONFIG_405EX)
static void ppc4xx_setup_utl(u32 port)
{
	u32 utl_base;

	/*
	 * Map UTL registers at 0xef4f_n000 (4K 0xfff mask) PEGPLn_REGMSK
	 */
	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE0), 0x00000000);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE0), CONFIG_SYS_PCIE0_UTLBASE);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE0), 0x00007001); /* 4k region, valid */
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE0), 0);
		break;

	case 1:
		mtdcr(DCRN_PEGPL_REGBAH(PCIE1), 0x00000000);
		mtdcr(DCRN_PEGPL_REGBAL(PCIE1), CONFIG_SYS_PCIE1_UTLBASE);
		mtdcr(DCRN_PEGPL_REGMSK(PCIE1), 0x00007001); /* 4k region, valid */
		mtdcr(DCRN_PEGPL_SPECIAL(PCIE1), 0);

		break;
	}
	utl_base = (port==0) ? CONFIG_SYS_PCIE0_UTLBASE : CONFIG_SYS_PCIE1_UTLBASE;

	/*
	 * Set buffer allocations and then assert VRB and TXE.
	 */
	out_be32((u32 *)(utl_base + PEUTL_OUTTR),   0x02000000);
	out_be32((u32 *)(utl_base + PEUTL_INTR),    0x02000000);
	out_be32((u32 *)(utl_base + PEUTL_OPDBSZ),  0x04000000);
	out_be32((u32 *)(utl_base + PEUTL_PBBSZ),   0x21000000);
	out_be32((u32 *)(utl_base + PEUTL_IPHBSZ),  0x02000000);
	out_be32((u32 *)(utl_base + PEUTL_IPDBSZ),  0x04000000);
	out_be32((u32 *)(utl_base + PEUTL_RCIRQEN), 0x00f00000);
	out_be32((u32 *)(utl_base + PEUTL_PCTL),    0x80800066);

	out_be32((u32 *)(utl_base + PEUTL_PBCTL),   0x0800000c);
	out_be32((u32 *)(utl_base + PEUTL_RCSTA),
		 in_be32((u32 *)(utl_base + PEUTL_RCSTA)) | 0x000040000);
}

int ppc4xx_init_pcie(void)
{
	/*
	 * Nothing to do on 405EX
	 */
	return 0;
}
#endif /* CONFIG_405EX */

/*
 * Board-specific pcie initialization
 * Platform code can reimplement ppc4xx_init_pcie_port_hw() if needed
 */

/*
 * Initialize various parts of the PCI Express core for our port:
 *
 * - Set as a root port and enable max width
 *   (PXIE0 -> X8, PCIE1 and PCIE2 -> X4).
 * - Set up UTL configuration.
 * - Increase SERDES drive strength to levels suggested by AMCC.
 * - De-assert RSTPYN, RSTDL and RSTGU.
 *
 * NOTICE for 440SPE revB chip: PESDRn_UTLSET2 is not set - we leave it
 * with default setting 0x11310000. The register has new fields,
 * PESDRn_UTLSET2[LKINE] in particular: clearing it leads to PCIE core
 * hang.
 */
#if defined(CONFIG_440SPE)
int __ppc4xx_init_pcie_port_hw(int port, int rootport)
{
	u32 val = 1 << 24;
	u32 utlset1;

	if (rootport) {
		val = PTYPE_ROOT_PORT << 20;
		utlset1 = 0x21222222;
	} else {
		val = PTYPE_LEGACY_ENDPOINT << 20;
		utlset1 = 0x20222222;
	}

	if (port == 0)
		val |= LNKW_X8 << 12;
	else
		val |= LNKW_X4 << 12;

	SDR_WRITE(SDRN_PESDR_DLPSET(port), val);
	SDR_WRITE(SDRN_PESDR_UTLSET1(port), utlset1);
	if (!ppc440spe_revB())
		SDR_WRITE(SDRN_PESDR_UTLSET2(port), 0x11000000);
	SDR_WRITE(SDRN_PESDR_HSSL0SET1(port), 0x35000000);
	SDR_WRITE(SDRN_PESDR_HSSL1SET1(port), 0x35000000);
	SDR_WRITE(SDRN_PESDR_HSSL2SET1(port), 0x35000000);
	SDR_WRITE(SDRN_PESDR_HSSL3SET1(port), 0x35000000);
	if (port == 0) {
		SDR_WRITE(PESDR0_HSSL4SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL5SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL6SET1, 0x35000000);
		SDR_WRITE(PESDR0_HSSL7SET1, 0x35000000);
	}
	SDR_WRITE(SDRN_PESDR_RCSSET(port), (SDR_READ(SDRN_PESDR_RCSSET(port)) &
					    ~(1 << 24 | 1 << 16)) | 1 << 12);

	return 0;
}
#endif /* CONFIG_440SPE */

#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
int __ppc4xx_init_pcie_port_hw(int port, int rootport)
{
	u32 val;
	u32 utlset1;

	if (rootport)
		val = PTYPE_ROOT_PORT << 20;
	else
		val = PTYPE_LEGACY_ENDPOINT << 20;

	if (port == 0) {
		val |= LNKW_X1 << 12;
		utlset1 = 0x20000000;
	} else {
		val |= LNKW_X4 << 12;
		utlset1 = 0x20101101;
	}

	SDR_WRITE(SDRN_PESDR_DLPSET(port), val);
	SDR_WRITE(SDRN_PESDR_UTLSET1(port), utlset1);
	SDR_WRITE(SDRN_PESDR_UTLSET2(port), 0x01210000);

	switch (port) {
	case 0:
		SDR_WRITE(PESDR0_L0CDRCTL, 0x00003230);
		SDR_WRITE(PESDR0_L0DRV, 0x00000130);
		SDR_WRITE(PESDR0_L0CLK, 0x00000006);

		SDR_WRITE(PESDR0_PHY_CTL_RST,0x10000000);
		break;

	case 1:
		SDR_WRITE(PESDR1_L0CDRCTL, 0x00003230);
		SDR_WRITE(PESDR1_L1CDRCTL, 0x00003230);
		SDR_WRITE(PESDR1_L2CDRCTL, 0x00003230);
		SDR_WRITE(PESDR1_L3CDRCTL, 0x00003230);
		SDR_WRITE(PESDR1_L0DRV, 0x00000130);
		SDR_WRITE(PESDR1_L1DRV, 0x00000130);
		SDR_WRITE(PESDR1_L2DRV, 0x00000130);
		SDR_WRITE(PESDR1_L3DRV, 0x00000130);
		SDR_WRITE(PESDR1_L0CLK, 0x00000006);
		SDR_WRITE(PESDR1_L1CLK, 0x00000006);
		SDR_WRITE(PESDR1_L2CLK, 0x00000006);
		SDR_WRITE(PESDR1_L3CLK, 0x00000006);

		SDR_WRITE(PESDR1_PHY_CTL_RST,0x10000000);
		break;
	}

	SDR_WRITE(SDRN_PESDR_RCSSET(port), SDR_READ(SDRN_PESDR_RCSSET(port)) |
		  (PESDRx_RCSSET_RSTGU | PESDRx_RCSSET_RSTPYN));

	/* Poll for PHY reset */
	switch (port) {
	case 0:
		while (!(SDR_READ(PESDR0_RSTSTA) & 0x1))
			udelay(10);
		break;
	case 1:
		while (!(SDR_READ(PESDR1_RSTSTA) & 0x1))
			udelay(10);
		break;
	}

	SDR_WRITE(SDRN_PESDR_RCSSET(port),
		  (SDR_READ(SDRN_PESDR_RCSSET(port)) &
		   ~(PESDRx_RCSSET_RSTGU | PESDRx_RCSSET_RSTDL)) |
		  PESDRx_RCSSET_RSTPYN);

	return 0;
}
#endif /* CONFIG_440SPE */

#if defined(CONFIG_405EX)
int __ppc4xx_init_pcie_port_hw(int port, int rootport)
{
	u32 val;

	if (rootport)
		val = 0x00401000;
	else
		val = 0x00101000;

	SDR_WRITE(SDRN_PESDR_DLPSET(port), val);
	SDR_WRITE(SDRN_PESDR_UTLSET1(port), 0x00000000);
	SDR_WRITE(SDRN_PESDR_UTLSET2(port), 0x01010000);
	SDR_WRITE(SDRN_PESDR_PHYSET1(port), 0x720F0000);
	SDR_WRITE(SDRN_PESDR_PHYSET2(port), 0x70600003);

	/* Assert the PE0_PHY reset */
	SDR_WRITE(SDRN_PESDR_RCSSET(port), 0x01010000);
	udelay(1000);

	/* deassert the PE0_hotreset */
	if (is_end_point(port))
		SDR_WRITE(SDRN_PESDR_RCSSET(port), 0x01111000);
	else
		SDR_WRITE(SDRN_PESDR_RCSSET(port), 0x01101000);

	/* poll for phy !reset */
	while (!(SDR_READ(SDRN_PESDR_PHYSTA(port)) & 0x00001000))
		;

	/* deassert the PE0_gpl_utl_reset */
	SDR_WRITE(SDRN_PESDR_RCSSET(port), 0x00101000);

	if (port == 0)
		mtdcr(DCRN_PEGPL_CFG(PCIE0), 0x10000000);  /* guarded on */
	else
		mtdcr(DCRN_PEGPL_CFG(PCIE1), 0x10000000);  /* guarded on */

	return 0;
}
#endif /* CONFIG_405EX */

int ppc4xx_init_pcie_port_hw(int port, int rootport)
__attribute__((weak, alias("__ppc4xx_init_pcie_port_hw")));

/*
 * We map PCI Express configuration access into the 512MB regions
 *
 * NOTICE: revB is very strict about PLB real addressess and ranges to
 * be mapped for config space; it seems to only work with d_nnnn_nnnn
 * range (hangs the core upon config transaction attempts when set
 * otherwise) while revA uses c_nnnn_nnnn.
 *
 * For 440SPe revA:
 *     PCIE0: 0xc_4000_0000
 *     PCIE1: 0xc_8000_0000
 *     PCIE2: 0xc_c000_0000
 *
 * For 440SPe revB:
 *     PCIE0: 0xd_0000_0000
 *     PCIE1: 0xd_2000_0000
 *     PCIE2: 0xd_4000_0000
 *
 * For 405EX:
 *     PCIE0: 0xa000_0000
 *     PCIE1: 0xc000_0000
 *
 * For 460EX/GT:
 *     PCIE0: 0xd_0000_0000
 *     PCIE1: 0xd_2000_0000
 */
static inline u64 ppc4xx_get_cfgaddr(int port)
{
#if defined(CONFIG_405EX)
	if (port == 0)
		return (u64)CONFIG_SYS_PCIE0_CFGBASE;
	else
		return (u64)CONFIG_SYS_PCIE1_CFGBASE;
#endif
#if defined(CONFIG_440SPE)
	if (ppc440spe_revB()) {
		switch (port) {
		default:	/* to satisfy compiler */
		case 0:
			return 0x0000000d00000000ULL;
		case 1:
			return 0x0000000d20000000ULL;
		case 2:
			return 0x0000000d40000000ULL;
		}
	} else {
		switch (port) {
		default:	/* to satisfy compiler */
		case 0:
			return 0x0000000c40000000ULL;
		case 1:
			return 0x0000000c80000000ULL;
		case 2:
			return 0x0000000cc0000000ULL;
		}
	}
#endif
#if defined(CONFIG_460EX) || defined(CONFIG_460GT)
	if (port == 0)
		return 0x0000000d00000000ULL;
	else
		return 0x0000000d20000000ULL;
#endif
}

/*
 *  4xx boards as end point and root point setup
 *                    and
 *    testing inbound and out bound windows
 *
 *  4xx boards can be plugged into another 4xx boards or you can get PCI-E
 *  cable which can be used to setup loop back from one port to another port.
 *  Please rememeber that unless there is a endpoint plugged in to root port it
 *  will not initialize. It is the same in case of endpoint , unless there is
 *  root port attached it will not initialize.
 *
 *  In this release of software all the PCI-E ports are configured as either
 *  endpoint or rootpoint.In future we will have support for selective ports
 *  setup as endpoint and root point in single board.
 *
 *  Once your board came up as root point , you can verify by reading
 *  /proc/bus/pci/devices. Where you can see the configuration registers
 *  of end point device attached to the port.
 *
 *  Enpoint cofiguration can be verified by connecting 4xx board to any
 *  host or another 4xx board. Then try to scan the device. In case of
 *  linux use "lspci" or appripriate os command.
 *
 *  How do I verify the inbound and out bound windows ? (4xx to 4xx)
 *  in this configuration inbound and outbound windows are setup to access
 *  sram memroy area. SRAM is at 0x4 0000 0000 , on PLB bus. This address
 *  is mapped at 0x90000000. From u-boot prompt write data 0xb000 0000,
 *  This is waere your POM(PLB out bound memory window) mapped. then
 *  read the data from other 4xx board's u-boot prompt at address
 *  0x9000 0000(SRAM). Data should match.
 *  In case of inbound , write data to u-boot command prompt at 0xb000 0000
 *  which is mapped to 0x4 0000 0000. Now on rootpoint yucca u-boot prompt check
 *  data at 0x9000 0000(SRAM).Data should match.
 */
int ppc4xx_init_pcie_port(int port, int rootport)
{
	static int core_init;
	volatile u32 val = 0;
	int attempts;
	u64 addr;
	u32 low, high;

	if (!core_init) {
		if (ppc4xx_init_pcie())
			return -1;
		++core_init;
	}

	/*
	 * Initialize various parts of the PCI Express core for our port
	 */
	ppc4xx_init_pcie_port_hw(port, rootport);

	/*
	 * Notice: the following delay has critical impact on device
	 * initialization - if too short (<50ms) the link doesn't get up.
	 */
	mdelay(100);

	val = SDR_READ(SDRN_PESDR_RCSSTS(port));
	if (val & (1 << 20)) {
		printf("PCIE%d: PGRST failed %08x\n", port, val);
		return -1;
	}

	/*
	 * Verify link is up
	 */
	val = SDR_READ(SDRN_PESDR_LOOP(port));
	if (!(val & 0x00001000)) {
		printf("PCIE%d: link is not up.\n", port);
		return -1;
	}

	/*
	 * Setup UTL registers - but only on revA!
	 * We use default settings for revB chip.
	 */
	if (!ppc440spe_revB())
		ppc4xx_setup_utl(port);

	/*
	 * We map PCI Express configuration access into the 512MB regions
	 */
	addr = ppc4xx_get_cfgaddr(port);
	low = U64_TO_U32_LOW(addr);
	high = U64_TO_U32_HIGH(addr);

	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_CFGBAH(PCIE0), high);
		mtdcr(DCRN_PEGPL_CFGBAL(PCIE0), low);
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE0), 0xe0000001); /* 512MB region, valid */
		break;
	case 1:
		mtdcr(DCRN_PEGPL_CFGBAH(PCIE1), high);
		mtdcr(DCRN_PEGPL_CFGBAL(PCIE1), low);
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE1), 0xe0000001); /* 512MB region, valid */
		break;
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	case 2:
		mtdcr(DCRN_PEGPL_CFGBAH(PCIE2), high);
		mtdcr(DCRN_PEGPL_CFGBAL(PCIE2), low);
		mtdcr(DCRN_PEGPL_CFGMSK(PCIE2), 0xe0000001); /* 512MB region, valid */
		break;
#endif
	}

	/*
	 * Check for VC0 active and assert RDY.
	 */
	attempts = 10;
	while(!(SDR_READ(SDRN_PESDR_RCSSTS(port)) & (1 << 16))) {
		if (!(attempts--)) {
			printf("PCIE%d: VC0 not active\n", port);
			return -1;
		}
		mdelay(1000);
	}
	SDR_WRITE(SDRN_PESDR_RCSSET(port),
		  SDR_READ(SDRN_PESDR_RCSSET(port)) | 1 << 20);
	mdelay(100);

	return 0;
}

int ppc4xx_init_pcie_rootport(int port)
{
	return ppc4xx_init_pcie_port(port, 1);
}

int ppc4xx_init_pcie_endport(int port)
{
	return ppc4xx_init_pcie_port(port, 0);
}

void ppc4xx_setup_pcie_rootpoint(struct pci_controller *hose, int port)
{
	volatile void *mbase = NULL;
	volatile void *rmbase = NULL;

	pci_set_ops(hose,
		    pcie_read_config_byte,
		    pcie_read_config_word,
		    pcie_read_config_dword,
		    pcie_write_config_byte,
		    pcie_write_config_word,
		    pcie_write_config_dword);

	switch (port) {
	case 0:
		mbase = (u32 *)CONFIG_SYS_PCIE0_XCFGBASE;
		rmbase = (u32 *)CONFIG_SYS_PCIE0_CFGBASE;
		hose->cfg_data = (u8 *)CONFIG_SYS_PCIE0_CFGBASE;
		break;
	case 1:
		mbase = (u32 *)CONFIG_SYS_PCIE1_XCFGBASE;
		rmbase = (u32 *)CONFIG_SYS_PCIE1_CFGBASE;
		hose->cfg_data = (u8 *)CONFIG_SYS_PCIE1_CFGBASE;
		break;
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	case 2:
		mbase = (u32 *)CONFIG_SYS_PCIE2_XCFGBASE;
		rmbase = (u32 *)CONFIG_SYS_PCIE2_CFGBASE;
		hose->cfg_data = (u8 *)CONFIG_SYS_PCIE2_CFGBASE;
		break;
#endif
	}

	/*
	 * Set bus numbers on our root port
	 */
	out_8((u8 *)mbase + PCI_PRIMARY_BUS, 0);
	out_8((u8 *)mbase + PCI_SECONDARY_BUS, 1);
	out_8((u8 *)mbase + PCI_SUBORDINATE_BUS, 1);

	/*
	 * Set up outbound translation to hose->mem_space from PLB
	 * addresses at an offset of 0xd_0000_0000.  We set the low
	 * bits of the mask to 11 to turn off splitting into 8
	 * subregions and to enable the outbound translation.
	 */
	out_le32(mbase + PECFG_POM0LAH, 0x00000000);
	out_le32(mbase + PECFG_POM0LAL, CONFIG_SYS_PCIE_MEMBASE +
		 port * CONFIG_SYS_PCIE_MEMSIZE);
	debug("PECFG_POM0LA=%08x.%08x\n", in_le32(mbase + PECFG_POM0LAH),
	      in_le32(mbase + PECFG_POM0LAL));

	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE0), CONFIG_SYS_PCIE_ADDR_HIGH);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE0), CONFIG_SYS_PCIE_MEMBASE +
		      port * CONFIG_SYS_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE0), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE0),
		      ~(CONFIG_SYS_PCIE_MEMSIZE - 1) | 3);
		debug("0:PEGPL_OMR1BA=%08x.%08x MSK=%08x.%08x\n",
		      mfdcr(DCRN_PEGPL_OMR1BAH(PCIE0)),
		      mfdcr(DCRN_PEGPL_OMR1BAL(PCIE0)),
		      mfdcr(DCRN_PEGPL_OMR1MSKH(PCIE0)),
		      mfdcr(DCRN_PEGPL_OMR1MSKL(PCIE0)));
		break;
	case 1:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE1), CONFIG_SYS_PCIE_ADDR_HIGH);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE1), CONFIG_SYS_PCIE_MEMBASE +
		      port * CONFIG_SYS_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE1), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE1),
		      ~(CONFIG_SYS_PCIE_MEMSIZE - 1) | 3);
		debug("1:PEGPL_OMR1BA=%08x.%08x MSK=%08x.%08x\n",
		      mfdcr(DCRN_PEGPL_OMR1BAH(PCIE1)),
		      mfdcr(DCRN_PEGPL_OMR1BAL(PCIE1)),
		      mfdcr(DCRN_PEGPL_OMR1MSKH(PCIE1)),
		      mfdcr(DCRN_PEGPL_OMR1MSKL(PCIE1)));
		break;
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	case 2:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE2), CONFIG_SYS_PCIE_ADDR_HIGH);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE2), CONFIG_SYS_PCIE_MEMBASE +
		      port * CONFIG_SYS_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE2), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE2),
		      ~(CONFIG_SYS_PCIE_MEMSIZE - 1) | 3);
		debug("2:PEGPL_OMR1BA=%08x.%08x MSK=%08x.%08x\n",
		      mfdcr(DCRN_PEGPL_OMR1BAH(PCIE2)),
		      mfdcr(DCRN_PEGPL_OMR1BAL(PCIE2)),
		      mfdcr(DCRN_PEGPL_OMR1MSKH(PCIE2)),
		      mfdcr(DCRN_PEGPL_OMR1MSKL(PCIE2)));
		break;
#endif
	}

	/* Set up 4GB inbound memory window at 0 */
	out_le32(mbase + PCI_BASE_ADDRESS_0, 0);
	out_le32(mbase + PCI_BASE_ADDRESS_1, 0);
	out_le32(mbase + PECFG_BAR0HMPA, 0x7ffffff);
	out_le32(mbase + PECFG_BAR0LMPA, 0);

	out_le32(mbase + PECFG_PIM01SAH, 0xffff0000);
	out_le32(mbase + PECFG_PIM01SAL, 0x00000000);
	out_le32(mbase + PECFG_PIM0LAL, 0);
	out_le32(mbase + PECFG_PIM0LAH, 0);
	out_le32(mbase + PECFG_PIM1LAL, 0x00000000);
	out_le32(mbase + PECFG_PIM1LAH, 0x00000004);
	out_le32(mbase + PECFG_PIMEN, 0x1);

	/* Enable I/O, Mem, and Busmaster cycles */
	out_le16((u16 *)(mbase + PCI_COMMAND),
		 in_le16((u16 *)(mbase + PCI_COMMAND)) |
		 PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	/* Set Device and Vendor Id */
	out_le16(mbase + 0x200, 0xaaa0 + port);
	out_le16(mbase + 0x202, 0xbed0 + port);

	/* Set Class Code to PCI-PCI bridge and Revision Id to 1 */
	out_le32(mbase + 0x208, 0x06040001);

	printf("PCIE%d: successfully set as root-complex\n", port);
}

int ppc4xx_setup_pcie_endpoint(struct pci_controller *hose, int port)
{
	volatile void *mbase = NULL;
	int attempts = 0;

	pci_set_ops(hose,
		    pcie_read_config_byte,
		    pcie_read_config_word,
		    pcie_read_config_dword,
		    pcie_write_config_byte,
		    pcie_write_config_word,
		    pcie_write_config_dword);

	switch (port) {
	case 0:
		mbase = (u32 *)CONFIG_SYS_PCIE0_XCFGBASE;
		hose->cfg_data = (u8 *)CONFIG_SYS_PCIE0_CFGBASE;
		break;
	case 1:
		mbase = (u32 *)CONFIG_SYS_PCIE1_XCFGBASE;
		hose->cfg_data = (u8 *)CONFIG_SYS_PCIE1_CFGBASE;
		break;
#if defined(CONFIG_SYS_PCIE2_CFGBASE)
	case 2:
		mbase = (u32 *)CONFIG_SYS_PCIE2_XCFGBASE;
		hose->cfg_data = (u8 *)CONFIG_SYS_PCIE2_CFGBASE;
		break;
#endif
	}

	/*
	 * Set up outbound translation to hose->mem_space from PLB
	 * addresses at an offset of 0xd_0000_0000.  We set the low
	 * bits of the mask to 11 to turn off splitting into 8
	 * subregions and to enable the outbound translation.
	 */
	out_le32(mbase + PECFG_POM0LAH, 0x00001ff8);
	out_le32(mbase + PECFG_POM0LAL, 0x00001000);

	switch (port) {
	case 0:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE0), CONFIG_SYS_PCIE_ADDR_HIGH);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE0), CONFIG_SYS_PCIE_MEMBASE +
		      port * CONFIG_SYS_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE0), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE0),
		      ~(CONFIG_SYS_PCIE_MEMSIZE - 1) | 3);
		break;
	case 1:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE1), CONFIG_SYS_PCIE_ADDR_HIGH);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE1), CONFIG_SYS_PCIE_MEMBASE +
		      port * CONFIG_SYS_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE1), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE1),
		      ~(CONFIG_SYS_PCIE_MEMSIZE - 1) | 3);
		break;
#if CONFIG_SYS_PCIE_NR_PORTS > 2
	case 2:
		mtdcr(DCRN_PEGPL_OMR1BAH(PCIE2), CONFIG_SYS_PCIE_ADDR_HIGH);
		mtdcr(DCRN_PEGPL_OMR1BAL(PCIE2), CONFIG_SYS_PCIE_MEMBASE +
		      port * CONFIG_SYS_PCIE_MEMSIZE);
		mtdcr(DCRN_PEGPL_OMR1MSKH(PCIE2), 0x7fffffff);
		mtdcr(DCRN_PEGPL_OMR1MSKL(PCIE2),
		      ~(CONFIG_SYS_PCIE_MEMSIZE - 1) | 3);
		break;
#endif
	}

	/* Set up 64MB inbound memory window at 0 */
	out_le32(mbase + PCI_BASE_ADDRESS_0, 0);
	out_le32(mbase + PCI_BASE_ADDRESS_1, 0);

	out_le32(mbase + PECFG_PIM01SAH, 0xffffffff);
	out_le32(mbase + PECFG_PIM01SAL, 0xfc000000);

	/* Setup BAR0 */
	out_le32(mbase + PECFG_BAR0HMPA, 0x7fffffff);
	out_le32(mbase + PECFG_BAR0LMPA, 0xfc000000 | PCI_BASE_ADDRESS_MEM_TYPE_64);

	/* Disable BAR1 & BAR2 */
	out_le32(mbase + PECFG_BAR1MPA, 0);
	out_le32(mbase + PECFG_BAR2HMPA, 0);
	out_le32(mbase + PECFG_BAR2LMPA, 0);

	out_le32(mbase + PECFG_PIM0LAL, U64_TO_U32_LOW(CONFIG_SYS_PCIE_INBOUND_BASE));
	out_le32(mbase + PECFG_PIM0LAH, U64_TO_U32_HIGH(CONFIG_SYS_PCIE_INBOUND_BASE));
	out_le32(mbase + PECFG_PIMEN, 0x1);

	/* Enable I/O, Mem, and Busmaster cycles */
	out_le16((u16 *)(mbase + PCI_COMMAND),
		 in_le16((u16 *)(mbase + PCI_COMMAND)) |
		 PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	out_le16(mbase + 0x200, 0xcaad);		/* Setting vendor ID */
	out_le16(mbase + 0x202, 0xfeed);		/* Setting device ID */

	/* Set Class Code to Processor/PPC */
	out_le32(mbase + 0x208, 0x0b200001);

	attempts = 10;
	while(!(SDR_READ(SDRN_PESDR_RCSSTS(port)) & (1 << 8))) {
		if (!(attempts--)) {
			printf("PCIE%d: BME not active\n", port);
			return -1;
		}
		mdelay(1000);
	}

	printf("PCIE%d: successfully set as endpoint\n", port);

	return 0;
}
#endif /* CONFIG_440SPE && CONFIG_PCI */
