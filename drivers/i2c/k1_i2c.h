/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023-2026 Spacemit Ltd.
 * Copyright (C) 2025-2026 RISCstar Ltd.
 */

#ifndef __SPACEMIT_I2C_H
#define __SPACEMIT_I2C_H

/* Shall the current transfer have a start/stop condition? */
#define I2C_COND_NORMAL		0
#define I2C_COND_START		1
#define I2C_COND_STOP		2

/* Shall the current transfer be ack/nacked or being waited for it? */
#define I2C_ACKNAK_WAITACK	1
#define I2C_ACKNAK_SENDACK	2
#define I2C_ACKNAK_SENDNAK	4

/* Specify who shall transfer the data (master or slave) */
#define I2C_READ		0
#define I2C_WRITE		1

#if (CONFIG_SYS_I2C_SPEED == 400000)
#define I2C_ICR_INIT	(ICR_FM | ICR_BEIE | ICR_IRFIE | ICR_ITEIE |	\
			 ICR_GCD | ICR_SCLE)
#else
#define I2C_ICR_INIT	(ICR_BEIE | ICR_IRFIE | ICR_ITEIE | ICR_GCD |	\
			 ICR_SCLE)
#endif

/* ----- Control register bits ---------------------------------------- */

#define ICR_START	0x1		/* start bit */
#define ICR_STOP	0x2		/* stop bit */
#define ICR_ACKNAK	0x4		/* send ACK(0) or NAK(1) */
#define ICR_TB		0x8		/* transfer byte bit */
#define ICR_MA		BIT(12)		/* master abort */
#define ICR_SCLE	BIT(13)		/* master clock enable, mona SCLEA */
#define ICR_IUE		BIT(14)		/* unit enable */
#define ICR_GCD		BIT(21)		/* general call disable */
#define ICR_ITEIE	BIT(19)		/* enable tx interrupts */
#define ICR_IRFIE	BIT(20)		/* enable rx interrupts, mona: DRFIE */
#define ICR_BEIE	BIT(22)		/* enable bus error ints */
#define ICR_SSDIE	BIT(24)		/* slave STOP detected int enable */
#define ICR_ALDIE	BIT(18)		/* enable arbitration interrupt */
#define ICR_SADIE	BIT(23)		/* slave address detected int enable */
#define ICR_UR		BIT(10)		/* unit reset */
#define ICR_SM		(0x0)		/* Standard Mode */
#define ICR_FM		BIT(8)		/* Fast Mode */
#define ICR_MODE_MASK	(0x300)		/* Mode mask */

/* ----- Status register bits ----------------------------------------- */

#define ISR_RWM		BIT(13)		/* read/write mode */
#define ISR_ACKNAK	BIT(14)		/* ack/nak status */
#define ISR_UB		BIT(15)		/* unit busy */
#define ISR_IBB		BIT(16)		/* bus busy */
#define ISR_SSD		BIT(24)		/* slave stop detected */
#define ISR_ALD		BIT(18)		/* arbitration loss detected */
#define ISR_ITE		BIT(19)		/* tx buffer empty */
#define ISR_IRF		BIT(20)		/* rx buffer full */
#define ISR_GCAD	BIT(21)		/* general call address detected */
#define ISR_SAD		BIT(23)		/* slave address detected */
#define ISR_BED		BIT(22)		/* bus error no ACK/NAK */

#define I2C_ISR_INIT	0x1FDE000

#endif
