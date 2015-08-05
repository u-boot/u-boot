/*
 * From Coreboot
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2012 The Chromium OS Authors.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <asm/arch/pch.h>

static int pch_revision_id = -1;
static int pch_type = -1;

int pch_silicon_revision(void)
{
	pci_dev_t dev;

	dev = PCH_LPC_DEV;

	if (pch_revision_id < 0)
		pch_revision_id = x86_pci_read_config8(dev, PCI_REVISION_ID);
	return pch_revision_id;
}

int pch_silicon_type(void)
{
	pci_dev_t dev;

	dev = PCH_LPC_DEV;

	if (pch_type < 0)
		pch_type = x86_pci_read_config8(dev, PCI_DEVICE_ID + 1);
	return pch_type;
}

int pch_silicon_supported(int type, int rev)
{
	int cur_type = pch_silicon_type();
	int cur_rev = pch_silicon_revision();

	switch (type) {
	case PCH_TYPE_CPT:
		/* CougarPoint minimum revision */
		if (cur_type == PCH_TYPE_CPT && cur_rev >= rev)
			return 1;
		/* PantherPoint any revision */
		if (cur_type == PCH_TYPE_PPT)
			return 1;
		break;

	case PCH_TYPE_PPT:
		/* PantherPoint minimum revision */
		if (cur_type == PCH_TYPE_PPT && cur_rev >= rev)
			return 1;
		break;
	}

	return 0;
}

#define IOBP_RETRY 1000
static inline int iobp_poll(void)
{
	unsigned try = IOBP_RETRY;
	u32 data;

	while (try--) {
		data = readl(RCB_REG(IOBPS));
		if ((data & 1) == 0)
			return 1;
		udelay(10);
	}

	printf("IOBP timeout\n");
	return 0;
}

void pch_iobp_update(u32 address, u32 andvalue, u32 orvalue)
{
	u32 data;

	/* Set the address */
	writel(address, RCB_REG(IOBPIRI));

	/* READ OPCODE */
	if (pch_silicon_supported(PCH_TYPE_CPT, PCH_STEP_B0))
		writel(IOBPS_RW_BX, RCB_REG(IOBPS));
	else
		writel(IOBPS_READ_AX, RCB_REG(IOBPS));
	if (!iobp_poll())
		return;

	/* Read IOBP data */
	data = readl(RCB_REG(IOBPD));
	if (!iobp_poll())
		return;

	/* Check for successful transaction */
	if ((readl(RCB_REG(IOBPS)) & 0x6) != 0) {
		printf("IOBP read 0x%08x failed\n", address);
		return;
	}

	/* Update the data */
	data &= andvalue;
	data |= orvalue;

	/* WRITE OPCODE */
	if (pch_silicon_supported(PCH_TYPE_CPT, PCH_STEP_B0))
		writel(IOBPS_RW_BX, RCB_REG(IOBPS));
	else
		writel(IOBPS_WRITE_AX, RCB_REG(IOBPS));
	if (!iobp_poll())
		return;

	/* Write IOBP data */
	writel(data, RCB_REG(IOBPD));
	if (!iobp_poll())
		return;
}
