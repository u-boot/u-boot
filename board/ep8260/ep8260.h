#ifndef __EP8260_H__
#define __EP8260_H__

typedef struct tt_ep_regs {
	volatile unsigned char bcsr0;
	volatile unsigned char bcsr1;
	volatile unsigned char bcsr2;
	volatile unsigned char bcsr3;
	volatile unsigned char bcsr4;
	volatile unsigned char bcsr5;
	volatile unsigned char bcsr6;
	volatile unsigned char bcsr7;
	volatile unsigned char bcsr8;
	volatile unsigned char bcsr9;
	volatile unsigned char bcsr10;
	volatile unsigned char bcsr11;
	volatile unsigned char bcsr12;
	volatile unsigned char bcsr13;
	volatile unsigned char bcsr14;
	volatile unsigned char bcsr15;
} t_ep_regs;
typedef t_ep_regs *tp_ep_regs;

#endif
