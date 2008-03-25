/*
Ported to U-Boot  by Christian Pellegrin <chri@ascensit.com>

Based on sources from the Linux kernel (pcnet_cs.c, 8390.h) and
eCOS(if_dp83902a.c, if_dp83902a.h). Both of these 2 wonderful world
are GPL, so this is, of course, GPL.

==========================================================================

      dev/dp83902a.h

      National Semiconductor DP83902a ethernet chip

==========================================================================
####ECOSGPLCOPYRIGHTBEGIN####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later version.

 eCos is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with eCos; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

 As a special exception, if other files instantiate templates or use macros
 or inline functions from this file, or you compile this file and link it
 with other works to produce a work based on this file, this file does not
 by itself cause the resulting work to be covered by the GNU General Public
 License. However the source code for this file must still be made available
 in accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work based on
 this file might be covered by the GNU General Public License.

 Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
 at http://sources.redhat.com/ecos/ecos-license/
 -------------------------------------------
####ECOSGPLCOPYRIGHTEND####
####BSDCOPYRIGHTBEGIN####

 -------------------------------------------

 Portions of this software may have been derived from OpenBSD or other sources,
 and are covered by the appropriate copyright disclaimers included herein.

 -------------------------------------------

####BSDCOPYRIGHTEND####
==========================================================================
#####DESCRIPTIONBEGIN####

 Author(s):    gthomas
 Contributors: gthomas, jskov
 Date:         2001-06-13
 Purpose:
 Description:

####DESCRIPTIONEND####

==========================================================================
*/

/*
 * NE2000 support header file.
 *		Created by Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#ifndef __DRIVERS_NE2000_H__
#define __DRIVERS_NE2000_H__

/* Enable NE2000 basic init function */
#define NE2000_BASIC_INIT

#define DP_DATA     0x10
#define START_PG    0x50    /* First page of TX buffer */
#define STOP_PG     0x80    /* Last page +1 of RX ring */

#define RX_START    0x50
#define RX_END      0x80

#define DP_IN(_b_, _o_, _d_)  (_d_) = *( (vu_char *) ((_b_)+(_o_)))
#define DP_OUT(_b_, _o_, _d_) *( (vu_char *) ((_b_)+(_o_))) = (_d_)
#define DP_IN_DATA(_b_, _d_)  (_d_) = *( (vu_char *) ((_b_)))
#define DP_OUT_DATA(_b_, _d_) *( (vu_char *) ((_b_))) = (_d_)

static void pcnet_reset_8390(void)
{
	int i, r;

	PRINTK("nic base is %lx\n", nic_base);

	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", nic_base+ E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE1+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", nic_base+ E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);
	PRINTK("cmd (at %lx) is %x\n", nic_base+ E8390_CMD, n2k_inb(E8390_CMD));
	n2k_outb(E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD);

	n2k_outb(n2k_inb(PCNET_RESET), PCNET_RESET);

	for (i = 0; i < 100; i++) {
		if ((r = (n2k_inb(EN0_ISR) & ENISR_RESET)) != 0)
			break;
		PRINTK("got %x in reset\n", r);
		udelay(100);
	}
	n2k_outb(ENISR_RESET, EN0_ISR); /* Ack intr. */

	if (i == 100)
		printf("pcnet_reset_8390() did not complete.\n");
} /* pcnet_reset_8390 */

int get_prom(u8* mac_addr)
{
	u8 prom[32];
	int i, j;
	struct {
		u_char value, offset;
	} program_seq[] = {
		{E8390_NODMA+E8390_PAGE0+E8390_STOP, E8390_CMD}, /* Select page 0*/
		{0x48,  EN0_DCFG},		/* Set byte-wide (0x48) access. */
		{0x00,  EN0_RCNTLO},		/* Clear the count regs. */
		{0x00,  EN0_RCNTHI},
		{0x00,  EN0_IMR},		/* Mask completion irq. */
		{0xFF,  EN0_ISR},
		{E8390_RXOFF, EN0_RXCR},	/* 0x20  Set to monitor */
		{E8390_TXOFF, EN0_TXCR},	/* 0x02  and loopback mode. */
		{32,    EN0_RCNTLO},
		{0x00,  EN0_RCNTHI},
		{0x00,  EN0_RSARLO},		/* DMA starting at 0x0000. */
		{0x00,  EN0_RSARHI},
		{E8390_RREAD+E8390_START, E8390_CMD},
	};

	PRINTK ("trying to get MAC via prom reading\n");

	pcnet_reset_8390 ();

	mdelay (10);

	for (i = 0; i < sizeof (program_seq) / sizeof (program_seq[0]); i++)
		n2k_outb (program_seq[i].value, program_seq[i].offset);

	PRINTK ("PROM:");
	for (i = 0; i < 32; i++) {
		prom[i] = n2k_inb (PCNET_DATAPORT);
		PRINTK (" %02x", prom[i]);
	}
	PRINTK ("\n");
	for (i = 0; i < NR_INFO; i++) {
		if ((prom[0] == hw_info[i].a0) &&
		    (prom[2] == hw_info[i].a1) &&
		    (prom[4] == hw_info[i].a2)) {
			PRINTK ("matched board %d\n", i);
			break;
		}
	}
	if ((i < NR_INFO) || ((prom[28] == 0x57) && (prom[30] == 0x57))) {
		PRINTK ("on exit i is %d/%ld\n", i, NR_INFO);
		PRINTK ("MAC address is ");
		for (j = 0; j < 6; j++) {
			mac_addr[j] = prom[j << 1];
			PRINTK ("%02x:", mac_addr[i]);
		}
		PRINTK ("\n");
		return (i < NR_INFO) ? i : 0;
	}
	return NULL;
}
#endif /* __DRIVERS_NE2000_H__ */
