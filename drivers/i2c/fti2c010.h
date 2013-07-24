/*
 * Faraday I2C Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __FTI2C010_H
#define __FTI2C010_H

/*
 * FTI2C010 registers
 */
struct fti2c010_regs {
	uint32_t cr;  /* 0x00: control register */
	uint32_t sr;  /* 0x04: status register */
	uint32_t cdr; /* 0x08: clock division register */
	uint32_t dr;  /* 0x0c: data register */
	uint32_t sar; /* 0x10: slave address register */
	uint32_t tgsr;/* 0x14: time & glitch suppression register */
	uint32_t bmr; /* 0x18: bus monitor register */
	uint32_t rsvd[5];
	uint32_t revr;/* 0x30: revision register */
};

/*
 * control register
 */
#define CR_ALIRQ      0x2000  /* arbitration lost interrupt (master) */
#define CR_SAMIRQ     0x1000  /* slave address match interrupt (slave) */
#define CR_STOPIRQ    0x800   /* stop condition interrupt (slave) */
#define CR_NAKRIRQ    0x400   /* NACK response interrupt (master) */
#define CR_DRIRQ      0x200   /* rx interrupt (both) */
#define CR_DTIRQ      0x100   /* tx interrupt (both) */
#define CR_TBEN       0x80    /* tx enable (both) */
#define CR_NAK        0x40    /* NACK (both) */
#define CR_STOP       0x20    /* stop (master) */
#define CR_START      0x10    /* start (master) */
#define CR_GCEN       0x8     /* general call support (slave) */
#define CR_SCLEN      0x4     /* enable clock out (master) */
#define CR_I2CEN      0x2     /* enable I2C (both) */
#define CR_I2CRST     0x1     /* reset I2C (both) */
#define CR_ENABLE     \
	(CR_ALIRQ | CR_NAKRIRQ | CR_DRIRQ | CR_DTIRQ | CR_SCLEN | CR_I2CEN)

/*
 * status register
 */
#define SR_CLRAL      0x400    /* clear arbitration lost */
#define SR_CLRGC      0x200    /* clear general call */
#define SR_CLRSAM     0x100    /* clear slave address match */
#define SR_CLRSTOP    0x80     /* clear stop */
#define SR_CLRNAKR    0x40     /* clear NACK respond */
#define SR_DR         0x20     /* rx ready */
#define SR_DT         0x10     /* tx done */
#define SR_BB         0x8      /* bus busy */
#define SR_BUSY       0x4      /* chip busy */
#define SR_ACK        0x2      /* ACK/NACK received */
#define SR_RW         0x1      /* set when master-rx or slave-tx mode */

/*
 * clock division register
 */
#define CDR_DIV(n)    ((n) & 0x3ffff)

/*
 * time & glitch suppression register
 */
#define TGSR_GSR(n)   (((n) & 0x7) << 10)
#define TGSR_TSR(n)   ((n) & 0x3ff)

/*
 * bus monitor register
 */
#define BMR_SCL       0x2      /* SCL is pull-up */
#define BMR_SDA       0x1      /* SDA is pull-up */

#endif /* __FTI2C010_H */
