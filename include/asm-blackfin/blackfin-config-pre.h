/*
 * blackfin-config-pre.h - common defines for Blackfin boards in config.h
 *
 * Copyright (c) 2007 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ASM_BLACKFIN_CONFIG_PRE_H__
#define __ASM_BLACKFIN_CONFIG_PRE_H__

/* Misc helper functions */
#define XMK_STR(x) #x
#define MK_STR(x) XMK_STR(x)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* Configurable Blackfin-specific monitor commands */
#define CFG_BFIN_CMD_BOOTLDR     0x01
#define CFG_BFIN_CMD_CPLBINFO    0x02
#define CFG_BFIN_CMD_OTP         0x04
#define CFG_BFIN_CMD_CACHE_DUMP  0x08

/* Bootmode defines -- your config needs to select this via BFIN_BOOT_MODE.
 * Depending on your cpu, some of these may not be valid, check your HRM.
 * The actual values here are meaningless as long as they're unique.
 */
#define BFIN_BOOT_BYPASS      1       /* bypass bootrom */
#define BFIN_BOOT_PARA        2       /* boot ldr out of parallel flash */
#define BFIN_BOOT_SPI_MASTER  3       /* boot ldr out of serial flash */
#define BFIN_BOOT_SPI_SLAVE   4       /* boot ldr as spi slave */
#define BFIN_BOOT_TWI_MASTER  5       /* boot ldr over twi device */
#define BFIN_BOOT_TWI_SLAVE   6       /* boot ldr over twi slave */
#define BFIN_BOOT_UART        7       /* boot ldr over uart */
#define BFIN_BOOT_IDLE        8       /* do nothing, just idle */
#define BFIN_BOOT_FIFO        9       /* boot ldr out of FIFO */
#define BFIN_BOOT_MEM         10      /* boot ldr out of memory (warmboot) */
#define BFIN_BOOT_16HOST_DMA  11      /* boot ldr from 16-bit host dma */
#define BFIN_BOOT_8HOST_DMA   12      /* boot ldr from 8-bit host dma */

#endif
