/*
 * Port Masks
 */

#ifndef __BFIN_PERIPHERAL_PORT__
#define __BFIN_PERIPHERAL_PORT__

/* PORTx_MUX Masks */
#define PORT_x_MUX_0_MASK	0x0003
#define PORT_x_MUX_1_MASK	0x000C
#define PORT_x_MUX_2_MASK	0x0030
#define PORT_x_MUX_3_MASK	0x00C0
#define PORT_x_MUX_4_MASK	0x0300
#define PORT_x_MUX_5_MASK	0x0C00
#define PORT_x_MUX_6_MASK	0x3000
#define PORT_x_MUX_7_MASK	0xC000

#define PORT_x_MUX_FUNC_1	(0x0)
#define PORT_x_MUX_FUNC_2	(0x1)
#define PORT_x_MUX_FUNC_3	(0x2)
#define PORT_x_MUX_FUNC_4	(0x3)
#define PORT_x_MUX_0_FUNC_1	(PORT_x_MUX_FUNC_1 << 0)
#define PORT_x_MUX_0_FUNC_2	(PORT_x_MUX_FUNC_2 << 0)
#define PORT_x_MUX_0_FUNC_3	(PORT_x_MUX_FUNC_3 << 0)
#define PORT_x_MUX_0_FUNC_4	(PORT_x_MUX_FUNC_4 << 0)
#define PORT_x_MUX_1_FUNC_1	(PORT_x_MUX_FUNC_1 << 2)
#define PORT_x_MUX_1_FUNC_2	(PORT_x_MUX_FUNC_2 << 2)
#define PORT_x_MUX_1_FUNC_3	(PORT_x_MUX_FUNC_3 << 2)
#define PORT_x_MUX_1_FUNC_4	(PORT_x_MUX_FUNC_4 << 2)
#define PORT_x_MUX_2_FUNC_1	(PORT_x_MUX_FUNC_1 << 4)
#define PORT_x_MUX_2_FUNC_2	(PORT_x_MUX_FUNC_2 << 4)
#define PORT_x_MUX_2_FUNC_3	(PORT_x_MUX_FUNC_3 << 4)
#define PORT_x_MUX_2_FUNC_4	(PORT_x_MUX_FUNC_4 << 4)
#define PORT_x_MUX_3_FUNC_1	(PORT_x_MUX_FUNC_1 << 6)
#define PORT_x_MUX_3_FUNC_2	(PORT_x_MUX_FUNC_2 << 6)
#define PORT_x_MUX_3_FUNC_3	(PORT_x_MUX_FUNC_3 << 6)
#define PORT_x_MUX_3_FUNC_4	(PORT_x_MUX_FUNC_4 << 6)
#define PORT_x_MUX_4_FUNC_1	(PORT_x_MUX_FUNC_1 << 8)
#define PORT_x_MUX_4_FUNC_2	(PORT_x_MUX_FUNC_2 << 8)
#define PORT_x_MUX_4_FUNC_3	(PORT_x_MUX_FUNC_3 << 8)
#define PORT_x_MUX_4_FUNC_4	(PORT_x_MUX_FUNC_4 << 8)
#define PORT_x_MUX_5_FUNC_1	(PORT_x_MUX_FUNC_1 << 10)
#define PORT_x_MUX_5_FUNC_2	(PORT_x_MUX_FUNC_2 << 10)
#define PORT_x_MUX_5_FUNC_3	(PORT_x_MUX_FUNC_3 << 10)
#define PORT_x_MUX_5_FUNC_4	(PORT_x_MUX_FUNC_4 << 10)
#define PORT_x_MUX_6_FUNC_1	(PORT_x_MUX_FUNC_1 << 12)
#define PORT_x_MUX_6_FUNC_2	(PORT_x_MUX_FUNC_2 << 12)
#define PORT_x_MUX_6_FUNC_3	(PORT_x_MUX_FUNC_3 << 12)
#define PORT_x_MUX_6_FUNC_4	(PORT_x_MUX_FUNC_4 << 12)
#define PORT_x_MUX_7_FUNC_1	(PORT_x_MUX_FUNC_1 << 14)
#define PORT_x_MUX_7_FUNC_2	(PORT_x_MUX_FUNC_2 << 14)
#define PORT_x_MUX_7_FUNC_3	(PORT_x_MUX_FUNC_3 << 14)
#define PORT_x_MUX_7_FUNC_4	(PORT_x_MUX_FUNC_4 << 14)

#include "../mach-common/bits/ports-f.h"
#include "../mach-common/bits/ports-g.h"
#include "../mach-common/bits/ports-h.h"

#endif
