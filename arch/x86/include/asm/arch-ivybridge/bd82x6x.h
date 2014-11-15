/*
 * Copyright (C) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_BD82X6X_H
#define _ASM_ARCH_BD82X6X_H

void bd82x6x_sata_init(pci_dev_t dev, const void *blob, int node);
void bd82x6x_sata_enable(pci_dev_t dev, const void *blob, int node);
void bd82x6x_pci_init(pci_dev_t dev);
int bd82x6x_init_pci_devices(void);
int bd82x6x_init(void);

#endif
