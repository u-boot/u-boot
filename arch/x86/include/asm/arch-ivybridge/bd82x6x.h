/*
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_BD82X6X_H
#define _ASM_ARCH_BD82X6X_H

void bd82x6x_sata_init(pci_dev_t dev, const void *blob, int node);
void bd82x6x_usb_ehci_init(pci_dev_t dev);
void bd82x6x_usb_xhci_init(pci_dev_t dev);
int gma_func0_init(struct udevice *dev, const void *blob, int node);
int bd82x6x_init_extra(void);

#endif
