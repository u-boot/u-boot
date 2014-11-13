/*
 * From Coreboot src/southbridge/intel/bd82x6x/early_me.c
 *
 * Copyright (C) 2011 The Chromium OS Authors. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <errno.h>
#include <asm/pci.h>
#include <asm/processor.h>
#include <asm/arch/me.h>
#include <asm/arch/pch.h>
#include <asm/io.h>

static const char *const me_ack_values[] = {
	[ME_HFS_ACK_NO_DID]	= "No DID Ack received",
	[ME_HFS_ACK_RESET]	= "Non-power cycle reset",
	[ME_HFS_ACK_PWR_CYCLE]	= "Power cycle reset",
	[ME_HFS_ACK_S3]		= "Go to S3",
	[ME_HFS_ACK_S4]		= "Go to S4",
	[ME_HFS_ACK_S5]		= "Go to S5",
	[ME_HFS_ACK_GBL_RESET]	= "Global Reset",
	[ME_HFS_ACK_CONTINUE]	= "Continue to boot"
};

static inline void pci_read_dword_ptr(void *ptr, int offset)
{
	u32 dword;

	dword = pci_read_config32(PCH_ME_DEV, offset);
	memcpy(ptr, &dword, sizeof(dword));
}

static inline void pci_write_dword_ptr(void *ptr, int offset)
{
	u32 dword = 0;
	memcpy(&dword, ptr, sizeof(dword));
	pci_write_config32(PCH_ME_DEV, offset, dword);
}

void intel_early_me_status(void)
{
	struct me_hfs hfs;
	struct me_gmes gmes;

	pci_read_dword_ptr(&hfs, PCI_ME_HFS);
	pci_read_dword_ptr(&gmes, PCI_ME_GMES);

	intel_me_status(&hfs, &gmes);
}

int intel_early_me_init(void)
{
	int count;
	struct me_uma uma;
	struct me_hfs hfs;

	debug("Intel ME early init\n");

	/* Wait for ME UMA SIZE VALID bit to be set */
	for (count = ME_RETRY; count > 0; --count) {
		pci_read_dword_ptr(&uma, PCI_ME_UMA);
		if (uma.valid)
			break;
		udelay(ME_DELAY);
	}
	if (!count) {
		printf("ERROR: ME is not ready!\n");
		return -EBUSY;
	}

	/* Check for valid firmware */
	pci_read_dword_ptr(&hfs, PCI_ME_HFS);
	if (hfs.fpt_bad) {
		printf("WARNING: ME has bad firmware\n");
		return -EBADF;
	}

	debug("Intel ME firmware is ready\n");

	return 0;
}

int intel_early_me_uma_size(void)
{
	struct me_uma uma;

	pci_read_dword_ptr(&uma, PCI_ME_UMA);
	if (uma.valid) {
		debug("ME: Requested %uMB UMA\n", uma.size);
		return uma.size;
	}

	debug("ME: Invalid UMA size\n");
	return -EINVAL;
}

static inline void set_global_reset(int enable)
{
	u32 etr3;

	etr3 = pci_read_config32(PCH_LPC_DEV, ETR3);

	/* Clear CF9 Without Resume Well Reset Enable */
	etr3 &= ~ETR3_CWORWRE;

	/* CF9GR indicates a Global Reset */
	if (enable)
		etr3 |= ETR3_CF9GR;
	else
		etr3 &= ~ETR3_CF9GR;

	pci_write_config32(PCH_LPC_DEV, ETR3, etr3);
}

int intel_early_me_init_done(u8 status)
{
	u8 reset;
	int count;
	u32 mebase_l, mebase_h;
	struct me_hfs hfs;
	struct me_did did = {
		.init_done = ME_INIT_DONE,
		.status = status
	};

	/* MEBASE from MESEG_BASE[35:20] */
	mebase_l = pci_read_config32(PCH_DEV, PCI_CPU_MEBASE_L);
	mebase_h = pci_read_config32(PCH_DEV, PCI_CPU_MEBASE_H);
	mebase_h &= 0xf;
	did.uma_base = (mebase_l >> 20) | (mebase_h << 12);

	/* Send message to ME */
	debug("ME: Sending Init Done with status: %d, UMA base: 0x%04x\n",
	      status, did.uma_base);

	pci_write_dword_ptr(&did, PCI_ME_H_GS);

	/* Must wait for ME acknowledgement */
	for (count = ME_RETRY; count > 0; --count) {
		pci_read_dword_ptr(&hfs, PCI_ME_HFS);
		if (hfs.bios_msg_ack)
			break;
		udelay(ME_DELAY);
	}
	if (!count) {
		printf("ERROR: ME failed to respond\n");
		return -1;
	}

	/* Return the requested BIOS action */
	debug("ME: Requested BIOS Action: %s\n", me_ack_values[hfs.ack_data]);

	/* Check status after acknowledgement */
	intel_early_me_status();

	reset = 0;
	switch (hfs.ack_data) {
	case ME_HFS_ACK_CONTINUE:
		/* Continue to boot */
		return 0;
	case ME_HFS_ACK_RESET:
		/* Non-power cycle reset */
		set_global_reset(0);
		reset = 0x06;
		break;
	case ME_HFS_ACK_PWR_CYCLE:
		/* Power cycle reset */
		set_global_reset(0);
		reset = 0x0e;
		break;
	case ME_HFS_ACK_GBL_RESET:
		/* Global reset */
		set_global_reset(1);
		reset = 0x0e;
		break;
	case ME_HFS_ACK_S3:
	case ME_HFS_ACK_S4:
	case ME_HFS_ACK_S5:
		break;
	}

	/* Perform the requested reset */
	if (reset) {
		outb(reset, 0xcf9);
		cpu_hlt();
	}
	return -1;
}
