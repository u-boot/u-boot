/*
 * Copyright (c) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ARCH_REPORT_PLATFORM_H
#define __ARCH_REPORT_PLATFORM_H

/**
 * report_platform_info() - Report platform information
 *
 * This reports information about the CPU and chipset.
 *
 * @dev:	Northbridge device
 */
void report_platform_info(struct udevice *dev);

#endif
