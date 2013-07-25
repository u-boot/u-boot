#ifndef __KAREF_H__
#define __KAREF_H__
/*
 * (C) Copyright 2005
 * Sandburst Corporation
 * Travis B. Sawyer
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Ka Reference Design OFEM FPGA Registers & definitions */
#include "hal_ka_sc_auto.h"
#include "hal_ka_of_auto.h"

typedef struct karef_board_id_s {
	const char name[40];
} KAREF_BOARD_ID_ST, *KAREF_BOARD_ID_PST;

/* SCAN FPGA */
typedef struct karef_fpga_regs_s
{
    volatile unsigned long revision_ul;       /* Read Only  */
    volatile unsigned long reset_ul;          /* Read/Write */
    volatile unsigned long interrupt_ul;      /* Read Only  */
    volatile unsigned long mask_ul;           /* Read/Write */
    volatile unsigned long scratch_ul;        /* Read/Write */
    volatile unsigned long scrmask_ul;        /* Read/Write */
    volatile unsigned long status_ul;         /* Read Only  */
    volatile unsigned long control_ul;        /* Read/Write */
    volatile unsigned long boardinfo_ul;      /* Read Only  */
    volatile unsigned long scan_from0_ul;     /* Read Only  */
    volatile unsigned long scan_from1_ul;     /* Read Only  */
    volatile unsigned long scan_to0_ul;       /* Read/Write */
    volatile unsigned long scan_to1_ul;       /* Read/Write */
    volatile unsigned long scan_control_ul;   /* Read/Write */
    volatile unsigned long pll_control_ul;    /* Read/Write */
    volatile unsigned long core_clock_cnt_ul; /* Read/Write */
    volatile unsigned long dr_clock_cnt_ul;   /* Read/Write */
    volatile unsigned long spi_clock_cnt_ul;  /* Read/Write */
    volatile unsigned long brdout_data_ul;    /* Read/Write */
    volatile unsigned long brdout_enable_ul;  /* Read/Write */
    volatile unsigned long brdin_data_ul;     /* Read Only  */
    volatile unsigned long misc_ul;           /* Read/Write */
} __attribute__((packed)) KAREF_FPGA_REGS_ST , * KAREF_FPGA_REGS_PST;

/* OFEM FPGA */
typedef struct ofem_fpga_regs_s
{
    volatile unsigned long revision_ul;       /* Read Only  */
    volatile unsigned long reset_ul;          /* Read/Write */
    volatile unsigned long interrupt_ul;      /* Read Only  */
    volatile unsigned long mask_ul;           /* Read/Write */
    volatile unsigned long scratch_ul;        /* Read/Write */
    volatile unsigned long scrmask_ul;        /* Read/Write */
    volatile unsigned long control_ul;        /* Read/Write */
    volatile unsigned long mac_flow_ctrl_ul;  /* Read/Write */
} __attribute__((packed)) OFEM_FPGA_REGS_ST , * OFEM_FPGA_REGS_PST;


#endif /* __KAREF_H__ */
