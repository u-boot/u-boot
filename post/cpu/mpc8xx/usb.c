/*
 * (C) Copyright 2002
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

/*
 * USB test
 *
 * The USB controller is tested in the local loopback mode.
 * It is configured so that endpoint 0 operates as host and endpoint 1
 * operates as function endpoint. After that an IN token transaction
 * is performed.
 * Refer to MPC850 User Manual, Section 32.11.1 USB Host Controller
 * Initialization Example.
 */

#ifdef CONFIG_POST

#include <post.h>

#if CONFIG_POST & CFG_POST_USB

#include <commproc.h>
#include <command.h>

#define TOUT_LOOP 100

#define	PROFF_USB		((uint)0x0000)

#define CPM_USB_EP0_BASE	0x0a00
#define CPM_USB_EP1_BASE	0x0a20

#define CPM_USB_DT0_BASE	0x0a80
#define CPM_USB_DT1_BASE	0x0a90
#define CPM_USB_DR0_BASE	0x0aa0
#define CPM_USB_DR1_BASE	0x0ab0

#define CPM_USB_RX0_BASE	0x0b00
#define CPM_USB_RX1_BASE	0x0b08
#define CPM_USB_TX0_BASE	0x0b20
#define CPM_USB_TX1_BASE	0x0b28

#define USB_EXPECT(x)		if (!(x)) goto Done;

typedef struct usb_param {
	ushort ep0ptr;
	ushort ep1ptr;
	ushort ep2ptr;
	ushort ep3ptr;
	uint rstate;
	uint rptr;
	ushort frame_n;
	ushort rbcnt;
	ushort rtemp;
} usb_param_t;

typedef struct usb_param_block {
	ushort rbase;
	ushort tbase;
	uchar rfcr;
	uchar tfcr;
	ushort mrblr;
	ushort rbptr;
	ushort tbptr;
	uint tstate;
	uint tptr;
	ushort tcrc;
	ushort tbcnt;
	uint res[2];
} usb_param_block_t;

typedef struct usb {
	uchar usmod;
	uchar usadr;
	uchar uscom;
	uchar res1;
	ushort usep[4];
	uchar res2[4];
	ushort usber;
	uchar res3[2];
	ushort usbmr;
	uchar res4;
	uchar usbs;
	uchar res5[8];
} usb_t;

int usb_post_test (int flags)
{
	int res = -1;
	volatile immap_t *im = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(im->im_cpm);
	volatile usb_param_t *pram_ptr;
	uint dpram;
	ushort DPRAM;
	volatile cbd_t *tx;
	volatile cbd_t *rx;
	volatile usb_t *usbr;
	volatile usb_param_block_t *ep0;
	volatile usb_param_block_t *ep1;
	int j;

	pram_ptr = (usb_param_t *) & (im->im_cpm.cp_dparam[PROFF_USB]);
	dpram = (uint) im->im_cpm.cp_dpmem;
	DPRAM = dpram;
	tx = (cbd_t *) (dpram + CPM_USB_TX0_BASE);
	rx = (cbd_t *) (dpram + CPM_USB_RX0_BASE);
	ep0 = (usb_param_block_t *) (dpram + CPM_USB_EP0_BASE);
	ep1 = (usb_param_block_t *) (dpram + CPM_USB_EP1_BASE);
	usbr = (usb_t *) & (im->im_cpm.cp_scc[0]);

	/* 01 */
	im->im_ioport.iop_padir &= ~(ushort) 0x0200;
	im->im_ioport.iop_papar |= (ushort) 0x0200;

	cp->cp_sicr &= ~0x000000FF;
	cp->cp_sicr |= 0x00000018;

	cp->cp_brgc4 = 0x00010001;

	/* 02 */
	im->im_ioport.iop_padir &= ~(ushort) 0x0002;
	im->im_ioport.iop_padir &= ~(ushort) 0x0001;

	im->im_ioport.iop_papar |= (ushort) 0x0002;
	im->im_ioport.iop_papar |= (ushort) 0x0001;

	/* 03 */
	im->im_ioport.iop_pcdir &= ~(ushort) 0x0020;
	im->im_ioport.iop_pcdir &= ~(ushort) 0x0010;

	im->im_ioport.iop_pcpar &= ~(ushort) 0x0020;
	im->im_ioport.iop_pcpar &= ~(ushort) 0x0010;

	im->im_ioport.iop_pcso |= (ushort) 0x0020;
	im->im_ioport.iop_pcso |= (ushort) 0x0010;

	/* 04 */
	im->im_ioport.iop_pcdir |= (ushort) 0x0200;
	im->im_ioport.iop_pcdir |= (ushort) 0x0100;

	im->im_ioport.iop_pcpar |= (ushort) 0x0200;
	im->im_ioport.iop_pcpar |= (ushort) 0x0100;

	/* 05 */
	pram_ptr->frame_n = 0;

	/* 06 */
	pram_ptr->ep0ptr = DPRAM + CPM_USB_EP0_BASE;
	pram_ptr->ep1ptr = DPRAM + CPM_USB_EP1_BASE;

	/* 07-10 */
	tx[0].cbd_sc = 0xB800;
	tx[0].cbd_datlen = 3;
	tx[0].cbd_bufaddr = dpram + CPM_USB_DT0_BASE;

	tx[1].cbd_sc = 0xBC80;
	tx[1].cbd_datlen = 3;
	tx[1].cbd_bufaddr = dpram + CPM_USB_DT1_BASE;

	rx[0].cbd_sc = 0xA000;
	rx[0].cbd_datlen = 0;
	rx[0].cbd_bufaddr = dpram + CPM_USB_DR0_BASE;

	rx[1].cbd_sc = 0xA000;
	rx[1].cbd_datlen = 0;
	rx[1].cbd_bufaddr = dpram + CPM_USB_DR1_BASE;

	/* 11-12 */
	*(volatile int *) (dpram + CPM_USB_DT0_BASE) = 0x69856000;
	*(volatile int *) (dpram + CPM_USB_DT1_BASE) = 0xABCD1234;

	*(volatile int *) (dpram + CPM_USB_DR0_BASE) = 0;
	*(volatile int *) (dpram + CPM_USB_DR1_BASE) = 0;

	/* 13-16 */
	ep0->rbase = DPRAM + CPM_USB_RX0_BASE;
	ep0->tbase = DPRAM + CPM_USB_TX0_BASE;
	ep0->rfcr = 0x18;
	ep0->tfcr = 0x18;
	ep0->mrblr = 0x100;
	ep0->rbptr = DPRAM + CPM_USB_RX0_BASE;
	ep0->tbptr = DPRAM + CPM_USB_TX0_BASE;
	ep0->tstate = 0;

	/* 17-20 */
	ep1->rbase = DPRAM + CPM_USB_RX1_BASE;
	ep1->tbase = DPRAM + CPM_USB_TX1_BASE;
	ep1->rfcr = 0x18;
	ep1->tfcr = 0x18;
	ep1->mrblr = 0x100;
	ep1->rbptr = DPRAM + CPM_USB_RX1_BASE;
	ep1->tbptr = DPRAM + CPM_USB_TX1_BASE;
	ep1->tstate = 0;

	/* 21-24 */
	usbr->usep[0] = 0x0000;
	usbr->usep[1] = 0x1100;
	usbr->usep[2] = 0x2200;
	usbr->usep[3] = 0x3300;

	/* 25 */
	usbr->usmod = 0x06;

	/* 26 */
	usbr->usadr = 0x05;

	/* 27 */
	usbr->uscom = 0;

	/* 28 */
	usbr->usmod |= 0x01;
	udelay (1);

	/* 29-30 */
	usbr->uscom = 0x80;
	usbr->uscom = 0x81;

	/* Wait for the data packet to be transmitted */
	for (j = 0; j < TOUT_LOOP; j++) {
		if (tx[1].cbd_sc & (ushort) 0x8000)
			udelay (1);
		else
			break;
	}

	USB_EXPECT (j < TOUT_LOOP);

	USB_EXPECT (tx[0].cbd_sc == 0x3800);
	USB_EXPECT (tx[0].cbd_datlen == 3);

	USB_EXPECT (tx[1].cbd_sc == 0x3C80);
	USB_EXPECT (tx[1].cbd_datlen == 3);

	USB_EXPECT (rx[0].cbd_sc == 0x2C00);
	USB_EXPECT (rx[0].cbd_datlen == 5);

	USB_EXPECT (*(volatile int *) (dpram + CPM_USB_DR0_BASE) ==
				0xABCD122B);
	USB_EXPECT (*(volatile char *) (dpram + CPM_USB_DR0_BASE + 4) == 0x42);

	res = 0;
  Done:

	return res;
}

#endif /* CONFIG_POST & CFG_POST_USB */

#endif /* CONFIG_POST */
