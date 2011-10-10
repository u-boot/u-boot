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

	uint8_t		new[8];
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
	usb_event_poll();
	usb_kbd_irq_worker(dev);
#elif	defined(CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP)
	struct usb_interface *iface;
	struct usb_kbd_pdata *data = dev->privptr;
	iface = &dev->config.if_desc[0];
	usb_get_report(dev, iface->desc.bInterfaceNumber,
			1, 1, data->new, sizeof(data->new));
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

		/* Check if this is the standard input device. */
		if (strcmp(stdinname, DEVNAME))
			return 1;

		/* Reassign the console */
		if (overwrite_console())
			return 1;

		error = console_assign(stdin, DEVNAME);
		if (error)
			return error;

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

#if 0
struct usb_hid_descriptor {
	unsigned char  bLength;
	unsigned char  bDescriptorType; /* 0x21 for HID */
	unsigned short bcdHID; /* release number */
	unsigned char  bCountryCode;
	unsigned char  bNumDescriptors;
	unsigned char  bReportDescriptorType;
	unsigned short wDescriptorLength;
} __packed;

/*
 * We parse each description item into this structure. Short items data
 * values are expanded to 32-bit signed int, long items contain a pointer
 * into the data area.
 */

struct hid_item {
	unsigned char format;
	unsigned char size;
	unsigned char type;
	unsigned char tag;
	union {
	    unsigned char   u8;
	    char            s8;
	    unsigned short  u16;
	    short           s16;
	    unsigned long   u32;
	    long            s32;
	    unsigned char  *longdata;
	} data;
};

/*
 * HID report item format
 */

#define HID_ITEM_FORMAT_SHORT	0
#define HID_ITEM_FORMAT_LONG	1

/*
 * Special tag indicating long items
 */

#define HID_ITEM_TAG_LONG	15


static struct usb_hid_descriptor usb_kbd_hid_desc;

void usb_kbd_display_hid(struct usb_hid_descriptor *hid)
{
	printf("USB_HID_DESC:\n");
	printf("  bLenght               0x%x\n", hid->bLength);
	printf("  bcdHID                0x%x\n", hid->bcdHID);
	printf("  bCountryCode          %d\n", hid->bCountryCode);
	printf("  bNumDescriptors       0x%x\n", hid->bNumDescriptors);
	printf("  bReportDescriptorType 0x%x\n", hid->bReportDescriptorType);
	printf("  wDescriptorLength     0x%x\n", hid->wDescriptorLength);
}


/*
 * Fetch a report description item from the data stream. We support long
 * items, though they are not used yet.
 */

static int fetch_item(unsigned char *start, unsigned char *end,
			struct hid_item *item)
{
	if ((end - start) > 0) {
		unsigned char b = *start++;
		item->type = (b >> 2) & 3;
		item->tag  = (b >> 4) & 15;
		if (item->tag == HID_ITEM_TAG_LONG) {
			item->format = HID_ITEM_FORMAT_LONG;
			if ((end - start) >= 2) {
				item->size = *start++;
				item->tag  = *start++;
				if ((end - start) >= item->size) {
					item->data.longdata = start;
					start += item->size;
					return item->size;
				}
			}
		} else {
			item->format = HID_ITEM_FORMAT_SHORT;
			item->size = b & 3;
			switch (item->size) {
			case 0:
				return item->size;
			case 1:
				if ((end - start) >= 1) {
					item->data.u8 = *start++;
					return item->size;
				}
				break;
			case 2:
				if ((end - start) >= 2) {
					item->data.u16 = le16_to_cpu(
						(unsigned short *)start);
					start += 2;
					return item->size;
				}
			case 3:
				item->size++;
				if ((end - start) >= 4) {
					item->data.u32 = le32_to_cpu(
						(unsigned long *)start);
					start += 4;
					return item->size;
				}
			}
		}
	}
	return -1;
}

/*
 * HID report descriptor item type (prefix bit 2, 3)
 */

#define HID_ITEM_TYPE_MAIN		0
#define HID_ITEM_TYPE_GLOBAL		1
#define HID_ITEM_TYPE_LOCAL		2
#define HID_ITEM_TYPE_RESERVED		3
/*
 * HID report descriptor main item tags
 */

#define HID_MAIN_ITEM_TAG_INPUT			8
#define HID_MAIN_ITEM_TAG_OUTPUT		9
#define HID_MAIN_ITEM_TAG_FEATURE		11
#define HID_MAIN_ITEM_TAG_BEGIN_COLLECTION	10
#define HID_MAIN_ITEM_TAG_END_COLLECTION	12
/*
 * HID report descriptor main item contents
 */

#define HID_MAIN_ITEM_CONSTANT		0x001
#define HID_MAIN_ITEM_VARIABLE		0x002
#define HID_MAIN_ITEM_RELATIVE		0x004
#define HID_MAIN_ITEM_WRAP		0x008
#define HID_MAIN_ITEM_NONLINEAR		0x010
#define HID_MAIN_ITEM_NO_PREFERRED	0x020
#define HID_MAIN_ITEM_NULL_STATE	0x040
#define HID_MAIN_ITEM_VOLATILE		0x080
#define HID_MAIN_ITEM_BUFFERED_BYTE	0x100

/*
 * HID report descriptor collection item types
 */

#define HID_COLLECTION_PHYSICAL		0
#define HID_COLLECTION_APPLICATION	1
#define HID_COLLECTION_LOGICAL		2
/*
 * HID report descriptor global item tags
 */

#define HID_GLOBAL_ITEM_TAG_USAGE_PAGE		0
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM	1
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM	2
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM	3
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM	4
#define HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT	5
#define HID_GLOBAL_ITEM_TAG_UNIT		6
#define HID_GLOBAL_ITEM_TAG_REPORT_SIZE		7
#define HID_GLOBAL_ITEM_TAG_REPORT_ID		8
#define HID_GLOBAL_ITEM_TAG_REPORT_COUNT	9
#define HID_GLOBAL_ITEM_TAG_PUSH		10
#define HID_GLOBAL_ITEM_TAG_POP			11

/*
 * HID report descriptor local item tags
 */

#define HID_LOCAL_ITEM_TAG_USAGE		0
#define HID_LOCAL_ITEM_TAG_USAGE_MINIMUM	1
#define HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM	2
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX	3
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM	4
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM	5
#define HID_LOCAL_ITEM_TAG_STRING_INDEX		7
#define HID_LOCAL_ITEM_TAG_STRING_MINIMUM	8
#define HID_LOCAL_ITEM_TAG_STRING_MAXIMUM	9
#define HID_LOCAL_ITEM_TAG_DELIMITER		10


static void usb_kbd_show_item(struct hid_item *item)
{
	switch (item->type) {
	case HID_ITEM_TYPE_MAIN:
		switch (item->tag) {
		case HID_MAIN_ITEM_TAG_INPUT:
			printf("Main Input");
			break;
		case HID_MAIN_ITEM_TAG_OUTPUT:
			printf("Main Output");
			break;
		case HID_MAIN_ITEM_TAG_FEATURE:
			printf("Main Feature");
			break;
		case HID_MAIN_ITEM_TAG_BEGIN_COLLECTION:
			printf("Main Begin Collection");
			break;
		case HID_MAIN_ITEM_TAG_END_COLLECTION:
			printf("Main End Collection");
			break;
		default:
			printf("Main reserved %d", item->tag);
			break;
		}
		break;
	case HID_ITEM_TYPE_GLOBAL:
		switch (item->tag) {
		case HID_GLOBAL_ITEM_TAG_USAGE_PAGE:
			printf("- Global Usage Page");
			break;
		case HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM:
			printf("- Global Logical Minimum");
			break;
		case HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM:
			printf("- Global Logical Maximum");
			break;
		case HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM:
			printf("- Global physical Minimum");
			break;
		case HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM:
			printf("- Global physical Maximum");
			break;
		case HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT:
			printf("- Global Unit Exponent");
			break;
		case HID_GLOBAL_ITEM_TAG_UNIT:
			printf("- Global Unit");
			break;
		case HID_GLOBAL_ITEM_TAG_REPORT_SIZE:
			printf("- Global Report Size");
			break;
		case HID_GLOBAL_ITEM_TAG_REPORT_ID:
			printf("- Global Report ID");
			break;
		case HID_GLOBAL_ITEM_TAG_REPORT_COUNT:
			printf("- Global Report Count");
			break;
		case HID_GLOBAL_ITEM_TAG_PUSH:
			printf("- Global Push");
			break;
		case HID_GLOBAL_ITEM_TAG_POP:
			printf("- Global Pop");
			break;
		default:
			printf("- Global reserved %d", item->tag);
			break;
		}
		break;
	case HID_ITEM_TYPE_LOCAL:
		switch (item->tag) {
		case HID_LOCAL_ITEM_TAG_USAGE:
			printf("-- Local Usage");
			break;
		case HID_LOCAL_ITEM_TAG_USAGE_MINIMUM:
			printf("-- Local Usage Minimum");
			break;
		case HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM:
			printf("-- Local Usage Maximum");
			break;
		case HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX:
			printf("-- Local Designator Index");
			break;
		case HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM:
			printf("-- Local Designator Minimum");
			break;
		case HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM:
			printf("-- Local Designator Maximum");
			break;
		case HID_LOCAL_ITEM_TAG_STRING_INDEX:
			printf("-- Local String Index");
			break;
		case HID_LOCAL_ITEM_TAG_STRING_MINIMUM:
			printf("-- Local String Minimum");
			break;
		case HID_LOCAL_ITEM_TAG_STRING_MAXIMUM:
			printf("-- Local String Maximum");
			break;
		case HID_LOCAL_ITEM_TAG_DELIMITER:
			printf("-- Local Delimiter");
			break;
		default:
			printf("-- Local reserved %d", item->tag);
			break;
		}
		break;
	default:
		printf("--- reserved %d", item->type);
		break;
	}
	switch (item->size) {
	case 1:
		printf("  %d", item->data.u8);
		break;
	case 2:
		printf("  %d", item->data.u16);
		break;
	case 4:
		printf("  %ld", item->data.u32);
		break;
	}
	printf("\n");
}


static int usb_kbd_get_hid_desc(struct usb_device *dev)
{
	unsigned char buffer[256];
	struct usb_descriptor_header *head;
	struct usb_config_descriptor *config;
	int index, len, i;
	unsigned char *start, *end;
	struct hid_item item;

	if (usb_get_configuration_no(dev, &buffer[0], 0) == -1)
		return -1;
	head = (struct usb_descriptor_header *)&buffer[0];
	if (head->bDescriptorType != USB_DT_CONFIG) {
		printf(" ERROR: NOT USB_CONFIG_DESC %x\n",
				head->bDescriptorType);
		return -1;
	}
	index = head->bLength;
	config = (struct usb_config_descriptor *)&buffer[0];
	len = le16_to_cpu(config->wTotalLength);
	/*
	 * Ok the first entry must be a configuration entry,
	 * now process the others
	 */
	head = (struct usb_descriptor_header *)&buffer[index];
	while (index+1 < len) {
		if (head->bDescriptorType == USB_DT_HID) {
			printf("HID desc found\n");
			memcpy(&usb_kbd_hid_desc, &buffer[index],
					buffer[index]);
			le16_to_cpus(&usb_kbd_hid_desc.bcdHID);
			le16_to_cpus(&usb_kbd_hid_desc.wDescriptorLength);
			usb_kbd_display_hid(&usb_kbd_hid_desc);
			len = 0;
			break;
		}
		index += head->bLength;
		head = (struct usb_descriptor_header *)&buffer[index];
	}
	if (len > 0)
		return -1;
	len = usb_kbd_hid_desc.wDescriptorLength;
	index = usb_get_class_descriptor(dev, 0, USB_DT_REPORT, 0, &buffer[0],
						len);
	if (index < 0) {
		printf("reading report descriptor failed\n");
		return -1;
	}
	printf(" report descriptor (size %u, read %d)\n", len, index);
	start = &buffer[0];
	end = &buffer[len];
	i = 0;
	do {
		index = fetch_item(start, end, &item);
		i += index;
		i++;
		if (index >= 0)
			usb_kbd_show_item(&item);

		start += index;
		start++;
	} while (index >= 0);

}

#endif
