// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 9elements GmbH
 */
#include <dm/acpi.h>
#include <dm/device.h>

#ifndef _ARMV8_CPU_H_
#define _ARMV8_CPU_H_

/**
 * armv8_cpu_fill_ssdt() - Fill the SSDT
 * Parses the FDT and writes the SSDT nodes.
 *
 * @dev: cpu device to generate ACPI tables for
 * @ctx: ACPI context pointer
 * @return:	0 if OK, or a negative error code.
 */
int armv8_cpu_fill_ssdt(const struct udevice *dev, struct acpi_ctx *ctx);

/**
 * armv8_cpu_fill_madt() - Fill the MADT
 * Parses the FDT and writes the MADT subtables.
 *
 * @dev: cpu device to generate ACPI tables for
 * @ctx: ACPI context pointer
 * @return:	0 if OK, or a negative error code.
 */
int armv8_cpu_fill_madt(const struct udevice *dev, struct acpi_ctx *ctx);

#endif