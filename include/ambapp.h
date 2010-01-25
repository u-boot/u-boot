/* Interface for accessing Gaisler AMBA Plug&Play Bus.
 * The AHB bus can be interfaced with a simpler bus -
 * the APB bus, also freely available in GRLIB at
 * www.gaisler.com.
 *
 * (C) Copyright 2009, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __AMBAPP_H__
#define __AMBAPP_H__

#include <ambapp_ids.h>

#ifndef __ASSEMBLER__
/* Structures used to access Plug&Play information directly */
struct ambapp_pnp_ahb {
	const unsigned int	id;		/* VENDOR, DEVICE, VER, IRQ, */
	const unsigned int	custom[3];
	const unsigned int	mbar[4];	/* MASK, ADDRESS, TYPE,
						 * CACHABLE/PREFETCHABLE */
};

struct ambapp_pnp_apb {
	const unsigned int	id;		/* VENDOR, DEVICE, VER, IRQ, */
	const unsigned int	iobar;		/* MASK, ADDRESS, TYPE,
						 * CACHABLE/PREFETCHABLE */
};

/* AMBA Plug&Play AHB Masters & Slaves information locations
 * Max devices is 64 supported by HW, however often only 16
 * are used.
 */
struct ambapp_pnp_info {
	struct ambapp_pnp_ahb	masters[64];
	struct ambapp_pnp_ahb	slaves[63];
	const unsigned int	unused[4];
	const unsigned int	systemid[4];
};

/* Describes a AMBA PnP bus */
struct ambapp_bus {
	int		buses;		/* Number of buses */
	unsigned int	ioareas[6];	/* PnP I/O AREAs of AHB buses */
	unsigned int	freq;		/* Frequency of bus0 [Hz] */
};

/* Processor Local AMBA bus */
extern struct ambapp_bus ambapp_plb;

/* Get Bus frequency of a certain AMBA bus */
extern unsigned int ambapp_bus_freq(
	struct ambapp_bus *abus,
	int ahb_bus_index
	);

/* AMBA PnP information of a APB Device */
typedef struct {
	unsigned int vendor;
	unsigned int device;
	unsigned char irq;
	unsigned char ver;
	unsigned int address;
	unsigned int mask;
	int ahb_bus_index;
} ambapp_apbdev;

/* AMBA PnP information of a AHB Device */
typedef struct {
	unsigned int vendor;
	unsigned int device;
	unsigned char irq;
	unsigned char ver;
	unsigned int userdef[3];
	unsigned int address[4];
	unsigned int mask[4];
	int type[4];
	int ahb_bus_index;
} ambapp_ahbdev;

/* Scan AMBA Bus for AHB Bridges */
extern void ambapp_bus_init(
	unsigned int ioarea,
	unsigned int freq,
	struct ambapp_bus *abus);

/* Find APB Slave device by index using breath first search.
 *
 * When vendor and device are both set to zero, any device
 * with a non-zero device ID will match the search. It may be
 * useful when processing all devices on a AMBA bus.
 */
extern int ambapp_apb_find(
	struct ambapp_bus *abus,
	int vendor,
	int device,
	int index,
	ambapp_apbdev *dev
	);

/* Find AHB Master device by index using breath first search.
 *
 * When vendor and device are both set to zero, any device
 * with a non-zero device ID will match the search. It may be
 * useful when processing all devices on a AMBA bus.
 */
extern int ambapp_ahbmst_find(
	struct ambapp_bus *abus,
	int vendor,
	int device,
	int index,
	ambapp_ahbdev *dev
	);

/* Find AHB Slave device by index using breath first search.
 *
 * When vendor and device are both set to zero, any device
 * with a non-zero device ID will match the search. It may be
 * useful when processing all devices on a AMBA bus.
 */
extern int ambapp_ahbslv_find(
	struct ambapp_bus *abus,
	int vendor,
	int device,
	int index,
	ambapp_ahbdev *dev
	);

/* Return number of APB Slave devices of a certain ID (VENDOR:DEVICE)
 * zero is returned if no devices was found.
 */
extern int ambapp_apb_count(struct ambapp_bus *abus, int vendor, int device);

/* Return number of AHB Master devices of a certain ID (VENDOR:DEVICE)
 * zero is returned if no devices was found.
 */
extern int ambapp_ahbmst_count(struct ambapp_bus *abus, int vendor, int device);

/* Return number of AHB Slave devices of a certain ID (VENDOR:DEVICE)
 * zero is returned if no devices was found.
 */
extern int ambapp_ahbslv_count(struct ambapp_bus *abus, int vendor, int device);

#ifdef CONFIG_CMD_AMBAPP

/* AMBA Plug&Play relocation & initialization */
int ambapp_init_reloc(void);

/* AMBA Plug&Play Name of Vendors and devices */

/* Return name of device */
char *ambapp_device_id2str(int vendor, int id);

/* Return name of vendor */
char *ambapp_vendor_id2str(int vendor);

/* Return description of a device */
char *ambapp_device_id2desc(int vendor, int id);

#endif

#endif /* defined(__ASSEMBLER__) */

#define AMBA_DEFAULT_IOAREA 0xfff00000
#define AMBA_CONF_AREA 0xff000
#define AMBA_AHB_SLAVE_CONF_AREA 0x800

#define DEV_NONE	0
#define DEV_AHB_MST	1
#define DEV_AHB_SLV	2
#define DEV_APB_SLV	3

#define AMBA_TYPE_APBIO 0x1
#define AMBA_TYPE_MEM 0x2
#define AMBA_TYPE_AHBIO 0x3

/* ID layout for APB and AHB devices */
#define AMBA_PNP_ID(vendor, device) (((vendor)<<24) | ((device)<<12))

/* APB Slave PnP layout definitions */
#define AMBA_APB_ID_OFS		(0*4)
#define AMBA_APB_IOBAR_OFS	(1*4)
#define AMBA_APB_CONF_LENGH	(2*4)

/* AHB Master/Slave layout PnP definitions */
#define AMBA_AHB_ID_OFS		(0*4)
#define AMBA_AHB_CUSTOM0_OFS	(1*4)
#define AMBA_AHB_CUSTOM1_OFS	(2*4)
#define AMBA_AHB_CUSTOM2_OFS	(3*4)
#define AMBA_AHB_MBAR0_OFS	(4*4)
#define AMBA_AHB_MBAR1_OFS	(5*4)
#define AMBA_AHB_MBAR2_OFS	(6*4)
#define AMBA_AHB_MBAR3_OFS	(7*4)
#define AMBA_AHB_CONF_LENGH	(8*4)

/* Macros for extracting information from AMBA PnP information
 * registers.
 */

#define amba_vendor(x) (((x) >> 24) & 0xff)

#define amba_device(x) (((x) >> 12) & 0xfff)

#define amba_irq(conf) ((conf) & 0x1f)

#define amba_ver(conf) (((conf)>>5) & 0x1f)

#define amba_iobar_start(base, iobar) \
	((base) | ((((iobar) & 0xfff00000)>>12) & (((iobar) & 0xfff0)<<4)))

#define amba_membar_start(mbar) \
	(((mbar) & 0xfff00000) & (((mbar) & 0xfff0) << 16))

#define amba_membar_type(mbar) ((mbar) & 0xf)

#define amba_membar_mask(mbar) (((mbar) >> 4) & 0xfff)

#define amba_ahbio_adr(addr, base_ioarea) \
	((unsigned int)(base_ioarea) | ((addr) >> 12))

#define amba_apb_mask(iobar) ((~(amba_membar_mask(iobar)<<8) & 0x000fffff) + 1)

/*************************** AMBA Plug&Play device register MAPS *****************/

/*
 *  The following defines the bits in the LEON UART Status Registers.
 */

#define LEON_REG_UART_STATUS_DR   0x00000001	/* Data Ready */
#define LEON_REG_UART_STATUS_TSE  0x00000002	/* TX Send Register Empty */
#define LEON_REG_UART_STATUS_THE  0x00000004	/* TX Hold Register Empty */
#define LEON_REG_UART_STATUS_BR   0x00000008	/* Break Error */
#define LEON_REG_UART_STATUS_OE   0x00000010	/* RX Overrun Error */
#define LEON_REG_UART_STATUS_PE   0x00000020	/* RX Parity Error */
#define LEON_REG_UART_STATUS_FE   0x00000040	/* RX Framing Error */
#define LEON_REG_UART_STATUS_ERR  0x00000078	/* Error Mask */

/*
 *  The following defines the bits in the LEON UART Ctrl Registers.
 */

#define LEON_REG_UART_CTRL_RE     0x00000001	/* Receiver enable */
#define LEON_REG_UART_CTRL_TE     0x00000002	/* Transmitter enable */
#define LEON_REG_UART_CTRL_RI     0x00000004	/* Receiver interrupt enable */
#define LEON_REG_UART_CTRL_TI     0x00000008	/* Transmitter interrupt enable */
#define LEON_REG_UART_CTRL_PS     0x00000010	/* Parity select */
#define LEON_REG_UART_CTRL_PE     0x00000020	/* Parity enable */
#define LEON_REG_UART_CTRL_FL     0x00000040	/* Flow control enable */
#define LEON_REG_UART_CTRL_LB     0x00000080	/* Loop Back enable */
#define LEON_REG_UART_CTRL_DBG    (1<<11)	/* Debug Bit used by GRMON */

#define LEON3_GPTIMER_EN 1
#define LEON3_GPTIMER_RL 2
#define LEON3_GPTIMER_LD 4
#define LEON3_GPTIMER_IRQEN 8

/*
 *  The following defines the bits in the LEON PS/2 Status Registers.
 */

#define LEON_REG_PS2_STATUS_DR   0x00000001	/* Data Ready */
#define LEON_REG_PS2_STATUS_PE   0x00000002	/* Parity error */
#define LEON_REG_PS2_STATUS_FE   0x00000004	/* Framing error */
#define LEON_REG_PS2_STATUS_KI   0x00000008	/* Keyboard inhibit */

/*
 *  The following defines the bits in the LEON PS/2 Ctrl Registers.
 */

#define LEON_REG_PS2_CTRL_RE     0x00000001	/* Receiver enable */
#define LEON_REG_PS2_CTRL_TE     0x00000002	/* Transmitter enable */
#define LEON_REG_PS2_CTRL_RI     0x00000004	/* Keyboard receive interrupt  */
#define LEON_REG_PS2_CTRL_TI     0x00000008	/* Keyboard transmit interrupt */

#ifndef __ASSEMBLER__

typedef struct {
	volatile unsigned int ilevel;
	volatile unsigned int ipend;
	volatile unsigned int iforce;
	volatile unsigned int iclear;
	volatile unsigned int mstatus;
	volatile unsigned int notused[11];
	volatile unsigned int cpu_mask[16];
	volatile unsigned int cpu_force[16];
} ambapp_dev_irqmp;

typedef struct {
	volatile unsigned int data;
	volatile unsigned int status;
	volatile unsigned int ctrl;
	volatile unsigned int scaler;
} ambapp_dev_apbuart;

typedef struct {
	volatile unsigned int val;
	volatile unsigned int rld;
	volatile unsigned int ctrl;
	volatile unsigned int unused;
} ambapp_dev_gptimer_element;

#define LEON3_GPTIMER_CTRL_EN	0x1	/* Timer enable */
#define LEON3_GPTIMER_CTRL_RS	0x2	/* Timer reStart  */
#define LEON3_GPTIMER_CTRL_LD	0x4	/* Timer reLoad */
#define LEON3_GPTIMER_CTRL_IE	0x8	/* interrupt enable */
#define LEON3_GPTIMER_CTRL_IP	0x10	/* interrupt flag/pending */
#define LEON3_GPTIMER_CTRL_CH	0x20	/* Chain with previous timer */

typedef struct {
	volatile unsigned int scalar;
	volatile unsigned int scalar_reload;
	volatile unsigned int config;
	volatile unsigned int unused;
	volatile ambapp_dev_gptimer_element e[8];
} ambapp_dev_gptimer;

typedef struct {
	volatile unsigned int iodata;
	volatile unsigned int ioout;
	volatile unsigned int iodir;
	volatile unsigned int irqmask;
	volatile unsigned int irqpol;
	volatile unsigned int irqedge;
} ambapp_dev_ioport;

typedef struct {
	volatile unsigned int write;
	volatile unsigned int dummy;
	volatile unsigned int txcolor;
	volatile unsigned int bgcolor;
} ambapp_dev_textvga;

typedef struct {
	volatile unsigned int data;
	volatile unsigned int status;
	volatile unsigned int ctrl;
} ambapp_dev_apbps2;

typedef struct {
	unsigned int mcfg1, mcfg2, mcfg3;
} ambapp_dev_mctrl;

typedef struct {
	unsigned int sdcfg;
} ambapp_dev_sdctrl;

typedef struct {
	unsigned int cfg1;
	unsigned int cfg2;
	unsigned int cfg3;
} ambapp_dev_ddr2spa;

typedef struct {
	unsigned int ctrl;
	unsigned int cfg;
} ambapp_dev_ddrspa;

#endif

#endif
