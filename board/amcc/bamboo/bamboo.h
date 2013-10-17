/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*----------------------------------------------------------------------------+
  | FPGA registers and bit definitions
  +----------------------------------------------------------------------------*/
/*
 * PowerPC 440EP Board FPGA is reached with physical address 0x80001FF0.
 * TLB initialization makes it correspond to logical address 0x80001FF0.
 * => Done init_chip.s in bootlib
 */
#define FPGA_BASE_ADDR	0x80002000

/*----------------------------------------------------------------------------+
  | Board Jumpers Setting Register
  |   Board Settings provided by jumpers
  +----------------------------------------------------------------------------*/
#define FPGA_SETTING_REG	    (FPGA_BASE_ADDR+0x3)
/* Boot from small flash */
#define	    FPGA_SET_REG_BOOT_SMALL_FLASH	    0x80
/* Operational Flash versus SRAM position in Memory Map */
#define	    FPGA_SET_REG_OP_CODE_SRAM_SEL_MASK	    0x40
#define	     FPGA_SET_REG_OP_CODE_FLASH_ABOVE	     0x40
#define	     FPGA_SET_REG_SRAM_ABOVE		     0x00
/* Boot From NAND Flash */
#define	    FPGA_SET_REG_BOOT_NAND_FLASH_MASK	    0x40
#define	    FPGA_SET_REG_BOOT_NAND_FLASH_SELECT	     0x00
/* On Board PCI Arbiter Select */
#define	    FPGA_SET_REG_PCI_EXT_ARBITER_SEL_MASK   0x10
#define	    FPGA_SET_REG_PCI_EXT_ARBITER_SEL	    0x00

/*----------------------------------------------------------------------------+
  | Functions Selection Register 1
  +----------------------------------------------------------------------------*/
#define FPGA_SELECTION_1_REG	    (FPGA_BASE_ADDR+0x4)
#define	    FPGA_SEL_1_REG_PHY_MASK	    0xE0
#define	    FPGA_SEL_1_REG_MII		    0x80
#define	    FPGA_SEL_1_REG_RMII		    0x40
#define	    FPGA_SEL_1_REG_SMII		    0x20
#define	    FPGA_SEL_1_REG_USB2_DEV_SEL	    0x10	   /* USB2 Device Selection */
#define	    FPGA_SEL_1_REG_USB2_HOST_SEL    0x08	   /* USB2 Host Selection */
#define	    FPGA_SEL_1_REG_NF_SELEC_MASK    0x07	   /* NF Selection Mask */
#define	    FPGA_SEL_1_REG_NF0_SEL_BY_NFCS1 0x04	   /* NF0 Selected by NF_CS1 */
#define	    FPGA_SEL_1_REG_NF1_SEL_BY_NFCS2 0x02	   /* NF1 Selected by NF_CS2 */
#define	    FPGA_SEL_1_REG_NF1_SEL_BY_NFCS3 0x01	   /* NF1 Selected by NF_CS3 */

/*----------------------------------------------------------------------------+
  | Functions Selection Register 2
  +----------------------------------------------------------------------------*/
#define FPGA_SELECTION_2_REG	    (FPGA_BASE_ADDR+0x5)
#define	    FPGA_SEL2_REG_IIC1_SCP_SEL_MASK 0x80	   /* IIC1 / SCP Selection */
#define	    FPGA_SEL2_REG_SEL_FRAM	    0x80	   /* FRAM on IIC1 bus selected - SCP Select */
#define	    FPGA_SEL2_REG_SEL_SCP	    0x80	   /* Identical to SCP Selection */
#define	    FPGA_SEL2_REG_SEL_IIC1	    0x00	   /* IIC1 Selection - Default Value */
#define	    FPGA_SEL2_REG_SEL_DMA_A_B	    0x40	   /* DMA A & B channels selected */
#define	    FPGA_SEL2_REG_SEL_DMA_C_D	    0x20	   /* DMA C & D channels selected */
#define	    FPGA_SEL2_REG_DMA_EOT_TC_3_SEL  0x10	   /* 0 = EOT - input to 440EP */
							   /* 1 = TC - output from 440EP */
#define	    FPGA_SEL2_REG_DMA_EOT_TC_2_SEL  0x08	   /* 0 = EOT (input to 440EP) */
							   /* 1 = TC (output from 440EP) */
#define	    FPGA_SEL2_REG_SEL_GPIO_1	    0x04	   /* EBC_GPIO & USB2_GPIO selected */
#define	    FPGA_SEL2_REG_SEL_GPIO_2	    0x02	   /* Ether._GPIO & UART_GPIO selected */
#define	    FPGA_SEL2_REG_SEL_GPIO_3	    0x01	   /* DMA_GPIO & Trace_GPIO selected */

/*----------------------------------------------------------------------------+
  | Functions Selection Register 3
  +----------------------------------------------------------------------------*/
#define FPGA_SELECTION_3_REG	    (FPGA_BASE_ADDR+0x6)
#define	    FPGA_SEL3_REG_EXP_SLOT_EN		    0x80    /* Expansion Slot enabled */
#define	    FPGA_SEL3_REG_SEL_UART_CONFIG_MASK	    0x70
#define	    FPGA_SEL3_REG_SEL_UART_CONFIG1	    0x40    /* one 8_pin UART */
#define	    FPGA_SEL3_REG_SEL_UART_CONFIG2	    0x20    /* two 4_pin UARTs */
#define	    FPGA_SEL3_REG_SEL_UART_CONFIG3	    0x10    /* one 4_pin & two 2_pin UARTs */
#define	    FPGA_SEL3_REG_SEL_UART_CONFIG4	    0x08    /* four 2_pin UARTs */
#define	    FPGA_SEL3_REG_DTR_DSR_MODE_4_PIN_UART   0x00    /* DTR/DSR mode for 4_pin_UART */
#define	    FPGA_SEL3_REG_RTS_CTS_MODE_4_PIN_UART   0x04    /* RTS/CTS mode for 4_pin_UART */

/*----------------------------------------------------------------------------+
  | Soft Reset Register
  +----------------------------------------------------------------------------*/
#define FPGA_RESET_REG		    (FPGA_BASE_ADDR+0x7)
#define	    FPGA_RESET_REG_RESET_USB20_DEV	    0x80    /* Hard Reset of the GT3200 */
#define	    FPGA_RESET_REG_RESET_DISPLAY	    0x40    /* Hard Reset on Display Device */
#define	    FPGA_RESET_REG_STATUS_LED_0		    0x08    /* 1 = Led On */
#define	    FPGA_RESET_REG_STATUS_LED_1		    0x04    /* 1 = Led On */
#define	    FPGA_RESET_REG_STATUS_LED_2		    0x02    /* 1 = Led On */
#define	    FPGA_RESET_REG_STATUS_LED_3		    0x01    /* 1 = Led On */


/*----------------------------------------------------------------------------+
| SDR Configuration registers
+----------------------------------------------------------------------------*/
#define	  SDR0_SDSTP1_EBC_ROM_BS_MASK  0x00006000  /* EBC Boot Size Mask */
#define	  SDR0_SDSTP1_EBC_ROM_BS_32BIT 0x00004000    /* EBC 32 bits */
#define	  SDR0_SDSTP1_EBC_ROM_BS_16BIT 0x00002000    /* EBC 16 Bits */
#define	  SDR0_SDSTP1_EBC_ROM_BS_8BIT  0x00000000    /* EBC  8 Bits */

#define	  SDR0_SDSTP1_BOOT_SEL_MASK    0x00001800   /* Boot device Selection Mask */
#define	  SDR0_SDSTP1_BOOT_SEL_EBC     0x00000000     /* EBC */
#define	  SDR0_SDSTP1_BOOT_SEL_PCI     0x00000800     /* PCI */
#define	  SDR0_SDSTP1_BOOT_SEL_NDFC    0x00001000     /* NDFC */

/* Serial Device Enabled - Addr = 0xA8 */
#define SDR0_PSTRP0_BOOTSTRAP_IIC_A8_EN SDR0_PSTRP0_BOOTSTRAP_SETTINGS5
/* Serial Device Enabled - Addr = 0xA4 */
#define SDR0_PSTRP0_BOOTSTRAP_IIC_A4_EN SDR0_PSTRP0_BOOTSTRAP_SETTINGS7

/* Pin Straps Reg */
#define SDR0_PSTRP0		     0x0040
#define SDR0_PSTRP0_BOOTSTRAP_MASK	0xE0000000  /* Strap Bits */

#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS0 0x00000000  /* Default strap settings 0 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS1 0x20000000  /* Default strap settings 1 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS2 0x40000000  /* Default strap settings 2 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS3 0x60000000  /* Default strap settings 3 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS4 0x80000000  /* Default strap settings 4 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS5 0xA0000000  /* Default strap settings 5 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS6 0xC0000000  /* Default strap settings 6 */
#define SDR0_PSTRP0_BOOTSTRAP_SETTINGS7 0xE0000000  /* Default strap settings 7 */

/*----------------------------------------------------------------------------+
| EBC Configuration Register - EBC0_CFG
+----------------------------------------------------------------------------*/
/* External Bus Three-State Control */
#define EBC0_CFG_EBTC_DRIVEN	    0x80000000
/* Device-Paced Time-out Disable */
#define EBC0_CFG_PTD_ENABLED	    0x00000000
/* Ready Timeout Count */
#define EBC0_CFG_RTC_MASK	    0x38000000
#define EBC0_CFG_RTC_16PERCLK	    0x00000000
#define EBC0_CFG_RTC_32PERCLK	    0x08000000
#define EBC0_CFG_RTC_64PERCLK	    0x10000000
#define EBC0_CFG_RTC_128PERCLK	    0x18000000
#define EBC0_CFG_RTC_256PERCLK	    0x20000000
#define EBC0_CFG_RTC_512PERCLK	    0x28000000
#define EBC0_CFG_RTC_1024PERCLK	    0x30000000
#define EBC0_CFG_RTC_2048PERCLK	    0x38000000
/* External Master Priority Low */
#define EBC0_CFG_EMPL_LOW	    0x00000000
#define EBC0_CFG_EMPL_MEDIUM_LOW    0x02000000
#define EBC0_CFG_EMPL_MEDIUM_HIGH   0x04000000
#define EBC0_CFG_EMPL_HIGH	    0x06000000
/* External Master Priority High */
#define EBC0_CFG_EMPH_LOW	    0x00000000
#define EBC0_CFG_EMPH_MEDIUM_LOW    0x00800000
#define EBC0_CFG_EMPH_MEDIUM_HIGH   0x01000000
#define EBC0_CFG_EMPH_HIGH	    0x01800000
/* Chip Select Three-State Control */
#define EBC0_CFG_CSTC_DRIVEN	    0x00400000
/* Burst Prefetch */
#define EBC0_CFG_BPF_ONEDW	    0x00000000
#define EBC0_CFG_BPF_TWODW	    0x00100000
#define EBC0_CFG_BPF_FOURDW	    0x00200000
/* External Master Size */
#define EBC0_CFG_EMS_8BIT	    0x00000000
/* Power Management Enable */
#define EBC0_CFG_PME_DISABLED	    0x00000000
#define EBC0_CFG_PME_ENABLED	    0x00020000
/* Power Management Timer */
#define EBC0_CFG_PMT_ENCODE(n)		((((unsigned long)(n))&0x1F)<<12)

/*----------------------------------------------------------------------------+
| Peripheral Bank Configuration Register - EBC0_BnCR
+----------------------------------------------------------------------------*/
/* BAS - Base Address Select */
#define EBC0_BNCR_BAS_ENCODE(n)		((((unsigned long)(n))&0xFFF00000)<<0)
/* BS - Bank Size */
#define EBC0_BNCR_BS_MASK	0x000E0000
#define EBC0_BNCR_BS_1MB	0x00000000
#define EBC0_BNCR_BS_2MB	0x00020000
#define EBC0_BNCR_BS_4MB	0x00040000
#define EBC0_BNCR_BS_8MB	0x00060000
#define EBC0_BNCR_BS_16MB	0x00080000
#define EBC0_BNCR_BS_32MB	0x000A0000
#define EBC0_BNCR_BS_64MB	0x000C0000
#define EBC0_BNCR_BS_128MB	0x000E0000
/* BU - Bank Usage */
#define EBC0_BNCR_BU_MASK	0x00018000
#define EBC0_BNCR_BU_RO		    0x00008000
#define EBC0_BNCR_BU_WO		    0x00010000
#define EBC0_BNCR_BU_RW		0x00018000
/* BW - Bus Width */
#define EBC0_BNCR_BW_MASK	0x00006000
#define EBC0_BNCR_BW_8BIT	0x00000000
#define EBC0_BNCR_BW_16BIT	0x00002000
#define EBC0_BNCR_BW_32BIT	0x00004000

/*----------------------------------------------------------------------------+
| Peripheral Bank Access Parameters - EBC0_BnAP
+----------------------------------------------------------------------------*/
/* Burst Mode Enable */
#define EBC0_BNAP_BME_ENABLED	    0x80000000
#define EBC0_BNAP_BME_DISABLED	    0x00000000
/* Transfert Wait */
#define EBC0_BNAP_TWT_ENCODE(n)	    ((((unsigned long)(n))&0xFF)<<23)	/* Bits 1:8 */
/* Chip Select On Timing */
#define EBC0_BNAP_CSN_ENCODE(n)	    ((((unsigned long)(n))&0x3)<<18)	/* Bits 12:13 */
/* Output Enable On Timing */
#define EBC0_BNAP_OEN_ENCODE(n)	    ((((unsigned long)(n))&0x3)<<16)	/* Bits 14:15 */
/* Write Back Enable On Timing */
#define EBC0_BNAP_WBN_ENCODE(n)	    ((((unsigned long)(n))&0x3)<<14)	/* Bits 16:17 */
/* Write Back Enable Off Timing */
#define EBC0_BNAP_WBF_ENCODE(n)	    ((((unsigned long)(n))&0x3)<<12)	/* Bits 18:19 */
/* Transfert Hold */
#define EBC0_BNAP_TH_ENCODE(n)	    ((((unsigned long)(n))&0x7)<<9)	/* Bits 20:22 */
/* PerReady Enable */
#define EBC0_BNAP_RE_ENABLED	    0x00000100
#define EBC0_BNAP_RE_DISABLED	    0x00000000
/* Sample On Ready */
#define EBC0_BNAP_SOR_DELAYED	    0x00000000
#define EBC0_BNAP_SOR_NOT_DELAYED   0x00000080
/* Byte Enable Mode */
#define EBC0_BNAP_BEM_WRITEONLY	    0x00000000
#define EBC0_BNAP_BEM_RW	    0x00000040
/* Parity Enable */
#define EBC0_BNAP_PEN_DISABLED	    0x00000000
#define EBC0_BNAP_PEN_ENABLED	    0x00000020

/*----------------------------------------------------------------------------+
| Define Boot devices
+----------------------------------------------------------------------------*/
/* */
#define BOOT_FROM_SMALL_FLASH		0x00
#define BOOT_FROM_LARGE_FLASH_OR_SRAM	0x01
#define BOOT_FROM_NAND_FLASH0		0x02
#define BOOT_FROM_PCI			0x03
#define BOOT_DEVICE_UNKNOWN		0x04


#define	 PVR_POWERPC_440EP_PASS1    0x42221850
#define	 PVR_POWERPC_440EP_PASS2    0x422218D3

#define GPIO0		0
#define GPIO1		1

/*#define MAX_SELECTION_NB	CORE_NB */
#define MAX_CORE_SELECT_NB	22

/*----------------------------------------------------------------------------+
  | PPC440EP GPIOs addresses.
  +----------------------------------------------------------------------------*/
#define GPIO0_REAL	 0xEF600B00

#define GPIO1_REAL	 0xEF600C00

/* Offsets */
#define GPIOx_OR    0x00	/* GPIO Output Register */
#define GPIOx_TCR   0x04	/* GPIO Three-State Control Register */
#define GPIOx_OSL   0x08	/* GPIO Output Select Register (Bits 0-31) */
#define GPIOx_OSH   0x0C	/* GPIO Ouput Select Register (Bits 32-63) */
#define GPIOx_TSL   0x10	/* GPIO Three-State Select Register (Bits 0-31) */
#define GPIOx_TSH   0x14	/* GPIO Three-State Select Register  (Bits 32-63) */
#define GPIOx_ODR   0x18	/* GPIO Open drain Register */
#define GPIOx_IR    0x1C	/* GPIO Input Register */
#define GPIOx_RR1   0x20	/* GPIO Receive Register 1 */
#define GPIOx_RR2   0x24	/* GPIO Receive Register 2 */
#define GPIOx_RR3   0x28	/* GPIO Receive Register 3 */
#define GPIOx_IS1L  0x30	/* GPIO Input Select Register 1 (Bits 0-31) */
#define GPIOx_IS1H  0x34	/* GPIO Input Select Register 1 (Bits 32-63) */
#define GPIOx_IS2L  0x38	/* GPIO Input Select Register 2 (Bits 0-31) */
#define GPIOx_IS2H  0x3C	/* GPIO Input Select Register 2 (Bits 32-63) */
#define GPIOx_IS3L  0x40	/* GPIO Input Select Register 3 (Bits 0-31) */
#define GPIOx_IS3H  0x44	/* GPIO Input Select Register 3 (Bits 32-63) */

/* GPIO0 */
#define GPIO0_IS1L	(GPIO0_BASE+GPIOx_IS1L)
#define GPIO0_IS1H	(GPIO0_BASE+GPIOx_IS1H)
#define GPIO0_IS2L	(GPIO0_BASE+GPIOx_IS2L)
#define GPIO0_IS2H	(GPIO0_BASE+GPIOx_IS2H)
#define GPIO0_IS3L	(GPIO0_BASE+GPIOx_IS3L)
#define GPIO0_IS3H	(GPIO0_BASE+GPIOx_IS3L)

/* GPIO1 */
#define GPIO1_IS1L	(GPIO1_BASE+GPIOx_IS1L)
#define GPIO1_IS1H	(GPIO1_BASE+GPIOx_IS1H)
#define GPIO1_IS2L	(GPIO1_BASE+GPIOx_IS2L)
#define GPIO1_IS2H	(GPIO1_BASE+GPIOx_IS2H)
#define GPIO1_IS3L	(GPIO1_BASE+GPIOx_IS3L)
#define GPIO1_IS3H	(GPIO1_BASE+GPIOx_IS3L)

#define GPIO_OS(x)	(x+GPIOx_OSL)	 /* GPIO Output Register High or Low */
#define GPIO_TS(x)	(x+GPIOx_TSL)	 /* GPIO Three-state Control Reg High or Low */
#define GPIO_IS1(x)	(x+GPIOx_IS1L)	 /* GPIO Input register1 High or Low */
#define GPIO_IS2(x)	(x+GPIOx_IS2L)	 /* GPIO Input register2 High or Low */
#define GPIO_IS3(x)	(x+GPIOx_IS3L)	 /* GPIO Input register3 High or Low */


/*----------------------------------------------------------------------------+
  |			XX     XX
  |
  | XXXXXX   XXX XX    XXX    XXX
  |    XX    XX X XX	XX     XX
  |   XX     XX X XX	XX     XX
  |  XX	     XX	  XX	XX     XX
  | XXXXXX   XXX  XXX  XXXX   XXXX
  +----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
  | Defines
  +----------------------------------------------------------------------------*/
typedef enum zmii_config { ZMII_CONFIGURATION_UNKNOWN,
			   ZMII_CONFIGURATION_IS_MII,
			   ZMII_CONFIGURATION_IS_RMII,
			   ZMII_CONFIGURATION_IS_SMII
} zmii_config_t;

/*----------------------------------------------------------------------------+
  | Declare Configuration values
  +----------------------------------------------------------------------------*/
typedef enum uart_config_nb { L1, L2, L3, L4 } uart_config_nb_t;
typedef enum core_selection { CORE_NOT_SELECTED, CORE_SELECTED} core_selection_t;
typedef enum config_list {  IIC_CORE,
			    SCP_CORE,
			    DMA_CHANNEL_AB,
			    UIC_4_9,
			    USB2_HOST,
			    DMA_CHANNEL_CD,
			    USB2_DEVICE,
			    PACKET_REJ_FUNC_AVAIL,
			    USB1_DEVICE,
			    EBC_MASTER,
			    NAND_FLASH,
			    UART_CORE0,
			    UART_CORE1,
			    UART_CORE2,
			    UART_CORE3,
			    MII_SEL,
			    RMII_SEL,
			    SMII_SEL,
			    PACKET_REJ_FUNC_EN,
			    UIC_0_3,
			    USB1_HOST,
			    PCI_PATCH,
			    CORE_NB
} core_list_t;

typedef enum block3_value { B3_V1,  B3_V2,  B3_V3,  B3_V4,  B3_V5,
			    B3_V6,  B3_V7,  B3_V8,  B3_V9,  B3_V10,
			    B3_V11, B3_V12, B3_V13, B3_V14, B3_V15,
			    B3_V16, B3_VALUE_UNKNOWN
} block3_value_t;

typedef enum config_validity { CONFIG_IS_VALID,
			       CONFIG_IS_INVALID
} config_validity_t;
