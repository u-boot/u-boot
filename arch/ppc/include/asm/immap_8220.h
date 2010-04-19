/*
 * MPC8220 Internal Memory Map
 * Copyright (c) 2004 TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * The Internal Memory Map of the 8220.
 *
 */
#ifndef __IMMAP_MPC8220__
#define __IMMAP_MPC8220__

/*
 * System configuration registers.
 */
typedef struct sys_conf {
    u16     mbar;       /* 0x00 */
    u16     res1;

    u16     res2;       /* 0x04 */
    u16     sdramds;

    u32     res3[6];    /* 0x08 */

    u32     cscfg[6];   /* 0x20 */

    u32     res4[2];    /* 0x38 */

    u8      res5[3];    /* 0x40 */
    u8      rstctrl;

    u8      res6[3];    /* 0x44 */
    u8      rststat;

    u32     res7[2];    /* 0x48 */

    u32     jtagid;     /* 0x50 */
} sysconf8220_t;


/*
 * Memory controller registers.
 */
typedef struct mem_ctlr {
    ushort  mode;           /* 0x100 */
    ushort  res1;
    u32     ctrl;           /* 0x104 */
    u32     cfg1;           /* 0x108 */
    u32     cfg2;           /* 0x10c */
} memctl8220_t;

/*
 * XLB Arbitration registers
 */
typedef struct xlb_arb
{
    uint    res1[16];       /* 0x200 */
    uint    config;         /* 0x240 */
    uint    version;        /* 0x244 */
    uint    status;         /* 0x248 */
    uint    intEnable;      /* 0x24c */
    uint    addrCap;        /* 0x250 */
    uint    busSigCap;      /* 0x254 */
    uint    addrTenTimeOut; /* 0x258 */
    uint    dataTenTimeOut; /* 0x25c */
    uint    busActTimeOut;  /* 0x260 */
    uint    mastPriEn;      /* 0x264 */
    uint    mastPriority;   /* 0x268 */
    uint    baseAddr;       /* 0x26c */
} xlbarb8220_t;

/*
 * Flexbus registers
 */
typedef struct flexbus
{
    ushort  csar0;          /* 0x00 */
    ushort  res1;
    uint    csmr0;          /* 0x04 */
    uint    cscr0;          /* 0x08 */

    ushort  csar1;          /* 0x0c */
    ushort  res2;
    uint    csmr1;          /* 0x10 */
    uint    cscr1;          /* 0x14 */

    ushort  csar2;          /* 0x18 */
    ushort  res3;
    uint    csmr2;          /* 0x1c */
    uint    cscr2;          /* 0x20 */

    ushort  csar3;          /* 0x24 */
    ushort  res4;
    uint    csmr3;          /* 0x28 */
    uint    cscr3;          /* 0x2c */

    ushort  csar4;          /* 0x30 */
    ushort  res5;
    uint    csmr4;          /* 0x34 */
    uint    cscr4;          /* 0x38 */

    ushort  csar5;          /* 0x3c */
    ushort  res6;
    uint    csmr5;          /* 0x40 */
    uint    cscr5;          /* 0x44 */
} flexbus8220_t;

/*
 * GPIO registers
 */
typedef struct gpio
{
    u32     out;        /* 0x00 */
    u32     obs;        /* 0x04 */
    u32     obc;        /* 0x08 */
    u32     obt;        /* 0x0c */
    u32     en;         /* 0x10 */
    u32     ebs;        /* 0x14 */
    u32     ebc;        /* 0x18 */
    u32     ebt;        /* 0x1c */
    u32     mc;         /* 0x20 */
    u32     st;         /* 0x24 */
    u32     intr;       /* 0x28 */
} gpio8220_t;

/*
 * General Purpose Timer registers
 */
typedef struct gptimer
{
    u8  OCPW;
    u8  OctIct;
    u8  Control;
    u8  Mode;

    u16 Prescl;  /* Prescale */
    u16 Count;   /* Count */

    u16 PwmWid;  /* PWM Width */
    u8  PwmOp;   /* Output Polarity */
    u8  PwmLd;   /* Immediate Update */

    u16 Capture; /* Capture internal counter */
    u8  OvfPin;  /* Ovf and Pin */
    u8  Int;     /* Interrupts */
} gptmr8220_t;

/*
 * PSC registers
 */
typedef struct psc
{
    u32 mr1_2;             /* 0x00 Mode reg 1 & 2 */
    u32 sr_csr;            /* 0x04 Status/Clock Select reg */
    u32 cr;                /* 0x08 Command reg */
    u8  xmitbuf[4];        /* 0x0c Receive/Transmit Buffer */
    u32 ipcr_acr;          /* 0x10 Input Port Change/Auxiliary Control reg */
    u32 isr_imr;           /* 0x14 Interrupt Status/Mask reg */
    u32 ctur;              /* 0x18 Counter Timer Upper reg */
    u32 ctlr;              /* 0x1c Counter Timer Lower reg */
    u32 rsvd1[4];          /* 0x20 ... 0x2c */
    u32 ivr;               /* 0x30 Interrupt Vector reg */
    u32 ipr;               /* 0x34 Input Port reg */
    u32 opsetr;            /* 0x38 Output Port Set reg */
    u32 opresetr;          /* 0x3c Output Port Reset reg */
    u32 sicr;              /* 0x40 PSC/IrDA control reg */
    u32 ircr1;             /* 0x44 IrDA control reg 1*/
    u32 ircr2;             /* 0x48 IrDA control reg 2*/
    u32 irsdr;             /* 0x4c IrDA SIR Divide reg */
    u32 irmdr;             /* 0x50 IrDA MIR Divide reg */
    u32 irfdr;             /* 0x54 PSC IrDA FIR Divide reg */
    u32 rfnum;             /* 0x58 RX-FIFO counter */
    u32 txnum;             /* 0x5c TX-FIFO counter */
    u32 rfdata;            /* 0x60 RX-FIFO data */
    u32 rfstat;            /* 0x64 RX-FIFO status */
    u32 rfcntl;            /* 0x68 RX-FIFO control */
    u32 rfalarm;           /* 0x6c RX-FIFO alarm */
    u32 rfrptr;            /* 0x70 RX-FIFO read pointer */
    u32 rfwptr;            /* 0x74 RX-FIFO write pointer */
    u32 rflfrptr;          /* 0x78 RX-FIFO last read frame pointer */
    u32 rflfwptr;          /* 0x7c RX-FIFO last write frame pointer */

    u32 tfdata;            /* 0x80 TX-FIFO data */
    u32 tfstat;            /* 0x84 TX-FIFO status */
    u32 tfcntl;            /* 0x88 TX-FIFO control */
    u32 tfalarm;           /* 0x8c TX-FIFO alarm */
    u32 tfrptr;            /* 0x90 TX-FIFO read pointer */
    u32 tfwptr;            /* 0x94 TX-FIFO write pointer */
    u32 tflfrptr;          /* 0x98 TX-FIFO last read frame pointer */
    u32 tflfwptr;          /* 0x9c TX-FIFO last write frame pointer */
} psc8220_t;

/*
 * Interrupt Controller registers
 */
typedef struct interrupt_controller {
} intctl8220_t;


/* Fast controllers
*/

/*
 * I2C registers
 */
typedef struct i2c
{
    u8   adr;            /* 0x00 */
    u8   res1[3];
    u8   fdr;            /* 0x04 */
    u8   res2[3];
    u8   cr;             /* 0x08 */
    u8   res3[3];
    u8   sr;             /* 0x0C */
    u8   res4[3];
    u8   dr;             /* 0x10 */
    u8   res5[3];
    u32  reserved0;      /* 0x14 */
    u32  reserved1;      /* 0x18 */
    u32  reserved2;      /* 0x1c */
    u8   icr;            /* 0x20 */
    u8   res6[3];
} i2c8220_t;

/*
 * Port Configuration Registers
 */
typedef struct pcfg
{
    uint    pcfg0;          /* 0x00 */
    uint    pcfg1;          /* 0x04 */
    uint    pcfg2;          /* 0x08 */
    uint    pcfg3;          /* 0x0c */
} pcfg8220_t;

/* ...and the whole thing wrapped up....
*/
typedef struct immap {
    sysconf8220_t   im_sysconf; /* System Configuration */
    memctl8220_t    im_memctl;  /* Memory Controller */
    xlbarb8220_t    im_xlbarb;  /* XLB Arbitration */
    psc8220_t       im_psc;     /* PSC controller */
    flexbus8220_t   im_fb;      /* FlexBus Controller */
    i2c8220_t       im_i2c;     /* I2C control/status */
    pcfg8220_t      im_pcfg;    /* Port configuration */
} immap_t;

#endif /* __IMMAP_MPC8220__ */
