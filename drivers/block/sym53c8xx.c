/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 * partly derived from
 * linux/drivers/scsi/sym53c8xx.c
 *
 */

/*
 * SCSI support based on the chip sym53C810.
 *
 * 09-19-2001 Andreas Heppel, Sysgo RTS GmbH <aheppel@sysgo.de>
 *		The local version of this driver for the BAB750 board does not
 *		use interrupts but polls the chip instead (see the call of
 *		'handle_scsi_int()' in 'scsi_issue()'.
 */

#include <common.h>

#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <sym53c8xx.h>
#include <scsi.h>

#undef	SYM53C8XX_DEBUG

#ifdef	SYM53C8XX_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#if defined(CONFIG_SCSI) && defined(CONFIG_SCSI_SYM53C8XX)

#undef SCSI_SINGLE_STEP
/*
 * Single Step is only used for debug purposes
 */
#ifdef SCSI_SINGLE_STEP
static unsigned long start_script_select;
static unsigned long start_script_msgout;
static unsigned long start_script_msgin;
static unsigned long start_script_msg_ext;
static unsigned long start_script_cmd;
static unsigned long start_script_data_in;
static unsigned long start_script_data_out;
static unsigned long start_script_status;
static unsigned long start_script_complete;
static unsigned long start_script_error;
static unsigned long start_script_reselection;
static unsigned int len_script_select;
static unsigned int len_script_msgout;
static unsigned int len_script_msgin;
static unsigned int len_script_msg_ext;
static unsigned int len_script_cmd;
static unsigned int len_script_data_in;
static unsigned int len_script_data_out;
static unsigned int len_script_status;
static unsigned int len_script_complete;
static unsigned int len_script_error;
static unsigned int len_script_reselection;
#endif


static unsigned short scsi_int_mask;	/* shadow register for SCSI related interrupts */
static unsigned char  script_int_mask;	/* shadow register for SCRIPT related interrupts */
static unsigned long script_select[8];	/* script for selection */
static unsigned long script_msgout[8];	/* script for message out phase (NOT USED) */
static unsigned long script_msgin[14];	/* script for message in phase */
static unsigned long script_msg_ext[32]; /* script for message in phase when more than 1 byte message */
static unsigned long script_cmd[18];    /* script for command phase */
static unsigned long script_data_in[8]; /* script for data in phase */
static unsigned long script_data_out[8]; /* script for data out phase */
static unsigned long script_status[6]; /* script for status phase */
static unsigned long script_complete[10]; /* script for complete */
static unsigned long script_reselection[4]; /* script for reselection (NOT USED) */
static unsigned long script_error[2]; /* script for error handling */

static unsigned long int_stat[3]; /* interrupt status */
static unsigned long scsi_mem_addr; /* base memory address =SCSI_MEM_ADDRESS; */

#define bus_to_phys(a)	pci_mem_to_phys(busdevfunc, (unsigned long) (a))
#define phys_to_bus(a)	pci_phys_to_mem(busdevfunc, (unsigned long) (a))

#define SCSI_MAX_RETRY 3 /* number of retries in scsi_issue() */

#define SCSI_MAX_RETRY_NOT_READY 10 /* number of retries when device is not ready */
#define SCSI_NOT_READY_TIME_OUT 500 /* timeout per retry when not ready */

/*********************************************************************************
 * forward declerations
 */

void scsi_chip_init(void);
void handle_scsi_int(void);


/********************************************************************************
 * reports SCSI errors to the user
 */
void scsi_print_error (ccb * pccb)
{
	int i;

	printf ("SCSI Error: Target %d LUN %d Command %02X\n", pccb->target,
		pccb->lun, pccb->cmd[0]);
	printf ("       CCB: ");
	for (i = 0; i < pccb->cmdlen; i++)
		printf ("%02X ", pccb->cmd[i]);
	printf ("(len=%d)\n", pccb->cmdlen);
	printf ("     Cntrl: ");
	switch (pccb->contr_stat) {
	case SIR_COMPLETE:
		printf ("Complete (no Error)\n");
		break;
	case SIR_SEL_ATN_NO_MSG_OUT:
		printf ("Selected with ATN no MSG out phase\n");
		break;
	case SIR_CMD_OUT_ILL_PH:
		printf ("Command out illegal phase\n");
		break;
	case SIR_MSG_RECEIVED:
		printf ("MSG received Error\n");
		break;
	case SIR_DATA_IN_ERR:
		printf ("Data in Error\n");
		break;
	case SIR_DATA_OUT_ERR:
		printf ("Data out Error\n");
		break;
	case SIR_SCRIPT_ERROR:
		printf ("Script Error\n");
		break;
	case SIR_MSG_OUT_NO_CMD:
		printf ("MSG out no Command phase\n");
		break;
	case SIR_MSG_OVER7:
		printf ("MSG in over 7 bytes\n");
		break;
	case INT_ON_FY:
		printf ("Interrupt on fly\n");
		break;
	case SCSI_SEL_TIME_OUT:
		printf ("SCSI Selection Timeout\n");
		break;
	case SCSI_HNS_TIME_OUT:
		printf ("SCSI Handshake Timeout\n");
		break;
	case SCSI_MA_TIME_OUT:
		printf ("SCSI Phase Error\n");
		break;
	case SCSI_UNEXP_DIS:
		printf ("SCSI unexpected disconnect\n");
		break;
	default:
		printf ("unknown status %lx\n", pccb->contr_stat);
		break;
	}
	printf ("     Sense: SK %x (", pccb->sense_buf[2] & 0x0f);
	switch (pccb->sense_buf[2] & 0xf) {
	case SENSE_NO_SENSE:
		printf ("No Sense)");
		break;
	case SENSE_RECOVERED_ERROR:
		printf ("Recovered Error)");
		break;
	case SENSE_NOT_READY:
		printf ("Not Ready)");
		break;
	case SENSE_MEDIUM_ERROR:
		printf ("Medium Error)");
		break;
	case SENSE_HARDWARE_ERROR:
		printf ("Hardware Error)");
		break;
	case SENSE_ILLEGAL_REQUEST:
		printf ("Illegal request)");
		break;
	case SENSE_UNIT_ATTENTION:
		printf ("Unit Attention)");
		break;
	case SENSE_DATA_PROTECT:
		printf ("Data Protect)");
		break;
	case SENSE_BLANK_CHECK:
		printf ("Blank check)");
		break;
	case SENSE_VENDOR_SPECIFIC:
		printf ("Vendor specific)");
		break;
	case SENSE_COPY_ABORTED:
		printf ("Copy aborted)");
		break;
	case SENSE_ABORTED_COMMAND:
		printf ("Aborted Command)");
		break;
	case SENSE_VOLUME_OVERFLOW:
		printf ("Volume overflow)");
		break;
	case SENSE_MISCOMPARE:
		printf ("Misscompare\n");
		break;
	default:
		printf ("Illegal Sensecode\n");
		break;
	}
	printf (" ASC %x ASCQ %x\n", pccb->sense_buf[12],
		pccb->sense_buf[13]);
	printf ("    Status: ");
	switch (pccb->status) {
	case S_GOOD:
		printf ("Good\n");
		break;
	case S_CHECK_COND:
		printf ("Check condition\n");
		break;
	case S_COND_MET:
		printf ("Condition Met\n");
		break;
	case S_BUSY:
		printf ("Busy\n");
		break;
	case S_INT:
		printf ("Intermediate\n");
		break;
	case S_INT_COND_MET:
		printf ("Intermediate condition met\n");
		break;
	case S_CONFLICT:
		printf ("Reservation conflict\n");
		break;
	case S_TERMINATED:
		printf ("Command terminated\n");
		break;
	case S_QUEUE_FULL:
		printf ("Task set full\n");
		break;
	default:
		printf ("unknown: %02X\n", pccb->status);
		break;
	}

}


/******************************************************************************
 * sets-up the SCSI controller
 * the base memory address is retrieved via the pci_read_config_dword
 */
void scsi_low_level_init(int busdevfunc)
{
	unsigned int cmd;
	unsigned int addr;
	unsigned char vec;

	pci_read_config_byte(busdevfunc, PCI_INTERRUPT_LINE, &vec);
	pci_read_config_dword(busdevfunc, PCI_BASE_ADDRESS_1, &addr);

	addr = bus_to_phys(addr & ~0xf);

	/*
	 * Enable bus mastering in case this has not been done, yet.
	 */
	pci_read_config_dword(busdevfunc, PCI_COMMAND, &cmd);
	cmd |= PCI_COMMAND_MASTER;
	pci_write_config_dword(busdevfunc, PCI_COMMAND, cmd);

	scsi_mem_addr = addr;

	scsi_chip_init();
	scsi_bus_reset();
}


/************************************************************************************
 * Low level Part of SCSI Driver
 */

/*
 * big-endian -> little endian conversion for the script
 */
unsigned long swap_script(unsigned long val)
{
	unsigned long tmp;
	tmp = ((val>>24)&0xff) | ((val>>8)&0xff00) | ((val<<8)&0xff0000) | ((val<<24)&0xff000000);
	return tmp;
}


void scsi_write_byte(ulong offset,unsigned char val)
{
	out8(scsi_mem_addr+offset,val);
}


unsigned char scsi_read_byte(ulong offset)
{
	return(in8(scsi_mem_addr+offset));
}


/********************************************************************************
 * interrupt handler
 */
void handle_scsi_int(void)
{
	unsigned char stat,stat1,stat2;
	unsigned short sstat;
	int i;
#ifdef SCSI_SINGLE_STEP
	unsigned long tt;
#endif
	stat=scsi_read_byte(ISTAT);
	if((stat & DIP)==DIP) { /* DMA Interrupt pending */
		stat1=scsi_read_byte(DSTAT);
#ifdef SCSI_SINGLE_STEP
		if((stat1 & SSI)==SSI) {
			tt=in32r(scsi_mem_addr+DSP);
			if(((tt)>=start_script_select) && ((tt)<start_script_select+len_script_select)) {
				printf("select %d\n",(tt-start_script_select)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_msgout) && ((tt)<start_script_msgout+len_script_msgout)) {
				printf("msgout %d\n",(tt-start_script_msgout)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_msgin) && ((tt)<start_script_msgin+len_script_msgin)) {
				printf("msgin %d\n",(tt-start_script_msgin)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_msg_ext) && ((tt)<start_script_msg_ext+len_script_msg_ext)) {
				printf("msgin_ext %d\n",(tt-start_script_msg_ext)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_cmd) && ((tt)<start_script_cmd+len_script_cmd)) {
				printf("cmd %d\n",(tt-start_script_cmd)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_data_in) && ((tt)<start_script_data_in+len_script_data_in)) {
				printf("data_in %d\n",(tt-start_script_data_in)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_data_out) && ((tt)<start_script_data_out+len_script_data_out)) {
				printf("data_out %d\n",(tt-start_script_data_out)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_status) && ((tt)<start_script_status+len_script_status)) {
				printf("status %d\n",(tt-start_script_status)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_complete) && ((tt)<start_script_complete+len_script_complete)) {
				printf("complete %d\n",(tt-start_script_complete)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_error) && ((tt)<start_script_error+len_script_error)) {
				printf("error %d\n",(tt-start_script_error)>>2);
				goto end_single;
			}
			if(((tt)>=start_script_reselection) && ((tt)<start_script_reselection+len_script_reselection)) {
				printf("reselection %d\n",(tt-start_script_reselection)>>2);
				goto end_single;
			}
			printf("sc: %lx\n",tt);
end_single:
			stat2=scsi_read_byte(DCNTL);
			stat2|=STD;
			scsi_write_byte(DCNTL,stat2);
		}
#endif
		if((stat1 & SIR)==SIR) /* script interrupt */
		{
			int_stat[0]=in32(scsi_mem_addr+DSPS);
		}
		if((stat1 & DFE)==0) { /* fifo not epmty */
			scsi_write_byte(CTEST3,CLF); /* Clear DMA FIFO */
			stat2=scsi_read_byte(STEST3);
			scsi_write_byte(STEST3,(stat2 | CSF)); /* Clear SCSI FIFO */
		}
	}
	if((stat & SIP)==SIP) {  /* scsi interrupt */
		sstat = (unsigned short)scsi_read_byte(SIST+1);
		sstat <<=8;
		sstat |= (unsigned short)scsi_read_byte(SIST);
		for(i=0;i<3;i++) {
			if(int_stat[i]==0)
				break; /* found an empty int status */
		}
		int_stat[i]=SCSI_INT_STATE | sstat;
		stat1=scsi_read_byte(DSTAT);
		if((stat1 & DFE)==0) { /* fifo not epmty */
			scsi_write_byte(CTEST3,CLF); /* Clear DMA FIFO */
			stat2=scsi_read_byte(STEST3);
			scsi_write_byte(STEST3,(stat2 | CSF)); /* Clear SCSI FIFO */
		}
	}
	if((stat & INTF)==INTF) { /* interrupt on Fly */
		scsi_write_byte(ISTAT,stat); /* clear it */
		for(i=0;i<3;i++) {
			if(int_stat[i]==0)
				break; /* found an empty int status */
		}
		int_stat[i]=INT_ON_FY;
	}
}

void scsi_bus_reset(void)
{
	unsigned char t;
	int i;
	int end = CONFIG_SYS_SCSI_SPIN_UP_TIME*1000;

	t=scsi_read_byte(SCNTL1);
	scsi_write_byte(SCNTL1,(t | CRST));
	udelay(50);
	scsi_write_byte(SCNTL1,t);

	puts("waiting for devices to spin up");
	for(i=0;i<end;i++) {
		udelay(1000); /* give the devices time to spin up */
		if (i % 1000 == 0)
			putc('.');
	}
	putc('\n');
	scsi_chip_init(); /* reinit the chip ...*/

}

void scsi_int_enable(void)
{
	scsi_write_byte(SIEN,(unsigned char)scsi_int_mask);
	scsi_write_byte(SIEN+1,(unsigned char)(scsi_int_mask>>8));
	scsi_write_byte(DIEN,script_int_mask);
}

void scsi_write_dsp(unsigned long start)
{
#ifdef SCSI_SINGLE_STEP
	unsigned char t;
#endif
	out32r(scsi_mem_addr + DSP,start);
#ifdef SCSI_SINGLE_STEP
	t=scsi_read_byte(DCNTL);
  t|=STD;
	scsi_write_byte(DCNTL,t);
#endif
}

/* only used for debug purposes */
void scsi_print_script(void)
{
	printf("script_select @         0x%08lX\n",(unsigned long)&script_select[0]);
	printf("script_msgout @         0x%08lX\n",(unsigned long)&script_msgout[0]);
	printf("script_msgin @          0x%08lX\n",(unsigned long)&script_msgin[0]);
	printf("script_msgext @         0x%08lX\n",(unsigned long)&script_msg_ext[0]);
	printf("script_cmd @            0x%08lX\n",(unsigned long)&script_cmd[0]);
	printf("script_data_in @        0x%08lX\n",(unsigned long)&script_data_in[0]);
	printf("script_data_out @       0x%08lX\n",(unsigned long)&script_data_out[0]);
	printf("script_status @         0x%08lX\n",(unsigned long)&script_status[0]);
	printf("script_complete @       0x%08lX\n",(unsigned long)&script_complete[0]);
	printf("script_error @          0x%08lX\n",(unsigned long)&script_error[0]);
}


void scsi_set_script(ccb *pccb)
{
	int busdevfunc = pccb->priv;
	int i;
	i=0;
	script_select[i++]=swap_script(SCR_REG_REG(GPREG, SCR_AND, 0xfe));
	script_select[i++]=0; /* LED ON */
	script_select[i++]=swap_script(SCR_CLR(SCR_TRG)); /* select initiator mode */
	script_select[i++]=0;
	/* script_select[i++]=swap_script(SCR_SEL_ABS_ATN | pccb->target << 16); */
	script_select[i++]=swap_script(SCR_SEL_ABS | pccb->target << 16);
	script_select[i++]=swap_script(phys_to_bus(&script_cmd[4])); /* error handling */
	script_select[i++]=swap_script(SCR_JUMP); /* next section */
	/*	script_select[i++]=swap_script((unsigned long)&script_msgout[0]); */ /* message out */
	script_select[i++]=swap_script(phys_to_bus(&script_cmd[0])); /* command out */

#ifdef SCSI_SINGLE_STEP
	start_script_select=(unsigned long)&script_select[0];
	len_script_select=i*4;
#endif

	i=0;
	script_msgout[i++]=swap_script(SCR_INT ^ IFFALSE (WHEN (SCR_MSG_OUT)));
	script_msgout[i++]=SIR_SEL_ATN_NO_MSG_OUT;
	script_msgout[i++]=swap_script(	SCR_MOVE_ABS(1) ^ SCR_MSG_OUT);
	script_msgout[i++]=swap_script(phys_to_bus(&pccb->msgout[0]));
	script_msgout[i++]=swap_script(SCR_JUMP ^ IFTRUE (WHEN (SCR_COMMAND))); /* if Command phase */
	script_msgout[i++]=swap_script(phys_to_bus(&script_cmd[0])); /* switch to command */
	script_msgout[i++]=swap_script(SCR_INT); /* interrupt if not */
	script_msgout[i++]=SIR_MSG_OUT_NO_CMD;

#ifdef SCSI_SINGLE_STEP
	start_script_msgout=(unsigned long)&script_msgout[0];
	len_script_msgout=i*4;
#endif
	i=0;
	script_cmd[i++]=swap_script(SCR_MOVE_ABS(pccb->cmdlen) ^ SCR_COMMAND);
	script_cmd[i++]=swap_script(phys_to_bus(&pccb->cmd[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (WHEN (SCR_MSG_IN))); /* message in ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_msgin[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (IF (SCR_DATA_OUT))); /* data out ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_data_out[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (IF (SCR_DATA_IN))); /* data in ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_data_in[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (IF (SCR_STATUS)));  /* status ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_status[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (IF (SCR_COMMAND)));  /* command ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_cmd[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (IF (SCR_MSG_OUT)));  /* message out ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_msgout[0]));
	script_cmd[i++]=swap_script(SCR_JUMP ^ IFTRUE (IF (SCR_MSG_IN))); /* just for error handling message in ? */
	script_cmd[i++]=swap_script(phys_to_bus(&script_msgin[0]));
	script_cmd[i++]=swap_script(SCR_INT); /* interrupt if not */
	script_cmd[i++]=SIR_CMD_OUT_ILL_PH;
#ifdef SCSI_SINGLE_STEP
	start_script_cmd=(unsigned long)&script_cmd[0];
	len_script_cmd=i*4;
#endif
	i=0;
	script_data_out[i++]=swap_script(SCR_MOVE_ABS(pccb->datalen)^ SCR_DATA_OUT); /* move */
	script_data_out[i++]=swap_script(phys_to_bus(pccb->pdata)); /* pointer to buffer */
	script_data_out[i++]=swap_script(SCR_JUMP ^ IFTRUE (WHEN (SCR_STATUS)));
	script_data_out[i++]=swap_script(phys_to_bus(&script_status[0]));
	script_data_out[i++]=swap_script(SCR_INT);
	script_data_out[i++]=SIR_DATA_OUT_ERR;

#ifdef SCSI_SINGLE_STEP
	start_script_data_out=(unsigned long)&script_data_out[0];
	len_script_data_out=i*4;
#endif
	i=0;
	script_data_in[i++]=swap_script(SCR_MOVE_ABS(pccb->datalen)^ SCR_DATA_IN); /* move  */
	script_data_in[i++]=swap_script(phys_to_bus(pccb->pdata)); /* pointer to buffer */
	script_data_in[i++]=swap_script(SCR_JUMP ^ IFTRUE (WHEN (SCR_STATUS)));
	script_data_in[i++]=swap_script(phys_to_bus(&script_status[0]));
	script_data_in[i++]=swap_script(SCR_INT);
	script_data_in[i++]=SIR_DATA_IN_ERR;
#ifdef SCSI_SINGLE_STEP
	start_script_data_in=(unsigned long)&script_data_in[0];
	len_script_data_in=i*4;
#endif
	i=0;
	script_msgin[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN);
	script_msgin[i++]=swap_script(phys_to_bus(&pccb->msgin[0]));
	script_msgin[i++]=swap_script(SCR_JUMP ^ IFTRUE (DATA (M_COMPLETE)));
	script_msgin[i++]=swap_script(phys_to_bus(&script_complete[0]));
	script_msgin[i++]=swap_script(SCR_JUMP ^ IFTRUE (DATA (M_DISCONNECT)));
	script_msgin[i++]=swap_script(phys_to_bus(&script_complete[0]));
	script_msgin[i++]=swap_script(SCR_JUMP ^ IFTRUE (DATA (M_SAVE_DP)));
	script_msgin[i++]=swap_script(phys_to_bus(&script_complete[0]));
	script_msgin[i++]=swap_script(SCR_JUMP ^ IFTRUE (DATA (M_RESTORE_DP)));
	script_msgin[i++]=swap_script(phys_to_bus(&script_complete[0]));
	script_msgin[i++]=swap_script(SCR_JUMP ^ IFTRUE (DATA (M_EXTENDED)));
	script_msgin[i++]=swap_script(phys_to_bus(&script_msg_ext[0]));
	script_msgin[i++]=swap_script(SCR_INT);
	script_msgin[i++]=SIR_MSG_RECEIVED;
#ifdef SCSI_SINGLE_STEP
	start_script_msgin=(unsigned long)&script_msgin[0];
	len_script_msgin=i*4;
#endif
	i=0;
	script_msg_ext[i++]=swap_script(SCR_CLR (SCR_ACK)); /* clear ACK */
	script_msg_ext[i++]=0;
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* assuming this is the msg length */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[1]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* next */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[2]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* next */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[3]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* next */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[4]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* next */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[5]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* next */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[6]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_MSG_IN); /* next */
	script_msg_ext[i++]=swap_script(phys_to_bus(&pccb->msgin[7]));
	script_msg_ext[i++]=swap_script(SCR_JUMP ^ IFFALSE (IF (SCR_MSG_IN)));
	script_msg_ext[i++]=swap_script(phys_to_bus(&script_complete[0])); /* no more bytes */
	script_msg_ext[i++]=swap_script(SCR_INT);
	script_msg_ext[i++]=SIR_MSG_OVER7;
#ifdef SCSI_SINGLE_STEP
	start_script_msg_ext=(unsigned long)&script_msg_ext[0];
	len_script_msg_ext=i*4;
#endif
	i=0;
	script_status[i++]=swap_script(SCR_MOVE_ABS (1) ^ SCR_STATUS);
	script_status[i++]=swap_script(phys_to_bus(&pccb->status));
	script_status[i++]=swap_script(SCR_JUMP ^ IFTRUE (WHEN (SCR_MSG_IN)));
	script_status[i++]=swap_script(phys_to_bus(&script_msgin[0]));
	script_status[i++]=swap_script(SCR_INT);
	script_status[i++]=SIR_STATUS_ILL_PH;
#ifdef SCSI_SINGLE_STEP
	start_script_status=(unsigned long)&script_status[0];
	len_script_status=i*4;
#endif
	i=0;
	script_complete[i++]=swap_script(SCR_REG_REG (SCNTL2, SCR_AND, 0x7f));
	script_complete[i++]=0;
	script_complete[i++]=swap_script(SCR_CLR (SCR_ACK|SCR_ATN));
	script_complete[i++]=0;
	script_complete[i++]=swap_script(SCR_WAIT_DISC);
	script_complete[i++]=0;
	script_complete[i++]=swap_script(SCR_REG_REG(GPREG, SCR_OR, 0x01));
	script_complete[i++]=0; /* LED OFF */
	script_complete[i++]=swap_script(SCR_INT);
	script_complete[i++]=SIR_COMPLETE;
#ifdef SCSI_SINGLE_STEP
	start_script_complete=(unsigned long)&script_complete[0];
	len_script_complete=i*4;
#endif
	i=0;
	script_error[i++]=swap_script(SCR_INT); /* interrupt if error */
	script_error[i++]=SIR_SCRIPT_ERROR;
#ifdef SCSI_SINGLE_STEP
	start_script_error=(unsigned long)&script_error[0];
	len_script_error=i*4;
#endif
	i=0;
	script_reselection[i++]=swap_script(SCR_CLR (SCR_TRG)); /* target status */
	script_reselection[i++]=0;
	script_reselection[i++]=swap_script(SCR_WAIT_RESEL);
	script_reselection[i++]=swap_script(phys_to_bus(&script_select[0])); /* len = 4 */
#ifdef SCSI_SINGLE_STEP
	start_script_reselection=(unsigned long)&script_reselection[0];
	len_script_reselection=i*4;
#endif
}


void scsi_issue(ccb *pccb)
{
	int busdevfunc = pccb->priv;
	int i;
	unsigned short sstat;
	int retrycnt;  /* retry counter */
	for(i=0;i<3;i++)
		int_stat[i]=0; /* delete all int status */
	/* struct pccb must be set-up correctly */
	retrycnt=0;
	PRINTF("ID %d issue cmd %02X\n",pccb->target,pccb->cmd[0]);
	pccb->trans_bytes=0; /* no bytes transferred yet */
	scsi_set_script(pccb); /* fill in SCRIPT		*/
	scsi_int_mask=STO | UDC | MA; /* | CMP; / * Interrupts which are enabled */
	script_int_mask=0xff; /* enable all Ints */
	scsi_int_enable();
	scsi_write_dsp(phys_to_bus(&script_select[0])); /* start script */
	/* now we have to wait for IRQs */
retry:
	/*
	 * This version of the driver is _not_ interrupt driven,
	 * but polls the chip's interrupt registers (ISTAT, DSTAT).
	 */
	while(int_stat[0]==0)
		handle_scsi_int();

	if(int_stat[0]==SIR_COMPLETE) {
		if(pccb->msgin[0]==M_DISCONNECT) {
			PRINTF("Wait for reselection\n");
			for(i=0;i<3;i++)
				int_stat[i]=0; /* delete all int status */
			scsi_write_dsp(phys_to_bus(&script_reselection[0])); /* start reselection script */
			goto retry;
		}
		pccb->contr_stat=SIR_COMPLETE;
		return;
	}
	if((int_stat[0] & SCSI_INT_STATE)==SCSI_INT_STATE) { /* scsi interrupt */
		sstat=(unsigned short)int_stat[0];
		if((sstat & STO)==STO) { /* selection timeout */
			pccb->contr_stat=SCSI_SEL_TIME_OUT;
			scsi_write_byte(GPREG,0x01);
			PRINTF("ID: %X Selection Timeout\n",pccb->target);
			return;
		}
		if((sstat & UDC)==UDC) { /* unexpected disconnect */
			pccb->contr_stat=SCSI_UNEXP_DIS;
			scsi_write_byte(GPREG,0x01);
			PRINTF("ID: %X Unexpected Disconnect\n",pccb->target);
			return;
		}
		if((sstat & RSL)==RSL) { /* reselection */
			pccb->contr_stat=SCSI_UNEXP_DIS;
			scsi_write_byte(GPREG,0x01);
			PRINTF("ID: %X Unexpected Disconnect\n",pccb->target);
			return;
		}
		if(((sstat & MA)==MA)||((sstat & HTH)==HTH)) { /* phase missmatch */
			if(retrycnt<SCSI_MAX_RETRY) {
				pccb->trans_bytes=pccb->datalen -
					((unsigned long)scsi_read_byte(DBC) |
					((unsigned long)scsi_read_byte(DBC+1)<<8) |
					((unsigned long)scsi_read_byte(DBC+2)<<16));
				for(i=0;i<3;i++)
					int_stat[i]=0; /* delete all int status */
				retrycnt++;
				PRINTF("ID: %X Phase Missmatch Retry %d Phase %02X transferred %lx\n",
						pccb->target,retrycnt,scsi_read_byte(SBCL),pccb->trans_bytes);
				scsi_write_dsp(phys_to_bus(&script_cmd[4])); /* start retry script */
				goto retry;
			}
			if((sstat & MA)==MA)
				pccb->contr_stat=SCSI_MA_TIME_OUT;
			else
				pccb->contr_stat=SCSI_HNS_TIME_OUT;
			PRINTF("Phase Missmatch stat %lx\n",pccb->contr_stat);
			return;
		} /* no phase int */
/*		if((sstat & CMP)==CMP) {
			pccb->contr_stat=SIR_COMPLETE;
			return;
		}
*/
		PRINTF("SCSI INT %lX\n",int_stat[0]);
		pccb->contr_stat=int_stat[0];
		return;
	} /* end scsi int */
	PRINTF("SCRIPT INT %lX phase %02X\n",int_stat[0],scsi_read_byte(SBCL));
	pccb->contr_stat=int_stat[0];
	return;
}

int scsi_exec(ccb *pccb)
{
	unsigned char tmpcmd[16],tmpstat;
	int i,retrycnt,t;
	unsigned long transbytes,datalen;
	unsigned char *tmpptr;
	retrycnt=0;
retry:
	scsi_issue(pccb);
	if(pccb->contr_stat!=SIR_COMPLETE)
		return false;
	if(pccb->status==S_GOOD)
		return true;
	if(pccb->status==S_CHECK_COND) { /* check condition */
		for(i=0;i<16;i++)
			tmpcmd[i]=pccb->cmd[i];
		pccb->cmd[0]=SCSI_REQ_SENSE;
		pccb->cmd[1]=pccb->lun<<5;
		pccb->cmd[2]=0;
		pccb->cmd[3]=0;
		pccb->cmd[4]=14;
		pccb->cmd[5]=0;
		pccb->cmdlen=6;
		pccb->msgout[0]=SCSI_IDENTIFY;
		transbytes=pccb->trans_bytes;
		tmpptr=pccb->pdata;
		pccb->pdata = &pccb->sense_buf[0];
		datalen=pccb->datalen;
		pccb->datalen=14;
		tmpstat=pccb->status;
		scsi_issue(pccb);
		for(i=0;i<16;i++)
			pccb->cmd[i]=tmpcmd[i];
		pccb->trans_bytes=transbytes;
		pccb->pdata=tmpptr;
		pccb->datalen=datalen;
		pccb->status=tmpstat;
		PRINTF("Request_sense sense key %x ASC %x ASCQ %x\n",pccb->sense_buf[2]&0x0f,
			pccb->sense_buf[12],pccb->sense_buf[13]);
		switch(pccb->sense_buf[2]&0xf) {
			case SENSE_NO_SENSE:
			case SENSE_RECOVERED_ERROR:
				/* seems to be ok */
				return true;
				break;
			case SENSE_NOT_READY:
				if((pccb->sense_buf[12]!=0x04)||(pccb->sense_buf[13]!=0x01)) {
					/* if device is not in process of becoming ready */
					return false;
					break;
				} /* else fall through */
			case SENSE_UNIT_ATTENTION:
				if(retrycnt<SCSI_MAX_RETRY_NOT_READY) {
					PRINTF("Target %d not ready, retry %d\n",pccb->target,retrycnt);
					for(t=0;t<SCSI_NOT_READY_TIME_OUT;t++)
						udelay(1000); /* 1sec wait */
					retrycnt++;
					goto retry;
				}
				PRINTF("Target %d not ready, %d retried\n",pccb->target,retrycnt);
				return false;
			default:
				return false;
		}
	}
	PRINTF("Status = %X\n",pccb->status);
	return false;
}


void scsi_chip_init(void)
{
	/* first we issue a soft reset */
	scsi_write_byte(ISTAT,SRST);
	udelay(1000);
	scsi_write_byte(ISTAT,0);
	/* setup chip */
	scsi_write_byte(SCNTL0,0xC0); /* full arbitration no start, no message, parity disabled, master */
	scsi_write_byte(SCNTL1,0x00);
	scsi_write_byte(SCNTL2,0x00);
#ifndef CONFIG_SYS_SCSI_SYM53C8XX_CCF    /* config value for none 40 MHz clocks */
	scsi_write_byte(SCNTL3,0x13); /* synchronous clock 40/4=10MHz, asynchronous 40MHz */
#else
	scsi_write_byte(SCNTL3,CONFIG_SYS_SCSI_SYM53C8XX_CCF); /* config value for none 40 MHz clocks */
#endif
	scsi_write_byte(SCID,0x47); /* ID=7, enable reselection */
	scsi_write_byte(SXFER,0x00); /* synchronous transfer period 10MHz, asynchronous */
	scsi_write_byte(SDID,0x00);  /* targed SCSI ID = 0 */
	scsi_int_mask=0x0000; /* no Interrupt is enabled */
	script_int_mask=0x00;
	scsi_int_enable();
	scsi_write_byte(GPREG,0x01); /* GPIO0 is LED (off) */
	scsi_write_byte(GPCNTL,0x0E); /* GPIO0 is Output */
	scsi_write_byte(STIME0,0x08); /* handshake timer disabled, selection timeout 512msec */
	scsi_write_byte(RESPID,0x80); /* repond only to the own ID (reselection) */
	scsi_write_byte(STEST1,0x00); /* not isolated, SCLK is used */
	scsi_write_byte(STEST2,0x00); /* no Lowlevel Mode? */
	scsi_write_byte(STEST3,0x80); /* enable tolerANT */
	scsi_write_byte(CTEST3,0x04); /* clear FIFO */
	scsi_write_byte(CTEST4,0x00);
	scsi_write_byte(CTEST5,0x00);
#ifdef SCSI_SINGLE_STEP
/*	scsi_write_byte(DCNTL,IRQM | SSM);	*/
	scsi_write_byte(DCNTL,IRQD | SSM);
	scsi_write_byte(DMODE,MAN);
#else
/*	scsi_write_byte(DCNTL,IRQM);	*/
	scsi_write_byte(DCNTL,IRQD);
	scsi_write_byte(DMODE,0x00);
#endif
}
#endif
