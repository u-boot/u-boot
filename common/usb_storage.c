/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * Most of this source has been derived from the Linux USB
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

/* Note:
 * Currently only the CBI transport protocoll has been implemented, and it
 * is only tested with a TEAC USB Floppy. Other Massstorages with CBI or CB
 * transport protocoll may work as well.
 */



#include <common.h>
#include <command.h>
#include <asm/processor.h>


#if (CONFIG_COMMANDS & CFG_CMD_USB)
#include <usb.h>

#ifdef CONFIG_USB_STORAGE

#undef	USB_STOR_DEBUG

#ifdef	USB_STOR_DEBUG
#define	USB_STOR_PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define USB_STOR_PRINTF(fmt,args...)
#endif

#include <scsi.h>
/* direction table -- this indicates the direction of the data
 * transfer for each command code -- a 1 indicates input
 */
unsigned char us_direction[256/8] = {
	0x28, 0x81, 0x14, 0x14, 0x20, 0x01, 0x90, 0x77,
	0x0C, 0x20, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#define US_DIRECTION(x) ((us_direction[x>>3] >> (x & 7)) & 1)

static unsigned char usb_stor_buf[512];
static ccb usb_ccb;

/*
 * CBI style
 */

#define US_CBI_ADSC		0


#define USB_MAX_STOR_DEV 5
static int usb_max_devs; /* number of highest available usb device */

static block_dev_desc_t usb_dev_desc[USB_MAX_STOR_DEV];

struct us_data;
typedef int (*trans_cmnd)(ccb*, struct us_data*);
typedef int (*trans_reset)(struct us_data*);

struct us_data {
	struct usb_device	*pusb_dev;       /* this usb_device */
	unsigned int		flags;		 /* from filter initially */
	unsigned char		ifnum;		 /* interface number */
	unsigned char		ep_in;		 /* in endpoint */
	unsigned char		ep_out;		 /* out ....... */
	unsigned char		ep_int;		 /* interrupt . */
	unsigned char		subclass;	 /* as in overview */
	unsigned char		protocol;	 /* .............. */
	unsigned char		attention_done;  /* force attn on first cmd */
	unsigned short	ip_data;	 /* interrupt data */
	int							action;		 /* what to do */
	int							ip_wanted; /* needed */
	int							*irq_handle;	 /* for USB int requests */
	unsigned int		irqpipe;	 /* pipe for release_irq */
	unsigned char		irqmaxp;	/* max packed for irq Pipe */
	unsigned char   irqinterval; /* Intervall for IRQ Pipe */
	ccb							*srb;		 /* current srb */
	trans_reset			transport_reset; /* reset routine */
	trans_cmnd			transport; /* transport routine */
};

static struct us_data usb_stor[USB_MAX_STOR_DEV];



#define USB_STOR_TRANSPORT_GOOD    0
#define USB_STOR_TRANSPORT_FAILED -1
#define USB_STOR_TRANSPORT_ERROR  -2






int usb_stor_get_info(struct usb_device *dev, struct us_data *us, block_dev_desc_t *dev_desc);
int usb_storage_probe(struct usb_device *dev, unsigned int ifnum,struct us_data *ss);
unsigned long usb_stor_read(int device, unsigned long blknr, unsigned long blkcnt, unsigned long *buffer);
struct usb_device * usb_get_dev_index(int index);
void uhci_show_temp_int_td(void);

block_dev_desc_t *usb_stor_get_dev(int index)
{
	return &usb_dev_desc[index];
}


void usb_show_progress(void)
{
	printf(".");
}

/*********************************************************************************
 * (re)-scan the usb and reports device info
 * to the user if mode = 1
 * returns current device or -1 if no
 */
int usb_stor_scan(int mode)
{
	unsigned char i;
	struct usb_device *dev;

	if(mode==1) {
		printf("scanning bus for storage devices...\n");
	}
	usb_disable_asynch(1); /* asynch transfer not allowed */

	for(i=0;i<USB_MAX_STOR_DEV;i++) {
		memset(&usb_dev_desc[i],0,sizeof(block_dev_desc_t));
		usb_dev_desc[i].target=0xff;
		usb_dev_desc[i].if_type=IF_TYPE_USB;
		usb_dev_desc[i].dev=i;
		usb_dev_desc[i].part_type=PART_TYPE_UNKNOWN;
		usb_dev_desc[i].block_read=usb_stor_read;
	}
	usb_max_devs=0;
	for(i=0;i<USB_MAX_DEVICE;i++) {
		dev=usb_get_dev_index(i); /* get device */
		USB_STOR_PRINTF("i=%d\n",i);
		if(dev==NULL) {
			break; /* no more devices avaiable */
		}
		if(usb_storage_probe(dev,0,&usb_stor[usb_max_devs])) { /* ok, it is a storage devices */
			/* get info and fill it in */

			if(usb_stor_get_info(dev, &usb_stor[usb_max_devs], &usb_dev_desc[usb_max_devs])) {
				if(mode==1) {
					printf ("  Device %d: ", usb_max_devs);
					dev_print(&usb_dev_desc[usb_max_devs]);
				} /* if mode */
				usb_max_devs++;
			} /* if get info ok */
		} /* if storage device */
		if(usb_max_devs==USB_MAX_STOR_DEV) {
			printf("max USB Storage Device reached: %d stopping\n",usb_max_devs);
			break;
		}
	} /* for */
	usb_disable_asynch(0); /* asynch transfer allowed */
	if(usb_max_devs>0)
		return 0;
	else
		return-1;
}

static int usb_stor_irq(struct usb_device *dev)
{
	struct us_data *us;
	us=(struct us_data *)dev->privptr;

	if(us->ip_wanted) {
		us->ip_wanted=0;
	}
	return 0;
}


#ifdef	USB_STOR_DEBUG

static void usb_show_srb(ccb * pccb)
{
	int i;
	printf("SRB: len %d datalen 0x%lX\n ",pccb->cmdlen,pccb->datalen);
	for(i=0;i<12;i++) {
		printf("%02X ",pccb->cmd[i]);
	}
	printf("\n");
}

static void display_int_status(unsigned long tmp)
{
	printf("Status: %s %s %s %s %s %s %s\n",
		(tmp & USB_ST_ACTIVE) ? "Active" : "",
		(tmp & USB_ST_STALLED) ? "Stalled" : "",
		(tmp & USB_ST_BUF_ERR) ? "Buffer Error" : "",
		(tmp & USB_ST_BABBLE_DET) ? "Babble Det" : "",
		(tmp & USB_ST_NAK_REC) ? "NAKed" : "",
		(tmp & USB_ST_CRC_ERR) ? "CRC Error" : "",
		(tmp & USB_ST_BIT_ERR) ? "Bitstuff Error" : "");
}
#endif
/***********************************************************************
 * Data transfer routines
 ***********************************************************************/

static int us_one_transfer(struct us_data *us, int pipe, char *buf, int length)
{
	int max_size;
	int this_xfer;
	int result;
	int partial;
	int maxtry;
	int stat;

	/* determine the maximum packet size for these transfers */
	max_size = usb_maxpacket(us->pusb_dev, pipe) * 16;

	/* while we have data left to transfer */
	while (length) {

		/* calculate how long this will be -- maximum or a remainder */
		this_xfer = length > max_size ? max_size : length;
		length -= this_xfer;

		/* setup the retry counter */
		maxtry = 10;

		/* set up the transfer loop */
		do {
			/* transfer the data */
			USB_STOR_PRINTF("Bulk xfer 0x%x(%d) try #%d\n",
				  (unsigned int)buf, this_xfer, 11 - maxtry);
			result = usb_bulk_msg(us->pusb_dev, pipe, buf,
					      this_xfer, &partial, USB_CNTL_TIMEOUT*5);
			USB_STOR_PRINTF("bulk_msg returned %d xferred %d/%d\n",
				  result, partial, this_xfer);
			if(us->pusb_dev->status!=0) {
				/* if we stall, we need to clear it before we go on */
#ifdef USB_STOR_DEBUG
				display_int_status(us->pusb_dev->status);
#endif
				if (us->pusb_dev->status & USB_ST_STALLED) {
					USB_STOR_PRINTF("stalled ->clearing endpoint halt for pipe 0x%x\n", pipe);
					stat = us->pusb_dev->status;
					usb_clear_halt(us->pusb_dev, pipe);
					us->pusb_dev->status=stat;
					if(this_xfer == partial) {
						USB_STOR_PRINTF("bulk transferred with error %X, but data ok\n",us->pusb_dev->status);
						return 0;
					}
					else
						return result;
				}
				if (us->pusb_dev->status & USB_ST_NAK_REC) {
					USB_STOR_PRINTF("Device NAKed bulk_msg\n");
					return result;
				}
				if(this_xfer == partial) {
					USB_STOR_PRINTF("bulk transferred with error %d, but data ok\n",us->pusb_dev->status);
					return 0;
				}
				/* if our try counter reaches 0, bail out */
				USB_STOR_PRINTF("bulk transferred with error %d, data %d\n",us->pusb_dev->status,partial);
				if (!maxtry--)
						return result;
			}
			/* update to show what data was transferred */
			this_xfer -= partial;
			buf += partial;
			/* continue until this transfer is done */
		} while ( this_xfer );
	}

	/* if we get here, we're done and successful */
	return 0;
}

/* FIXME: this reset function doesn't really reset the port, and it
 * should. Actually it should probably do what it's doing here, and
 * reset the port physically
 */
static int usb_stor_CB_reset(struct us_data *us)
{
	unsigned char cmd[12];
	int result;

	USB_STOR_PRINTF("CB_reset\n");
	memset(cmd, 0xFF, sizeof(cmd));
	cmd[0] = SCSI_SEND_DIAG;
	cmd[1] = 4;
	result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev,0),
				 US_CBI_ADSC, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
				 0, us->ifnum, cmd, sizeof(cmd), USB_CNTL_TIMEOUT*5);

	/* long wait for reset */
	wait_ms(1500);
	USB_STOR_PRINTF("CB_reset result %d: status %X clearing endpoint halt\n",result,us->pusb_dev->status);
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_in));
	usb_clear_halt(us->pusb_dev, usb_rcvbulkpipe(us->pusb_dev, us->ep_out));

	USB_STOR_PRINTF("CB_reset done\n");
	return 0;
}

/* FIXME: we also need a CBI_command which sets up the completion
 * interrupt, and waits for it
 */
int usb_stor_CB_comdat(ccb *srb, struct us_data *us)
{
	int result;
	int dir_in,retry;
	unsigned int pipe;
	unsigned long status;

	retry=5;
		dir_in=US_DIRECTION(srb->cmd[0]);

		if(dir_in)
			pipe=usb_rcvbulkpipe(us->pusb_dev, us->ep_in);
		else
			pipe=usb_sndbulkpipe(us->pusb_dev, us->ep_out);
	while(retry--) {
		USB_STOR_PRINTF("CBI gets a command: Try %d\n",5-retry);
#ifdef USB_STOR_DEBUG
		usb_show_srb(srb);
#endif
		/* let's send the command via the control pipe */
		result = usb_control_msg(us->pusb_dev, usb_sndctrlpipe(us->pusb_dev,0),
					 US_CBI_ADSC, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
					 0, us->ifnum,
					 srb->cmd, srb->cmdlen, USB_CNTL_TIMEOUT*5);
		USB_STOR_PRINTF("CB_transport: control msg returned %d, status %X\n",result,us->pusb_dev->status);
		/* check the return code for the command */
		if (result < 0) {
			if(us->pusb_dev->status & USB_ST_STALLED) {
				status=us->pusb_dev->status;
				USB_STOR_PRINTF(" stall during command found, clear pipe\n");
				usb_clear_halt(us->pusb_dev,  usb_sndctrlpipe(us->pusb_dev,0));
				us->pusb_dev->status=status;
			}
			USB_STOR_PRINTF(" error during command %02X Stat = %X\n",srb->cmd[0],us->pusb_dev->status);
			return result;
		}
		/* transfer the data payload for this command, if one exists*/

		USB_STOR_PRINTF("CB_transport: control msg returned %d, direction is %s to go 0x%lx\n",result,dir_in ? "IN" : "OUT",srb->datalen);
		if (srb->datalen) {
			result = us_one_transfer(us, pipe, srb->pdata,srb->datalen);
			USB_STOR_PRINTF("CBI attempted to transfer data, result is %d status %lX, len %d\n", result,us->pusb_dev->status,us->pusb_dev->act_len);
			if(!(us->pusb_dev->status & USB_ST_NAK_REC))
				break;
		} /* if (srb->datalen) */
		else
			break;
	}
	/* return result */

	return result;
}


int usb_stor_CBI_get_status(ccb *srb, struct us_data *us)
{
	int timeout;

	us->ip_wanted=1;
	submit_int_msg(us->pusb_dev,us->irqpipe,
			(void *)&us->ip_data,us->irqmaxp ,us->irqinterval);
  timeout=1000;
  while(timeout--) {
  	if((volatile int *)us->ip_wanted==0)
			break;
		wait_ms(10);
	}
	if (us->ip_wanted) {
		printf("       Did not get interrupt on CBI\n");
		us->ip_wanted = 0;
		return USB_STOR_TRANSPORT_ERROR;
	}
	USB_STOR_PRINTF("Got interrupt data 0x%x, transfered %d status 0x%lX\n", us->ip_data,us->pusb_dev->irq_act_len,us->pusb_dev->irq_status);
	/* UFI gives us ASC and ASCQ, like a request sense */
	if (us->subclass == US_SC_UFI) {
		if (srb->cmd[0] == SCSI_REQ_SENSE ||
		    srb->cmd[0] == SCSI_INQUIRY)
			return USB_STOR_TRANSPORT_GOOD; /* Good */
		else
			if (us->ip_data)
				return USB_STOR_TRANSPORT_FAILED;
			else
				return USB_STOR_TRANSPORT_GOOD;
	}
	/* otherwise, we interpret the data normally */
	switch (us->ip_data) {
		case 0x0001:
			return USB_STOR_TRANSPORT_GOOD;
		case 0x0002:
			return USB_STOR_TRANSPORT_FAILED;
		default:
			return USB_STOR_TRANSPORT_ERROR;
	} /* switch */
	return USB_STOR_TRANSPORT_ERROR;
}

#define USB_TRANSPORT_UNKNOWN_RETRY 5
#define USB_TRANSPORT_NOT_READY_RETRY 10

int usb_stor_CB_transport(ccb *srb, struct us_data *us)
{
	int result,status;
	ccb *psrb;
	ccb reqsrb;
	int retry,notready;

	psrb=&reqsrb;
	status=USB_STOR_TRANSPORT_GOOD;
	retry=0;
	notready=0;
	/* issue the command */
do_retry:
	result=usb_stor_CB_comdat(srb,us);
	USB_STOR_PRINTF("command / Data returned %d, status %X\n",result,us->pusb_dev->status);
	/* if this is an CBI Protocol, get IRQ */
	if(us->protocol==US_PR_CBI) {
		status=usb_stor_CBI_get_status(srb,us);
		/* if the status is error, report it */
		if(status==USB_STOR_TRANSPORT_ERROR) {
			USB_STOR_PRINTF(" USB CBI Command Error\n");
			return status;
		}
		srb->sense_buf[12]=(unsigned char)(us->ip_data>>8);
		srb->sense_buf[13]=(unsigned char)(us->ip_data&0xff);
		if(!us->ip_data) {
		/* if the status is good, report it */
			if(status==USB_STOR_TRANSPORT_GOOD) {
				USB_STOR_PRINTF(" USB CBI Command Good\n");
				return status;
			}
		}
	}
	/* do we have to issue an auto request? */
	/* HERE we have to check the result */
	if((result<0) && !(us->pusb_dev->status & USB_ST_STALLED)) {
		USB_STOR_PRINTF("ERROR %X\n",us->pusb_dev->status);
		us->transport_reset(us);
		return USB_STOR_TRANSPORT_ERROR;
	}
	if((us->protocol==US_PR_CBI) &&
			((srb->cmd[0]==SCSI_REQ_SENSE) ||
		 	(srb->cmd[0]==SCSI_INQUIRY))) { /* do not issue an autorequest after request sense */
		USB_STOR_PRINTF("No auto request and good\n");
		return USB_STOR_TRANSPORT_GOOD;
	}
	/* issue an request_sense */
	memset(&psrb->cmd[0],0,12);
	psrb->cmd[0]=SCSI_REQ_SENSE;
	psrb->cmd[1]=srb->lun<<5;
	psrb->cmd[4]=18;
	psrb->datalen=18;
	psrb->pdata=&srb->sense_buf[0];
	psrb->cmdlen=12;
	/* issue the command */
	result=usb_stor_CB_comdat(psrb,us);
	USB_STOR_PRINTF("auto request returned %d\n",result);
	/* if this is an CBI Protocol, get IRQ */
	if(us->protocol==US_PR_CBI) {
	 	status=usb_stor_CBI_get_status(psrb,us);
	}
	if((result<0)&&!(us->pusb_dev->status & USB_ST_STALLED)) {
		USB_STOR_PRINTF(" AUTO REQUEST ERROR %d\n",us->pusb_dev->status);
		return USB_STOR_TRANSPORT_ERROR;
	}
	USB_STOR_PRINTF("autorequest returned 0x%02X 0x%02X 0x%02X 0x%02X\n",srb->sense_buf[0],srb->sense_buf[2],srb->sense_buf[12],srb->sense_buf[13]);
	/* Check the auto request result */
	if((srb->sense_buf[2]==0) &&
		 (srb->sense_buf[12]==0) &&
		 (srb->sense_buf[13]==0)) /* ok, no sense */
		return USB_STOR_TRANSPORT_GOOD;
	/* Check the auto request result */
	switch(srb->sense_buf[2]) {
		case 0x01: /* Recovered Error */
			return USB_STOR_TRANSPORT_GOOD;
		 	break;
		case 0x02: /* Not Ready */
			if(notready++ > USB_TRANSPORT_NOT_READY_RETRY) {
				printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X 0x%02X (NOT READY)\n",
					srb->cmd[0],srb->sense_buf[0],srb->sense_buf[2],srb->sense_buf[12],srb->sense_buf[13]);
				return USB_STOR_TRANSPORT_FAILED;
			}
			else {
				wait_ms(100);
				goto do_retry;
			}
			break;
		default:
			if(retry++ > USB_TRANSPORT_UNKNOWN_RETRY) {
				printf("cmd 0x%02X returned 0x%02X 0x%02X 0x%02X 0x%02X\n",
					srb->cmd[0],srb->sense_buf[0],srb->sense_buf[2],srb->sense_buf[12],srb->sense_buf[13]);
				return USB_STOR_TRANSPORT_FAILED;
			}
			else
				goto do_retry;
			break;
	}
	return USB_STOR_TRANSPORT_FAILED;
}



static int usb_inquiry(ccb *srb,struct us_data *ss)
{
	int retry,i;
	retry=3;
	do {
		memset(&srb->cmd[0],0,12);
		srb->cmd[0]=SCSI_INQUIRY;
		srb->cmd[1]=srb->lun<<5;
		srb->cmd[4]=36;
		srb->datalen=36;
		srb->cmdlen=12;
		i=ss->transport(srb,ss);
		USB_STOR_PRINTF("inquiry returns %d\n",i);
		if(i==0)
			break;
	}while(retry--);
	if(!retry) {
		printf("error in inquiry\n");
		return -1;
	}
	return 0;
}

static int usb_request_sense(ccb *srb,struct us_data *ss)
{
	char *ptr;
	return 0;
	ptr=srb->pdata;
	memset(&srb->cmd[0],0,12);
	srb->cmd[0]=SCSI_REQ_SENSE;
	srb->cmd[1]=srb->lun<<5;
	srb->cmd[4]=18;
	srb->datalen=18;
	srb->pdata=&srb->sense_buf[0];
	srb->cmdlen=12;
	ss->transport(srb,ss);
	USB_STOR_PRINTF("Request Sense returned %02X %02X %02X\n",srb->sense_buf[2],srb->sense_buf[12],srb->sense_buf[13]);
	srb->pdata=ptr;
	return 0;
}

static int usb_test_unit_ready(ccb *srb,struct us_data *ss)
{
	int retries=10;
	do {
		memset(&srb->cmd[0],0,12);
		srb->cmd[0]=SCSI_TST_U_RDY;
		srb->cmd[1]=srb->lun<<5;
		srb->datalen=0;
		srb->cmdlen=12;
		if(ss->transport(srb,ss)==USB_STOR_TRANSPORT_GOOD)
		{
			return 0;
		}
	} while(retries--);
	return -1;
}

static int usb_read_capacity(ccb *srb,struct us_data *ss)
{
	int retry;
	retry=2; /* retries */
	do {
		memset(&srb->cmd[0],0,12);
		srb->cmd[0]=SCSI_RD_CAPAC;
		srb->cmd[1]=srb->lun<<5;
		srb->datalen=8;
		srb->cmdlen=12;
		if(ss->transport(srb,ss)==USB_STOR_TRANSPORT_GOOD) {
			return 0;
		}
	}while(retry--);
	return -1;
}

static int usb_read_10(ccb *srb,struct us_data *ss, unsigned long start, unsigned short blocks)
{
	memset(&srb->cmd[0],0,12);
	srb->cmd[0]=SCSI_READ10;
	srb->cmd[1]=srb->lun<<5;
	srb->cmd[2]=((unsigned char) (start>>24))&0xff;
	srb->cmd[3]=((unsigned char) (start>>16))&0xff;
	srb->cmd[4]=((unsigned char) (start>>8))&0xff;
	srb->cmd[5]=((unsigned char) (start))&0xff;
	srb->cmd[7]=((unsigned char) (blocks>>8))&0xff;
	srb->cmd[8]=(unsigned char) blocks & 0xff;
	srb->cmdlen=12;
	USB_STOR_PRINTF("read10: start %lx blocks %x\n",start,blocks);
	return ss->transport(srb,ss);
}


#define USB_MAX_READ_BLK 20

unsigned long usb_stor_read(int device, unsigned long blknr, unsigned long blkcnt, unsigned long *buffer)
{
	unsigned long start,blks, buf_addr;
	unsigned short smallblks;
	struct usb_device *dev;
	int retry,i;
	ccb *srb=&usb_ccb;
	device&=0xff;
	/* Setup  device
	 */
	USB_STOR_PRINTF("\nusb_read: dev %d \n",device);
	dev=NULL;
	for(i=0;i<USB_MAX_DEVICE;i++) {
		dev=usb_get_dev_index(i);
		if(dev==NULL) {
			return 0;
		}
		if(dev->devnum==usb_dev_desc[device].target)
			break;
	}

	usb_disable_asynch(1); /* asynch transfer not allowed */
	srb->lun=usb_dev_desc[device].lun;
	buf_addr=(unsigned long)buffer;
	start=blknr;
	blks=blkcnt;
	if(usb_test_unit_ready(srb,(struct us_data *)dev->privptr)) {
		printf("Device NOT ready\n   Request Sense returned %02X %02X %02X\n",
			srb->sense_buf[2],srb->sense_buf[12],srb->sense_buf[13]);
		return 0;
	}
	USB_STOR_PRINTF("\nusb_read: dev %d startblk %lx, blccnt %lx buffer %lx\n",device,start,blks, buf_addr);
	do {
		retry=2;
		srb->pdata=(unsigned char *)buf_addr;
		if(blks>USB_MAX_READ_BLK) {
			smallblks=USB_MAX_READ_BLK;
		}
		else {
			smallblks=(unsigned short) blks;
		}
retry_it:
		if(smallblks==USB_MAX_READ_BLK)
			usb_show_progress();
		srb->datalen=usb_dev_desc[device].blksz * smallblks;
		srb->pdata=(unsigned char *)buf_addr;
		if(usb_read_10(srb,(struct us_data *)dev->privptr, start, smallblks)) {
			USB_STOR_PRINTF("Read ERROR\n");
			usb_request_sense(srb,(struct us_data *)dev->privptr);
			if(retry--)
				goto retry_it;
			blkcnt-=blks;
			break;
		}
		start+=smallblks;
		blks-=smallblks;
		buf_addr+=srb->datalen;
	} while(blks!=0);
	USB_STOR_PRINTF("usb_read: end startblk %lx, blccnt %x buffer %lx\n",start,smallblks,buf_addr);
	usb_disable_asynch(0); /* asynch transfer allowed */
	if(blkcnt>=USB_MAX_READ_BLK)
		printf("\n");
	return(blkcnt);
}


/* Probe to see if a new device is actually a Storage device */
int usb_storage_probe(struct usb_device *dev, unsigned int ifnum,struct us_data *ss)
{
	struct usb_interface_descriptor *iface;
	int i;
	unsigned int flags = 0;

	int protocol = 0;
	int subclass = 0;


	memset(ss, 0, sizeof(struct us_data));

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];

#if 0
	/* this is the place to patch some storage devices */
	USB_STOR_PRINTF("iVendor %X iProduct %X\n",dev->descriptor.idVendor,dev->descriptor.idProduct);
	if ((dev->descriptor.idVendor) == 0x066b && (dev->descriptor.idProduct) == 0x0103) {
		USB_STOR_PRINTF("patched for E-USB\n");
		protocol = US_PR_CB;
		subclass = US_SC_UFI;	    /* an assumption */
	}
#endif

	if (dev->descriptor.bDeviceClass != 0 ||
			iface->bInterfaceClass != USB_CLASS_MASS_STORAGE ||
			iface->bInterfaceSubClass < US_SC_MIN ||
			iface->bInterfaceSubClass > US_SC_MAX) {
		/* if it's not a mass storage, we go no further */
		return 0;
	}

	/* At this point, we know we've got a live one */
	USB_STOR_PRINTF("\n\nUSB Mass Storage device detected\n");

	/* Initialize the us_data structure with some useful info */
	ss->flags = flags;
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->attention_done = 0;

	/* If the device has subclass and protocol, then use that.  Otherwise,
	 * take data from the specific interface.
	 */
	if (subclass) {
		ss->subclass = subclass;
		ss->protocol = protocol;
	} else {
		ss->subclass = iface->bInterfaceSubClass;
		ss->protocol = iface->bInterfaceProtocol;
	}

	/* set the handler pointers based on the protocol */
	USB_STOR_PRINTF("Transport: ");
	switch (ss->protocol) {
	case US_PR_CB:
		USB_STOR_PRINTF("Control/Bulk\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		break;

	case US_PR_CBI:
		USB_STOR_PRINTF("Control/Bulk/Interrupt\n");
		ss->transport = usb_stor_CB_transport;
		ss->transport_reset = usb_stor_CB_reset;
		break;
	default:
		printf("USB Starage Transport unknown / not yet implemented\n");
		return 0;
		break;
	}

	/*
	 * We are expecting a minimum of 2 endpoints - in and out (bulk).
	 * An optional interrupt is OK (necessary for CBI protocol).
	 * We will ignore any others.
	 */
	for (i = 0; i < iface->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &  USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_BULK) {
			if (iface->ep_desc[i].bEndpointAddress & USB_DIR_IN)
				ss->ep_in = iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
			else
				ss->ep_out = iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		    == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
		}
	}
	USB_STOR_PRINTF("Endpoints In %d Out %d Int %d\n",
		  ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (usb_set_interface(dev, iface->bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out ||
	    (ss->protocol == US_PR_CBI && ss->ep_int == 0)) {
		USB_STOR_PRINTF("Problems with device\n");
		return 0;
	}
	/* set class specific stuff */
	/* We only handle certain protocols.  Currently, this is
	 * the only one.
	 */
	if (ss->subclass != US_SC_UFI) {
		printf("Sorry, protocol %d not yet supported.\n",ss->subclass);
		return 0;
	}
	if(ss->ep_int) /* we had found an interrupt endpoint, prepare irq pipe */
	{
		/* set up the IRQ pipe and handler */

		ss->irqinterval = (ss->irqinterval > 0) ? ss->irqinterval : 255;
		ss->irqpipe = usb_rcvintpipe(ss->pusb_dev, ss->ep_int);
		ss->irqmaxp = usb_maxpacket(dev, ss->irqpipe);
		dev->irq_handle=usb_stor_irq;
		dev->privptr=(void *)ss;
	}
	return 1;
}

int usb_stor_get_info(struct usb_device *dev,struct us_data *ss,block_dev_desc_t *dev_desc)
{
	unsigned char perq,modi;
	unsigned long cap[2];
	unsigned long *capacity,*blksz;
	ccb *pccb=&usb_ccb;

	ss->transport_reset(ss);
	pccb->pdata=usb_stor_buf;

	dev_desc->target=dev->devnum;
	pccb->lun=dev_desc->lun;
	USB_STOR_PRINTF(" address %d\n",dev_desc->target);

	if(usb_inquiry(pccb,ss))
		return -1;
	perq=usb_stor_buf[0];
	modi=usb_stor_buf[1];
	if((perq & 0x1f)==0x1f) {
		return 0; /* skip unknown devices */
	}
	if((modi&0x80)==0x80) {/* drive is removable */
		dev_desc->removable=1;
	}
	memcpy(&dev_desc->vendor[0], &usb_stor_buf[8], 8);
	memcpy(&dev_desc->product[0], &usb_stor_buf[16], 16);
	memcpy(&dev_desc->revision[0], &usb_stor_buf[32], 4);
	dev_desc->vendor[8]=0;
	dev_desc->product[16]=0;
	dev_desc->revision[4]=0;
	USB_STOR_PRINTF("ISO Vers %X, Response Data %X\n",usb_stor_buf[2],usb_stor_buf[3]);
	if(usb_test_unit_ready(pccb,ss)) {
		printf("Device NOT ready\n   Request Sense returned %02X %02X %02X\n",pccb->sense_buf[2],pccb->sense_buf[12],pccb->sense_buf[13]);
		if(dev_desc->removable==1) {
			dev_desc->type=perq;
			return 1;
		}
		else
			return 0;
	}
	pccb->pdata=(unsigned char *)&cap[0];
	memset(pccb->pdata,0,8);
	if(usb_read_capacity(pccb,ss)!=0) {
		printf("READ_CAP ERROR\n");
		cap[0]=2880;
		cap[1]=0x200;
	}
	USB_STOR_PRINTF("Read Capacity returns: 0x%lx, 0x%lx\n",cap[0],cap[1]);
#if 0
	if(cap[0]>(0x200000 * 10)) /* greater than 10 GByte */
		cap[0]>>=16;
#endif
	cap[0]+=1;
	capacity=&cap[0];
	blksz=&cap[1];
	USB_STOR_PRINTF("Capacity = 0x%lx, blocksz = 0x%lx\n",*capacity,*blksz);
	dev_desc->lba=*capacity;
	dev_desc->blksz=*blksz;
	dev_desc->type=perq;
	USB_STOR_PRINTF(" address %d\n",dev_desc->target);
	USB_STOR_PRINTF("partype: %d\n",dev_desc->part_type);

	init_part(dev_desc);

	USB_STOR_PRINTF("partype: %d\n",dev_desc->part_type);
	return 1;
}

#endif
#endif /* CONFIG_USB_STORAGE */



