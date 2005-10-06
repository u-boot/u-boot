/*
 * (C) Copyright 2004
 * Texas Instruments, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _OMAP2420_SYS_H_
#define _OMAP2420_SYS_H_

#include <asm/arch/sizes.h>

/*
 * 2420 specific Section
 */

/* L3 Firewall */
#define A_REQINFOPERM0        0x68005048
#define A_READPERM0           0x68005050
#define A_WRITEPERM0          0x68005058
/* #define GP_DEVICE	(BIT8|BIT9)  FIXME -- commented out to make compile -- FIXME */

/* L3 Firewall */
#define A_REQINFOPERM0        0x68005048
#define A_READPERM0           0x68005050
#define A_WRITEPERM0          0x68005058

/* CONTROL */
#define OMAP2420_CTRL_BASE    (0x48000000)
#define CONTROL_STATUS        (OMAP2420_CTRL_BASE + 0x2F8)

/* device type */
#define TST_DEVICE	0x0
#define EMU_DEVICE	0x1
#define HS_DEVICE	0x2
#define GP_DEVICE	0x3

/* TAP information */
#define OMAP2420_TAP_BASE     (0x48014000)
#define TAP_IDCODE_REG        (OMAP2420_TAP_BASE+0x204)
#define PRODUCTION_ID         (OMAP2420_TAP_BASE+0x208)

/* GPMC */
#define OMAP2420_GPMC_BASE    (0x6800A000)
#define GPMC_SYSCONFIG        (OMAP2420_GPMC_BASE+0x10)
#define GPMC_IRQENABLE        (OMAP2420_GPMC_BASE+0x1C)
#define GPMC_TIMEOUT_CONTROL  (OMAP2420_GPMC_BASE+0x40)
#define GPMC_CONFIG           (OMAP2420_GPMC_BASE+0x50)
#define GPMC_CONFIG1_0        (OMAP2420_GPMC_BASE+0x60)
#define GPMC_CONFIG2_0        (OMAP2420_GPMC_BASE+0x64)
#define GPMC_CONFIG3_0        (OMAP2420_GPMC_BASE+0x68)
#define GPMC_CONFIG4_0        (OMAP2420_GPMC_BASE+0x6C)
#define GPMC_CONFIG5_0        (OMAP2420_GPMC_BASE+0x70)
#define GPMC_CONFIG6_0        (OMAP2420_GPMC_BASE+0x74)
#define GPMC_CONFIG7_0	      (OMAP2420_GPMC_BASE+0x78)
#define GPMC_CONFIG1_1        (OMAP2420_GPMC_BASE+0x90)
#define GPMC_CONFIG2_1        (OMAP2420_GPMC_BASE+0x94)
#define GPMC_CONFIG3_1        (OMAP2420_GPMC_BASE+0x98)
#define GPMC_CONFIG4_1        (OMAP2420_GPMC_BASE+0x9C)
#define GPMC_CONFIG5_1        (OMAP2420_GPMC_BASE+0xA0)
#define GPMC_CONFIG6_1        (OMAP2420_GPMC_BASE+0xA4)
#define GPMC_CONFIG7_1	      (OMAP2420_GPMC_BASE+0xA8)

/* SMS */
#define OMAP2420_SMS_BASE 0x68008000
#define SMS_SYSCONFIG     (OMAP2420_SMS_BASE+0x10)
#define SMS_CLASS_ARB0    (OMAP2420_SMS_BASE+0xD0)
# define BURSTCOMPLETE_GROUP7    BIT31

/* SDRC */
#define OMAP2420_SDRC_BASE 0x68009000
#define SDRC_SYSCONFIG     (OMAP2420_SDRC_BASE+0x10)
#define SDRC_STATUS        (OMAP2420_SDRC_BASE+0x14)
#define SDRC_CS_CFG        (OMAP2420_SDRC_BASE+0x40)
#define SDRC_SHARING       (OMAP2420_SDRC_BASE+0x44)
#define SDRC_DLLA_CTRL     (OMAP2420_SDRC_BASE+0x60)
#define SDRC_DLLB_CTRL     (OMAP2420_SDRC_BASE+0x68)
#define SDRC_POWER         (OMAP2420_SDRC_BASE+0x70)
#define SDRC_MCFG_0        (OMAP2420_SDRC_BASE+0x80)
#define SDRC_MR_0          (OMAP2420_SDRC_BASE+0x84)
#define SDRC_ACTIM_CTRLA_0 (OMAP2420_SDRC_BASE+0x9C)
#define SDRC_ACTIM_CTRLB_0 (OMAP2420_SDRC_BASE+0xA0)
#define SDRC_ACTIM_CTRLA_1 (OMAP2420_SDRC_BASE+0xC4)
#define SDRC_ACTIM_CTRLB_1 (OMAP2420_SDRC_BASE+0xC8)
#define SDRC_RFR_CTRL      (OMAP2420_SDRC_BASE+0xA4)
#define SDRC_MANUAL_0      (OMAP2420_SDRC_BASE+0xA8)
#define OMAP2420_SDRC_CS0  0x80000000
#define OMAP2420_SDRC_CS1  0xA0000000
#define CMD_NOP            0x0
#define CMD_PRECHARGE      0x1
#define CMD_AUTOREFRESH    0x2
#define CMD_ENTR_PWRDOWN   0x3
#define CMD_EXIT_PWRDOWN   0x4
#define CMD_ENTR_SRFRSH    0x5
#define CMD_CKE_HIGH       0x6
#define CMD_CKE_LOW        0x7
#define SOFTRESET          BIT1
#define SMART_IDLE         (0x2 << 3)
#define REF_ON_IDLE        (0x1 << 6)


/* UART */
#define OMAP2420_UART1	      0x4806A000
#define OMAP2420_UART2	      0x4806C000
#define OMAP2420_UART3        0x4806E000

/* General Purpose Timers */
#define OMAP2420_GPT1         0x48028000
#define OMAP2420_GPT2         0x4802A000
#define OMAP2420_GPT3         0x48078000
#define OMAP2420_GPT4         0x4807A000
#define OMAP2420_GPT5         0x4807C000
#define OMAP2420_GPT6         0x4807E000
#define OMAP2420_GPT7         0x48080000
#define OMAP2420_GPT8         0x48082000
#define OMAP2420_GPT9         0x48084000
#define OMAP2420_GPT10        0x48086000
#define OMAP2420_GPT11        0x48088000
#define OMAP2420_GPT12        0x4808A000

/* timer regs offsets (32 bit regs) */
#define TIDR       0x0      /* r */
#define TIOCP_CFG  0x10     /* rw */
#define TISTAT     0x14     /* r */
#define TISR       0x18     /* rw */
#define TIER       0x1C     /* rw */
#define TWER       0x20     /* rw */
#define TCLR       0x24     /* rw */
#define TCRR       0x28     /* rw */
#define TLDR       0x2C     /* rw */
#define TTGR       0x30     /* rw */
#define TWPS       0x34     /* r */
#define TMAR       0x38     /* rw */
#define TCAR1      0x3c     /* r */
#define TSICR      0x40     /* rw */
#define TCAR2      0x44     /* r */

/* WatchDog Timers (1 secure, 3 GP) */
#define WD1_BASE              0x48020000
#define WD2_BASE              0x48022000
#define WD3_BASE              0x48024000
#define WD4_BASE              0x48026000
#define WWPS       0x34     /* r */
#define WSPR       0x48     /* rw */
#define WD_UNLOCK1 0xAAAA
#define WD_UNLOCK2 0x5555

/* PRCM */
#define OMAP2420_CM_BASE 0x48008000
#define PRCM_CLKCFG_CTRL (OMAP2420_CM_BASE+0x080)
#define CM_CLKSEL_MPU    (OMAP2420_CM_BASE+0x140)
#define CM_FCLKEN1_CORE  (OMAP2420_CM_BASE+0x200)
#define CM_FCLKEN2_CORE  (OMAP2420_CM_BASE+0x204)
#define CM_ICLKEN1_CORE  (OMAP2420_CM_BASE+0x210)
#define CM_ICLKEN2_CORE  (OMAP2420_CM_BASE+0x214)
#define CM_CLKSEL1_CORE  (OMAP2420_CM_BASE+0x240)
#define CM_CLKSEL_WKUP   (OMAP2420_CM_BASE+0x440)
#define CM_CLKSEL2_CORE  (OMAP2420_CM_BASE+0x244)
#define CM_CLKSEL_GFX    (OMAP2420_CM_BASE+0x340)
#define PM_RSTCTRL_WKUP  (OMAP2420_CM_BASE+0x450)
#define CM_CLKEN_PLL     (OMAP2420_CM_BASE+0x500)
#define CM_IDLEST_CKGEN  (OMAP2420_CM_BASE+0x520)
#define CM_CLKSEL1_PLL   (OMAP2420_CM_BASE+0x540)
#define CM_CLKSEL2_PLL   (OMAP2420_CM_BASE+0x544)
#define CM_CLKSEL_DSP    (OMAP2420_CM_BASE+0x840)

/*
 * H4 specific Section
 */

/*
 *  The 2420's chip selects are programmable.  The mask ROM
 *  does configure CS0 to 0x08000000 before dispatch.  So, if
 *  you want your code to live below that address, you have to
 *  be prepared to jump though hoops, to reset the base address.
 */
#if defined(CONFIG_OMAP2420H4)
/* GPMC */
#ifdef CONFIG_VIRTIO_A        /* Pre version B */
# define H4_CS0_BASE           0x08000000  /* flash (64 Meg aligned) */
# define H4_CS1_BASE           0x04000000  /* debug board */
# define H4_CS2_BASE           0x0A000000  /* wifi board */
#else
# define H4_CS0_BASE           0x04000000  /* flash (64 Meg aligned) */
# define H4_CS1_BASE           0x08000000  /* debug board */
# define H4_CS2_BASE           0x0A000000  /* wifi board */
#endif

/* base address for indirect vectors (internal boot mode) */
#define SRAM_OFFSET0          0x40000000
#define SRAM_OFFSET1          0x00200000
#define SRAM_OFFSET2          0x0000F800
#define SRAM_VECT_CODE       (SRAM_OFFSET0|SRAM_OFFSET1|SRAM_OFFSET2)

#define LOW_LEVEL_SRAM_STACK  0x4020FFFC

#define PERIFERAL_PORT_BASE   0x480FE003

/* FPGA on Debug board.*/
#define ETH_CONTROL_REG       (H4_CS1_BASE+0x30b)
#define LAN_RESET_REGISTER    (H4_CS1_BASE+0x1c)
#endif  /* endif CONFIG_2420H4 */

#endif
