/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _DRAMC1_H
#define _DRAMC1_H
#include <stdio.h>
#include <dm/device.h>
#include <linux/delay.h>
//========================
// Compile Option To Enable Functions
//========================
#define DRAMC0_BASE 0x10214000
#define DDRPHY_BASE 0x10213000
#define EMI_BASE 0x10203000
#define DRAMC_NAO_BASE 0x1020e000
#define DRAM_BASE 0x40000000ULL
#define ENABLE_DFS
#define TOPRGU_BASE	     0x10212000
#define TIMEOUT 3
#define CQ_DMA_BASE	0x10217c00

// EMI address definition.
#define EMI_CONA        EMI_BASE+0x000
#define EMI_CONB		EMI_BASE+0x008
#define EMI_CONC		EMI_BASE+0x010
#define EMI_COND		EMI_BASE+0x018
#define EMI_CONE		EMI_BASE+0x020
#define EMI_CONF        EMI_BASE+0x028
#define EMI_CONG		EMI_BASE+0x030
#define EMI_CONH		EMI_BASE+0x038
#define EMI_CONI		EMI_BASE+0x040
#define EMI_CONM		EMI_BASE+0x060
#define EMI_DFTB		EMI_BASE+0x0E8		// TESTB
#define EMI_DFTD		EMI_BASE+0x0F8		// TESTD
#define EMI_SLCT		EMI_BASE+0x158
#define EMI_ARBA		EMI_BASE+0x100
#define EMI_ARBB		EMI_BASE+0x108
#define EMI_ARBC		EMI_BASE+0x110
#define EMI_ARBD		EMI_BASE+0x118
#define EMI_ARBE		EMI_BASE+0x120
#define EMI_ARBF		EMI_BASE+0x128
#define EMI_ARBG		EMI_BASE+0x130
#define EMI_ARBI		EMI_BASE+0x140
#define EMI_BMEN		EMI_BASE+0x400

//=======================

typedef struct {
    char *name;
    char **factor_tbl;
    char *curr_val;
    char *opt_val;
    void (*factor_handler) (char *);
} tuning_factor;

typedef struct {
    void (*ett_print_banner) (unsigned int);
    void (*ett_print_before_start_loop_zero) (void);
    void (*ett_print_before_each_round_of_loop_zero) (void);
    unsigned int (*ett_print_result) (void);
    void (*ett_print_after_each_round_of_loop_zero) (void);
    void (*ett_calc_opt_value) (unsigned int, unsigned int *, unsigned int *);
    void (*ett_print_after_finish_loop_n) (int);
} print_callbacks;

#define ETT_TUNING_FACTOR_NUMS(x)    (sizeof(x)/sizeof(tuning_factor))

typedef struct {
    int (*test_case) (unsigned int, unsigned int, void *);
    unsigned int start;
    unsigned int range;
    void *ext_arg;
} test_case;

struct mtk_lpddr3_priv;

#define DDR_PHY_RESET() do { \
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_PHYCTL1)) \
        | (1 << 28), \
        DRAMC_PHYCTL1); \
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_GDDR3CTL1)) \
        | (1 << 25),    \
        DRAMC_GDDR3CTL1); \
    udelay(1000); \
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_PHYCTL1)) \
        & (~(1 << 28)),    \
        DRAMC_PHYCTL1); \
    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_GDDR3CTL1)) \
        & (~(1 << 25)),    \
        DRAMC_GDDR3CTL1); \
} while(0)

#define ETT_TUNING_FACTOR_NUMS(x)	(sizeof(x)/sizeof(tuning_factor))


unsigned int DRAMC_READ_REG(unsigned int offset);
void DRAMC_WRITE_REG(unsigned int val, unsigned int offset);
void DRAMC_WRITE_SET(unsigned int val, unsigned int offset);
void DRAMC_WRITE_CLEAR(unsigned int val, unsigned int offset);
/*
#define DRAMC_WRITE_REG(val,offset)  do{ \
                                      (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) = (unsigned int)(val); \
                                      }while(0)

#define DRAMC_WRITE_REG_W(val,offset)     do{ \
                                      (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) = (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) = (unsigned int)(val); \
                                      }while(0)

#define DRAMC_WRITE_REG_H(val,offset)     do{ \
                                      (*(volatile unsigned short *)(DRAMC0_BASE + (offset))) = (unsigned short)(val); \
                                      (*(volatile unsigned short *)(DDRPHY_BASE + (offset))) = (unsigned short)(val); \
                                      (*(volatile unsigned short *)(DDRPHY_BASE + (offset))) = (unsigned short)(val); \
                                      }while(0)
#define DRAMC_WRITE_REG_B(val,offset)     do{ \
                                      (*(volatile unsigned char *)(DRAMC0_BASE + (offset))) = (unsigned char)(val); \
                                      (*(volatile unsigned char *)(DDRPHY_BASE + (offset))) = (unsigned char)(val); \
                                      (*(volatile unsigned char *)(DDRPHY_BASE + (offset))) = (unsigned char)(val); \
                                      }while(0)
#define DRAMC_READ_REG(offset)         ( \
                                        (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) |\
                                        (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) |\
                                        (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) \
                                       )
#define DRAMC_WRITE_SET(val,offset)     do{ \
                                      (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) |= (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) |= (unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) |= (unsigned int)(val); \
                                      }while(0)

#define DRAMC_WRITE_CLEAR(val,offset)     do{ \
                                      (*(volatile unsigned int *)(DRAMC0_BASE + (offset))) &= ~(unsigned int)(val); \
                                      (*(volatile unsigned int *)(DDRPHY_BASE + (offset))) &= ~(unsigned int)(val); \
                                      (*(volatile unsigned int *)(DRAMC_NAO_BASE + (offset))) &= ~(unsigned int)(val); \
                                      }while(0)
*/
#define DDRPHY_WRITE_REG(val,offset)    __raw_writel(val, (DDRPHY_BASE + (offset)))
#define DRAMC0_WRITE_REG(val,offset)    __raw_writel(val, (DRAMC0_BASE + (offset)))
#define DRAMC_NAO_WRITE_REG(val,offset) __raw_writel(val, (DRAMC_NAO_BASE + (offset)))
#define MCUSYS_CFGREG_WRITE_REG(val,offset) __raw_writel(val, (MCUSYS_CFGREG_BASE + (offset)))


#define ETT_TEST_CASE_NUMS(x)	(sizeof(x)/sizeof(test_case))

#define GRAY_ENCODED(a) (a)

#ifndef NULL
#define NULL    0
#endif


/* define supported DRAM types */
enum
{
  TYPE_mDDR = 1,
  TYPE_LPDDR2,
  TYPE_LPDDR3,
  TYPE_PCDDR3,
  TYPE_LPDDR4,
};

extern int do_dqs_gw_calib_2(void);
#endif
