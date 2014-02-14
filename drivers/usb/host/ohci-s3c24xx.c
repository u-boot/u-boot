/*
 * URB OHCI HCD (Host Controller Driver) for USB on the S3C2400.
 *
 * (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * Note: Much of this code has been derived from Linux 2.4
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2002 David Brownell
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
/*
 * IMPORTANT NOTES
 * 1 - this driver is intended for use with USB Mass Storage Devices
 *     (BBB) ONLY. There is NO support for Interrupt or Isochronous pipes!
 */

#include <common.h>
/* #include <pci.h> no PCI on the S3C24X0 */

#if defined(CONFIG_USB_OHCI) && defined(CONFIG_S3C24X0)

#include <asm/arch/s3c24x0_cpu.h>
#include <asm/io.h>
#include <malloc.h>
#include <usb.h>
#include "ohci-s3c24xx.h"

#define OHCI_USE_NPS		/* force NoPowerSwitching mode */
#undef OHCI_VERBOSE_DEBUG	/* not always helpful */


/* For initializing controller (mask in an HCFS mode too) */
#define	OHCI_CONTROL_INIT \
	(OHCI_CTRL_CBSR & 0x3) | OHCI_CTRL_IE | OHCI_CTRL_PLE

#define min_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x : __y; })

#undef DEBUG
#ifdef DEBUG
#define dbg(format, arg...) printf("DEBUG: " format "\n", ## arg)
#else
#define dbg(format, arg...) do {} while(0)
#endif /* DEBUG */
#define err(format, arg...) printf("ERROR: " format "\n", ## arg)
#undef SHOW_INFO
#ifdef SHOW_INFO
#define info(format, arg...) printf("INFO: " format "\n", ## arg)
#else
#define info(format, arg...) do {} while(0)
#endif

#define m16_swap(x) swap_16(x)
#define m32_swap(x) swap_32(x)

/* global struct ohci */
static struct ohci gohci;
/* this must be aligned to a 256 byte boundary */
struct ohci_hcca ghcca[1];
/* a pointer to the aligned storage */
struct ohci_hcca *phcca;
/* this allocates EDs for all possible endpoints */
struct ohci_device ohci_dev;
/* urb_priv */
struct urb_priv urb_priv;
/* RHSC flag */
int got_rhsc;
/* device which was disconnected */
struct usb_device *devgone;
/* flag guarding URB transation */
int urb_finished = 0;

/*-------------------------------------------------------------------------*/

/* AMD-756 (D2 rev) reports corrupt register contents in some cases.
 * The erratum (#4) description is incorrect.  AMD's workaround waits
 * till some bits (mostly reserved) are clear; ok for all revs.
 */
#define OHCI_QUIRK_AMD756 0xabcd
#define read_roothub(hc, register, mask) ({ \
	u32 temp = readl (&hc->regs->roothub.register); \
	if (hc->flags & OHCI_QUIRK_AMD756) \
		while (temp & mask) \
			temp = readl (&hc->regs->roothub.register); \
	temp; })

static u32 roothub_a(struct ohci *hc)
{
	return read_roothub(hc, a, 0xfc0fe000);
}
static inline u32 roothub_b(struct ohci *hc)
{
	return readl(&hc->regs->roothub.b);
}
static inline u32 roothub_status(struct ohci *hc)
{
	return readl(&hc->regs->roothub.status);
}
static u32 roothub_portstatus(struct ohci *hc, int i)
{
	return read_roothub(hc, portstatus[i], 0xffe0fce0);
}

/* forward declaration */
static int hc_interrupt(void);
static void td_submit_job(struct usb_device *dev, unsigned long pipe,
			  void *buffer, int transfer_len,
			  struct devrequest *setup, struct urb_priv *urb,
			  int interval);

/*-------------------------------------------------------------------------*
 * URB support functions
 *-------------------------------------------------------------------------*/

/* free HCD-private data associated with this URB */

static void urb_free_priv(struct urb_priv *urb)
{
	int i;
	int last;
	struct td *td;

	last = urb->length - 1;
	if (last >= 0) {
		for (i = 0; i <= last; i++) {
			td = urb->td[i];
			if (td) {
				td->usb_dev = NULL;
				urb->td[i] = NULL;
			}
		}
	}
}

/*-------------------------------------------------------------------------*/

#ifdef DEBUG
static int sohci_get_current_frame_number(struct usb_device *dev);

/* debug| print the main components of an URB
 * small: 0) header + data packets 1) just header */

static void pkt_print(struct usb_device *dev, unsigned long pipe, void *buffer,
		      int transfer_len, struct devrequest *setup, char *str,
		      int small)
{
	struct urb_priv *purb = &urb_priv;

	dbg("%s URB:[%4x] dev:%2d,ep:%2d-%c,type:%s,len:%d/%d stat:%#lx",
	    str,
	    sohci_get_current_frame_number(dev),
	    usb_pipedevice(pipe),
	    usb_pipeendpoint(pipe),
	    usb_pipeout(pipe) ? 'O' : 'I',
	    usb_pipetype(pipe) < 2 ?
		(usb_pipeint(pipe) ? "INTR" : "ISOC") :
		(usb_pipecontrol(pipe) ? "CTRL" : "BULK"),
	    purb->actual_length, transfer_len, dev->status);
#ifdef	OHCI_VERBOSE_DEBUG
	if (!small) {
		int i, len;

		if (usb_pipecontrol(pipe)) {
			printf(__FILE__ ": cmd(8):");
			for (i = 0; i < 8; i++)
				printf(" %02x", ((__u8 *) setup)[i]);
			printf("\n");
		}
		if (transfer_len > 0 && buffer) {
			printf(__FILE__ ": data(%d/%d):",
			       purb->actual_length, transfer_len);
			len = usb_pipeout(pipe) ?
			    transfer_len : purb->actual_length;
			for (i = 0; i < 16 && i < len; i++)
				printf(" %02x", ((__u8 *) buffer)[i]);
			printf("%s\n", i < len ? "..." : "");
		}
	}
#endif
}

/* just for debugging; prints non-empty branches of the
   int ed tree inclusive iso eds*/
void ep_print_int_eds(struct ohci *ohci, char *str)
{
	int i, j;
	__u32 *ed_p;
	for (i = 0; i < 32; i++) {
		j = 5;
		ed_p = &(ohci->hcca->int_table[i]);
		if (*ed_p == 0)
			continue;
		printf(__FILE__ ": %s branch int %2d(%2x):", str, i, i);
		while (*ed_p != 0 && j--) {
			struct ed *ed = (struct ed *) m32_swap(ed_p);
			printf(" ed: %4x;", ed->hwINFO);
			ed_p = &ed->hwNextED;
		}
		printf("\n");
	}
}

static void ohci_dump_intr_mask(char *label, __u32 mask)
{
	dbg("%s: 0x%08x%s%s%s%s%s%s%s%s%s",
	    label,
	    mask,
	    (mask & OHCI_INTR_MIE) ? " MIE" : "",
	    (mask & OHCI_INTR_OC) ? " OC" : "",
	    (mask & OHCI_INTR_RHSC) ? " RHSC" : "",
	    (mask & OHCI_INTR_FNO) ? " FNO" : "",
	    (mask & OHCI_INTR_UE) ? " UE" : "",
	    (mask & OHCI_INTR_RD) ? " RD" : "",
	    (mask & OHCI_INTR_SF) ? " SF" : "",
	    (mask & OHCI_INTR_WDH) ? " WDH" : "",
	    (mask & OHCI_INTR_SO) ? " SO" : "");
}

static void maybe_print_eds(char *label, __u32 value)
{
	struct ed *edp = (struct ed *) value;

	if (value) {
		dbg("%s %08x", label, value);
		dbg("%08x", edp->hwINFO);
		dbg("%08x", edp->hwTailP);
		dbg("%08x", edp->hwHeadP);
		dbg("%08x", edp->hwNextED);
	}
}

static char *hcfs2string(int state)
{
	switch (state) {
	case OHCI_USB_RESET:
		return "reset";
	case OHCI_USB_RESUME:
		return "resume";
	case OHCI_USB_OPER:
		return "operational";
	case OHCI_USB_SUSPEND:
		return "suspend";
	}
	return "?";
}

/* dump control and status registers */
static void ohci_dump_status(struct ohci *controller)
{
	struct ohci_regs *regs = controller->regs;
	__u32 temp;

	temp = readl(&regs->revision) & 0xff;
	if (temp != 0x10)
		dbg("spec %d.%d", (temp >> 4), (temp & 0x0f));

	temp = readl(&regs->control);
	dbg("control: 0x%08x%s%s%s HCFS=%s%s%s%s%s CBSR=%d", temp,
	    (temp & OHCI_CTRL_RWE) ? " RWE" : "",
	    (temp & OHCI_CTRL_RWC) ? " RWC" : "",
	    (temp & OHCI_CTRL_IR) ? " IR" : "",
	    hcfs2string(temp & OHCI_CTRL_HCFS),
	    (temp & OHCI_CTRL_BLE) ? " BLE" : "",
	    (temp & OHCI_CTRL_CLE) ? " CLE" : "",
	    (temp & OHCI_CTRL_IE) ? " IE" : "",
	    (temp & OHCI_CTRL_PLE) ? " PLE" : "", temp & OHCI_CTRL_CBSR);

	temp = readl(&regs->cmdstatus);
	dbg("cmdstatus: 0x%08x SOC=%d%s%s%s%s", temp,
	    (temp & OHCI_SOC) >> 16,
	    (temp & OHCI_OCR) ? " OCR" : "",
	    (temp & OHCI_BLF) ? " BLF" : "",
	    (temp & OHCI_CLF) ? " CLF" : "", (temp & OHCI_HCR) ? " HCR" : "");

	ohci_dump_intr_mask("intrstatus", readl(&regs->intrstatus));
	ohci_dump_intr_mask("intrenable", readl(&regs->intrenable));

	maybe_print_eds("ed_periodcurrent", readl(&regs->ed_periodcurrent));

	maybe_print_eds("ed_controlhead", readl(&regs->ed_controlhead));
	maybe_print_eds("ed_controlcurrent", readl(&regs->ed_controlcurrent));

	maybe_print_eds("ed_bulkhead", readl(&regs->ed_bulkhead));
	maybe_print_eds("ed_bulkcurrent", readl(&regs->ed_bulkcurrent));

	maybe_print_eds("donehead", readl(&regs->donehead));
}

static void ohci_dump_roothub(struct ohci *controller, int verbose)
{
	__u32 temp, ndp, i;

	temp = roothub_a(controller);
	ndp = (temp & RH_A_NDP);

	if (verbose) {
		dbg("roothub.a: %08x POTPGT=%d%s%s%s%s%s NDP=%d", temp,
		    ((temp & RH_A_POTPGT) >> 24) & 0xff,
		    (temp & RH_A_NOCP) ? " NOCP" : "",
		    (temp & RH_A_OCPM) ? " OCPM" : "",
		    (temp & RH_A_DT) ? " DT" : "",
		    (temp & RH_A_NPS) ? " NPS" : "",
		    (temp & RH_A_PSM) ? " PSM" : "", ndp);
		temp = roothub_b(controller);
		dbg("roothub.b: %08x PPCM=%04x DR=%04x",
		    temp, (temp & RH_B_PPCM) >> 16, (temp & RH_B_DR)
		    );
		temp = roothub_status(controller);
		dbg("roothub.status: %08x%s%s%s%s%s%s",
		    temp,
		    (temp & RH_HS_CRWE) ? " CRWE" : "",
		    (temp & RH_HS_OCIC) ? " OCIC" : "",
		    (temp & RH_HS_LPSC) ? " LPSC" : "",
		    (temp & RH_HS_DRWE) ? " DRWE" : "",
		    (temp & RH_HS_OCI) ? " OCI" : "",
		    (temp & RH_HS_LPS) ? " LPS" : "");
	}

	for (i = 0; i < ndp; i++) {
		temp = roothub_portstatus(controller, i);
		dbg("roothub.portstatus [%d] = 0x%08x%s%s%s%s%s%s%s%s%s%s%s%s",
		    i,
		    temp,
		    (temp & RH_PS_PRSC) ? " PRSC" : "",
		    (temp & RH_PS_OCIC) ? " OCIC" : "",
		    (temp & RH_PS_PSSC) ? " PSSC" : "",
		    (temp & RH_PS_PESC) ? " PESC" : "",
		    (temp & RH_PS_CSC) ? " CSC" : "",
		    (temp & RH_PS_LSDA) ? " LSDA" : "",
		    (temp & RH_PS_PPS) ? " PPS" : "",
		    (temp & RH_PS_PRS) ? " PRS" : "",
		    (temp & RH_PS_POCI) ? " POCI" : "",
		    (temp & RH_PS_PSS) ? " PSS" : "",
		    (temp & RH_PS_PES) ? " PES" : "",
		    (temp & RH_PS_CCS) ? " CCS" : "");
	}
}

static void ohci_dump(struct ohci *controller, int verbose)
{
	dbg("OHCI controller usb-%s state", controller->slot_name);

	/* dumps some of the state we know about */
	ohci_dump_status(controller);
	if (verbose)
		ep_print_int_eds(controller, "hcca");
	dbg("hcca frame #%04x", controller->hcca->frame_no);
	ohci_dump_roothub(controller, 1);
}

#endif /* DEBUG */

/*-------------------------------------------------------------------------*
 * Interface functions (URB)
 *-------------------------------------------------------------------------*/

/* get a transfer request */

int sohci_submit_job(struct usb_device *dev, unsigned long pipe, void *buffer,
		     int transfer_len, struct devrequest *setup, int interval)
{
	struct ohci *ohci;
	struct ed *ed;
	struct urb_priv *purb_priv;
	int i, size = 0;

	ohci = &gohci;

	/* when controller's hung, permit only roothub cleanup attempts
	 * such as powering down ports */
	if (ohci->disabled) {
		err("sohci_submit_job: EPIPE");
		return -1;
	}

	/* if we have an unfinished URB from previous transaction let's
	 * fail and scream as quickly as possible so as not to corrupt
	 * further communication */
	if (!urb_finished) {
		err("sohci_submit_job: URB NOT FINISHED");
		return -1;
	}
	/* we're about to begin a new transaction here
	   so mark the URB unfinished */
	urb_finished = 0;

	/* every endpoint has a ed, locate and fill it */
	ed = ep_add_ed(dev, pipe);
	if (!ed) {
		err("sohci_submit_job: ENOMEM");
		return -1;
	}

	/* for the private part of the URB we need the number of TDs (size) */
	switch (usb_pipetype(pipe)) {
	case PIPE_BULK:
		/* one TD for every 4096 Byte */
		size = (transfer_len - 1) / 4096 + 1;
		break;
	case PIPE_CONTROL:
		/* 1 TD for setup, 1 for ACK and 1 for every 4096 B */
		size = (transfer_len == 0) ? 2 : (transfer_len - 1) / 4096 + 3;
		break;
	}

	if (size >= (N_URB_TD - 1)) {
		err("need %d TDs, only have %d", size, N_URB_TD);
		return -1;
	}
	purb_priv = &urb_priv;
	purb_priv->pipe = pipe;

	/* fill the private part of the URB */
	purb_priv->length = size;
	purb_priv->ed = ed;
	purb_priv->actual_length = 0;

	/* allocate the TDs */
	/* note that td[0] was allocated in ep_add_ed */
	for (i = 0; i < size; i++) {
		purb_priv->td[i] = td_alloc(dev);
		if (!purb_priv->td[i]) {
			purb_priv->length = i;
			urb_free_priv(purb_priv);
			err("sohci_submit_job: ENOMEM");
			return -1;
		}
	}

	if (ed->state == ED_NEW || (ed->state & ED_DEL)) {
		urb_free_priv(purb_priv);
		err("sohci_submit_job: EINVAL");
		return -1;
	}

	/* link the ed into a chain if is not already */
	if (ed->state != ED_OPER)
		ep_link(ohci, ed);

	/* fill the TDs and link it to the ed */
	td_submit_job(dev, pipe, buffer, transfer_len, setup, purb_priv,
		      interval);

	return 0;
}

/*-------------------------------------------------------------------------*/

#ifdef DEBUG
/* tell us the current USB frame number */

static int sohci_get_current_frame_number(struct usb_device *usb_dev)
{
	struct ohci *ohci = &gohci;

	return m16_swap(ohci->hcca->frame_no);
}
#endif

/*-------------------------------------------------------------------------*
 * ED handling functions
 *-------------------------------------------------------------------------*/

/* link an ed into one of the HC chains */

static int ep_link(struct ohci *ohci, struct ed *edi)
{
	struct ed *ed = edi;

	ed->state = ED_OPER;

	switch (ed->type) {
	case PIPE_CONTROL:
		ed->hwNextED = 0;
		if (ohci->ed_controltail == NULL) {
			writel((u32)ed, &ohci->regs->ed_controlhead);
		} else {
			ohci->ed_controltail->hwNextED = (__u32) m32_swap(ed);
		}
		ed->ed_prev = ohci->ed_controltail;
		if (!ohci->ed_controltail && !ohci->ed_rm_list[0] &&
		    !ohci->ed_rm_list[1] && !ohci->sleeping) {
			ohci->hc_control |= OHCI_CTRL_CLE;
			writel(ohci->hc_control, &ohci->regs->control);
		}
		ohci->ed_controltail = edi;
		break;

	case PIPE_BULK:
		ed->hwNextED = 0;
		if (ohci->ed_bulktail == NULL) {
			writel((u32)ed, &ohci->regs->ed_bulkhead);
		} else {
			ohci->ed_bulktail->hwNextED = (__u32) m32_swap(ed);
		}
		ed->ed_prev = ohci->ed_bulktail;
		if (!ohci->ed_bulktail && !ohci->ed_rm_list[0] &&
		    !ohci->ed_rm_list[1] && !ohci->sleeping) {
			ohci->hc_control |= OHCI_CTRL_BLE;
			writel(ohci->hc_control, &ohci->regs->control);
		}
		ohci->ed_bulktail = edi;
		break;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* unlink an ed from one of the HC chains.
 * just the link to the ed is unlinked.
 * the link from the ed still points to another operational ed or 0
 * so the HC can eventually finish the processing of the unlinked ed */

static int ep_unlink(struct ohci *ohci, struct ed *ed)
{
	struct ed *next;
	ed->hwINFO |= m32_swap(OHCI_ED_SKIP);

	switch (ed->type) {
	case PIPE_CONTROL:
		if (ed->ed_prev == NULL) {
			if (!ed->hwNextED) {
				ohci->hc_control &= ~OHCI_CTRL_CLE;
				writel(ohci->hc_control, &ohci->regs->control);
			}
			writel(m32_swap(*((__u32 *) &ed->hwNextED)),
			       &ohci->regs->ed_controlhead);
		} else {
			ed->ed_prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_controltail == ed) {
			ohci->ed_controltail = ed->ed_prev;
		} else {
			next = (struct ed *)m32_swap(*((__u32 *)&ed->hwNextED));
			next->ed_prev = ed->ed_prev;
		}
		break;

	case PIPE_BULK:
		if (ed->ed_prev == NULL) {
			if (!ed->hwNextED) {
				ohci->hc_control &= ~OHCI_CTRL_BLE;
				writel(ohci->hc_control, &ohci->regs->control);
			}
			writel(m32_swap(*((__u32 *) &ed->hwNextED)),
			       &ohci->regs->ed_bulkhead);
		} else {
			ed->ed_prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_bulktail == ed) {
			ohci->ed_bulktail = ed->ed_prev;
		} else {
			next = (struct ed *)m32_swap(*((__u32 *)&ed->hwNextED));
			next->ed_prev = ed->ed_prev;
		}
		break;
	}
	ed->state = ED_UNLINK;
	return 0;
}

/*-------------------------------------------------------------------------*/

/* add/reinit an endpoint; this should be done once at the usb_set_configuration
 * command, but the USB stack is a little bit stateless  so we do it at every
 * transaction. If the state of the ed is ED_NEW then a dummy td is added and
 * the state is changed to ED_UNLINK. In all other cases the state is left
 * unchanged. The ed info fields are setted anyway even though most of them
 * should not change */

static struct ed *ep_add_ed(struct usb_device *usb_dev, unsigned long pipe)
{
	struct td *td;
	struct ed *ed_ret;
	struct ed *ed;

	ed = ed_ret = &ohci_dev.ed[(usb_pipeendpoint(pipe) << 1) |
				   (usb_pipecontrol(pipe) ? 0 :
				    usb_pipeout(pipe))];

	if ((ed->state & ED_DEL) || (ed->state & ED_URB_DEL)) {
		err("ep_add_ed: pending delete");
		/* pending delete request */
		return NULL;
	}

	if (ed->state == ED_NEW) {
		ed->hwINFO = m32_swap(OHCI_ED_SKIP);	/* skip ed */
		/* dummy td; end of td list for ed */
		td = td_alloc(usb_dev);
		ed->hwTailP = (__u32) m32_swap(td);
		ed->hwHeadP = ed->hwTailP;
		ed->state = ED_UNLINK;
		ed->type = usb_pipetype(pipe);
		ohci_dev.ed_cnt++;
	}

	ed->hwINFO = m32_swap(usb_pipedevice(pipe)
			      | usb_pipeendpoint(pipe) << 7
			      | (usb_pipeisoc(pipe) ? 0x8000 : 0)
			      | (usb_pipecontrol(pipe) ? 0 :
				 (usb_pipeout(pipe) ? 0x800 : 0x1000))
			      | (usb_dev->speed == USB_SPEED_LOW) << 13 |
			      usb_maxpacket(usb_dev, pipe) << 16);

	return ed_ret;
}

/*-------------------------------------------------------------------------*
 * TD handling functions
 *-------------------------------------------------------------------------*/

/* enqueue next TD for this URB (OHCI spec 5.2.8.2) */

static void td_fill(struct ohci *ohci, unsigned int info, void *data, int len,
		    struct usb_device *dev, int index,
		    struct urb_priv *urb_priv)
{
	struct td *td, *td_pt;
#ifdef OHCI_FILL_TRACE
	int i;
#endif

	if (index > urb_priv->length) {
		err("index > length");
		return;
	}
	/* use this td as the next dummy */
	td_pt = urb_priv->td[index];
	td_pt->hwNextTD = 0;

	/* fill the old dummy TD */
	td = urb_priv->td[index] =
	    (struct td *) (m32_swap(urb_priv->ed->hwTailP) & ~0xf);

	td->ed = urb_priv->ed;
	td->next_dl_td = NULL;
	td->index = index;
	td->data = (__u32) data;
#ifdef OHCI_FILL_TRACE
	if (usb_pipebulk(urb_priv->pipe) && usb_pipeout(urb_priv->pipe)) {
		for (i = 0; i < len; i++)
			printf("td->data[%d] %#2x ", i,
			       ((unsigned char *)td->data)[i]);
		printf("\n");
	}
#endif
	if (!len)
		data = 0;

	td->hwINFO = (__u32) m32_swap(info);
	td->hwCBP = (__u32) m32_swap(data);
	if (data)
		td->hwBE = (__u32) m32_swap(data + len - 1);
	else
		td->hwBE = 0;
	td->hwNextTD = (__u32) m32_swap(td_pt);

	/* append to queue */
	td->ed->hwTailP = td->hwNextTD;
}

/*-------------------------------------------------------------------------*/

/* prepare all TDs of a transfer */

static void td_submit_job(struct usb_device *dev, unsigned long pipe,
			  void *buffer, int transfer_len,
			  struct devrequest *setup, struct urb_priv *urb,
			  int interval)
{
	struct ohci *ohci = &gohci;
	int data_len = transfer_len;
	void *data;
	int cnt = 0;
	__u32 info = 0;
	unsigned int toggle = 0;

	/* OHCI handles the DATA-toggles itself, we just
	   use the USB-toggle bits for reseting */
	if (usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe))) {
		toggle = TD_T_TOGGLE;
	} else {
		toggle = TD_T_DATA0;
		usb_settoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe),
			      1);
	}
	urb->td_cnt = 0;
	if (data_len)
		data = buffer;
	else
		data = 0;

	switch (usb_pipetype(pipe)) {
	case PIPE_BULK:
		info = usb_pipeout(pipe) ? TD_CC | TD_DP_OUT : TD_CC | TD_DP_IN;
		while (data_len > 4096) {
			td_fill(ohci, info | (cnt ? TD_T_TOGGLE : toggle), data,
				4096, dev, cnt, urb);
			data += 4096;
			data_len -= 4096;
			cnt++;
		}
		info = usb_pipeout(pipe) ?
				TD_CC | TD_DP_OUT :
				TD_CC | TD_R | TD_DP_IN;
		td_fill(ohci, info | (cnt ? TD_T_TOGGLE : toggle), data,
			data_len, dev, cnt, urb);
		cnt++;

		if (!ohci->sleeping)
			/* start bulk list */
			writel(OHCI_BLF, &ohci->regs->cmdstatus);
		break;

	case PIPE_CONTROL:
		info = TD_CC | TD_DP_SETUP | TD_T_DATA0;
		td_fill(ohci, info, setup, 8, dev, cnt++, urb);
		if (data_len > 0) {
			info = usb_pipeout(pipe) ?
			    TD_CC | TD_R | TD_DP_OUT | TD_T_DATA1 :
			    TD_CC | TD_R | TD_DP_IN | TD_T_DATA1;
			/* NOTE:  mishandles transfers >8K, some >4K */
			td_fill(ohci, info, data, data_len, dev, cnt++, urb);
		}
		info = usb_pipeout(pipe) ?
		    TD_CC | TD_DP_IN | TD_T_DATA1 :
		    TD_CC | TD_DP_OUT | TD_T_DATA1;
		td_fill(ohci, info, data, 0, dev, cnt++, urb);
		if (!ohci->sleeping)
			/* start Control list */
			writel(OHCI_CLF, &ohci->regs->cmdstatus);
		break;
	}
	if (urb->length != cnt)
		dbg("TD LENGTH %d != CNT %d", urb->length, cnt);
}

/*-------------------------------------------------------------------------*
 * Done List handling functions
 *-------------------------------------------------------------------------*/


/* calculate the transfer length and update the urb */

static void dl_transfer_length(struct td *td)
{
	__u32 tdBE, tdCBP;
	struct urb_priv *lurb_priv = &urb_priv;

	tdBE = m32_swap(td->hwBE);
	tdCBP = m32_swap(td->hwCBP);

	if (!(usb_pipecontrol(lurb_priv->pipe) &&
	      ((td->index == 0) || (td->index == lurb_priv->length - 1)))) {
		if (tdBE != 0) {
			if (td->hwCBP == 0)
				lurb_priv->actual_length += tdBE - td->data + 1;
			else
				lurb_priv->actual_length += tdCBP - td->data;
		}
	}
}

/*-------------------------------------------------------------------------*/

/* replies to the request have to be on a FIFO basis so
 * we reverse the reversed done-list */

static struct td *dl_reverse_done_list(struct ohci *ohci)
{
	__u32 td_list_hc;
	__u32 tmp;
	struct td *td_rev = NULL;
	struct td *td_list = NULL;
	struct urb_priv *lurb_priv = NULL;

	td_list_hc = m32_swap(ohci->hcca->done_head) & 0xfffffff0;
	ohci->hcca->done_head = 0;

	while (td_list_hc) {
		td_list = (struct td *) td_list_hc;

		if (TD_CC_GET(m32_swap(td_list->hwINFO))) {
			lurb_priv = &urb_priv;
			dbg(" USB-error/status: %x : %p",
			    TD_CC_GET(m32_swap(td_list->hwINFO)), td_list);
			if (td_list->ed->hwHeadP & m32_swap(0x1)) {
				if (lurb_priv &&
				    ((td_list->index+1) < lurb_priv->length)) {
					tmp = lurb_priv->length - 1;
					td_list->ed->hwHeadP =
						(lurb_priv->td[tmp]->hwNextTD &
						 m32_swap(0xfffffff0)) |
						 (td_list->ed->hwHeadP &
						  m32_swap(0x2));
					lurb_priv->td_cnt += lurb_priv->length -
							     td_list->index - 1;
				} else
					td_list->ed->hwHeadP &=
					    m32_swap(0xfffffff2);
			}
		}

		td_list->next_dl_td = td_rev;
		td_rev = td_list;
		td_list_hc = m32_swap(td_list->hwNextTD) & 0xfffffff0;
	}

	return td_list;
}

/*-------------------------------------------------------------------------*/

/* td done list */
static int dl_done_list(struct ohci *ohci, struct td *td_list)
{
	struct td *td_list_next = NULL;
	struct ed *ed;
	int cc = 0;
	int stat = 0;
	/* urb_t *urb; */
	struct urb_priv *lurb_priv;
	__u32 tdINFO, edHeadP, edTailP;

	while (td_list) {
		td_list_next = td_list->next_dl_td;

		lurb_priv = &urb_priv;
		tdINFO = m32_swap(td_list->hwINFO);

		ed = td_list->ed;

		dl_transfer_length(td_list);

		/* error code of transfer */
		cc = TD_CC_GET(tdINFO);
		if (cc != 0) {
			dbg("ConditionCode %#x", cc);
			stat = cc_to_error[cc];
		}

		/* see if this done list makes for all TD's of current URB,
		 * and mark the URB finished if so */
		if (++(lurb_priv->td_cnt) == lurb_priv->length) {
			if ((ed->state & (ED_OPER | ED_UNLINK)))
				urb_finished = 1;
			else
				dbg("dl_done_list: strange.., ED state %x, "
				    "ed->state\n");
		} else
			dbg("dl_done_list: processing TD %x, len %x\n",
			    lurb_priv->td_cnt, lurb_priv->length);

		if (ed->state != ED_NEW) {
			edHeadP = m32_swap(ed->hwHeadP) & 0xfffffff0;
			edTailP = m32_swap(ed->hwTailP);

			/* unlink eds if they are not busy */
			if ((edHeadP == edTailP) && (ed->state == ED_OPER))
				ep_unlink(ohci, ed);
		}

		td_list = td_list_next;
	}
	return stat;
}

/*-------------------------------------------------------------------------*
 * Virtual Root Hub
 *-------------------------------------------------------------------------*/

#include <usbroothubdes.h>

/* Hub class-specific descriptor is constructed dynamically */


/*-------------------------------------------------------------------------*/

#define OK(x)			len = (x); break
#ifdef DEBUG
#define WR_RH_STAT(x) \
{ \
	info("WR:status %#8x", (x)); \
	writel((x), &gohci.regs->roothub.status); \
}
#define WR_RH_PORTSTAT(x) \
{ \
	info("WR:portstatus[%d] %#8x", wIndex-1, (x)); \
	writel((x), &gohci.regs->roothub.portstatus[wIndex-1]); \
}
#else
#define WR_RH_STAT(x) \
	writel((x), &gohci.regs->roothub.status)
#define WR_RH_PORTSTAT(x)\
	writel((x), &gohci.regs->roothub.portstatus[wIndex-1])
#endif
#define RD_RH_STAT	roothub_status(&gohci)
#define RD_RH_PORTSTAT	roothub_portstatus(&gohci, wIndex-1)

/* request to virtual root hub */

int rh_check_port_status(struct ohci *controller)
{
	__u32 temp, ndp, i;
	int res;

	res = -1;
	temp = roothub_a(controller);
	ndp = (temp & RH_A_NDP);
	for (i = 0; i < ndp; i++) {
		temp = roothub_portstatus(controller, i);
		/* check for a device disconnect */
		if (((temp & (RH_PS_PESC | RH_PS_CSC)) ==
		     (RH_PS_PESC | RH_PS_CSC)) && ((temp & RH_PS_CCS) == 0)) {
			res = i;
			break;
		}
	}
	return res;
}

static int ohci_submit_rh_msg(struct usb_device *dev, unsigned long pipe,
			      void *buffer, int transfer_len,
			      struct devrequest *cmd)
{
	void *data = buffer;
	int leni = transfer_len;
	int len = 0;
	int stat = 0;
	union {
		__u32 word[4];
		__u16 hword[8];
		__u8 byte[16];
	} datab;
	__u8 *data_buf = datab.byte;
	__u16 bmRType_bReq;
	__u16 wValue;
	__u16 wIndex;
	__u16 wLength;

#ifdef DEBUG
	urb_priv.actual_length = 0;
	pkt_print(dev, pipe, buffer, transfer_len, cmd, "SUB(rh)",
		  usb_pipein(pipe));
#else
	mdelay(1);
#endif
	if (usb_pipeint(pipe)) {
		info("Root-Hub submit IRQ: NOT implemented");
		return 0;
	}

	bmRType_bReq = cmd->requesttype | (cmd->request << 8);
	wValue = m16_swap(cmd->value);
	wIndex = m16_swap(cmd->index);
	wLength = m16_swap(cmd->length);

	info("Root-Hub: adr: %2x cmd(%1x): %08x %04x %04x %04x",
	     dev->devnum, 8, bmRType_bReq, wValue, wIndex, wLength);

	switch (bmRType_bReq) {
		/* Request Destination:
		   without flags: Device,
		   RH_INTERFACE: interface,
		   RH_ENDPOINT: endpoint,
		   RH_CLASS means HUB here,
		   RH_OTHER | RH_CLASS  almost ever means HUB_PORT here
		 */

	case RH_GET_STATUS:
		datab.hword[0] = m16_swap(1);
		OK(2);
	case RH_GET_STATUS | RH_INTERFACE:
		datab.hword[0] = m16_swap(0);
		OK(2);
	case RH_GET_STATUS | RH_ENDPOINT:
		datab.hword[0] = m16_swap(0);
		OK(2);
	case RH_GET_STATUS | RH_CLASS:
		datab.word[0] =
		    m32_swap(RD_RH_STAT & ~(RH_HS_CRWE | RH_HS_DRWE));
		OK(4);
	case RH_GET_STATUS | RH_OTHER | RH_CLASS:
		datab.word[0] = m32_swap(RD_RH_PORTSTAT);
		OK(4);

	case RH_CLEAR_FEATURE | RH_ENDPOINT:
		switch (wValue) {
		case (RH_ENDPOINT_STALL):
			OK(0);
		}
		break;

	case RH_CLEAR_FEATURE | RH_CLASS:
		switch (wValue) {
		case RH_C_HUB_LOCAL_POWER:
			OK(0);
		case (RH_C_HUB_OVER_CURRENT):
			WR_RH_STAT(RH_HS_OCIC);
			OK(0);
		}
		break;

	case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
		switch (wValue) {
		case (RH_PORT_ENABLE):
			WR_RH_PORTSTAT(RH_PS_CCS);
			OK(0);
		case (RH_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_POCI);
			OK(0);
		case (RH_PORT_POWER):
			WR_RH_PORTSTAT(RH_PS_LSDA);
			OK(0);
		case (RH_C_PORT_CONNECTION):
			WR_RH_PORTSTAT(RH_PS_CSC);
			OK(0);
		case (RH_C_PORT_ENABLE):
			WR_RH_PORTSTAT(RH_PS_PESC);
			OK(0);
		case (RH_C_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_PSSC);
			OK(0);
		case (RH_C_PORT_OVER_CURRENT):
			WR_RH_PORTSTAT(RH_PS_OCIC);
			OK(0);
		case (RH_C_PORT_RESET):
			WR_RH_PORTSTAT(RH_PS_PRSC);
			OK(0);
		}
		break;

	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		switch (wValue) {
		case (RH_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_PSS);
			OK(0);
		case (RH_PORT_RESET):	/* BUG IN HUP CODE ******** */
			if (RD_RH_PORTSTAT & RH_PS_CCS)
				WR_RH_PORTSTAT(RH_PS_PRS);
			OK(0);
		case (RH_PORT_POWER):
			WR_RH_PORTSTAT(RH_PS_PPS);
			OK(0);
		case (RH_PORT_ENABLE):	/* BUG IN HUP CODE ******** */
			if (RD_RH_PORTSTAT & RH_PS_CCS)
				WR_RH_PORTSTAT(RH_PS_PES);
			OK(0);
		}
		break;

	case RH_SET_ADDRESS:
		gohci.rh.devnum = wValue;
		OK(0);

	case RH_GET_DESCRIPTOR:
		switch ((wValue & 0xff00) >> 8) {
		case (0x01):	/* device descriptor */
			len = min_t(unsigned int,
				    leni,
				    min_t(unsigned int,
					  sizeof(root_hub_dev_des), wLength));
			data_buf = root_hub_dev_des;
			OK(len);
		case (0x02):	/* configuration descriptor */
			len = min_t(unsigned int,
				    leni,
				    min_t(unsigned int,
					  sizeof(root_hub_config_des),
					  wLength));
			data_buf = root_hub_config_des;
			OK(len);
		case (0x03):	/* string descriptors */
			if (wValue == 0x0300) {
				len = min_t(unsigned int,
					    leni,
					    min_t(unsigned int,
						  sizeof(root_hub_str_index0),
						  wLength));
				data_buf = root_hub_str_index0;
				OK(len);
			}
			if (wValue == 0x0301) {
				len = min_t(unsigned int,
					    leni,
					    min_t(unsigned int,
						  sizeof(root_hub_str_index1),
						  wLength));
				data_buf = root_hub_str_index1;
				OK(len);
			}
		default:
			stat = USB_ST_STALLED;
		}
		break;

	case RH_GET_DESCRIPTOR | RH_CLASS:
		{
			__u32 temp = roothub_a(&gohci);

			data_buf[0] = 9;	/* min length; */
			data_buf[1] = 0x29;
			data_buf[2] = temp & RH_A_NDP;
			data_buf[3] = 0;
			if (temp & RH_A_PSM)
				/* per-port power switching? */
				data_buf[3] |= 0x1;
			if (temp & RH_A_NOCP)
				/* no overcurrent reporting? */
				data_buf[3] |= 0x10;
			else if (temp & RH_A_OCPM)
				/* per-port overcurrent reporting? */
				data_buf[3] |= 0x8;

			/* corresponds to data_buf[4-7] */
			datab.word[1] = 0;
			data_buf[5] = (temp & RH_A_POTPGT) >> 24;
			temp = roothub_b(&gohci);
			data_buf[7] = temp & RH_B_DR;
			if (data_buf[2] < 7) {
				data_buf[8] = 0xff;
			} else {
				data_buf[0] += 2;
				data_buf[8] = (temp & RH_B_DR) >> 8;
				data_buf[10] = data_buf[9] = 0xff;
			}

			len = min_t(unsigned int, leni,
				    min_t(unsigned int, data_buf[0], wLength));
			OK(len);
		}

	case RH_GET_CONFIGURATION:
		*(__u8 *) data_buf = 0x01;
		OK(1);

	case RH_SET_CONFIGURATION:
		WR_RH_STAT(0x10000);
		OK(0);

	default:
		dbg("unsupported root hub command");
		stat = USB_ST_STALLED;
	}

#ifdef	DEBUG
	ohci_dump_roothub(&gohci, 1);
#else
	mdelay(1);
#endif

	len = min_t(int, len, leni);
	if (data != data_buf)
		memcpy(data, data_buf, len);
	dev->act_len = len;
	dev->status = stat;

#ifdef DEBUG
	if (transfer_len)
		urb_priv.actual_length = transfer_len;
	pkt_print(dev, pipe, buffer, transfer_len, cmd, "RET(rh)",
		  0 /*usb_pipein(pipe) */);
#else
	mdelay(1);
#endif

	return stat;
}

/*-------------------------------------------------------------------------*/

/* common code for handling submit messages - used for all but root hub */
/* accesses. */
int submit_common_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		      int transfer_len, struct devrequest *setup, int interval)
{
	int stat = 0;
	int maxsize = usb_maxpacket(dev, pipe);
	int timeout;

	/* device pulled? Shortcut the action. */
	if (devgone == dev) {
		dev->status = USB_ST_CRC_ERR;
		return 0;
	}
#ifdef DEBUG
	urb_priv.actual_length = 0;
	pkt_print(dev, pipe, buffer, transfer_len, setup, "SUB",
		  usb_pipein(pipe));
#else
	mdelay(1);
#endif
	if (!maxsize) {
		err("submit_common_message: pipesize for pipe %lx is zero",
		    pipe);
		return -1;
	}

	if (sohci_submit_job(dev, pipe, buffer, transfer_len, setup, interval) <
	    0) {
		err("sohci_submit_job failed");
		return -1;
	}

	mdelay(10);
	/* ohci_dump_status(&gohci); */

	/* allow more time for a BULK device to react - some are slow */
#define BULK_TO	 5000		/* timeout in milliseconds */
	if (usb_pipebulk(pipe))
		timeout = BULK_TO;
	else
		timeout = 100;

	/* wait for it to complete */
	for (;;) {
		/* check whether the controller is done */
		stat = hc_interrupt();

		if (stat < 0) {
			stat = USB_ST_CRC_ERR;
			break;
		}

		/* NOTE: since we are not interrupt driven in U-Boot and always
		 * handle only one URB at a time, we cannot assume the
		 * transaction finished on the first successful return from
		 * hc_interrupt().. unless the flag for current URB is set,
		 * meaning that all TD's to/from device got actually
		 * transferred and processed. If the current URB is not
		 * finished we need to re-iterate this loop so as
		 * hc_interrupt() gets called again as there needs to be some
		 * more TD's to process still */
		if ((stat >= 0) && (stat != 0xff) && (urb_finished)) {
			/* 0xff is returned for an SF-interrupt */
			break;
		}

		if (--timeout) {
			mdelay(1);
			if (!urb_finished)
				dbg("\%");

		} else {
			err("CTL:TIMEOUT ");
			dbg("submit_common_msg: TO status %x\n", stat);
			stat = USB_ST_CRC_ERR;
			urb_finished = 1;
			break;
		}
	}

#if 0
	/* we got an Root Hub Status Change interrupt */
	if (got_rhsc) {
#ifdef DEBUG
		ohci_dump_roothub(&gohci, 1);
#endif
		got_rhsc = 0;
		/* abuse timeout */
		timeout = rh_check_port_status(&gohci);
		if (timeout >= 0) {
#if 0			/* this does nothing useful, but leave it here
			   in case that changes */
			/* the called routine adds 1 to the passed value */
			usb_hub_port_connect_change(gohci.rh.dev, timeout - 1);
#endif
			/*
			 * XXX
			 * This is potentially dangerous because it assumes
			 * that only one device is ever plugged in!
			 */
			devgone = dev;
		}
	}
#endif

	dev->status = stat;
	dev->act_len = transfer_len;

#ifdef DEBUG
	pkt_print(dev, pipe, buffer, transfer_len, setup, "RET(ctlr)",
		  usb_pipein(pipe));
#else
	mdelay(1);
#endif

	/* free TDs in urb_priv */
	urb_free_priv(&urb_priv);
	return 0;
}

/* submit routines called from usb.c */
int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		    int transfer_len)
{
	info("submit_bulk_msg");
	return submit_common_msg(dev, pipe, buffer, transfer_len, NULL, 0);
}

int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		       int transfer_len, struct devrequest *setup)
{
	int maxsize = usb_maxpacket(dev, pipe);

	info("submit_control_msg");
#ifdef DEBUG
	urb_priv.actual_length = 0;
	pkt_print(dev, pipe, buffer, transfer_len, setup, "SUB",
		  usb_pipein(pipe));
#else
	mdelay(1);
#endif
	if (!maxsize) {
		err("submit_control_message: pipesize for pipe %lx is zero",
		    pipe);
		return -1;
	}
	if (((pipe >> 8) & 0x7f) == gohci.rh.devnum) {
		gohci.rh.dev = dev;
		/* root hub - redirect */
		return ohci_submit_rh_msg(dev, pipe, buffer, transfer_len,
					  setup);
	}

	return submit_common_msg(dev, pipe, buffer, transfer_len, setup, 0);
}

int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
		   int transfer_len, int interval)
{
	info("submit_int_msg");
	return -1;
}

/*-------------------------------------------------------------------------*
 * HC functions
 *-------------------------------------------------------------------------*/

/* reset the HC and BUS */

static int hc_reset(struct ohci *ohci)
{
	int timeout = 30;
	int smm_timeout = 50;	/* 0,5 sec */

	if (readl(&ohci->regs->control) & OHCI_CTRL_IR) {
		/* SMM owns the HC - request ownership */
		writel(OHCI_OCR, &ohci->regs->cmdstatus);
		info("USB HC TakeOver from SMM");
		while (readl(&ohci->regs->control) & OHCI_CTRL_IR) {
			mdelay(10);
			if (--smm_timeout == 0) {
				err("USB HC TakeOver failed!");
				return -1;
			}
		}
	}

	/* Disable HC interrupts */
	writel(OHCI_INTR_MIE, &ohci->regs->intrdisable);

	dbg("USB HC reset_hc usb-%s: ctrl = 0x%X ;",
	    ohci->slot_name, readl(&ohci->regs->control));

	/* Reset USB (needed by some controllers) */
	writel(0, &ohci->regs->control);

	/* HC Reset requires max 10 us delay */
	writel(OHCI_HCR, &ohci->regs->cmdstatus);
	while ((readl(&ohci->regs->cmdstatus) & OHCI_HCR) != 0) {
		if (--timeout == 0) {
			err("USB HC reset timed out!");
			return -1;
		}
		udelay(1);
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* Start an OHCI controller, set the BUS operational
 * enable interrupts
 * connect the virtual root hub */

static int hc_start(struct ohci *ohci)
{
	__u32 mask;
	unsigned int fminterval;

	ohci->disabled = 1;

	/* Tell the controller where the control and bulk lists are
	 * The lists are empty now. */

	writel(0, &ohci->regs->ed_controlhead);
	writel(0, &ohci->regs->ed_bulkhead);

	/* a reset clears this */
	writel((__u32) ohci->hcca, &ohci->regs->hcca);

	fminterval = 0x2edf;
	writel((fminterval * 9) / 10, &ohci->regs->periodicstart);
	fminterval |= ((((fminterval - 210) * 6) / 7) << 16);
	writel(fminterval, &ohci->regs->fminterval);
	writel(0x628, &ohci->regs->lsthresh);

	/* start controller operations */
	ohci->hc_control = OHCI_CONTROL_INIT | OHCI_USB_OPER;
	ohci->disabled = 0;
	writel(ohci->hc_control, &ohci->regs->control);

	/* disable all interrupts */
	mask = (OHCI_INTR_SO | OHCI_INTR_WDH | OHCI_INTR_SF | OHCI_INTR_RD |
		OHCI_INTR_UE | OHCI_INTR_FNO | OHCI_INTR_RHSC |
		OHCI_INTR_OC | OHCI_INTR_MIE);
	writel(mask, &ohci->regs->intrdisable);
	/* clear all interrupts */
	mask &= ~OHCI_INTR_MIE;
	writel(mask, &ohci->regs->intrstatus);
	/* Choose the interrupts we care about now  - but w/o MIE */
	mask = OHCI_INTR_RHSC | OHCI_INTR_UE | OHCI_INTR_WDH | OHCI_INTR_SO;
	writel(mask, &ohci->regs->intrenable);

#ifdef	OHCI_USE_NPS
	/* required for AMD-756 and some Mac platforms */
	writel((roothub_a(ohci) | RH_A_NPS) & ~RH_A_PSM,
	       &ohci->regs->roothub.a);
	writel(RH_HS_LPSC, &ohci->regs->roothub.status);
#endif /* OHCI_USE_NPS */

	/* POTPGT delay is bits 24-31, in 2 ms units. */
	mdelay((roothub_a(ohci) >> 23) & 0x1fe);

	/* connect the virtual root hub */
	ohci->rh.devnum = 0;

	return 0;
}

/*-------------------------------------------------------------------------*/

/* an interrupt happens */

static int hc_interrupt(void)
{
	struct ohci *ohci = &gohci;
	struct ohci_regs *regs = ohci->regs;
	int ints;
	int stat = -1;

	if ((ohci->hcca->done_head != 0) &&
	    !(m32_swap(ohci->hcca->done_head) & 0x01)) {

		ints = OHCI_INTR_WDH;

	} else {
		ints = readl(&regs->intrstatus);
		if (ints == ~(u32) 0) {
			ohci->disabled++;
			err("%s device removed!", ohci->slot_name);
			return -1;
		}
		ints &= readl(&regs->intrenable);
		if (ints == 0) {
			dbg("hc_interrupt: returning..\n");
			return 0xff;
		}
	}

	/* dbg("Interrupt: %x frame: %x", ints,
	    le16_to_cpu(ohci->hcca->frame_no)); */

	if (ints & OHCI_INTR_RHSC) {
		got_rhsc = 1;
		stat = 0xff;
	}

	if (ints & OHCI_INTR_UE) {
		ohci->disabled++;
		err("OHCI Unrecoverable Error, controller usb-%s disabled",
		    ohci->slot_name);
		/* e.g. due to PCI Master/Target Abort */

#ifdef	DEBUG
		ohci_dump(ohci, 1);
#else
		mdelay(1);
#endif
		/* FIXME: be optimistic, hope that bug won't repeat often. */
		/* Make some non-interrupt context restart the controller. */
		/* Count and limit the retries though; either hardware or */
		/* software errors can go forever... */
		hc_reset(ohci);
		return -1;
	}

	if (ints & OHCI_INTR_WDH) {
		mdelay(1);

		writel(OHCI_INTR_WDH, &regs->intrdisable);
		stat = dl_done_list(&gohci, dl_reverse_done_list(&gohci));
		writel(OHCI_INTR_WDH, &regs->intrenable);
	}

	if (ints & OHCI_INTR_SO) {
		dbg("USB Schedule overrun\n");
		writel(OHCI_INTR_SO, &regs->intrenable);
		stat = -1;
	}

	/* FIXME:  this assumes SOF (1/ms) interrupts don't get lost... */
	if (ints & OHCI_INTR_SF) {
		unsigned int frame = m16_swap(ohci->hcca->frame_no) & 1;
		mdelay(1);
		writel(OHCI_INTR_SF, &regs->intrdisable);
		if (ohci->ed_rm_list[frame] != NULL)
			writel(OHCI_INTR_SF, &regs->intrenable);
		stat = 0xff;
	}

	writel(ints, &regs->intrstatus);
	return stat;
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/* De-allocate all resources.. */

static void hc_release_ohci(struct ohci *ohci)
{
	dbg("USB HC release ohci usb-%s", ohci->slot_name);

	if (!ohci->disabled)
		hc_reset(ohci);
}

/*-------------------------------------------------------------------------*/

/*
 * low level initalisation routine, called from usb.c
 */
static char ohci_inited = 0;

int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	struct s3c24x0_gpio *gpio = s3c24x0_get_base_gpio();

	/*
	 * Set the 48 MHz UPLL clocking. Values are taken from
	 * "PLL value selection guide", 6-23, s3c2400_UM.pdf.
	 */
	clk_power->upllcon = ((40 << 12) + (1 << 4) + 2);
	gpio->misccr |= 0x8;	/* 1 = use pads related USB for USB host */

	/*
	 * Enable USB host clock.
	 */
	clk_power->clkcon |= (1 << 4);

	memset(&gohci, 0, sizeof(struct ohci));
	memset(&urb_priv, 0, sizeof(struct urb_priv));

	/* align the storage */
	if ((__u32) &ghcca[0] & 0xff) {
		err("HCCA not aligned!!");
		return -1;
	}
	phcca = &ghcca[0];
	info("aligned ghcca %p", phcca);
	memset(&ohci_dev, 0, sizeof(struct ohci_device));
	if ((__u32) &ohci_dev.ed[0] & 0x7) {
		err("EDs not aligned!!");
		return -1;
	}
	memset(gtd, 0, sizeof(struct td) * (NUM_TD + 1));
	if ((__u32) gtd & 0x7) {
		err("TDs not aligned!!");
		return -1;
	}
	ptd = gtd;
	gohci.hcca = phcca;
	memset(phcca, 0, sizeof(struct ohci_hcca));

	gohci.disabled = 1;
	gohci.sleeping = 0;
	gohci.irq = -1;
	gohci.regs = (struct ohci_regs *)S3C24X0_USB_HOST_BASE;

	gohci.flags = 0;
	gohci.slot_name = "s3c2400";

	if (hc_reset(&gohci) < 0) {
		hc_release_ohci(&gohci);
		/* Initialization failed */
		clk_power->clkcon &= ~(1 << 4);
		return -1;
	}

	/* FIXME this is a second HC reset; why?? */
	gohci.hc_control = OHCI_USB_RESET;
	writel(gohci.hc_control, &gohci.regs->control);
	mdelay(10);

	if (hc_start(&gohci) < 0) {
		err("can't start usb-%s", gohci.slot_name);
		hc_release_ohci(&gohci);
		/* Initialization failed */
		clk_power->clkcon &= ~(1 << 4);
		return -1;
	}
#ifdef	DEBUG
	ohci_dump(&gohci, 1);
#else
	mdelay(1);
#endif
	ohci_inited = 1;
	urb_finished = 1;

	return 0;
}

int usb_lowlevel_stop(int index)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();

	/* this gets called really early - before the controller has */
	/* even been initialized! */
	if (!ohci_inited)
		return 0;
	/* TODO release any interrupts, etc. */
	/* call hc_release_ohci() here ? */
	hc_reset(&gohci);
	/* may not want to do this */
	clk_power->clkcon &= ~(1 << 4);
	return 0;
}

#endif /* defined(CONFIG_USB_OHCI) && defined(CONFIG_S3C24X0) */

#if defined(CONFIG_USB_OHCI_NEW) && \
    defined(CONFIG_SYS_USB_OHCI_CPU_INIT) && \
    defined(CONFIG_S3C24X0)

int usb_cpu_init(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	struct s3c24x0_gpio *gpio = s3c24x0_get_base_gpio();

	/*
	 * Set the 48 MHz UPLL clocking. Values are taken from
	 * "PLL value selection guide", 6-23, s3c2400_UM.pdf.
	 */
	writel((40 << 12) + (1 << 4) + 2, &clk_power->upllcon);
	/* 1 = use pads related USB for USB host */
	writel(readl(&gpio->misccr) | 0x8, &gpio->misccr);

	/*
	 * Enable USB host clock.
	 */
	writel(readl(&clk_power->clkcon) | (1 << 4), &clk_power->clkcon);

	return 0;
}

int usb_cpu_stop(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	/* may not want to do this */
	writel(readl(&clk_power->clkcon) & ~(1 << 4), &clk_power->clkcon);
	return 0;
}

int usb_cpu_init_fail(void)
{
	struct s3c24x0_clock_power *clk_power = s3c24x0_get_base_clock_power();
	writel(readl(&clk_power->clkcon) & ~(1 << 4), &clk_power->clkcon);
	return 0;
}

#endif /* defined(CONFIG_USB_OHCI_NEW) && \
	   defined(CONFIG_SYS_USB_OHCI_CPU_INIT) && \
	   defined(CONFIG_S3C24X0) */
