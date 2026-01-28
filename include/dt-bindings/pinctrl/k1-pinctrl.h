/* SPDX-License-Identifier: (GPL-2.0 OR MIT) */
/*
 * Copyright (C) 2025-2026 RISCStar Ltd.
 */

#ifndef __DT_BINDINGS_K1_PINCTRL_H
#define __DT_BINDINGS_K1_PINCTRL_H

/* pin mux */
#define MUX_MODE0	0
#define MUX_MODE1	1
#define MUX_MODE2	2
#define MUX_MODE3	3
#define MUX_MODE4	4
#define MUX_MODE5	5
#define MUX_MODE6	6
#define MUX_MODE7	7

/* strong pull resistor */
#define SPU_EN		BIT(3)

/* edge detect */
#define EDGE_NONE	BIT(6)
#define EDGE_RISE	BIT(4)
#define EDGE_FALL	BIT(5)
#define EDGE_BOTH	(EDGE_RISE | EDGE_FALL)

/* slew rate output control */
#define SLE_EN		BIT(7)

/* schmitter trigger input threshold */
#define ST00		(0 << 8)
#define ST01		BIT(8)
#define ST02		BIT(9)
#define ST03		(BIT(8) | BIT(9))

/* driver strength*/
#define PAD_DS_3V	BIT(10)
#define PAD_DS_SLOW0	(0 << 11)
#define PAD_DS_SLOW1	BIT(11)
#define PAD_DS_MEDIUM	BIT(12)
#define PAD_DS_FAST	(BIT(11) | BIT(12))

#define PAD_1V8_DS0	PAD_DS_SLOW0
#define PAD_1V8_DS1	PAD_DS_SLOW1
#define PAD_1V8_DS2	PAD_DS_MEDIUM
#define PAD_1V8_DS3	PAD_DS_FAST

#define PAD_3V_DS0	(PAD_DS_SLOW0 | PAD_DS_3V)
#define PAD_3V_DS1	(PAD_DS_SLOW1 | PAD_DS_3V)
#define PAD_3V_DS2	(PAD_DS_MEDIUM | PAD_DS_3V)
#define PAD_3V_DS3	(PAD_DS_FAST | PAD_DS_3V)

/* pull up/down */
#define PULL_DIS	(0 << 13)	/* bit[15:13] 000 */
#define PULL_UP		(6 << 13)	/* bit[15:13] 110 */
#define PULL_DOWN	(5 << 13)	/* bit[15:13] 101 */

#endif /* __DT_BINDINGS_K1_PINCTRL_H */
