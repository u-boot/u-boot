#ifndef __GIC_V2_H__
#define __GIC_V2_H__

/* register offsets for the ARM generic interrupt controller (GIC) */

#define GIC_DIST_OFFSET		0x1000
#define GICD_CTLR		0x0000
#define GICD_TYPER		0x0004
#define GICD_IGROUPRn		0x0080
#define GICD_SGIR		0x0F00

#define GIC_CPU_OFFSET_A9	0x0100
#define GIC_CPU_OFFSET_A15	0x2000
#define GICC_CTLR		0x0000
#define GICC_PMR		0x0004
#define GICC_IAR		0x000C
#define GICC_EOIR		0x0010

#endif
