/*
 * Port Masks
 */

#ifndef __BFIN_PERIPHERAL_PORT__
#define __BFIN_PERIPHERAL_PORT__

/* PORTx_MUX Masks */
#define PORT_x_MUX_0_MASK	0x00000003
#define PORT_x_MUX_1_MASK	0x0000000C
#define PORT_x_MUX_2_MASK	0x00000030
#define PORT_x_MUX_3_MASK	0x000000C0
#define PORT_x_MUX_4_MASK	0x00000300
#define PORT_x_MUX_5_MASK	0x00000C00
#define PORT_x_MUX_6_MASK	0x00003000
#define PORT_x_MUX_7_MASK	0x0000C000
#define PORT_x_MUX_8_MASK	0x00030000
#define PORT_x_MUX_9_MASK	0x000C0000
#define PORT_x_MUX_10_MASK	0x00300000
#define PORT_x_MUX_11_MASK	0x00C00000
#define PORT_x_MUX_12_MASK	0x03000000
#define PORT_x_MUX_13_MASK	0x0C000000
#define PORT_x_MUX_14_MASK	0x30000000
#define PORT_x_MUX_15_MASK	0xC0000000

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
#define PORT_x_MUX_8_FUNC_1	(PORT_x_MUX_FUNC_1 << 16)
#define PORT_x_MUX_8_FUNC_2	(PORT_x_MUX_FUNC_2 << 16)
#define PORT_x_MUX_8_FUNC_3	(PORT_x_MUX_FUNC_3 << 16)
#define PORT_x_MUX_8_FUNC_4	(PORT_x_MUX_FUNC_4 << 16)
#define PORT_x_MUX_9_FUNC_1	(PORT_x_MUX_FUNC_1 << 18)
#define PORT_x_MUX_9_FUNC_2	(PORT_x_MUX_FUNC_2 << 18)
#define PORT_x_MUX_9_FUNC_3	(PORT_x_MUX_FUNC_3 << 18)
#define PORT_x_MUX_9_FUNC_4	(PORT_x_MUX_FUNC_4 << 18)
#define PORT_x_MUX_10_FUNC_1	(PORT_x_MUX_FUNC_1 << 20)
#define PORT_x_MUX_10_FUNC_2	(PORT_x_MUX_FUNC_2 << 20)
#define PORT_x_MUX_10_FUNC_3	(PORT_x_MUX_FUNC_3 << 20)
#define PORT_x_MUX_10_FUNC_4	(PORT_x_MUX_FUNC_4 << 20)
#define PORT_x_MUX_11_FUNC_1	(PORT_x_MUX_FUNC_1 << 22)
#define PORT_x_MUX_11_FUNC_2	(PORT_x_MUX_FUNC_2 << 22)
#define PORT_x_MUX_11_FUNC_3	(PORT_x_MUX_FUNC_3 << 22)
#define PORT_x_MUX_11_FUNC_4	(PORT_x_MUX_FUNC_4 << 22)
#define PORT_x_MUX_12_FUNC_1	(PORT_x_MUX_FUNC_1 << 24)
#define PORT_x_MUX_12_FUNC_2	(PORT_x_MUX_FUNC_2 << 24)
#define PORT_x_MUX_12_FUNC_3	(PORT_x_MUX_FUNC_3 << 24)
#define PORT_x_MUX_12_FUNC_4	(PORT_x_MUX_FUNC_4 << 24)
#define PORT_x_MUX_13_FUNC_1	(PORT_x_MUX_FUNC_1 << 26)
#define PORT_x_MUX_13_FUNC_2	(PORT_x_MUX_FUNC_2 << 26)
#define PORT_x_MUX_13_FUNC_3	(PORT_x_MUX_FUNC_3 << 26)
#define PORT_x_MUX_13_FUNC_4	(PORT_x_MUX_FUNC_4 << 26)
#define PORT_x_MUX_14_FUNC_1	(PORT_x_MUX_FUNC_1 << 28)
#define PORT_x_MUX_14_FUNC_2	(PORT_x_MUX_FUNC_2 << 28)
#define PORT_x_MUX_14_FUNC_3	(PORT_x_MUX_FUNC_3 << 28)
#define PORT_x_MUX_14_FUNC_4	(PORT_x_MUX_FUNC_4 << 28)
#define PORT_x_MUX_15_FUNC_1	(PORT_x_MUX_FUNC_1 << 30)
#define PORT_x_MUX_15_FUNC_2	(PORT_x_MUX_FUNC_2 << 30)
#define PORT_x_MUX_15_FUNC_3	(PORT_x_MUX_FUNC_3 << 30)
#define PORT_x_MUX_15_FUNC_4	(PORT_x_MUX_FUNC_4 << 30)

#include "../mach-common/bits/ports-a.h"
#include "../mach-common/bits/ports-b.h"
#include "../mach-common/bits/ports-c.h"
#include "../mach-common/bits/ports-d.h"
#include "../mach-common/bits/ports-e.h"
#include "../mach-common/bits/ports-f.h"
#include "../mach-common/bits/ports-g.h"

#endif
