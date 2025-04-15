// SPDX-License-Identifier: GPL-1.0+
/*
 * Renesas USB
 *
 * Copyright (C) 2011 Renesas Solutions Corp.
 * Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
 *
 * Ported to u-boot
 * Copyright (C) 2016 GlobalLogic
 */
#ifndef RENESAS_USB_H
#define RENESAS_USB_H

#include <linux/usb/ch9.h>
#include <linux/compat.h>

struct platform_device {
	const char	*name;
	struct device	dev;
};

/*
 * module type
 *
 * it will be return value from get_id
 */
enum {
	USBHS_HOST = 0,
	USBHS_GADGET,
	USBHS_MAX,
};

/*
 * parameters for renesas usbhs
 *
 * some register needs USB chip specific parameters.
 * This struct show it to driver
 */

struct renesas_usbhs_driver_pipe_config {
	u8 type;	/* USB_ENDPOINT_XFER_xxx */
	u16 bufsize;
	u8 bufnum;
	bool double_buf;
};
#define RENESAS_USBHS_PIPE(_type, _size, _num, _double_buf)	{	\
			.type = (_type),		\
			.bufsize = (_size),		\
			.bufnum = (_num),		\
			.double_buf = (_double_buf),	\
	}

struct renesas_usbhs_driver_param {
	/*
	 * pipe settings
	 */
	struct renesas_usbhs_driver_pipe_config *pipe_configs;
	int pipe_size; /* pipe_configs array size */

	/*
	 * option:
	 *
	 * for BUSWAIT :: BWAIT
	 * see
	 *	renesas_usbhs/common.c :: usbhsc_set_buswait()
	 * */
	int buswait_bwait;

	/*
	 * option:
	 *
	 * delay time from notify_hotplug callback
	 */
	int detection_delay; /* msec */

	/*
	 * option:
	 *
	 * dma id for dmaengine
	 * The data transfer direction on D0FIFO/D1FIFO should be
	 * fixed for keeping consistency.
	 * So, the platform id settings will be..
	 *	.d0_tx_id = xx_TX,
	 *	.d1_rx_id = xx_RX,
	 * or
	 *	.d1_tx_id = xx_TX,
	 *	.d0_rx_id = xx_RX,
	 */
	int d0_tx_id;
	int d0_rx_id;
	int d1_tx_id;
	int d1_rx_id;
	int d2_tx_id;
	int d2_rx_id;
	int d3_tx_id;
	int d3_rx_id;

	/*
	 * option:
	 *
	 * pio <--> dma border.
	 */
	int pio_dma_border; /* default is 64byte */

	uintptr_t type;
	u32 enable_gpio;

	/*
	 * option:
	 */
	u32 has_otg:1; /* for controlling PWEN/EXTLP */
	u32 has_sudmac:1; /* for SUDMAC */
	u32 has_usb_dmac:1; /* for USB-DMAC */
	u32 cfifo_byte_addr:1; /* CFIFO is byte addressable */
#define USBHS_USB_DMAC_XFER_SIZE	32	/* hardcode the xfer size */
	u32 multi_clks:1;
	u32 has_new_pipe_configs:1;
};

#define USBHS_TYPE_RCAR_GEN3		2

struct usbhs_priv;
struct usb_gadget *usbhsg_get_gadget(struct usbhs_priv *priv);

#endif /* RENESAS_USB_H */
