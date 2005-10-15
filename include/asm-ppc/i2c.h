/*
 * Freescale I2C Controller
 *
 * This software may be used and distributed according to the
 * terms of the GNU Public License, Version 2, incorporated
 * herein by reference.
 *
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2003, Motorola, Inc.
 * author: Eran Liberty (liberty@freescale.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _ASM_I2C_H_
#define _ASM_I2C_H_

#include <asm/types.h>

typedef struct i2c
{
    u8 adr;          /**< I2C slave address              */
#define I2C_ADR	      0xFE
#define I2C_ADR_SHIFT 1
#define I2C_ADR_RES   ~(I2C_ADR)
    u8 res0[3];
    u8 fdr;          /**< I2C frequency divider register */
#define IC2_FDR       0x3F
#define IC2_FDR_SHIFT 0
#define IC2_FDR_RES   ~(IC2_FDR)
    u8 res1[3];
    u8 cr;           /**< I2C control redister           */
#define I2C_CR_MEN	  0x80
#define I2C_CR_MIEN	  0x40
#define I2C_CR_MSTA   0x20
#define I2C_CR_MTX    0x10
#define I2C_CR_TXAK   0x08
#define I2C_CR_RSTA   0x04
#define I2C_CR_BCST   0x01
    u8 res2[3];
    u8 sr;           /**< I2C status register            */
#define I2C_SR_MCF    0x80
#define I2C_SR_MAAS   0x40
#define I2C_SR_MBB    0x20
#define I2C_SR_MAL    0x10
#define I2C_SR_BCSTM  0x08
#define I2C_SR_SRW    0x04
#define I2C_SR_MIF    0x02
#define I2C_SR_RXAK   0x01
    u8 res3[3];
    u8 dr;           /**< I2C data register              */
#define I2C_DR 0xFF
#define I2C_DR_SHIFT 0
#define I2C_DR_RES ~(I2C_DR)
    u8 res4[3];
    u8 dfsrr;        /**< I2C digital filter sampling rate register */
#define I2C_DFSRR 0x3F
#define I2C_DFSRR_SHIFT 0
#define I2C_DFSRR_RES ~(I2C_DR)
    u8 res5[3];
    u8 res6[0xE8];
} i2c_t;

#ifndef CFG_HZ
#error CFG_HZ is not defined in /include/configs/${BOARD}.h
#endif
#define I2C_TIMEOUT (CFG_HZ/4)

#ifndef CFG_IMMRBAR
#error CFG_IMMRBAR is not defined in /include/configs/${BOARD}.h
#endif

#ifndef CFG_I2C_OFFSET
#error CFG_I2C_OFFSET is not defined in /include/configs/${BOARD}.h
#endif

#if defined(CONFIG_MPC8349ADS) || defined(CONFIG_TQM834X)
/*
 * MPC8349 have two i2c bus
 */
extern i2c_t * mpc8349_i2c;
#define I2C mpc8349_i2c
#else
#define I2C ((i2c_t*)(CFG_IMMRBAR + CFG_I2C_OFFSET))
#endif

#define I2C_READ  1
#define I2C_WRITE 0

#endif	/* _ASM_I2C_H_ */
