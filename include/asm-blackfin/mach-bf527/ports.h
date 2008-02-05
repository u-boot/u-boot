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

/* Port F Masks */
#define PF0			0x0001
#define PF1			0x0002
#define PF2			0x0004
#define PF3			0x0008
#define PF4			0x0010
#define PF5			0x0020
#define PF6			0x0040
#define PF7			0x0080
#define PF8			0x0100
#define PF9			0x0200
#define PF10			0x0400
#define PF11			0x0800
#define PF12			0x1000
#define PF13			0x2000
#define PF14			0x4000
#define PF15			0x8000

/* Port G Masks */
#define PG0			0x0001
#define PG1			0x0002
#define PG2			0x0004
#define PG3			0x0008
#define PG4			0x0010
#define PG5			0x0020
#define PG6			0x0040
#define PG7			0x0080
#define PG8			0x0100
#define PG9			0x0200
#define PG10			0x0400
#define PG11			0x0800
#define PG12			0x1000
#define PG13			0x2000
#define PG14			0x4000
#define PG15			0x8000

/* Port H Masks */
#define PH0			0x0001
#define PH1			0x0002
#define PH2			0x0004
#define PH3			0x0008
#define PH4			0x0010
#define PH5			0x0020
#define PH6			0x0040
#define PH7			0x0080
#define PH8			0x0100
#define PH9			0x0200
#define PH10			0x0400
#define PH11			0x0800
#define PH12			0x1000
#define PH13			0x2000
#define PH14			0x4000
#define PH15			0x8000

/* Port J Masks */
#define PJ0			0x0001
#define PJ1			0x0002
#define PJ2			0x0004
#define PJ3			0x0008
#define PJ4			0x0010
#define PJ5			0x0020
#define PJ6			0x0040
#define PJ7			0x0080
#define PJ8			0x0100
#define PJ9			0x0200
#define PJ10			0x0400
#define PJ11			0x0800
#define PJ12			0x1000
#define PJ13			0x2000
#define PJ14			0x4000
#define PJ15			0x8000

#endif
