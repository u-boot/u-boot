/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */
#ifndef _CLOCKS_OMAP4_H_
#define _CLOCKS_OMAP4_H_

#define LDELAY		1000000

#include <asm/ti-common/omap_clock.h>

/* ALTCLKSRC */
#define ALTCLKSRC_MODE_ACTIVE		1
#define ALTCLKSRC_MODE_MASK		3
#define ALTCLKSRC_ENABLE_INT_MASK	4
#define ALTCLKSRC_ENABLE_EXT_MASK	8

/* CM_COREAON_USB_PHY_CORE_CLKCTRL */
#define USBPHY_CORE_CLKCTRL_OPTFCLKEN_CLK32K	BIT(8)

/* CM_L3INIT_USBPHY_CLKCTRL */
#define USBPHY_CLKCTRL_OPTFCLKEN_PHY_48M_MASK	BIT(8)

/* TWL6030 SMPS */
#define SMPS_REG_ADDR_VCORE1	0x55
#define SMPS_REG_ADDR_VCORE2	0x5B
#define SMPS_REG_ADDR_VCORE3	0x61
/* TWL6032 SMPS */
#define SMPS_REG_ADDR_SMPS1	0x55
#define SMPS_REG_ADDR_SMPS2	0x5B
#define SMPS_REG_ADDR_SMPS5	0x49

/* PMIC */
#define SMPS_I2C_SLAVE_ADDR	0x12

/* Clock Defines */
#define V_OSCK			38400000	/* Clock output from T2 */
#define V_SCLK                   V_OSCK

struct omap4_scrm_regs {
	u32 revision;           /* 0x0000 */
	u32 pad00[63];
	u32 clksetuptime;       /* 0x0100 */
	u32 pmicsetuptime;      /* 0x0104 */
	u32 pad01[2];
	u32 altclksrc;          /* 0x0110 */
	u32 pad02[2];
	u32 c2cclkm;            /* 0x011c */
	u32 pad03[56];
	u32 extclkreq;          /* 0x0200 */
	u32 accclkreq;          /* 0x0204 */
	u32 pwrreq;             /* 0x0208 */
	u32 pad04[1];
	u32 auxclkreq0;         /* 0x0210 */
	u32 auxclkreq1;         /* 0x0214 */
	u32 auxclkreq2;         /* 0x0218 */
	u32 auxclkreq3;         /* 0x021c */
	u32 auxclkreq4;         /* 0x0220 */
	u32 auxclkreq5;         /* 0x0224 */
	u32 pad05[3];
	u32 c2cclkreq;          /* 0x0234 */
	u32 pad06[54];
	u32 auxclk0;            /* 0x0310 */
	u32 auxclk1;            /* 0x0314 */
	u32 auxclk2;            /* 0x0318 */
	u32 auxclk3;            /* 0x031c */
	u32 auxclk4;            /* 0x0320 */
	u32 auxclk5;            /* 0x0324 */
	u32 pad07[54];
	u32 rsttime_reg;        /* 0x0400 */
	u32 pad08[6];
	u32 c2crstctrl;         /* 0x041c */
	u32 extpwronrstctrl;    /* 0x0420 */
	u32 pad09[59];
	u32 extwarmrstst_reg;   /* 0x0510 */
	u32 apewarmrstst_reg;   /* 0x0514 */
	u32 pad10[1];
	u32 c2cwarmrstst_reg;   /* 0x051C */
};

#endif /* _CLOCKS_OMAP4_H_ */
