/*
 * ISP116x register declarations and HCD data structures
 *
 * Copyright (C) 2007 Rodolfo Giometti <giometti@linux.it>
 * Copyright (C) 2007 Eurotech S.p.A. <info@eurotech.it>
 * Copyright (C) 2005 Olav Kongas <ok@artecdesign.ee>
 * Portions:
 * Copyright (C) 2004 Lothar Wassmann
 * Copyright (C) 2004 Psion Teklogix
 * Copyright (C) 2004 David Brownell
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifdef DEBUG
#define DBG(fmt, args...)	\
		printf("isp116x: %s: " fmt "\n" , __FUNCTION__ , ## args)
#else
#define DBG(fmt, args...)	do {} while (0)
#endif

#ifdef VERBOSE
#    define VDBG		DBG
#else
#    define VDBG(fmt, args...)	do {} while (0)
#endif

#define ERR(fmt, args...)	\
		printf("isp116x: %s: " fmt "\n" , __FUNCTION__ , ## args)
#define WARN(fmt, args...)	\
		printf("isp116x: %s: " fmt "\n" , __FUNCTION__ , ## args)
#define INFO(fmt, args...)	\
		printf("isp116x: " fmt "\n" , ## args)

/* ------------------------------------------------------------------------- */

/* us of 1ms frame */
#define  MAX_LOAD_LIMIT		850

/* Full speed: max # of bytes to transfer for a single urb
   at a time must be < 1024 && must be multiple of 64.
   832 allows transfering 4kiB within 5 frames. */
#define MAX_TRANSFER_SIZE_FULLSPEED	832

/* Low speed: there is no reason to schedule in very big
   chunks; often the requested long transfers are for
   string descriptors containing short strings. */
#define MAX_TRANSFER_SIZE_LOWSPEED	64

/* Bytetime (us), a rough indication of how much time it
   would take to transfer a byte of useful data over USB */
#define BYTE_TIME_FULLSPEED	1
#define BYTE_TIME_LOWSPEED	20

/* Buffer sizes */
#define ISP116x_BUF_SIZE	4096
#define ISP116x_ITL_BUFSIZE	0
#define ISP116x_ATL_BUFSIZE	((ISP116x_BUF_SIZE) - 2*(ISP116x_ITL_BUFSIZE))

#define ISP116x_WRITE_OFFSET	0x80

/* --- ISP116x registers/bits ---------------------------------------------- */

#define	HCREVISION	0x00
#define	HCCONTROL	0x01
#define		HCCONTROL_HCFS	(3 << 6)	/* host controller
						   functional state */
#define		HCCONTROL_USB_RESET	(0 << 6)
#define		HCCONTROL_USB_RESUME	(1 << 6)
#define		HCCONTROL_USB_OPER	(2 << 6)
#define		HCCONTROL_USB_SUSPEND	(3 << 6)
#define		HCCONTROL_RWC	(1 << 9)	/* remote wakeup connected */
#define		HCCONTROL_RWE	(1 << 10)	/* remote wakeup enable */
#define	HCCMDSTAT	0x02
#define		HCCMDSTAT_HCR	(1 << 0)	/* host controller reset */
#define		HCCMDSTAT_SOC	(3 << 16)	/* scheduling overrun count */
#define	HCINTSTAT	0x03
#define		HCINT_SO	(1 << 0)	/* scheduling overrun */
#define		HCINT_WDH	(1 << 1)	/* writeback of done_head */
#define		HCINT_SF	(1 << 2)	/* start frame */
#define		HCINT_RD	(1 << 3)	/* resume detect */
#define		HCINT_UE	(1 << 4)	/* unrecoverable error */
#define		HCINT_FNO	(1 << 5)	/* frame number overflow */
#define		HCINT_RHSC	(1 << 6)	/* root hub status change */
#define		HCINT_OC	(1 << 30)	/* ownership change */
#define		HCINT_MIE	(1 << 31)	/* master interrupt enable */
#define	HCINTENB	0x04
#define	HCINTDIS	0x05
#define	HCFMINTVL	0x0d
#define	HCFMREM		0x0e
#define	HCFMNUM		0x0f
#define	HCLSTHRESH	0x11
#define	HCRHDESCA	0x12
#define		RH_A_NDP	(0x3 << 0)	/* # downstream ports */
#define		RH_A_PSM	(1 << 8)	/* power switching mode */
#define		RH_A_NPS	(1 << 9)	/* no power switching */
#define		RH_A_DT		(1 << 10)	/* device type (mbz) */
#define		RH_A_OCPM	(1 << 11)	/* overcurrent protection
						   mode */
#define		RH_A_NOCP	(1 << 12)	/* no overcurrent protection */
#define		RH_A_POTPGT	(0xff << 24)	/* power on -> power good
						   time */
#define	HCRHDESCB	0x13
#define		RH_B_DR		(0xffff << 0)	/* device removable flags */
#define		RH_B_PPCM	(0xffff << 16)	/* port power control mask */
#define	HCRHSTATUS	0x14
#define		RH_HS_LPS	(1 << 0)	/* local power status */
#define		RH_HS_OCI	(1 << 1)	/* over current indicator */
#define		RH_HS_DRWE	(1 << 15)	/* device remote wakeup
						   enable */
#define		RH_HS_LPSC	(1 << 16)	/* local power status change */
#define		RH_HS_OCIC	(1 << 17)	/* over current indicator
						   change */
#define		RH_HS_CRWE	(1 << 31)	/* clear remote wakeup
						   enable */
#define	HCRHPORT1	0x15
#define		RH_PS_CCS	(1 << 0)	/* current connect status */
#define		RH_PS_PES	(1 << 1)	/* port enable status */
#define		RH_PS_PSS	(1 << 2)	/* port suspend status */
#define		RH_PS_POCI	(1 << 3)	/* port over current
						   indicator */
#define		RH_PS_PRS	(1 << 4)	/* port reset status */
#define		RH_PS_PPS	(1 << 8)	/* port power status */
#define		RH_PS_LSDA	(1 << 9)	/* low speed device attached */
#define		RH_PS_CSC	(1 << 16)	/* connect status change */
#define		RH_PS_PESC	(1 << 17)	/* port enable status change */
#define		RH_PS_PSSC	(1 << 18)	/* port suspend status
						   change */
#define		RH_PS_OCIC	(1 << 19)	/* over current indicator
						   change */
#define		RH_PS_PRSC	(1 << 20)	/* port reset status change */
#define		HCRHPORT_CLRMASK	(0x1f << 16)
#define	HCRHPORT2	0x16
#define	HCHWCFG		0x20
#define		HCHWCFG_15KRSEL		(1 << 12)
#define		HCHWCFG_CLKNOTSTOP	(1 << 11)
#define		HCHWCFG_ANALOG_OC	(1 << 10)
#define		HCHWCFG_DACK_MODE	(1 << 8)
#define		HCHWCFG_EOT_POL		(1 << 7)
#define		HCHWCFG_DACK_POL	(1 << 6)
#define		HCHWCFG_DREQ_POL	(1 << 5)
#define		HCHWCFG_DBWIDTH_MASK	(0x03 << 3)
#define		HCHWCFG_DBWIDTH(n)	(((n) << 3) & HCHWCFG_DBWIDTH_MASK)
#define		HCHWCFG_INT_POL		(1 << 2)
#define		HCHWCFG_INT_TRIGGER	(1 << 1)
#define		HCHWCFG_INT_ENABLE	(1 << 0)
#define	HCDMACFG	0x21
#define		HCDMACFG_BURST_LEN_MASK	(0x03 << 5)
#define		HCDMACFG_BURST_LEN(n)	(((n) << 5) & HCDMACFG_BURST_LEN_MASK)
#define		HCDMACFG_BURST_LEN_1	HCDMACFG_BURST_LEN(0)
#define		HCDMACFG_BURST_LEN_4	HCDMACFG_BURST_LEN(1)
#define		HCDMACFG_BURST_LEN_8	HCDMACFG_BURST_LEN(2)
#define		HCDMACFG_DMA_ENABLE	(1 << 4)
#define		HCDMACFG_BUF_TYPE_MASK	(0x07 << 1)
#define		HCDMACFG_CTR_SEL	(1 << 2)
#define		HCDMACFG_ITLATL_SEL	(1 << 1)
#define		HCDMACFG_DMA_RW_SELECT	(1 << 0)
#define	HCXFERCTR	0x22
#define	HCuPINT		0x24
#define		HCuPINT_SOF		(1 << 0)
#define		HCuPINT_ATL		(1 << 1)
#define		HCuPINT_AIIEOT		(1 << 2)
#define		HCuPINT_OPR		(1 << 4)
#define		HCuPINT_SUSP		(1 << 5)
#define		HCuPINT_CLKRDY		(1 << 6)
#define	HCuPINTENB	0x25
#define	HCCHIPID	0x27
#define		HCCHIPID_MASK		0xff00
#define		HCCHIPID_MAGIC		0x6100
#define	HCSCRATCH	0x28
#define	HCSWRES		0x29
#define		HCSWRES_MAGIC		0x00f6
#define	HCITLBUFLEN	0x2a
#define	HCATLBUFLEN	0x2b
#define	HCBUFSTAT	0x2c
#define		HCBUFSTAT_ITL0_FULL	(1 << 0)
#define		HCBUFSTAT_ITL1_FULL	(1 << 1)
#define		HCBUFSTAT_ATL_FULL	(1 << 2)
#define		HCBUFSTAT_ITL0_DONE	(1 << 3)
#define		HCBUFSTAT_ITL1_DONE	(1 << 4)
#define		HCBUFSTAT_ATL_DONE	(1 << 5)
#define	HCRDITL0LEN	0x2d
#define	HCRDITL1LEN	0x2e
#define	HCITLPORT	0x40
#define	HCATLPORT	0x41

/* PTD accessor macros. */
#define PTD_GET_COUNT(p)	(((p)->count & PTD_COUNT_MSK) >> 0)
#define PTD_COUNT(v)		(((v) << 0) & PTD_COUNT_MSK)
#define PTD_GET_TOGGLE(p)	(((p)->count & PTD_TOGGLE_MSK) >> 10)
#define PTD_TOGGLE(v)		(((v) << 10) & PTD_TOGGLE_MSK)
#define PTD_GET_ACTIVE(p)	(((p)->count & PTD_ACTIVE_MSK) >> 11)
#define PTD_ACTIVE(v)		(((v) << 11) & PTD_ACTIVE_MSK)
#define PTD_GET_CC(p)		(((p)->count & PTD_CC_MSK) >> 12)
#define PTD_CC(v)		(((v) << 12) & PTD_CC_MSK)
#define PTD_GET_MPS(p)		(((p)->mps & PTD_MPS_MSK) >> 0)
#define PTD_MPS(v)		(((v) << 0) & PTD_MPS_MSK)
#define PTD_GET_SPD(p)		(((p)->mps & PTD_SPD_MSK) >> 10)
#define PTD_SPD(v)		(((v) << 10) & PTD_SPD_MSK)
#define PTD_GET_LAST(p)		(((p)->mps & PTD_LAST_MSK) >> 11)
#define PTD_LAST(v)		(((v) << 11) & PTD_LAST_MSK)
#define PTD_GET_EP(p)		(((p)->mps & PTD_EP_MSK) >> 12)
#define PTD_EP(v)		(((v) << 12) & PTD_EP_MSK)
#define PTD_GET_LEN(p)		(((p)->len & PTD_LEN_MSK) >> 0)
#define PTD_LEN(v)		(((v) << 0) & PTD_LEN_MSK)
#define PTD_GET_DIR(p)		(((p)->len & PTD_DIR_MSK) >> 10)
#define PTD_DIR(v)		(((v) << 10) & PTD_DIR_MSK)
#define PTD_GET_B5_5(p)		(((p)->len & PTD_B5_5_MSK) >> 13)
#define PTD_B5_5(v)		(((v) << 13) & PTD_B5_5_MSK)
#define PTD_GET_FA(p)		(((p)->faddr & PTD_FA_MSK) >> 0)
#define PTD_FA(v)		(((v) << 0) & PTD_FA_MSK)
#define PTD_GET_FMT(p)		(((p)->faddr & PTD_FMT_MSK) >> 7)
#define PTD_FMT(v)		(((v) << 7) & PTD_FMT_MSK)

/*  Hardware transfer status codes -- CC from ptd->count */
#define TD_CC_NOERROR      0x00
#define TD_CC_CRC          0x01
#define TD_CC_BITSTUFFING  0x02
#define TD_CC_DATATOGGLEM  0x03
#define TD_CC_STALL        0x04
#define TD_DEVNOTRESP      0x05
#define TD_PIDCHECKFAIL    0x06
#define TD_UNEXPECTEDPID   0x07
#define TD_DATAOVERRUN     0x08
#define TD_DATAUNDERRUN    0x09
    /* 0x0A, 0x0B reserved for hardware */
#define TD_BUFFEROVERRUN   0x0C
#define TD_BUFFERUNDERRUN  0x0D
    /* 0x0E, 0x0F reserved for HCD */
#define TD_NOTACCESSED     0x0F

/* ------------------------------------------------------------------------- */

#define	LOG2_PERIODIC_SIZE	5	/* arbitrary; this matches OHCI */
#define	PERIODIC_SIZE		(1 << LOG2_PERIODIC_SIZE)

/* Philips transfer descriptor */
struct ptd {
	u16 count;
#define	PTD_COUNT_MSK	(0x3ff << 0)
#define	PTD_TOGGLE_MSK	(1 << 10)
#define	PTD_ACTIVE_MSK	(1 << 11)
#define	PTD_CC_MSK	(0xf << 12)
	u16 mps;
#define	PTD_MPS_MSK	(0x3ff << 0)
#define	PTD_SPD_MSK	(1 << 10)
#define	PTD_LAST_MSK	(1 << 11)
#define	PTD_EP_MSK	(0xf << 12)
	u16 len;
#define	PTD_LEN_MSK	(0x3ff << 0)
#define	PTD_DIR_MSK	(3 << 10)
#define	PTD_DIR_SETUP	(0)
#define	PTD_DIR_OUT	(1)
#define	PTD_DIR_IN	(2)
#define	PTD_B5_5_MSK	(1 << 13)
	u16 faddr;
#define	PTD_FA_MSK	(0x7f << 0)
#define	PTD_FMT_MSK	(1 << 7)
} __attribute__ ((packed, aligned(2)));

struct isp116x_ep {
	struct usb_device *udev;
	struct ptd ptd;

	u8 maxpacket;
	u8 epnum;
	u8 nextpid;

	u16 length;		/* of current packet */
	unsigned char *data;	/* to databuf */

	u16 error_count;
};

/* URB struct */
#define N_URB_TD		48
#define URB_DEL			1
typedef struct {
	struct isp116x_ep *ed;
	void *transfer_buffer;	/* (in) associated data buffer */
	int actual_length;	/* (return) actual transfer length */
	unsigned long pipe;	/* (in) pipe information */
#if 0
	int state;
#endif
} urb_priv_t;

struct isp116x_platform_data {
	/* Enable internal resistors on downstream ports */
	unsigned sel15Kres:1;
	/* On-chip overcurrent detection */
	unsigned oc_enable:1;
	/* Enable wakeup by devices on usb bus (e.g. wakeup
	   by attachment/detachment or by device activity
	   such as moving a mouse). When chosen, this option
	   prevents stopping internal clock, increasing
	   thereby power consumption in suspended state. */
	unsigned remote_wakeup_enable:1;
};

struct isp116x {
	u16 *addr_reg;
	u16 *data_reg;

	struct isp116x_platform_data *board;

	struct dentry *dentry;
	unsigned long stat1, stat2, stat4, stat8, stat16;

	/* Status flags */
	unsigned disabled:1;
	unsigned sleeping:1;

	/* Root hub registers */
	u32 rhdesca;
	u32 rhdescb;
	u32 rhstatus;
	u32 rhport[2];

	/* Schedule for the current frame */
	struct isp116x_ep *atl_active;
	int atl_buflen;
	int atl_bufshrt;
	int atl_last_dir;
	int atl_finishing;
};

/* ------------------------------------------------- */

/* Inter-io delay (ns). The chip is picky about access timings; it
 * expects at least:
 * 150ns delay between consecutive accesses to DATA_REG,
 * 300ns delay between access to ADDR_REG and DATA_REG
 * OE, WE MUST NOT be changed during these intervals
 */
#if defined(UDELAY)
#define	isp116x_delay(h,d)	udelay(d)
#else
#define	isp116x_delay(h,d)	do {} while (0)
#endif

static inline void isp116x_write_addr(struct isp116x *isp116x, unsigned reg)
{
	writew(reg & 0xff, isp116x->addr_reg);
	isp116x_delay(isp116x, UDELAY);
}

static inline void isp116x_write_data16(struct isp116x *isp116x, u16 val)
{
	writew(val, isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
}

static inline void isp116x_raw_write_data16(struct isp116x *isp116x, u16 val)
{
	__raw_writew(val, isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
}

static inline u16 isp116x_read_data16(struct isp116x *isp116x)
{
	u16 val;

	val = readw(isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
	return val;
}

static inline u16 isp116x_raw_read_data16(struct isp116x *isp116x)
{
	u16 val;

	val = __raw_readw(isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
	return val;
}

static inline void isp116x_write_data32(struct isp116x *isp116x, u32 val)
{
	writew(val & 0xffff, isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
	writew(val >> 16, isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
}

static inline u32 isp116x_read_data32(struct isp116x *isp116x)
{
	u32 val;

	val = (u32) readw(isp116x->data_reg);
	isp116x_delay(isp116x, UDELAY);
	val |= ((u32) readw(isp116x->data_reg)) << 16;
	isp116x_delay(isp116x, UDELAY);
	return val;
}

/* Let's keep register access functions out of line. Hint:
   we wait at least 150 ns at every access.
*/
static u16 isp116x_read_reg16(struct isp116x *isp116x, unsigned reg)
{
	isp116x_write_addr(isp116x, reg);
	return isp116x_read_data16(isp116x);
}

static u32 isp116x_read_reg32(struct isp116x *isp116x, unsigned reg)
{
	isp116x_write_addr(isp116x, reg);
	return isp116x_read_data32(isp116x);
}

static void isp116x_write_reg16(struct isp116x *isp116x, unsigned reg,
				unsigned val)
{
	isp116x_write_addr(isp116x, reg | ISP116x_WRITE_OFFSET);
	isp116x_write_data16(isp116x, (u16) (val & 0xffff));
}

static void isp116x_write_reg32(struct isp116x *isp116x, unsigned reg,
				unsigned val)
{
	isp116x_write_addr(isp116x, reg | ISP116x_WRITE_OFFSET);
	isp116x_write_data32(isp116x, (u32) val);
}

/* --- USB HUB constants (not OHCI-specific; see hub.h) -------------------- */

/* destination of request */
#define RH_INTERFACE               0x01
#define RH_ENDPOINT                0x02
#define RH_OTHER                   0x03

#define RH_CLASS                   0x20
#define RH_VENDOR                  0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS           0x0080
#define RH_CLEAR_FEATURE        0x0100
#define RH_SET_FEATURE          0x0300
#define RH_SET_ADDRESS          0x0500
#define RH_GET_DESCRIPTOR       0x0680
#define RH_SET_DESCRIPTOR       0x0700
#define RH_GET_CONFIGURATION    0x0880
#define RH_SET_CONFIGURATION    0x0900
#define RH_GET_STATE            0x0280
#define RH_GET_INTERFACE        0x0A80
#define RH_SET_INTERFACE        0x0B00
#define RH_SYNC_FRAME           0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP               0x2000

/* Hub port features */
#define RH_PORT_CONNECTION         0x00
#define RH_PORT_ENABLE             0x01
#define RH_PORT_SUSPEND            0x02
#define RH_PORT_OVER_CURRENT       0x03
#define RH_PORT_RESET              0x04
#define RH_PORT_POWER              0x08
#define RH_PORT_LOW_SPEED          0x09

#define RH_C_PORT_CONNECTION       0x10
#define RH_C_PORT_ENABLE           0x11
#define RH_C_PORT_SUSPEND          0x12
#define RH_C_PORT_OVER_CURRENT     0x13
#define RH_C_PORT_RESET            0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER       0x00
#define RH_C_HUB_OVER_CURRENT      0x01

#define RH_DEVICE_REMOTE_WAKEUP    0x00
#define RH_ENDPOINT_STALL          0x01

#define RH_ACK                     0x01
#define RH_REQ_ERR                 -1
#define RH_NACK                    0x00
