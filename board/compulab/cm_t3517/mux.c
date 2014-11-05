/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mux.h>
#include <asm/io.h>

void set_muxconf_regs(void)
{
	/* SDRC */
	MUX_VAL(CP(SDRC_D0),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D1),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D2),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D3),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D4),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D5),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D6),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D7),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D8),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D9),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D10),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D11),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D12),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D13),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D14),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D15),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D16),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D17),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D18),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D19),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D20),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D21),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D22),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D23),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D24),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D25),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D26),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D27),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D28),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D29),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D30),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_D31),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_CLK),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_DQS0),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_DQS1),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_DQS2),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_DQS3),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_CKE0),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(SDRC_CKE1),		(IDIS | PTD | DIS | M7));

	/* GPMC */
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0));

	/* SB-T35 SD/MMC WP GPIO59 */
	MUX_VAL(CP(GPMC_CLK),		(IEN  | PTU | EN  | M4)); /*GPIO_59*/
	MUX_VAL(CP(GPMC_NWE),		(IDIS | PTD | DIS | M0));
	MUX_VAL(CP(GPMC_NOE),		(IDIS | PTD | DIS | M0));
	MUX_VAL(CP(GPMC_NADV_ALE),	(IDIS | PTD | DIS | M0));
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0));
	/* SB-T35 Audio Enable GPIO61 */
	MUX_VAL(CP(GPMC_NBE1),		(IDIS | PTU | EN  | M4)); /*GPIO_61*/
	MUX_VAL(CP(GPMC_NWP),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(GPMC_WAIT0),		(IEN  | PTU | EN  | M0));

	/* UART3 Console */
	MUX_VAL(CP(UART3_RX_IRRX),	(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(UART3_TX_IRTX),	(IDIS | PTD | DIS | M0));
	/* RTC V3020 nCS GPIO163 */
	MUX_VAL(CP(UART3_CTS_RCTX),	(IEN  | PTU | EN  | M4)); /*GPIO_163*/

	/* SB-T35 SD/MMC CD GPIO144 */
	MUX_VAL(CP(UART2_CTS),		(IEN  | PTU | EN  | M4)); /*GPIO_144*/
	/* WIFI nRESET GPIO145 */
	MUX_VAL(CP(UART2_RTS),		(IEN  | PTD | EN  | M4)); /*GPIO_145*/

	/* MMC1 */
	MUX_VAL(CP(MMC1_CLK),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | EN  | M0));

	/* I2C */
	MUX_VAL(CP(I2C1_SCL),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(I2C1_SDA),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTU | EN  | M0));

	/* Green LED GPIO186 */
	MUX_VAL(CP(SYS_CLKOUT2),	(IDIS | PTU | DIS | M4)); /*GPIO_186*/

	/* RTC V3020 CS Enable GPIO160 */
	MUX_VAL(CP(MCBSP_CLKS),		(IEN  | PTD | EN  | M4)); /*GPIO_160*/

	/* SYS_BOOT */
	MUX_VAL(CP(SYS_BOOT0),		(IEN  | PTU | DIS | M4)); /*GPIO_2*/
	MUX_VAL(CP(SYS_BOOT1),		(IEN  | PTU | DIS | M4)); /*GPIO_3*/
	MUX_VAL(CP(SYS_BOOT2),		(IEN  | PTU | DIS | M4)); /*GPIO_4*/
	MUX_VAL(CP(SYS_BOOT3),		(IEN  | PTU | DIS | M4)); /*GPIO_5*/
	MUX_VAL(CP(SYS_BOOT4),		(IEN  | PTU | DIS | M4)); /*GPIO_6*/
	MUX_VAL(CP(SYS_BOOT5),		(IEN  | PTU | DIS | M4)); /*GPIO_7*/
}
