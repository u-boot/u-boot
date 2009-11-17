/*
 * URB OHCI HCD (Host Controller Driver) for USB on the AT91RM9200 and PCI bus.
 *
 * Interrupt support is added. Now, it has been tested
 * on ULI1575 chip and works well with USB keyboard.
 *
 * (C) Copyright 2007
 * Zhang Wei, Freescale Semiconductor, Inc. <wei.zhang@freescale.com>
 *
 * (C) Copyright 2003
 * Gary Jennejohn, DENX Software Engineering <garyj@denx.de>
 *
 * Note: Much of this code has been derived from Linux 2.4
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2002 David Brownell
 *
 * Modified for the MP2USB by (C) Copyright 2005 Eric Benard
 * ebenard@eukrea.com - based on s3c24x0's driver
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
/*
 * IMPORTANT NOTES
 * 1 - Read doc/README.generic_usb_ohci
 * 2 - this driver is intended for use with USB Mass Storage Devices
 *     (BBB) and USB keyboard. There is NO support for Isochronous pipes!
 * 2 - when running on a PQFP208 AT91RM9200, define CONFIG_AT91C_PQFP_UHPBUG
 *     to activate workaround for bug #41 or this driver will NOT work!
 */

#include <common.h>
#include <asm/byteorder.h>

#if defined(CONFIG_PCI_OHCI)
# include <pci.h>
#if !defined(CONFIG_PCI_OHCI_DEVNO)
#define CONFIG_PCI_OHCI_DEVNO	0
#endif
#endif

#include <malloc.h>
#include <usb.h>

#include "ohci.h"

#ifdef CONFIG_AT91RM9200
#include <asm/arch/hardware.h>	/* needed for AT91_USB_HOST_BASE */
#endif

#if defined(CONFIG_ARM920T) || \
    defined(CONFIG_S3C24X0) || \
    defined(CONFIG_S3C6400) || \
    defined(CONFIG_440EP) || \
    defined(CONFIG_PCI_OHCI) || \
    defined(CONFIG_MPC5200) || \
    defined(CONFIG_SYS_OHCI_USE_NPS)
# define OHCI_USE_NPS		/* force NoPowerSwitching mode */
#endif

#undef OHCI_VERBOSE_DEBUG	/* not always helpful */
#undef DEBUG
#undef SHOW_INFO
#undef OHCI_FILL_TRACE

/* For initializing controller (mask in an HCFS mode too) */
#define OHCI_CONTROL_INIT \
	(OHCI_CTRL_CBSR & 0x3) | OHCI_CTRL_IE | OHCI_CTRL_PLE

/*
 * e.g. PCI controllers need this
 */
#ifdef CONFIG_SYS_OHCI_SWAP_REG_ACCESS
# define readl(a) __swap_32(*((volatile u32 *)(a)))
# define writel(a, b) (*((volatile u32 *)(b)) = __swap_32((volatile u32)a))
#else
# define readl(a) (*((volatile u32 *)(a)))
# define writel(a, b) (*((volatile u32 *)(b)) = ((volatile u32)a))
#endif /* CONFIG_SYS_OHCI_SWAP_REG_ACCESS */

#define min_t(type, x, y) \
		    ({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })

#ifdef CONFIG_PCI_OHCI
static struct pci_device_id ohci_pci_ids[] = {
	{0x10b9, 0x5237},	/* ULI1575 PCI OHCI module ids */
	{0x1033, 0x0035},	/* NEC PCI OHCI module ids */
	{0x1131, 0x1561},	/* Philips 1561 PCI OHCI module ids */
	/* Please add supported PCI OHCI controller ids here */
	{0, 0}
};
#endif

#ifdef CONFIG_PCI_EHCI_DEVNO
static struct pci_device_id ehci_pci_ids[] = {
	{0x1131, 0x1562},	/* Philips 1562 PCI EHCI module ids */
	/* Please add supported PCI EHCI controller ids here */
	{0, 0}
};
#endif

#ifdef DEBUG
#define dbg(format, arg...) printf("DEBUG: " format "\n", ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif /* DEBUG */
#define err(format, arg...) printf("ERROR: " format "\n", ## arg)
#ifdef SHOW_INFO
#define info(format, arg...) printf("INFO: " format "\n", ## arg)
#else
#define info(format, arg...) do {} while (0)
#endif

#ifdef CONFIG_SYS_OHCI_BE_CONTROLLER
# define m16_swap(x) cpu_to_be16(x)
# define m32_swap(x) cpu_to_be32(x)
#else
# define m16_swap(x) cpu_to_le16(x)
# define m32_swap(x) cpu_to_le32(x)
#endif /* CONFIG_SYS_OHCI_BE_CONTROLLER */

/* global ohci_t */
static ohci_t gohci;
/* this must be aligned to a 256 byte boundary */
struct ohci_hcca ghcca[1];
/* a pointer to the aligned storage */
struct ohci_hcca *phcca;
/* this allocates EDs for all possible endpoints */
struct ohci_device ohci_dev;
/* device which was disconnected */
struct usb_device *devgone;

static inline u32 roothub_a(struct ohci *hc)
	{ return readl(&hc->regs->roothub.a); }
static inline u32 roothub_b(struct ohci *hc)
	{ return readl(&hc->regs->roothub.b); }
static inline u32 roothub_status(struct ohci *hc)
	{ return readl(&hc->regs->roothub.status); }
static inline u32 roothub_portstatus(struct ohci *hc, int i)
	{ return readl(&hc->regs->roothub.portstatus[i]); }

/* forward declaration */
static int hc_interrupt(void);
static void td_submit_job(struct usb_device *dev, unsigned long pipe,
			  void *buffer, int transfer_len,
			  struct devrequest *setup, urb_priv_t *urb,
			  int interval);

/*-------------------------------------------------------------------------*
 * URB support functions
 *-------------------------------------------------------------------------*/

/* free HCD-private data associated with this URB */

static void urb_free_priv(urb_priv_t *urb)
{
	int		i;
	int		last;
	struct td	*td;

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
	free(urb);
}

/*-------------------------------------------------------------------------*/

#ifdef DEBUG
static int sohci_get_current_frame_number(struct usb_device *dev);

/* debug| print the main components of an URB
 * small: 0) header + data packets 1) just header */

static void pkt_print(urb_priv_t *purb, struct usb_device *dev,
		      unsigned long pipe, void *buffer, int transfer_len,
		      struct devrequest *setup, char *str, int small)
{
	dbg("%s URB:[%4x] dev:%2lu,ep:%2lu-%c,type:%s,len:%d/%d stat:%#lx",
			str,
			sohci_get_current_frame_number(dev),
			usb_pipedevice(pipe),
			usb_pipeendpoint(pipe),
			usb_pipeout(pipe)? 'O': 'I',
			usb_pipetype(pipe) < 2 ? \
				(usb_pipeint(pipe)? "INTR": "ISOC"): \
				(usb_pipecontrol(pipe)? "CTRL": "BULK"),
			(purb ? purb->actual_length : 0),
			transfer_len, dev->status);
#ifdef	OHCI_VERBOSE_DEBUG
	if (!small) {
		int i, len;

		if (usb_pipecontrol(pipe)) {
			printf(__FILE__ ": cmd(8):");
			for (i = 0; i < 8 ; i++)
				printf(" %02x", ((__u8 *) setup) [i]);
			printf("\n");
		}
		if (transfer_len > 0 && buffer) {
			printf(__FILE__ ": data(%d/%d):",
				(purb ? purb->actual_length : 0),
				transfer_len);
			len = usb_pipeout(pipe)? transfer_len:
					(purb ? purb->actual_length : 0);
			for (i = 0; i < 16 && i < len; i++)
				printf(" %02x", ((__u8 *) buffer) [i]);
			printf("%s\n", i < len? "...": "");
		}
	}
#endif
}

/* just for debugging; prints non-empty branches of the int ed tree
 * inclusive iso eds */
void ep_print_int_eds(ohci_t *ohci, char *str)
{
	int i, j;
	 __u32 *ed_p;
	for (i = 0; i < 32; i++) {
		j = 5;
		ed_p = &(ohci->hcca->int_table [i]);
		if (*ed_p == 0)
		    continue;
		printf(__FILE__ ": %s branch int %2d(%2x):", str, i, i);
		while (*ed_p != 0 && j--) {
			ed_t *ed = (ed_t *)m32_swap(ed_p);
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
		(mask & OHCI_INTR_SO) ? " SO" : ""
		);
}

static void maybe_print_eds(char *label, __u32 value)
{
	ed_t *edp = (ed_t *)value;

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
	case OHCI_USB_RESET:	return "reset";
	case OHCI_USB_RESUME:	return "resume";
	case OHCI_USB_OPER:	return "operational";
	case OHCI_USB_SUSPEND:	return "suspend";
	}
	return "?";
}

/* dump control and status registers */
static void ohci_dump_status(ohci_t *controller)
{
	struct ohci_regs	*regs = controller->regs;
	__u32			temp;

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
		(temp & OHCI_CTRL_PLE) ? " PLE" : "",
		temp & OHCI_CTRL_CBSR
		);

	temp = readl(&regs->cmdstatus);
	dbg("cmdstatus: 0x%08x SOC=%d%s%s%s%s", temp,
		(temp & OHCI_SOC) >> 16,
		(temp & OHCI_OCR) ? " OCR" : "",
		(temp & OHCI_BLF) ? " BLF" : "",
		(temp & OHCI_CLF) ? " CLF" : "",
		(temp & OHCI_HCR) ? " HCR" : ""
		);

	ohci_dump_intr_mask("intrstatus", readl(&regs->intrstatus));
	ohci_dump_intr_mask("intrenable", readl(&regs->intrenable));

	maybe_print_eds("ed_periodcurrent", readl(&regs->ed_periodcurrent));

	maybe_print_eds("ed_controlhead", readl(&regs->ed_controlhead));
	maybe_print_eds("ed_controlcurrent", readl(&regs->ed_controlcurrent));

	maybe_print_eds("ed_bulkhead", readl(&regs->ed_bulkhead));
	maybe_print_eds("ed_bulkcurrent", readl(&regs->ed_bulkcurrent));

	maybe_print_eds("donehead", readl(&regs->donehead));
}

static void ohci_dump_roothub(ohci_t *controller, int verbose)
{
	__u32			temp, ndp, i;

	temp = roothub_a(controller);
	ndp = (temp & RH_A_NDP);
#ifdef CONFIG_AT91C_PQFP_UHPBUG
	ndp = (ndp == 2) ? 1:0;
#endif
	if (verbose) {
		dbg("roothub.a: %08x POTPGT=%d%s%s%s%s%s NDP=%d", temp,
			((temp & RH_A_POTPGT) >> 24) & 0xff,
			(temp & RH_A_NOCP) ? " NOCP" : "",
			(temp & RH_A_OCPM) ? " OCPM" : "",
			(temp & RH_A_DT) ? " DT" : "",
			(temp & RH_A_NPS) ? " NPS" : "",
			(temp & RH_A_PSM) ? " PSM" : "",
			ndp
			);
		temp = roothub_b(controller);
		dbg("roothub.b: %08x PPCM=%04x DR=%04x",
			temp,
			(temp & RH_B_PPCM) >> 16,
			(temp & RH_B_DR)
			);
		temp = roothub_status(controller);
		dbg("roothub.status: %08x%s%s%s%s%s%s",
			temp,
			(temp & RH_HS_CRWE) ? " CRWE" : "",
			(temp & RH_HS_OCIC) ? " OCIC" : "",
			(temp & RH_HS_LPSC) ? " LPSC" : "",
			(temp & RH_HS_DRWE) ? " DRWE" : "",
			(temp & RH_HS_OCI) ? " OCI" : "",
			(temp & RH_HS_LPS) ? " LPS" : ""
			);
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
			(temp & RH_PS_CCS) ? " CCS" : ""
			);
	}
}

static void ohci_dump(ohci_t *controller, int verbose)
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

int sohci_submit_job(urb_priv_t *urb, struct devrequest *setup)
{
	ohci_t *ohci;
	ed_t *ed;
	urb_priv_t *purb_priv = urb;
	int i, size = 0;
	struct usb_device *dev = urb->dev;
	unsigned long pipe = urb->pipe;
	void *buffer = urb->transfer_buffer;
	int transfer_len = urb->transfer_buffer_length;
	int interval = urb->interval;

	ohci = &gohci;

	/* when controller's hung, permit only roothub cleanup attempts
	 * such as powering down ports */
	if (ohci->disabled) {
		err("sohci_submit_job: EPIPE");
		return -1;
	}

	/* we're about to begin a new transaction here so mark the
	 * URB unfinished */
	urb->finished = 0;

	/* every endpoint has a ed, locate and fill it */
	ed = ep_add_ed(dev, pipe, interval, 1);
	if (!ed) {
		err("sohci_submit_job: ENOMEM");
		return -1;
	}

	/* for the private part of the URB we need the number of TDs (size) */
	switch (usb_pipetype(pipe)) {
	case PIPE_BULK: /* one TD for every 4096 Byte */
		size = (transfer_len - 1) / 4096 + 1;
		break;
	case PIPE_CONTROL:/* 1 TD for setup, 1 for ACK and 1 for every 4096 B */
		size = (transfer_len == 0)? 2:
					(transfer_len - 1) / 4096 + 3;
		break;
	case PIPE_INTERRUPT: /* 1 TD */
		size = 1;
		break;
	}

	ed->purb = urb;

	if (size >= (N_URB_TD - 1)) {
		err("need %d TDs, only have %d", size, N_URB_TD);
		return -1;
	}
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
	td_submit_job(dev, pipe, buffer, transfer_len,
		      setup, purb_priv, interval);

	return 0;
}

static inline int sohci_return_job(struct ohci *hc, urb_priv_t *urb)
{
	struct ohci_regs *regs = hc->regs;

	switch (usb_pipetype(urb->pipe)) {
	case PIPE_INTERRUPT:
		/* implicitly requeued */
		if (urb->dev->irq_handle &&
				(urb->dev->irq_act_len = urb->actual_length)) {
			writel(OHCI_INTR_WDH, &regs->intrenable);
			readl(&regs->intrenable); /* PCI posting flush */
			urb->dev->irq_handle(urb->dev);
			writel(OHCI_INTR_WDH, &regs->intrdisable);
			readl(&regs->intrdisable); /* PCI posting flush */
		}
		urb->actual_length = 0;
		td_submit_job(
				urb->dev,
				urb->pipe,
				urb->transfer_buffer,
				urb->transfer_buffer_length,
				NULL,
				urb,
				urb->interval);
		break;
	case PIPE_CONTROL:
	case PIPE_BULK:
		break;
	default:
		return 0;
	}
	return 1;
}

/*-------------------------------------------------------------------------*/

#ifdef DEBUG
/* tell us the current USB frame number */

static int sohci_get_current_frame_number(struct usb_device *usb_dev)
{
	ohci_t *ohci = &gohci;

	return m16_swap(ohci->hcca->frame_no);
}
#endif

/*-------------------------------------------------------------------------*
 * ED handling functions
 *-------------------------------------------------------------------------*/

/* search for the right branch to insert an interrupt ed into the int tree
 * do some load ballancing;
 * returns the branch and
 * sets the interval to interval = 2^integer (ld (interval)) */

static int ep_int_ballance(ohci_t *ohci, int interval, int load)
{
	int i, branch = 0;

	/* search for the least loaded interrupt endpoint
	 * branch of all 32 branches
	 */
	for (i = 0; i < 32; i++)
		if (ohci->ohci_int_load [branch] > ohci->ohci_int_load [i])
			branch = i;

	branch = branch % interval;
	for (i = branch; i < 32; i += interval)
		ohci->ohci_int_load [i] += load;

	return branch;
}

/*-------------------------------------------------------------------------*/

/*  2^int( ld (inter)) */

static int ep_2_n_interval(int inter)
{
	int i;
	for (i = 0; ((inter >> i) > 1) && (i < 5); i++);
	return 1 << i;
}

/*-------------------------------------------------------------------------*/

/* the int tree is a binary tree
 * in order to process it sequentially the indexes of the branches have to
 * be mapped the mapping reverses the bits of a word of num_bits length */
static int ep_rev(int num_bits, int word)
{
	int i, wout = 0;

	for (i = 0; i < num_bits; i++)
		wout |= (((word >> i) & 1) << (num_bits - i - 1));
	return wout;
}

/*-------------------------------------------------------------------------*
 * ED handling functions
 *-------------------------------------------------------------------------*/

/* link an ed into one of the HC chains */

static int ep_link(ohci_t *ohci, ed_t *edi)
{
	volatile ed_t *ed = edi;
	int int_branch;
	int i;
	int inter;
	int interval;
	int load;
	__u32 *ed_p;

	ed->state = ED_OPER;
	ed->int_interval = 0;

	switch (ed->type) {
	case PIPE_CONTROL:
		ed->hwNextED = 0;
		if (ohci->ed_controltail == NULL)
			writel(ed, &ohci->regs->ed_controlhead);
		else
			ohci->ed_controltail->hwNextED =
						   m32_swap((unsigned long)ed);

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
		if (ohci->ed_bulktail == NULL)
			writel(ed, &ohci->regs->ed_bulkhead);
		else
			ohci->ed_bulktail->hwNextED =
						   m32_swap((unsigned long)ed);

		ed->ed_prev = ohci->ed_bulktail;
		if (!ohci->ed_bulktail && !ohci->ed_rm_list[0] &&
			!ohci->ed_rm_list[1] && !ohci->sleeping) {
			ohci->hc_control |= OHCI_CTRL_BLE;
			writel(ohci->hc_control, &ohci->regs->control);
		}
		ohci->ed_bulktail = edi;
		break;

	case PIPE_INTERRUPT:
		load = ed->int_load;
		interval = ep_2_n_interval(ed->int_period);
		ed->int_interval = interval;
		int_branch = ep_int_ballance(ohci, interval, load);
		ed->int_branch = int_branch;

		for (i = 0; i < ep_rev(6, interval); i += inter) {
			inter = 1;
			for (ed_p = &(ohci->hcca->int_table[\
						ep_rev(5, i) + int_branch]);
				(*ed_p != 0) &&
				(((ed_t *)ed_p)->int_interval >= interval);
				ed_p = &(((ed_t *)ed_p)->hwNextED))
					inter = ep_rev(6,
						 ((ed_t *)ed_p)->int_interval);
			ed->hwNextED = *ed_p;
			*ed_p = m32_swap((unsigned long)ed);
		}
		break;
	}
	return 0;
}

/*-------------------------------------------------------------------------*/

/* scan the periodic table to find and unlink this ED */
static void periodic_unlink(struct ohci *ohci, volatile struct ed *ed,
			    unsigned index, unsigned period)
{
	for (; index < NUM_INTS; index += period) {
		__u32	*ed_p = &ohci->hcca->int_table [index];

		/* ED might have been unlinked through another path */
		while (*ed_p != 0) {
			if (((struct ed *)
					m32_swap((unsigned long)ed_p)) == ed) {
				*ed_p = ed->hwNextED;
				break;
			}
			ed_p = &(((struct ed *)
				     m32_swap((unsigned long)ed_p))->hwNextED);
		}
	}
}

/* unlink an ed from one of the HC chains.
 * just the link to the ed is unlinked.
 * the link from the ed still points to another operational ed or 0
 * so the HC can eventually finish the processing of the unlinked ed */

static int ep_unlink(ohci_t *ohci, ed_t *edi)
{
	volatile ed_t *ed = edi;
	int i;

	ed->hwINFO |= m32_swap(OHCI_ED_SKIP);

	switch (ed->type) {
	case PIPE_CONTROL:
		if (ed->ed_prev == NULL) {
			if (!ed->hwNextED) {
				ohci->hc_control &= ~OHCI_CTRL_CLE;
				writel(ohci->hc_control, &ohci->regs->control);
			}
			writel(m32_swap(*((__u32 *)&ed->hwNextED)),
				&ohci->regs->ed_controlhead);
		} else {
			ed->ed_prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_controltail == ed) {
			ohci->ed_controltail = ed->ed_prev;
		} else {
			((ed_t *)m32_swap(
			    *((__u32 *)&ed->hwNextED)))->ed_prev = ed->ed_prev;
		}
		break;

	case PIPE_BULK:
		if (ed->ed_prev == NULL) {
			if (!ed->hwNextED) {
				ohci->hc_control &= ~OHCI_CTRL_BLE;
				writel(ohci->hc_control, &ohci->regs->control);
			}
			writel(m32_swap(*((__u32 *)&ed->hwNextED)),
			       &ohci->regs->ed_bulkhead);
		} else {
			ed->ed_prev->hwNextED = ed->hwNextED;
		}
		if (ohci->ed_bulktail == ed) {
			ohci->ed_bulktail = ed->ed_prev;
		} else {
			((ed_t *)m32_swap(
			     *((__u32 *)&ed->hwNextED)))->ed_prev = ed->ed_prev;
		}
		break;

	case PIPE_INTERRUPT:
		periodic_unlink(ohci, ed, 0, 1);
		for (i = ed->int_branch; i < 32; i += ed->int_interval)
		    ohci->ohci_int_load[i] -= ed->int_load;
		break;
	}
	ed->state = ED_UNLINK;
	return 0;
}

/*-------------------------------------------------------------------------*/

/* add/reinit an endpoint; this should be done once at the
 * usb_set_configuration command, but the USB stack is a little bit
 * stateless so we do it at every transaction if the state of the ed
 * is ED_NEW then a dummy td is added and the state is changed to
 * ED_UNLINK in all other cases the state is left unchanged the ed
 * info fields are setted anyway even though most of them should not
 * change
 */
static ed_t *ep_add_ed(struct usb_device *usb_dev, unsigned long pipe,
			int interval, int load)
{
	td_t *td;
	ed_t *ed_ret;
	volatile ed_t *ed;

	ed = ed_ret = &ohci_dev.ed[(usb_pipeendpoint(pipe) << 1) |
			(usb_pipecontrol(pipe)? 0: usb_pipeout(pipe))];

	if ((ed->state & ED_DEL) || (ed->state & ED_URB_DEL)) {
		err("ep_add_ed: pending delete");
		/* pending delete request */
		return NULL;
	}

	if (ed->state == ED_NEW) {
		/* dummy td; end of td list for ed */
		td = td_alloc(usb_dev);
		ed->hwTailP = m32_swap((unsigned long)td);
		ed->hwHeadP = ed->hwTailP;
		ed->state = ED_UNLINK;
		ed->type = usb_pipetype(pipe);
		ohci_dev.ed_cnt++;
	}

	ed->hwINFO = m32_swap(usb_pipedevice(pipe)
			| usb_pipeendpoint(pipe) << 7
			| (usb_pipeisoc(pipe)? 0x8000: 0)
			| (usb_pipecontrol(pipe)? 0: \
					   (usb_pipeout(pipe)? 0x800: 0x1000))
			| usb_pipeslow(pipe) << 13
			| usb_maxpacket(usb_dev, pipe) << 16);

	if (ed->type == PIPE_INTERRUPT && ed->state == ED_UNLINK) {
		ed->int_period = interval;
		ed->int_load = load;
	}

	return ed_ret;
}

/*-------------------------------------------------------------------------*
 * TD handling functions
 *-------------------------------------------------------------------------*/

/* enqueue next TD for this URB (OHCI spec 5.2.8.2) */

static void td_fill(ohci_t *ohci, unsigned int info,
	void *data, int len,
	struct usb_device *dev, int index, urb_priv_t *urb_priv)
{
	volatile td_t  *td, *td_pt;
#ifdef OHCI_FILL_TRACE
	int i;
#endif

	if (index > urb_priv->length) {
		err("index > length");
		return;
	}
	/* use this td as the next dummy */
	td_pt = urb_priv->td [index];
	td_pt->hwNextTD = 0;

	/* fill the old dummy TD */
	td = urb_priv->td [index] =
			     (td_t *)(m32_swap(urb_priv->ed->hwTailP) & ~0xf);

	td->ed = urb_priv->ed;
	td->next_dl_td = NULL;
	td->index = index;
	td->data = (__u32)data;
#ifdef OHCI_FILL_TRACE
	if (usb_pipebulk(urb_priv->pipe) && usb_pipeout(urb_priv->pipe)) {
		for (i = 0; i < len; i++)
		printf("td->data[%d] %#2x ", i, ((unsigned char *)td->data)[i]);
		printf("\n");
	}
#endif
	if (!len)
		data = 0;

	td->hwINFO = m32_swap(info);
	td->hwCBP = m32_swap((unsigned long)data);
	if (data)
		td->hwBE = m32_swap((unsigned long)(data + len - 1));
	else
		td->hwBE = 0;

	td->hwNextTD = m32_swap((unsigned long)td_pt);

	/* append to queue */
	td->ed->hwTailP = td->hwNextTD;
}

/*-------------------------------------------------------------------------*/

/* prepare all TDs of a transfer */

static void td_submit_job(struct usb_device *dev, unsigned long pipe,
			  void *buffer, int transfer_len,
			  struct devrequest *setup, urb_priv_t *urb,
			  int interval)
{
	ohci_t *ohci = &gohci;
	int data_len = transfer_len;
	void *data;
	int cnt = 0;
	__u32 info = 0;
	unsigned int toggle = 0;

	/* OHCI handles the DATA-toggles itself, we just use the USB-toggle
	 * bits for reseting */
	if (usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe))) {
		toggle = TD_T_TOGGLE;
	} else {
		toggle = TD_T_DATA0;
		usb_settoggle(dev, usb_pipeendpoint(pipe),
				usb_pipeout(pipe), 1);
	}
	urb->td_cnt = 0;
	if (data_len)
		data = buffer;
	else
		data = 0;

	switch (usb_pipetype(pipe)) {
	case PIPE_BULK:
		info = usb_pipeout(pipe)?
			TD_CC | TD_DP_OUT : TD_CC | TD_DP_IN ;
		while (data_len > 4096) {
			td_fill(ohci, info | (cnt? TD_T_TOGGLE:toggle),
				data, 4096, dev, cnt, urb);
			data += 4096; data_len -= 4096; cnt++;
		}
		info = usb_pipeout(pipe)?
			TD_CC | TD_DP_OUT : TD_CC | TD_R | TD_DP_IN ;
		td_fill(ohci, info | (cnt? TD_T_TOGGLE:toggle), data,
			data_len, dev, cnt, urb);
		cnt++;

		if (!ohci->sleeping) {
			/* start bulk list */
			writel(OHCI_BLF, &ohci->regs->cmdstatus);
		}
		break;

	case PIPE_CONTROL:
		/* Setup phase */
		info = TD_CC | TD_DP_SETUP | TD_T_DATA0;
		td_fill(ohci, info, setup, 8, dev, cnt++, urb);

		/* Optional Data phase */
		if (data_len > 0) {
			info = usb_pipeout(pipe)?
				TD_CC | TD_R | TD_DP_OUT | TD_T_DATA1 :
				TD_CC | TD_R | TD_DP_IN | TD_T_DATA1;
			/* NOTE:  mishandles transfers >8K, some >4K */
			td_fill(ohci, info, data, data_len, dev, cnt++, urb);
		}

		/* Status phase */
		info = usb_pipeout(pipe)?
			TD_CC | TD_DP_IN | TD_T_DATA1:
			TD_CC | TD_DP_OUT | TD_T_DATA1;
		td_fill(ohci, info, data, 0, dev, cnt++, urb);

		if (!ohci->sleeping) {
			/* start Control list */
			writel(OHCI_CLF, &ohci->regs->cmdstatus);
		}
		break;

	case PIPE_INTERRUPT:
		info = usb_pipeout(urb->pipe)?
			TD_CC | TD_DP_OUT | toggle:
			TD_CC | TD_R | TD_DP_IN | toggle;
		td_fill(ohci, info, data, data_len, dev, cnt++, urb);
		break;
	}
	if (urb->length != cnt)
		dbg("TD LENGTH %d != CNT %d", urb->length, cnt);
}

/*-------------------------------------------------------------------------*
 * Done List handling functions
 *-------------------------------------------------------------------------*/

/* calculate the transfer length and update the urb */

static void dl_transfer_length(td_t *td)
{
	__u32 tdINFO, tdBE, tdCBP;
	urb_priv_t *lurb_priv = td->ed->purb;

	tdINFO = m32_swap(td->hwINFO);
	tdBE   = m32_swap(td->hwBE);
	tdCBP  = m32_swap(td->hwCBP);

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
static void check_status(td_t *td_list)
{
	urb_priv_t *lurb_priv = td_list->ed->purb;
	int	   urb_len    = lurb_priv->length;
	__u32      *phwHeadP  = &td_list->ed->hwHeadP;
	int	   cc;

	cc = TD_CC_GET(m32_swap(td_list->hwINFO));
	if (cc) {
		err(" USB-error: %s (%x)", cc_to_string[cc], cc);

		if (*phwHeadP & m32_swap(0x1)) {
			if (lurb_priv &&
			    ((td_list->index + 1) < urb_len)) {
				*phwHeadP =
					(lurb_priv->td[urb_len - 1]->hwNextTD &\
							m32_swap(0xfffffff0)) |
						   (*phwHeadP & m32_swap(0x2));

				lurb_priv->td_cnt += urb_len -
						     td_list->index - 1;
			} else
				*phwHeadP &= m32_swap(0xfffffff2);
		}
#ifdef CONFIG_MPC5200
		td_list->hwNextTD = 0;
#endif
	}
}

/* replies to the request have to be on a FIFO basis so
 * we reverse the reversed done-list */
static td_t *dl_reverse_done_list(ohci_t *ohci)
{
	__u32 td_list_hc;
	td_t *td_rev = NULL;
	td_t *td_list = NULL;

	td_list_hc = m32_swap(ohci->hcca->done_head) & 0xfffffff0;
	ohci->hcca->done_head = 0;

	while (td_list_hc) {
		td_list = (td_t *)td_list_hc;
		check_status(td_list);
		td_list->next_dl_td = td_rev;
		td_rev = td_list;
		td_list_hc = m32_swap(td_list->hwNextTD) & 0xfffffff0;
	}
	return td_list;
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

static void finish_urb(ohci_t *ohci, urb_priv_t *urb, int status)
{
	if ((status & (ED_OPER | ED_UNLINK)) && (urb->state != URB_DEL))
		urb->finished = sohci_return_job(ohci, urb);
	else
		dbg("finish_urb: strange.., ED state %x, \n", status);
}

/*
 * Used to take back a TD from the host controller. This would normally be
 * called from within dl_done_list, however it may be called directly if the
 * HC no longer sees the TD and it has not appeared on the donelist (after
 * two frames).  This bug has been observed on ZF Micro systems.
 */
static int takeback_td(ohci_t *ohci, td_t *td_list)
{
	ed_t *ed;
	int cc;
	int stat = 0;
	/* urb_t *urb; */
	urb_priv_t *lurb_priv;
	__u32 tdINFO, edHeadP, edTailP;

	tdINFO = m32_swap(td_list->hwINFO);

	ed = td_list->ed;
	lurb_priv = ed->purb;

	dl_transfer_length(td_list);

	lurb_priv->td_cnt++;

	/* error code of transfer */
	cc = TD_CC_GET(tdINFO);
	if (cc) {
		err("USB-error: %s (%x)", cc_to_string[cc], cc);
		stat = cc_to_error[cc];
	}

	/* see if this done list makes for all TD's of current URB,
	* and mark the URB finished if so */
	if (lurb_priv->td_cnt == lurb_priv->length)
		finish_urb(ohci, lurb_priv, ed->state);

	dbg("dl_done_list: processing TD %x, len %x\n",
		lurb_priv->td_cnt, lurb_priv->length);

	if (ed->state != ED_NEW && (!usb_pipeint(lurb_priv->pipe))) {
		edHeadP = m32_swap(ed->hwHeadP) & 0xfffffff0;
		edTailP = m32_swap(ed->hwTailP);

		/* unlink eds if they are not busy */
		if ((edHeadP == edTailP) && (ed->state == ED_OPER))
			ep_unlink(ohci, ed);
	}
	return stat;
}

static int dl_done_list(ohci_t *ohci)
{
	int stat = 0;
	td_t	*td_list = dl_reverse_done_list(ohci);

	while (td_list) {
		td_t	*td_next = td_list->next_dl_td;
		stat = takeback_td(ohci, td_list);
		td_list = td_next;
	}
	return stat;
}

/*-------------------------------------------------------------------------*
 * Virtual Root Hub
 *-------------------------------------------------------------------------*/

/* Device descriptor */
static __u8 root_hub_dev_des[] =
{
	0x12,	    /*	__u8  bLength; */
	0x01,	    /*	__u8  bDescriptorType; Device */
	0x10,	    /*	__u16 bcdUSB; v1.1 */
	0x01,
	0x09,	    /*	__u8  bDeviceClass; HUB_CLASSCODE */
	0x00,	    /*	__u8  bDeviceSubClass; */
	0x00,	    /*	__u8  bDeviceProtocol; */
	0x08,	    /*	__u8  bMaxPacketSize0; 8 Bytes */
	0x00,	    /*	__u16 idVendor; */
	0x00,
	0x00,	    /*	__u16 idProduct; */
	0x00,
	0x00,	    /*	__u16 bcdDevice; */
	0x00,
	0x00,	    /*	__u8  iManufacturer; */
	0x01,	    /*	__u8  iProduct; */
	0x00,	    /*	__u8  iSerialNumber; */
	0x01	    /*	__u8  bNumConfigurations; */
};

/* Configuration descriptor */
static __u8 root_hub_config_des[] =
{
	0x09,	    /*	__u8  bLength; */
	0x02,	    /*	__u8  bDescriptorType; Configuration */
	0x19,	    /*	__u16 wTotalLength; */
	0x00,
	0x01,	    /*	__u8  bNumInterfaces; */
	0x01,	    /*	__u8  bConfigurationValue; */
	0x00,	    /*	__u8  iConfiguration; */
	0x40,	    /*	__u8  bmAttributes;
	 Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup, 4..0: resvd */
	0x00,	    /*	__u8  MaxPower; */

	/* interface */
	0x09,	    /*	__u8  if_bLength; */
	0x04,	    /*	__u8  if_bDescriptorType; Interface */
	0x00,	    /*	__u8  if_bInterfaceNumber; */
	0x00,	    /*	__u8  if_bAlternateSetting; */
	0x01,	    /*	__u8  if_bNumEndpoints; */
	0x09,	    /*	__u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,	    /*	__u8  if_bInterfaceSubClass; */
	0x00,	    /*	__u8  if_bInterfaceProtocol; */
	0x00,	    /*	__u8  if_iInterface; */

	/* endpoint */
	0x07,	    /*	__u8  ep_bLength; */
	0x05,	    /*	__u8  ep_bDescriptorType; Endpoint */
	0x81,	    /*	__u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,	    /*	__u8  ep_bmAttributes; Interrupt */
	0x02,	    /*	__u16 ep_wMaxPacketSize; ((MAX_ROOT_PORTS + 1) / 8 */
	0x00,
	0xff	    /*	__u8  ep_bInterval; 255 ms */
};

static unsigned char root_hub_str_index0[] =
{
	0x04,			/*  __u8  bLength; */
	0x03,			/*  __u8  bDescriptorType; String-descriptor */
	0x09,			/*  __u8  lang ID */
	0x04,			/*  __u8  lang ID */
};

static unsigned char root_hub_str_index1[] =
{
	28,			/*  __u8  bLength; */
	0x03,			/*  __u8  bDescriptorType; String-descriptor */
	'O',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'H',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'C',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'I',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	' ',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'R',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'o',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'o',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	't',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	' ',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'H',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'u',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
	'b',			/*  __u8  Unicode */
	0,				/*  __u8  Unicode */
};

/* Hub class-specific descriptor is constructed dynamically */

/*-------------------------------------------------------------------------*/

#define OK(x)			len = (x); break
#ifdef DEBUG
#define WR_RH_STAT(x)		{info("WR:status %#8x", (x)); writel((x), \
						&gohci.regs->roothub.status); }
#define WR_RH_PORTSTAT(x)	{info("WR:portstatus[%d] %#8x", wIndex-1, \
		(x)); writel((x), &gohci.regs->roothub.portstatus[wIndex-1]); }
#else
#define WR_RH_STAT(x)		writel((x), &gohci.regs->roothub.status)
#define WR_RH_PORTSTAT(x)	writel((x), \
				    &gohci.regs->roothub.portstatus[wIndex-1])
#endif
#define RD_RH_STAT		roothub_status(&gohci)
#define RD_RH_PORTSTAT		roothub_portstatus(&gohci, wIndex-1)

/* request to virtual root hub */

int rh_check_port_status(ohci_t *controller)
{
	__u32 temp, ndp, i;
	int res;

	res = -1;
	temp = roothub_a(controller);
	ndp = (temp & RH_A_NDP);
#ifdef CONFIG_AT91C_PQFP_UHPBUG
	ndp = (ndp == 2) ? 1:0;
#endif
	for (i = 0; i < ndp; i++) {
		temp = roothub_portstatus(controller, i);
		/* check for a device disconnect */
		if (((temp & (RH_PS_PESC | RH_PS_CSC)) ==
			(RH_PS_PESC | RH_PS_CSC)) &&
			((temp & RH_PS_CCS) == 0)) {
			res = i;
			break;
		}
	}
	return res;
}

static int ohci_submit_rh_msg(struct usb_device *dev, unsigned long pipe,
		void *buffer, int transfer_len, struct devrequest *cmd)
{
	void *data = buffer;
	int leni = transfer_len;
	int len = 0;
	int stat = 0;
	__u32 datab[4];
	__u8 *data_buf = (__u8 *)datab;
	__u16 bmRType_bReq;
	__u16 wValue;
	__u16 wIndex;
	__u16 wLength;

#ifdef DEBUG
pkt_print(NULL, dev, pipe, buffer, transfer_len,
	  cmd, "SUB(rh)", usb_pipein(pipe));
#else
	wait_ms(1);
#endif
	if (usb_pipeint(pipe)) {
		info("Root-Hub submit IRQ: NOT implemented");
		return 0;
	}

	bmRType_bReq  = cmd->requesttype | (cmd->request << 8);
	wValue	      = le16_to_cpu(cmd->value);
	wIndex	      = le16_to_cpu(cmd->index);
	wLength	      = le16_to_cpu(cmd->length);

	info("Root-Hub: adr: %2x cmd(%1x): %08x %04x %04x %04x",
		dev->devnum, 8, bmRType_bReq, wValue, wIndex, wLength);

	switch (bmRType_bReq) {
	/* Request Destination:
	   without flags: Device,
	   RH_INTERFACE: interface,
	   RH_ENDPOINT: endpoint,
	   RH_CLASS means HUB here,
	   RH_OTHER | RH_CLASS	almost ever means HUB_PORT here
	*/

	case RH_GET_STATUS:
		*(__u16 *) data_buf = cpu_to_le16(1);
		OK(2);
	case RH_GET_STATUS | RH_INTERFACE:
		*(__u16 *) data_buf = cpu_to_le16(0);
		OK(2);
	case RH_GET_STATUS | RH_ENDPOINT:
		*(__u16 *) data_buf = cpu_to_le16(0);
		OK(2);
	case RH_GET_STATUS | RH_CLASS:
		*(__u32 *) data_buf = cpu_to_le32(
				RD_RH_STAT & ~(RH_HS_CRWE | RH_HS_DRWE));
		OK(4);
	case RH_GET_STATUS | RH_OTHER | RH_CLASS:
		*(__u32 *) data_buf = cpu_to_le32(RD_RH_PORTSTAT);
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
		case (RH_PORT_ENABLE):        WR_RH_PORTSTAT(RH_PS_CCS);  OK(0);
		case (RH_PORT_SUSPEND):       WR_RH_PORTSTAT(RH_PS_POCI); OK(0);
		case (RH_PORT_POWER):         WR_RH_PORTSTAT(RH_PS_LSDA); OK(0);
		case (RH_C_PORT_CONNECTION):  WR_RH_PORTSTAT(RH_PS_CSC);  OK(0);
		case (RH_C_PORT_ENABLE):      WR_RH_PORTSTAT(RH_PS_PESC); OK(0);
		case (RH_C_PORT_SUSPEND):     WR_RH_PORTSTAT(RH_PS_PSSC); OK(0);
		case (RH_C_PORT_OVER_CURRENT):WR_RH_PORTSTAT(RH_PS_OCIC); OK(0);
		case (RH_C_PORT_RESET):       WR_RH_PORTSTAT(RH_PS_PRSC); OK(0);
		}
		break;

	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		switch (wValue) {
		case (RH_PORT_SUSPEND):
			WR_RH_PORTSTAT(RH_PS_PSS);  OK(0);
		case (RH_PORT_RESET): /* BUG IN HUP CODE *********/
			if (RD_RH_PORTSTAT & RH_PS_CCS)
				WR_RH_PORTSTAT(RH_PS_PRS);
			OK(0);
		case (RH_PORT_POWER):
			WR_RH_PORTSTAT(RH_PS_PPS);
			wait_ms(100);
			OK(0);
		case (RH_PORT_ENABLE): /* BUG IN HUP CODE *********/
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
		case (0x01): /* device descriptor */
			len = min_t(unsigned int,
					leni,
					min_t(unsigned int,
					sizeof(root_hub_dev_des),
					wLength));
			data_buf = root_hub_dev_des; OK(len);
		case (0x02): /* configuration descriptor */
			len = min_t(unsigned int,
					leni,
					min_t(unsigned int,
					sizeof(root_hub_config_des),
					wLength));
			data_buf = root_hub_config_des; OK(len);
		case (0x03): /* string descriptors */
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

		data_buf [0] = 9;		/* min length; */
		data_buf [1] = 0x29;
		data_buf [2] = temp & RH_A_NDP;
#ifdef CONFIG_AT91C_PQFP_UHPBUG
		data_buf [2] = (data_buf [2] == 2) ? 1:0;
#endif
		data_buf [3] = 0;
		if (temp & RH_A_PSM)	/* per-port power switching? */
			data_buf [3] |= 0x1;
		if (temp & RH_A_NOCP)	/* no overcurrent reporting? */
			data_buf [3] |= 0x10;
		else if (temp & RH_A_OCPM)/* per-port overcurrent reporting? */
			data_buf [3] |= 0x8;

		/* corresponds to data_buf[4-7] */
		datab [1] = 0;
		data_buf [5] = (temp & RH_A_POTPGT) >> 24;
		temp = roothub_b(&gohci);
		data_buf [7] = temp & RH_B_DR;
		if (data_buf [2] < 7) {
			data_buf [8] = 0xff;
		} else {
			data_buf [0] += 2;
			data_buf [8] = (temp & RH_B_DR) >> 8;
			data_buf [10] = data_buf [9] = 0xff;
		}

		len = min_t(unsigned int, leni,
			    min_t(unsigned int, data_buf [0], wLength));
		OK(len);
	}

	case RH_GET_CONFIGURATION:	*(__u8 *) data_buf = 0x01; OK(1);

	case RH_SET_CONFIGURATION:	WR_RH_STAT(0x10000); OK(0);

	default:
		dbg("unsupported root hub command");
		stat = USB_ST_STALLED;
	}

#ifdef	DEBUG
	ohci_dump_roothub(&gohci, 1);
#else
	wait_ms(1);
#endif

	len = min_t(int, len, leni);
	if (data != data_buf)
	    memcpy(data, data_buf, len);
	dev->act_len = len;
	dev->status = stat;

#ifdef DEBUG
	pkt_print(NULL, dev, pipe, buffer,
		  transfer_len, cmd, "RET(rh)", 0/*usb_pipein(pipe)*/);
#else
	wait_ms(1);
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
	urb_priv_t *urb;

	urb = malloc(sizeof(urb_priv_t));
	memset(urb, 0, sizeof(urb_priv_t));

	urb->dev = dev;
	urb->pipe = pipe;
	urb->transfer_buffer = buffer;
	urb->transfer_buffer_length = transfer_len;
	urb->interval = interval;

	/* device pulled? Shortcut the action. */
	if (devgone == dev) {
		dev->status = USB_ST_CRC_ERR;
		return 0;
	}

#ifdef DEBUG
	urb->actual_length = 0;
	pkt_print(urb, dev, pipe, buffer, transfer_len,
		  setup, "SUB", usb_pipein(pipe));
#else
	wait_ms(1);
#endif
	if (!maxsize) {
		err("submit_common_message: pipesize for pipe %lx is zero",
			pipe);
		return -1;
	}

	if (sohci_submit_job(urb, setup) < 0) {
		err("sohci_submit_job failed");
		return -1;
	}

#if 0
	wait_ms(10);
	/* ohci_dump_status(&gohci); */
#endif

	/* allow more time for a BULK device to react - some are slow */
#define BULK_TO	 5000	/* timeout in milliseconds */
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
		if ((stat >= 0) && (stat != 0xff) && (urb->finished)) {
			/* 0xff is returned for an SF-interrupt */
			break;
		}

		if (--timeout) {
			wait_ms(1);
			if (!urb->finished)
				dbg("*");

		} else {
			err("CTL:TIMEOUT ");
			dbg("submit_common_msg: TO status %x\n", stat);
			urb->finished = 1;
			stat = USB_ST_CRC_ERR;
			break;
		}
	}

	dev->status = stat;
	dev->act_len = transfer_len;

#ifdef DEBUG
	pkt_print(urb, dev, pipe, buffer, transfer_len,
		  setup, "RET(ctlr)", usb_pipein(pipe));
#else
	wait_ms(1);
#endif

	/* free TDs in urb_priv */
	if (!usb_pipeint(pipe))
		urb_free_priv(urb);
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
	pkt_print(NULL, dev, pipe, buffer, transfer_len,
		  setup, "SUB", usb_pipein(pipe));
#else
	wait_ms(1);
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
	return submit_common_msg(dev, pipe, buffer, transfer_len, NULL,
			interval);
}

/*-------------------------------------------------------------------------*
 * HC functions
 *-------------------------------------------------------------------------*/

/* reset the HC and BUS */

static int hc_reset(ohci_t *ohci)
{
#ifdef CONFIG_PCI_EHCI_DEVNO
	pci_dev_t pdev;
#endif
	int timeout = 30;
	int smm_timeout = 50; /* 0,5 sec */

	dbg("%s\n", __FUNCTION__);

#ifdef CONFIG_PCI_EHCI_DEVNO
	/*
	 *  Some multi-function controllers (e.g. ISP1562) allow root hub
	 * resetting via EHCI registers only.
	 */
	pdev = pci_find_devices(ehci_pci_ids, CONFIG_PCI_EHCI_DEVNO);
	if (pdev != -1) {
		u32 base;
		int timeout = 1000;

		pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0, &base);
		writel(readl(base + EHCI_USBCMD_OFF) | EHCI_USBCMD_HCRESET,
			base + EHCI_USBCMD_OFF);

		while (readl(base + EHCI_USBCMD_OFF) & EHCI_USBCMD_HCRESET) {
			if (timeout-- <= 0) {
				printf("USB RootHub reset timed out!");
				break;
			}
			udelay(1);
		}
	} else
		printf("No EHCI func at %d index!\n", CONFIG_PCI_EHCI_DEVNO);
#endif
	if (readl(&ohci->regs->control) & OHCI_CTRL_IR) {
		/* SMM owns the HC */
		writel(OHCI_OCR, &ohci->regs->cmdstatus);/* request ownership */
		info("USB HC TakeOver from SMM");
		while (readl(&ohci->regs->control) & OHCI_CTRL_IR) {
			wait_ms(10);
			if (--smm_timeout == 0) {
				err("USB HC TakeOver failed!");
				return -1;
			}
		}
	}

	/* Disable HC interrupts */
	writel(OHCI_INTR_MIE, &ohci->regs->intrdisable);

	dbg("USB HC reset_hc usb-%s: ctrl = 0x%X ;\n",
		ohci->slot_name,
		readl(&ohci->regs->control));

	/* Reset USB (needed by some controllers) */
	ohci->hc_control = 0;
	writel(ohci->hc_control, &ohci->regs->control);

	/* HC Reset requires max 10 us delay */
	writel(OHCI_HCR,  &ohci->regs->cmdstatus);
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

static int hc_start(ohci_t *ohci)
{
	__u32 mask;
	unsigned int fminterval;

	ohci->disabled = 1;

	/* Tell the controller where the control and bulk lists are
	 * The lists are empty now. */

	writel(0, &ohci->regs->ed_controlhead);
	writel(0, &ohci->regs->ed_bulkhead);

	writel((__u32)ohci->hcca, &ohci->regs->hcca); /* a reset clears this */

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
#endif	/* OHCI_USE_NPS */

#define mdelay(n) ({unsigned long msec = (n); while (msec--) udelay(1000); })
	/* POTPGT delay is bits 24-31, in 2 ms units. */
	mdelay((roothub_a(ohci) >> 23) & 0x1fe);

	/* connect the virtual root hub */
	ohci->rh.devnum = 0;

	return 0;
}

/*-------------------------------------------------------------------------*/

/* Poll USB interrupt. */
void usb_event_poll(void)
{
	hc_interrupt();
}

/* an interrupt happens */

static int hc_interrupt(void)
{
	ohci_t *ohci = &gohci;
	struct ohci_regs *regs = ohci->regs;
	int ints;
	int stat = -1;

	if ((ohci->hcca->done_head != 0) &&
				!(m32_swap(ohci->hcca->done_head) & 0x01)) {
		ints =  OHCI_INTR_WDH;
	} else {
		ints = readl(&regs->intrstatus);
		if (ints == ~(u32)0) {
			ohci->disabled++;
			err("%s device removed!", ohci->slot_name);
			return -1;
		} else {
			ints &= readl(&regs->intrenable);
			if (ints == 0) {
				dbg("hc_interrupt: returning..\n");
				return 0xff;
			}
		}
	}

	/* dbg("Interrupt: %x frame: %x", ints,
					le16_to_cpu(ohci->hcca->frame_no)); */

	if (ints & OHCI_INTR_RHSC)
		stat = 0xff;

	if (ints & OHCI_INTR_UE) {
		ohci->disabled++;
		err("OHCI Unrecoverable Error, controller usb-%s disabled",
			ohci->slot_name);
		/* e.g. due to PCI Master/Target Abort */

#ifdef	DEBUG
		ohci_dump(ohci, 1);
#else
		wait_ms(1);
#endif
		/* FIXME: be optimistic, hope that bug won't repeat often. */
		/* Make some non-interrupt context restart the controller. */
		/* Count and limit the retries though; either hardware or */
		/* software errors can go forever... */
		hc_reset(ohci);
		return -1;
	}

	if (ints & OHCI_INTR_WDH) {
		wait_ms(1);
		writel(OHCI_INTR_WDH, &regs->intrdisable);
		(void)readl(&regs->intrdisable); /* flush */
		stat = dl_done_list(&gohci);
		writel(OHCI_INTR_WDH, &regs->intrenable);
		(void)readl(&regs->intrdisable); /* flush */
	}

	if (ints & OHCI_INTR_SO) {
		dbg("USB Schedule overrun\n");
		writel(OHCI_INTR_SO, &regs->intrenable);
		stat = -1;
	}

	/* FIXME:  this assumes SOF (1/ms) interrupts don't get lost... */
	if (ints & OHCI_INTR_SF) {
		unsigned int frame = m16_swap(ohci->hcca->frame_no) & 1;
		wait_ms(1);
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

static void hc_release_ohci(ohci_t *ohci)
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

int usb_lowlevel_init(void)
{
#ifdef CONFIG_PCI_OHCI
	pci_dev_t pdev;
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
	/* cpu dependant init */
	if (usb_cpu_init())
		return -1;
#endif

#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
	/*  board dependant init */
	if (usb_board_init())
		return -1;
#endif
	memset(&gohci, 0, sizeof(ohci_t));

	/* align the storage */
	if ((__u32)&ghcca[0] & 0xff) {
		err("HCCA not aligned!!");
		return -1;
	}
	phcca = &ghcca[0];
	info("aligned ghcca %p", phcca);
	memset(&ohci_dev, 0, sizeof(struct ohci_device));
	if ((__u32)&ohci_dev.ed[0] & 0x7) {
		err("EDs not aligned!!");
		return -1;
	}
	memset(gtd, 0, sizeof(td_t) * (NUM_TD + 1));
	if ((__u32)gtd & 0x7) {
		err("TDs not aligned!!");
		return -1;
	}
	ptd = gtd;
	gohci.hcca = phcca;
	memset(phcca, 0, sizeof(struct ohci_hcca));

	gohci.disabled = 1;
	gohci.sleeping = 0;
	gohci.irq = -1;
#ifdef CONFIG_PCI_OHCI
	pdev = pci_find_devices(ohci_pci_ids, CONFIG_PCI_OHCI_DEVNO);

	if (pdev != -1) {
		u16 vid, did;
		u32 base;
		pci_read_config_word(pdev, PCI_VENDOR_ID, &vid);
		pci_read_config_word(pdev, PCI_DEVICE_ID, &did);
		printf("OHCI pci controller (%04x, %04x) found @(%d:%d:%d)\n",
				vid, did, (pdev >> 16) & 0xff,
				(pdev >> 11) & 0x1f, (pdev >> 8) & 0x7);
		pci_read_config_dword(pdev, PCI_BASE_ADDRESS_0, &base);
		printf("OHCI regs address 0x%08x\n", base);
		gohci.regs = (struct ohci_regs *)base;
	} else
		return -1;
#else
	gohci.regs = (struct ohci_regs *)CONFIG_SYS_USB_OHCI_REGS_BASE;
#endif

	gohci.flags = 0;
	gohci.slot_name = CONFIG_SYS_USB_OHCI_SLOT_NAME;

	if (hc_reset (&gohci) < 0) {
		hc_release_ohci (&gohci);
		err ("can't reset usb-%s", gohci.slot_name);
#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
		/* board dependant cleanup */
		usb_board_init_fail();
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
		/* cpu dependant cleanup */
		usb_cpu_init_fail();
#endif
		return -1;
	}

	if (hc_start(&gohci) < 0) {
		err("can't start usb-%s", gohci.slot_name);
		hc_release_ohci(&gohci);
		/* Initialization failed */
#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
		/* board dependant cleanup */
		usb_board_stop();
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
		/* cpu dependant cleanup */
		usb_cpu_stop();
#endif
		return -1;
	}

#ifdef	DEBUG
	ohci_dump(&gohci, 1);
#else
	wait_ms(1);
#endif
	ohci_inited = 1;
	return 0;
}

int usb_lowlevel_stop(void)
{
	/* this gets called really early - before the controller has */
	/* even been initialized! */
	if (!ohci_inited)
		return 0;
	/* TODO release any interrupts, etc. */
	/* call hc_release_ohci() here ? */
	hc_reset(&gohci);

#ifdef CONFIG_SYS_USB_OHCI_BOARD_INIT
	/* board dependant cleanup */
	if (usb_board_stop())
		return -1;
#endif

#ifdef CONFIG_SYS_USB_OHCI_CPU_INIT
	/* cpu dependant cleanup */
	if (usb_cpu_stop())
		return -1;
#endif
	/* This driver is no longer initialised. It needs a new low-level
	 * init (board/cpu) before it can be used again. */
	ohci_inited = 0;
	return 0;
}
