/*
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Code in faintly related to linux/arch/ppc/8xx_io:
 * MPC8xx CPM I2C interface. Copyright (c) 1999 Dan Malek (dmalek@jlc.net).
 *
 * This file implements functions to read the MBX's Vital Product Data
 * (VPD). I can't use the more general i2c code in mpc8xx/... since I need
 * the VPD at a time where there is no RAM available yet. Hence the VPD is
 * read into a special area in the DPRAM (see config_MBX.h::CFG_DPRAMVPD).
 *
 * -----------------------------------------------------------------
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
#ifdef CONFIG_8xx
#include <commproc.h>
#endif
#include "vpd.h"

/* Location of receive/transmit buffer descriptors
 * Allocate one transmit bd and one receive bd.
 * IIC_BD_FREE points to free bd space which we'll use as tx buffer.
 */
#define IIC_BD_TX1     (BD_IIC_START + 0*sizeof(cbd_t))
#define IIC_BD_TX2     (BD_IIC_START + 1*sizeof(cbd_t))
#define IIC_BD_RX      (BD_IIC_START + 2*sizeof(cbd_t))
#define IIC_BD_FREE    (BD_IIC_START + 3*sizeof(cbd_t))

/* FIXME -- replace 0x2000 with offsetof */
#define VPD_P ((vpd_t *)(CFG_IMMR + 0x2000 + CFG_DPRAMVPD))

/* transmit/receive buffers */
#define IIC_RX_LENGTH 128

#define WITH_MICROCODE_PATCH

vpd_packet_t * vpd_find_packet(u_char ident)
{
    vpd_packet_t *packet;
    vpd_t *vpd = VPD_P;

    packet = (vpd_packet_t *)&vpd->packets;
    while ((packet->identifier != ident) && packet->identifier != 0xFF)
    {
	packet = (vpd_packet_t *)((char *)packet + packet->size + 2);
    }
    return packet;
}

void vpd_init(void)
{
    volatile immap_t  *im = (immap_t *)CFG_IMMR;
    volatile cpm8xx_t *cp = &(im->im_cpm);
    volatile i2c8xx_t *i2c = (i2c8xx_t *)&(im->im_i2c);
    volatile iic_t *iip;
#ifdef WITH_MICROCODE_PATCH
    ulong reloc = 0;
#endif

    iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];

    /*
     * kludge: when running from flash, no microcode patch can be
     * installed. However, the DPMEM usually contains non-zero
     * garbage at the relocatable patch base location, so lets clear
     * it now. This way the rest of the code can support the microcode
     * patch dynamically.
     */
    if ((ulong)vpd_init & 0xff000000)
      iip->iic_rpbase = 0;

#ifdef WITH_MICROCODE_PATCH
    /* Check for and use a microcode relocation patch. */
    if ((reloc = iip->iic_rpbase))
      iip = (iic_t *)&cp->cp_dpmem[iip->iic_rpbase];
#endif
    /* Initialize Port B IIC pins */
    cp->cp_pbpar |= 0x00000030;
    cp->cp_pbdir |= 0x00000030;
    cp->cp_pbodr |= 0x00000030;

    i2c->i2c_i2mod = 0x04;  /* filter clock */
    i2c->i2c_i2add = 0x34;	/* select an arbitrary (unique) address */
    i2c->i2c_i2brg = 0x07;  /* make clock run maximum slow	*/
    i2c->i2c_i2cmr = 0x00;  /* disable interrupts */
    i2c->i2c_i2cer = 0x1f;  /* clear events */
    i2c->i2c_i2com = 0x01;  /* configure i2c to work as master */

    if (vpd_read(0xa4, (uchar*)VPD_P, VPD_EEPROM_SIZE, 0) != VPD_EEPROM_SIZE)
    {
	hang();
    }
}


/* Read from I2C.
 * This is a two step process.  First, we send the "dummy" write
 * to set the device offset for the read.  Second, we perform
 * the read operation.
 */
int vpd_read(uint iic_device, uchar *buf, int count, int offset)
{
    volatile immap_t  *im = (immap_t *)CFG_IMMR;
    volatile cpm8xx_t *cp = &(im->im_cpm);
    volatile i2c8xx_t *i2c = (i2c8xx_t *)&(im->im_i2c);
    volatile iic_t *iip;
    volatile cbd_t *tbdf1, *tbdf2, *rbdf;
    uchar *tb;
    uchar event;
#ifdef WITH_MICROCODE_PATCH
    ulong reloc = 0;
#endif

    iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];
#ifdef WITH_MICROCODE_PATCH
    /* Check for and use a microcode relocation patch. */
    if ((reloc = iip->iic_rpbase))
      iip = (iic_t *)&cp->cp_dpmem[iip->iic_rpbase];
#endif
    tbdf1 = (cbd_t *)&cp->cp_dpmem[IIC_BD_TX1];
    tbdf2 = (cbd_t *)&cp->cp_dpmem[IIC_BD_TX2];
    rbdf  = (cbd_t *)&cp->cp_dpmem[IIC_BD_RX];

    /* Send a "dummy write" operation.  This is a write request with
     * only the offset sent, followed by another start condition.
     * This will ensure we start reading from the first location
     * of the EEPROM.
     */
    tb = (uchar*)&cp->cp_dpmem[IIC_BD_FREE];
    tb[0] = iic_device & 0xfe;	/* device address */
    tb[1] = offset;	        /* offset */
    tbdf1->cbd_bufaddr = (uint)tb;
    tbdf1->cbd_datlen = 2;
    tbdf1->cbd_sc = 0x8400;

    tb += 2;
    tb[0] = iic_device | 1;	/* device address */
    tbdf2->cbd_bufaddr = (uint)tb;
    tbdf2->cbd_datlen = count+1;
    tbdf2->cbd_sc = 0xbc00;

    rbdf->cbd_bufaddr = (uint)buf;
    rbdf->cbd_datlen = 0;
    rbdf->cbd_sc = 0xb000;

    iip->iic_tbase = IIC_BD_TX1;
    iip->iic_tbptr = IIC_BD_TX1;
    iip->iic_rbase = IIC_BD_RX;
    iip->iic_rbptr = IIC_BD_RX;
    iip->iic_rfcr = 0x15;
    iip->iic_tfcr = 0x15;
    iip->iic_mrblr = count;
    iip->iic_rstate = 0;
    iip->iic_tstate = 0;

    i2c->i2c_i2cer = 0x1f;  /* clear event mask */
    i2c->i2c_i2mod |= 1;    /* enable iic operation */
    i2c->i2c_i2com |= 0x80;	/* start master */

    /* wait for IIC transfer */
    do {
	__asm__ volatile ("eieio");
	event = i2c->i2c_i2cer;
    } while (event == 0);

    if ((event & 0x10) || (event & 0x04)) {
	count = -1;
	goto bailout;
    }

bailout:
    i2c->i2c_i2mod &= ~1;   /* turn off iic operation */
    i2c->i2c_i2cer = 0x1f;  /* clear event mask */

    return count;
}
