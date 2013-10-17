/*-
 * Copyright (c) 2007-2008, Juniper Networks, Inc.
 * Copyright (c) 2008, Excito Elektronik i Skåne AB
 * Copyright (c) 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2 of
 * the License.
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
#include <errno.h>
#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <usb.h>
#include <asm/io.h>
#include <malloc.h>
#include <watchdog.h>
#include <linux/compiler.h>

#include "ehci.h"

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

/*
 * EHCI spec page 20 says that the HC may take up to 16 uFrames (= 4ms) to halt.
 * Let's time out after 8 to have a little safety margin on top of that.
 */
#define HCHALT_TIMEOUT (8 * 1000)

static struct ehci_ctrl ehcic[CONFIG_USB_MAX_CONTROLLER_COUNT];

#define ALIGN_END_ADDR(type, ptr, size)			\
	((uint32_t)(ptr) + roundup((size) * sizeof(type), USB_DMA_MINALIGN))

static struct descriptor {
	struct usb_hub_descriptor hub;
	struct usb_device_descriptor device;
	struct usb_linux_config_descriptor config;
	struct usb_linux_interface_descriptor interface;
	struct usb_endpoint_descriptor endpoint;
}  __attribute__ ((packed)) descriptor = {
	{
		0x8,		/* bDescLength */
		0x29,		/* bDescriptorType: hub descriptor */
		2,		/* bNrPorts -- runtime modified */
		0,		/* wHubCharacteristics */
		10,		/* bPwrOn2PwrGood */
		0,		/* bHubCntrCurrent */
		{},		/* Device removable */
		{}		/* at most 7 ports! XXX */
	},
	{
		0x12,		/* bLength */
		1,		/* bDescriptorType: UDESC_DEVICE */
		cpu_to_le16(0x0200), /* bcdUSB: v2.0 */
		9,		/* bDeviceClass: UDCLASS_HUB */
		0,		/* bDeviceSubClass: UDSUBCLASS_HUB */
		1,		/* bDeviceProtocol: UDPROTO_HSHUBSTT */
		64,		/* bMaxPacketSize: 64 bytes */
		0x0000,		/* idVendor */
		0x0000,		/* idProduct */
		cpu_to_le16(0x0100), /* bcdDevice */
		1,		/* iManufacturer */
		2,		/* iProduct */
		0,		/* iSerialNumber */
		1		/* bNumConfigurations: 1 */
	},
	{
		0x9,
		2,		/* bDescriptorType: UDESC_CONFIG */
		cpu_to_le16(0x19),
		1,		/* bNumInterface */
		1,		/* bConfigurationValue */
		0,		/* iConfiguration */
		0x40,		/* bmAttributes: UC_SELF_POWER */
		0		/* bMaxPower */
	},
	{
		0x9,		/* bLength */
		4,		/* bDescriptorType: UDESC_INTERFACE */
		0,		/* bInterfaceNumber */
		0,		/* bAlternateSetting */
		1,		/* bNumEndpoints */
		9,		/* bInterfaceClass: UICLASS_HUB */
		0,		/* bInterfaceSubClass: UISUBCLASS_HUB */
		0,		/* bInterfaceProtocol: UIPROTO_HSHUBSTT */
		0		/* iInterface */
	},
	{
		0x7,		/* bLength */
		5,		/* bDescriptorType: UDESC_ENDPOINT */
		0x81,		/* bEndpointAddress:
				 * UE_DIR_IN | EHCI_INTR_ENDPT
				 */
		3,		/* bmAttributes: UE_INTERRUPT */
		8,		/* wMaxPacketSize */
		255		/* bInterval */
	},
};

#if defined(CONFIG_EHCI_IS_TDI)
#define ehci_is_TDI()	(1)
#else
#define ehci_is_TDI()	(0)
#endif

int __ehci_get_port_speed(struct ehci_hcor *hcor, uint32_t reg)
{
	return PORTSC_PSPD(reg);
}

int ehci_get_port_speed(struct ehci_hcor *hcor, uint32_t reg)
	__attribute__((weak, alias("__ehci_get_port_speed")));

void __ehci_set_usbmode(int index)
{
	uint32_t tmp;
	uint32_t *reg_ptr;

	reg_ptr = (uint32_t *)((u8 *)&ehcic[index].hcor->or_usbcmd + USBMODE);
	tmp = ehci_readl(reg_ptr);
	tmp |= USBMODE_CM_HC;
#if defined(CONFIG_EHCI_MMIO_BIG_ENDIAN)
	tmp |= USBMODE_BE;
#endif
	ehci_writel(reg_ptr, tmp);
}

void ehci_set_usbmode(int index)
	__attribute__((weak, alias("__ehci_set_usbmode")));

void __ehci_powerup_fixup(uint32_t *status_reg, uint32_t *reg)
{
	mdelay(50);
}

void ehci_powerup_fixup(uint32_t *status_reg, uint32_t *reg)
	__attribute__((weak, alias("__ehci_powerup_fixup")));

static int handshake(uint32_t *ptr, uint32_t mask, uint32_t done, int usec)
{
	uint32_t result;
	do {
		result = ehci_readl(ptr);
		udelay(5);
		if (result == ~(uint32_t)0)
			return -1;
		result &= mask;
		if (result == done)
			return 0;
		usec--;
	} while (usec > 0);
	return -1;
}

static int ehci_reset(int index)
{
	uint32_t cmd;
	int ret = 0;

	cmd = ehci_readl(&ehcic[index].hcor->or_usbcmd);
	cmd = (cmd & ~CMD_RUN) | CMD_RESET;
	ehci_writel(&ehcic[index].hcor->or_usbcmd, cmd);
	ret = handshake((uint32_t *)&ehcic[index].hcor->or_usbcmd,
			CMD_RESET, 0, 250 * 1000);
	if (ret < 0) {
		printf("EHCI fail to reset\n");
		goto out;
	}

	if (ehci_is_TDI())
		ehci_set_usbmode(index);

#ifdef CONFIG_USB_EHCI_TXFIFO_THRESH
	cmd = ehci_readl(&ehcic[index].hcor->or_txfilltuning);
	cmd &= ~TXFIFO_THRESH_MASK;
	cmd |= TXFIFO_THRESH(CONFIG_USB_EHCI_TXFIFO_THRESH);
	ehci_writel(&ehcic[index].hcor->or_txfilltuning, cmd);
#endif
out:
	return ret;
}

static int ehci_shutdown(struct ehci_ctrl *ctrl)
{
	int i, ret = 0;
	uint32_t cmd, reg;

	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd &= ~(CMD_PSE | CMD_ASE);
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
	ret = handshake(&ctrl->hcor->or_usbsts, STS_ASS | STS_PSS, 0,
		100 * 1000);

	if (!ret) {
		for (i = 0; i < CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS; i++) {
			reg = ehci_readl(&ctrl->hcor->or_portsc[i]);
			reg |= EHCI_PS_SUSP;
			ehci_writel(&ctrl->hcor->or_portsc[i], reg);
		}

		cmd &= ~CMD_RUN;
		ehci_writel(&ctrl->hcor->or_usbcmd, cmd);
		ret = handshake(&ctrl->hcor->or_usbsts, STS_HALT, STS_HALT,
			HCHALT_TIMEOUT);
	}

	if (ret)
		puts("EHCI failed to shut down host controller.\n");

	return ret;
}

static int ehci_td_buffer(struct qTD *td, void *buf, size_t sz)
{
	uint32_t delta, next;
	uint32_t addr = (uint32_t)buf;
	int idx;

	if (addr != ALIGN(addr, ARCH_DMA_MINALIGN))
		debug("EHCI-HCD: Misaligned buffer address (%p)\n", buf);

	flush_dcache_range(addr, ALIGN(addr + sz, ARCH_DMA_MINALIGN));

	idx = 0;
	while (idx < QT_BUFFER_CNT) {
		td->qt_buffer[idx] = cpu_to_hc32(addr);
		td->qt_buffer_hi[idx] = 0;
		next = (addr + EHCI_PAGE_SIZE) & ~(EHCI_PAGE_SIZE - 1);
		delta = next - addr;
		if (delta >= sz)
			break;
		sz -= delta;
		addr = next;
		idx++;
	}

	if (idx == QT_BUFFER_CNT) {
		printf("out of buffer pointers (%u bytes left)\n", sz);
		return -1;
	}

	return 0;
}

static inline u8 ehci_encode_speed(enum usb_device_speed speed)
{
	#define QH_HIGH_SPEED	2
	#define QH_FULL_SPEED	0
	#define QH_LOW_SPEED	1
	if (speed == USB_SPEED_HIGH)
		return QH_HIGH_SPEED;
	if (speed == USB_SPEED_LOW)
		return QH_LOW_SPEED;
	return QH_FULL_SPEED;
}

static int
ehci_submit_async(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int length, struct devrequest *req)
{
	ALLOC_ALIGN_BUFFER(struct QH, qh, 1, USB_DMA_MINALIGN);
	struct qTD *qtd;
	int qtd_count = 0;
	int qtd_counter = 0;
	volatile struct qTD *vtd;
	unsigned long ts;
	uint32_t *tdp;
	uint32_t endpt, maxpacket, token, usbsts;
	uint32_t c, toggle;
	uint32_t cmd;
	int timeout;
	int ret = 0;
	struct ehci_ctrl *ctrl = dev->controller;

	debug("dev=%p, pipe=%lx, buffer=%p, length=%d, req=%p\n", dev, pipe,
	      buffer, length, req);
	if (req != NULL)
		debug("req=%u (%#x), type=%u (%#x), value=%u (%#x), index=%u\n",
		      req->request, req->request,
		      req->requesttype, req->requesttype,
		      le16_to_cpu(req->value), le16_to_cpu(req->value),
		      le16_to_cpu(req->index));

#define PKT_ALIGN	512
	/*
	 * The USB transfer is split into qTD transfers. Eeach qTD transfer is
	 * described by a transfer descriptor (the qTD). The qTDs form a linked
	 * list with a queue head (QH).
	 *
	 * Each qTD transfer starts with a new USB packet, i.e. a packet cannot
	 * have its beginning in a qTD transfer and its end in the following
	 * one, so the qTD transfer lengths have to be chosen accordingly.
	 *
	 * Each qTD transfer uses up to QT_BUFFER_CNT data buffers, mapped to
	 * single pages. The first data buffer can start at any offset within a
	 * page (not considering the cache-line alignment issues), while the
	 * following buffers must be page-aligned. There is no alignment
	 * constraint on the size of a qTD transfer.
	 */
	if (req != NULL)
		/* 1 qTD will be needed for SETUP, and 1 for ACK. */
		qtd_count += 1 + 1;
	if (length > 0 || req == NULL) {
		/*
		 * Determine the qTD transfer size that will be used for the
		 * data payload (not considering the first qTD transfer, which
		 * may be longer or shorter, and the final one, which may be
		 * shorter).
		 *
		 * In order to keep each packet within a qTD transfer, the qTD
		 * transfer size is aligned to PKT_ALIGN, which is a multiple of
		 * wMaxPacketSize (except in some cases for interrupt transfers,
		 * see comment in submit_int_msg()).
		 *
		 * By default, i.e. if the input buffer is aligned to PKT_ALIGN,
		 * QT_BUFFER_CNT full pages will be used.
		 */
		int xfr_sz = QT_BUFFER_CNT;
		/*
		 * However, if the input buffer is not aligned to PKT_ALIGN, the
		 * qTD transfer size will be one page shorter, and the first qTD
		 * data buffer of each transfer will be page-unaligned.
		 */
		if ((uint32_t)buffer & (PKT_ALIGN - 1))
			xfr_sz--;
		/* Convert the qTD transfer size to bytes. */
		xfr_sz *= EHCI_PAGE_SIZE;
		/*
		 * Approximate by excess the number of qTDs that will be
		 * required for the data payload. The exact formula is way more
		 * complicated and saves at most 2 qTDs, i.e. a total of 128
		 * bytes.
		 */
		qtd_count += 2 + length / xfr_sz;
	}
/*
 * Threshold value based on the worst-case total size of the allocated qTDs for
 * a mass-storage transfer of 65535 blocks of 512 bytes.
 */
#if CONFIG_SYS_MALLOC_LEN <= 64 + 128 * 1024
#warning CONFIG_SYS_MALLOC_LEN may be too small for EHCI
#endif
	qtd = memalign(USB_DMA_MINALIGN, qtd_count * sizeof(struct qTD));
	if (qtd == NULL) {
		printf("unable to allocate TDs\n");
		return -1;
	}

	memset(qh, 0, sizeof(struct QH));
	memset(qtd, 0, qtd_count * sizeof(*qtd));

	toggle = usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe));

	/*
	 * Setup QH (3.6 in ehci-r10.pdf)
	 *
	 *   qh_link ................. 03-00 H
	 *   qh_endpt1 ............... 07-04 H
	 *   qh_endpt2 ............... 0B-08 H
	 * - qh_curtd
	 *   qh_overlay.qt_next ...... 13-10 H
	 * - qh_overlay.qt_altnext
	 */
	qh->qh_link = cpu_to_hc32((uint32_t)&ctrl->qh_list | QH_LINK_TYPE_QH);
	c = (dev->speed != USB_SPEED_HIGH) && !usb_pipeendpoint(pipe);
	maxpacket = usb_maxpacket(dev, pipe);
	endpt = QH_ENDPT1_RL(8) | QH_ENDPT1_C(c) |
		QH_ENDPT1_MAXPKTLEN(maxpacket) | QH_ENDPT1_H(0) |
		QH_ENDPT1_DTC(QH_ENDPT1_DTC_DT_FROM_QTD) |
		QH_ENDPT1_EPS(ehci_encode_speed(dev->speed)) |
		QH_ENDPT1_ENDPT(usb_pipeendpoint(pipe)) | QH_ENDPT1_I(0) |
		QH_ENDPT1_DEVADDR(usb_pipedevice(pipe));
	qh->qh_endpt1 = cpu_to_hc32(endpt);
	endpt = QH_ENDPT2_MULT(1) | QH_ENDPT2_PORTNUM(dev->portnr) |
		QH_ENDPT2_HUBADDR(dev->parent->devnum) |
		QH_ENDPT2_UFCMASK(0) | QH_ENDPT2_UFSMASK(0);
	qh->qh_endpt2 = cpu_to_hc32(endpt);
	qh->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);

	tdp = &qh->qh_overlay.qt_next;

	if (req != NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 *
		 *   [ buffer, buffer_hi ] loaded with "req".
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = QT_TOKEN_DT(0) | QT_TOKEN_TOTALBYTES(sizeof(*req)) |
			QT_TOKEN_IOC(0) | QT_TOKEN_CPAGE(0) | QT_TOKEN_CERR(3) |
			QT_TOKEN_PID(QT_TOKEN_PID_SETUP) |
			QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		if (ehci_td_buffer(&qtd[qtd_counter], req, sizeof(*req))) {
			printf("unable to construct SETUP TD\n");
			goto fail;
		}
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((uint32_t)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
		toggle = 1;
	}

	if (length > 0 || req == NULL) {
		uint8_t *buf_ptr = buffer;
		int left_length = length;

		do {
			/*
			 * Determine the size of this qTD transfer. By default,
			 * QT_BUFFER_CNT full pages can be used.
			 */
			int xfr_bytes = QT_BUFFER_CNT * EHCI_PAGE_SIZE;
			/*
			 * However, if the input buffer is not page-aligned, the
			 * portion of the first page before the buffer start
			 * offset within that page is unusable.
			 */
			xfr_bytes -= (uint32_t)buf_ptr & (EHCI_PAGE_SIZE - 1);
			/*
			 * In order to keep each packet within a qTD transfer,
			 * align the qTD transfer size to PKT_ALIGN.
			 */
			xfr_bytes &= ~(PKT_ALIGN - 1);
			/*
			 * This transfer may be shorter than the available qTD
			 * transfer size that has just been computed.
			 */
			xfr_bytes = min(xfr_bytes, left_length);

			/*
			 * Setup request qTD (3.5 in ehci-r10.pdf)
			 *
			 *   qt_next ................ 03-00 H
			 *   qt_altnext ............. 07-04 H
			 *   qt_token ............... 0B-08 H
			 *
			 *   [ buffer, buffer_hi ] loaded with "buffer".
			 */
			qtd[qtd_counter].qt_next =
					cpu_to_hc32(QT_NEXT_TERMINATE);
			qtd[qtd_counter].qt_altnext =
					cpu_to_hc32(QT_NEXT_TERMINATE);
			token = QT_TOKEN_DT(toggle) |
				QT_TOKEN_TOTALBYTES(xfr_bytes) |
				QT_TOKEN_IOC(req == NULL) | QT_TOKEN_CPAGE(0) |
				QT_TOKEN_CERR(3) |
				QT_TOKEN_PID(usb_pipein(pipe) ?
					QT_TOKEN_PID_IN : QT_TOKEN_PID_OUT) |
				QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
			qtd[qtd_counter].qt_token = cpu_to_hc32(token);
			if (ehci_td_buffer(&qtd[qtd_counter], buf_ptr,
						xfr_bytes)) {
				printf("unable to construct DATA TD\n");
				goto fail;
			}
			/* Update previous qTD! */
			*tdp = cpu_to_hc32((uint32_t)&qtd[qtd_counter]);
			tdp = &qtd[qtd_counter++].qt_next;
			/*
			 * Data toggle has to be adjusted since the qTD transfer
			 * size is not always an even multiple of
			 * wMaxPacketSize.
			 */
			if ((xfr_bytes / maxpacket) & 1)
				toggle ^= 1;
			buf_ptr += xfr_bytes;
			left_length -= xfr_bytes;
		} while (left_length > 0);
	}

	if (req != NULL) {
		/*
		 * Setup request qTD (3.5 in ehci-r10.pdf)
		 *
		 *   qt_next ................ 03-00 H
		 *   qt_altnext ............. 07-04 H
		 *   qt_token ............... 0B-08 H
		 */
		qtd[qtd_counter].qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
		qtd[qtd_counter].qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
		token = QT_TOKEN_DT(1) | QT_TOKEN_TOTALBYTES(0) |
			QT_TOKEN_IOC(1) | QT_TOKEN_CPAGE(0) | QT_TOKEN_CERR(3) |
			QT_TOKEN_PID(usb_pipein(pipe) ?
				QT_TOKEN_PID_OUT : QT_TOKEN_PID_IN) |
			QT_TOKEN_STATUS(QT_TOKEN_STATUS_ACTIVE);
		qtd[qtd_counter].qt_token = cpu_to_hc32(token);
		/* Update previous qTD! */
		*tdp = cpu_to_hc32((uint32_t)&qtd[qtd_counter]);
		tdp = &qtd[qtd_counter++].qt_next;
	}

	ctrl->qh_list.qh_link = cpu_to_hc32((uint32_t)qh | QH_LINK_TYPE_QH);

	/* Flush dcache */
	flush_dcache_range((uint32_t)&ctrl->qh_list,
		ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1));
	flush_dcache_range((uint32_t)qh, ALIGN_END_ADDR(struct QH, qh, 1));
	flush_dcache_range((uint32_t)qtd,
			   ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

	/* Set async. queue head pointer. */
	ehci_writel(&ctrl->hcor->or_asynclistaddr, (uint32_t)&ctrl->qh_list);

	usbsts = ehci_readl(&ctrl->hcor->or_usbsts);
	ehci_writel(&ctrl->hcor->or_usbsts, (usbsts & 0x3f));

	/* Enable async. schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd |= CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, STS_ASS,
			100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STS_ASS set\n");
		goto fail;
	}

	/* Wait for TDs to be processed. */
	ts = get_timer(0);
	vtd = &qtd[qtd_counter - 1];
	timeout = USB_TIMEOUT_MS(pipe);
	do {
		/* Invalidate dcache */
		invalidate_dcache_range((uint32_t)&ctrl->qh_list,
			ALIGN_END_ADDR(struct QH, &ctrl->qh_list, 1));
		invalidate_dcache_range((uint32_t)qh,
			ALIGN_END_ADDR(struct QH, qh, 1));
		invalidate_dcache_range((uint32_t)qtd,
			ALIGN_END_ADDR(struct qTD, qtd, qtd_count));

		token = hc32_to_cpu(vtd->qt_token);
		if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE))
			break;
		WATCHDOG_RESET();
	} while (get_timer(ts) < timeout);

	/*
	 * Invalidate the memory area occupied by buffer
	 * Don't try to fix the buffer alignment, if it isn't properly
	 * aligned it's upper layer's fault so let invalidate_dcache_range()
	 * vow about it. But we have to fix the length as it's actual
	 * transfer length and can be unaligned. This is potentially
	 * dangerous operation, it's responsibility of the calling
	 * code to make sure enough space is reserved.
	 */
	invalidate_dcache_range((uint32_t)buffer,
		ALIGN((uint32_t)buffer + length, ARCH_DMA_MINALIGN));

	/* Check that the TD processing happened */
	if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE)
		printf("EHCI timed out on TD - token=%#x\n", token);

	/* Disable async schedule. */
	cmd = ehci_readl(&ctrl->hcor->or_usbcmd);
	cmd &= ~CMD_ASE;
	ehci_writel(&ctrl->hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&ctrl->hcor->or_usbsts, STS_ASS, 0,
			100 * 1000);
	if (ret < 0) {
		printf("EHCI fail timeout STS_ASS reset\n");
		goto fail;
	}

	token = hc32_to_cpu(qh->qh_overlay.qt_token);
	if (!(QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_ACTIVE)) {
		debug("TOKEN=%#x\n", token);
		switch (QT_TOKEN_GET_STATUS(token) &
			~(QT_TOKEN_STATUS_SPLITXSTATE | QT_TOKEN_STATUS_PERR)) {
		case 0:
			toggle = QT_TOKEN_GET_DT(token);
			usb_settoggle(dev, usb_pipeendpoint(pipe),
				       usb_pipeout(pipe), toggle);
			dev->status = 0;
			break;
		case QT_TOKEN_STATUS_HALTED:
			dev->status = USB_ST_STALLED;
			break;
		case QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_STATUS_DATBUFERR:
		case QT_TOKEN_STATUS_DATBUFERR:
			dev->status = USB_ST_BUF_ERR;
			break;
		case QT_TOKEN_STATUS_HALTED | QT_TOKEN_STATUS_BABBLEDET:
		case QT_TOKEN_STATUS_BABBLEDET:
			dev->status = USB_ST_BABBLE_DET;
			break;
		default:
			dev->status = USB_ST_CRC_ERR;
			if (QT_TOKEN_GET_STATUS(token) & QT_TOKEN_STATUS_HALTED)
				dev->status |= USB_ST_STALLED;
			break;
		}
		dev->act_len = length - QT_TOKEN_GET_TOTALBYTES(token);
	} else {
		dev->act_len = 0;
#ifndef CONFIG_USB_EHCI_FARADAY
		debug("dev=%u, usbsts=%#x, p[1]=%#x, p[2]=%#x\n",
		      dev->devnum, ehci_readl(&ctrl->hcor->or_usbsts),
		      ehci_readl(&ctrl->hcor->or_portsc[0]),
		      ehci_readl(&ctrl->hcor->or_portsc[1]));
#endif
	}

	free(qtd);
	return (dev->status != USB_ST_NOT_PROC) ? 0 : -1;

fail:
	free(qtd);
	return -1;
}

__weak uint32_t *ehci_get_portsc_register(struct ehci_hcor *hcor, int port)
{
	if (port < 0 || port >= CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS) {
		/* Printing the message would cause a scan failure! */
		debug("The request port(%u) is not configured\n", port);
		return NULL;
	}

	return (uint32_t *)&hcor->or_portsc[port];
}

int
ehci_submit_root(struct usb_device *dev, unsigned long pipe, void *buffer,
		 int length, struct devrequest *req)
{
	uint8_t tmpbuf[4];
	u16 typeReq;
	void *srcptr = NULL;
	int len, srclen;
	uint32_t reg;
	uint32_t *status_reg;
	int port = le16_to_cpu(req->index) & 0xff;
	struct ehci_ctrl *ctrl = dev->controller;

	srclen = 0;

	debug("req=%u (%#x), type=%u (%#x), value=%u, index=%u\n",
	      req->request, req->request,
	      req->requesttype, req->requesttype,
	      le16_to_cpu(req->value), le16_to_cpu(req->index));

	typeReq = req->request | req->requesttype << 8;

	switch (typeReq) {
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		status_reg = ehci_get_portsc_register(ctrl->hcor, port - 1);
		if (!status_reg)
			return -1;
		break;
	default:
		status_reg = NULL;
		break;
	}

	switch (typeReq) {
	case DeviceRequest | USB_REQ_GET_DESCRIPTOR:
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_DEVICE:
			debug("USB_DT_DEVICE request\n");
			srcptr = &descriptor.device;
			srclen = descriptor.device.bLength;
			break;
		case USB_DT_CONFIG:
			debug("USB_DT_CONFIG config\n");
			srcptr = &descriptor.config;
			srclen = descriptor.config.bLength +
					descriptor.interface.bLength +
					descriptor.endpoint.bLength;
			break;
		case USB_DT_STRING:
			debug("USB_DT_STRING config\n");
			switch (le16_to_cpu(req->value) & 0xff) {
			case 0:	/* Language */
				srcptr = "\4\3\1\0";
				srclen = 4;
				break;
			case 1:	/* Vendor */
				srcptr = "\16\3u\0-\0b\0o\0o\0t\0";
				srclen = 14;
				break;
			case 2:	/* Product */
				srcptr = "\52\3E\0H\0C\0I\0 "
					 "\0H\0o\0s\0t\0 "
					 "\0C\0o\0n\0t\0r\0o\0l\0l\0e\0r\0";
				srclen = 42;
				break;
			default:
				debug("unknown value DT_STRING %x\n",
					le16_to_cpu(req->value));
				goto unknown;
			}
			break;
		default:
			debug("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_GET_DESCRIPTOR | ((USB_DIR_IN | USB_RT_HUB) << 8):
		switch (le16_to_cpu(req->value) >> 8) {
		case USB_DT_HUB:
			debug("USB_DT_HUB config\n");
			srcptr = &descriptor.hub;
			srclen = descriptor.hub.bLength;
			break;
		default:
			debug("unknown value %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		break;
	case USB_REQ_SET_ADDRESS | (USB_RECIP_DEVICE << 8):
		debug("USB_REQ_SET_ADDRESS\n");
		ctrl->rootdev = le16_to_cpu(req->value);
		break;
	case DeviceOutRequest | USB_REQ_SET_CONFIGURATION:
		debug("USB_REQ_SET_CONFIGURATION\n");
		/* Nothing to do */
		break;
	case USB_REQ_GET_STATUS | ((USB_DIR_IN | USB_RT_HUB) << 8):
		tmpbuf[0] = 1;	/* USB_STATUS_SELFPOWERED */
		tmpbuf[1] = 0;
		srcptr = tmpbuf;
		srclen = 2;
		break;
	case USB_REQ_GET_STATUS | ((USB_RT_PORT | USB_DIR_IN) << 8):
		memset(tmpbuf, 0, 4);
		reg = ehci_readl(status_reg);
		if (reg & EHCI_PS_CS)
			tmpbuf[0] |= USB_PORT_STAT_CONNECTION;
		if (reg & EHCI_PS_PE)
			tmpbuf[0] |= USB_PORT_STAT_ENABLE;
		if (reg & EHCI_PS_SUSP)
			tmpbuf[0] |= USB_PORT_STAT_SUSPEND;
		if (reg & EHCI_PS_OCA)
			tmpbuf[0] |= USB_PORT_STAT_OVERCURRENT;
		if (reg & EHCI_PS_PR)
			tmpbuf[0] |= USB_PORT_STAT_RESET;
		if (reg & EHCI_PS_PP)
			tmpbuf[1] |= USB_PORT_STAT_POWER >> 8;

		if (ehci_is_TDI()) {
			switch (ehci_get_port_speed(ctrl->hcor, reg)) {
			case PORTSC_PSPD_FS:
				break;
			case PORTSC_PSPD_LS:
				tmpbuf[1] |= USB_PORT_STAT_LOW_SPEED >> 8;
				break;
			case PORTSC_PSPD_HS:
			default:
				tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
				break;
			}
		} else {
			tmpbuf[1] |= USB_PORT_STAT_HIGH_SPEED >> 8;
		}

		if (reg & EHCI_PS_CSC)
			tmpbuf[2] |= USB_PORT_STAT_C_CONNECTION;
		if (reg & EHCI_PS_PEC)
			tmpbuf[2] |= USB_PORT_STAT_C_ENABLE;
		if (reg & EHCI_PS_OCC)
			tmpbuf[2] |= USB_PORT_STAT_C_OVERCURRENT;
		if (ctrl->portreset & (1 << port))
			tmpbuf[2] |= USB_PORT_STAT_C_RESET;

		srcptr = tmpbuf;
		srclen = 4;
		break;
	case USB_REQ_SET_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg |= EHCI_PS_PE;
			ehci_writel(status_reg, reg);
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&ctrl->hccr->cr_hcsparams))) {
				reg |= EHCI_PS_PP;
				ehci_writel(status_reg, reg);
			}
			break;
		case USB_PORT_FEAT_RESET:
			if ((reg & (EHCI_PS_PE | EHCI_PS_CS)) == EHCI_PS_CS &&
			    !ehci_is_TDI() &&
			    EHCI_PS_IS_LOWSPEED(reg)) {
				/* Low speed device, give up ownership. */
				debug("port %d low speed --> companion\n",
				      port - 1);
				reg |= EHCI_PS_PO;
				ehci_writel(status_reg, reg);
				break;
			} else {
				int ret;

				reg |= EHCI_PS_PR;
				reg &= ~EHCI_PS_PE;
				ehci_writel(status_reg, reg);
				/*
				 * caller must wait, then call GetPortStatus
				 * usb 2.0 specification say 50 ms resets on
				 * root
				 */
				ehci_powerup_fixup(status_reg, &reg);

				ehci_writel(status_reg, reg & ~EHCI_PS_PR);
				/*
				 * A host controller must terminate the reset
				 * and stabilize the state of the port within
				 * 2 milliseconds
				 */
				ret = handshake(status_reg, EHCI_PS_PR, 0,
						2 * 1000);
				if (!ret)
					ctrl->portreset |= 1 << port;
				else
					printf("port(%d) reset error\n",
					       port - 1);
			}
			break;
		case USB_PORT_FEAT_TEST:
			ehci_shutdown(ctrl);
			reg &= ~(0xf << 16);
			reg |= ((le16_to_cpu(req->index) >> 8) & 0xf) << 16;
			ehci_writel(status_reg, reg);
			break;
		default:
			debug("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		/* unblock posted writes */
		(void) ehci_readl(&ctrl->hcor->or_usbcmd);
		break;
	case USB_REQ_CLEAR_FEATURE | ((USB_DIR_OUT | USB_RT_PORT) << 8):
		reg = ehci_readl(status_reg);
		reg &= ~EHCI_PS_CLEAR;
		switch (le16_to_cpu(req->value)) {
		case USB_PORT_FEAT_ENABLE:
			reg &= ~EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_C_ENABLE:
			reg |= EHCI_PS_PE;
			break;
		case USB_PORT_FEAT_POWER:
			if (HCS_PPC(ehci_readl(&ctrl->hccr->cr_hcsparams)))
				reg &= ~EHCI_PS_PP;
			break;
		case USB_PORT_FEAT_C_CONNECTION:
			reg |= EHCI_PS_CSC;
			break;
		case USB_PORT_FEAT_OVER_CURRENT:
			reg |= EHCI_PS_OCC;
			break;
		case USB_PORT_FEAT_C_RESET:
			ctrl->portreset &= ~(1 << port);
			break;
		default:
			debug("unknown feature %x\n", le16_to_cpu(req->value));
			goto unknown;
		}
		ehci_writel(status_reg, reg);
		/* unblock posted write */
		(void) ehci_readl(&ctrl->hcor->or_usbcmd);
		break;
	default:
		debug("Unknown request\n");
		goto unknown;
	}

	mdelay(1);
	len = min3(srclen, le16_to_cpu(req->length), length);
	if (srcptr != NULL && len > 0)
		memcpy(buffer, srcptr, len);
	else
		debug("Len is 0\n");

	dev->act_len = len;
	dev->status = 0;
	return 0;

unknown:
	debug("requesttype=%x, request=%x, value=%x, index=%x, length=%x\n",
	      req->requesttype, req->request, le16_to_cpu(req->value),
	      le16_to_cpu(req->index), le16_to_cpu(req->length));

	dev->act_len = 0;
	dev->status = USB_ST_STALLED;
	return -1;
}

int usb_lowlevel_stop(int index)
{
	ehci_shutdown(&ehcic[index]);
	return ehci_hcd_stop(index);
}

int usb_lowlevel_init(int index, void **controller)
{
	uint32_t reg;
	uint32_t cmd;
	struct QH *qh_list;
	struct QH *periodic;
	int i;

	if (ehci_hcd_init(index, &ehcic[index].hccr, &ehcic[index].hcor))
		return -1;

	/* EHCI spec section 4.1 */
	if (ehci_reset(index))
		return -1;

#if defined(CONFIG_EHCI_HCD_INIT_AFTER_RESET)
	if (ehci_hcd_init(index, &ehcic[index].hccr, &ehcic[index].hcor))
		return -1;
#endif
	/* Set the high address word (aka segment) for 64-bit controller */
	if (ehci_readl(&ehcic[index].hccr->cr_hccparams) & 1)
		ehci_writel(ehcic[index].hcor->or_ctrldssegment, 0);

	qh_list = &ehcic[index].qh_list;

	/* Set head of reclaim list */
	memset(qh_list, 0, sizeof(*qh_list));
	qh_list->qh_link = cpu_to_hc32((uint32_t)qh_list | QH_LINK_TYPE_QH);
	qh_list->qh_endpt1 = cpu_to_hc32(QH_ENDPT1_H(1) |
						QH_ENDPT1_EPS(USB_SPEED_HIGH));
	qh_list->qh_curtd = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);
	qh_list->qh_overlay.qt_token =
			cpu_to_hc32(QT_TOKEN_STATUS(QT_TOKEN_STATUS_HALTED));

	flush_dcache_range((uint32_t)qh_list,
			   ALIGN_END_ADDR(struct QH, qh_list, 1));

	/* Set async. queue head pointer. */
	ehci_writel(&ehcic[index].hcor->or_asynclistaddr, (uint32_t)qh_list);

	/*
	 * Set up periodic list
	 * Step 1: Parent QH for all periodic transfers.
	 */
	periodic = &ehcic[index].periodic_queue;
	memset(periodic, 0, sizeof(*periodic));
	periodic->qh_link = cpu_to_hc32(QH_LINK_TERMINATE);
	periodic->qh_overlay.qt_next = cpu_to_hc32(QT_NEXT_TERMINATE);
	periodic->qh_overlay.qt_altnext = cpu_to_hc32(QT_NEXT_TERMINATE);

	flush_dcache_range((uint32_t)periodic,
			   ALIGN_END_ADDR(struct QH, periodic, 1));

	/*
	 * Step 2: Setup frame-list: Every microframe, USB tries the same list.
	 *         In particular, device specifications on polling frequency
	 *         are disregarded. Keyboards seem to send NAK/NYet reliably
	 *         when polled with an empty buffer.
	 *
	 *         Split Transactions will be spread across microframes using
	 *         S-mask and C-mask.
	 */
	if (ehcic[index].periodic_list == NULL)
		ehcic[index].periodic_list = memalign(4096, 1024 * 4);

	if (!ehcic[index].periodic_list)
		return -ENOMEM;
	for (i = 0; i < 1024; i++) {
		ehcic[index].periodic_list[i] = (uint32_t)periodic
						| QH_LINK_TYPE_QH;
	}

	flush_dcache_range((uint32_t)ehcic[index].periodic_list,
			   ALIGN_END_ADDR(uint32_t, ehcic[index].periodic_list,
					  1024));

	/* Set periodic list base address */
	ehci_writel(&ehcic[index].hcor->or_periodiclistbase,
		(uint32_t)ehcic[index].periodic_list);

	reg = ehci_readl(&ehcic[index].hccr->cr_hcsparams);
	descriptor.hub.bNbrPorts = HCS_N_PORTS(reg);
	debug("Register %x NbrPorts %d\n", reg, descriptor.hub.bNbrPorts);
	/* Port Indicators */
	if (HCS_INDICATOR(reg))
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
				| 0x80, &descriptor.hub.wHubCharacteristics);
	/* Port Power Control */
	if (HCS_PPC(reg))
		put_unaligned(get_unaligned(&descriptor.hub.wHubCharacteristics)
				| 0x01, &descriptor.hub.wHubCharacteristics);

	/* Start the host controller. */
	cmd = ehci_readl(&ehcic[index].hcor->or_usbcmd);
	/*
	 * Philips, Intel, and maybe others need CMD_RUN before the
	 * root hub will detect new devices (why?); NEC doesn't
	 */
	cmd &= ~(CMD_LRESET|CMD_IAAD|CMD_PSE|CMD_ASE|CMD_RESET);
	cmd |= CMD_RUN;
	ehci_writel(&ehcic[index].hcor->or_usbcmd, cmd);

#ifndef CONFIG_USB_EHCI_FARADAY
	/* take control over the ports */
	cmd = ehci_readl(&ehcic[index].hcor->or_configflag);
	cmd |= FLAG_CF;
	ehci_writel(&ehcic[index].hcor->or_configflag, cmd);
#endif

	/* unblock posted write */
	cmd = ehci_readl(&ehcic[index].hcor->or_usbcmd);
	mdelay(5);
	reg = HC_VERSION(ehci_readl(&ehcic[index].hccr->cr_capbase));
	printf("USB EHCI %x.%02x\n", reg >> 8, reg & 0xff);

	ehcic[index].rootdev = 0;

	*controller = &ehcic[index];
	return 0;
}

int
submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		int length)
{

	if (usb_pipetype(pipe) != PIPE_BULK) {
		debug("non-bulk pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}
	return ehci_submit_async(dev, pipe, buffer, length, NULL);
}

int
submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int length, struct devrequest *setup)
{
	struct ehci_ctrl *ctrl = dev->controller;

	if (usb_pipetype(pipe) != PIPE_CONTROL) {
		debug("non-control pipe (type=%lu)", usb_pipetype(pipe));
		return -1;
	}

	if (usb_pipedevice(pipe) == ctrl->rootdev) {
		if (!ctrl->rootdev)
			dev->speed = USB_SPEED_HIGH;
		return ehci_submit_root(dev, pipe, buffer, length, setup);
	}
	return ehci_submit_async(dev, pipe, buffer, length, setup);
}

struct int_queue {
	struct QH *first;
	struct QH *current;
	struct QH *last;
	struct qTD *tds;
};

#define NEXT_QH(qh) (struct QH *)((qh)->qh_link & ~0x1f)

static int
enable_periodic(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	struct ehci_hcor *hcor = ctrl->hcor;
	int ret;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd |= CMD_PSE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts,
			STS_PSS, STS_PSS, 100 * 1000);
	if (ret < 0) {
		printf("EHCI failed: timeout when enabling periodic list\n");
		return -ETIMEDOUT;
	}
	udelay(1000);
	return 0;
}

static int
disable_periodic(struct ehci_ctrl *ctrl)
{
	uint32_t cmd;
	struct ehci_hcor *hcor = ctrl->hcor;
	int ret;

	cmd = ehci_readl(&hcor->or_usbcmd);
	cmd &= ~CMD_PSE;
	ehci_writel(&hcor->or_usbcmd, cmd);

	ret = handshake((uint32_t *)&hcor->or_usbsts,
			STS_PSS, 0, 100 * 1000);
	if (ret < 0) {
		printf("EHCI failed: timeout when disabling periodic list\n");
		return -ETIMEDOUT;
	}
	return 0;
}

static int periodic_schedules;

struct int_queue *
create_int_queue(struct usb_device *dev, unsigned long pipe, int queuesize,
		 int elementsize, void *buffer)
{
	struct ehci_ctrl *ctrl = dev->controller;
	struct int_queue *result = NULL;
	int i;

	debug("Enter create_int_queue\n");
	if (usb_pipetype(pipe) != PIPE_INTERRUPT) {
		debug("non-interrupt pipe (type=%lu)", usb_pipetype(pipe));
		return NULL;
	}

	/* limit to 4 full pages worth of data -
	 * we can safely fit them in a single TD,
	 * no matter the alignment
	 */
	if (elementsize >= 16384) {
		debug("too large elements for interrupt transfers\n");
		return NULL;
	}

	result = malloc(sizeof(*result));
	if (!result) {
		debug("ehci intr queue: out of memory\n");
		goto fail1;
	}
	result->first = memalign(32, sizeof(struct QH) * queuesize);
	if (!result->first) {
		debug("ehci intr queue: out of memory\n");
		goto fail2;
	}
	result->current = result->first;
	result->last = result->first + queuesize - 1;
	result->tds = memalign(32, sizeof(struct qTD) * queuesize);
	if (!result->tds) {
		debug("ehci intr queue: out of memory\n");
		goto fail3;
	}
	memset(result->first, 0, sizeof(struct QH) * queuesize);
	memset(result->tds, 0, sizeof(struct qTD) * queuesize);

	for (i = 0; i < queuesize; i++) {
		struct QH *qh = result->first + i;
		struct qTD *td = result->tds + i;
		void **buf = &qh->buffer;

		qh->qh_link = (uint32_t)(qh+1) | QH_LINK_TYPE_QH;
		if (i == queuesize - 1)
			qh->qh_link = QH_LINK_TERMINATE;

		qh->qh_overlay.qt_next = (uint32_t)td;
		qh->qh_endpt1 = (0 << 28) | /* No NAK reload (ehci 4.9) */
			(usb_maxpacket(dev, pipe) << 16) | /* MPS */
			(1 << 14) |
			QH_ENDPT1_EPS(ehci_encode_speed(dev->speed)) |
			(usb_pipeendpoint(pipe) << 8) | /* Endpoint Number */
			(usb_pipedevice(pipe) << 0);
		qh->qh_endpt2 = (1 << 30) | /* 1 Tx per mframe */
			(1 << 0); /* S-mask: microframe 0 */
		if (dev->speed == USB_SPEED_LOW ||
				dev->speed == USB_SPEED_FULL) {
			debug("TT: port: %d, hub address: %d\n",
				dev->portnr, dev->parent->devnum);
			qh->qh_endpt2 |= (dev->portnr << 23) |
				(dev->parent->devnum << 16) |
				(0x1c << 8); /* C-mask: microframes 2-4 */
		}

		td->qt_next = QT_NEXT_TERMINATE;
		td->qt_altnext = QT_NEXT_TERMINATE;
		debug("communication direction is '%s'\n",
		      usb_pipein(pipe) ? "in" : "out");
		td->qt_token = (elementsize << 16) |
			((usb_pipein(pipe) ? 1 : 0) << 8) | /* IN/OUT token */
			0x80; /* active */
		td->qt_buffer[0] = (uint32_t)buffer + i * elementsize;
		td->qt_buffer[1] = (td->qt_buffer[0] + 0x1000) & ~0xfff;
		td->qt_buffer[2] = (td->qt_buffer[0] + 0x2000) & ~0xfff;
		td->qt_buffer[3] = (td->qt_buffer[0] + 0x3000) & ~0xfff;
		td->qt_buffer[4] = (td->qt_buffer[0] + 0x4000) & ~0xfff;

		*buf = buffer + i * elementsize;
	}

	flush_dcache_range((uint32_t)buffer,
			   ALIGN_END_ADDR(char, buffer,
					  queuesize * elementsize));
	flush_dcache_range((uint32_t)result->first,
			   ALIGN_END_ADDR(struct QH, result->first,
					  queuesize));
	flush_dcache_range((uint32_t)result->tds,
			   ALIGN_END_ADDR(struct qTD, result->tds,
					  queuesize));

	if (disable_periodic(ctrl) < 0) {
		debug("FATAL: periodic should never fail, but did");
		goto fail3;
	}

	/* hook up to periodic list */
	struct QH *list = &ctrl->periodic_queue;
	result->last->qh_link = list->qh_link;
	list->qh_link = (uint32_t)result->first | QH_LINK_TYPE_QH;

	flush_dcache_range((uint32_t)result->last,
			   ALIGN_END_ADDR(struct QH, result->last, 1));
	flush_dcache_range((uint32_t)list,
			   ALIGN_END_ADDR(struct QH, list, 1));

	if (enable_periodic(ctrl) < 0) {
		debug("FATAL: periodic should never fail, but did");
		goto fail3;
	}
	periodic_schedules++;

	debug("Exit create_int_queue\n");
	return result;
fail3:
	if (result->tds)
		free(result->tds);
fail2:
	if (result->first)
		free(result->first);
	if (result)
		free(result);
fail1:
	return NULL;
}

void *poll_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	struct QH *cur = queue->current;

	/* depleted queue */
	if (cur == NULL) {
		debug("Exit poll_int_queue with completed queue\n");
		return NULL;
	}
	/* still active */
	invalidate_dcache_range((uint32_t)cur,
				ALIGN_END_ADDR(struct QH, cur, 1));
	if (cur->qh_overlay.qt_token & 0x80) {
		debug("Exit poll_int_queue with no completed intr transfer. "
		      "token is %x\n", cur->qh_overlay.qt_token);
		return NULL;
	}
	if (!(cur->qh_link & QH_LINK_TERMINATE))
		queue->current++;
	else
		queue->current = NULL;
	debug("Exit poll_int_queue with completed intr transfer. "
	      "token is %x at %p (first at %p)\n", cur->qh_overlay.qt_token,
	      &cur->qh_overlay.qt_token, queue->first);
	return cur->buffer;
}

/* Do not free buffers associated with QHs, they're owned by someone else */
int
destroy_int_queue(struct usb_device *dev, struct int_queue *queue)
{
	struct ehci_ctrl *ctrl = dev->controller;
	int result = -1;
	unsigned long timeout;

	if (disable_periodic(ctrl) < 0) {
		debug("FATAL: periodic should never fail, but did");
		goto out;
	}
	periodic_schedules--;

	struct QH *cur = &ctrl->periodic_queue;
	timeout = get_timer(0) + 500; /* abort after 500ms */
	while (!(cur->qh_link & QH_LINK_TERMINATE)) {
		debug("considering %p, with qh_link %x\n", cur, cur->qh_link);
		if (NEXT_QH(cur) == queue->first) {
			debug("found candidate. removing from chain\n");
			cur->qh_link = queue->last->qh_link;
			result = 0;
			break;
		}
		cur = NEXT_QH(cur);
		if (get_timer(0) > timeout) {
			printf("Timeout destroying interrupt endpoint queue\n");
			result = -1;
			goto out;
		}
	}

	if (periodic_schedules > 0) {
		result = enable_periodic(ctrl);
		if (result < 0)
			debug("FATAL: periodic should never fail, but did");
	}

out:
	free(queue->tds);
	free(queue->first);
	free(queue);

	return result;
}

int
submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
	       int length, int interval)
{
	void *backbuffer;
	struct int_queue *queue;
	unsigned long timeout;
	int result = 0, ret;

	debug("dev=%p, pipe=%lu, buffer=%p, length=%d, interval=%d",
	      dev, pipe, buffer, length, interval);

	/*
	 * Interrupt transfers requiring several transactions are not supported
	 * because bInterval is ignored.
	 *
	 * Also, ehci_submit_async() relies on wMaxPacketSize being a power of 2
	 * <= PKT_ALIGN if several qTDs are required, while the USB
	 * specification does not constrain this for interrupt transfers. That
	 * means that ehci_submit_async() would support interrupt transfers
	 * requiring several transactions only as long as the transfer size does
	 * not require more than a single qTD.
	 */
	if (length > usb_maxpacket(dev, pipe)) {
		printf("%s: Interrupt transfers requiring several "
			"transactions are not supported.\n", __func__);
		return -1;
	}

	queue = create_int_queue(dev, pipe, 1, length, buffer);

	timeout = get_timer(0) + USB_TIMEOUT_MS(pipe);
	while ((backbuffer = poll_int_queue(dev, queue)) == NULL)
		if (get_timer(0) > timeout) {
			printf("Timeout poll on interrupt endpoint\n");
			result = -ETIMEDOUT;
			break;
		}

	if (backbuffer != buffer) {
		debug("got wrong buffer back (%x instead of %x)\n",
		      (uint32_t)backbuffer, (uint32_t)buffer);
		return -EINVAL;
	}

	invalidate_dcache_range((uint32_t)buffer,
				ALIGN_END_ADDR(char, buffer, length));

	ret = destroy_int_queue(dev, queue);
	if (ret < 0)
		return ret;

	/* everything worked out fine */
	return result;
}
