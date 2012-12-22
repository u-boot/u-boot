/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Part of this source has been derived from the Linux USB
 * project.
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
 *
 */
#include <common.h>
#include <malloc.h>
#include <stdio_dev.h>
#include <asm/byteorder.h>

#include <usb.h>

#ifdef	USB_KBD_DEBUG
#define USB_KBD_PRINTF(fmt, args...)	printf(fmt, ##args)
#else
#define USB_KBD_PRINTF(fmt, args...)
#endif

/*
 * If overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console(void);
#else
int overwrite_console(void)
{
	return 0;
}
#endif

/* Keyboard sampling rate */
#define REPEAT_RATE	(40 / 4)	/* 40msec -> 25cps */
#define REPEAT_DELAY	10		/* 10 x REPEAT_RATE = 400msec */

#define NUM_LOCK	0x53
#define CAPS_LOCK	0x39
#define SCROLL_LOCK	0x47

/* Modifier bits */
#define LEFT_CNTR	(1 << 0)
#define LEFT_SHIFT	(1 << 1)
#define LEFT_ALT	(1 << 2)
#define LEFT_GUI	(1 << 3)
#define RIGHT_CNTR	(1 << 4)
#define RIGHT_SHIFT	(1 << 5)
#define RIGHT_ALT	(1 << 6)
#define RIGHT_GUI	(1 << 7)

/* Size of the keyboard buffer */
#define USB_KBD_BUFFER_LEN	0x20

/* Device name */
#define DEVNAME			"usbkbd"

/* Keyboard maps */
static const unsigned char usb_kbd_numkey[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'\r', 0x1b, '\b', '\t', ' ', '-', '=', '[', ']',
	'\\', '#', ';', '\'', '`', ',', '.', '/'
};
static const unsigned char usb_kbd_numkey_shifted[] = {
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'\r', 0x1b, '\b', '\t', ' ', '_', '+', '{', '}',
	'|', '~', ':', '"', '~', '<', '>', '?'
};

static const unsigned char usb_kbd_num_keypad[] = {
	'/', '*', '-', '+', '\r',
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'.', 0, 0, 0, '='
};

/*
 * map arrow keys to ^F/^B ^N/^P, can't really use the proper
 * ANSI sequence for arrow keys because the queuing code breaks
 * when a single keypress expands to 3 queue elements
 */
static const unsigned char usb_kbd_arrow[] = {
	0x6, 0x2, 0xe, 0x10
};

/*
 * NOTE: It's important for the NUM, CAPS, SCROLL-lock bits to be in this
 *       order. See usb_kbd_setled() function!
 */
#define USB_KBD_NUMLOCK		(1 << 0)
#define USB_KBD_CAPSLOCK	(1 << 1)
#define USB_KBD_SCROLLLOCK	(1 << 2)
#define USB_KBD_CTRL		(1 << 3)

#define USB_KBD_LEDMASK		\
	(USB_KBD_NUMLOCK | USB_KBD_CAPSLOCK | USB_KBD_SCROLLLOCK)

struct usb_kbd_pdata {
	uint32_t	repeat_delay;

	uint32_t	usb_in_pointer;
	uint32_t	usb_out_pointer;
	uint8_t		usb_kbd_buffer[USB_KBD_BUFFER_LEN];

	uint8_t		*new;
	uint8_t		old[8];

	uint8_t		flags;
};

/* Generic keyboard event polling. */
void usb_kbd_generic_poll(void)
{
	struct stdio_dev *dev;
	struct usb_device *usb_kbd_dev;
	struct usb_kbd_pdata *data;
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep;
	int pipe;
	int maxp;

	/* Get the pointer to USB Keyboard device pointer */
	dev = stdio_get_by_name(DEVNAME);
	usb_kbd_dev = (struct usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;
	iface = &usb_kbd_dev->config.if_desc[0];
	ep = &iface->ep_desc[0];
	pipe = usb_rcvintpipe(usb_kbd_dev, ep->bEndpointAddress);

	/* Submit a interrupt transfer request */
	maxp = usb_maxpacket(usb_kbd_dev, pipe);
	usb_submit_int_msg(usb_kbd_dev, pipe, data->new,
			maxp > 8 ? 8 : maxp, ep->bInterval);
}

/* Puts character in the queue and sets up the in and out pointer. */
static void usb_kbd_put_queue(struct usb_kbd_pdata *data, char c)
{
	if (data->usb_in_pointer == USB_KBD_BUFFER_LEN - 1) {
		/* Check for buffer full. */
		if (data->usb_out_pointer == 0)
			return;

		data->usb_in_pointer = 0;
	} else {
		/* Check for buffer full. */
		if (data->usb_in_pointer == data->usb_out_pointer - 1)
			return;

		data->usb_in_pointer++;
	}

	data->usb_kbd_buffer[data->usb_in_pointer] = c;
}

/*
 * Set the LEDs. Since this is used in the irq routine, the control job is
 * issued with a timeout of 0. This means, that the job is queued without
 * waiting for job completion.
 */
static void usb_kbd_setled(struct usb_device *dev)
{
	struct usb_interface *iface = &dev->config.if_desc[0];
	struct usb_kbd_pdata *data = dev->privptr;
	uint32_t leds = data->flags & USB_KBD_LEDMASK;

	usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
		USB_REQ_SET_REPORT, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0x200, iface->desc.bInterfaceNumber, (void *)&leds, 1, 0);
}

#define CAPITAL_MASK	0x20
/* Translate the scancode in ASCII */
static int usb_kbd_translate(struct usb_kbd_pdata *data, unsigned char scancode,
				unsigned char modifier, int pressed)
{
	uint8_t keycode = 0;

	/* Key released */
	if (pressed == 0) {
		data->repeat_delay = 0;
		return 0;
	}

	if (pressed == 2) {
		data->repeat_delay++;
		if (data->repeat_delay < REPEAT_DELAY)
			return 0;

		data->repeat_delay = REPEAT_DELAY;
	}

	/* Alphanumeric values */
	if ((scancode > 3) && (scancode <= 0x1d)) {
		keycode = scancode - 4 + 'a';

		if (data->flags & USB_KBD_CAPSLOCK)
			keycode &= ~CAPITAL_MASK;

		if (modifier & (LEFT_SHIFT | RIGHT_SHIFT)) {
			/* Handle CAPSLock + Shift pressed simultaneously */
			if (keycode & CAPITAL_MASK)
				keycode &= ~CAPITAL_MASK;
			else
				keycode |= CAPITAL_MASK;
		}
	}

	if ((scancode > 0x1d) && (scancode < 0x3a)) {
		/* Shift pressed */
		if (modifier & (LEFT_SHIFT | RIGHT_SHIFT))
			keycode = usb_kbd_numkey_shifted[scancode - 0x1e];
		else
			keycode = usb_kbd_numkey[scancode - 0x1e];
	}

	/* Arrow keys */
	if ((scancode >= 0x4f) && (scancode <= 0x52))
		keycode = usb_kbd_arrow[scancode - 0x4f];

	/* Numeric keypad */
	if ((scancode >= 0x54) && (scancode <= 0x67))
		keycode = usb_kbd_num_keypad[scancode - 0x54];

	if (data->flags & USB_KBD_CTRL)
		keycode = scancode - 0x3;

	if (pressed == 1) {
		if (scancode == NUM_LOCK) {
			data->flags ^= USB_KBD_NUMLOCK;
			return 1;
		}

		if (scancode == CAPS_LOCK) {
			data->flags ^= USB_KBD_CAPSLOCK;
			return 1;
		}
		if (scancode == SCROLL_LOCK) {
			data->flags ^= USB_KBD_SCROLLLOCK;
			return 1;
		}
	}

	/* Report keycode if any */
	if (keycode) {
		USB_KBD_PRINTF("%c", keycode);
		usb_kbd_put_queue(data, keycode);
	}

	return 0;
}

static uint32_t usb_kbd_service_key(struct usb_device *dev, int i, int up)
{
	uint32_t res = 0;
	struct usb_kbd_pdata *data = dev->privptr;
	uint8_t *new;
	uint8_t *old;

	if (up) {
		new = data->old;
		old = data->new;
	} else {
		new = data->new;
		old = data->old;
	}

	if ((old[i] > 3) && (memscan(new + 2, old[i], 6) == new + 8))
		res |= usb_kbd_translate(data, old[i], data->new[0], up);

	return res;
}

/* Interrupt service routine */
static int usb_kbd_irq_worker(struct usb_device *dev)
{
	struct usb_kbd_pdata *data = dev->privptr;
	int i, res = 0;

	/* No combo key pressed */
	if (data->new[0] == 0x00)
		data->flags &= ~USB_KBD_CTRL;
	/* Left or Right Ctrl pressed */
	else if ((data->new[0] == LEFT_CNTR) || (data->new[0] == RIGHT_CNTR))
		data->flags |= USB_KBD_CTRL;

	for (i = 2; i < 8; i++) {
		res |= usb_kbd_service_key(dev, i, 0);
		res |= usb_kbd_service_key(dev, i, 1);
	}

	/* Key is still pressed */
	if ((data->new[2] > 3) && (data->old[2] == data->new[2]))
		res |= usb_kbd_translate(data, data->new[2], data->new[0], 2);

	if (res == 1)
		usb_kbd_setled(dev);

	memcpy(data->old, data->new, 8);

	return 1;
}

/* Keyboard interrupt handler */
static int usb_kbd_irq(struct usb_device *dev)
{
	if ((dev->irq_status != 0) || (dev->irq_act_len != 8)) {
		USB_KBD_PRINTF("USB KBD: Error %lX, len %d\n",
				dev->irq_status, dev->irq_act_len);
		return 1;
	}

	return usb_kbd_irq_worker(dev);
}

/* Interrupt polling */
static inline void usb_kbd_poll_for_event(struct usb_device *dev)
{
#if	defined(CONFIG_SYS_USB_EVENT_POLL)
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep;
	struct usb_kbd_pdata *data;
	int pipe;
	int maxp;

	/* Get the pointer to USB Keyboard device pointer */
	data = dev->privptr;
	iface = &dev->config.if_desc[0];
	ep = &iface->ep_desc[0];
	pipe = usb_rcvintpipe(dev, ep->bEndpointAddress);

	/* Submit a interrupt transfer request */
	maxp = usb_maxpacket(dev, pipe);
	usb_submit_int_msg(dev, pipe, &data->new[0],
			maxp > 8 ? 8 : maxp, ep->bInterval);

	usb_kbd_irq_worker(dev);
#elif	defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	struct usb_interface *iface;
	struct usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[0];
	usb_get_report(dev, iface->desc.bInterfaceNumber,
			1, 0, data->new, sizeof(data->new));
	if (memcmp(data->old, data->new, sizeof(data->new)))
		usb_kbd_irq_worker(dev);
#endif
}

/* test if a character is in the queue */
static int usb_kbd_testc(void)
{
	struct stdio_dev *dev;
	struct usb_device *usb_kbd_dev;
	struct usb_kbd_pdata *data;

	dev = stdio_get_by_name(DEVNAME);
	usb_kbd_dev = (struct usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	usb_kbd_poll_for_event(usb_kbd_dev);

	return !(data->usb_in_pointer == data->usb_out_pointer);
}

/* gets the character from the queue */
static int usb_kbd_getc(void)
{
	struct stdio_dev *dev;
	struct usb_device *usb_kbd_dev;
	struct usb_kbd_pdata *data;

	dev = stdio_get_by_name(DEVNAME);
	usb_kbd_dev = (struct usb_device *)dev->priv;
	data = usb_kbd_dev->privptr;

	while (data->usb_in_pointer == data->usb_out_pointer)
		usb_kbd_poll_for_event(usb_kbd_dev);

	if (data->usb_out_pointer == USB_KBD_BUFFER_LEN - 1)
		data->usb_out_pointer = 0;
	else
		data->usb_out_pointer++;

	return data->usb_kbd_buffer[data->usb_out_pointer];
}

/* probes the USB device dev for keyboard type. */
static int usb_kbd_probe(struct usb_device *dev, unsigned int ifnum)
{
	struct usb_interface *iface;
	struct usb_endpoint_descriptor *ep;
	struct usb_kbd_pdata *data;
	int pipe, maxp;

	if (dev->descriptor.bNumConfigurations != 1)
		return 0;

	iface = &dev->config.if_desc[ifnum];

	if (iface->desc.bInterfaceClass != 3)
		return 0;

	if (iface->desc.bInterfaceSubClass != 1)
		return 0;

	if (iface->desc.bInterfaceProtocol != 1)
		return 0;

	if (iface->desc.bNumEndpoints != 1)
		return 0;

	ep = &iface->ep_desc[0];

	/* Check if endpoint 1 is interrupt endpoint */
	if (!(ep->bEndpointAddress & 0x80))
		return 0;

	if ((ep->bmAttributes & 3) != 3)
		return 0;

	USB_KBD_PRINTF("USB KBD: found set protocol...\n");

	data = malloc(sizeof(struct usb_kbd_pdata));
	if (!data) {
		printf("USB KBD: Error allocating private data\n");
		return 0;
	}

	/* Clear private data */
	memset(data, 0, sizeof(struct usb_kbd_pdata));

	/* allocate input buffer aligned and sized to USB DMA alignment */
	data->new = memalign(USB_DMA_MINALIGN, roundup(8, USB_DMA_MINALIGN));

	/* Insert private data into USB device structure */
	dev->privptr = data;

	/* Set IRQ handler */
	dev->irq_handle = usb_kbd_irq;

	pipe = usb_rcvintpipe(dev, ep->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe);

	/* We found a USB Keyboard, install it. */
	usb_set_protocol(dev, iface->desc.bInterfaceNumber, 0);

	USB_KBD_PRINTF("USB KBD: found set idle...\n");
	usb_set_idle(dev, iface->desc.bInterfaceNumber, REPEAT_RATE, 0);

	USB_KBD_PRINTF("USB KBD: enable interrupt pipe...\n");
	usb_submit_int_msg(dev, pipe, data->new, maxp > 8 ? 8 : maxp,
				ep->bInterval);

	/* Success. */
	return 1;
}

/* Search for keyboard and register it if found. */
int drv_usb_kbd_init(void)
{
	struct stdio_dev usb_kbd_dev, *old_dev;
	struct usb_device *dev;
	char *stdinname = getenv("stdin");
	int error, i;

	/* Scan all USB Devices */
	for (i = 0; i < USB_MAX_DEVICE; i++) {
		/* Get USB device. */
		dev = usb_get_dev_index(i);
		if (!dev)
			return -1;

		if (dev->devnum == -1)
			continue;

		/* Try probing the keyboard */
		if (usb_kbd_probe(dev, 0) != 1)
			continue;

		/* We found a keyboard, check if it is already registered. */
		USB_KBD_PRINTF("USB KBD: found set up device.\n");
		old_dev = stdio_get_by_name(DEVNAME);
		if (old_dev) {
			/* Already registered, just return ok. */
			USB_KBD_PRINTF("USB KBD: is already registered.\n");
			return 1;
		}

		/* Register the keyboard */
		USB_KBD_PRINTF("USB KBD: register.\n");
		memset(&usb_kbd_dev, 0, sizeof(struct stdio_dev));
		strcpy(usb_kbd_dev.name, DEVNAME);
		usb_kbd_dev.flags =  DEV_FLAGS_INPUT | DEV_FLAGS_SYSTEM;
		usb_kbd_dev.putc = NULL;
		usb_kbd_dev.puts = NULL;
		usb_kbd_dev.getc = usb_kbd_getc;
		usb_kbd_dev.tstc = usb_kbd_testc;
		usb_kbd_dev.priv = (void *)dev;
		error = stdio_register(&usb_kbd_dev);
		if (error)
			return error;

#ifdef CONFIG_CONSOLE_MUX
		error = iomux_doenv(stdin, stdinname);
		if (error)
			return error;
#else
		/* Check if this is the standard input device. */
		if (strcmp(stdinname, DEVNAME))
			return 1;

		/* Reassign the console */
		if (overwrite_console())
			return 1;

		error = console_assign(stdin, DEVNAME);
		if (error)
			return error;
#endif

		return 1;
	}

	/* No USB Keyboard found */
	return -1;
}

/* Deregister the keyboard. */
int usb_kbd_deregister(void)
{
#ifdef CONFIG_SYS_STDIO_DEREGISTER
	return stdio_deregister(DEVNAME);
#else
	return 1;
#endif
}
