/*
 * (C) Copyright 2003
 * Gerry Hamel, geh@ti.com, Texas Instruments
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	 USA
 *
 */

#include <common.h>

#ifdef CONFIG_USB_TTY

#include <circbuf.h>
#include <devices.h>
#include "usbtty.h"

#if 0
#define TTYDBG(fmt,args...) serial_printf("[%s] %s %d: "fmt, __FILE__,__FUNCTION__,__LINE__,##args)
#else
#define TTYDBG(fmt,args...) do{}while(0)
#endif

#if 0
#define TTYERR(fmt,args...) serial_printf("ERROR![%s] %s %d: "fmt, __FILE__,__FUNCTION__,__LINE__,##args)
#else
#define TTYERR(fmt,args...) do{}while(0)
#endif

/*
 * Buffers to hold input and output data
 */
#define USBTTY_BUFFER_SIZE 256
static circbuf_t usbtty_input;
static circbuf_t usbtty_output;


/*
 * Instance variables
 */
static device_t usbttydev;
static struct usb_device_instance	 device_instance[1];
static struct usb_bus_instance		 bus_instance[1];
static struct usb_configuration_instance config_instance[NUM_CONFIGS];
static struct usb_interface_instance	 interface_instance[NUM_INTERFACES];
static struct usb_alternate_instance	 alternate_instance[NUM_INTERFACES];
static struct usb_endpoint_instance	 endpoint_instance[NUM_ENDPOINTS+1]; /* one extra for control endpoint */

/*
 * Static allocation of urbs
 */
#define RECV_ENDPOINT 1
#define TX_ENDPOINT 2

/*
 * Global flag
 */
int usbtty_configured_flag = 0;


/*
 * Serial number
 */
static char serial_number[16];

/*
 * Descriptors
 */
static u8 wstrLang[4] = {4,USB_DT_STRING,0x9,0x4};
static u8 wstrManufacturer[2 + 2*(sizeof(CONFIG_USBD_MANUFACTURER)-1)];
static u8 wstrProduct[2 + 2*(sizeof(CONFIG_USBD_PRODUCT_NAME)-1)];
static u8 wstrSerial[2 + 2*(sizeof(serial_number) - 1)];
static u8 wstrConfiguration[2 + 2*(sizeof(CONFIG_USBD_CONFIGURATION_STR)-1)];
static u8 wstrInterface[2 + 2*(sizeof(CONFIG_USBD_INTERFACE_STR)-1)];

static struct usb_string_descriptor *usbtty_string_table[] = {
  (struct usb_string_descriptor*)wstrLang,
  (struct usb_string_descriptor*)wstrManufacturer,
  (struct usb_string_descriptor*)wstrProduct,
  (struct usb_string_descriptor*)wstrSerial,
  (struct usb_string_descriptor*)wstrConfiguration,
  (struct usb_string_descriptor*)wstrInterface
};
extern struct usb_string_descriptor **usb_strings; /* defined and used by omap1510_ep0.c */

static struct usb_device_descriptor device_descriptor = {
  bLength:	      sizeof(struct usb_device_descriptor),
  bDescriptorType:    USB_DT_DEVICE,
  bcdUSB:	      USB_BCD_VERSION,
  bDeviceClass:	      USBTTY_DEVICE_CLASS,
  bDeviceSubClass:    USBTTY_DEVICE_SUBCLASS,
  bDeviceProtocol:    USBTTY_DEVICE_PROTOCOL,
  bMaxPacketSize0:    EP0_MAX_PACKET_SIZE,
  idVendor:	      CONFIG_USBD_VENDORID,
  idProduct:	      CONFIG_USBD_PRODUCTID,
  bcdDevice:	      USBTTY_BCD_DEVICE,
  iManufacturer:      STR_MANUFACTURER,
  iProduct:	      STR_PRODUCT,
  iSerialNumber:      STR_SERIAL,
  bNumConfigurations: NUM_CONFIGS
  };
static struct usb_configuration_descriptor config_descriptors[NUM_CONFIGS] = {
  {
    bLength:		 sizeof(struct usb_configuration_descriptor),
    bDescriptorType:	 USB_DT_CONFIG,
    wTotalLength:	 (sizeof(struct usb_configuration_descriptor)*NUM_CONFIGS) +
			 (sizeof(struct usb_interface_descriptor)*NUM_INTERFACES) +
			 (sizeof(struct usb_endpoint_descriptor)*NUM_ENDPOINTS),
    bNumInterfaces:	 NUM_INTERFACES,
    bConfigurationValue: 1,
    iConfiguration:	 STR_CONFIG,
    bmAttributes:	 BMATTRIBUTE_SELF_POWERED | BMATTRIBUTE_RESERVED,
    bMaxPower:		 USBTTY_MAXPOWER
  },
};
static struct usb_interface_descriptor interface_descriptors[NUM_INTERFACES] = {
  {
    bLength:		 sizeof(struct usb_interface_descriptor),
    bDescriptorType:	 USB_DT_INTERFACE,
    bInterfaceNumber:	 0,
    bAlternateSetting:	 0,
    bNumEndpoints:	 NUM_ENDPOINTS,
    bInterfaceClass:	 USBTTY_INTERFACE_CLASS,
    bInterfaceSubClass:	 USBTTY_INTERFACE_SUBCLASS,
    bInterfaceProtocol:	 USBTTY_INTERFACE_PROTOCOL,
    iInterface:		 STR_INTERFACE
  },
};
static struct usb_endpoint_descriptor ep_descriptors[NUM_ENDPOINTS] = {
  {
    bLength:		 sizeof(struct usb_endpoint_descriptor),
    bDescriptorType:	 USB_DT_ENDPOINT,
    bEndpointAddress:	 CONFIG_USBD_SERIAL_OUT_ENDPOINT | USB_DIR_OUT,
    bmAttributes:	 USB_ENDPOINT_XFER_BULK,
    wMaxPacketSize:	 CONFIG_USBD_SERIAL_OUT_PKTSIZE,
    bInterval:		 0
  },
  {
    bLength:		 sizeof(struct usb_endpoint_descriptor),
    bDescriptorType:	 USB_DT_ENDPOINT,
    bEndpointAddress:	 CONFIG_USBD_SERIAL_IN_ENDPOINT | USB_DIR_IN,
    bmAttributes:	 USB_ENDPOINT_XFER_BULK,
    wMaxPacketSize:	 CONFIG_USBD_SERIAL_IN_PKTSIZE,
    bInterval:		 0
  },
  {
    bLength:		 sizeof(struct usb_endpoint_descriptor),
    bDescriptorType:	 USB_DT_ENDPOINT,
    bEndpointAddress:	 CONFIG_USBD_SERIAL_INT_ENDPOINT | USB_DIR_IN,
    bmAttributes:	 USB_ENDPOINT_XFER_INT,
    wMaxPacketSize:	 CONFIG_USBD_SERIAL_INT_PKTSIZE,
    bInterval:		 0
  },
};
static struct usb_endpoint_descriptor *ep_descriptor_ptrs[NUM_ENDPOINTS] = {
  &(ep_descriptors[0]),
  &(ep_descriptors[1]),
  &(ep_descriptors[2]),
};

/* utility function for converting char* to wide string used by USB */
static void str2wide (char *str, u16 * wide)
{
	int i;

	for (i = 0; i < strlen (str) && str[i]; i++)
		wide[i] = (u16) str[i];
}

/*
 * Prototypes
 */
static void usbtty_init_strings (void);
static void usbtty_init_instances (void);
static void usbtty_init_endpoints (void);

static void usbtty_event_handler (struct usb_device_instance *device,
				  usb_device_event_t event, int data);
static int usbtty_configured (void);

static int write_buffer (circbuf_t * buf);
static int fill_buffer (circbuf_t * buf);

void usbtty_poll (void);
static void pretend_interrupts (void);


/*
 * Test whether a character is in the RX buffer
 */
int usbtty_tstc (void)
{
	usbtty_poll ();
	return (usbtty_input.size > 0);
}

/*
 * Read a single byte from the usb client port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int usbtty_getc (void)
{
	char c;

	while (usbtty_input.size <= 0) {
		usbtty_poll ();
	}

	buf_pop (&usbtty_input, &c, 1);
	return c;
}

/*
 * Output a single byte to the usb client port.
 */
void usbtty_putc (const char c)
{
	buf_push (&usbtty_output, &c, 1);
	/* If \n, also do \r */
	if (c == '\n')
		buf_push (&usbtty_output, "\r", 1);

	/* Poll at end to handle new data... */
	if ((usbtty_output.size + 2) >= usbtty_output.totalsize) {
		usbtty_poll ();
	}
}


/* usbtty_puts() helper function for finding the next '\n' in a string */
static int next_nl_pos (const char *s)
{
	int i;

	for (i = 0; s[i] != '\0'; i++) {
		if (s[i] == '\n')
			return i;
	}
	return i;
}

/*
 * Output a string to the usb client port.
 */
static void __usbtty_puts (const char *str, int len)
{
	int maxlen = usbtty_output.totalsize;
	int space, n;

	/* break str into chunks < buffer size, if needed */
	while (len > 0) {
		space = maxlen - usbtty_output.size;

		/* Empty buffer here, if needed, to ensure space... */
		if (space <= 0) {
			write_buffer (&usbtty_output);
			space = maxlen - usbtty_output.size;
			if (space <= 0) {
				space = len;	/* allow old data to be overwritten. */
			}
		}

		n = MIN (space, MIN (len, maxlen));
		buf_push (&usbtty_output, str, n);

		str += n;
		len -= n;
	}
}

void usbtty_puts (const char *str)
{
	int n;
	int len = strlen (str);

	/* add '\r' for each '\n' */
	while (len > 0) {
		n = next_nl_pos (str);

		if (str[n] == '\n') {
			__usbtty_puts (str, n + 1);
			__usbtty_puts ("\r", 1);
			str += (n + 1);
			len -= (n + 1);
		} else {
			/* No \n found.	 All done. */
			__usbtty_puts (str, n);
			break;
		}
	}

	/* Poll at end to handle new data... */
	usbtty_poll ();
}

/*
 * Initialize the usb client port.
 *
 */
int drv_usbtty_init (void)
{
	int rc;
	char * sn;
	int snlen;

	if (!(sn = getenv("serial#"))) {
		sn = "000000000000";
	}
	snlen = strlen(sn);
	if (snlen > sizeof(serial_number) - 1) {
		printf ("Warning: serial number %s is too long (%d > %d)\n",
			sn, snlen, sizeof(serial_number) - 1);
		snlen = sizeof(serial_number) - 1;
	}
	memcpy (serial_number, sn, snlen);
	serial_number[snlen] = '\0';

	/* prepare buffers... */
	buf_init (&usbtty_input, USBTTY_BUFFER_SIZE);
	buf_init (&usbtty_output, USBTTY_BUFFER_SIZE);

	/* Now, set up USB controller and infrastructure */
	udc_init ();		/* Basic USB initialization */

	usbtty_init_strings ();
	usbtty_init_instances ();

	udc_startup_events (device_instance);	/* Enable our device, initialize udc pointers */
	udc_connect ();		/* Enable pullup for host detection */

	usbtty_init_endpoints ();

	/* Device initialization */
	memset (&usbttydev, 0, sizeof (usbttydev));

	strcpy (usbttydev.name, "usbtty");
	usbttydev.ext = 0;	/* No extensions */
	usbttydev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_OUTPUT;
	usbttydev.tstc = usbtty_tstc;	/* 'tstc' function */
	usbttydev.getc = usbtty_getc;	/* 'getc' function */
	usbttydev.putc = usbtty_putc;	/* 'putc' function */
	usbttydev.puts = usbtty_puts;	/* 'puts' function */

	rc = device_register (&usbttydev);

	return (rc == 0) ? 1 : rc;
}

static void usbtty_init_strings (void)
{
	struct usb_string_descriptor *string;

	string = (struct usb_string_descriptor *) wstrManufacturer;
	string->bLength = sizeof (wstrManufacturer);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_MANUFACTURER, string->wData);

	string = (struct usb_string_descriptor *) wstrProduct;
	string->bLength = sizeof (wstrProduct);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_PRODUCT_NAME, string->wData);

	string = (struct usb_string_descriptor *) wstrSerial;
	string->bLength = 2 + 2*strlen(serial_number);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (serial_number, string->wData);

	string = (struct usb_string_descriptor *) wstrConfiguration;
	string->bLength = sizeof (wstrConfiguration);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_CONFIGURATION_STR, string->wData);

	string = (struct usb_string_descriptor *) wstrInterface;
	string->bLength = sizeof (wstrInterface);
	string->bDescriptorType = USB_DT_STRING;
	str2wide (CONFIG_USBD_INTERFACE_STR, string->wData);

	/* Now, initialize the string table for ep0 handling */
	usb_strings = usbtty_string_table;
}

static void usbtty_init_instances (void)
{
	int i;

	/* initialize device instance */
	memset (device_instance, 0, sizeof (struct usb_device_instance));
	device_instance->device_state = STATE_INIT;
	device_instance->device_descriptor = &device_descriptor;
	device_instance->event = usbtty_event_handler;
	device_instance->bus = bus_instance;
	device_instance->configurations = NUM_CONFIGS;
	device_instance->configuration_instance_array = config_instance;

	/* initialize bus instance */
	memset (bus_instance, 0, sizeof (struct usb_bus_instance));
	bus_instance->device = device_instance;
	bus_instance->endpoint_array = endpoint_instance;
	bus_instance->max_endpoints = 1;
	bus_instance->maxpacketsize = 64;
	bus_instance->serial_number_str = serial_number;

	/* configuration instance */
	memset (config_instance, 0,
		sizeof (struct usb_configuration_instance));
	config_instance->interfaces = NUM_INTERFACES;
	config_instance->configuration_descriptor = config_descriptors;
	config_instance->interface_instance_array = interface_instance;

	/* interface instance */
	memset (interface_instance, 0,
		sizeof (struct usb_interface_instance));
	interface_instance->alternates = 1;
	interface_instance->alternates_instance_array = alternate_instance;

	/* alternates instance */
	memset (alternate_instance, 0,
		sizeof (struct usb_alternate_instance));
	alternate_instance->interface_descriptor = interface_descriptors;
	alternate_instance->endpoints = NUM_ENDPOINTS;
	alternate_instance->endpoints_descriptor_array = ep_descriptor_ptrs;

	/* endpoint instances */
	memset (&endpoint_instance[0], 0,
		sizeof (struct usb_endpoint_instance));
	endpoint_instance[0].endpoint_address = 0;
	endpoint_instance[0].rcv_packetSize = EP0_MAX_PACKET_SIZE;
	endpoint_instance[0].rcv_attributes = USB_ENDPOINT_XFER_CONTROL;
	endpoint_instance[0].tx_packetSize = EP0_MAX_PACKET_SIZE;
	endpoint_instance[0].tx_attributes = USB_ENDPOINT_XFER_CONTROL;
	udc_setup_ep (device_instance, 0, &endpoint_instance[0]);

	for (i = 1; i <= NUM_ENDPOINTS; i++) {
		memset (&endpoint_instance[i], 0,
			sizeof (struct usb_endpoint_instance));

		endpoint_instance[i].endpoint_address =
			ep_descriptors[i - 1].bEndpointAddress;

		endpoint_instance[i].rcv_packetSize =
			ep_descriptors[i - 1].wMaxPacketSize;
		endpoint_instance[i].rcv_attributes =
			ep_descriptors[i - 1].bmAttributes;

		endpoint_instance[i].tx_packetSize =
			ep_descriptors[i - 1].wMaxPacketSize;
		endpoint_instance[i].tx_attributes =
			ep_descriptors[i - 1].bmAttributes;

		urb_link_init (&endpoint_instance[i].rcv);
		urb_link_init (&endpoint_instance[i].rdy);
		urb_link_init (&endpoint_instance[i].tx);
		urb_link_init (&endpoint_instance[i].done);

		if (endpoint_instance[i].endpoint_address & USB_DIR_IN)
			endpoint_instance[i].tx_urb =
				usbd_alloc_urb (device_instance,
						&endpoint_instance[i]);
		else
			endpoint_instance[i].rcv_urb =
				usbd_alloc_urb (device_instance,
						&endpoint_instance[i]);
	}
}

static void usbtty_init_endpoints (void)
{
	int i;

	bus_instance->max_endpoints = NUM_ENDPOINTS + 1;
	for (i = 0; i <= NUM_ENDPOINTS; i++) {
		udc_setup_ep (device_instance, i, &endpoint_instance[i]);
	}
}


/*********************************************************************************/

static struct urb *next_urb (struct usb_device_instance *device,
			     struct usb_endpoint_instance *endpoint)
{
	struct urb *current_urb = NULL;
	int space;

	/* If there's a queue, then we should add to the last urb */
	if (!endpoint->tx_queue) {
		current_urb = endpoint->tx_urb;
	} else {
		/* Last urb from tx chain */
		current_urb =
			p2surround (struct urb, link, endpoint->tx.prev);
	}

	/* Make sure this one has enough room */
	space = current_urb->buffer_length - current_urb->actual_length;
	if (space > 0) {
		return current_urb;
	} else {		/* No space here */
		/* First look at done list */
		current_urb = first_urb_detached (&endpoint->done);
		if (!current_urb) {
			current_urb = usbd_alloc_urb (device, endpoint);
		}

		urb_append (&endpoint->tx, current_urb);
		endpoint->tx_queue++;
	}
	return current_urb;
}

static int write_buffer (circbuf_t * buf)
{
	if (!usbtty_configured ()) {
		return 0;
	}

	if (buf->size) {

		struct usb_endpoint_instance *endpoint =
			&endpoint_instance[TX_ENDPOINT];
		struct urb *current_urb = NULL;
		char *dest;

		int space_avail;
		int popnum, popped;
		int total = 0;

		/* Break buffer into urb sized pieces, and link each to the endpoint */
		while (buf->size > 0) {
			current_urb = next_urb (device_instance, endpoint);
			if (!current_urb) {
				TTYERR ("current_urb is NULL, buf->size %d\n",
					buf->size);
				return total;
			}

			dest = current_urb->buffer +
				current_urb->actual_length;

			space_avail =
				current_urb->buffer_length -
				current_urb->actual_length;
			popnum = MIN (space_avail, buf->size);
			if (popnum == 0)
				break;

			popped = buf_pop (buf, dest, popnum);
			if (popped == 0)
				break;
			current_urb->actual_length += popped;
			total += popped;

			/* If endpoint->last == 0, then transfers have not started on this endpoint */
			if (endpoint->last == 0) {
				udc_endpoint_write (endpoint);
			}

		}		/* end while */
		return total;
	}			/* end if tx_urb */

	return 0;
}

static int fill_buffer (circbuf_t * buf)
{
	struct usb_endpoint_instance *endpoint =
		&endpoint_instance[RECV_ENDPOINT];

	if (endpoint->rcv_urb && endpoint->rcv_urb->actual_length) {
		unsigned int nb = endpoint->rcv_urb->actual_length;
		char *src = (char *) endpoint->rcv_urb->buffer;

		buf_push (buf, src, nb);
		endpoint->rcv_urb->actual_length = 0;

		return nb;
	}

	return 0;
}

static int usbtty_configured (void)
{
	return usbtty_configured_flag;
}

/*********************************************************************************/

static void usbtty_event_handler (struct usb_device_instance *device,
				  usb_device_event_t event, int data)
{
	switch (event) {
	case DEVICE_RESET:
	case DEVICE_BUS_INACTIVE:
		usbtty_configured_flag = 0;
		break;
	case DEVICE_CONFIGURED:
		usbtty_configured_flag = 1;
		break;

	case DEVICE_ADDRESS_ASSIGNED:
		usbtty_init_endpoints ();

	default:
		break;
	}
}

/*********************************************************************************/


/*
 * Since interrupt handling has not yet been implemented, we use this function
 * to handle polling.  This is called by the tstc,getc,putc,puts routines to
 * update the USB state.
 */
void usbtty_poll (void)
{
	/* New interrupts? */
	pretend_interrupts ();

	/* Write any output data to host buffer (do this before checking interrupts to avoid missing one) */
	if (usbtty_configured ()) {
		write_buffer (&usbtty_output);
	}

	/* New interrupts? */
	pretend_interrupts ();

	/* Check for new data from host.. (do this after checking interrupts to get latest data) */
	if (usbtty_configured ()) {
		fill_buffer (&usbtty_input);
	}

	/* New interrupts? */
	pretend_interrupts ();
}

static void pretend_interrupts (void)
{
	/* Loop while we have interrupts.
	 * If we don't do this, the input chain
	 * polling delay is likely to miss
	 * host requests.
	 */
	while (inw (UDC_IRQ_SRC) & ~UDC_SOF_Flg) {
		/* Handle any new IRQs */
		omap1510_udc_irq ();
		omap1510_udc_noniso_irq ();
	}
}
#endif
