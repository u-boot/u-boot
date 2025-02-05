/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Specialty padding for the R-Car Gen2 SPL JTAG loading
 */

#ifndef __BOOT0_H
#define __BOOT0_H

#if IS_ENABLED(CONFIG_RCAR_GEN2)
_start:
	ARM_VECTORS

#ifdef CONFIG_XPL_BUILD
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
	.word	0x0badc0d3;
#endif
#endif

#if IS_ENABLED(CONFIG_R8A779G0)

#ifdef CONFIG_XPL_BUILD
	/* r1=0xe6170800 */
	.inst	0xe3a004e6	/* mov     r0,     #0xe6000000 */
	.inst	0xe3801817	/* orr     r1, r0, #0x170000 */
	.inst	0xe3814b02	/* orr     r4, r1, #0x800 */

	/* r0=0xe6280000 */
	.inst	0xe380070a	/* orr     r0, r0, #0x280000 */

	/* APMU_RVBARPLC0 = (address of 'b reset' below) | CA_CORE0_VLD_RVBARP */
	.inst	0xe28f30a8	/* add     r3, pc, #0xa8 */
	.inst	0xe3833001	/* orr     r3, r3, #1 */
	.inst	0xe5843038	/* str     r3, [r4, #56]   @ 0x38 */

	/* APMU_RVBARPHC0 = 0 */
	.inst	0xe3a05000	/* mov     r5, #0 */
	.inst	0xe584503c	/* str     r5, [r4, #60]   @ 0x3c */

	/* PRR & 0xff00 ?= 0x5c00, test if this is V4H or V4M */
	.inst	0xe3a024ff	/* mov     r2, #0xff000000 */
	.inst	0xe382260f	/* orr     r2, r2, #0xf00000 */
	.inst	0xe5923044	/* ldr     r3, [r2, #68]   @ 0x44 */
	.inst	0xe2033cff	/* and     r3, r3, #0xff00 */
	.inst	0xe3530b17	/* cmp     r3, #0x5c00 */
	.inst	0x1a00000a	/* bne     68 <reset-0x18> */
	/* if (SoC is V4H) { */
	/* AP_CORE_APSREG_AP_CLUSTER_N_AUX0 |= AP_CORE_APSREG_AP_CLUSTER_N_AUX0_INIT */
	.inst	0xe5903010	/* ldr     r3, [r0, #16] */
	.inst	0xe3833003	/* orr     r3, r3, #3 */
	.inst	0xe5803010	/* str     r3, [r0, #16] */
	/* AP_CORE_APSREG_CCI500_AUX |= AP_CORE_APSREG_CCI500_AUX_ACTDIS */
	.inst	0xe3800a09	/* orr     r0, r0, #36864  @ 0x9000 */
	.inst	0xe5903010	/* ldr     r3, [r0, #16] */
	.inst	0xe3833001	/* orr     r3, r3, #1 */
	.inst	0xe5803010	/* str     r3, [r0, #16] */
	/* AP_CORE_APSREG_P_CCI500_AUX |= AP_CORE_APSREG_P_CCI500_AUX_ASPRTM */
	.inst	0xe3800802	/* orr     r0, r0, #131072 @ 0x20000 */
	.inst	0xe5903010	/* ldr     r3, [r0, #16] */
	.inst	0xe3833002	/* orr     r3, r3, #2 */
	.inst	0xe5803010	/* str     r3, [r0, #16] */
	/* } */
	/* APMU_PWRCTRLC0 = CA_CORE0_WUP_REQ */
	.inst	0xe3a03001	/* mov     r3, #1 */
	.inst	0xe5843000	/* str     r3, [r4] */
	/* Test for APMU_CRBARP valid BAR flags and jump to CR entry point */
	.inst	0xe3814c03	/* orr     r4, r1, #768    @ 0x300 */
	.inst	0xe584503c	/* str     r5, [r4, #60]   @ 0x3c */
	.inst	0xe594203c	/* ldr     r2, [r4, #60]   @ 0x3c */
	.inst	0xe20230ff	/* and     r3, r2, #255    @ 0xff */
	.inst	0xe3530011	/* cmp     r3, #17 */
	.inst	0x1afffffb	/* bne     78 <reset-0x28> */
	/* Invalidate icache before jump to follow up software */
	.inst	0xe3a00000	/* mov     r0, #0 */
	.inst	0xee070f15	/* mcr     15, 0, r0, cr7, cr5, {0} */
	.inst	0xf57ff04f	/* dsb     sy */
	.inst	0xf57ff06f	/* isb     sy */
	/* Jump to follow up software */
	.inst	0xe1a02922	/* lsr     r2, r2, #18 */
	.inst	0xe1a02902	/* lsl     r2, r2, #18 */
	.inst	0xe1a0f002	/* mov     pc, r2 */
	.inst	0xeafffffe	/* b       94 <reset-0xc> */
	.inst	0xe1a00000	/* nop                     @ (mov r0, r0) */
	.inst	0xe1a00000	/* nop                     @ (mov r0, r0) */
	.inst	0xe1a00000	/* nop                     @ (mov r0, r0) */
	.inst	0xe1a00000	/* nop                     @ (mov r0, r0) */
	.inst	0xe1a00000	/* nop                     @ (mov r0, r0) */
	.inst	0xe1a00000	/* nop                     @ (mov r0, r0) */
	/* Offset 0xa0 */
#endif
	b	reset
#endif

#endif /* __BOOT0_H */
