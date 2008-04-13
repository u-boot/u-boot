/* Interface for accessing Gaisler AMBA Plug&Play Bus.
 * The AHB bus can be interfaced with a simpler bus -
 * the APB bus, also freely available in GRLIB at
 * www.gaisler.com.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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

#ifndef __AMBAPP_H__
#define __AMBAPP_H__

/* Default location of Plug&Play info
 * normally 0xfffff000 for AHB masters
 * and 0xfffff800 for AHB slaves.
 * Normally no need to change this.
 */
#define LEON3_IO_AREA 0xfff00000
#define LEON3_CONF_AREA  0xff000
#define LEON3_AHB_SLAVE_CONF_AREA (1 << 11)

/* Max devices this software will support */
#define LEON3_AHB_MASTERS 16
#define LEON3_AHB_SLAVES 16
/*#define LEON3_APB_MASTERS 1*/ /* Number of APB buses that has Plug&Play */
#define LEON3_APB_SLAVES 16	/* Total number of APB slaves per APB bus */

/* Vendor codes */
#define VENDOR_GAISLER       1
#define VENDOR_PENDER        2
#define VENDOR_ESA           4
#define VENDOR_ASTRIUM       6
#define VENDOR_OPENCHIP      7
#define VENDOR_OPENCORES     8
#define VENDOR_CONTRIB       9
#define VENDOR_EONIC         11
#define VENDOR_RADIONOR      15
#define VENDOR_GLEICHMANN    16
#define VENDOR_MENTA         17
#define VENDOR_SUN           19
#define VENDOR_EMBEDDIT      234
#define VENDOR_CAL           202

/* Gaisler Research device id's */
#define GAISLER_LEON3    0x003
#define GAISLER_LEON3DSU 0x004
#define GAISLER_ETHAHB   0x005
#define GAISLER_APBMST   0x006
#define GAISLER_AHBUART  0x007
#define GAISLER_SRCTRL   0x008
#define GAISLER_SDCTRL   0x009
#define GAISLER_APBUART  0x00C
#define GAISLER_IRQMP    0x00D
#define GAISLER_AHBRAM   0x00E
#define GAISLER_GPTIMER  0x011
#define GAISLER_PCITRG   0x012
#define GAISLER_PCISBRG  0x013
#define GAISLER_PCIFBRG  0x014
#define GAISLER_PCITRACE 0x015
#define GAISLER_PCIDMA   0x016
#define GAISLER_AHBTRACE 0x017
#define GAISLER_ETHDSU   0x018
#define GAISLER_PIOPORT  0x01A
#define GAISLER_AHBJTAG  0x01c
#define GAISLER_SPW      0x01f
#define GAISLER_ATACTRL  0x024
#define GAISLER_VGA      0x061
#define GAISLER_KBD      0X060
#define GAISLER_ETHMAC   0x01D
#define GAISLER_DDRSPA   0x025
#define GAISLER_EHCI     0x026
#define GAISLER_UHCI     0x027
#define GAISLER_SPW2     0x029
#define GAISLER_DDR2SPA  0x02E
#define GAISLER_AHBSTAT  0x052
#define GAISLER_FTMCTRL  0x054

#define GAISLER_L2TIME   0xffd	/* internal device: leon2 timer */
#define GAISLER_L2C      0xffe	/* internal device: leon2compat */
#define GAISLER_PLUGPLAY 0xfff	/* internal device: plug & play configarea */

/* European Space Agency device id's */
#define ESA_LEON2        0x2
#define ESA_MCTRL        0xF

/* Opencores device id's */
#define OPENCORES_PCIBR  0x4
#define OPENCORES_ETHMAC 0x5

/* Vendor codes */

/*
 *
 * Macros for manipulating Configuration registers
 *
 */

#define amba_vendor(x) (((x) >> 24) & 0xff)

#define amba_device(x) (((x) >> 12) & 0xfff)

#define amba_membar_start(mbar) \
 (((mbar) & 0xfff00000) & (((mbar) & 0xfff0) << 16))

#define amba_iobar_start(base, iobar) \
 ((base) | ((((iobar) & 0xfff00000)>>12) & (((iobar) & 0xfff0)<<4)) )

#define amba_irq(conf) ((conf) & 0xf)

#define amba_ver(conf) (((conf)>>5) & 0x1f)

#define amba_membar_type(mbar) ((mbar) & 0xf)

#define amba_membar_mask(mbar) (((mbar)>>4) & 0xfff)

#define AMBA_TYPE_APBIO 0x1
#define AMBA_TYPE_MEM   0x2
#define AMBA_TYPE_AHBIO 0x3

#define AMBA_TYPE_AHBIO_ADDR(addr) (LEON3_IO_AREA | ((addr) >> 12))

#ifndef __ASSEMBLER__

#ifdef CONFIG_CMD_AMBAPP

/* AMBA Plug&Play relocation & initialization */
int ambapp_init_reloc(void);

/* AMBA Plug&Play Name of Vendors and devices */

/* Return name of device */
char *ambapp_device_id2str(int vendor, int id);

/* Return name of vendor */
char *ambapp_vendor_id2str(int vendor);
#endif

/*
 *  Types and structure used for AMBA Plug & Play bus scanning
 */

/* AMBA Plug&Play AHB information layout */
typedef struct {
	unsigned int conf;
	unsigned int userdef[3];
	unsigned int bars[4];
} ahbctrl_pp_dev;

/* Prototypes for scanning AMBA Plug&Play bus for AMBA
 *  i)   AHB Masters
 *  ii)  AHB Slaves
 *  iii) APB Slaves (APB MST is a AHB Slave)
 */

typedef struct {
	unsigned char irq;
	unsigned char ver;
	unsigned int address;
} ambapp_apbdev;

typedef struct {
	unsigned char irq;
	unsigned char ver;
	unsigned int userdef[3];
	unsigned int address[4];
} ambapp_ahbdev;

/* AMBA Plug&Play AHB Masters & Slaves information locations
 * Max devices is 64 supported by HW, however often only 8
 * are used.
 */
typedef struct {
	ahbctrl_pp_dev masters[64];
	ahbctrl_pp_dev slaves[64];
} ahbctrl_info;

/* AMBA Plug&Play AHB information layout */
typedef struct {
	unsigned int conf;
	unsigned int bar;
} apbctrl_pp_dev;

/* All functions return the number of found devices
 * 0 = no devices found
 */

/****************************** APB SLAVES ******************************/
int ambapp_apb_count(unsigned int vendor, unsigned int driver);

int ambapp_apb_first(unsigned int vendor,
		     unsigned int driver, ambapp_apbdev * dev);

int ambapp_apb_next(unsigned int vendor,
		    unsigned int driver, ambapp_apbdev * dev, int index);

int ambapp_apbs_first(unsigned int vendor,
		      unsigned int driver, ambapp_apbdev * dev, int max_cnt);

/****************************** AHB MASTERS ******************************/
int ambapp_ahbmst_count(unsigned int vendor, unsigned int driver);

int ambapp_ahbmst_first(unsigned int vendor,
			unsigned int driver, ambapp_ahbdev * dev);

int ambapp_ahbmst_next(unsigned int vendor,
		       unsigned int driver, ambapp_ahbdev * dev, int index);

int ambapp_ahbmsts_first(unsigned int vendor,
			 unsigned int driver, ambapp_ahbdev * dev, int max_cnt);

/****************************** AHB SLAVES ******************************/
int ambapp_ahbslv_count(unsigned int vendor, unsigned int driver);

int ambapp_ahbslv_first(unsigned int vendor,
			unsigned int driver, ambapp_ahbdev * dev);

int ambapp_ahbslv_next(unsigned int vendor,
		       unsigned int driver, ambapp_ahbdev * dev, int index);

int ambapp_ahbslvs_first(unsigned int vendor,
			 unsigned int driver, ambapp_ahbdev * dev, int max_cnt);

/*************************** AHB/APB only regs functions *************************
 * During start up, no memory is available we can use the simplified functions
 * to get to the memory controller.
 *
 * Functions uses no stack/memory, only registers.
 */
unsigned int ambapp_apb_next_nomem(register unsigned int vendor,	/* Plug&Play Vendor ID */
				   register unsigned int driver,	/* Plug&Play Device ID */
				   register int index);

ahbctrl_pp_dev *ambapp_ahb_next_nomem(register unsigned int vendor,	/* Plug&Play Vendor ID */
				      register unsigned int driver,	/* Plug&Play Device ID */
				      register unsigned int opts,	/* scan for AHB 1=slave, 0=masters */
				      register int index);

unsigned int ambapp_ahb_get_info(ahbctrl_pp_dev * ahb, int info);

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
