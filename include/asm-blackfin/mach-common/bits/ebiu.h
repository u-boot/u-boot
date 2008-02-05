/*
 * EBIU Masks
 */

#ifndef __BFIN_PERIPHERAL_EBIU__
#define __BFIN_PERIPHERAL_EBIU__

/* EBIU_AMGCTL Masks */
#define AMCKEN		0x0001		/* Enable CLKOUT */
#define AMBEN_NONE	0x0000		/* All Banks Disabled */
#define AMBEN_B0	0x0002		/* Enable Asynchronous Memory Bank 0 only */
#define AMBEN_B0_B1	0x0004		/* Enable Asynchronous Memory Banks 0 & 1 only */
#define AMBEN_B0_B1_B2	0x0006		/* Enable Asynchronous Memory Banks 0,/ 1, and 2 */
#define AMBEN_ALL	0x0008		/* Enable Asynchronous Memory Banks (all) 0, 1, 2, and 3 */
#define B0_PEN		0x0010		/* Enable 16-bit packing Bank 0 */
#define B1_PEN		0x0020		/* Enable 16-bit packing Bank 1 */
#define B2_PEN		0x0040		/* Enable 16-bit packing Bank 2 */
#define B3_PEN		0x0080		/* Enable 16-bit packing Bank 3 */
#define CDPRIO		0x0100		/* Core has priority over DMA for external accesses */

/* EBIU_AMGCTL Bit Positions */
#define AMCKEN_P	0x00000000	/* Enable CLKOUT */
#define AMBEN_P0	0x00000001	/* Asynchronous Memory Enable, 000 - banks 0-3 disabled, 001 - Bank 0 enabled */
#define AMBEN_P1	0x00000002	/* Asynchronous Memory Enable, 010 - banks 0&1 enabled, 011 - banks 0-3 enabled */
#define AMBEN_P2	0x00000003	/* Asynchronous Memory Enable, 1xx - All banks (bank 0, 1, 2, and 3) enabled */
#define B0_PEN_P	0x00000004	/* Enable 16-bit packing Bank 0 */
#define B1_PEN_P	0x00000005	/* Enable 16-bit packing Bank 1 */
#define B2_PEN_P	0x00000006	/* Enable 16-bit packing Bank 2 */
#define B3_PEN_P	0x00000007	/* Enable 16-bit packing Bank 3 */
#define CDPRIO_P	0x00000008	/* Core has priority over DMA for external accesses */

/* EBIU_AMBCTL0 Masks */
#define B0RDYEN		0x00000001	/* Bank 0 RDY Enable, 0=disable, 1=enable */
#define B0RDYPOL	0x00000002	/* Bank 0 RDY Active high, 0=active low, 1=active high */
#define B0TT_1		0x00000004	/* Bank 0 Transition Time from Read to Write = 1 cycle */
#define B0TT_2		0x00000008	/* Bank 0 Transition Time from Read to Write = 2 cycles */
#define B0TT_3		0x0000000C	/* Bank 0 Transition Time from Read to Write = 3 cycles */
#define B0TT_4		0x00000000	/* Bank 0 Transition Time from Read to Write = 4 cycles */
#define B0ST_1		0x00000010	/* Bank 0 Setup Time from AOE asserted to Read/Write asserted=1 cycle */
#define B0ST_2		0x00000020	/* Bank 0 Setup Time from AOE asserted to Read/Write asserted=2 cycles */
#define B0ST_3		0x00000030	/* Bank 0 Setup Time from AOE asserted to Read/Write asserted=3 cycles */
#define B0ST_4		0x00000000	/* Bank 0 Setup Time from AOE asserted to Read/Write asserted=4 cycles */
#define B0HT_1		0x00000040	/* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 1 cycle */
#define B0HT_2		0x00000080	/* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 2 cycles */
#define B0HT_3		0x000000C0	/* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 3 cycles */
#define B0HT_0		0x00000000	/* Bank 0 Hold Time from Read/Write deasserted to AOE deasserted = 0 cycles */
#define B0RAT_1		0x00000100	/* Bank 0 Read Access Time = 1 cycle */
#define B0RAT_2		0x00000200	/* Bank 0 Read Access Time = 2 cycles */
#define B0RAT_3		0x00000300	/* Bank 0 Read Access Time = 3 cycles */
#define B0RAT_4		0x00000400	/* Bank 0 Read Access Time = 4 cycles */
#define B0RAT_5		0x00000500	/* Bank 0 Read Access Time = 5 cycles */
#define B0RAT_6		0x00000600	/* Bank 0 Read Access Time = 6 cycles */
#define B0RAT_7		0x00000700	/* Bank 0 Read Access Time = 7 cycles */
#define B0RAT_8		0x00000800	/* Bank 0 Read Access Time = 8 cycles */
#define B0RAT_9		0x00000900	/* Bank 0 Read Access Time = 9 cycles */
#define B0RAT_10	0x00000A00	/* Bank 0 Read Access Time = 10 cycles */
#define B0RAT_11	0x00000B00	/* Bank 0 Read Access Time = 11 cycles */
#define B0RAT_12	0x00000C00	/* Bank 0 Read Access Time = 12 cycles */
#define B0RAT_13	0x00000D00	/* Bank 0 Read Access Time = 13 cycles */
#define B0RAT_14	0x00000E00	/* Bank 0 Read Access Time = 14 cycles */
#define B0RAT_15	0x00000F00	/* Bank 0 Read Access Time = 15 cycles */
#define B0WAT_1		0x00001000	/* Bank 0 Write Access Time = 1 cycle */
#define B0WAT_2		0x00002000	/* Bank 0 Write Access Time = 2 cycles */
#define B0WAT_3		0x00003000	/* Bank 0 Write Access Time = 3 cycles */
#define B0WAT_4		0x00004000	/* Bank 0 Write Access Time = 4 cycles */
#define B0WAT_5		0x00005000	/* Bank 0 Write Access Time = 5 cycles */
#define B0WAT_6		0x00006000	/* Bank 0 Write Access Time = 6 cycles */
#define B0WAT_7		0x00007000	/* Bank 0 Write Access Time = 7 cycles */
#define B0WAT_8		0x00008000	/* Bank 0 Write Access Time = 8 cycles */
#define B0WAT_9		0x00009000	/* Bank 0 Write Access Time = 9 cycles */
#define B0WAT_10	0x0000A000	/* Bank 0 Write Access Time = 10 cycles */
#define B0WAT_11	0x0000B000	/* Bank 0 Write Access Time = 11 cycles */
#define B0WAT_12	0x0000C000	/* Bank 0 Write Access Time = 12 cycles */
#define B0WAT_13	0x0000D000	/* Bank 0 Write Access Time = 13 cycles */
#define B0WAT_14	0x0000E000	/* Bank 0 Write Access Time = 14 cycles */
#define B0WAT_15	0x0000F000	/* Bank 0 Write Access Time = 15 cycles */
#define B1RDYEN		0x00010000	/* Bank 1 RDY enable, 0=disable, 1=enable */
#define B1RDYPOL	0x00020000	/* Bank 1 RDY Active high, 0=active low, 1=active high */
#define B1TT_1		0x00040000	/* Bank 1 Transition Time from Read to Write = 1 cycle */
#define B1TT_2		0x00080000	/* Bank 1 Transition Time from Read to Write = 2 cycles */
#define B1TT_3		0x000C0000	/* Bank 1 Transition Time from Read to Write = 3 cycles */
#define B1TT_4		0x00000000	/* Bank 1 Transition Time from Read to Write = 4 cycles */
#define B1ST_1		0x00100000	/* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B1ST_2		0x00200000	/* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B1ST_3		0x00300000	/* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B1ST_4		0x00000000	/* Bank 1 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B1HT_1		0x00400000	/* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B1HT_2		0x00800000	/* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B1HT_3		0x00C00000	/* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B1HT_0		0x00000000	/* Bank 1 Hold Time from Read or Write deasserted to AOE deasserted = 0 cycles */
#define B1RAT_1		0x01000000	/* Bank 1 Read Access Time = 1 cycle */
#define B1RAT_2		0x02000000	/* Bank 1 Read Access Time = 2 cycles */
#define B1RAT_3		0x03000000	/* Bank 1 Read Access Time = 3 cycles */
#define B1RAT_4		0x04000000	/* Bank 1 Read Access Time = 4 cycles */
#define B1RAT_5		0x05000000	/* Bank 1 Read Access Time = 5 cycles */
#define B1RAT_6		0x06000000	/* Bank 1 Read Access Time = 6 cycles */
#define B1RAT_7		0x07000000	/* Bank 1 Read Access Time = 7 cycles */
#define B1RAT_8		0x08000000	/* Bank 1 Read Access Time = 8 cycles */
#define B1RAT_9		0x09000000	/* Bank 1 Read Access Time = 9 cycles */
#define B1RAT_10	0x0A000000	/* Bank 1 Read Access Time = 10 cycles */
#define B1RAT_11	0x0B000000	/* Bank 1 Read Access Time = 11 cycles */
#define B1RAT_12	0x0C000000	/* Bank 1 Read Access Time = 12 cycles */
#define B1RAT_13	0x0D000000	/* Bank 1 Read Access Time = 13 cycles */
#define B1RAT_14	0x0E000000	/* Bank 1 Read Access Time = 14 cycles */
#define B1RAT_15	0x0F000000	/* Bank 1 Read Access Time = 15 cycles */
#define B1WAT_1		0x10000000	/* Bank 1 Write Access Time = 1 cycle */
#define B1WAT_2		0x20000000	/* Bank 1 Write Access Time = 2 cycles */
#define B1WAT_3		0x30000000	/* Bank 1 Write Access Time = 3 cycles */
#define B1WAT_4		0x40000000	/* Bank 1 Write Access Time = 4 cycles */
#define B1WAT_5		0x50000000	/* Bank 1 Write Access Time = 5 cycles */
#define B1WAT_6		0x60000000	/* Bank 1 Write Access Time = 6 cycles */
#define B1WAT_7		0x70000000	/* Bank 1 Write Access Time = 7 cycles */
#define B1WAT_8		0x80000000	/* Bank 1 Write Access Time = 8 cycles */
#define B1WAT_9		0x90000000	/* Bank 1 Write Access Time = 9 cycles */
#define B1WAT_10	0xA0000000	/* Bank 1 Write Access Time = 10 cycles */
#define B1WAT_11	0xB0000000	/* Bank 1 Write Access Time = 11 cycles */
#define B1WAT_12	0xC0000000	/* Bank 1 Write Access Time = 12 cycles */
#define B1WAT_13	0xD0000000	/* Bank 1 Write Access Time = 13 cycles */
#define B1WAT_14	0xE0000000	/* Bank 1 Write Access Time = 14 cycles */
#define B1WAT_15	0xF0000000	/* Bank 1 Write Access Time = 15 cycles */

/* EBIU_AMBCTL1 Masks */
#define B2RDYEN		0x00000001	/* Bank 2 RDY Enable, 0=disable, 1=enable */
#define B2RDYPOL	0x00000002	/* Bank 2 RDY Active high, 0=active low, 1=active high */
#define B2TT_1		0x00000004	/* Bank 2 Transition Time from Read to Write = 1 cycle */
#define B2TT_2		0x00000008	/* Bank 2 Transition Time from Read to Write = 2 cycles */
#define B2TT_3		0x0000000C	/* Bank 2 Transition Time from Read to Write = 3 cycles */
#define B2TT_4		0x00000000	/* Bank 2 Transition Time from Read to Write = 4 cycles */
#define B2ST_1		0x00000010	/* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B2ST_2		0x00000020	/* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B2ST_3		0x00000030	/* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B2ST_4		0x00000000	/* Bank 2 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B2HT_1		0x00000040	/* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B2HT_2		0x00000080	/* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B2HT_3		0x000000C0	/* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B2HT_0		0x00000000	/* Bank 2 Hold Time from Read or Write deasserted to AOE deasserted = 0 cycles */
#define B2RAT_1		0x00000100	/* Bank 2 Read Access Time = 1 cycle */
#define B2RAT_2		0x00000200	/* Bank 2 Read Access Time = 2 cycles */
#define B2RAT_3		0x00000300	/* Bank 2 Read Access Time = 3 cycles */
#define B2RAT_4		0x00000400	/* Bank 2 Read Access Time = 4 cycles */
#define B2RAT_5		0x00000500	/* Bank 2 Read Access Time = 5 cycles */
#define B2RAT_6		0x00000600	/* Bank 2 Read Access Time = 6 cycles */
#define B2RAT_7		0x00000700	/* Bank 2 Read Access Time = 7 cycles */
#define B2RAT_8		0x00000800	/* Bank 2 Read Access Time = 8 cycles */
#define B2RAT_9		0x00000900	/* Bank 2 Read Access Time = 9 cycles */
#define B2RAT_10	0x00000A00	/* Bank 2 Read Access Time = 10 cycles */
#define B2RAT_11	0x00000B00	/* Bank 2 Read Access Time = 11 cycles */
#define B2RAT_12	0x00000C00	/* Bank 2 Read Access Time = 12 cycles */
#define B2RAT_13	0x00000D00	/* Bank 2 Read Access Time = 13 cycles */
#define B2RAT_14	0x00000E00	/* Bank 2 Read Access Time = 14 cycles */
#define B2RAT_15	0x00000F00	/* Bank 2 Read Access Time = 15 cycles */
#define B2WAT_1		0x00001000	/* Bank 2 Write Access Time = 1 cycle */
#define B2WAT_2		0x00002000	/* Bank 2 Write Access Time = 2 cycles */
#define B2WAT_3		0x00003000	/* Bank 2 Write Access Time = 3 cycles */
#define B2WAT_4		0x00004000	/* Bank 2 Write Access Time = 4 cycles */
#define B2WAT_5		0x00005000	/* Bank 2 Write Access Time = 5 cycles */
#define B2WAT_6		0x00006000	/* Bank 2 Write Access Time = 6 cycles */
#define B2WAT_7		0x00007000	/* Bank 2 Write Access Time = 7 cycles */
#define B2WAT_8		0x00008000	/* Bank 2 Write Access Time = 8 cycles */
#define B2WAT_9		0x00009000	/* Bank 2 Write Access Time = 9 cycles */
#define B2WAT_10	0x0000A000	/* Bank 2 Write Access Time = 10 cycles */
#define B2WAT_11	0x0000B000	/* Bank 2 Write Access Time = 11 cycles */
#define B2WAT_12	0x0000C000	/* Bank 2 Write Access Time = 12 cycles */
#define B2WAT_13	0x0000D000	/* Bank 2 Write Access Time = 13 cycles */
#define B2WAT_14	0x0000E000	/* Bank 2 Write Access Time = 14 cycles */
#define B2WAT_15	0x0000F000	/* Bank 2 Write Access Time = 15 cycles */
#define B3RDYEN		0x00010000	/* Bank 3 RDY enable, 0=disable, 1=enable */
#define B3RDYPOL	0x00020000	/* Bank 3 RDY Active high, 0=active low, 1=active high */
#define B3TT_1		0x00040000	/* Bank 3 Transition Time from Read to Write = 1 cycle */
#define B3TT_2		0x00080000	/* Bank 3 Transition Time from Read to Write = 2 cycles */
#define B3TT_3		0x000C0000	/* Bank 3 Transition Time from Read to Write = 3 cycles */
#define B3TT_4		0x00000000	/* Bank 3 Transition Time from Read to Write = 4 cycles */
#define B3ST_1		0x00100000	/* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 1 cycle */
#define B3ST_2		0x00200000	/* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 2 cycles */
#define B3ST_3		0x00300000	/* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 3 cycles */
#define B3ST_4		0x00000000	/* Bank 3 Setup Time from AOE asserted to Read or Write asserted = 4 cycles */
#define B3HT_1		0x00400000	/* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 1 cycle */
#define B3HT_2		0x00800000	/* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 2 cycles */
#define B3HT_3		0x00C00000	/* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 3 cycles */
#define B3HT_0		0x00000000	/* Bank 3 Hold Time from Read or Write deasserted to AOE deasserted = 0 cycles */
#define B3RAT_1		0x01000000	/* Bank 3 Read Access Time = 1 cycle */
#define B3RAT_2		0x02000000	/* Bank 3 Read Access Time = 2 cycles */
#define B3RAT_3		0x03000000	/* Bank 3 Read Access Time = 3 cycles */
#define B3RAT_4		0x04000000	/* Bank 3 Read Access Time = 4 cycles */
#define B3RAT_5		0x05000000	/* Bank 3 Read Access Time = 5 cycles */
#define B3RAT_6		0x06000000	/* Bank 3 Read Access Time = 6 cycles */
#define B3RAT_7		0x07000000	/* Bank 3 Read Access Time = 7 cycles */
#define B3RAT_8		0x08000000	/* Bank 3 Read Access Time = 8 cycles */
#define B3RAT_9		0x09000000	/* Bank 3 Read Access Time = 9 cycles */
#define B3RAT_10	0x0A000000	/* Bank 3 Read Access Time = 10 cycles */
#define B3RAT_11	0x0B000000	/* Bank 3 Read Access Time = 11 cycles */
#define B3RAT_12	0x0C000000	/* Bank 3 Read Access Time = 12 cycles */
#define B3RAT_13	0x0D000000	/* Bank 3 Read Access Time = 13 cycles */
#define B3RAT_14	0x0E000000	/* Bank 3 Read Access Time = 14 cycles */
#define B3RAT_15	0x0F000000	/* Bank 3 Read Access Time = 15 cycles */
#define B3WAT_1		0x10000000	/* Bank 3 Write Access Time = 1 cycle */
#define B3WAT_2		0x20000000	/* Bank 3 Write Access Time = 2 cycles */
#define B3WAT_3		0x30000000	/* Bank 3 Write Access Time = 3 cycles */
#define B3WAT_4		0x40000000	/* Bank 3 Write Access Time = 4 cycles */
#define B3WAT_5		0x50000000	/* Bank 3 Write Access Time = 5 cycles */
#define B3WAT_6		0x60000000	/* Bank 3 Write Access Time = 6 cycles */
#define B3WAT_7		0x70000000	/* Bank 3 Write Access Time = 7 cycles */
#define B3WAT_8		0x80000000	/* Bank 3 Write Access Time = 8 cycles */
#define B3WAT_9		0x90000000	/* Bank 3 Write Access Time = 9 cycles */
#define B3WAT_10	0xA0000000	/* Bank 3 Write Access Time = 10 cycles */
#define B3WAT_11	0xB0000000	/* Bank 3 Write Access Time = 11 cycles */
#define B3WAT_12	0xC0000000	/* Bank 3 Write Access Time = 12 cycles */
#define B3WAT_13	0xD0000000	/* Bank 3 Write Access Time = 13 cycles */
#define B3WAT_14	0xE0000000	/* Bank 3 Write Access Time = 14 cycles */
#define B3WAT_15	0xF0000000	/* Bank 3 Write Access Time = 15 cycles */

/* Only available on newer parts */
#ifdef EBIU_MODE

/* EBIU_MBSCTL Bit Positions */
#define AMSB0CTL_P	0
#define AMSB1CTL_P	2
#define AMSB2CTL_P	4
#define AMSB3CTL_P	6

/* EBIU_MBSCTL Masks */
#define AMSB0CTL_MASK	(0x3 << AMSB0CTL_P)	/* Async Memory Bank 0 Control Modes */
#define AMSB0CTL_NONE	(0x0 << AMSB0CTL_P)	/* Control Mode - 00 - No logic */
#define AMSB0CTL_ARE	(0x1 << AMSB0CTL_P)	/* Control Mode - 01 - OR-ed with /ARE */
#define AMSB0CTL_AOE	(0x2 << AMSB0CTL_P)	/* Control Mode - 02 - OR-ed with /AOE */
#define AMSB0CTL_AWE	(0x3 << AMSB0CTL_P)	/* Control Mode - 03 - OR-ed with /AWE */
#define AMSB1CTL_MASK	(0x3 << AMSB1CTL_P)	/* Async Memory Bank 1 Control Modes */
#define AMSB1CTL_NONE	(0x0 << AMSB1CTL_P)	/* Control Mode - 00 - No logic */
#define AMSB1CTL_ARE	(0x1 << AMSB1CTL_P)	/* Control Mode - 01 - OR-ed with /ARE */
#define AMSB1CTL_AOE	(0x2 << AMSB1CTL_P)	/* Control Mode - 02 - OR-ed with /AOE */
#define AMSB1CTL_AWE	(0x3 << AMSB1CTL_P)	/* Control Mode - 03 - OR-ed with /AWE */
#define AMSB2CTL_MASK	(0x3 << AMSB2CTL_P)	/* Async Memory Bank 2 Control Modes */
#define AMSB2CTL_NONE	(0x0 << AMSB2CTL_P)	/* Control Mode - 00 - No logic */
#define AMSB2CTL_ARE	(0x1 << AMSB2CTL_P)	/* Control Mode - 01 - OR-ed with /ARE */
#define AMSB2CTL_AOE	(0x2 << AMSB2CTL_P)	/* Control Mode - 02 - OR-ed with /AOE */
#define AMSB2CTL_AWE	(0x3 << AMSB2CTL_P)	/* Control Mode - 03 - OR-ed with /AWE */
#define AMSB3CTL_MASK	(0x3 << AMSB3CTL_P)	/* Async Memory Bank 3 Control Modes */
#define AMSB3CTL_NONE	(0x0 << AMSB3CTL_P)	/* Control Mode - 00 - No logic */
#define AMSB3CTL_ARE	(0x1 << AMSB3CTL_P)	/* Control Mode - 01 - OR-ed with /ARE */
#define AMSB3CTL_AOE	(0x2 << AMSB3CTL_P)	/* Control Mode - 02 - OR-ed with /AOE */
#define AMSB3CTL_AWE	(0x3 << AMSB3CTL_P)	/* Control Mode - 03 - OR-ed with /AWE */

/* EBIU_MODE Bit Positions */
#define B0MODE_P	0
#define B1MODE_P	2
#define B2MODE_P	4
#define B3MODE_P	6

/* EBIU_MODE Masks */
#define B0MODE_MASK	(0x3 << B0MODE_P)	/* Async Memory Bank 0 Access Mode */
#define B0MODE_ASYNC	(0x0 << B0MODE_P)	/* Access Mode - 00 - Asynchronous Mode */
#define B0MODE_FLASH	(0x1 << B0MODE_P)	/* Access Mode - 01 - Asynchronous Flash Mode */
#define B0MODE_PAGE	(0x2 << B0MODE_P)	/* Access Mode - 10 - Asynchronous Page Mode */
#define B0MODE_BURST	(0x3 << B0MODE_P)	/* Access Mode - 11 - Synchronous (Burst) Mode */
#define B1MODE_MASK	(0x3 << B1MODE_P)	/* Async Memory Bank 1 Access Mode */
#define B1MODE_ASYNC	(0x0 << B1MODE_P)	/* Access Mode - 00 - Asynchronous Mode */
#define B1MODE_FLASH	(0x1 << B1MODE_P)	/* Access Mode - 01 - Asynchronous Flash Mode */
#define B1MODE_PAGE	(0x2 << B1MODE_P)	/* Access Mode - 10 - Asynchronous Page Mode */
#define B1MODE_BURST	(0x3 << B1MODE_P)	/* Access Mode - 11 - Synchronous (Burst) Mode */
#define B2MODE_MASK	(0x3 << B2MODE_P)	/* Async Memory Bank 2 Access Mode */
#define B2MODE_ASYNC	(0x0 << B2MODE_P)	/* Access Mode - 00 - Asynchronous Mode */
#define B2MODE_FLASH	(0x1 << B2MODE_P)	/* Access Mode - 01 - Asynchronous Flash Mode */
#define B2MODE_PAGE	(0x2 << B2MODE_P)	/* Access Mode - 10 - Asynchronous Page Mode */
#define B2MODE_BURST	(0x3 << B2MODE_P)	/* Access Mode - 11 - Synchronous (Burst) Mode */
#define B3MODE_MASK	(0x3 << B3MODE_P)	/* Async Memory Bank 3 Access Mode */
#define B3MODE_ASYNC	(0x0 << B3MODE_P)	/* Access Mode - 00 - Asynchronous Mode */
#define B3MODE_FLASH	(0x1 << B3MODE_P)	/* Access Mode - 01 - Asynchronous Flash Mode */
#define B3MODE_PAGE	(0x2 << B3MODE_P)	/* Access Mode - 10 - Asynchronous Page Mode */
#define B3MODE_BURST	(0x3 << B3MODE_P)	/* Access Mode - 11 - Synchronous (Burst) Mode */

/* EBIU_FCTL Bit Positions */
#define TESTSETLOCK_P	0
#define BCLK_P		1
#define PGWS_P		3
#define PGSZ_P		6
#define RDDL_P		7

/* EBIU_FCTL Masks */
#define TESTSETLOCK	(0x1 << TESTSETLOCK_P)	/* Test set lock */
#define BCLK_MASK	(0x3 << BCLK_P)		/* Burst clock frequency */
#define BCLK_2		(0x1 << BCLK_P)		/* Burst clock frequency - SCLK/2 */
#define BCLK_3		(0x2 << BCLK_P)		/* Burst clock frequency - SCLK/3 */
#define BCLK_4		(0x3 << BCLK_P)		/* Burst clock frequency - SCLK/4 */
#define PGWS_MASK	(0x7 << PGWS_P)		/* Page wait states */
#define PGWS_0		(0x0 << PGWS_P)		/* Page wait states - 0 cycles */
#define PGWS_1		(0x1 << PGWS_P)		/* Page wait states - 1 cycles */
#define PGWS_2		(0x2 << PGWS_P)		/* Page wait states - 2 cycles */
#define PGWS_3		(0x3 << PGWS_P)		/* Page wait states - 3 cycles */
#define PGWS_4		(0x4 << PGWS_P)		/* Page wait states - 4 cycles */
#define PGSZ		(0x1 << PGSZ_P)		/* Page size */
#define PGSZ_4		(0x0 << PGSZ_P)		/* Page size - 4 words */
#define PGSZ_8		(0x1 << PGSZ_P)		/* Page size - 8 words */
#define RDDL		(0x38 << RDDL_P)	/* Read data delay */

/* EBIU_ARBSTAT Masks */
#define ARBSTAT		0x00000001	/* Arbitration status */
#define BGSTAT		0x00000002	/* External Bus grant status */

#endif /* EBIU_MODE */

/* Only available on SDRAM based-parts */
#ifdef EBIU_SDGCTL

/* EBIU_SDGCTL Masks */
#define SCTLE		0x00000001	/* Enable SCLK[0], /SRAS, /SCAS, /SWE, SDQM[3:0] */
#define SCK1E		0x00000002	/* Enable CLKOUT, /SCLK1 */
#define CL_2		0x00000008	/* SDRAM CAS latency = 2 cycles */
#define CL_3		0x0000000C	/* SDRAM CAS latency = 3 cycles */
#define PASR_ALL	0x00000000	/* All 4 SDRAM Banks Refreshed In Self-Refresh */
#define PASR_B0_B1	0x00000010	/* SDRAM Banks 0 and 1 Are Refreshed In Self-Refresh */
#define PASR_B0		0x00000020	/* Only SDRAM Bank 0 Is Refreshed In Self-Refresh */
#define TRAS_1		0x00000040	/* SDRAM tRAS = 1 cycle */
#define TRAS_2		0x00000080	/* SDRAM tRAS = 2 cycles */
#define TRAS_3		0x000000C0	/* SDRAM tRAS = 3 cycles */
#define TRAS_4		0x00000100	/* SDRAM tRAS = 4 cycles */
#define TRAS_5		0x00000140	/* SDRAM tRAS = 5 cycles */
#define TRAS_6		0x00000180	/* SDRAM tRAS = 6 cycles */
#define TRAS_7		0x000001C0	/* SDRAM tRAS = 7 cycles */
#define TRAS_8		0x00000200	/* SDRAM tRAS = 8 cycles */
#define TRAS_9		0x00000240	/* SDRAM tRAS = 9 cycles */
#define TRAS_10		0x00000280	/* SDRAM tRAS = 10 cycles */
#define TRAS_11		0x000002C0	/* SDRAM tRAS = 11 cycles */
#define TRAS_12		0x00000300	/* SDRAM tRAS = 12 cycles */
#define TRAS_13		0x00000340	/* SDRAM tRAS = 13 cycles */
#define TRAS_14		0x00000380	/* SDRAM tRAS = 14 cycles */
#define TRAS_15		0x000003C0	/* SDRAM tRAS = 15 cycles */
#define TRP_1		0x00000800	/* SDRAM tRP = 1 cycle */
#define TRP_2		0x00001000	/* SDRAM tRP = 2 cycles */
#define TRP_3		0x00001800	/* SDRAM tRP = 3 cycles */
#define TRP_4		0x00002000	/* SDRAM tRP = 4 cycles */
#define TRP_5		0x00002800	/* SDRAM tRP = 5 cycles */
#define TRP_6		0x00003000	/* SDRAM tRP = 6 cycles */
#define TRP_7		0x00003800	/* SDRAM tRP = 7 cycles */
#define TRCD_1		0x00008000	/* SDRAM tRCD = 1 cycle */
#define TRCD_2		0x00010000	/* SDRAM tRCD = 2 cycles */
#define TRCD_3		0x00018000	/* SDRAM tRCD = 3 cycles */
#define TRCD_4		0x00020000	/* SDRAM tRCD = 4 cycles */
#define TRCD_5		0x00028000	/* SDRAM tRCD = 5 cycles */
#define TRCD_6		0x00030000	/* SDRAM tRCD = 6 cycles */
#define TRCD_7		0x00038000	/* SDRAM tRCD = 7 cycles */
#define TWR_1		0x00080000	/* SDRAM tWR = 1 cycle */
#define TWR_2		0x00100000	/* SDRAM tWR = 2 cycles */
#define TWR_3		0x00180000	/* SDRAM tWR = 3 cycles */
#define PUPSD		0x00200000	/* Power-up start delay */
#define PSM		0x00400000	/* SDRAM power-up sequence = Precharge, mode register set, 8 CBR refresh cycles */
#define PSS		0x00800000	/* enable SDRAM power-up sequence on next SDRAM access */
#define SRFS		0x01000000	/* Start SDRAM self-refresh mode */
#define EBUFE		0x02000000	/* Enable external buffering timing */
#define FBBRW		0x04000000	/* Fast back-to-back read write enable */
#define EMREN		0x10000000	/* Extended mode register enable */
#define TCSR		0x20000000	/* Temp compensated self refresh value 85 deg C */
#define CDDBG		0x40000000	/* Tristate SDRAM controls during bus grant */

/* EBIU_SDBCTL Masks */
#define EBE		0x0001		/* Enable SDRAM External Bank */
#define EBSZ_16		0x0000		/* SDRAM External Bank Size = 16MB */
#define EBSZ_32		0x0002		/* SDRAM External Bank Size = 32MB */
#define EBSZ_64		0x0004		/* SDRAM External Bank Size = 64MB */
#define EBSZ_128	0x0006		/* SDRAM External Bank Size = 128MB */
#define EBSZ_256	0x0007		/* SDRAM External Bank Size = 256MB */
#define EBSZ_512	0x0008		/* SDRAM External Bank Size = 512MB */
#define EBCAW_8		0x0000		/* SDRAM External Bank Column Address Width = 8 Bits */
#define EBCAW_9		0x0010		/* SDRAM External Bank Column Address Width = 9 Bits */
#define EBCAW_10	0x0020		/* SDRAM External Bank Column Address Width = 10 Bits */
#define EBCAW_11	0x0030		/* SDRAM External Bank Column Address Width = 11 Bits */

#ifdef __ADSPBF561__

#define EB0E		(EBE<<0)	/* Enable SDRAM external bank 0 */
#define EB0SZ_16	(EBSZ_16<<0)	/* SDRAM external bank size = 16MB */
#define EB0SZ_32	(EBSZ_32<<0)	/* SDRAM external bank size = 32MB */
#define EB0SZ_64	(EBSZ_64<<0)	/* SDRAM external bank size = 64MB */
#define EB0SZ_128	(EBSZ_128<<0)	/* SDRAM external bank size = 128MB */
#define EB0CAW_8	(EBCAW_8<<0)	/* SDRAM external bank column address width = 8 bits */
#define EB0CAW_9	(EBCAW_9<<0)	/* SDRAM external bank column address width = 9 bits */
#define EB0CAW_10	(EBCAW_10<<0)	/* SDRAM external bank column address width = 9 bits */
#define EB0CAW_11	(EBCAW_11<<0)	/* SDRAM external bank column address width = 9 bits */

#define EB1E		(EBE<<8)	/* Enable SDRAM external bank 0 */
#define EB1SZ_16	(EBSZ_16<<8)	/* SDRAM external bank size = 16MB */
#define EB1SZ_32	(EBSZ_32<<8)	/* SDRAM external bank size = 32MB */
#define EB1SZ_64	(EBSZ_64<<8)	/* SDRAM external bank size = 64MB */
#define EB1SZ_128	(EBSZ_128<<8)	/* SDRAM external bank size = 128MB */
#define EB1CAW_8	(EBCAW_8<<8)	/* SDRAM external bank column address width = 8 bits */
#define EB1CAW_9	(EBCAW_9<<8)	/* SDRAM external bank column address width = 9 bits */
#define EB1CAW_10	(EBCAW_10<<8)	/* SDRAM external bank column address width = 9 bits */
#define EB1CAW_11	(EBCAW_11<<8)	/* SDRAM external bank column address width = 9 bits */

#define EB2E		(EBE<<16)	/* Enable SDRAM external bank 0 */
#define EB2SZ_16	(EBSZ_16<<16)	/* SDRAM external bank size = 16MB */
#define EB2SZ_32	(EBSZ_32<<16)	/* SDRAM external bank size = 32MB */
#define EB2SZ_64	(EBSZ_64<<16)	/* SDRAM external bank size = 64MB */
#define EB2SZ_128	(EBSZ_128<<16)	/* SDRAM external bank size = 128MB */
#define EB2CAW_8	(EBCAW_8<<16)	/* SDRAM external bank column address width = 8 bits */
#define EB2CAW_9	(EBCAW_9<<16)	/* SDRAM external bank column address width = 9 bits */
#define EB2CAW_10	(EBCAW_10<<16)	/* SDRAM external bank column address width = 9 bits */
#define EB2CAW_11	(EBCAW_11<<16)	/* SDRAM external bank column address width = 9 bits */

#define EB3E		(EBE<<24)	/* Enable SDRAM external bank 0 */
#define EB3SZ_16	(EBSZ_16<<24)	/* SDRAM external bank size = 16MB */
#define EB3SZ_32	(EBSZ_32<<24)	/* SDRAM external bank size = 32MB */
#define EB3SZ_64	(EBSZ_64<<24)	/* SDRAM external bank size = 64MB */
#define EB3SZ_128	(EBSZ_128<<24)	/* SDRAM external bank size = 128MB */
#define EB3CAW_8	(EBCAW_8<<24)	/* SDRAM external bank column address width = 8 bits */
#define EB3CAW_9	(EBCAW_9<<24)	/* SDRAM external bank column address width = 9 bits */
#define EB3CAW_10	(EBCAW_10<<24)	/* SDRAM external bank column address width = 9 bits */
#define EB3CAW_11	(EBCAW_11<<24)	/* SDRAM external bank column address width = 9 bits */

#endif /* BF561 */

/* EBIU_SDSTAT Masks */
#define SDCI		0x0001		/* SDRAM controller is idle */
#define SDSRA		0x0002		/* SDRAM SDRAM self refresh is active */
#define SDPUA		0x0004		/* SDRAM power up active */
#define SDRS		0x0008		/* SDRAM is in reset state */
#define SDEASE		0x0010		/* SDRAM EAB sticky error status - W1C */
#define BGSTAT		0x0020		/* Bus granted */

#endif /* EBIU_SDGCTL */

#endif
