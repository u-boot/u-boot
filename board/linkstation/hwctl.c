/*
 * hwctl.c
 *
 * LinkStation HW Control Driver
 *
 * Copyright (C) 2001-2004  BUFFALO INC.
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL), incorporated herein by reference.
 * Drivers based on or derived from this code fall under the GPL and must
 * retain the authorship, copyright and license notice.  This file is not
 * a complete program and may only be used when the entire operating
 * system is licensed under the GPL.
 *
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/io.h>

#define AVR_PORT CONFIG_SYS_NS16550_COM2

/* 2005.5.10 BUFFALO add */
/*--------------------------------------------------------------*/
static inline void miconCntl_SendUart(unsigned char dat)
{
	out_8((unsigned char *)AVR_PORT, dat);
	mdelay(1);
}

/*--------------------------------------------------------------*/
void miconCntl_SendCmd(unsigned char dat)
{
	int i;

	for (i=0; i<4; i++){
		miconCntl_SendUart(dat);
	}
}

/*--------------------------------------------------------------*/
void miconCntl_FanLow(void)
{
#ifdef CONFIG_HTGL
	miconCntl_SendCmd(0x5C);
#endif
}

/*--------------------------------------------------------------*/
void miconCntl_FanHigh(void)
{
#ifdef CONFIG_HTGL
	miconCntl_SendCmd(0x5D);
#endif
}

/*--------------------------------------------------------------*/
/* 1000Mbps */
void miconCntl_Eth1000M(int up)
{
#ifdef CONFIG_HTGL
	if (up)
		miconCntl_SendCmd(0x93);
	else
		miconCntl_SendCmd(0x92);
#else
	if (up)
		miconCntl_SendCmd(0x5D);
	else
		miconCntl_SendCmd(0x5C);
#endif
}

/*--------------------------------------------------------------*/
/* 100Mbps */
void miconCntl_Eth100M(int up)
{
#ifdef CONFIG_HTGL
	if (up)
		miconCntl_SendCmd(0x91);
	else
		miconCntl_SendCmd(0x90);
#else
	if (up)
		miconCntl_SendCmd(0x5C);
#endif
}

/*--------------------------------------------------------------*/
/* 10Mbps */
void miconCntl_Eth10M(int up)
{
#ifdef CONFIG_HTGL
	if (up)
		miconCntl_SendCmd(0x8F);
	else
		miconCntl_SendCmd(0x8E);
#else
	if (up)
		miconCntl_SendCmd(0x5C);
#endif
}

/*--------------------------------------------------------------*/
/*  */
void miconCntl_5f(void)
{
	miconCntl_SendCmd(0x5F);
	mdelay(100);
}

/*--------------------------------------------------------------*/
/* "reboot start" signal */
void miconCntl_Reboot(void)
{
	miconCntl_SendCmd(0x43);
}

/*--------------------------------------------------------------*/
/* Disable watchdog timer */
void miconCntl_DisWDT(void)
{
	miconCntl_SendCmd(0x41); /* A */
	miconCntl_SendCmd(0x46); /* F */
	miconCntl_SendCmd(0x4A); /* J */
	miconCntl_SendCmd(0x3E); /* > */
	miconCntl_SendCmd(0x56); /* V */
	miconCntl_SendCmd(0x3E); /* > */
	miconCntl_SendCmd(0x5A); /* Z */
	miconCntl_SendCmd(0x56); /* V */
	miconCntl_SendCmd(0x4B); /* K */
}
