/** @file
 *
 *  RPi defines for constructing ACPI tables
 *
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *  Copyright (c) 2019, ARM Ltd. All rights reserved.
 *  Copyright (c) 2018, Andrei Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) Microsoft Corporation. All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-2-Clause-Patent
 *
 **/

#ifndef __RPI_ACPITABLES_H__
#define __RPI_ACPITABLES_H__

#include <acpi/acpi_table.h>

// The ASL compiler can't perform arithmetic on MEMORY32FIXED ()
// parameters so you can't pass a constant like BASE + OFFSET.
// We therefore define a macro that can perform arithmetic base
// address update with an offset.
#define MEMORY32SETBASE(BufName, MemName, VarName, Offset)       \
    CreateDwordField (^BufName, ^MemName._BAS, VarName)          \
    Add (BCM2836_SOC_REGISTERS, Offset, VarName)

//------------------------------------------------------------------------
// Interrupts. These are specific to each platform
//------------------------------------------------------------------------
#if defined(CONFIG_TARGET_RPI_3)
#define BCM2836_V3D_BUS_INTERRUPT               0x2A
#define BCM2836_DMA_INTERRUPT                   0x3B
#define BCM2836_SPI1_INTERRUPT                  0x3D
#define BCM2836_SPI2_INTERRUPT                  0x3D
#define BCM2836_HVS_INTERRUPT                   0x41
#define BCM2836_HDMI0_INTERRUPT                 0x48
#define BCM2836_HDMI1_INTERRUPT                 0x49
#define BCM2836_PV2_INTERRUPT                   0x4A
#define BCM2836_PV0_INTERRUPT                   0x4D
#define BCM2836_PV1_INTERRUPT                   0x4E
#define BCM2836_MBOX_INTERRUPT                  0x61
#define BCM2836_VCHIQ_INTERRUPT                 0x62
#define BCM2386_GPIO_INTERRUPT0                 0x51
#define BCM2386_GPIO_INTERRUPT1                 0x52
#define BCM2386_GPIO_INTERRUPT2                 0x53
#define BCM2386_GPIO_INTERRUPT3                 0x54
#define BCM2836_I2C1_INTERRUPT                  0x55
#define BCM2836_I2C2_INTERRUPT                  0x55
#define BCM2836_SPI0_INTERRUPT                  0x56
#define BCM2836_USB_INTERRUPT                   0x29
#define BCM2836_SDHOST_INTERRUPT                0x58
#define BCM2836_MMCHS1_INTERRUPT                0x5E
#define BCM2836_MINI_UART_INTERRUPT             0x3D
#define BCM2836_PL011_UART_INTERRUPT            0x59
#elif defined(CONFIG_TARGET_RPI_4)
#define BCM2836_V3D_BUS_INTERRUPT               0x2A
#define BCM2836_DMA_INTERRUPT                   0x3B
#define BCM2836_SPI1_INTERRUPT                  0x7D
#define BCM2836_SPI2_INTERRUPT                  0x7D
#define BCM2836_HVS_INTERRUPT                   0x41
#define BCM2836_HDMI0_INTERRUPT                 0x48
#define BCM2836_HDMI1_INTERRUPT                 0x49
#define BCM2836_PV2_INTERRUPT                   0x4A
#define BCM2836_PV0_INTERRUPT                   0x4D
#define BCM2836_PV1_INTERRUPT                   0x4E
#define BCM2836_MBOX_INTERRUPT                  0x41
#define BCM2836_VCHIQ_INTERRUPT                 0x42
#define BCM2386_GPIO_INTERRUPT0                 0x91
#define BCM2386_GPIO_INTERRUPT1                 0x92
#define BCM2386_GPIO_INTERRUPT2                 0x93
#define BCM2386_GPIO_INTERRUPT3                 0x94
#define BCM2836_I2C1_INTERRUPT                  0x95
#define BCM2836_I2C2_INTERRUPT                  0x95
#define BCM2836_SPI0_INTERRUPT                  0x96
#define BCM2836_USB_INTERRUPT                   0x69
#define BCM2836_SDHOST_INTERRUPT                0x98
#define BCM2836_MMCHS1_INTERRUPT                0x9E
#define BCM2836_MINI_UART_INTERRUPT             0x7D
#define BCM2836_PL011_UART_INTERRUPT            0x99
#define GENET_INTERRUPT0                        0xBD
#define GENET_INTERRUPT1                        0xBE
#define GENET_BASE_ADDRESS                      0xFD580000
#define GENET_LENGTH                            0x10000
#define THERM_SENSOR_BASE_ADDRESS               0xFD5d2200
#define THERM_SENSOR_LENGTH                     0x8
#else
#error "Unsupported rpi module for ACPI tables"
#endif

#endif // __ACPITABLES_H__
