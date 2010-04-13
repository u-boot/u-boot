#ifndef __FSL_SERDES_H
#define __FSL_SERDES_H

#include <config.h>

#define FSL_SERDES_CLK_100		(0 << 28)
#define FSL_SERDES_CLK_125		(1 << 28)
#define FSL_SERDES_CLK_150		(3 << 28)
#define FSL_SERDES_PROTO_SATA		0
#define FSL_SERDES_PROTO_PEX		1
#define FSL_SERDES_PROTO_PEX_X2		2
#define FSL_SERDES_PROTO_SGMII		3
#define FSL_SERDES_VDD_1V		1

#ifdef CONFIG_FSL_SERDES
extern void fsl_setup_serdes(u32 offset, char proto, u32 rfcks, char vdd);
#else
static void fsl_setup_serdes(u32 offset, char proto, u32 rfcks, char vdd) {}
#endif /* CONFIG_FSL_SERDES */

#endif /* __FSL_SERDES_H */
