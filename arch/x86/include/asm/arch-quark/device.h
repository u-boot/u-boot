/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _QUARK_DEVICE_H_
#define _QUARK_DEVICE_H_

#include <pci.h>

#define QUARK_HOST_BRIDGE	PCI_BDF(0, 0, 0)
#define QUARK_MMC_SDIO		PCI_BDF(0, 20, 0)
#define QUARK_UART0		PCI_BDF(0, 20, 1)
#define QUARK_USB_DEVICE	PCI_BDF(0, 20, 2)
#define QUARK_USB_EHCI		PCI_BDF(0, 20, 3)
#define QUARK_USB_OHCI		PCI_BDF(0, 20, 4)
#define QUARK_UART1		PCI_BDF(0, 20, 5)
#define QUARK_EMAC0		PCI_BDF(0, 20, 6)
#define QUARK_EMAC1		PCI_BDF(0, 20, 7)
#define QUARK_SPI0		PCI_BDF(0, 21, 0)
#define QUARK_SPI1		PCI_BDF(0, 21, 1)
#define QUARK_I2C_GPIO		PCI_BDF(0, 21, 2)
#define QUARK_PCIE0		PCI_BDF(0, 23, 0)
#define QUARK_PCIE1		PCI_BDF(0, 23, 1)
#define QUARK_LEGACY_BRIDGE	PCI_BDF(0, 31, 0)

#endif /* _QUARK_DEVICE_H_ */
