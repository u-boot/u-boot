#ifndef __GIC_H__
#define __GIC_H__

/* Register offsets for the ARM generic interrupt controller (GIC) */

#define GIC_DIST_OFFSET		0x1000
#define GIC_CPU_OFFSET_A9	0x0100
#define GIC_CPU_OFFSET_A15	0x2000

/* Distributor Registers */
#define GICD_CTLR		0x0000
#define GICD_TYPER		0x0004
#define GICD_IIDR		0x0008
#define GICD_STATUSR		0x0010
#define GICD_SETSPI_NSR		0x0040
#define GICD_CLRSPI_NSR		0x0048
#define GICD_SETSPI_SR		0x0050
#define GICD_CLRSPI_SR		0x0058
#define GICD_SEIR		0x0068
#define GICD_IGROUPRn		0x0080
#define GICD_ISENABLERn		0x0100
#define GICD_ICENABLERn		0x0180
#define GICD_ISPENDRn		0x0200
#define GICD_ICPENDRn		0x0280
#define GICD_ISACTIVERn		0x0300
#define GICD_ICACTIVERn		0x0380
#define GICD_IPRIORITYRn	0x0400
#define GICD_ITARGETSRn		0x0800
#define GICD_ICFGR		0x0c00
#define GICD_IGROUPMODRn	0x0d00
#define GICD_NSACRn		0x0e00
#define GICD_SGIR		0x0f00
#define GICD_CPENDSGIRn		0x0f10
#define GICD_SPENDSGIRn		0x0f20
#define GICD_IROUTERn		0x6000

/* Cpu Interface Memory Mapped Registers */
#define GICC_CTLR		0x0000
#define GICC_PMR		0x0004
#define GICC_BPR		0x0008
#define GICC_IAR		0x000C
#define GICC_EOIR		0x0010
#define GICC_RPR		0x0014
#define GICC_HPPIR		0x0018
#define GICC_ABPR		0x001c
#define GICC_AIAR		0x0020
#define GICC_AEOIR		0x0024
#define GICC_AHPPIR		0x0028
#define GICC_APRn		0x00d0
#define GICC_NSAPRn		0x00e0
#define GICC_IIDR		0x00fc
#define GICC_DIR		0x1000

#endif /* __GIC_H__ */
