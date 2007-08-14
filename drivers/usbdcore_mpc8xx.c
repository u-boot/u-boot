/*
 * Copyright (C) 2006 by Bryan O'Donoghue, CodeHermit
 * bodonoghue@CodeHermit.ie
 *
 * References
 * DasUBoot/drivers/usbdcore_omap1510.c, for design and implementation ideas.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/*
 * Notes :
 * 1.	#define __SIMULATE_ERROR__ to inject a CRC error into every 2nd TX
 *		packet to force the USB re-transmit protocol.
 *
 * 2.	#define __DEBUG_UDC__ to switch on debug tracing to serial console
 *	be careful that tracing doesn't create Hiesen-bugs with respect to
 *	response timeouts to control requests.
 *
 * 3.	This driver should be able to support any higher level driver that
 *	that wants to do either of the two standard UDC implementations
 *	Control-Bulk-Interrupt or  Bulk-IN/Bulk-Out standards. Hence
 *	gserial and cdc_acm should work with this code.
 *
 * 4.	NAK events never actually get raised at all, the documentation
 *	is just wrong !
 *
 * 5.	For some reason, cbd_datlen is *always* +2 the value it should be.
 *	this means that having an RX cbd of 16 bytes is not possible, since
 *	the same size is reported for 14 bytes received as 16 bytes received
 *	until we can find out why this happens, RX cbds must be limited to 8
 *	bytes. TODO: check errata for this behaviour.
 *
 * 6.	Right now this code doesn't support properly powering up with the USB
 *	cable attached to the USB host my development board the Adder87x doesn't
 *	have a pull-up fitted to allow this, so it is necessary to power the
 *	board and *then* attached the USB cable to the host. However somebody
 *	with a different design in their board may be able to keep the cable
 *	constantly connected and simply enable/disable a pull-up  re
 *	figure 31.1 in MPC885RM.pdf instead of having to power up the board and
 *	then attach the cable !
 *
 */
#include <common.h>
#include <config.h>

#if defined(CONFIG_MPC885_FAMILY) && defined(CONFIG_USB_DEVICE)
#include <commproc.h>
#include "usbdcore.h"
#include "usbdcore_mpc8xx.h"
#include "usbdcore_ep0.h"

#define ERR(fmt, args...)\
	serial_printf("ERROR : [%s] %s:%d: "fmt,\
				__FILE__,__FUNCTION__,__LINE__, ##args)
#ifdef __DEBUG_UDC__
#define DBG(fmt,args...)\
		serial_printf("[%s] %s:%d: "fmt,\
				__FILE__,__FUNCTION__,__LINE__, ##args)
#else
#define DBG(fmt,args...)
#endif

/* Static Data */
#ifdef __SIMULATE_ERROR__
static char err_poison_test = 0;
#endif
static struct mpc8xx_ep ep_ref[MAX_ENDPOINTS];
static u32 address_base = STATE_NOT_READY;
static mpc8xx_udc_state_t udc_state = 0;
static struct usb_device_instance *udc_device = 0;
static volatile usb_epb_t *endpoints[MAX_ENDPOINTS];
static volatile cbd_t *tx_cbd[TX_RING_SIZE];
static volatile cbd_t *rx_cbd[RX_RING_SIZE];
static volatile immap_t *immr = 0;
static volatile cpm8xx_t *cp = 0;
static volatile usb_pram_t *usb_paramp = 0;
static volatile usb_t *usbp = 0;
static int rx_ct = 0;
static int tx_ct = 0;

/* Static Function Declarations */
static void mpc8xx_udc_state_transition_up (usb_device_state_t initial,
					    usb_device_state_t final);
static void mpc8xx_udc_state_transition_down (usb_device_state_t initial,
					      usb_device_state_t final);
static void mpc8xx_udc_stall (unsigned int ep);
static void mpc8xx_udc_flush_tx_fifo (int epid);
static void mpc8xx_udc_flush_rx_fifo (void);
static void mpc8xx_udc_clear_rxbd (volatile cbd_t * rx_cbdp);
static void mpc8xx_udc_init_tx (struct usb_endpoint_instance *epi,
				struct urb *tx_urb);
static void mpc8xx_udc_dump_request (struct usb_device_request *request);
static void mpc8xx_udc_clock_init (volatile immap_t * immr,
				   volatile cpm8xx_t * cp);
static int mpc8xx_udc_ep_tx (struct usb_endpoint_instance *epi);
static int mpc8xx_udc_epn_rx (unsigned int epid, volatile cbd_t * rx_cbdp);
static void mpc8xx_udc_ep0_rx (volatile cbd_t * rx_cbdp);
static void mpc8xx_udc_cbd_init (void);
static void mpc8xx_udc_endpoint_init (void);
static void mpc8xx_udc_cbd_attach (int ep, uchar tx_size, uchar rx_size);
static u32 mpc8xx_udc_alloc (u32 data_size, u32 alignment);
static int mpc8xx_udc_ep0_rx_setup (volatile cbd_t * rx_cbdp);
static void mpc8xx_udc_set_nak (unsigned int ep);
static short mpc8xx_udc_handle_txerr (void);
static void mpc8xx_udc_advance_rx (volatile cbd_t ** rx_cbdp, int epid);

/******************************************************************************
			       Global Linkage
 *****************************************************************************/

/* udc_init
 *
 * Do initial bus gluing
 */
int udc_init (void)
{
	/* Init various pointers */
	immr = (immap_t *) CFG_IMMR;
	cp = (cpm8xx_t *) & (immr->im_cpm);
	usb_paramp = (usb_pram_t *) & (cp->cp_dparam[PROFF_USB]);
	usbp = (usb_t *) & (cp->cp_scc[0]);

	memset (ep_ref, 0x00, (sizeof (struct mpc8xx_ep) * MAX_ENDPOINTS));

	udc_device = 0;
	udc_state = STATE_NOT_READY;

	usbp->usmod = 0x00;
	usbp->uscom = 0;

	/* Set USB Frame #0, Respond at Address & Get a clock source  */
	usbp->usaddr = 0x00;
	mpc8xx_udc_clock_init (immr, cp);

	/* PA15, PA14 as perhiperal USBRXD and USBOE */
	immr->im_ioport.iop_padir &= ~0x0003;
	immr->im_ioport.iop_papar |= 0x0003;

	/* PC11/PC10 as peripheral USBRXP USBRXN */
	immr->im_ioport.iop_pcso |= 0x0030;

	/* PC7/PC6 as perhiperal USBTXP and USBTXN */
	immr->im_ioport.iop_pcdir |= 0x0300;
	immr->im_ioport.iop_pcpar |= 0x0300;

	/* Set the base address */
	address_base = (u32) (cp->cp_dpmem + CPM_USB_BASE);

	/* Initialise endpoints and circular buffers */
	mpc8xx_udc_endpoint_init ();
	mpc8xx_udc_cbd_init ();

	/* Assign allocated Dual Port Endpoint descriptors */
	usb_paramp->ep0ptr = (u32) endpoints[0];
	usb_paramp->ep1ptr = (u32) endpoints[1];
	usb_paramp->ep2ptr = (u32) endpoints[2];
	usb_paramp->ep3ptr = (u32) endpoints[3];
	usb_paramp->frame_n = 0;

	DBG ("ep0ptr=0x%08x ep1ptr=0x%08x ep2ptr=0x%08x ep3ptr=0x%08x\n",
	     usb_paramp->ep0ptr, usb_paramp->ep1ptr, usb_paramp->ep2ptr,
	     usb_paramp->ep3ptr);

	return 0;
}

/* udc_irq
 *
 * Poll for whatever events may have occured
 */
void udc_irq (void)
{
	int epid = 0;
	volatile cbd_t *rx_cbdp = 0;
	volatile cbd_t *rx_cbdp_base = 0;

	if (udc_state != STATE_READY) {
		return;
	}

	if (usbp->usber & USB_E_BSY) {
		/* This shouldn't happen. If it does then it's a bug ! */
		usbp->usber |= USB_E_BSY;
		mpc8xx_udc_flush_rx_fifo ();
	}

	/* Scan all RX/Bidirectional Endpoints for RX data. */
	for (epid = 0; epid < MAX_ENDPOINTS; epid++) {
		if (!ep_ref[epid].prx) {
			continue;
		}
		rx_cbdp = rx_cbdp_base = ep_ref[epid].prx;

		do {
			if (!(rx_cbdp->cbd_sc & RX_BD_E)) {

				if (rx_cbdp->cbd_sc & 0x1F) {
					/* Corrupt data discard it.
					 * Controller has NAK'd this packet.
					 */
					mpc8xx_udc_clear_rxbd (rx_cbdp);

				} else {
					if (!epid) {
						mpc8xx_udc_ep0_rx (rx_cbdp);

					} else {
						/* Process data */
						mpc8xx_udc_set_nak (epid);
						mpc8xx_udc_epn_rx (epid, rx_cbdp);
						mpc8xx_udc_clear_rxbd (rx_cbdp);
					}
				}

				/* Advance RX CBD pointer */
				mpc8xx_udc_advance_rx (&rx_cbdp, epid);
				ep_ref[epid].prx = rx_cbdp;
			} else {
				/* Advance RX CBD pointer */
				mpc8xx_udc_advance_rx (&rx_cbdp, epid);
			}

		} while (rx_cbdp != rx_cbdp_base);
	}

	/* Handle TX events as appropiate, the correct place to do this is
	 * in a tx routine. Perhaps TX on epn was pre-empted by ep0
	 */

	if (usbp->usber & USB_E_TXB) {
		usbp->usber |= USB_E_TXB;
	}

	if (usbp->usber & (USB_TX_ERRMASK)) {
		mpc8xx_udc_handle_txerr ();
	}

	/* Switch to the default state, respond at the default address */
	if (usbp->usber & USB_E_RESET) {
		usbp->usber |= USB_E_RESET;
		usbp->usaddr = 0x00;
		udc_device->device_state = STATE_DEFAULT;
	}

	/* if(usbp->usber&USB_E_IDLE){
	   We could suspend here !
	   usbp->usber|=USB_E_IDLE;
	   DBG("idle state change\n");
	   }
	   if(usbp->usbs){
	   We could resume here when IDLE is deasserted !
	   Not worth doing, so long as we are self powered though.
	   }
	*/

	return;
}

/* udc_endpoint_write
 *
 * Write some data to an endpoint
 */
int udc_endpoint_write (struct usb_endpoint_instance *epi)
{
	int ep = 0;
	short epid = 1, unnak = 0, ret = 0;

	if (udc_state != STATE_READY) {
		ERR ("invalid udc_state != STATE_READY!\n");
		return -1;
	}

	if (!udc_device || !epi) {
		return -1;
	}

	if (udc_device->device_state != STATE_CONFIGURED) {
		return -1;
	}

	ep = epi->endpoint_address & 0x03;
	if (ep >= MAX_ENDPOINTS) {
		return -1;
	}

	/* Set NAK for all RX endpoints during TX */
	for (epid = 1; epid < MAX_ENDPOINTS; epid++) {

		/* Don't set NAK on DATA IN/CONTROL endpoints */
		if (ep_ref[epid].sc & USB_DIR_IN) {
			continue;
		}

		if (!(usbp->usep[epid] & (USEP_THS_NAK | USEP_RHS_NAK))) {
			unnak |= 1 << epid;
		}

		mpc8xx_udc_set_nak (epid);
	}

	mpc8xx_udc_init_tx (&udc_device->bus->endpoint_array[ep],
			    epi->tx_urb);
	ret = mpc8xx_udc_ep_tx (&udc_device->bus->endpoint_array[ep]);

	/* Remove temporary NAK */
	for (epid = 1; epid < MAX_ENDPOINTS; epid++) {
		if (unnak & (1 << epid)) {
			udc_unset_nak (epid);
		}
	}

	return ret;
}

/* mpc8xx_udc_assign_urb
 *
 * Associate a given urb to an endpoint TX or RX transmit/receive buffers
 */
static int mpc8xx_udc_assign_urb (int ep, char direction)
{
	struct usb_endpoint_instance *epi = 0;

	if (ep >= MAX_ENDPOINTS) {
		goto err;
	}
	epi = &udc_device->bus->endpoint_array[ep];
	if (!epi) {
		goto err;
	}

	if (!ep_ref[ep].urb) {
		ep_ref[ep].urb = usbd_alloc_urb (udc_device, udc_device->bus->endpoint_array);
		if (!ep_ref[ep].urb) {
			goto err;
		}
	} else {
		ep_ref[ep].urb->actual_length = 0;
	}

	switch (direction) {
	case USB_DIR_IN:
		epi->tx_urb = ep_ref[ep].urb;
		break;
	case USB_DIR_OUT:
		epi->rcv_urb = ep_ref[ep].urb;
		break;
	default:
		goto err;
	}
	return 0;

      err:
	udc_state = STATE_ERROR;
	return -1;
}

/* udc_setup_ep
 *
 * Associate U-Boot software endpoints to mpc8xx endpoint parameter ram
 * Isochronous endpoints aren't yet supported!
 */
void udc_setup_ep (struct usb_device_instance *device, unsigned int ep,
		   struct usb_endpoint_instance *epi)
{
	uchar direction = 0;
	int ep_attrib = 0;

	if (epi && (ep < MAX_ENDPOINTS)) {

		if (ep == 0) {
			if (epi->rcv_attributes != USB_ENDPOINT_XFER_CONTROL
			    || epi->tx_attributes !=
			    USB_ENDPOINT_XFER_CONTROL) {

				/* ep0 must be a control endpoint */
				udc_state = STATE_ERROR;
				return;

			}
			if (!(ep_ref[ep].sc & EP_ATTACHED)) {
				mpc8xx_udc_cbd_attach (ep, epi->tx_packetSize,
						       epi->rcv_packetSize);
			}
			usbp->usep[ep] = 0x0000;
			return;
		}

		if ((epi->endpoint_address & USB_ENDPOINT_DIR_MASK)
		    == USB_DIR_IN) {

			direction = 1;
			ep_attrib = epi->tx_attributes;
			epi->rcv_packetSize = 0;
			ep_ref[ep].sc |= USB_DIR_IN;
		} else {

			direction = 0;
			ep_attrib = epi->rcv_attributes;
			epi->tx_packetSize = 0;
			ep_ref[ep].sc &= ~USB_DIR_IN;
		}

		if (mpc8xx_udc_assign_urb (ep, epi->endpoint_address
					   & USB_ENDPOINT_DIR_MASK)) {
			return;
		}

		switch (ep_attrib) {
		case USB_ENDPOINT_XFER_CONTROL:
			if (!(ep_ref[ep].sc & EP_ATTACHED)) {
				mpc8xx_udc_cbd_attach (ep,
						       epi->tx_packetSize,
						       epi->rcv_packetSize);
			}
			usbp->usep[ep] = ep << 12;
			epi->rcv_urb = epi->tx_urb = ep_ref[ep].urb;

			break;
		case USB_ENDPOINT_XFER_BULK:
		case USB_ENDPOINT_XFER_INT:
			if (!(ep_ref[ep].sc & EP_ATTACHED)) {
				if (direction) {
					mpc8xx_udc_cbd_attach (ep,
							       epi->tx_packetSize,
							       0);
				} else {
					mpc8xx_udc_cbd_attach (ep,
							       0,
							       epi->rcv_packetSize);
				}
			}
			usbp->usep[ep] = (ep << 12) | ((ep_attrib) << 8);

			break;
		case USB_ENDPOINT_XFER_ISOC:
		default:
			serial_printf ("Error endpoint attrib %d>3\n", ep_attrib);
			udc_state = STATE_ERROR;
			break;
		}
	}

}

/* udc_connect
 *
 * Move state, switch on the USB
 */
void udc_connect (void)
{
	/* Enable pull-up resistor on D+
	 * TODO: fit a pull-up resistor to drive SE0 for > 2.5us
	 */

	if (udc_state != STATE_ERROR) {
		udc_state = STATE_READY;
		usbp->usmod |= USMOD_EN;
	}
}

/* udc_disconnect
 *
 * Disconnect is not used but, is included for completeness
 */
void udc_disconnect (void)
{
	/* Disable pull-up resistor on D-
	 * TODO: fix a pullup resistor to control this
	 */

	if (udc_state != STATE_ERROR) {
		udc_state = STATE_NOT_READY;
	}
	usbp->usmod &= ~USMOD_EN;
}

/* udc_enable
 *
 * Grab an EP0 URB, register interest in a subset of USB events
 */
void udc_enable (struct usb_device_instance *device)
{
	if (udc_state == STATE_ERROR) {
		return;
	}

	udc_device = device;

	if (!ep_ref[0].urb) {
		ep_ref[0].urb = usbd_alloc_urb (device, device->bus->endpoint_array);
	}

	/* Register interest in all events except SOF, enable transceiver */
	usbp->usber = 0x03FF;
	usbp->usbmr = 0x02F7;

	return;
}

/* udc_disable
 *
 * disable the currently hooked device
 */
void udc_disable (void)
{
	int i = 0;

	if (udc_state == STATE_ERROR) {
		DBG ("Won't disable UDC. udc_state==STATE_ERROR !\n");
		return;
	}

	udc_device = 0;

	for (; i < MAX_ENDPOINTS; i++) {
		if (ep_ref[i].urb) {
			usbd_dealloc_urb (ep_ref[i].urb);
			ep_ref[i].urb = 0;
		}
	}

	usbp->usbmr = 0x00;
	usbp->usmod = ~USMOD_EN;
	udc_state = STATE_NOT_READY;
}

/* udc_startup_events
 *
 * Enable the specified device
 */
void udc_startup_events (struct usb_device_instance *device)
{
	udc_enable (device);
	if (udc_state == STATE_READY) {
		usbd_device_event_irq (device, DEVICE_CREATE, 0);
	}
}

/* udc_set_nak
 *
 * Allow upper layers to signal lower layers should not accept more RX data
 *
 */
void udc_set_nak (int epid)
{
	if (epid) {
		mpc8xx_udc_set_nak (epid);
	}
}

/* udc_unset_nak
 *
 * Suspend sending of NAK tokens for DATA OUT tokens on a given endpoint.
 * Switch off NAKing on this endpoint to accept more data output from host.
 *
 */
void udc_unset_nak (int epid)
{
	if (epid > MAX_ENDPOINTS) {
		return;
	}

	if (usbp->usep[epid] & (USEP_THS_NAK | USEP_RHS_NAK)) {
		usbp->usep[epid] &= ~(USEP_THS_NAK | USEP_RHS_NAK);
		__asm__ ("eieio");
	}
}

/******************************************************************************
			      Static Linkage
******************************************************************************/

/* udc_state_transition_up
 * udc_state_transition_down
 *
 * Helper functions to implement device state changes.	The device states and
 * the events that transition between them are:
 *
 *				STATE_ATTACHED
 *				||	/\
 *				\/	||
 *	DEVICE_HUB_CONFIGURED			DEVICE_HUB_RESET
 *				||	/\
 *				\/	||
 *				STATE_POWERED
 *				||	/\
 *				\/	||
 *	DEVICE_RESET				DEVICE_POWER_INTERRUPTION
 *				||	/\
 *				\/	||
 *				STATE_DEFAULT
 *				||	/\
 *				\/	||
 *	DEVICE_ADDRESS_ASSIGNED			DEVICE_RESET
 *				||	/\
 *				\/	||
 *				STATE_ADDRESSED
 *				||	/\
 *				\/	||
 *	DEVICE_CONFIGURED			DEVICE_DE_CONFIGURED
 *				||	/\
 *				\/	||
 *				STATE_CONFIGURED
 *
 * udc_state_transition_up transitions up (in the direction from STATE_ATTACHED
 * to STATE_CONFIGURED) from the specified initial state to the specified final
 * state, passing through each intermediate state on the way.  If the initial
 * state is at or above (i.e. nearer to STATE_CONFIGURED) the final state, then
 * no state transitions will take place.
 *
 * udc_state_transition_down transitions down (in the direction from
 * STATE_CONFIGURED to STATE_ATTACHED) from the specified initial state to the
 * specified final state, passing through each intermediate state on the way.
 * If the initial state is at or below (i.e. nearer to STATE_ATTACHED) the final
 * state, then no state transitions will take place.
 *
 */

static void mpc8xx_udc_state_transition_up (usb_device_state_t initial,
					    usb_device_state_t final)
{
	if (initial < final) {
		switch (initial) {
		case STATE_ATTACHED:
			usbd_device_event_irq (udc_device,
					       DEVICE_HUB_CONFIGURED, 0);
			if (final == STATE_POWERED)
				break;
		case STATE_POWERED:
			usbd_device_event_irq (udc_device, DEVICE_RESET, 0);
			if (final == STATE_DEFAULT)
				break;
		case STATE_DEFAULT:
			usbd_device_event_irq (udc_device,
					       DEVICE_ADDRESS_ASSIGNED, 0);
			if (final == STATE_ADDRESSED)
				break;
		case STATE_ADDRESSED:
			usbd_device_event_irq (udc_device, DEVICE_CONFIGURED,
					       0);
		case STATE_CONFIGURED:
			break;
		default:
			break;
		}
	}
}

static void mpc8xx_udc_state_transition_down (usb_device_state_t initial,
					      usb_device_state_t final)
{
	if (initial > final) {
		switch (initial) {
		case STATE_CONFIGURED:
			usbd_device_event_irq (udc_device,
					       DEVICE_DE_CONFIGURED, 0);
			if (final == STATE_ADDRESSED)
				break;
		case STATE_ADDRESSED:
			usbd_device_event_irq (udc_device, DEVICE_RESET, 0);
			if (final == STATE_DEFAULT)
				break;
		case STATE_DEFAULT:
			usbd_device_event_irq (udc_device,
					       DEVICE_POWER_INTERRUPTION, 0);
			if (final == STATE_POWERED)
				break;
		case STATE_POWERED:
			usbd_device_event_irq (udc_device, DEVICE_HUB_RESET,
					       0);
		case STATE_ATTACHED:
			break;
		default:
			break;
		}
	}
}

/* mpc8xx_udc_stall
 *
 * Force returning of STALL tokens on the given endpoint. Protocol or function
 * STALL conditions are permissable here
 */
static void mpc8xx_udc_stall (unsigned int ep)
{
	usbp->usep[ep] |= STALL_BITMASK;
}

/* mpc8xx_udc_set_nak
 *
 * Force returning of NAK responses for the given endpoint as a kind of very
 * simple flow control
 */
static void mpc8xx_udc_set_nak (unsigned int ep)
{
	usbp->usep[ep] |= NAK_BITMASK;
	__asm__ ("eieio");
}

/* mpc8xx_udc_handle_txerr
 *
 * Handle errors relevant to TX. Return a status code to allow calling
 * indicative of what if anything happened
 */
static short mpc8xx_udc_handle_txerr ()
{
	short ep = 0, ret = 0;

	for (; ep < TX_RING_SIZE; ep++) {
		if (usbp->usber & (0x10 << ep)) {

			/* Timeout or underrun */
			if (tx_cbd[ep]->cbd_sc & 0x06) {
				ret = 1;
				mpc8xx_udc_flush_tx_fifo (ep);

			} else {
				if (usbp->usep[ep] & STALL_BITMASK) {
					if (!ep) {
						usbp->usep[ep] &= ~STALL_BITMASK;
					}
				}	/* else NAK */
			}
			usbp->usber |= (0x10 << ep);
		}
	}
	return ret;
}

/* mpc8xx_udc_advance_rx
 *
 * Advance cbd rx
 */
static void mpc8xx_udc_advance_rx (volatile cbd_t ** rx_cbdp, int epid)
{
	if ((*rx_cbdp)->cbd_sc & RX_BD_W) {
		*rx_cbdp = (volatile cbd_t *) (endpoints[epid]->rbase + CFG_IMMR);

	} else {
		(*rx_cbdp)++;
	}
}


/* mpc8xx_udc_flush_tx_fifo
 *
 * Flush a given TX fifo. Assumes one tx cbd per endpoint
 */
static void mpc8xx_udc_flush_tx_fifo (int epid)
{
	volatile cbd_t *tx_cbdp = 0;

	if (epid > MAX_ENDPOINTS) {
		return;
	}

	/* TX stop */
	immr->im_cpm.cp_cpcr = ((epid << 2) | 0x1D01);
	__asm__ ("eieio");
	while (immr->im_cpm.cp_cpcr & 0x01);

	usbp->uscom = 0x40 | 0;

	/* reset ring */
	tx_cbdp = (cbd_t *) (endpoints[epid]->tbptr + CFG_IMMR);
	tx_cbdp->cbd_sc = (TX_BD_I | TX_BD_W);


	endpoints[epid]->tptr = endpoints[epid]->tbase;
	endpoints[epid]->tstate = 0x00;
	endpoints[epid]->tbcnt = 0x00;

	/* TX start */
	immr->im_cpm.cp_cpcr = ((epid << 2) | 0x2D01);
	__asm__ ("eieio");
	while (immr->im_cpm.cp_cpcr & 0x01);

	return;
}

/* mpc8xx_udc_flush_rx_fifo
 *
 * For the sake of completeness of the namespace, it seems like
 * a good-design-decision (tm) to include mpc8xx_udc_flush_rx_fifo();
 * If RX_BD_E is true => a driver bug either here or in an upper layer
 * not polling frequently enough. If RX_BD_E is true we have told the host
 * we have accepted data but, the CPM found it had no-where to put that data
 * which needless to say would be a bad thing.
 */
static void mpc8xx_udc_flush_rx_fifo ()
{
	int i = 0;

	for (i = 0; i < RX_RING_SIZE; i++) {
		if (!(rx_cbd[i]->cbd_sc & RX_BD_E)) {
			ERR ("buf %p used rx data len = 0x%x sc=0x%x!\n",
			     rx_cbd[i], rx_cbd[i]->cbd_datlen,
			     rx_cbd[i]->cbd_sc);

		}
	}
	ERR ("BUG : Input over-run\n");
}

/* mpc8xx_udc_clear_rxbd
 *
 * Release control of RX CBD to CP.
 */
static void mpc8xx_udc_clear_rxbd (volatile cbd_t * rx_cbdp)
{
	rx_cbdp->cbd_datlen = 0x0000;
	rx_cbdp->cbd_sc = ((rx_cbdp->cbd_sc & RX_BD_W) | (RX_BD_E | RX_BD_I));
	__asm__ ("eieio");
}

/* mpc8xx_udc_tx_irq
 *
 * Parse for tx timeout, control RX or USB reset/busy conditions
 * Return -1 on timeout, -2 on fatal error, else return zero
 */
static int mpc8xx_udc_tx_irq (int ep)
{
	int i = 0;

	if (usbp->usber & (USB_TX_ERRMASK)) {
		if (mpc8xx_udc_handle_txerr ()) {
			/* Timeout, controlling function must retry send */
			return -1;
		}
	}

	if (usbp->usber & (USB_E_RESET | USB_E_BSY)) {
		/* Fatal, abandon TX transaction */
		return -2;
	}

	if (usbp->usber & USB_E_RXB) {
		for (i = 0; i < RX_RING_SIZE; i++) {
			if (!(rx_cbd[i]->cbd_sc & RX_BD_E)) {
				if ((rx_cbd[i] == ep_ref[0].prx) || ep) {
					return -2;
				}
			}
		}
	}

	return 0;
}

/* mpc8xx_udc_ep_tx
 *
 * Transmit in a re-entrant fashion outbound USB packets.
 * Implement retry/timeout mechanism described in USB specification
 * Toggle DATA0/DATA1 pids as necessary
 * Introduces non-standard tx_retry. The USB standard has no scope for slave
 * devices to give up TX, however tx_retry stops us getting stuck in an endless
 * TX loop.
 */
static int mpc8xx_udc_ep_tx (struct usb_endpoint_instance *epi)
{
	struct urb *urb = epi->tx_urb;
	volatile cbd_t *tx_cbdp = 0;
	unsigned int ep = 0, pkt_len = 0, x = 0, tx_retry = 0;
	int ret = 0;

	if (!epi || (epi->endpoint_address & 0x03) >= MAX_ENDPOINTS || !urb) {
		return -1;
	}

	ep = epi->endpoint_address & 0x03;
	tx_cbdp = (cbd_t *) (endpoints[ep]->tbptr + CFG_IMMR);

	if (tx_cbdp->cbd_sc & TX_BD_R || usbp->usber & USB_E_TXB) {
		mpc8xx_udc_flush_tx_fifo (ep);
		usbp->usber |= USB_E_TXB;
	};

	while (tx_retry++ < 100) {
		ret = mpc8xx_udc_tx_irq (ep);
		if (ret == -1) {
			/* ignore timeout here */
		} else if (ret == -2) {
			/* Abandon TX */
			mpc8xx_udc_flush_tx_fifo (ep);
			return -1;
		}

		tx_cbdp = (cbd_t *) (endpoints[ep]->tbptr + CFG_IMMR);
		while (tx_cbdp->cbd_sc & TX_BD_R) {
		};
		tx_cbdp->cbd_sc = (tx_cbdp->cbd_sc & TX_BD_W);

		pkt_len = urb->actual_length - epi->sent;

		if (pkt_len > epi->tx_packetSize || pkt_len > EP_MAX_PKT) {
			pkt_len = MIN (epi->tx_packetSize, EP_MAX_PKT);
		}

		for (x = 0; x < pkt_len; x++) {
			*((unsigned char *) (tx_cbdp->cbd_bufaddr + x)) =
				urb->buffer[epi->sent + x];
		}
		tx_cbdp->cbd_datlen = pkt_len;
		tx_cbdp->cbd_sc |= (CBD_TX_BITMASK | ep_ref[ep].pid);
		__asm__ ("eieio");

#ifdef __SIMULATE_ERROR__
		if (++err_poison_test == 2) {
			err_poison_test = 0;
			tx_cbdp->cbd_sc &= ~TX_BD_TC;
		}
#endif

		usbp->uscom = (USCOM_STR | ep);

		while (!(usbp->usber & USB_E_TXB)) {
			ret = mpc8xx_udc_tx_irq (ep);
			if (ret == -1) {
				/* TX timeout */
				break;
			} else if (ret == -2) {
				if (usbp->usber & USB_E_TXB) {
					usbp->usber |= USB_E_TXB;
				}
				mpc8xx_udc_flush_tx_fifo (ep);
				return -1;
			}
		};

		if (usbp->usber & USB_E_TXB) {
			usbp->usber |= USB_E_TXB;
		}

		/* ACK must be present <= 18bit times from TX */
		if (ret == -1) {
			continue;
		}

		/* TX ACK : USB 2.0 8.7.2, Toggle PID, Advance TX */
		epi->sent += pkt_len;
		epi->last = MIN (urb->actual_length - epi->sent, epi->tx_packetSize);
		TOGGLE_TX_PID (ep_ref[ep].pid);

		if (epi->sent >= epi->tx_urb->actual_length) {

			epi->tx_urb->actual_length = 0;
			epi->sent = 0;

			if (ep_ref[ep].sc & EP_SEND_ZLP) {
				ep_ref[ep].sc &= ~EP_SEND_ZLP;
			} else {
				return 0;
			}
		}
	}

	ERR ("TX fail, endpoint 0x%x tx bytes 0x%x/0x%x\n", ep, epi->sent,
	     epi->tx_urb->actual_length);

	return -1;
}

/* mpc8xx_udc_dump_request
 *
 * Dump a control request to console
 */
static void mpc8xx_udc_dump_request (struct usb_device_request *request)
{
	DBG ("bmRequestType:%02x bRequest:%02x wValue:%04x "
	     "wIndex:%04x wLength:%04x ?\n",
	     request->bmRequestType,
	     request->bRequest,
	     request->wValue, request->wIndex, request->wLength);

	return;
}

/* mpc8xx_udc_ep0_rx_setup
 *
 * Decode received ep0 SETUP packet. return non-zero on error
 */
static int mpc8xx_udc_ep0_rx_setup (volatile cbd_t * rx_cbdp)
{
	unsigned int x = 0;
	struct urb *purb = ep_ref[0].urb;
	struct usb_endpoint_instance *epi =
		&udc_device->bus->endpoint_array[0];

	for (; x < rx_cbdp->cbd_datlen; x++) {
		*(((unsigned char *) &ep_ref[0].urb->device_request) + x) =
			*((unsigned char *) (rx_cbdp->cbd_bufaddr + x));
	}

	mpc8xx_udc_clear_rxbd (rx_cbdp);

	if (ep0_recv_setup (purb)) {
		mpc8xx_udc_dump_request (&purb->device_request);
		return -1;
	}

	if ((purb->device_request.bmRequestType & USB_REQ_DIRECTION_MASK)
	    == USB_REQ_HOST2DEVICE) {

		switch (purb->device_request.bRequest) {
		case USB_REQ_SET_ADDRESS:
			/* Send the Status OUT ZLP */
			ep_ref[0].pid = TX_BD_PID_DATA1;
			purb->actual_length = 0;
			mpc8xx_udc_init_tx (epi, purb);
			mpc8xx_udc_ep_tx (epi);

			/* Move to the addressed state */
			usbp->usaddr = udc_device->address;
			mpc8xx_udc_state_transition_up (udc_device->device_state,
							STATE_ADDRESSED);
			return 0;

		case USB_REQ_SET_CONFIGURATION:
			if (!purb->device_request.wValue) {
				/* Respond at default address */
				usbp->usaddr = 0x00;
				mpc8xx_udc_state_transition_down (udc_device->device_state,
								  STATE_ADDRESSED);
			} else {
				/* TODO: Support multiple configurations */
				mpc8xx_udc_state_transition_up (udc_device->device_state,
								STATE_CONFIGURED);
				for (x = 1; x < MAX_ENDPOINTS; x++) {
					if ((udc_device->bus->endpoint_array[x].endpoint_address & USB_ENDPOINT_DIR_MASK)
					    == USB_DIR_IN) {
						ep_ref[x].pid = TX_BD_PID_DATA0;
					} else {
						ep_ref[x].pid = RX_BD_PID_DATA0;
					}
					/* Set configuration must unstall endpoints */
					usbp->usep[x] &= ~STALL_BITMASK;
				}
			}
			break;
		default:
			/* CDC/Vendor specific */
			break;
		}

		/* Send ZLP as ACK in Status OUT phase */
		ep_ref[0].pid = TX_BD_PID_DATA1;
		purb->actual_length = 0;
		mpc8xx_udc_init_tx (epi, purb);
		mpc8xx_udc_ep_tx (epi);

	} else {

		if (purb->actual_length) {
			ep_ref[0].pid = TX_BD_PID_DATA1;
			mpc8xx_udc_init_tx (epi, purb);

			if (!(purb->actual_length % EP0_MAX_PACKET_SIZE)) {
				ep_ref[0].sc |= EP_SEND_ZLP;
			}

			if (purb->device_request.wValue ==
			    USB_DESCRIPTOR_TYPE_DEVICE) {
				if (le16_to_cpu (purb->device_request.wLength)
				    > purb->actual_length) {
					/* Send EP0_MAX_PACKET_SIZE bytes
					 * unless correct size requested.
					 */
					if (purb->actual_length > epi->tx_packetSize) {
						purb->actual_length = epi->tx_packetSize;
					}
				}
			}
			mpc8xx_udc_ep_tx (epi);

		} else {
			/* Corrupt SETUP packet? */
			ERR ("Zero length data or SETUP with DATA-IN phase ?\n");
			return 1;
		}
	}
	return 0;
}

/* mpc8xx_udc_init_tx
 *
 * Setup some basic parameters for a TX transaction
 */
static void mpc8xx_udc_init_tx (struct usb_endpoint_instance *epi,
				struct urb *tx_urb)
{
	epi->sent = 0;
	epi->last = 0;
	epi->tx_urb = tx_urb;
}

/* mpc8xx_udc_ep0_rx
 *
 * Receive ep0/control USB data. Parse and possibly send a response.
 */
static void mpc8xx_udc_ep0_rx (volatile cbd_t * rx_cbdp)
{
	if (rx_cbdp->cbd_sc & RX_BD_PID_SETUP) {

		/* Unconditionally accept SETUP packets */
		if (mpc8xx_udc_ep0_rx_setup (rx_cbdp)) {
			mpc8xx_udc_stall (0);
		}

	} else {

		mpc8xx_udc_clear_rxbd (rx_cbdp);

		if ((rx_cbdp->cbd_datlen - 2)) {
			/* SETUP with a DATA phase
			 * outside of SETUP packet.
			 * Reply with STALL.
			 */
			mpc8xx_udc_stall (0);
		}
	}
}

/* mpc8xx_udc_epn_rx
 *
 * Receive some data from cbd into USB system urb data abstraction
 * Upper layers should NAK if there is insufficient RX data space
 */
static int mpc8xx_udc_epn_rx (unsigned int epid, volatile cbd_t * rx_cbdp)
{
	struct usb_endpoint_instance *epi = 0;
	struct urb *urb = 0;
	unsigned int x = 0;

	if (epid >= MAX_ENDPOINTS || !rx_cbdp->cbd_datlen) {
		return 0;
	}

	/* USB 2.0 PDF section 8.6.4
	 * Discard data with invalid PID it is a resend.
	 */
	if (ep_ref[epid].pid != (rx_cbdp->cbd_sc & 0xC0)) {
		return 1;
	}
	TOGGLE_RX_PID (ep_ref[epid].pid);

	epi = &udc_device->bus->endpoint_array[epid];
	urb = epi->rcv_urb;

	for (; x < (rx_cbdp->cbd_datlen - 2); x++) {
		*((unsigned char *) (urb->buffer + urb->actual_length + x)) =
			*((unsigned char *) (rx_cbdp->cbd_bufaddr + x));
	}

	if (x) {
		usbd_rcv_complete (epi, x, 0);
		if (ep_ref[epid].urb->status == RECV_ERROR) {
			DBG ("RX error unset NAK\n");
			udc_unset_nak (epid);
		}
	}
	return x;
}

/* mpc8xx_udc_clock_init
 *
 * Obtain a clock reference for Full Speed Signaling
 */
static void mpc8xx_udc_clock_init (volatile immap_t * immr,
				   volatile cpm8xx_t * cp)
{

#if defined(CFG_USB_EXTC_CLK)

	/* This has been tested with a 48MHz crystal on CLK6 */
	switch (CFG_USB_EXTC_CLK) {
	case 1:
		immr->im_ioport.iop_papar |= 0x0100;
		immr->im_ioport.iop_padir &= ~0x0100;
		cp->cp_sicr |= 0x24;
		break;
	case 2:
		immr->im_ioport.iop_papar |= 0x0200;
		immr->im_ioport.iop_padir &= ~0x0200;
		cp->cp_sicr |= 0x2D;
		break;
	case 3:
		immr->im_ioport.iop_papar |= 0x0400;
		immr->im_ioport.iop_padir &= ~0x0400;
		cp->cp_sicr |= 0x36;
		break;
	case 4:
		immr->im_ioport.iop_papar |= 0x0800;
		immr->im_ioport.iop_padir &= ~0x0800;
		cp->cp_sicr |= 0x3F;
		break;
	default:
		udc_state = STATE_ERROR;
		break;
	}

#elif defined(CFG_USB_BRGCLK)

	/* This has been tested with brgclk == 50MHz */
	DECLARE_GLOBAL_DATA_PTR;
	int divisor = 0;

	if (gd->cpu_clk < 48000000L) {
		ERR ("brgclk is too slow for full-speed USB!\n");
		udc_state = STATE_ERROR;
		return;
	}

	/* Assume the brgclk is 'good enough', we want !(gd->cpu_clk%48Mhz)
	 * but, can /probably/ live with close-ish alternative rates.
	 */
	divisor = (gd->cpu_clk / 48000000L) - 1;
	cp->cp_sicr &= ~0x0000003F;

	switch (CFG_USB_BRGCLK) {
	case 1:
		cp->cp_brgc1 |= (divisor | CPM_BRG_EN);
		cp->cp_sicr &= ~0x2F;
		break;
	case 2:
		cp->cp_brgc2 |= (divisor | CPM_BRG_EN);
		cp->cp_sicr |= 0x00000009;
		break;
	case 3:
		cp->cp_brgc3 |= (divisor | CPM_BRG_EN);
		cp->cp_sicr |= 0x00000012;
		break;
	case 4:
		cp->cp_brgc4 = (divisor | CPM_BRG_EN);
		cp->cp_sicr |= 0x0000001B;
		break;
	default:
		udc_state = STATE_ERROR;
		break;
	}

#else
#error "CFG_USB_EXTC_CLK or CFG_USB_BRGCLK must be defined"
#endif

}

/* mpc8xx_udc_cbd_attach
 *
 * attach a cbd to and endpoint
 */
static void mpc8xx_udc_cbd_attach (int ep, uchar tx_size, uchar rx_size)
{

	if (!tx_cbd[ep] || !rx_cbd[ep] || ep >= MAX_ENDPOINTS) {
		udc_state = STATE_ERROR;
		return;
	}

	if (tx_size > USB_MAX_PKT || rx_size > USB_MAX_PKT ||
	    (!tx_size && !rx_size)) {
		udc_state = STATE_ERROR;
		return;
	}

	/* Attach CBD to appropiate Parameter RAM Endpoint data structure */
	if (rx_size) {
		endpoints[ep]->rbase = (u32) rx_cbd[rx_ct];
		endpoints[ep]->rbptr = (u32) rx_cbd[rx_ct];
		rx_ct++;

		if (!ep) {

			endpoints[ep]->rbptr = (u32) rx_cbd[rx_ct];
			rx_cbd[rx_ct]->cbd_sc |= RX_BD_W;
			rx_ct++;

		} else {
			rx_ct += 2;
			endpoints[ep]->rbptr = (u32) rx_cbd[rx_ct];
			rx_cbd[rx_ct]->cbd_sc |= RX_BD_W;
			rx_ct++;
		}

		/* Where we expect to RX data on this endpoint */
		ep_ref[ep].prx = rx_cbd[rx_ct - 1];
	} else {

		ep_ref[ep].prx = 0;
		endpoints[ep]->rbase = 0;
		endpoints[ep]->rbptr = 0;
	}

	if (tx_size) {
		endpoints[ep]->tbase = (u32) tx_cbd[tx_ct];
		endpoints[ep]->tbptr = (u32) tx_cbd[tx_ct];
		tx_ct++;
	} else {
		endpoints[ep]->tbase = 0;
		endpoints[ep]->tbptr = 0;
	}

	endpoints[ep]->tstate = 0;
	endpoints[ep]->tbcnt = 0;
	endpoints[ep]->mrblr = EP_MAX_PKT;
	endpoints[ep]->rfcr = 0x18;
	endpoints[ep]->tfcr = 0x18;
	ep_ref[ep].sc |= EP_ATTACHED;

	DBG ("ep %d rbase 0x%08x rbptr 0x%08x tbase 0x%08x tbptr 0x%08x prx = %p\n",
		ep, endpoints[ep]->rbase, endpoints[ep]->rbptr,
		endpoints[ep]->tbase, endpoints[ep]->tbptr,
		ep_ref[ep].prx);

	return;
}

/* mpc8xx_udc_cbd_init
 *
 * Allocate space for a cbd and allocate TX/RX data space
 */
static void mpc8xx_udc_cbd_init (void)
{
	int i = 0;

	for (; i < TX_RING_SIZE; i++) {
		tx_cbd[i] = (cbd_t *)
			mpc8xx_udc_alloc (sizeof (cbd_t), sizeof (int));
	}

	for (i = 0; i < RX_RING_SIZE; i++) {
		rx_cbd[i] = (cbd_t *)
			mpc8xx_udc_alloc (sizeof (cbd_t), sizeof (int));
	}

	for (i = 0; i < TX_RING_SIZE; i++) {
		tx_cbd[i]->cbd_bufaddr =
			mpc8xx_udc_alloc (EP_MAX_PKT, sizeof (int));

		tx_cbd[i]->cbd_sc = (TX_BD_I | TX_BD_W);
		tx_cbd[i]->cbd_datlen = 0x0000;
	}


	for (i = 0; i < RX_RING_SIZE; i++) {
		rx_cbd[i]->cbd_bufaddr =
			mpc8xx_udc_alloc (EP_MAX_PKT, sizeof (int));
		rx_cbd[i]->cbd_sc = (RX_BD_I | RX_BD_E);
		rx_cbd[i]->cbd_datlen = 0x0000;

	}

	return;
}

/* mpc8xx_udc_endpoint_init
 *
 * Attach an endpoint to some dpram
 */
static void mpc8xx_udc_endpoint_init (void)
{
	int i = 0;

	for (; i < MAX_ENDPOINTS; i++) {
		endpoints[i] = (usb_epb_t *)
			mpc8xx_udc_alloc (sizeof (usb_epb_t), 32);
	}
}

/* mpc8xx_udc_alloc
 *
 * Grab the address of some dpram
 */
static u32 mpc8xx_udc_alloc (u32 data_size, u32 alignment)
{
	u32 retaddr = address_base;

	while (retaddr % alignment) {
		retaddr++;
	}
	address_base += data_size;

	return retaddr;
}

#endif /* CONFIG_MPC885_FAMILY && CONFIG_USB_DEVICE) */
