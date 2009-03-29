#ifndef __HW_S3C4510_H
#define __HW_S3C4510_H

/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 *
 * Description:   Samsung S3C4510B register layout
 */

/*------------------------------------------------------------------------
 *	  ASIC Address Definition
 *----------------------------------------------------------------------*/

/* L1 8KB on chip SRAM base address */
#define SRAM_BASE       (0x03fe0000)

/* Special Register Start Address After System Reset */
#define REG_BASE	(0x03ff0000)
#define SPSTR		(REG_BASE)

/* *********************** */
/* System Manager Register */
/* *********************** */
#define REG_SYSCFG	(REG_BASE+0x0000)

#define REG_CLKCON      (REG_BASE+0x3000)
#define REG_EXTACON0	(REG_BASE+0x3008)
#define REG_EXTACON1	(REG_BASE+0x300c)
#define REG_EXTDBWTH	(REG_BASE+0x3010)
#define REG_ROMCON0	(REG_BASE+0x3014)
#define REG_ROMCON1	(REG_BASE+0x3018)
#define REG_ROMCON2	(REG_BASE+0x301c)
#define REG_ROMCON3	(REG_BASE+0x3020)
#define REG_ROMCON4	(REG_BASE+0x3024)
#define REG_ROMCON5	(REG_BASE+0x3028)
#define REG_DRAMCON0	(REG_BASE+0x302c)
#define REG_DRAMCON1	(REG_BASE+0x3030)
#define REG_DRAMCON2	(REG_BASE+0x3034)
#define REG_DRAMCON3	(REG_BASE+0x3038)
#define REG_REFEXTCON	(REG_BASE+0x303c)

/* *********************** */
/* Ethernet BDMA Register  */
/* *********************** */
#define REG_BDMATXCON	(REG_BASE+0x9000)
#define REG_BDMARXCON	(REG_BASE+0x9004)
#define REG_BDMATXPTR	(REG_BASE+0x9008)
#define REG_BDMARXPTR	(REG_BASE+0x900c)
#define REG_BDMARXLSZ	(REG_BASE+0x9010)
#define REG_BDMASTAT	(REG_BASE+0x9014)

/* Content Address Memory */
#define REG_CAM_BASE	(REG_BASE+0x9100)

#define REG_BDMATXBUF	(REG_BASE+0x9200)
#define REG_BDMARXBUF	(REG_BASE+0x9800)

/* *********************** */
/* Ethernet MAC Register   */
/* *********************** */
#define REG_MACCON	(REG_BASE+0xa000)
#define REG_CAMCON	(REG_BASE+0xa004)
#define REG_MACTXCON	(REG_BASE+0xa008)
#define REG_MACTXSTAT	(REG_BASE+0xa00c)
#define REG_MACRXCON	(REG_BASE+0xa010)
#define REG_MACRXSTAT	(REG_BASE+0xa014)
#define REG_STADATA	(REG_BASE+0xa018)
#define REG_STACON	(REG_BASE+0xa01c)
#define REG_CAMEN	(REG_BASE+0xa028)
#define REG_EMISSCNT	(REG_BASE+0xa03c)
#define REG_EPZCNT	(REG_BASE+0xa040)
#define REG_ERMPZCNT	(REG_BASE+0xa044)
#define REG_ETXSTAT	(REG_BASE+0x9040)
#define REG_MACRXDESTR	(REG_BASE+0xa064)
#define REG_MACRXSTATEM	(REG_BASE+0xa090)
#define REG_MACRXFIFO	(REG_BASE+0xa200)

/********************/
/* I2C Bus Register */
/********************/
#define REG_I2C_CON	(REG_BASE+0xf000)
#define REG_I2C_BUF	(REG_BASE+0xf004)
#define REG_I2C_PS	(REG_BASE+0xf008)
#define REG_I2C_COUNT	(REG_BASE+0xf00c)

/********************/
/*    GDMA 0        */
/********************/
#define REG_GDMACON0	(REG_BASE+0xb000)
#define REG_GDMA0_RUN_ENABLE (REG_BASE+0xb020)
#define REG_GDMASRC0	(REG_BASE+0xb004)
#define REG_GDMADST0	(REG_BASE+0xb008)
#define REG_GDMACNT0	(REG_BASE+0xb00c)

/********************/
/*    GDMA 1        */
/********************/
#define REG_GDMACON1	(REG_BASE+0xc000)
#define REG_GDMA1_RUN_ENABLE (REG_BASE+0xc020)
#define REG_GDMASRC1	(REG_BASE+0xc004)
#define REG_GDMADST1	(REG_BASE+0xc008)
#define REG_GDMACNT1	(REG_BASE+0xc00c)

/********************/
/*      UART 0      */
/********************/
#define UART0_BASE       (REG_BASE+0xd000)
#define REG_UART0_LCON   (REG_BASE+0xd000)
#define REG_UART0_CTRL   (REG_BASE+0xd004)
#define REG_UART0_STAT   (REG_BASE+0xd008)
#define REG_UART0_TXB    (REG_BASE+0xd00c)
#define REG_UART0_RXB    (REG_BASE+0xd010)
#define REG_UART0_BAUD_DIV    (REG_BASE+0xd014)
#define REG_UART0_BAUD_CNT    (REG_BASE+0xd018)
#define REG_UART0_BAUD_CLK    (REG_BASE+0xd01C)

/********************/
/*     UART 1       */
/********************/
#define UART1_BASE       (REG_BASE+0xe000)
#define REG_UART1_LCON   (REG_BASE+0xe000)
#define REG_UART1_CTRL   (REG_BASE+0xe004)
#define REG_UART1_STAT   (REG_BASE+0xe008)
#define REG_UART1_TXB    (REG_BASE+0xe00c)
#define REG_UART1_RXB    (REG_BASE+0xe010)
#define REG_UART1_BAUD_DIV    (REG_BASE+0xe014)
#define REG_UART1_BAUD_CNT    (REG_BASE+0xe018)
#define REG_UART1_BAUD_CLK    (REG_BASE+0xe01C)

/********************/
/*  Timer Register  */
/********************/
#define REG_TMOD	(REG_BASE+0x6000)
#define REG_TDATA0	(REG_BASE+0x6004)
#define REG_TDATA1	(REG_BASE+0x6008)
#define REG_TCNT0	(REG_BASE+0x600c)
#define REG_TCNT1	(REG_BASE+0x6010)

/**********************/
/* I/O Port Interface */
/**********************/
#define REG_IOPMODE	(REG_BASE+0x5000)
#define REG_IOPCON	(REG_BASE+0x5004)
#define REG_IOPDATA	(REG_BASE+0x5008)

/*********************************/
/* Interrupt Controller Register */
/*********************************/
#define REG_INTMODE     (REG_BASE+0x4000)
#define REG_INTPEND     (REG_BASE+0x4004)
#define REG_INTMASK     (REG_BASE+0x4008)

#define REG_INTPRI0     (REG_BASE+0x400c)
#define REG_INTPRI1	(REG_BASE+0x4010)
#define REG_INTPRI2	(REG_BASE+0x4014)
#define REG_INTPRI3	(REG_BASE+0x4018)
#define REG_INTPRI4	(REG_BASE+0x401c)
#define REG_INTPRI5	(REG_BASE+0x4020)
#define REG_INTOFFSET	(REG_BASE+0x4024)
#define REG_INTPNDPRI	(REG_BASE+0x4028)
#define REG_INTPNDTST	(REG_BASE+0x402C)

/*********************************/
/* CACHE CONTROL MASKS           */
/*********************************/
#define CACHE_STALL      (0x00000001)
#define CACHE_ENABLE     (0x00000002)
#define CACHE_WRITE_BUFF (0x00000004)
#define CACHE_MODE       (0x00000030)
#define CACHE_MODE_00    (0x00000000)
#define CACHE_MODE_01    (0x00000010)
#define CACHE_MODE_10    (0x00000020)

/*********************************/
/* CACHE RAM BASE ADDRESSES      */
/*********************************/
#define CACHE_SET0_RAM   (0x10000000)
#define CACHE_SET1_RAM   (0x10800000)
#define CACHE_TAG_RAM    (0x11000000)

/*********************************/
/* CACHE_DISABLE MASK            */
/*********************************/
#define CACHE_DISABLE_MASK (0x04000000)

#define GET_REG(reg)       (*((volatile u32 *)(reg)))
#define PUT_REG(reg, val)  (*((volatile u32 *)(reg)) = ((u32)(val)))
#define SET_REG(reg, mask) (PUT_REG((reg), GET_REG((reg)) |  mask))
#define CLR_REG(reg, mask) (PUT_REG((reg), GET_REG((reg)) & ~mask))
#define PUT_U16(reg, val)  (*((volatile u16 *)(reg)) = ((u16)(val)))
#define PUT__U8(reg, val)  (*((volatile u8  *)(reg)) = (( u8)((val)&0xFF)))
#define GET__U8(reg)       (*((volatile u8  *)(reg)))

#define PUT_LED(val)       (PUT_REG(REG_IOPDATA, (~val)&0xFF))
#define GET_LED()          ((~GET_REG( REG_IOPDATA)) & 0xFF)
#define SET_LED(val)       { u32 led = GET_LED(); led |= 1 << (val);  PUT_LED( led); }
#define CLR_LED(val)       { u32 led = GET_LED(); led &= ~(1 << (val));  PUT_LED( led); }

/***********************************/
/* CLOCK CONSTANTS -- 50 MHz Clock */
/***********************************/

#define CLK_FREQ_MHZ       (50)
#define t_data_us(t)       ((t)*CLK_FREQ_MHZ-1)   /* t is time tick,unit[us] */
#define t_data_ms(t)       (t_data_us((t)*1000))  /* t is time tick,unit[ms] */

/*********************************************************/
/*	       TIMER MODE REGISTER                       */
/*********************************************************/
#define  TM0_RUN      0x01  /* Timer 0 enable */
#define  TM0_TOGGLE   0x02  /* 0, interval mode */
#define  TM0_OUT_1    0x04  /* Timer 0 Initial TOUT0 value */
#define  TM1_RUN      0x08  /* Timer 1 enable */
#define  TM1_TOGGLE   0x10  /* 0, interval mode */
#define  TM1_OUT_1    0x20  /* Timer 0 Initial TOUT0 value */


/*********************************/
/* INTERRUPT SOURCES             */
/*********************************/
#define INT_EXTINT0	0
#define INT_EXTINT1	1
#define INT_EXTINT2	2
#define INT_EXTINT3	3
#define INT_UARTTX0	4
#define INT_UARTRX0	5
#define INT_UARTTX1	6
#define INT_UARTRX1	7
#define INT_GDMA0	8
#define INT_GDMA1	9
#define INT_TIMER0	10
#define INT_TIMER1	11
#define INT_HDLCTXA	12
#define INT_HDLCRXA	13
#define INT_HDLCTXB	14
#define INT_HDLCRXB	15
#define INT_BDMATX	16
#define INT_BDMARX	17
#define INT_MACTX	18
#define INT_MACRX	19
#define INT_IIC		20
#define INT_GLOBAL	21
#define N_IRQS         (21)

#ifndef __ASSEMBLER__
struct _irq_handler {
	void                *m_data;
	void (*m_func)( void *data);
};

#endif

#endif /* __S3C4510_h */
