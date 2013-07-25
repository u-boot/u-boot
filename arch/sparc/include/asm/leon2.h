/* LEON2 header file. LEON2 is a SOC processor.
 *
 * (C) Copyright 2008
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __LEON2_H__
#define __LEON2_H__

#ifdef CONFIG_LEON2

/* LEON 2 I/O register definitions */
#define LEON2_PREGS	0x80000000
#define LEON2_MCFG1	0x00
#define LEON2_MCFG2	0x04
#define LEON2_ECTRL	0x08
#define LEON2_FADDR	0x0C
#define LEON2_MSTAT	0x10
#define LEON2_CCTRL	0x14
#define LEON2_PWDOWN	0x18
#define LEON2_WPROT1	0x1C
#define LEON2_WPROT2	0x20
#define LEON2_LCONF	0x24
#define LEON2_TCNT0	0x40
#define LEON2_TRLD0	0x44
#define LEON2_TCTRL0	0x48
#define LEON2_TCNT1	0x50
#define LEON2_TRLD1	0x54
#define LEON2_TCTRL1	0x58
#define LEON2_SCNT	0x60
#define LEON2_SRLD	0x64
#define LEON2_UART0	0x70
#define LEON2_UDATA0	0x70
#define LEON2_USTAT0	0x74
#define LEON2_UCTRL0	0x78
#define LEON2_USCAL0	0x7C
#define LEON2_UART1	0x80
#define LEON2_UDATA1	0x80
#define LEON2_USTAT1	0x84
#define LEON2_UCTRL1	0x88
#define LEON2_USCAL1	0x8C
#define LEON2_IMASK	0x90
#define LEON2_IPEND	0x94
#define LEON2_IFORCE	0x98
#define LEON2_ICLEAR	0x9C
#define LEON2_IOREG	0xA0
#define LEON2_IODIR	0xA4
#define LEON2_IOICONF	0xA8
#define LEON2_IPEND2	0xB0
#define LEON2_IMASK2	0xB4
#define LEON2_ISTAT2	0xB8
#define LEON2_ICLEAR2	0xBC

#ifndef __ASSEMBLER__
/*
 *  Structure for LEON memory mapped registers.
 *
 *  Source: Section 6.1 - On-chip registers
 *
 *  NOTE:  There is only one of these structures per CPU, its base address
 *         is 0x80000000, and the variable LEON_REG is placed there by the
 *         linkcmds file.
 */
typedef struct {
	volatile unsigned int Memory_Config_1;
	volatile unsigned int Memory_Config_2;
	volatile unsigned int Edac_Control;
	volatile unsigned int Failed_Address;
	volatile unsigned int Memory_Status;
	volatile unsigned int Cache_Control;
	volatile unsigned int Power_Down;
	volatile unsigned int Write_Protection_1;
	volatile unsigned int Write_Protection_2;
	volatile unsigned int Leon_Configuration;
	volatile unsigned int dummy2;
	volatile unsigned int dummy3;
	volatile unsigned int dummy4;
	volatile unsigned int dummy5;
	volatile unsigned int dummy6;
	volatile unsigned int dummy7;
	volatile unsigned int Timer_Counter_1;
	volatile unsigned int Timer_Reload_1;
	volatile unsigned int Timer_Control_1;
	volatile unsigned int Watchdog;
	volatile unsigned int Timer_Counter_2;
	volatile unsigned int Timer_Reload_2;
	volatile unsigned int Timer_Control_2;
	volatile unsigned int dummy8;
	volatile unsigned int Scaler_Counter;
	volatile unsigned int Scaler_Reload;
	volatile unsigned int dummy9;
	volatile unsigned int dummy10;
	volatile unsigned int UART_Channel_1;
	volatile unsigned int UART_Status_1;
	volatile unsigned int UART_Control_1;
	volatile unsigned int UART_Scaler_1;
	volatile unsigned int UART_Channel_2;
	volatile unsigned int UART_Status_2;
	volatile unsigned int UART_Control_2;
	volatile unsigned int UART_Scaler_2;
	volatile unsigned int Interrupt_Mask;
	volatile unsigned int Interrupt_Pending;
	volatile unsigned int Interrupt_Force;
	volatile unsigned int Interrupt_Clear;
	volatile unsigned int PIO_Data;
	volatile unsigned int PIO_Direction;
	volatile unsigned int PIO_Interrupt;
} LEON2_regs;

typedef struct {
	volatile unsigned int UART_Channel;
	volatile unsigned int UART_Status;
	volatile unsigned int UART_Control;
	volatile unsigned int UART_Scaler;
} LEON2_Uart_regs;

#endif

/*
 *  The following constants are intended to be used ONLY in assembly
 *  language files.
 *
 *  NOTE:  The intended style of usage is to load the address of LEON REGS
 *         into a register and then use these as displacements from
 *         that register.
 */
#define  LEON_REG_MEMCFG1_OFFSET                                  0x00
#define  LEON_REG_MEMCFG2_OFFSET                                  0x04
#define  LEON_REG_EDACCTRL_OFFSET                                 0x08
#define  LEON_REG_FAILADDR_OFFSET                                 0x0C
#define  LEON_REG_MEMSTATUS_OFFSET                                0x10
#define  LEON_REG_CACHECTRL_OFFSET                                0x14
#define  LEON_REG_POWERDOWN_OFFSET                                0x18
#define  LEON_REG_WRITEPROT1_OFFSET                               0x1C
#define  LEON_REG_WRITEPROT2_OFFSET                               0x20
#define  LEON_REG_LEONCONF_OFFSET                                 0x24
#define  LEON_REG_UNIMPLEMENTED_2_OFFSET                          0x28
#define  LEON_REG_UNIMPLEMENTED_3_OFFSET                          0x2C
#define  LEON_REG_UNIMPLEMENTED_4_OFFSET                          0x30
#define  LEON_REG_UNIMPLEMENTED_5_OFFSET                          0x34
#define  LEON_REG_UNIMPLEMENTED_6_OFFSET                          0x38
#define  LEON_REG_UNIMPLEMENTED_7_OFFSET                          0x3C
#define  LEON_REG_TIMERCNT1_OFFSET                                0x40
#define  LEON_REG_TIMERLOAD1_OFFSET                               0x44
#define  LEON_REG_TIMERCTRL1_OFFSET                               0x48
#define  LEON_REG_WDOG_OFFSET                                     0x4C
#define  LEON_REG_TIMERCNT2_OFFSET                                0x50
#define  LEON_REG_TIMERLOAD2_OFFSET                               0x54
#define  LEON_REG_TIMERCTRL2_OFFSET                               0x58
#define  LEON_REG_UNIMPLEMENTED_8_OFFSET                          0x5C
#define  LEON_REG_SCALERCNT_OFFSET                                0x60
#define  LEON_REG_SCALER_LOAD_OFFSET                              0x64
#define  LEON_REG_UNIMPLEMENTED_9_OFFSET                          0x68
#define  LEON_REG_UNIMPLEMENTED_10_OFFSET                         0x6C
#define  LEON_REG_UARTDATA1_OFFSET                                0x70
#define  LEON_REG_UARTSTATUS1_OFFSET                              0x74
#define  LEON_REG_UARTCTRL1_OFFSET                                0x78
#define  LEON_REG_UARTSCALER1_OFFSET                              0x7C
#define  LEON_REG_UARTDATA2_OFFSET                                0x80
#define  LEON_REG_UARTSTATUS2_OFFSET                              0x84
#define  LEON_REG_UARTCTRL2_OFFSET                                0x88
#define  LEON_REG_UARTSCALER2_OFFSET                              0x8C
#define  LEON_REG_IRQMASK_OFFSET                                  0x90
#define  LEON_REG_IRQPEND_OFFSET                                  0x94
#define  LEON_REG_IRQFORCE_OFFSET                                 0x98
#define  LEON_REG_IRQCLEAR_OFFSET                                 0x9C
#define  LEON_REG_PIODATA_OFFSET                                  0xA0
#define  LEON_REG_PIODIR_OFFSET                                   0xA4
#define  LEON_REG_PIOIRQ_OFFSET                                   0xA8
#define  LEON_REG_SIM_RAM_SIZE_OFFSET                             0xF4
#define  LEON_REG_SIM_ROM_SIZE_OFFSET                             0xF8

/*
 *  Interrupt Sources
 *
 *  The interrupt source numbers directly map to the trap type and to
 *  the bits used in the Interrupt Clear, Interrupt Force, Interrupt Mask,
 *  and the Interrupt Pending Registers.
 */
#define LEON_INTERRUPT_CORRECTABLE_MEMORY_ERROR	1
#define LEON_INTERRUPT_UART_1_RX_TX		2
#define LEON_INTERRUPT_UART_0_RX_TX		3
#define LEON_INTERRUPT_EXTERNAL_0		4
#define LEON_INTERRUPT_EXTERNAL_1		5
#define LEON_INTERRUPT_EXTERNAL_2		6
#define LEON_INTERRUPT_EXTERNAL_3		7
#define LEON_INTERRUPT_TIMER1			8
#define LEON_INTERRUPT_TIMER2			9
#define LEON_INTERRUPT_EMPTY1			10
#define LEON_INTERRUPT_EMPTY2			11
#define LEON_INTERRUPT_OPEN_ETH			12
#define LEON_INTERRUPT_EMPTY4			13
#define LEON_INTERRUPT_EMPTY5			14
#define LEON_INTERRUPT_EMPTY6			15

/* Timer Bits */
#define LEON2_TIMER_CTRL_EN	0x1	/* Timer enable */
#define LEON2_TIMER_CTRL_RS	0x2	/* Timer reStart  */
#define LEON2_TIMER_CTRL_LD	0x4	/* Timer reLoad */
#define LEON2_TIMER1_IRQNO	8	/* Timer 1 IRQ number */
#define LEON2_TIMER2_IRQNO	9	/* Timer 2 IRQ number */
#define LEON2_TIMER1_IE		(1<<LEON2_TIMER1_IRQNO)	/* Timer 1 interrupt enable */
#define LEON2_TIMER2_IE		(1<<LEON2_TIMER2_IRQNO)	/* Timer 2 interrupt enable */

/* UART bits */
#define LEON2_UART_CTRL_RE	1	/* UART Receiver enable */
#define LEON2_UART_CTRL_TE	2	/* UART Transmitter enable */
#define LEON2_UART_CTRL_RI	4	/* UART Receiver Interrupt enable */
#define LEON2_UART_CTRL_TI	8	/* UART Transmitter Interrupt enable */
#define LEON2_UART_CTRL_DBG (1<<11)	/* Debug Bit used by GRMON */

#define LEON2_UART_STAT_DR	1	/* UART Data Ready */
#define LEON2_UART_STAT_TSE	2	/* UART Transmit Shift Reg empty */
#define LEON2_UART_STAT_THE	4	/* UART Transmit Hold Reg empty */

#else
#error Include LEON2 header file only if LEON2 processor
#endif

#endif
