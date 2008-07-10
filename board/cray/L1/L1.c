/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <asm/processor.h>
#include <4xx_i2c.h>
#include <command.h>
#include <rtc.h>
#include <post.h>
#include <net.h>
#include <malloc.h>

#define L1_MEMSIZE (32*1024*1024)

/* the std. DHCP stufff */
#define DHCP_ROUTER       3
#define DHCP_NETMASK      1
#define DHCP_BOOTFILE     67
#define DHCP_ROOTPATH     17
#define DHCP_HOSTNAME     12

/* some extras used by CRAY
 *
 * on the server this looks like:
 *
 * option L1-initrd-image code 224 = string;
 * option L1-initrd-image "/opt/craysv2/craymcu/l1/flash/initrd.image"
 */
#define DHCP_L1_INITRD  224

/* new, [better?] way via official vendor-extensions, defining an option
 * space.
 * on the server this looks like:
 *
 * option space CRAYL1;
 * option CRAYL1.initrd     code 3 = string;
 * ..etc...
 */
#define DHCP_VENDOR_SPECX   43
#define DHCP_VX_INITRD       3
#define DHCP_VX_BOOTCMD      4
#define DHCP_VX_BOOTARGS     5
#define DHCP_VX_ROOTDEV      6
#define DHCP_VX_FROMFLASH    7
#define DHCP_VX_BOOTSCRIPT   8
#define DHCP_VX_RCFILE	     9
#define DHCP_VX_MAGIC        10

/* Things DHCP server can tellme about.  If there's no flash address, then
 * they dont participate in 'update' to flash, and we force their values
 * back to '0' every boot to be sure to get them fresh from DHCP.  Yes, I
 * know this is a pain...
 *
 * If I get no bootfile, boot from flash.  If rootpath, use that.  If no
 * rootpath use initrd in flash.
 */
typedef struct dhcp_item_s {
	u8 dhcp_option;
	u8 dhcp_vendor_option;
	char *dhcpvalue;
	char *envname;
} dhcp_item_t;
static dhcp_item_t Things[] = {
	{DHCP_ROUTER, 0, NULL, "gateway"},
	{DHCP_NETMASK, 0, NULL, "netmask"},
	{DHCP_BOOTFILE, 0, NULL, "bootfile"},
	{DHCP_ROOTPATH, 0, NULL, "rootpath"},
	{DHCP_HOSTNAME, 0, NULL, "hostname"},
	{DHCP_L1_INITRD, 0, NULL, "initrd"},
/* and the other way.. */
	{DHCP_VENDOR_SPECX, DHCP_VX_INITRD, NULL, "initrd"},
	{DHCP_VENDOR_SPECX, DHCP_VX_BOOTCMD, NULL, "bootcmd"},
	{DHCP_VENDOR_SPECX, DHCP_VX_FROMFLASH, NULL, "fromflash"},
	{DHCP_VENDOR_SPECX, DHCP_VX_BOOTSCRIPT, NULL, "bootscript"},
	{DHCP_VENDOR_SPECX, DHCP_VX_RCFILE, NULL, "rcfile"},
	{DHCP_VENDOR_SPECX, DHCP_VX_BOOTARGS, NULL, "xbootargs"},
	{DHCP_VENDOR_SPECX, DHCP_VX_ROOTDEV, NULL, NULL},
	{DHCP_VENDOR_SPECX, DHCP_VX_MAGIC, NULL, NULL}
};

#define N_THINGS ((sizeof(Things))/(sizeof(dhcp_item_t)))

extern char bootscript[];

/* Here is the boot logic as HUSH script. Overridden by any TFP provided
 * bootscript file.
 */

static void init_sdram (void);

/* ------------------------------------------------------------------------- */
int board_early_init_f (void)
{
	/* Running from ROM: global data is still READONLY */
	init_sdram ();
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000020);	/* set all but FPGA SMI to be non-critical */
	mtdcr (uicpr, 0xFFFFFFE0);	/* set int polarities */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	return 0;
}

/* ------------------------------------------------------------------------- */
int checkboard (void)
{
	return (0);
}
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
int misc_init_r (void)
{
	char *s, *e;
	image_header_t *hdr;
	time_t timestamp;
	struct rtc_time tm;
	char bootcmd[32];

	hdr = (image_header_t *) (CFG_MONITOR_BASE - image_get_header_size ());
#if defined(CONFIG_FIT)
	if (genimg_get_format ((void *)hdr) != IMAGE_FORMAT_LEGACY) {
		puts ("Non legacy image format not supported\n");
		return -1;
	}
#endif

	timestamp = (time_t)image_get_time (hdr);
	to_tm (timestamp, &tm);
	printf ("Welcome to U-Boot on Cray L1. Compiled %4d-%02d-%02d  %2d:%02d:%02d (UTC)\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

#define FACTORY_SETTINGS 0xFFFC0000
	if ((s = getenv ("ethaddr")) == NULL) {
		e = (char *) (FACTORY_SETTINGS);
		if (*(e + 0) != '0'
			|| *(e + 1) != '0'
			|| *(e + 2) != ':'
			|| *(e + 3) != '4' || *(e + 4) != '0' || *(e + 17) != '\0') {
			printf ("No valid MAC address in flash location 0x3C0000!\n");
		} else {
			printf ("Factory MAC: %s\n", e);
			setenv ("ethaddr", e);
		}
	}
	sprintf (bootcmd,"autoscript %X",(unsigned)bootscript);
	setenv ("bootcmd", bootcmd);
	return (0);
}

/* ------------------------------------------------------------------------- */
phys_size_t initdram (int board_type)
{
	return (L1_MEMSIZE);
}

/* ------------------------------------------------------------------------- */
/* stubs so we can print dates w/o any nvram RTC.*/
int rtc_get (struct rtc_time *tmp)
{
	return 0;
}
void rtc_set (struct rtc_time *tmp)
{
	return;
}
void rtc_reset (void)
{
	return;
}

/* ------------------------------------------------------------------------- */
/*  Do sdram bank init in C so I can read it..no console to print to yet!
 */
static void init_sdram (void)
{
 unsigned long tmp;

	/* write SDRAM bank 0 register */
	mtdcr (memcfga, mem_mb0cf);
	mtdcr (memcfgd, 0x00062001);

/* Set the SDRAM Timing reg, SDTR1 and the refresh timer reg, RTR.	*/
/* To set the appropriate timings, we need to know the SDRAM speed.	*/
/* We can use the PLB speed since the SDRAM speed is the same as	*/
/* the PLB speed. The PLB speed is the FBK divider times the		*/
/* 405GP reference clock, which on the L1 is 25Mhz.			*/
/* Thus, if FBK div is 2, SDRAM is 50Mhz; if FBK div is 3, SDRAM is	*/
/* 150Mhz; if FBK is 3, SDRAM is 150Mhz.				*/

	/* divisor = ((mfdcr(strap)>> 28) & 0x3); */

/* write SDRAM timing for 100Mhz. */
	mtdcr (memcfga, mem_sdtr1);
	mtdcr (memcfgd, 0x0086400D);

/* write SDRAM refresh interval register */
	mtdcr (memcfga, mem_rtr);
	mtdcr (memcfgd, 0x05F00000);
	udelay (200);

/* sdram controller.*/
	mtdcr (memcfga, mem_mcopt1);
	mtdcr (memcfgd, 0x90800000);
	udelay (200);

/* initially, disable ECC on all banks */
	udelay (200);
	mtdcr (memcfga, mem_ecccf);
	tmp = mfdcr (memcfgd);
	tmp &= 0xff0fffff;
	mtdcr (memcfga, mem_ecccf);
	mtdcr (memcfgd, tmp);

	return;
}

extern int memory_post_test (int flags);

int testdram (void)
{
 unsigned long tmp;
	uint *pstart = (uint *) 0x00000000;
	uint *pend = (uint *) L1_MEMSIZE;
	uint *p;

	if (getenv_r("booted",NULL,0) <= 0)
	{
		printf ("testdram..");
	/*AA*/
		for (p = pstart; p < pend; p++)
			*p = 0xaaaaaaaa;
		for (p = pstart; p < pend; p++) {
			if (*p != 0xaaaaaaaa) {
				printf ("SDRAM test fails at: %08x, was %08x expected %08x\n",
						(uint) p, *p, 0xaaaaaaaa);
				return 1;
			}
		}
	/*55*/
		for (p = pstart; p < pend; p++)
			*p = 0x55555555;
		for (p = pstart; p < pend; p++) {
			if (*p != 0x55555555) {
				printf ("SDRAM test fails at: %08x, was %08x expected %08x\n",
						(uint) p, *p, 0x55555555);
				return 1;
			}
		}
	/*addr*/
		for (p = pstart; p < pend; p++)
			*p = (unsigned)p;
		for (p = pstart; p < pend; p++) {
			if (*p != (unsigned)p) {
				printf ("SDRAM test fails at: %08x, was %08x expected %08x\n",
						(uint) p, *p, (uint)p);
				return 1;
			}
		}
		printf ("Success. ");
	}
	printf ("Enable ECC..");

	mtdcr (memcfga, mem_mcopt1);
	tmp = (mfdcr (memcfgd) & ~0xFFE00000) | 0x90800000;
	mtdcr (memcfga, mem_mcopt1);
	mtdcr (memcfgd, tmp);
	udelay (600);
	for (p = (unsigned long) 0; ((unsigned long) p < L1_MEMSIZE); *p++ = 0L)
		;
	udelay (400);
	mtdcr (memcfga, mem_ecccf);
	tmp = mfdcr (memcfgd);
	tmp |= 0x00800000;
	mtdcr (memcfgd, tmp);
	udelay (400);
	printf ("enabled.\n");
	return (0);
}

/* ------------------------------------------------------------------------- */
static u8 *dhcp_env_update (u8 thing, u8 * pop)
{
	u8 i, oplen;

	oplen = *(pop + 1);

	if ((Things[thing].dhcpvalue = malloc (oplen)) == NULL) {
		printf ("Whoops! failed to malloc space for DHCP thing %s\n",
				Things[thing].envname);
		return NULL;
	}
	for (i = 0; (i < oplen); i++)
		if ((*(Things[thing].dhcpvalue + i) = *(pop + 2 + i)) == ' ')
			break;
	*(Things[thing].dhcpvalue + i) = '\0';

/* set env. */
	if (Things[thing].envname)
	{
		setenv (Things[thing].envname, Things[thing].dhcpvalue);
	}
	return ((u8 *)(Things[thing].dhcpvalue));
}

/* ------------------------------------------------------------------------- */
u8 *dhcp_vendorex_prep (u8 * e)
{
	u8 thing;

/* ask for the things I want. */
	*e++ = 55;					/* Parameter Request List */
	*e++ = N_THINGS;
	for (thing = 0; thing < N_THINGS; thing++)
		*e++ = Things[thing].dhcp_option;
	*e++ = 255;

	return e;
}

/* ------------------------------------------------------------------------- */
/* .. return NULL means it wasnt mine, non-null means I got it..*/
u8 *dhcp_vendorex_proc (u8 * pop)
{
	u8 oplen, *sub_op, sub_oplen, *retval;
	u8 thing = 0;

	retval = NULL;
	oplen = *(pop + 1);
/* if pop is vender spec indicator, there are sub-options. */
	if (*pop == DHCP_VENDOR_SPECX) {
		for (sub_op = pop + 2;
		     oplen && (sub_oplen = *(sub_op + 1));
		     oplen -= sub_oplen, sub_op += (sub_oplen + 2)) {
			for (thing = 0; thing < N_THINGS; thing++) {
			    if (*sub_op == Things[thing].dhcp_vendor_option) {
					if (!(retval = dhcp_env_update (thing, sub_op))) {
						return NULL;
					}
			    }
			}
		}
	} else {
		for (thing = 0; thing < N_THINGS; thing++) {
			if (*pop == Things[thing].dhcp_option)
				if (!(retval = dhcp_env_update (thing, pop)))
					return NULL;
		}
	}
	return (pop);
}
