/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/**
 *
 *  Copyright (c) 2020, Pete Batard <pete@akeo.ie>
 *
 **/

#include <asm/arch/acpi/bcm2836.h>

#ifndef __BCM2836_GPU_H__
#define __BCM2836_GPU_H__

/* VideoCore constants */

#define BCM2836_VCHIQ_OFFSET                  0x0000B840
#define BCM2836_VCHIQ_BASE_ADDRESS            (BCM2836_SOC_REGISTERS + BCM2836_VCHIQ_OFFSET)
#define BCM2836_VCHIQ_LENGTH                  0x00000010

#define BCM2836_V3D_BUS_OFFSET                0x00C00000
#define BCM2836_V3D_BUS_BASE_ADDRESS          (BCM2836_SOC_REGISTERS + BCM2836_V3D_BUS_OFFSET)
#define BCM2836_V3D_BUS_LENGTH                0x00001000

#define BCM2836_HVS_OFFSET                    0x00400000
#define BCM2836_HVS_BASE_ADDRESS              (BCM2836_SOC_REGISTERS + BCM2836_HVS_OFFSET)
#define BCM2836_HVS_LENGTH                    0x00006000

#define BCM2836_PV0_OFFSET                    0x00206000
#define BCM2836_PV0_BASE_ADDRESS              (BCM2836_SOC_REGISTERS + BCM2836_PV0_OFFSET)
#define BCM2836_PV0_LENGTH                    0x00000100

#define BCM2836_PV1_OFFSET                    0x00207000
#define BCM2836_PV1_BASE_ADDRESS              (BCM2836_SOC_REGISTERS + BCM2836_PV1_OFFSET)
#define BCM2836_PV1_LENGTH                    0x00000100

#define BCM2836_PV2_OFFSET                    0x00807000
#define BCM2836_PV2_BASE_ADDRESS              (BCM2836_SOC_REGISTERS + BCM2836_PV2_OFFSET)
#define BCM2836_PV2_LENGTH                    0x00000100

#define BCM2836_HDMI0_OFFSET                  0x00902000
#define BCM2836_HDMI0_BASE_ADDRESS            (BCM2836_SOC_REGISTERS + BCM2836_HDMI0_OFFSET)
#define BCM2836_HDMI0_LENGTH                  0x00000600

#define BCM2836_HDMI1_OFFSET                  0x00808000
#define BCM2836_HDMI1_BASE_ADDRESS            (BCM2836_SOC_REGISTERS + BCM2836_HDMI1_OFFSET)
#define BCM2836_HDMI1_LENGTH                  0x00000100

#endif /* __BCM2836_MISC_H__ */
