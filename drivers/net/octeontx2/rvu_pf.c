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
#include <asm/io.h>
#include <errno.h>
#include <asm/types.h>
#include <asm/arch/board.h>
#include "cgx.h"
#include "nix.h"

extern struct udevice *rvu_af_dev;

int rvu_pf_init(struct rvu_pf *rvu, int nix_id)
{
	struct nix *nix;
	struct eth_pdata *pdata = dev_get_platdata(rvu->dev);

	debug("%s: Allocating nix%d lf\n", __func__, nix_id);
	nix = nix_lf_alloc(rvu->dev, nix_id);
	if (!nix) {
		printf("%s: Error allocating lf for pf %d\n",
		       __func__, rvu->pfid);
		return -1;
	}
	rvu->nix = nix;

	/* to make post_probe happy */
	if (is_valid_ethaddr(nix->lmac->mac_addr)) {
		memcpy(pdata->enetaddr, nix->lmac->mac_addr, 6);
		eth_env_set_enetaddr_by_index("eth", rvu->dev->seq,
					      pdata->enetaddr);
	}

	return 0;
}

static const struct eth_ops nix_eth_ops = {
	.start			= nix_lf_init,
	.send			= nix_lf_xmit,
	.recv			= nix_lf_recv,
	.free_pkt		= nix_lf_free_pkt,
	.stop			= nix_lf_halt,
	.write_hwaddr		= nix_lf_setup_mac,
};

int rvu_pf_probe(struct udevice *dev)
{
	struct rvu_pf *rvu = dev_get_priv(dev);
	struct lmac *cgx_lmac;
	int err, nix_id;
	char name[16];

	debug("%s: name: %s\n", __func__, dev->name);

	rvu->pf_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_2, PCI_REGION_MEM);
	rvu->pfid = dev->seq + 1; // RVU PF's start from 1;
	rvu->dev = dev;
	if (!rvu_af_dev) {
		printf("%s: Error: Could not find RVU AF device\n",
		       __func__);
		return -1;
	}
	rvu->afdev = rvu_af_dev;

	/* Retrieve the NIX ID from the LMAC pointer. */
	cgx_lmac = nix_get_cgx_lmac(rvu->pfid);
	if (!cgx_lmac) {
		printf("RVU PF%d: cannot locate LMAC, unknown NIX ID\n",
		       rvu->pfid);
		return -1;
	}

	switch (cgx_lmac->p2x_sel) {
	case P2X1_NIX0:
	case P2X2_NIX1:
		nix_id = cgx_lmac->p2x_sel - P2X1_NIX0;
		break;
	default:
		printf("RVU PF%d: invalid LMAC P2X_SEL %d, unknown NIX ID\n",
		       rvu->pfid, (int)cgx_lmac->p2x_sel);
		return -1;
	}

	debug("RVU PF%d: using NIX%d\n", rvu->pfid, nix_id);

	rvu_get_lfid_for_pf(rvu->pfid, nix_id, &rvu->nix_lfid, &rvu->npa_lfid);

	err = rvu_pf_init(rvu, nix_id);
	if (err)
		printf("%s: Error %d adding nix\n", __func__, err);

	/*
	 * modify device name to include index/sequence number,
	 * for better readability, this is 1:1 mapping with eth0/1/2.. names.
	 */
	sprintf(name, "rvu_pf#%d", dev->seq);
	device_set_name(dev, name);
	debug("%s: name: %s\n", __func__, dev->name);
	return err;
}

int rvu_pf_remove(struct udevice *dev)
{
	struct rvu_pf *rvu = dev_get_priv(dev);

	nix_lf_shutdown(rvu->nix);
	npa_lf_shutdown(rvu->nix);

	debug("%s: rvu pf%d down --\n", __func__,  rvu->pfid);

	return 0;
}

U_BOOT_DRIVER(rvu_pf) = {
	.name   = "rvu_pf",
	.id     = UCLASS_ETH,
	.probe	= rvu_pf_probe,
	.remove = rvu_pf_remove,
	.ops    = &nix_eth_ops,
	.priv_auto_alloc_size = sizeof(struct rvu_pf),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static struct pci_device_id rvu_pf_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVID_OCTEONTX2_RVU_PF) },
	{}
};

U_BOOT_PCI_DEVICE(rvu_pf, rvu_pf_supported);
