/* SPDX-License-Identifier: BSD-2-Clause-Patent */
/**
 *
 *  Copyright (c) 2019, ARM Limited. All rights reserved.
 *  Copyright (c) 2017, Andrei Warkentin <andrey.warkentin@gmail.com>
 *  Copyright (c) 2016, Linaro Limited. All rights reserved.
 *
 **/

#ifndef __BCM2836_H__
#define __BCM2836_H__

/*
 * Both "core" and SoC perpherals (1M each).
 */
#define BCM2836_SOC_REGISTERS                 0xfe000000
#define BCM2836_SOC_REGISTER_LENGTH           0x02000000

/*
 * Offset between the CPU's view and the VC's view of system memory.
 */
#define BCM2836_DMA_DEVICE_OFFSET             0xc0000000

/* watchdog constants */
#define BCM2836_WDOG_OFFSET                   0x00100000
#define BCM2836_WDOG_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_WDOG_OFFSET)
#define BCM2836_WDOG_PASSWORD                 0x5a000000
#define BCM2836_WDOG_RSTC_OFFSET              0x0000001c
#define BCM2836_WDOG_WDOG_OFFSET              0x00000024
#define BCM2836_WDOG_RSTC_WRCFG_MASK          0x00000030
#define BCM2836_WDOG_RSTC_WRCFG_FULL_RESET    0x00000020

/* clock manager constants */
#define BCM2836_CM_OFFSET                     0x00101000
#define BCM2836_CM_BASE                       (BCM2836_SOC_REGISTERS + BCM2836_CM_OFFSET)
#define BCM2836_CM_GEN_CLOCK_CONTROL          0x0000
#define BCM2836_CM_GEN_CLOCK_DIVISOR          0x0004
#define BCM2836_CM_VPU_CLOCK_CONTROL          0x0008
#define BCM2836_CM_VPU_CLOCK_DIVISOR          0x000c
#define BCM2836_CM_SYSTEM_CLOCK_CONTROL       0x0010
#define BCM2836_CM_SYSTEM_CLOCK_DIVISOR       0x0014
#define BCM2836_CM_H264_CLOCK_CONTROL         0x0028
#define BCM2836_CM_H264_CLOCK_DIVISOR         0x002c
#define BCM2836_CM_PWM_CLOCK_CONTROL          0x00a0
#define BCM2836_CM_PWM_CLOCK_DIVISOR          0x00a4
#define BCM2836_CM_UART_CLOCK_CONTROL         0x00f0
#define BCM2836_CM_UART_CLOCK_DIVISOR         0x00f4
#define BCM2836_CM_SDC_CLOCK_CONTROL          0x01a8
#define BCM2836_CM_SDC_CLOCK_DIVISOR          0x01ac
#define BCM2836_CM_ARM_CLOCK_CONTROL          0x01b0
#define BCM2836_CM_ARM_CLOCK_DIVISOR          0x01b4
#define BCM2836_CM_EMMC_CLOCK_CONTROL         0x01c0
#define BCM2836_CM_EMMC_CLOCK_DIVISOR         0x01c4

/* mailbox interface constants */
#define BCM2836_MBOX_OFFSET                   0x0000b880
#define BCM2836_MBOX_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_MBOX_OFFSET)
#define BCM2836_MBOX_LENGTH                   0x00000024
#define BCM2836_MBOX_READ_OFFSET              0x00000000
#define BCM2836_MBOX_STATUS_OFFSET            0x00000018
#define BCM2836_MBOX_CONFIG_OFFSET            0x0000001c
#define BCM2836_MBOX_WRITE_OFFSET             0x00000020

#define BCM2836_MBOX_STATUS_FULL              0x1f
#define BCM2836_MBOX_STATUS_EMPTY             0x1e

#define BCM2836_MBOX_NUM_CHANNELS             16

/* interrupt controller constants */
#define BCM2836_INTC_TIMER_CONTROL_OFFSET     0x00000040
#define BCM2836_INTC_TIMER_PENDING_OFFSET     0x00000060

/* usb constants */
#define BCM2836_USB_OFFSET                    0x00980000
#define BCM2836_USB_BASE_ADDRESS              (BCM2836_SOC_REGISTERS + BCM2836_USB_OFFSET)
#define BCM2836_USB_LENGTH                    0x00010000

/* serial based protocol constants */
#define BCM2836_PL011_UART_OFFSET             0x00201000
#define BCM2836_PL011_UART_BASE_ADDRESS       (BCM2836_SOC_REGISTERS + BCM2836_PL011_UART_OFFSET)
#define BCM2836_PL011_UART_LENGTH             0x00001000

#define BCM2836_MINI_UART_OFFSET              0x00215000
#define BCM2836_MINI_UART_BASE_ADDRESS        (BCM2836_SOC_REGISTERS + BCM2836_MINI_UART_OFFSET)
#define BCM2836_MINI_UART_LENGTH              0x00000070

#define BCM2836_I2C0_OFFSET                   0x00205000
#define BCM2836_I2C0_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_I2C0_OFFSET)
#define BCM2836_I2C0_LENGTH                   0x00000020

#define BCM2836_I2C1_OFFSET                   0x00804000
#define BCM2836_I2C1_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_I2C1_OFFSET)
#define BCM2836_I2C1_LENGTH                   0x00000020

#define BCM2836_I2C2_OFFSET                   0x00805000
#define BCM2836_I2C2_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_I2C2_OFFSET)
#define BCM2836_I2C2_LENGTH                   0x00000020

#define BCM2836_SPI0_OFFSET                   0x00204000
#define BCM2836_SPI0_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_SPI0_OFFSET)
#define BCM2836_SPI0_LENGTH                   0x00000020

#define BCM2836_SPI1_OFFSET                   0x00215080
#define BCM2836_SPI1_LENGTH                   0x00000040
#define BCM2836_SPI1_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_SPI1_OFFSET)

#define BCM2836_SPI2_OFFSET                   0x002150C0
#define BCM2836_SPI2_LENGTH                   0x00000040
#define BCM2836_SPI2_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_SPI2_OFFSET)

#define BCM2836_SYSTEM_TIMER_OFFSET           0x00003000
#define BCM2836_SYSTEM_TIMER_LENGTH           0x00000020
#define BCM2836_SYSTEM_TIMER_ADDRESS          (BCM2836_SOC_REGISTERS + BCM2836_SYSTEM_TIMER_OFFSET)

/* dma constants */
#define BCM2836_DMA0_OFFSET                   0x00007000
#define BCM2836_DMA0_BASE_ADDRESS             (BCM2836_SOC_REGISTERS + BCM2836_DMA0_OFFSET)

#define BCM2836_DMA15_OFFSET                  0x00E05000
#define BCM2836_DMA15_BASE_ADDRESS            (BCM2836_SOC_REGISTERS + BCM2836_DMA15_OFFSET)

#define BCM2836_DMA_CTRL_OFFSET               0x00007FE0
#define BCM2836_DMA_CTRL_BASE_ADDRESS         (BCM2836_SOC_REGISTERS + BCM2836_DMA_CTRL_OFFSET)

#define BCM2836_DMA_CHANNEL_LENGTH            0x00000100

#endif /*__BCM2836_H__ */
