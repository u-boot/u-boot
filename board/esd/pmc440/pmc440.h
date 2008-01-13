/*
 * (C) Copyright 2007
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __PMC440_H__
#define __PMC440_H__


/*-----------------------------------------------------------------------
 * GPIOs
 */
#define GPIO1_INTA_FAKE           (0x80000000 >> (45-32)) /* GPIO45 OD */
#define GPIO1_NONMONARCH          (0x80000000 >> (63-32)) /* GPIO63 I */
#define GPIO1_PPC_EREADY          (0x80000000 >> (62-32)) /* GPIO62 I/O */
#define GPIO1_M66EN               (0x80000000 >> (61-32)) /* GPIO61 I */
#define GPIO1_POST_N              (0x80000000 >> (60-32)) /* GPIO60 O */
#define GPIO1_IOEN_N              (0x80000000 >> (50-32)) /* GPIO50 O */
#define GPIO1_HWID_MASK           (0xf0000000 >> (56-32)) /* GPIO56..59 I */

#define GPIO1_USB_PWR_N           (0x80000000 >> (32-32)) /* GPIO32 I */
#define GPIO0_LED_RUN_N           (0x80000000 >> 30)      /* GPIO30 O */
#define GPIO0_EP_EEP              (0x80000000 >> 23)      /* GPIO23 O */
#define GPIO0_USB_ID              (0x80000000 >> 21)      /* GPIO21 I */
#define GPIO0_USB_PRSNT           (0x80000000 >> 20)      /* GPIO20 I */
#define GPIO0_SELF_RST            (0x80000000 >> 6)       /* GPIO6  OD */

/* FPGA programming pin configuration */
#define GPIO1_FPGA_PRG            (0x80000000 >> (53-32)) /* FPGA program pin (ppc output) */
#define GPIO1_FPGA_CLK            (0x80000000 >> (51-32)) /* FPGA clk pin (ppc output)     */
#define GPIO1_FPGA_DATA           (0x80000000 >> (52-32)) /* FPGA data pin (ppc output)    */
#define GPIO1_FPGA_DONE           (0x80000000 >> (55-32)) /* FPGA done pin (ppc input)     */
#define GPIO1_FPGA_INIT           (0x80000000 >> (54-32)) /* FPGA init pin (ppc input)     */
#define GPIO0_FPGA_FORCEINIT      (0x80000000 >> 27)      /* low: force INIT# low */

/*-----------------------------------------------------------------------
 * FPGA interface
 */
#define FPGA_BA CFG_FPGA_BASE0
#define FPGA_OUT32(p,v) out_be32(((void*)(p)), (v))
#define FPGA_IN32(p) in_be32((void*)(p))
#define FPGA_SETBITS(p,v) out_be32(((void*)(p)), in_be32((void*)(p)) | (v))
#define FPGA_CLRBITS(p,v) out_be32(((void*)(p)), in_be32((void*)(p)) & ~(v))

struct pmc440_fifo_s {
	u32 data;
	u32 ctrl;
};

/* fifo ctrl register */
#define FIFO_IE              (1 << 15)
#define FIFO_OVERFLOW        (1 << 10)
#define FIFO_EMPTY           (1 <<  9)
#define FIFO_FULL            (1 <<  8)
#define FIFO_LEVEL_MASK      0x000000ff

#define FIFO_COUNT           4

struct pmc440_fpga_s {
	u32 ctrla;
	u32 status;
	u32 ctrlb;
	u32 pad1[0x40 / sizeof(u32) - 3];
	u32 irig_time;                  /* offset: 0x0040 */
	u32 irig_tod;
	u32 irig_cf;
	u32 pad2;
	u32 irig_rx_time;               /* offset: 0x0050 */
	u32 pad3[3];
	u32 hostctrl;                   /* offset: 0x0060 */
	u32 pad4[0x20 / sizeof(u32) - 1];
	struct pmc440_fifo_s fifo[FIFO_COUNT]; /* 0x0080..0x009f */
};

typedef struct pmc440_fpga_s pmc440_fpga_t;

/* ctrl register */
#define CTRL_HOST_IE         (1 <<  8)

/* outputs */
#define RESET_EN    (1 << 31)
#define CLOCK_EN    (1 << 30)
#define RESET_OUT   (1 << 19)
#define CLOCK_OUT   (1 << 22)
#define RESET_OUT   (1 << 19)
#define IRIGB_R_OUT (1 << 14)


/* status register */
#define STATUS_VERSION_SHIFT 24
#define STATUS_VERSION_MASK  0xff000000
#define STATUS_HWREV_SHIFT   20
#define STATUS_HWREV_MASK    0x00f00000

#define STATUS_CAN_ISF       (1 << 11)
#define STATUS_CSTM_ISF      (1 << 10)
#define STATUS_FIFO_ISF      (1 <<  9)
#define STATUS_HOST_ISF      (1 <<  8)


/* inputs */
#define RESET_IN    (1 << 0)
#define CLOCK_IN    (1 << 1)
#define IRIGB_R_IN  (1 << 5)


/* hostctrl register */
#define HOSTCTRL_PMCRSTOUT_GATE (1 <<  17)
#define HOSTCTRL_PMCRSTOUT_FLAG (1 <<  16)
#define HOSTCTRL_CSTM1IE_GATE (1 <<  7)
#define HOSTCTRL_CSTM1IW_FLAG (1 <<  6)
#define HOSTCTRL_CSTM0IE_GATE (1 <<  5)
#define HOSTCTRL_CSTM0IW_FLAG (1 <<  4)
#define HOSTCTRL_FIFOIE_GATE (1 <<  3)
#define HOSTCTRL_FIFOIE_FLAG (1 <<  2)
#define HOSTCTRL_HCINT_GATE  (1 <<  1)
#define HOSTCTRL_HCINT_FLAG  (1 <<  0)

#define NGCC_CTRL_BASE         (CFG_FPGA_BASE0 + 0x80000)
#define NGCC_CTRL_FPGARST_N    (1 <<  2)

/*-----------------------------------------------------------------------
 * FPGA to PPC interrupt
 */
#define IRQ0_FPGA            (32+28) /* UIC1 - FPGA internal */
#define IRQ1_FPGA            (32+30) /* UIC1 - custom module */
#define IRQ2_FPGA            (64+ 3) /* UIC2 - custom module / CAN */
#define IRQ_ETH0             (64+ 4) /* UIC2 */
#define IRQ_ETH1             (   27) /* UIC0 */
#define IRQ_RTC              (64+ 0) /* UIC2 */
#define IRQ_PCIA             (64+ 1) /* UIC2 */
#define IRQ_PCIB             (32+18) /* UIC1 */
#define IRQ_PCIC             (32+19) /* UIC1 */
#define IRQ_PCID             (32+20) /* UIC1 */

#endif /* __PMC440_H__ */
