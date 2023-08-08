// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#include <common.h>
#include <net.h>
#include <malloc.h>
#include <dm.h>
#include <misc.h>
#include <errno.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/arch/board.h>
#include <asm/arch/csrs/csrs-npa.h>
#include "nix.h"

struct udevice *rvu_af_dev;

inline struct rvu_af *get_af(void)
{
	return rvu_af_dev ? dev_get_priv(rvu_af_dev) : NULL;
}

void rvu_get_lfid_for_pf(int pf, int nix_id, int *nix_lfid, int *npa_lfid)
{
	union nixx_af_rvu_lf_cfg_debug nix_lf_dbg;
	union npa_af_rvu_lf_cfg_debug npa_lf_dbg;
	union rvu_pf_func_s pf_func;
	struct rvu_af *af = dev_get_priv(rvu_af_dev);
	struct nix_af *nix_af = af->nix_af[nix_id];

	pf_func.u = 0;
	pf_func.s.pf = pf;

	nix_lf_dbg.u = 0;
	nix_lf_dbg.s.pf_func = pf_func.u & 0xFFFF;
	nix_lf_dbg.s.exec = 1;
	nix_af_reg_write(nix_af, NIXX_AF_RVU_LF_CFG_DEBUG(),
			 nix_lf_dbg.u);
	do {
		nix_lf_dbg.u = nix_af_reg_read(nix_af,
					       NIXX_AF_RVU_LF_CFG_DEBUG());
	} while (nix_lf_dbg.s.exec);

	if (nix_lf_dbg.s.lf_valid)
		*nix_lfid = nix_lf_dbg.s.lf;

	debug("%s: NIX%d lf_valid %d lf %d\n", __func__, nix_id,
	      nix_lf_dbg.s.lf_valid, nix_lf_dbg.s.lf);

	npa_lf_dbg.u = 0;
	npa_lf_dbg.s.pf_func = pf_func.u & 0xFFFF;
	npa_lf_dbg.s.exec = 1;
	npa_af_reg_write(nix_af->npa_af, NPA_AF_RVU_LF_CFG_DEBUG(),
			 npa_lf_dbg.u);
	do {
		npa_lf_dbg.u = npa_af_reg_read(nix_af->npa_af,
					       NPA_AF_RVU_LF_CFG_DEBUG());
	} while (npa_lf_dbg.s.exec);

	if (npa_lf_dbg.s.lf_valid)
		*npa_lfid = npa_lf_dbg.s.lf;
	debug("%s: npa lf_valid %d lf %d\n", __func__,
	      npa_lf_dbg.s.lf_valid, npa_lf_dbg.s.lf);
}

struct nix_af *rvu_af_init(struct rvu_af *rvu_af, int nix_id)
{
	struct nix_af *nix_af;
	union rvu_af_addr_s block_addr;
	int err;

	nix_af = (struct nix_af *)calloc(1, sizeof(struct nix_af));
	if (!nix_af) {
		printf("%s: out of memory\n", __func__);
		goto error;
	}

	nix_af->dev = rvu_af->dev;

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NIXX(nix_id);
	nix_af->nix_af_base = rvu_af->af_base + block_addr.u;

	/* Allocate an NPA_AF structure for the first NIX_AF.
	 * Share this NPA_AF pointer amongst all subsequent NIX_AFs
	 * as only a single NPA AF exists in hardware.
	 */
	if (nix_id == 0) {
		nix_af->npa_af = (struct npa_af *)
				calloc(1, sizeof(struct npa_af));
	} else {
		nix_af->npa_af = rvu_af->nix_af[0]->npa_af;
	}
	if (!nix_af->npa_af) {
		printf("%s: out of memory\n", __func__);
		goto error;
	}

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NPA;
	nix_af->npa_af->npa_af_base = rvu_af->af_base + block_addr.u;

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NPC;
	nix_af->npc_af_base = rvu_af->af_base + block_addr.u;

	/* Only initialize NPA_AF for first NIX_AF instance.
	 * It is shared amongst all NIX_AF instances (see allocation above).
	 */
	if (nix_id == 0) {
		debug("%s: Setting up npa admin\n", __func__);
		err = npa_af_setup(nix_af->npa_af);
		if (err) {
			printf("%s: Error %d setting up NPA admin\n",
			       __func__, err);
			goto error;
		}
	}

	debug("%s: Setting up nix af\n", __func__);
	err = nix_af_setup(nix_af);
	if (err) {
		printf("%s: Error %d setting up NIX%d admin\n",
		       __func__, err, nix_id);
		goto error;
	}
	debug("%s: NIX%d nix_af: %p\n", __func__, nix_id, nix_af);
	return nix_af;

error:
	if (nix_af->npa_af) {
		/* only free the actual allocation (i.e. first one) */
		if (nix_id == 0) {
			free(nix_af->npa_af);
			memset(nix_af, 0, sizeof(*nix_af));
		}
	}
	if (nix_af)
		free(nix_af);
	return NULL;
}

int rvu_af_probe(struct udevice *dev)
{
	struct rvu_af *af_ptr = dev_get_priv(dev);
	union rvu_pf_block_addrx_disc addr_disc;
	void __iomem *bar2_base;
	int nix_id, blk;

	af_ptr->af_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0,
					 PCI_REGION_MEM);
	bar2_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_2,
				   PCI_REGION_MEM);
	debug("%s RVU AF BAR0 %p, BAR2 %p\n", __func__, af_ptr->af_base,
	      bar2_base);
	af_ptr->dev = dev;
	rvu_af_dev = dev;

	for (nix_id = 0; nix_id < MAX_RVU_NIX; nix_id++) {
		/* Verify that this NIX block is implemented in h/w. */
		blk = RVU_BLOCK_ADDR_E_NIXX(nix_id);
		addr_disc.u = rvu_bar2_reg_read(bar2_base,
						RVU_PF_BLOCK_ADDRX_DISC(blk));
		if (!addr_disc.s.imp)
			break;

		af_ptr->nix_af[nix_id] = rvu_af_init(af_ptr, nix_id);
		if (!af_ptr->nix_af) {
			printf("%s: Error: could not initialize NIX%d AF\n",
			       __func__, nix_id);
			return -1;
		}
		debug("%s: Initialized NIX%d\n", __func__, nix_id);
	}

	return 0;
}

int rvu_af_remove(struct udevice *dev)
{
	struct rvu_af *rvu_af = dev_get_priv(dev);
	struct nix_af *nix_af;
	int i;

	for (i = MAX_RVU_NIX - 1; (int)i >= 0; i--) {
		nix_af = rvu_af->nix_af[i];
		if (!nix_af)
			continue;
		nix_af_shutdown(nix_af);

		/* Only a single instance of NPA & NPC exists in h/w. */
		if (i != 0)
			continue;
		npa_af_shutdown(nix_af->npa_af);
		npc_af_shutdown(nix_af);
	}

	debug("%s: rvu af down --\n", __func__);
	return 0;
}

U_BOOT_DRIVER(rvu_af) = {
	.name   = "rvu_af",
	.id     = UCLASS_MISC,
	.probe  = rvu_af_probe,
	.remove = rvu_af_remove,
	.priv_auto_alloc_size = sizeof(struct rvu_af),
};

static struct pci_device_id rvu_af_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVID_OCTEONTX2_RVU_AF) },
	{}
};

U_BOOT_PCI_DEVICE(rvu_af, rvu_af_supported);
