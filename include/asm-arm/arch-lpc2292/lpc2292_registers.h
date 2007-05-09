#ifndef __LPC2292_REGISTERS_H
#define __LPC2292_REGISTERS_H

#include <config.h>

/* Macros for reading/writing registers */
#define PUT8(reg, value) (*(volatile unsigned char*)(reg) = (value))
#define PUT16(reg, value) (*(volatile unsigned short*)(reg) = (value))
#define PUT32(reg, value) (*(volatile unsigned int*)(reg) = (value))
#define GET8(reg) (*(volatile unsigned char*)(reg))
#define GET16(reg) (*(volatile unsigned short*)(reg))
#define GET32(reg) (*(volatile unsigned int*)(reg))

/* External Memory Controller */

#define BCFG0 0xFFE00000	/* 32-bits */
#define BCFG1 0xFFE00004	/* 32-bits */
#define BCFG2 0xFFE00008	/* 32-bits */
#define BCFG3 0xFFE0000c	/* 32-bits */

/* System Control Block */

#define EXTINT   0xE01FC140
#define EXTWAKE  0xE01FC144
#define EXTMODE  0xE01FC148
#define EXTPOLAR 0xE01FC14C
#define MEMMAP   0xE01FC040
#define PLLCON   0xE01FC080
#define PLLCFG   0xE01FC084
#define PLLSTAT  0xE01FC088
#define PLLFEED  0xE01FC08C
#define PCON     0xE01FC0C0
#define PCONP    0xE01FC0C4
#define VPBDIV   0xE01FC100

/* Memory Acceleration Module */

#define MAMCR  0xE01FC000
#define MAMTIM 0xE01FC004

/* Vectored Interrupt Controller */

#define VICIRQStatus    0xFFFFF000
#define VICFIQStatus    0xFFFFF004
#define VICRawIntr      0xFFFFF008
#define VICIntSelect    0xFFFFF00C
#define VICIntEnable    0xFFFFF010
#define VICIntEnClr     0xFFFFF014
#define VICSoftInt      0xFFFFF018
#define VICSoftIntClear 0xFFFFF01C
#define VICProtection   0xFFFFF020
#define VICVectAddr     0xFFFFF030
#define VICDefVectAddr  0xFFFFF034
#define VICVectAddr0    0xFFFFF100
#define VICVectAddr1    0xFFFFF104
#define VICVectAddr2    0xFFFFF108
#define VICVectAddr3    0xFFFFF10C
#define VICVectAddr4    0xFFFFF110
#define VICVectAddr5    0xFFFFF114
#define VICVectAddr6    0xFFFFF118
#define VICVectAddr7    0xFFFFF11C
#define VICVectAddr8    0xFFFFF120
#define VICVectAddr9    0xFFFFF124
#define VICVectAddr10   0xFFFFF128
#define VICVectAddr11   0xFFFFF12C
#define VICVectAddr12   0xFFFFF130
#define VICVectAddr13   0xFFFFF134
#define VICVectAddr14   0xFFFFF138
#define VICVectAddr15   0xFFFFF13C
#define VICVectCntl0    0xFFFFF200
#define VICVectCntl1	0xFFFFF204
#define VICVectCntl2	0xFFFFF208
#define VICVectCntl3	0xFFFFF20C
#define VICVectCntl4	0xFFFFF210
#define VICVectCntl5	0xFFFFF214
#define VICVectCntl6	0xFFFFF218
#define VICVectCntl7	0xFFFFF21C
#define VICVectCntl8	0xFFFFF220
#define VICVectCntl9	0xFFFFF224
#define VICVectCntl10	0xFFFFF228
#define VICVectCntl11	0xFFFFF22C
#define VICVectCntl12	0xFFFFF230
#define VICVectCntl13	0xFFFFF234
#define VICVectCntl14	0xFFFFF238
#define VICVectCntl15	0xFFFFF23C

/* Pin connect block */

#define PINSEL0 0xE002C000	/* 32 bits */
#define PINSEL1 0xE002C004	/* 32 bits */
#define PINSEL2 0xE002C014	/* 32 bits */

/* GPIO */

#define IO0PIN 0xE0028000
#define IO0SET 0xE0028004
#define IO0DIR 0xE0028008
#define IO0CLR 0xE002800C
#define IO1PIN 0xE0028010
#define IO1SET 0xE0028014
#define IO1DIR 0xE0028018
#define IO1CLR 0xE002801C
#define IO2PIN 0xE0028020
#define IO2SET 0xE0028024
#define IO2DIR 0xE0028028
#define IO2CLR 0xE002802C
#define IO3PIN 0xE0028030
#define IO3SET 0xE0028034
#define IO3DIR 0xE0028038
#define IO3CLR 0xE002803C

/* Uarts */

#define U0RBR 0xE000C000
#define U0THR 0xE000C000
#define U0IER 0xE000C004
#define U0IIR 0xE000C008
#define U0FCR 0xE000C008
#define U0LCR 0xE000C00C
#define U0LSR 0xE000C014
#define U0SCR 0xE000C01C
#define U0DLL 0xE000C000
#define U0DLM 0xE000C004

#define U1RBR 0xE0010000
#define U1THR 0xE0010000
#define U1IER 0xE0010004
#define U1IIR 0xE0010008
#define U1FCR 0xE0010008
#define U1LCR 0xE001000C
#define U1MCR 0xE0010010
#define U1LSR 0xE0010014
#define U1MSR 0xE0010018
#define U1SCR 0xE001001C
#define U1DLL 0xE0010000
#define U1DLM 0xE0010004

/* I2C */

#define I2CONSET 0xE001C000
#define I2STAT   0xE001C004
#define I2DAT    0xE001C008
#define I2ADR    0xE001C00C
#define I2SCLH   0xE001C010
#define I2SCLL   0xE001C014
#define I2CONCLR 0xE001C018

/* SPI */

#define S0SPCR  0xE0020000
#define S0SPSR  0xE0020004
#define S0SPDR  0xE0020008
#define S0SPCCR 0xE002000C
#define S0SPINT 0xE002001C

#define S1SPCR  0xE0030000
#define S1SPSR  0xE0030004
#define S1SPDR  0xE0030008
#define S1SPCCR 0xE003000C
#define S1SPINT 0xE003001C

/* CAN controller */

/* skip for now */

/* Timers */

#define T0IR  0xE0004000
#define T0TCR 0xE0004004
#define T0TC  0xE0004008
#define T0PR  0xE000400C
#define T0PC  0xE0004010
#define T0MCR 0xE0004014
#define T0MR0 0xE0004018
#define T0MR1 0xE000401C
#define T0MR2 0xE0004020
#define T0MR3 0xE0004024
#define T0CCR 0xE0004028
#define T0CR0 0xE000402C
#define T0CR1 0xE0004030
#define T0CR2 0xE0004034
#define T0CR3 0xE0004038
#define T0EMR 0xE000403C

#define T1IR  0xE0008000
#define T1TCR 0xE0008004
#define T1TC  0xE0008008
#define T1PR  0xE000800C
#define T1PC  0xE0008010
#define T1MCR 0xE0008014
#define T1MR0 0xE0008018
#define T1MR1 0xE000801C
#define T1MR2 0xE0008020
#define T1MR3 0xE0008024
#define T1CCR 0xE0008028
#define T1CR0 0xE000802C
#define T1CR1 0xE0008030
#define T1CR2 0xE0008034
#define T1CR3 0xE0008038
#define T1EMR 0xE000803C

/* PWM */

/* skip for now */

/* A/D converter */

/* skip for now */

/* Real Time Clock */

/* skip for now */

/* Watchdog */

#define WDMOD  0xE0000000
#define WDTC   0xE0000004
#define WDFEED 0xE0000008
#define WDTV   0xE000000C

/* EmbeddedICE LOGIC */

/* skip for now */

#endif
