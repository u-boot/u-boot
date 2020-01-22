/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Interrupt Timer Subsystem
 *
 * Copyright (C) 2017 Intel Corporation.
 * Copyright 2019 Google LLC
 *
 * Modified from coreboot itss.h
 */

#ifndef _ASM_ARCH_ITSS_H
#define _ASM_ARCH_ITSS_H

#define GPIO_IRQ_START	50
#define GPIO_IRQ_END	ITSS_MAX_IRQ

#define ITSS_MAX_IRQ	119
#define IRQS_PER_IPC	32
#define NUM_IPC_REGS	((ITSS_MAX_IRQ + IRQS_PER_IPC - 1) / IRQS_PER_IPC)

/* Max PXRC registers in ITSS */
#define MAX_PXRC_CONFIG	(PCR_ITSS_PIRQH_ROUT - PCR_ITSS_PIRQA_ROUT + 1)

/* PIRQA Routing Control Register */
#define PCR_ITSS_PIRQA_ROUT	0x3100
/* PIRQB Routing Control Register */
#define PCR_ITSS_PIRQB_ROUT	0x3101
/* PIRQC Routing Control Register */
#define PCR_ITSS_PIRQC_ROUT	0x3102
/* PIRQD Routing Control Register */
#define PCR_ITSS_PIRQD_ROUT	0x3103
/* PIRQE Routing Control Register */
#define PCR_ITSS_PIRQE_ROUT	0x3104
/* PIRQF Routing Control Register */
#define PCR_ITSS_PIRQF_ROUT	0x3105
/* PIRQG Routing Control Register */
#define PCR_ITSS_PIRQG_ROUT	0x3106
/* PIRQH Routing Control Register */
#define PCR_ITSS_PIRQH_ROUT	0x3107
/* ITSS Interrupt polarity control */
#define PCR_ITSS_IPC0_CONF	0x3200
/* ITSS Power reduction control */
#define PCR_ITSS_ITSSPRC	0x3300

#endif /* _ASM_ARCH_ITSS_H */
