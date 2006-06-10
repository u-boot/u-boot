/*
 * (C) Copyright 2005
 * BuS Elektronik GmbH & Co.KG <esw@bus-elektonik.de>
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
 */

#include <common.h>
#include <asm/m5282.h>
#include "VCxK.h"

vu_char *vcxk_bws = (vu_char *)(CFG_CS3_BASE);
#define VCXK_BWS vcxk_bws

static ulong vcxk_driver;


ulong search_vcxk_driver(void);
void vcxk_cls(void);
void vcxk_setbrightness(short brightness);
int vcxk_request(void);
int vcxk_acknowledge_wait(void);
void vcxk_clear(void);

int init_vcxk(void)
{
	VIDEO_Invert_CFG &= ~VIDEO_Invert_IO;
	VIDEO_INVERT_PORT |= VIDEO_INVERT_PIN;
	VIDEO_INVERT_DDR  |= VIDEO_INVERT_PIN;

	VIDEO_REQUEST_PORT |= VIDEO_REQUEST_PIN;
	VIDEO_REQUEST_DDR |= VIDEO_REQUEST_PIN;

	VIDEO_ACKNOWLEDGE_DDR &= ~VIDEO_ACKNOWLEDGE_PIN;

	vcxk_driver = search_vcxk_driver();
	if (vcxk_driver)
	{
		/* use flash resist driver */
	}
	else
	{
		vcxk_cls();
		vcxk_cls();
		vcxk_setbrightness(1000);
	}
	VIDEO_ENABLE_DDR |= VIDEO_ENABLE_PIN;
	VIDEO_ENABLE_PORT |= VIDEO_ENABLE_PIN;
	VIDEO_ENABLE_PORT &= ~VIDEO_ENABLE_PIN;
	return 1;
}

void 	vcxk_loadimage(ulong source)
{
	int cnt;
	vcxk_acknowledge_wait();
	for (cnt=0; cnt<16384; cnt++)
	{
		VCXK_BWS[cnt*2] = (*(vu_char*) source);
		source++;
	}
	vcxk_request();
}

void vcxk_cls(void)
{
	vcxk_acknowledge_wait();
	vcxk_clear();
	vcxk_request();
}

void vcxk_clear(void)
{
	int cnt;
	for (cnt=0; cnt<16384; cnt++)
	{
		VCXK_BWS[cnt*2] = 0x00;
	}
}

void vcxk_setbrightness(short brightness)
{
	VCXK_BWS[0x8000]=(brightness >> 4) +2;
	VCXK_BWS[0xC000]= (brightness + 23) >> 8;
	VCXK_BWS[0xC001]= (brightness + 23) & 0xFF;
}

int vcxk_request(void)
{
	if (vcxk_driver)
	{
		/* use flash resist driver */
	}
	else
	{
		VIDEO_REQUEST_PORT &= ~VIDEO_REQUEST_PIN;
		VIDEO_REQUEST_PORT |= VIDEO_REQUEST_PIN;
	}
	return 1;
}

int vcxk_acknowledge_wait(void)
{
	if (vcxk_driver)
	{
		/* use flash resist driver */
	}
	else
	{
		while (!(VIDEO_ACKNOWLEDGE_PORT & VIDEO_ACKNOWLEDGE_PIN));
	}
	return 1;
}

ulong search_vcxk_driver(void)
{
	return 0;
}

/* eof */
