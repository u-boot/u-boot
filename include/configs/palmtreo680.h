/*
 * Palm Treo 680 configuration file
 *
 * Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 *
 */

#ifndef	__CONFIG_H
#define	__CONFIG_H

/*
 * High Level Board Configuration Options
 */
#define CONFIG_CPU_PXA27X
#define CONFIG_PALMTREO680
#define CONFIG_MACH_TYPE                MACH_TYPE_TREO680

#define CONFIG_SYS_MALLOC_LEN           (4096*1024)

#define CONFIG_LZMA

/*
 * Serial Console Configuration
 */
#define CONFIG_PXA_SERIAL
#define CONFIG_FFUART                   1
#define CONFIG_BAUDRATE                 9600
#define CONFIG_SYS_BAUDRATE_TABLE       { 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_CONS_INDEX               3

/* we have nand (although technically nand *is* flash...) */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_LCD
/* #define CONFIG_KEYBOARD */  /* TODO */

/*
 * Bootloader Components Configuration
 */
#define CONFIG_CMD_ENV
#define CONFIG_CMD_MMC
#define CONFIG_CMD_NAND

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS

/*
 * MMC Card Configuration
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_PXA_MMC_GENERIC

#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION
#endif

/*
 * LCD
 */
#ifdef CONFIG_LCD
#define CONFIG_PXA_LCD
#define CONFIG_ACX544AKN
#define CONFIG_LCD_LOGO
#define CONFIG_SYS_LCD_PXA_NO_L_BIAS /* don't configure GPIO77 as L_BIAS */
#define LCD_BPP LCD_COLOR16
#define CONFIG_FB_ADDR 0x5c000000    /* internal SRAM */
#define CONFIG_CMD_BMP
#define CONFIG_SPLASH_SCREEN         /* requires "splashimage" env var */
#define CONFIG_SPLASH_SCREEN_ALIGN   /* requires "splashpos" env var */
#define CONFIG_VIDEO_BMP_GZIP
#define CONFIG_SYS_VIDEO_LOGO_MAX_SIZE  (2 << 20)

#endif

/*
 * KGDB
 */
#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE            230400  /* kgdb serial port speed */
#endif

/*
 * HUSH Shell Configuration
 */
#define CONFIG_SYS_HUSH_PARSER          1
#define CONFIG_SYS_PROMPT_HUSH_PS2      "> "

#define CONFIG_SYS_LONGHELP
#ifdef CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT               "$ "
#else
#endif
#define CONFIG_SYS_CBSIZE               256
#define CONFIG_SYS_PBSIZE               \
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)
#define CONFIG_SYS_MAXARGS              16
#define CONFIG_SYS_BARGSIZE             CONFIG_SYS_CBSIZE
#define CONFIG_SYS_DEVICE_NULLDEV       1

/*
 * Clock Configuration
 */
#define CONFIG_SYS_CPUSPEED             0x210           /* 416MHz ; N=2,L=16 */

/*
 * Stack sizes
 */
#define CONFIG_STACKSIZE                (128*1024)      /* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ            (4*1024)        /* IRQ stack */
#define CONFIG_STACKSIZE_FIQ            (4*1024)        /* FIQ stack */
#endif

/*
 * DRAM Map
 */
#define CONFIG_NR_DRAM_BANKS            1               /* 1 bank of DRAM */
#define PHYS_SDRAM_1                    0xa0000000      /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE               0x04000000      /* 64 MB */

#define CONFIG_SYS_DRAM_BASE            0xa0000000
#define CONFIG_SYS_DRAM_SIZE            0x04000000      /* 64 MB DRAM */

#define CONFIG_SYS_MEMTEST_START        0xa0400000      /* memtest works on */
#define CONFIG_SYS_MEMTEST_END          0xa0800000      /* 4 ... 8 MB in DRAM */
#define CONFIG_SYS_LOAD_ADDR            CONFIG_SYS_DRAM_BASE
#define CONFIG_SYS_SDRAM_BASE           PHYS_SDRAM_1

/*
 * GPIO settings
 */
#define CONFIG_SYS_GAFR0_L_VAL  0x0E000000
#define CONFIG_SYS_GAFR0_U_VAL  0xA500001A
#define CONFIG_SYS_GAFR1_L_VAL  0x60000002
#define CONFIG_SYS_GAFR1_U_VAL  0xAAA07959
#define CONFIG_SYS_GAFR2_L_VAL  0x02AAAAAA
#define CONFIG_SYS_GAFR2_U_VAL  0x41440F08
#define CONFIG_SYS_GAFR3_L_VAL  0x56AA95FF
#define CONFIG_SYS_GAFR3_U_VAL  0x00001401
#define CONFIG_SYS_GPCR0_VAL    0x1FF80400
#define CONFIG_SYS_GPCR1_VAL    0x03003FC1
#define CONFIG_SYS_GPCR2_VAL    0x01C1E000
#define CONFIG_SYS_GPCR3_VAL    0x01C1E000
#define CONFIG_SYS_GPDR0_VAL    0xCFF90400
#define CONFIG_SYS_GPDR1_VAL    0xFB22BFC1
#define CONFIG_SYS_GPDR2_VAL    0x93CDFFDF
#define CONFIG_SYS_GPDR3_VAL    0x0069FF81
#define CONFIG_SYS_GPSR0_VAL    0x02000018
#define CONFIG_SYS_GPSR1_VAL    0x00000000
#define CONFIG_SYS_GPSR2_VAL    0x000C0000
#define CONFIG_SYS_GPSR3_VAL    0x00080000

#define CONFIG_SYS_PSSR_VAL     0x30

/*
 * Clock settings
 */
#define CONFIG_SYS_CKEN         0x01ffffff
#define CONFIG_SYS_CCCR         0x02000210

/*
 * Memory settings
 */
#define CONFIG_SYS_MSC0_VAL     0x7ff844c8
#define CONFIG_SYS_MSC1_VAL     0x7ff86ab4
#define CONFIG_SYS_MSC2_VAL     0x7ff87ff8
#define CONFIG_SYS_MDCNFG_VAL   0x0B880acd
#define CONFIG_SYS_MDREFR_VAL   0x201fa031
#define CONFIG_SYS_MDMRS_VAL    0x00320032
#define CONFIG_SYS_FLYCNFG_VAL  0x00000000
#define CONFIG_SYS_SXCNFG_VAL   0x40044004
#define CONFIG_SYS_MECR_VAL     0x00000003
#define CONFIG_SYS_MCMEM0_VAL   0x0001c391
#define CONFIG_SYS_MCMEM1_VAL   0x0001c391
#define CONFIG_SYS_MCATT0_VAL   0x0001c391
#define CONFIG_SYS_MCATT1_VAL   0x0001c391
#define CONFIG_SYS_MCIO0_VAL    0x00014611
#define CONFIG_SYS_MCIO1_VAL    0x0001c391

/*
 * USB
 */
#define CONFIG_USB_DEVICE
#define CONFIG_USB_TTY
#define CONFIG_USB_DEV_PULLUP_GPIO 114

/*
 * SPL
 */
#define CONFIG_SPL_TEXT_BASE    0xa1700000 /* IPL loads SPL here */
#define CONFIG_SPL_STACK        0x5c040000 /* end of internal SRAM */
#define CONFIG_SPL_NAND_SUPPORT /* build libnand for spl */
#define CONFIG_SPL_NAND_DOCG4   /* use lean docg4 nand spl driver */
#define CONFIG_SPL_LIBGENERIC_SUPPORT  /* spl uses memcpy */

/*
 * NAND
 */
#define CONFIG_NAND_DOCG4
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE 1 /* only one device */
#define CONFIG_SYS_NAND_BASE 0x00000000 /* mapped to reset vector */
#define CONFIG_SYS_NAND_PAGE_SIZE 0x200
#define CONFIG_SYS_NAND_BLOCK_SIZE 0x40000
#define CONFIG_BITREVERSE       /* needed by docg4 driver */
#define CONFIG_BCH              /* needed by docg4 driver */

/*
 * IMPORTANT NOTE: this is the size of the concatenated spl + u-boot image.  It
 * will be rounded up to the next 64k boundary (the spl flash block size), so it
 * does not have to be exact, but you must ensure that it is not less than the
 * actual image size, or it may fail to boot (bricked phone)!
 * (Tip: reduces to three blocks with lcd and mmc support removed from u-boot.)
*/
#define CONFIG_SYS_NAND_U_BOOT_SIZE 0x40000 /* four 64k flash blocks */

/*
 * This is the byte offset into the flash at which the concatenated spl + u-boot
 * image is placed.  It must be at the start of a block (256k boundary).  Blocks
 * 0 - 5 are write-protected, so we start at block 6.
 */
#define CONFIG_SYS_NAND_U_BOOT_OFFS 0x180000  /* block 6 */

/* DRAM address to which u-boot proper is loaded (before it relocates itself) */
#define CONFIG_SYS_NAND_U_BOOT_DST  0xa0000000
#define CONFIG_SYS_NAND_U_BOOT_START CONFIG_SYS_NAND_U_BOOT_DST

/* passed to linker by Makefile as arg to -Ttext option */
#define CONFIG_SYS_TEXT_BASE 0xa0000000

#define CONFIG_SYS_INIT_SP_ADDR         0x5c040000 /* end of internal SRAM */

/*
 * environment
 */
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_BUILD_ENVCRC
#define CONFIG_ENV_SIZE 0x200
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_EXTRA_ENV_SETTINGS  \
	"stdin=usbtty\0"           \
	"stdout=usbtty\0"          \
	"stderr=usbtty"
#define CONFIG_BOOTARGS "mtdparts=Msys_Diskonchip_G4:1536k(protected_part)ro,1024k(bootloader_part),-(filesys_part) \
ip=192.168.11.102:::255.255.255.0:treo:usb0"
#define CONFIG_BOOTDELAY   3

#if 0 /* example: try 2nd mmc partition, then nand */
#define CONFIG_BOOTCOMMAND                                              \
	"mmc rescan; "                                                  \
	"if mmcinfo && ext2load mmc 0:2 0xa1000000 uImage; then "       \
	    "bootm 0xa1000000; "					\
	"elif nand read 0xa1000000 0x280000 0x240000; then "            \
	    "bootm 0xa1000000; "					\
	"fi; "
#endif

/* u-boot lives at end of SDRAM, so use start of SDRAM for stand alone apps */
#define CONFIG_STANDALONE_LOAD_ADDR 0xa0000000

#define CONFIG_SYS_DCACHE_OFF
#define CONFIG_SYS_ICACHE_OFF

#endif  /* __CONFIG_H */
