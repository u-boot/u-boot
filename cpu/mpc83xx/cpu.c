/*
 * Copyright (C) 2004-2006 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * CPU specific code for the MPC83xx family.
 *
 * Derived from the MPC8260 and MPC85xx.
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mpc83xx.h>
#include <asm/processor.h>
#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#elif defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <libfdt_env.h>
#endif

DECLARE_GLOBAL_DATA_PTR;


int checkcpu(void)
{
	volatile immap_t *immr;
	ulong clock = gd->cpu_clk;
	u32 pvr = get_pvr();
	u32 spridr;
	char buf[32];

	immr = (immap_t *)CFG_IMMR;

	puts("CPU:   ");

	switch (pvr & 0xffff0000) {
		case PVR_E300C1:
			printf("e300c1, ");
			break;

		case PVR_E300C2:
			printf("e300c2, ");
			break;

		case PVR_E300C3:
			printf("e300c3, ");
			break;

		default:
			printf("Unknown core, ");
	}

	spridr = immr->sysconf.spridr;
	switch(spridr) {
	case SPR_8349E_REV10:
	case SPR_8349E_REV11:
	case SPR_8349E_REV31:
		puts("MPC8349E, ");
		break;
	case SPR_8349_REV10:
	case SPR_8349_REV11:
	case SPR_8349_REV31:
		puts("MPC8349, ");
		break;
	case SPR_8347E_REV10_TBGA:
	case SPR_8347E_REV11_TBGA:
	case SPR_8347E_REV31_TBGA:
	case SPR_8347E_REV10_PBGA:
	case SPR_8347E_REV11_PBGA:
	case SPR_8347E_REV31_PBGA:
		puts("MPC8347E, ");
		break;
	case SPR_8347_REV10_TBGA:
	case SPR_8347_REV11_TBGA:
	case SPR_8347_REV31_TBGA:
	case SPR_8347_REV10_PBGA:
	case SPR_8347_REV11_PBGA:
	case SPR_8347_REV31_PBGA:
		puts("MPC8347, ");
		break;
	case SPR_8343E_REV10:
	case SPR_8343E_REV11:
	case SPR_8343E_REV31:
		puts("MPC8343E, ");
		break;
	case SPR_8343_REV10:
	case SPR_8343_REV11:
	case SPR_8343_REV31:
		puts("MPC8343, ");
		break;
	case SPR_8360E_REV10:
	case SPR_8360E_REV11:
	case SPR_8360E_REV12:
	case SPR_8360E_REV20:
	case SPR_8360E_REV21:
		puts("MPC8360E, ");
		break;
	case SPR_8360_REV10:
	case SPR_8360_REV11:
	case SPR_8360_REV12:
	case SPR_8360_REV20:
	case SPR_8360_REV21:
		puts("MPC8360, ");
		break;
	case SPR_8323E_REV10:
	case SPR_8323E_REV11:
		puts("MPC8323E, ");
		break;
	case SPR_8323_REV10:
	case SPR_8323_REV11:
		puts("MPC8323, ");
		break;
	case SPR_8321E_REV10:
	case SPR_8321E_REV11:
		puts("MPC8321E, ");
		break;
	case SPR_8321_REV10:
	case SPR_8321_REV11:
		puts("MPC8321, ");
		break;
	case SPR_8311_REV10:
		puts("MPC8311, ");
		break;
	case SPR_8311E_REV10:
		puts("MPC8311E, ");
		break;
	case SPR_8313_REV10:
		puts("MPC8313, ");
		break;
	case SPR_8313E_REV10:
		puts("MPC8313E, ");
		break;
	default:
		printf("Rev: Unknown revision number:%08x\n"
			"Warning: Unsupported cpu revision!\n",spridr);
		return 0;
	}

#if defined(CONFIG_MPC834X)
	/* Multiple revisons of 834x processors may have the same SPRIDR value.
	 * So use PVR to identify the revision number.
	 */
	printf("Rev: %02x at %s MHz", PVR_MAJ(pvr)<<4 | PVR_MIN(pvr), strmhz(buf, clock));
#else
	printf("Rev: %02x at %s MHz", spridr & 0x0000FFFF, strmhz(buf, clock));
#endif
	printf(", CSB: %4d MHz\n", gd->csb_clk / 1000000);

	return 0;
}


/*
 * Program a UPM with the code supplied in the table.
 *
 * The 'dummy' variable is used to increment the MAD. 'dummy' is
 * supposed to be a pointer to the memory of the device being
 * programmed by the UPM.  The data in the MDR is written into
 * memory and the MAD is incremented every time there's a read
 * from 'dummy'. Unfortunately, the current prototype for this
 * function doesn't allow for passing the address of this
 * device, and changing the prototype will break a number lots
 * of other code, so we need to use a round-about way of finding
 * the value for 'dummy'.
 *
 * The value can be extracted from the base address bits of the
 * Base Register (BR) associated with the specific UPM.  To find
 * that BR, we need to scan all 8 BRs until we find the one that
 * has its MSEL bits matching the UPM we want.  Once we know the
 * right BR, we can extract the base address bits from it.
 *
 * The MxMR and the BR and OR of the chosen bank should all be
 * configured before calling this function.
 *
 * Parameters:
 * upm: 0=UPMA, 1=UPMB, 2=UPMC
 * table: Pointer to an array of values to program
 * size: Number of elements in the array.  Must be 64 or less.
 */
void upmconfig (uint upm, uint *table, uint size)
{
#if defined(CONFIG_MPC834X)
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile lbus83xx_t *lbus = &immap->lbus;
	volatile uchar *dummy = NULL;
	const u32 msel = (upm + 4) << BR_MSEL_SHIFT;	/* What the MSEL field in BRn should be */
	volatile u32 *mxmr = &lbus->mamr + upm;	/* Pointer to mamr, mbmr, or mcmr */
	uint i;

	/* Scan all the banks to determine the base address of the device */
	for (i = 0; i < 8; i++) {
		if ((lbus->bank[i].br & BR_MSEL) == msel) {
			dummy = (uchar *) (lbus->bank[i].br & BR_BA);
			break;
		}
	}

	if (!dummy) {
		printf("Error: %s() could not find matching BR\n", __FUNCTION__);
		hang();
	}

	/* Set the OP field in the MxMR to "write" and the MAD field to 000000 */
	*mxmr = (*mxmr & 0xCFFFFFC0) | 0x10000000;

	for (i = 0; i < size; i++) {
		lbus->mdr = table[i];
		__asm__ __volatile__ ("sync");
		*dummy;	/* Write the value to memory and increment MAD */
		__asm__ __volatile__ ("sync");
	}

	/* Set the OP field in the MxMR to "normal" and the MAD field to 000000 */
	*mxmr &= 0xCFFFFFC0;
#else
	printf("Error: %s() not defined for this configuration.\n", __FUNCTION__);
	hang();
#endif
}


int
do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong msr;
#ifndef MPC83xx_RESET
	ulong addr;
#endif

	volatile immap_t *immap = (immap_t *) CFG_IMMR;

#ifdef MPC83xx_RESET
	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~( MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/* enable Reset Control Reg */
	immap->reset.rpr = 0x52535445;
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* confirm Reset Control Reg is enabled */
	while(!((immap->reset.rcer) & RCER_CRE));

	printf("Resetting the board.");
	printf("\n");

	udelay(200);

	/* perform reset, only one bit */
	immap->reset.rcr = RCR_SWHR;

#else	/* ! MPC83xx_RESET */

	immap->reset.rmr = RMR_CSRE;    /* Checkstop Reset enable */

	/* Interrupts and MMU off */
	__asm__ __volatile__ ("mfmsr    %0":"=r" (msr):);

	msr &= ~(MSR_ME | MSR_EE | MSR_IR | MSR_DR);
	__asm__ __volatile__ ("mtmsr    %0"::"r" (msr));

	/*
	 * Trying to execute the next instruction at a non-existing address
	 * should cause a machine check, resulting in reset
	 */
	addr = CFG_RESET_ADDRESS;

	printf("resetting the board.");
	printf("\n");
	((void (*)(void)) addr) ();
#endif	/* MPC83xx_RESET */

	return 1;
}


/*
 * Get timebase clock frequency (like cpu_clk in Hz)
 */

unsigned long get_tbclk(void)
{
	ulong tbclk;

	tbclk = (gd->bus_clk + 3L) / 4L;

	return tbclk;
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
	int re_enable = disable_interrupts();

	/* Reset the 83xx watchdog */
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	immr->wdt.swsrr = 0x556c;
	immr->wdt.swsrr = 0xaa39;

	if (re_enable)
		enable_interrupts ();
}
#endif

#if defined(CONFIG_OF_LIBFDT)

/*
 * "Setter" functions used to add/modify FDT entries.
 */
static int fdt_set_eth0(void *blob, int nodeoffset, const char *name, bd_t *bd)
{
	/*
	 * Fix it up if it exists, don't create it if it doesn't exist.
	 */
	if (fdt_get_property(blob, nodeoffset, name, 0)) {
		return fdt_setprop(blob, nodeoffset, name, bd->bi_enetaddr, 6);
	}
	return 0;
}
#ifdef CONFIG_HAS_ETH1
/* second onboard ethernet port */
static int fdt_set_eth1(void *blob, int nodeoffset, const char *name, bd_t *bd)
{
	/*
	 * Fix it up if it exists, don't create it if it doesn't exist.
	 */
	if (fdt_get_property(blob, nodeoffset, name, 0)) {
		return fdt_setprop(blob, nodeoffset, name, bd->bi_enet1addr, 6);
	}
	return 0;
}
#endif
#ifdef CONFIG_HAS_ETH2
/* third onboard ethernet port */
static int fdt_set_eth2(void *blob, int nodeoffset, const char *name, bd_t *bd)
{
	/*
	 * Fix it up if it exists, don't create it if it doesn't exist.
	 */
	if (fdt_get_property(blob, nodeoffset, name, 0)) {
		return fdt_setprop(blob, nodeoffset, name, bd->bi_enet2addr, 6);
	}
	return 0;
}
#endif
#ifdef CONFIG_HAS_ETH3
/* fourth onboard ethernet port */
static int fdt_set_eth3(void *blob, int nodeoffset, const char *name, bd_t *bd)
{
	/*
	 * Fix it up if it exists, don't create it if it doesn't exist.
	 */
	if (fdt_get_property(blob, nodeoffset, name, 0)) {
		return fdt_setprop(blob, nodeoffset, name, bd->bi_enet3addr, 6);
	}
	return 0;
}
#endif

static int fdt_set_busfreq(void *blob, int nodeoffset, const char *name, bd_t *bd)
{
	u32  tmp;
	/*
	 * Create or update the property.
	 */
	tmp = cpu_to_be32(bd->bi_busfreq);
	return fdt_setprop(blob, nodeoffset, name, &tmp, sizeof(tmp));
}

static int fdt_set_tbfreq(void *blob, int nodeoffset, const char *name, bd_t *bd)
{
	u32  tmp;
	/*
	 * Create or update the property.
	 */
	tmp = cpu_to_be32(OF_TBCLK);
	return fdt_setprop(blob, nodeoffset, name, &tmp, sizeof(tmp));
}


/*
 * Fixups to the fdt.
 */
static const struct {
	char *node;
	char *prop;
	int (*set_fn)(void *blob, int nodeoffset, const char *name, bd_t *bd);
} fixup_props[] = {
	{	"/cpus/" OF_CPU,
		"timebase-frequency",
		fdt_set_tbfreq
	},
	{	"/cpus/" OF_CPU,
		"bus-frequency",
		fdt_set_busfreq
	},
	{	"/cpus/" OF_CPU,
		"clock-frequency",
		fdt_set_busfreq
	},
	{	"/" OF_SOC "/serial@4500",
		"clock-frequency",
		fdt_set_busfreq
	},
	{	"/" OF_SOC "/serial@4600",
		"clock-frequency",
		fdt_set_busfreq
	},
#ifdef CONFIG_TSEC1
	{	"/" OF_SOC "/ethernet@24000",
		"mac-address",
		fdt_set_eth0
	},
	{	"/" OF_SOC "/ethernet@24000",
		"local-mac-address",
		fdt_set_eth0
	},
#endif
#ifdef CONFIG_TSEC2
	{	"/" OF_SOC "/ethernet@25000",
		"mac-address",
		fdt_set_eth1
	},
	{	"/" OF_SOC "/ethernet@25000",
		"local-mac-address",
		fdt_set_eth1
	},
#endif
#ifdef CONFIG_UEC_ETH1
#if CFG_UEC1_UCC_NUM == 0  /* UCC1 */
	{	"/" OF_QE "/ucc@2000",
		"mac-address",
		fdt_set_eth0
	},
	{	"/" OF_QE "/ucc@2000",
		"local-mac-address",
		fdt_set_eth0
	},
#elif CFG_UEC1_UCC_NUM == 2  /* UCC3 */
	{	"/" OF_QE "/ucc@2200",
		"mac-address",
		fdt_set_eth0
	},
	{	"/" OF_QE "/ucc@2200",
		"local-mac-address",
		fdt_set_eth0
	},
#endif
#endif /* CONFIG_UEC_ETH1 */
#ifdef CONFIG_UEC_ETH2
#if CFG_UEC2_UCC_NUM == 1  /* UCC2 */
	{	"/" OF_QE "/ucc@3000",
		"mac-address",
		fdt_set_eth1
	},
	{	"/" OF_QE "/ucc@3000",
		"local-mac-address",
		fdt_set_eth1
	},
#elif CFG_UEC1_UCC_NUM == 3  /* UCC4 */
	{	"/" OF_QE "/ucc@3200",
		"mac-address",
		fdt_set_eth1
	},
	{	"/" OF_QE "/ucc@3200",
		"local-mac-address",
		fdt_set_eth1
	},
#endif
#endif /* CONFIG_UEC_ETH2 */
};

void
ft_cpu_setup(void *blob, bd_t *bd)
{
	int  nodeoffset;
	int  err;
	int  j;

	for (j = 0; j < (sizeof(fixup_props) / sizeof(fixup_props[0])); j++) {
		nodeoffset = fdt_find_node_by_path(blob, fixup_props[j].node);
		if (nodeoffset >= 0) {
			err = fixup_props[j].set_fn(blob, nodeoffset,
						    fixup_props[j].prop, bd);
			if (err < 0)
				debug("Problem setting %s = %s: %s\n",
					fixup_props[j].node,
					fixup_props[j].prop,
					fdt_strerror(err));
		} else {
			debug("Couldn't find %s: %s\n",
				fixup_props[j].node,
				fdt_strerror(nodeoffset));
		}
	}
}
#elif defined(CONFIG_OF_FLAT_TREE)
void
ft_cpu_setup(void *blob, bd_t *bd)
{
	u32 *p;
	int len;
	ulong clock;

	clock = bd->bi_busfreq;
	p = ft_get_prop(blob, "/cpus/" OF_CPU "/bus-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

	p = ft_get_prop(blob, "/" OF_SOC "/bus-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

	p = ft_get_prop(blob, "/" OF_SOC "/serial@4500/clock-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

	p = ft_get_prop(blob, "/" OF_SOC "/serial@4600/clock-frequency", &len);
	if (p != NULL)
		*p = cpu_to_be32(clock);

#ifdef CONFIG_TSEC1
	p = ft_get_prop(blob, "/" OF_SOC "/ethernet@24000/mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);

	p = ft_get_prop(blob, "/" OF_SOC "/ethernet@24000/local-mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);
#endif

#ifdef CONFIG_TSEC2
	p = ft_get_prop(blob, "/" OF_SOC "/ethernet@25000/mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enet1addr, 6);

	p = ft_get_prop(blob, "/" OF_SOC "/ethernet@25000/local-mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enet1addr, 6);
#endif

#ifdef CONFIG_UEC_ETH1
#if CFG_UEC1_UCC_NUM == 0  /* UCC1 */
	p = ft_get_prop(blob, "/" OF_QE "/ucc@2000/mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);

	p = ft_get_prop(blob, "/" OF_QE "/ucc@2000/local-mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);
#elif CFG_UEC1_UCC_NUM == 2  /* UCC3 */
	p = ft_get_prop(blob, "/" OF_QE "/ucc@2200/mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);

	p = ft_get_prop(blob, "/" OF_QE "/ucc@2200/local-mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enetaddr, 6);
#endif
#endif

#ifdef CONFIG_UEC_ETH2
#if CFG_UEC2_UCC_NUM == 1  /* UCC2 */
	p = ft_get_prop(blob, "/" OF_QE "/ucc@3000/mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enet1addr, 6);

	p = ft_get_prop(blob, "/" OF_QE "/ucc@3000/local-mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enet1addr, 6);
#elif CFG_UEC2_UCC_NUM == 3  /* UCC4 */
	p = ft_get_prop(blob, "/" OF_QE "/ucc@3200/mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enet1addr, 6);

	p = ft_get_prop(blob, "/" OF_QE "/ucc@3200/local-mac-address", &len);
	if (p != NULL)
		memcpy(p, bd->bi_enet1addr, 6);
#endif
#endif
}
#endif

#if defined(CONFIG_DDR_ECC)
void dma_init(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile dma83xx_t *dma = &immap->dma;
	volatile u32 status = swab32(dma->dmasr0);
	volatile u32 dmamr0 = swab32(dma->dmamr0);

	debug("DMA-init\n");

	/* initialize DMASARn, DMADAR and DMAABCRn */
	dma->dmadar0 = (u32)0;
	dma->dmasar0 = (u32)0;
	dma->dmabcr0 = 0;

	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* clear CS bit */
	dmamr0 &= ~DMA_CHANNEL_START;
	dma->dmamr0 = swab32(dmamr0);
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* while the channel is busy, spin */
	while(status & DMA_CHANNEL_BUSY) {
		status = swab32(dma->dmasr0);
	}

	debug("DMA-init end\n");
}

uint dma_check(void)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile dma83xx_t *dma = &immap->dma;
	volatile u32 status = swab32(dma->dmasr0);
	volatile u32 byte_count = swab32(dma->dmabcr0);

	/* while the channel is busy, spin */
	while (status & DMA_CHANNEL_BUSY) {
		status = swab32(dma->dmasr0);
	}

	if (status & DMA_CHANNEL_TRANSFER_ERROR) {
		printf ("DMA Error: status = %x @ %d\n", status, byte_count);
	}

	return status;
}

int dma_xfer(void *dest, u32 count, void *src)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile dma83xx_t *dma = &immap->dma;
	volatile u32 dmamr0;

	/* initialize DMASARn, DMADAR and DMAABCRn */
	dma->dmadar0 = swab32((u32)dest);
	dma->dmasar0 = swab32((u32)src);
	dma->dmabcr0 = swab32(count);

	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* init direct transfer, clear CS bit */
	dmamr0 = (DMA_CHANNEL_TRANSFER_MODE_DIRECT |
			DMA_CHANNEL_SOURCE_ADDRESS_HOLD_8B |
			DMA_CHANNEL_SOURCE_ADRESSS_HOLD_EN);

	dma->dmamr0 = swab32(dmamr0);

	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	/* set CS to start DMA transfer */
	dmamr0 |= DMA_CHANNEL_START;
	dma->dmamr0 = swab32(dmamr0);
	__asm__ __volatile__ ("sync");
	__asm__ __volatile__ ("isync");

	return ((int)dma_check());
}
#endif /*CONFIG_DDR_ECC*/
