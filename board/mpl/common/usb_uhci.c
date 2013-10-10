/*
 * Part of this code has been derived from linux:
 * Universal Host Controller Interface driver for USB (take II).
 *
 * (c) 1999-2001 Georg Acher, acher@in.tum.de (executive slave) (base guitar)
 *               Deti Fliegl, deti@fliegl.de (executive slave) (lead voice)
 *               Thomas Sailer, sailer@ife.ee.ethz.ch (chief consultant) (cheer leader)
 *               Roman Weissgaerber, weissg@vienna.at (virt root hub) (studio porter)
 * (c) 2000      Yggdrasil Computing, Inc. (port of new PCI interface support
 *               from usb-ohci.c by Adam Richter, adam@yggdrasil.com).
 * (C) 2000      David Brownell, david-b@pacbell.net (usb-ohci.c)
 *
 * HW-initalization based on material of
 *
 * (C) Copyright 1999 Linus Torvalds
 * (C) Copyright 1999 Johannes Erdfelt
 * (C) Copyright 1999 Randy Dunlap
 * (C) Copyright 1999 Gregory P. Smith
 *
 *
 * Adapted for U-Boot:
 * (C) Copyright 2001 Denis Peter, MPL AG Switzerland
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/**********************************************************************
 * How it works:
 * -------------
 * The framelist / Transfer descriptor / Queue Heads are similar like
 * in the linux usb_uhci.c.
 *
 * During initialization, the following skeleton is allocated in init_skel:
 *
 *         framespecific           |           common chain
 *
 * framelist[]
 * [  0 ]-----> TD ---------\
 * [  1 ]-----> TD ----------> TD ------> QH -------> QH -------> QH ---> NULL
 *   ...        TD ---------/
 * [1023]-----> TD --------/
 *
 *              ^^             ^^         ^^          ^^          ^^
 *              7 TDs for      1 TD for   Start of    Start of    End Chain
 *              INT (2-128ms)  1ms-INT    CTRL Chain  BULK Chain
 *
 *
 * Since this is a bootloader, the isochronous transfer descriptor have been removed.
 *
 * Interrupt Transfers.
 * --------------------
 * For Interrupt transfers USB_MAX_TEMP_INT_TD Transfer descriptor are available. They
 * will be inserted after the appropriate (depending the interval setting) skeleton TD.
 * If an interrupt has been detected the dev->irqhandler is called. The status and number
 * of transfered bytes is stored in dev->irq_status resp. dev->irq_act_len. If the
 * dev->irqhandler returns 0, the interrupt TD is removed and disabled. If an 1 is returned,
 * the interrupt TD will be reactivated.
 *
 * Control Transfers
 * -----------------
 * Control Transfers are issued by filling the tmp_td with the appropriate data and connect
 * them to the qh_cntrl queue header. Before other control/bulk transfers can be issued,
 * the programm has to wait for completion. This does not allows asynchronous data transfer.
 *
 * Bulk Transfers
 * --------------
 * Bulk Transfers are issued by filling the tmp_td with the appropriate data and connect
 * them to the qh_bulk queue header. Before other control/bulk transfers can be issued,
 * the programm has to wait for completion. This does not allows asynchronous data transfer.
 *
 *
 */

#include <common.h>
#include <pci.h>

#ifdef CONFIG_USB_UHCI

#include <usb.h>
#include "usb_uhci.h"

#define USB_MAX_TEMP_TD      128  /* number of temporary TDs for bulk and control transfers */
#define USB_MAX_TEMP_INT_TD  32   /* number of temporary TDs for Interrupt transfers */


#undef USB_UHCI_DEBUG

#ifdef	USB_UHCI_DEBUG
#define	USB_UHCI_PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define USB_UHCI_PRINTF(fmt,args...)
#endif


static int irqvec = -1;            /* irq vector, if -1 uhci is stopped / reseted */
unsigned int usb_base_addr;       /* base address */

static uhci_td_t td_int[8];        /* Interrupt Transfer descriptors */
static uhci_qh_t qh_cntrl;         /* control Queue Head */
static uhci_qh_t qh_bulk;          /*  bulk Queue Head */
static uhci_qh_t qh_end;           /* end Queue Head */
static uhci_td_t td_last;          /* last TD (linked with end chain) */

/* temporary tds */
static uhci_td_t tmp_td[USB_MAX_TEMP_TD];          /* temporary bulk/control td's  */
static uhci_td_t tmp_int_td[USB_MAX_TEMP_INT_TD];  /* temporary interrupt td's  */

static unsigned long framelist[1024] __attribute__ ((aligned (0x1000))); /* frame list */

static struct virt_root_hub rh;   /* struct for root hub */

/**********************************************************************
 * some forward decleration
 */
int uhci_submit_rh_msg(struct usb_device *dev, unsigned long pipe,
						void *buffer, int transfer_len,struct devrequest *setup);

/* fill a td with the approproiate data. Link, status, info and buffer
 * are used by the USB controller itselfes, dev is used to identify the
 * "connected" device
 */
void usb_fill_td(uhci_td_t* td,unsigned long link,unsigned long status,
					unsigned long info, unsigned long buffer, unsigned long dev)
{
	td->link=swap_32(link);
	td->status=swap_32(status);
	td->info=swap_32(info);
	td->buffer=swap_32(buffer);
	td->dev_ptr=dev;
}

/* fill a qh with the approproiate data. Head and element are used by the USB controller
 * itselfes. As soon as a valid dev_ptr is filled, a td chain is connected to the qh.
 * Please note, that after completion of the td chain, the entry element is removed /
 * marked invalid by the USB controller.
 */
void usb_fill_qh(uhci_qh_t* qh,unsigned long head,unsigned long element)
{
	qh->head=swap_32(head);
	qh->element=swap_32(element);
	qh->dev_ptr=0L;
}

/* get the status of a td->status
 */
unsigned long usb_uhci_td_stat(unsigned long status)
{
	unsigned long result=0;
	result |= (status & TD_CTRL_NAK)      ? USB_ST_NAK_REC : 0;
	result |= (status & TD_CTRL_STALLED)  ? USB_ST_STALLED : 0;
	result |= (status & TD_CTRL_DBUFERR)  ? USB_ST_BUF_ERR : 0;
	result |= (status & TD_CTRL_BABBLE)   ? USB_ST_BABBLE_DET : 0;
	result |= (status & TD_CTRL_CRCTIMEO) ? USB_ST_CRC_ERR : 0;
	result |= (status & TD_CTRL_BITSTUFF) ? USB_ST_BIT_ERR : 0;
	result |= (status & TD_CTRL_ACTIVE)   ? USB_ST_NOT_PROC : 0;
	return result;
}

/* get the status and the transfered len of a td chain.
 * called from the completion handler
 */
int usb_get_td_status(uhci_td_t *td,struct usb_device *dev)
{
	unsigned long temp,info;
	unsigned long stat;
	uhci_td_t *mytd=td;

	if(dev->devnum==rh.devnum)
		return 0;
	dev->act_len=0;
	stat=0;
	do {
		temp=swap_32((unsigned long)mytd->status);
		stat=usb_uhci_td_stat(temp);
		info=swap_32((unsigned long)mytd->info);
		if(((info & 0xff)!= USB_PID_SETUP) &&
				(((info >> 21) & 0x7ff)!= 0x7ff) &&
				(temp & 0x7FF)!=0x7ff)
		{  /* if not setup and not null data pack */
			dev->act_len+=(temp & 0x7FF) + 1; /* the transfered len is act_len + 1 */
		}
		if(stat) {           /* status no ok */
			dev->status=stat;
			return -1;
		}
		temp=swap_32((unsigned long)mytd->link);
		mytd=(uhci_td_t *)(temp & 0xfffffff0);
	}while((temp & 0x1)==0); /* process all TDs */
	dev->status=stat;
	return 0; /* Ok */
}


/*-------------------------------------------------------------------
 *                         LOW LEVEL STUFF
 *          assembles QHs und TDs for control, bulk and iso
 *-------------------------------------------------------------------*/

/* Submits a control message. That is a Setup, Data and Status transfer.
 * Routine does not wait for completion.
 */
int submit_control_msg(struct usb_device *dev, unsigned long pipe, void *buffer,
										int transfer_len,struct devrequest *setup)
{
	unsigned long destination, status;
	int maxsze = usb_maxpacket(dev, pipe);
	unsigned long dataptr;
	int len;
	int pktsze;
	int i=0;

	if (!maxsze) {
		USB_UHCI_PRINTF("uhci_submit_control_urb: pipesize for pipe %lx is zero\n", pipe);
		return -1;
	}
	if(((pipe>>8)&0x7f)==rh.devnum) {
		/* this is the root hub -> redirect it */
		return uhci_submit_rh_msg(dev,pipe,buffer,transfer_len,setup);
	}
	USB_UHCI_PRINTF("uhci_submit_control start len %x, maxsize %x\n",transfer_len,maxsze);
	/* The "pipe" thing contains the destination in bits 8--18 */
	destination = (pipe & PIPE_DEVEP_MASK) | USB_PID_SETUP; /* Setup stage */
	/* 3 errors */
	status = (pipe & TD_CTRL_LS) | TD_CTRL_ACTIVE | (3 << 27);
	/* (urb->transfer_flags & USB_DISABLE_SPD ? 0 : TD_CTRL_SPD); */
	/*  Build the TD for the control request, try forever, 8 bytes of data */
	usb_fill_td(&tmp_td[i],UHCI_PTR_TERM ,status, destination | (7 << 21),(unsigned long)setup,(unsigned long)dev);
#if 0
	{
		char *sp=(char *)setup;
		printf("SETUP to pipe %lx: %x %x %x %x %x %x %x %x\n", pipe,
		    sp[0],sp[1],sp[2],sp[3],sp[4],sp[5],sp[6],sp[7]);
	}
#endif
	dataptr = (unsigned long)buffer;
	len=transfer_len;

	/* If direction is "send", change the frame from SETUP (0x2D)
	   to OUT (0xE1). Else change it from SETUP to IN (0x69). */
	destination = (pipe & PIPE_DEVEP_MASK) | ((pipe & USB_DIR_IN)==0 ? USB_PID_OUT : USB_PID_IN);
	while (len > 0) {
		/* data stage */
		pktsze = len;
		i++;
		if (pktsze > maxsze)
			pktsze = maxsze;
		destination ^= 1 << TD_TOKEN_TOGGLE;	/* toggle DATA0/1 */
		usb_fill_td(&tmp_td[i],UHCI_PTR_TERM, status, destination | ((pktsze - 1) << 21),dataptr,(unsigned long)dev);	/* Status, pktsze bytes of data */
		tmp_td[i-1].link=swap_32((unsigned long)&tmp_td[i]);

		dataptr += pktsze;
		len -= pktsze;
	}

	/*  Build the final TD for control status */
	/* It's only IN if the pipe is out AND we aren't expecting data */

	destination &= ~UHCI_PID;
	if (((pipe & USB_DIR_IN)==0) || (transfer_len == 0))
		destination |= USB_PID_IN;
	else
		destination |= USB_PID_OUT;
	destination |= 1 << TD_TOKEN_TOGGLE;	/* End in Data1 */
	i++;
	status &=~TD_CTRL_SPD;
	/* no limit on errors on final packet , 0 bytes of data */
	usb_fill_td(&tmp_td[i],UHCI_PTR_TERM, status | TD_CTRL_IOC, destination | (UHCI_NULL_DATA_SIZE << 21),0,(unsigned long)dev);
	tmp_td[i-1].link=swap_32((unsigned long)&tmp_td[i]);	/* queue status td */
	/*	usb_show_td(i+1);*/
	USB_UHCI_PRINTF("uhci_submit_control end (%d tmp_tds used)\n",i);
	/* first mark the control QH element terminated */
	qh_cntrl.element=0xffffffffL;
	/* set qh active */
	qh_cntrl.dev_ptr=(unsigned long)dev;
	/* fill in tmp_td_chain */
	qh_cntrl.element=swap_32((unsigned long)&tmp_td[0]);
	return 0;
}

/*-------------------------------------------------------------------
 * Prepare TDs for bulk transfers.
 */
int submit_bulk_msg(struct usb_device *dev, unsigned long pipe, void *buffer,int transfer_len)
{
	unsigned long destination, status,info;
	unsigned long dataptr;
	int maxsze = usb_maxpacket(dev, pipe);
	int len;
	int i=0;

	if(transfer_len < 0) {
		printf("Negative transfer length in submit_bulk\n");
		return -1;
	}
	if (!maxsze)
		return -1;
	/* The "pipe" thing contains the destination in bits 8--18. */
	destination = (pipe & PIPE_DEVEP_MASK) | usb_packetid (pipe);
	/* 3 errors */
	status = (pipe & TD_CTRL_LS) | TD_CTRL_ACTIVE | (3 << 27);
	/*	((urb->transfer_flags & USB_DISABLE_SPD) ? 0 : TD_CTRL_SPD) | (3 << 27); */
	/* Build the TDs for the bulk request */
	len = transfer_len;
	dataptr = (unsigned long)buffer;
	do {
		int pktsze = len;
		if (pktsze > maxsze)
			pktsze = maxsze;
		/* pktsze bytes of data  */
		info = destination | (((pktsze - 1)&UHCI_NULL_DATA_SIZE) << 21) |
			(usb_gettoggle (dev, usb_pipeendpoint (pipe), usb_pipeout (pipe)) << TD_TOKEN_TOGGLE);

		if((len-pktsze)==0)
			status |= TD_CTRL_IOC;	/* last one generates INT */

		usb_fill_td(&tmp_td[i],UHCI_PTR_TERM, status, info,dataptr,(unsigned long)dev);	/* Status, pktsze bytes of data */
		if(i>0)
			tmp_td[i-1].link=swap_32((unsigned long)&tmp_td[i]);
		i++;
		dataptr += pktsze;
		len -= pktsze;
		usb_dotoggle (dev, usb_pipeendpoint (pipe), usb_pipeout (pipe));
	} while (len > 0);
	/* first mark the bulk QH element terminated */
	qh_bulk.element=0xffffffffL;
	/* set qh active */
	qh_bulk.dev_ptr=(unsigned long)dev;
	/* fill in tmp_td_chain */
	qh_bulk.element=swap_32((unsigned long)&tmp_td[0]);
	return 0;
}


/* search a free interrupt td
 */
uhci_td_t *uhci_alloc_int_td(void)
{
	int i;
	for(i=0;i<USB_MAX_TEMP_INT_TD;i++) {
		if(tmp_int_td[i].dev_ptr==0) /* no device assigned -> free TD */
			return &tmp_int_td[i];
	}
	return NULL;
}

#if 0
void uhci_show_temp_int_td(void)
{
	int i;
	for(i=0;i<USB_MAX_TEMP_INT_TD;i++) {
		if((tmp_int_td[i].dev_ptr&0x01)!=0x1L) /* no device assigned -> free TD */
			printf("temp_td %d is assigned to dev %lx\n",i,tmp_int_td[i].dev_ptr);
	}
	printf("all others temp_tds are free\n");
}
#endif
/*-------------------------------------------------------------------
 * submits USB interrupt (ie. polling ;-)
 */
int submit_int_msg(struct usb_device *dev, unsigned long pipe, void *buffer,int transfer_len, int interval)
{
	int nint, n;
	unsigned long status, destination;
	unsigned long info,tmp;
	uhci_td_t *mytd;
	if (interval < 0 || interval >= 256)
		return -1;

	if (interval == 0)
		nint = 0;
	else {
		for (nint = 0, n = 1; nint <= 8; nint++, n += n)	/* round interval down to 2^n */
		 {
			if(interval < n) {
				interval = n / 2;
				break;
			}
		}
		nint--;
	}

	USB_UHCI_PRINTF("Rounded interval to %i, chain  %i\n", interval, nint);
	mytd=uhci_alloc_int_td();
	if(mytd==NULL) {
		printf("No free INT TDs found\n");
		return -1;
	}
	status = (pipe & TD_CTRL_LS) | TD_CTRL_ACTIVE | TD_CTRL_IOC | (3 << 27);
/*		(urb->transfer_flags & USB_DISABLE_SPD ? 0 : TD_CTRL_SPD) | (3 << 27);
*/

	destination =(pipe & PIPE_DEVEP_MASK) | usb_packetid (pipe) | (((transfer_len - 1) & 0x7ff) << 21);

	info = destination | (usb_gettoggle(dev, usb_pipeendpoint(pipe), usb_pipeout(pipe)) << TD_TOKEN_TOGGLE);
	tmp = swap_32(td_int[nint].link);
	usb_fill_td(mytd,tmp,status, info,(unsigned long)buffer,(unsigned long)dev);
	/* Link it */
	tmp = swap_32((unsigned long)mytd);
	td_int[nint].link=tmp;

	usb_dotoggle (dev, usb_pipeendpoint (pipe), usb_pipeout (pipe));

	return 0;
}

/**********************************************************************
 * Low Level functions
 */


void reset_hc(void)
{

	/* Global reset for 100ms */
	out16r( usb_base_addr + USBPORTSC1,0x0204);
	out16r( usb_base_addr + USBPORTSC2,0x0204);
	out16r( usb_base_addr + USBCMD,USBCMD_GRESET | USBCMD_RS);
	/* Turn off all interrupts */
	out16r(usb_base_addr + USBINTR,0);
	mdelay(50);
	out16r( usb_base_addr + USBCMD,0);
	mdelay(10);
}

void start_hc(void)
{
	int timeout = 1000;

	while(in16r(usb_base_addr + USBCMD) & USBCMD_HCRESET) {
		if (!--timeout) {
			printf("USBCMD_HCRESET timed out!\n");
			break;
		}
	}
	/* Turn on all interrupts */
	out16r(usb_base_addr + USBINTR,USBINTR_TIMEOUT | USBINTR_RESUME | USBINTR_IOC | USBINTR_SP);
	/* Start at frame 0 */
	out16r(usb_base_addr + USBFRNUM,0);
	/* set Framebuffer base address */
	out32r(usb_base_addr+USBFLBASEADD,(unsigned long)&framelist);
	/* Run and mark it configured with a 64-byte max packet */
	out16r(usb_base_addr + USBCMD,USBCMD_RS | USBCMD_CF | USBCMD_MAXP);
}

/* Initialize the skeleton
 */
void usb_init_skel(void)
{
	unsigned long temp;
	int n;

	for(n=0;n<USB_MAX_TEMP_INT_TD;n++)
		tmp_int_td[n].dev_ptr=0L; /* no devices connected */
	/* last td */
	usb_fill_td(&td_last,UHCI_PTR_TERM,TD_CTRL_IOC ,0,0,0L);
  /* usb_fill_td(&td_last,UHCI_PTR_TERM,0,0,0); */
	/* End Queue Header */
	usb_fill_qh(&qh_end,UHCI_PTR_TERM,(unsigned long)&td_last);
	/* Bulk Queue Header */
	temp=(unsigned long)&qh_end;
	usb_fill_qh(&qh_bulk,temp | UHCI_PTR_QH,UHCI_PTR_TERM);
	/* Control Queue Header */
	temp=(unsigned long)&qh_bulk;
	usb_fill_qh(&qh_cntrl, temp | UHCI_PTR_QH,UHCI_PTR_TERM);
	/* 1ms Interrupt td */
	temp=(unsigned long)&qh_cntrl;
	usb_fill_td(&td_int[0],temp | UHCI_PTR_QH,0,0,0,0L);
	temp=(unsigned long)&td_int[0];
	for(n=1; n<8; n++)
		usb_fill_td(&td_int[n],temp,0,0,0,0L);
	for (n = 0; n < 1024; n++) {
	/* link all framelist pointers to one of the interrupts */
		int m, o;
		if ((n&127)==127)
			framelist[n]= swap_32((unsigned long)&td_int[0]);
		else
			for (o = 1, m = 2; m <= 128; o++, m += m)
				if ((n & (m - 1)) == ((m - 1) / 2))
						framelist[n]= swap_32((unsigned long)&td_int[o]);
	}
}

/* check the common skeleton for completed transfers, and update the status
 * of the "connected" device. Called from the IRQ routine.
 */
void usb_check_skel(void)
{
	struct usb_device *dev;
	/* start with the control qh */
	if(qh_cntrl.dev_ptr!=0) /* it's a device assigned check if this caused IRQ */
	{
		dev=(struct usb_device *)qh_cntrl.dev_ptr;
		usb_get_td_status(&tmp_td[0],dev); /* update status */
		if(!(dev->status & USB_ST_NOT_PROC)) { /* is not active anymore, disconnect devices */
			qh_cntrl.dev_ptr=0;
		}
	}
	/* now process the bulk */
	if(qh_bulk.dev_ptr!=0) /* it's a device assigned check if this caused IRQ */
	{
		dev=(struct usb_device *)qh_bulk.dev_ptr;
		usb_get_td_status(&tmp_td[0],dev); /* update status */
		if(!(dev->status & USB_ST_NOT_PROC)) { /* is not active anymore, disconnect devices */
			qh_bulk.dev_ptr=0;
		}
	}
}

/* check the interrupt chain, ubdate the status of the appropriate device,
 * call the appropriate irqhandler and reactivate the TD if the irqhandler
 * returns with 1
 */
void usb_check_int_chain(void)
{
	int i,res;
	unsigned long link,status;
	struct usb_device *dev;
	uhci_td_t *td,*prevtd;

	for(i=0;i<8;i++) {
		prevtd = &td_int[i]; /* the first previous td is the skeleton td */
		link=swap_32(td_int[i].link) & 0xfffffff0; /* next in chain */
		td=(uhci_td_t *)link; /* assign it */
		/* all interrupt TDs are finally linked to the td_int[0].
		 * so we process all until we find the td_int[0].
		 * if int0 chain points to a QH, we're also done
	   */
		while(((i>0) && (link != (unsigned long)&td_int[0])) ||
					((i==0) && !(swap_32(td->link) &  UHCI_PTR_QH)))
		{
			/* check if a device is assigned with this td */
			status=swap_32(td->status);
			if((td->dev_ptr!=0L) && !(status & TD_CTRL_ACTIVE)) {
				/* td is not active and a device is assigned -> call irqhandler */
				dev=(struct usb_device *)td->dev_ptr;
				dev->irq_act_len=((status & 0x7FF)==0x7FF) ? 0 : (status & 0x7FF) + 1; /* transfered length */
				dev->irq_status=usb_uhci_td_stat(status); /* get status */
				res=dev->irq_handle(dev); /* call irqhandler */
				if(res==1) {
					/* reactivate */
					status|=TD_CTRL_ACTIVE;
					td->status=swap_32(status);
					prevtd=td; /* previous td = this td */
				}
				else {
					prevtd->link=td->link; /* link previous td directly to the nex td -> unlinked */
					/* remove device pointer */
					td->dev_ptr=0L;
				}
			} /* if we call the irq handler */
			link=swap_32(td->link) & 0xfffffff0; /* next in chain */
			td=(uhci_td_t *)link; /* assign it */
		} /* process all td in this int chain */
	} /* next interrupt chain */
}


/* usb interrupt service routine.
 */
void handle_usb_interrupt(void)
{
	unsigned short status;

	/*
	 * Read the interrupt status, and write it back to clear the
	 * interrupt cause
	 */

	status = in16r(usb_base_addr + USBSTS);

	if (!status)		/* shared interrupt, not mine */
		return;
	if (status != 1) {
		/* remove host controller halted state */
		if ((status&0x20) && ((in16r(usb_base_addr+USBCMD) && USBCMD_RS)==0)) {
			out16r(usb_base_addr + USBCMD, USBCMD_RS | in16r(usb_base_addr + USBCMD));
		}
	}
	usb_check_int_chain(); /* call interrupt handlers for int tds */
	usb_check_skel(); /* call completion handler for common transfer routines */
	out16r(usb_base_addr+USBSTS,status);
}


/* init uhci
 */
int usb_lowlevel_init(int index, enum usb_init_type init, void **controller)
{
	unsigned char temp;
	int	busdevfunc;

	busdevfunc=pci_find_device(USB_UHCI_VEND_ID,USB_UHCI_DEV_ID,0); /* get PCI Device ID */
	if(busdevfunc==-1) {
		printf("Error USB UHCI (%04X,%04X) not found\n",USB_UHCI_VEND_ID,USB_UHCI_DEV_ID);
		return -1;
	}
	pci_read_config_byte(busdevfunc,PCI_INTERRUPT_LINE,&temp);
	irqvec = temp;
	irq_free_handler(irqvec);
	USB_UHCI_PRINTF("Interrupt Line = %d, is %d\n",irqvec);
	pci_read_config_byte(busdevfunc,PCI_INTERRUPT_PIN,&temp);
	USB_UHCI_PRINTF("Interrupt Pin = %ld\n",temp);
	pci_read_config_dword(busdevfunc,PCI_BASE_ADDRESS_4,&usb_base_addr);
	USB_UHCI_PRINTF("IO Base Address = 0x%lx\n",usb_base_addr);
	usb_base_addr&=0xFFFFFFF0;
	usb_base_addr+=CONFIG_SYS_ISA_IO_BASE_ADDRESS;
	rh.devnum = 0;
	usb_init_skel();
	reset_hc();
	start_hc();
	irq_install_handler(irqvec, (interrupt_handler_t *)handle_usb_interrupt, NULL);
	return 0;
}

/* stop uhci
 */
int usb_lowlevel_stop(int index)
{
	if(irqvec==-1)
		return 1;
	irq_free_handler(irqvec);
	reset_hc();
	irqvec = -1;
	return 0;
}

/*******************************************************************************************
 * Virtual Root Hub
 * Since the uhci does not have a real HUB, we simulate one ;-)
 */
#undef	USB_RH_DEBUG

#ifdef	USB_RH_DEBUG
#define	USB_RH_PRINTF(fmt,args...)	printf (fmt ,##args)
static void usb_display_wValue(unsigned short wValue,unsigned short wIndex);
static void usb_display_Req(unsigned short req);
#else
#define USB_RH_PRINTF(fmt,args...)
static void usb_display_wValue(unsigned short wValue,unsigned short wIndex) {}
static void usb_display_Req(unsigned short req) {}
#endif

static unsigned char root_hub_dev_des[] =
{
	0x12,			/*  __u8  bLength; */
	0x01,			/*  __u8  bDescriptorType; Device */
	0x00,			/*  __u16 bcdUSB; v1.0 */
	0x01,
	0x09,			/*  __u8  bDeviceClass; HUB_CLASSCODE */
	0x00,			/*  __u8  bDeviceSubClass; */
	0x00,			/*  __u8  bDeviceProtocol; */
	0x08,			/*  __u8  bMaxPacketSize0; 8 Bytes */
	0x00,			/*  __u16 idVendor; */
	0x00,
	0x00,			/*  __u16 idProduct; */
	0x00,
	0x00,			/*  __u16 bcdDevice; */
	0x00,
	0x01,			/*  __u8  iManufacturer; */
	0x00,			/*  __u8  iProduct; */
	0x00,			/*  __u8  iSerialNumber; */
	0x01			/*  __u8  bNumConfigurations; */
};


/* Configuration descriptor */
static unsigned char root_hub_config_des[] =
{
	0x09,			/*  __u8  bLength; */
	0x02,			/*  __u8  bDescriptorType; Configuration */
	0x19,			/*  __u16 wTotalLength; */
	0x00,
	0x01,			/*  __u8  bNumInterfaces; */
	0x01,			/*  __u8  bConfigurationValue; */
	0x00,			/*  __u8  iConfiguration; */
	0x40,			/*  __u8  bmAttributes;
				   Bit 7: Bus-powered, 6: Self-powered, 5 Remote-wakwup, 4..0: resvd */
	0x00,			/*  __u8  MaxPower; */

     /* interface */
	0x09,			/*  __u8  if_bLength; */
	0x04,			/*  __u8  if_bDescriptorType; Interface */
	0x00,			/*  __u8  if_bInterfaceNumber; */
	0x00,			/*  __u8  if_bAlternateSetting; */
	0x01,			/*  __u8  if_bNumEndpoints; */
	0x09,			/*  __u8  if_bInterfaceClass; HUB_CLASSCODE */
	0x00,			/*  __u8  if_bInterfaceSubClass; */
	0x00,			/*  __u8  if_bInterfaceProtocol; */
	0x00,			/*  __u8  if_iInterface; */

     /* endpoint */
	0x07,			/*  __u8  ep_bLength; */
	0x05,			/*  __u8  ep_bDescriptorType; Endpoint */
	0x81,			/*  __u8  ep_bEndpointAddress; IN Endpoint 1 */
	0x03,			/*  __u8  ep_bmAttributes; Interrupt */
	0x08,			/*  __u16 ep_wMaxPacketSize; 8 Bytes */
	0x00,
	0xff			/*  __u8  ep_bInterval; 255 ms */
};


static unsigned char root_hub_hub_des[] =
{
	0x09,			/*  __u8  bLength; */
	0x29,			/*  __u8  bDescriptorType; Hub-descriptor */
	0x02,			/*  __u8  bNbrPorts; */
	0x00,			/* __u16  wHubCharacteristics; */
	0x00,
	0x01,			/*  __u8  bPwrOn2pwrGood; 2ms */
	0x00,			/*  __u8  bHubContrCurrent; 0 mA */
	0x00,			/*  __u8  DeviceRemovable; *** 7 Ports max *** */
	0xff			/*  __u8  PortPwrCtrlMask; *** 7 ports max *** */
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
	'U',			/*  __u8  Unicode */
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


/*
 * Root Hub Control Pipe (interrupt Pipes are not supported)
 */


int uhci_submit_rh_msg(struct usb_device *dev, unsigned long pipe, void *buffer,int transfer_len,struct devrequest *cmd)
{
	void *data = buffer;
	int leni = transfer_len;
	int len = 0;
	int status = 0;
	int stat = 0;
	int i;

	unsigned short cstatus;

	unsigned short bmRType_bReq;
	unsigned short wValue;
	unsigned short wIndex;
	unsigned short wLength;

	if (usb_pipeint(pipe)) {
		printf("Root-Hub submit IRQ: NOT implemented\n");
#if 0
		uhci->rh.urb = urb;
		uhci->rh.send = 1;
		uhci->rh.interval = urb->interval;
		rh_init_int_timer (urb);
#endif
		return 0;
	}
	bmRType_bReq = cmd->requesttype | cmd->request << 8;
	wValue = swap_16(cmd->value);
	wIndex = swap_16(cmd->index);
	wLength = swap_16(cmd->length);
	usb_display_Req(bmRType_bReq);
	for (i = 0; i < 8; i++)
		rh.c_p_r[i] = 0;
	USB_RH_PRINTF("Root-Hub: adr: %2x cmd(%1x): %02x%02x %04x %04x %04x\n",
	     dev->devnum, 8, cmd->requesttype,cmd->request, wValue, wIndex, wLength);

	switch (bmRType_bReq) {
		/* Request Destination:
		   without flags: Device,
		   RH_INTERFACE: interface,
		   RH_ENDPOINT: endpoint,
		   RH_CLASS means HUB here,
		   RH_OTHER | RH_CLASS  almost ever means HUB_PORT here
		 */

	case RH_GET_STATUS:
		*(unsigned short *) data = swap_16(1);
		len=2;
		break;
	case RH_GET_STATUS | RH_INTERFACE:
		*(unsigned short *) data = swap_16(0);
		len=2;
		break;
	case RH_GET_STATUS | RH_ENDPOINT:
		*(unsigned short *) data = swap_16(0);
		len=2;
		break;
	case RH_GET_STATUS | RH_CLASS:
		*(unsigned long *) data = swap_32(0);
		len=4;
		break;	/* hub power ** */
	case RH_GET_STATUS | RH_OTHER | RH_CLASS:

		status = in16r(usb_base_addr + USBPORTSC1 + 2 * (wIndex - 1));
		cstatus = ((status & USBPORTSC_CSC) >> (1 - 0)) |
			((status & USBPORTSC_PEC) >> (3 - 1)) |
			(rh.c_p_r[wIndex - 1] << (0 + 4));
		status = (status & USBPORTSC_CCS) |
			((status & USBPORTSC_PE) >> (2 - 1)) |
			((status & USBPORTSC_SUSP) >> (12 - 2)) |
			((status & USBPORTSC_PR) >> (9 - 4)) |
			(1 << 8) |	/* power on ** */
			((status & USBPORTSC_LSDA) << (-8 + 9));

		*(unsigned short *) data = swap_16(status);
		*(unsigned short *) (data + 2) = swap_16(cstatus);
		len=4;
		break;
	case RH_CLEAR_FEATURE | RH_ENDPOINT:
		switch (wValue) {
		case (RH_ENDPOINT_STALL):
			len=0;
			break;
		}
		break;

	case RH_CLEAR_FEATURE | RH_CLASS:
		switch (wValue) {
		case (RH_C_HUB_OVER_CURRENT):
			len=0;	/* hub power over current ** */
			break;
		}
		break;

	case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
		usb_display_wValue(wValue,wIndex);
		switch (wValue) {
		case (RH_PORT_ENABLE):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) & ~USBPORTSC_PE;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		case (RH_PORT_SUSPEND):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) & ~USBPORTSC_SUSP;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		case (RH_PORT_POWER):
			len=0;	/* port power ** */
			break;
		case (RH_C_PORT_CONNECTION):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) | USBPORTSC_CSC;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		case (RH_C_PORT_ENABLE):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) | USBPORTSC_PEC;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		case (RH_C_PORT_SUSPEND):
/*** WR_RH_PORTSTAT(RH_PS_PSSC); */
			len=0;
			break;
		case (RH_C_PORT_OVER_CURRENT):
			len=0;
			break;
		case (RH_C_PORT_RESET):
			rh.c_p_r[wIndex - 1] = 0;
			len=0;
			break;
		}
		break;
	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		usb_display_wValue(wValue,wIndex);
		switch (wValue) {
		case (RH_PORT_SUSPEND):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) | USBPORTSC_SUSP;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		case (RH_PORT_RESET):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) | USBPORTSC_PR;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			mdelay(10);
			status = (status & 0xfff5) & ~USBPORTSC_PR;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			udelay(10);
			status = (status & 0xfff5) | USBPORTSC_PE;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			mdelay(10);
			status = (status & 0xfff5) | 0xa;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		case (RH_PORT_POWER):
			len=0;	/* port power ** */
			break;
		case (RH_PORT_ENABLE):
			status = in16r(usb_base_addr+USBPORTSC1+2*(wIndex-1));
			status = (status & 0xfff5) | USBPORTSC_PE;
			out16r(usb_base_addr+USBPORTSC1+2*(wIndex-1),status);
			len=0;
			break;
		}
		break;

	case RH_SET_ADDRESS:
		rh.devnum = wValue;
		len=0;
		break;
	case RH_GET_DESCRIPTOR:
		switch ((wValue & 0xff00) >> 8) {
		case (0x01):	/* device descriptor */
			i=sizeof(root_hub_config_des);
			status=i > wLength ? wLength : i;
			len = leni > status ? status : leni;
			memcpy (data, root_hub_dev_des, len);
			break;
		case (0x02):	/* configuration descriptor */
			i=sizeof(root_hub_config_des);
			status=i > wLength ? wLength : i;
			len = leni > status ? status : leni;
			memcpy (data, root_hub_config_des, len);
			break;
		case (0x03):	/*string descriptors */
			if(wValue==0x0300) {
				i=sizeof(root_hub_str_index0);
				status = i > wLength ? wLength : i;
				len = leni > status ? status : leni;
				memcpy (data, root_hub_str_index0, len);
				break;
			}
			if(wValue==0x0301) {
				i=sizeof(root_hub_str_index1);
				status = i > wLength ? wLength : i;
				len = leni > status ? status : leni;
				memcpy (data, root_hub_str_index1, len);
				break;
			}
			stat = USB_ST_STALLED;
		}
		break;

	case RH_GET_DESCRIPTOR | RH_CLASS:
		root_hub_hub_des[2] = 2;
		i=sizeof(root_hub_hub_des);
		status= i > wLength ? wLength : i;
		len = leni > status ? status : leni;
		memcpy (data, root_hub_hub_des, len);
		break;
	case RH_GET_CONFIGURATION:
		*(unsigned char *) data = 0x01;
		len = 1;
		break;
	case RH_SET_CONFIGURATION:
		len=0;
		break;
	default:
		stat = USB_ST_STALLED;
	}
	USB_RH_PRINTF("Root-Hub stat %lx port1: %x port2: %x\n\n",stat,
	     in16r(usb_base_addr + USBPORTSC1), in16r(usb_base_addr + USBPORTSC2));
	dev->act_len=len;
	dev->status=stat;
	return stat;

}

/********************************************************************************
 * Some Debug Routines
 */

#ifdef	USB_RH_DEBUG

static void usb_display_Req(unsigned short req)
{
	USB_RH_PRINTF("- Root-Hub Request: ");
	switch (req) {
	case RH_GET_STATUS:
		USB_RH_PRINTF("Get Status ");
		break;
	case RH_GET_STATUS | RH_INTERFACE:
		USB_RH_PRINTF("Get Status Interface ");
		break;
	case RH_GET_STATUS | RH_ENDPOINT:
		USB_RH_PRINTF("Get Status Endpoint ");
		break;
	case RH_GET_STATUS | RH_CLASS:
		USB_RH_PRINTF("Get Status Class");
		break;	/* hub power ** */
	case RH_GET_STATUS | RH_OTHER | RH_CLASS:
		USB_RH_PRINTF("Get Status Class Others");
		break;
	case RH_CLEAR_FEATURE | RH_ENDPOINT:
		USB_RH_PRINTF("Clear Feature Endpoint ");
		break;
	case RH_CLEAR_FEATURE | RH_CLASS:
		USB_RH_PRINTF("Clear Feature Class ");
		break;
	case RH_CLEAR_FEATURE | RH_OTHER | RH_CLASS:
		USB_RH_PRINTF("Clear Feature Other Class ");
		break;
	case RH_SET_FEATURE | RH_OTHER | RH_CLASS:
		USB_RH_PRINTF("Set Feature Other Class ");
		break;
	case RH_SET_ADDRESS:
		USB_RH_PRINTF("Set Address ");
		break;
	case RH_GET_DESCRIPTOR:
		USB_RH_PRINTF("Get Descriptor ");
		break;
	case RH_GET_DESCRIPTOR | RH_CLASS:
		USB_RH_PRINTF("Get Descriptor Class ");
		break;
	case RH_GET_CONFIGURATION:
		USB_RH_PRINTF("Get Configuration ");
		break;
	case RH_SET_CONFIGURATION:
		USB_RH_PRINTF("Get Configuration ");
		break;
	default:
		USB_RH_PRINTF("****UNKNOWN**** 0x%04X ",req);
	}
	USB_RH_PRINTF("\n");

}

static void usb_display_wValue(unsigned short wValue,unsigned short wIndex)
{
	switch (wValue) {
		case (RH_PORT_ENABLE):
			USB_RH_PRINTF("Root-Hub: Enable Port %d\n",wIndex);
			break;
		case (RH_PORT_SUSPEND):
			USB_RH_PRINTF("Root-Hub: Suspend Port %d\n",wIndex);
			break;
		case (RH_PORT_POWER):
			USB_RH_PRINTF("Root-Hub: Port Power %d\n",wIndex);
			break;
		case (RH_C_PORT_CONNECTION):
			USB_RH_PRINTF("Root-Hub: C Port Connection Port %d\n",wIndex);
			break;
		case (RH_C_PORT_ENABLE):
			USB_RH_PRINTF("Root-Hub: C Port Enable Port %d\n",wIndex);
			break;
		case (RH_C_PORT_SUSPEND):
			USB_RH_PRINTF("Root-Hub: C Port Suspend Port %d\n",wIndex);
			break;
		case (RH_C_PORT_OVER_CURRENT):
			USB_RH_PRINTF("Root-Hub: C Port Over Current Port %d\n",wIndex);
			break;
		case (RH_C_PORT_RESET):
			USB_RH_PRINTF("Root-Hub: C Port reset Port %d\n",wIndex);
			break;
		default:
			USB_RH_PRINTF("Root-Hub: unknown %x %x\n",wValue,wIndex);
			break;
	}
}

#endif


#ifdef	USB_UHCI_DEBUG

static int usb_display_td(uhci_td_t *td)
{
	unsigned long tmp;
	int valid;

	printf("TD at %p:\n",td);

	tmp=swap_32(td->link);
	printf("Link points to 0x%08lX, %s first, %s, %s\n",tmp&0xfffffff0,
		((tmp & 0x4)==0x4) ? "Depth" : "Breath",
		((tmp & 0x2)==0x2) ? "QH" : "TD",
		((tmp & 0x1)==0x1) ? "invalid" : "valid");
	valid=((tmp & 0x1)==0x0);
	tmp=swap_32(td->status);
	printf("     %s %ld Errors %s %s %s \n     %s %s %s %s %s %s\n     Len 0x%lX\n",
		(((tmp>>29)&0x1)==0x1) ? "SPD Enable" : "SPD Disable",
		((tmp>>28)&0x3),
		(((tmp>>26)&0x1)==0x1) ? "Low Speed" : "Full Speed",
		(((tmp>>25)&0x1)==0x1) ? "ISO " : "",
		(((tmp>>24)&0x1)==0x1) ? "IOC " : "",
		(((tmp>>23)&0x1)==0x1) ? "Active " : "Inactive ",
		(((tmp>>22)&0x1)==0x1) ? "Stalled" : "",
		(((tmp>>21)&0x1)==0x1) ? "Data Buffer Error" : "",
		(((tmp>>20)&0x1)==0x1) ? "Babble" : "",
		(((tmp>>19)&0x1)==0x1) ? "NAK" : "",
		(((tmp>>18)&0x1)==0x1) ? "Bitstuff Error" : "",
		(tmp&0x7ff));
	tmp=swap_32(td->info);
	printf("     MaxLen 0x%lX\n",((tmp>>21)&0x7FF));
	printf("     %s Endpoint 0x%lX Dev Addr 0x%lX PID 0x%lX\n",((tmp>>19)&0x1)==0x1 ? "TOGGLE" : "",
		((tmp>>15)&0xF),((tmp>>8)&0x7F),tmp&0xFF);
	tmp=swap_32(td->buffer);
	printf("     Buffer 0x%08lX\n",tmp);
	printf("     DEV %08lX\n",td->dev_ptr);
	return valid;
}


void usb_show_td(int max)
{
	int i;
	if(max>0) {
		for(i=0;i<max;i++) {
			usb_display_td(&tmp_td[i]);
		}
	}
	else {
		i=0;
		do {
			printf("tmp_td[%d]\n",i);
		}while(usb_display_td(&tmp_td[i++]));
	}
}


#endif
#endif /* CONFIG_USB_UHCI */

/* EOF */
