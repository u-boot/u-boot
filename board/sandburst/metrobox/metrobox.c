/*
 *  Copyright (c) 2005
 *  Travis B. Sawyer,  Sandburst Corporation, tsawyer@sandburst.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>
#include <common.h>
#include <command.h>
#include "metrobox.h"
#include "metrobox_version.h"
#include <timestamp.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <spd_sdram.h>
#include <i2c.h>
#include "../common/sb_common.h"
#if defined(CONFIG_HAS_ETH0) || defined(CONFIG_HAS_ETH1) || \
    defined(CONFIG_HAS_ETH2) || defined(CONFIG_HAS_ETH3)
#include <net.h>
#endif

void fpga_init (void);

METROBOX_BOARD_ID_ST board_id_as[] =
{	{"Undefined"},			    /* Not specified */
	{"2x10Gb"},			    /* 2 ports, 10 GbE */
	{"20x1Gb"},			    /* 20 ports, 1 GbE */
	{"Reserved"},			     /* Reserved for future use */
};

/*************************************************************************
 *  board_early_init_f
 *
 *  Setup chip selects, initialize the Opto-FPGA, initialize
 *  interrupt polarity and triggers.
 ************************************************************************/
int board_early_init_f (void)
{
	ppc440_gpio_regs_t *gpio_regs;

	/* Enable GPIO interrupts */
	mtsdr(SDR0_PFC0, 0x00103E00);

	/* Setup access for LEDs, and system topology info */
	gpio_regs = (ppc440_gpio_regs_t *)CONFIG_SYS_GPIO_BASE;
	gpio_regs->open_drain = SBCOMMON_GPIO_SYS_LEDS;
	gpio_regs->tri_state  = SBCOMMON_GPIO_DBGLEDS;

	/* Turn on all the leds for now */
	gpio_regs->out = SBCOMMON_GPIO_LEDS;

	/*--------------------------------------------------------------------+
	  | Initialize EBC CONFIG
	  +-------------------------------------------------------------------*/
	mtebc(EBC0_CFG,
	      EBC_CFG_LE_UNLOCK	   | EBC_CFG_PTD_ENABLE |
	      EBC_CFG_RTC_64PERCLK | EBC_CFG_ATC_PREVIOUS |
	      EBC_CFG_DTC_PREVIOUS | EBC_CFG_CTC_PREVIOUS |
	      EBC_CFG_EMC_DEFAULT  | EBC_CFG_PME_DISABLE |
	      EBC_CFG_PR_32);

	/*--------------------------------------------------------------------+
	  | 1/2 MB FLASH. Initialize bank 0 with default values.
	  +-------------------------------------------------------------------*/
	mtebc(PB0AP,
	      EBC_BXAP_BME_DISABLED | EBC_BXAP_TWT_ENCODE(8) |
	      EBC_BXAP_BCE_DISABLE  | EBC_BXAP_CSN_ENCODE(1) |
	      EBC_BXAP_OEN_ENCODE(1)| EBC_BXAP_WBN_ENCODE(1) |
	      EBC_BXAP_WBF_ENCODE(1)| EBC_BXAP_TH_ENCODE(1) |
	      EBC_BXAP_RE_DISABLED  | EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);

	mtebc(PB0CR, EBC_BXCR_BAS_ENCODE(CONFIG_SYS_FLASH_BASE) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_8BIT);
	/*--------------------------------------------------------------------+
	  | 8KB NVRAM/RTC. Initialize bank 1 with default values.
	  +-------------------------------------------------------------------*/
	mtebc(PB1AP,
	      EBC_BXAP_BME_DISABLED | EBC_BXAP_TWT_ENCODE(10) |
	      EBC_BXAP_BCE_DISABLE  | EBC_BXAP_CSN_ENCODE(1) |
	      EBC_BXAP_OEN_ENCODE(1)| EBC_BXAP_WBN_ENCODE(1) |
	      EBC_BXAP_WBF_ENCODE(1)| EBC_BXAP_TH_ENCODE(1) |
	      EBC_BXAP_RE_DISABLED  | EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);

	mtebc(PB1CR, EBC_BXCR_BAS_ENCODE(0x48000000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_8BIT);

	/*--------------------------------------------------------------------+
	  | Compact Flash, uses 2 Chip Selects (2 & 6)
	  +-------------------------------------------------------------------*/
	mtebc(PB2AP,
	      EBC_BXAP_BME_DISABLED | EBC_BXAP_TWT_ENCODE(8) |
	      EBC_BXAP_BCE_DISABLE  | EBC_BXAP_CSN_ENCODE(1) |
	      EBC_BXAP_OEN_ENCODE(1)| EBC_BXAP_WBN_ENCODE(1) |
	      EBC_BXAP_WBF_ENCODE(0)| EBC_BXAP_TH_ENCODE(1) |
	      EBC_BXAP_RE_DISABLED  | EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);

	mtebc(PB2CR, EBC_BXCR_BAS_ENCODE(0xF0000000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_16BIT);

	/*--------------------------------------------------------------------+
	  | OPTO & OFEM FPGA. Initialize bank 3 with default values.
	  +-------------------------------------------------------------------*/
	mtebc(PB3AP,
	      EBC_BXAP_RE_ENABLED    | EBC_BXAP_SOR_NONDELAYED |
	      EBC_BXAP_BME_DISABLED  | EBC_BXAP_TWT_ENCODE(3) |
	      EBC_BXAP_TH_ENCODE(1)  | EBC_BXAP_WBF_ENCODE(0) |
	      EBC_BXAP_CSN_ENCODE(1) | EBC_BXAP_PEN_DISABLED |
	      EBC_BXAP_OEN_ENCODE(1) | EBC_BXAP_BEM_RW);

	mtebc(PB3CR, EBC_BXCR_BAS_ENCODE(0x48200000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_32BIT);

	/*--------------------------------------------------------------------+
	  | MAC A for metrobox
	  | MAC A & B for Kamino.  OFEM FPGA decodes the addresses
	  | Initialize bank 4 with default values.
	  +-------------------------------------------------------------------*/
	mtebc(PB4AP,
	      EBC_BXAP_RE_ENABLED    | EBC_BXAP_SOR_NONDELAYED |
	      EBC_BXAP_BME_DISABLED  | EBC_BXAP_TWT_ENCODE(3) |
	      EBC_BXAP_TH_ENCODE(1)  | EBC_BXAP_WBF_ENCODE(0) |
	      EBC_BXAP_CSN_ENCODE(1) | EBC_BXAP_PEN_DISABLED |
	      EBC_BXAP_OEN_ENCODE(1) | EBC_BXAP_BEM_RW);

	mtebc(PB4CR, EBC_BXCR_BAS_ENCODE(0x48600000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_32BIT);

	/*--------------------------------------------------------------------+
	  | Metrobox MAC B  Initialize bank 5 with default values.
	  | KA REF FPGA	 Initialize bank 5 with default values.
	  +-------------------------------------------------------------------*/
	mtebc(PB5AP,
	      EBC_BXAP_RE_ENABLED    | EBC_BXAP_SOR_NONDELAYED |
	      EBC_BXAP_BME_DISABLED  | EBC_BXAP_TWT_ENCODE(3) |
	      EBC_BXAP_TH_ENCODE(1)  | EBC_BXAP_WBF_ENCODE(0) |
	      EBC_BXAP_CSN_ENCODE(1) | EBC_BXAP_PEN_DISABLED |
	      EBC_BXAP_OEN_ENCODE(1) | EBC_BXAP_BEM_RW);

	mtebc(PB5CR, EBC_BXCR_BAS_ENCODE(0x48700000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_32BIT);

	/*--------------------------------------------------------------------+
	  | Compact Flash, uses 2 Chip Selects (2 & 6)
	  +-------------------------------------------------------------------*/
	mtebc(PB6AP,
	      EBC_BXAP_BME_DISABLED | EBC_BXAP_TWT_ENCODE(8) |
	      EBC_BXAP_BCE_DISABLE  | EBC_BXAP_CSN_ENCODE(1) |
	      EBC_BXAP_OEN_ENCODE(1)| EBC_BXAP_WBN_ENCODE(1) |
	      EBC_BXAP_WBF_ENCODE(0)| EBC_BXAP_TH_ENCODE(1) |
	      EBC_BXAP_RE_DISABLED  | EBC_BXAP_BEM_WRITEONLY |
	      EBC_BXAP_PEN_DISABLED);

	mtebc(PB6CR, EBC_BXCR_BAS_ENCODE(0xF0100000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_16BIT);

	/*--------------------------------------------------------------------+
	  | BME-32. Initialize bank 7 with default values.
	  +-------------------------------------------------------------------*/
	mtebc(PB7AP,
	      EBC_BXAP_RE_ENABLED    | EBC_BXAP_SOR_NONDELAYED |
	      EBC_BXAP_BME_DISABLED  | EBC_BXAP_TWT_ENCODE(3) |
	      EBC_BXAP_TH_ENCODE(1)  | EBC_BXAP_WBF_ENCODE(0) |
	      EBC_BXAP_CSN_ENCODE(1) | EBC_BXAP_PEN_DISABLED |
	      EBC_BXAP_OEN_ENCODE(1) | EBC_BXAP_BEM_RW);

	mtebc(PB7CR, EBC_BXCR_BAS_ENCODE(0x48500000) |
	      EBC_BXCR_BS_1MB | EBC_BXCR_BU_RW | EBC_BXCR_BW_32BIT);

	/*--------------------------------------------------------------------+
	 * Setup the interrupt controller polarities, triggers, etc.
	 +-------------------------------------------------------------------*/
	/*
	 * Because of the interrupt handling rework to handle 440GX interrupts
	 * with the common code, we needed to change names of the UIC registers.
	 * Here the new relationship:
	 *
	 * U-Boot name	440GX name
	 * -----------------------
	 * UIC0		UICB0
	 * UIC1		UIC0
	 * UIC2		UIC1
	 * UIC3		UIC2
	 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */
	mtdcr (UIC1ER, 0x00000000);	/* disable all */
	mtdcr (UIC1CR, 0x00000000);	/* all non- critical */
	mtdcr (UIC1PR, 0xfffffe03);	/* polarity */
	mtdcr (UIC1TR, 0x01c00000);	/* trigger edge vs level */
	mtdcr (UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */

	mtdcr (UIC2SR, 0xffffffff);	/* clear all */
	mtdcr (UIC2ER, 0x00000000);	/* disable all */
	mtdcr (UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC2PR, 0xffffc8ff);	/* polarity */
	mtdcr (UIC2TR, 0x00ff0000);	/* trigger edge vs level */
	mtdcr (UIC2VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC2SR, 0xffffffff);	/* clear all */

	mtdcr (UIC3SR, 0xffffffff);	/* clear all */
	mtdcr (UIC3ER, 0x00000000);	/* disable all */
	mtdcr (UIC3CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC3PR, 0xffff83ff);	/* polarity */
	mtdcr (UIC3TR, 0x00ff8c0f);	/* trigger edge vs level */
	mtdcr (UIC3VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC3SR, 0xffffffff);	/* clear all */

	mtdcr (UIC0SR, 0xfc000000);	/* clear all */
	mtdcr (UIC0ER, 0x00000000);	/* disable all */
	mtdcr (UIC0CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC0PR, 0xfc000000);
	mtdcr (UIC0TR, 0x00000000);
	mtdcr (UIC0VR, 0x00000001);

	fpga_init();

	return 0;
}

/*************************************************************************
 *  checkboard
 *
 *  Dump pertinent info to the console
 ************************************************************************/
int checkboard (void)
{
	sys_info_t sysinfo;
	unsigned char brd_rev, brd_id;
	unsigned short sernum;
	unsigned char opto_rev, opto_id;
	OPTO_FPGA_REGS_ST *opto_ps;

	opto_ps = (OPTO_FPGA_REGS_ST *)CONFIG_SYS_FPGA_BASE;

	opto_rev = (unsigned char)((opto_ps->revision_ul &
				    SAND_HAL_XC_XCVR_CNTL_REVISION_REVISION_MASK)
				   >> SAND_HAL_XC_XCVR_CNTL_REVISION_REVISION_SHIFT);

	opto_id = (unsigned char)((opto_ps->revision_ul &
				   SAND_HAL_XC_XCVR_CNTL_REVISION_IDENTIFICATION_MASK)
				  >> SAND_HAL_XC_XCVR_CNTL_REVISION_IDENTIFICATION_SHIFT);

	brd_rev = (unsigned char)((opto_ps->boardinfo_ul &
				   SAND_HAL_XC_XCVR_CNTL_BRD_INFO_BRD_REV_MASK)
				  >> SAND_HAL_XC_XCVR_CNTL_BRD_INFO_BRD_REV_SHIFT);

	brd_id = (unsigned char)((opto_ps->boardinfo_ul &
				  SAND_HAL_XC_XCVR_CNTL_BRD_INFO_BRD_ID_MASK)
				 >> SAND_HAL_XC_XCVR_CNTL_BRD_INFO_BRD_ID_SHIFT);

	get_sys_info (&sysinfo);

	sernum = sbcommon_get_serial_number();
	printf ("Board: Sandburst Corporation MetroBox Serial Number: %d\n", sernum);
	printf ("%s\n", METROBOX_U_BOOT_REL_STR);

	printf ("Built %s %s by %s\n", U_BOOT_DATE, U_BOOT_TIME, BUILDUSER);
	if (sbcommon_get_master()) {
		printf("Slot 0 - Master\nSlave board");
		if (sbcommon_secondary_present())
			printf(" present\n");
		else
			printf(" not detected\n");
	} else {
		printf("Slot 1 - Slave\n\n");
	}

	printf ("OptoFPGA ID:\t0x%02X\tRev:  0x%02X\n", opto_id, opto_rev);
	printf ("Board Rev:\t0x%02X\tID:  %s\n", brd_rev, board_id_as[brd_id].name);

	/* Fix the ack in the bme 32 */
	udelay(5000);
	out32(CONFIG_SYS_BME32_BASE + 0x0000000C, 0x00000001);
	asm("eieio");


	return (0);
}

/*************************************************************************
 *  misc_init_f
 *
 *  Initialize I2C bus one to gain access to the fans
 ************************************************************************/
int misc_init_f (void)
{
	/* Turn on fans */
	sbcommon_fans();

	return (0);
}

/*************************************************************************
 *  misc_init_r
 *
 *  Do nothing.
 ************************************************************************/
int misc_init_r (void)
{
	unsigned short sernum;
	char envstr[255];
	uchar enetaddr[6];
	unsigned char opto_rev;
	OPTO_FPGA_REGS_ST *opto_ps;

	opto_ps = (OPTO_FPGA_REGS_ST *)CONFIG_SYS_FPGA_BASE;

	if(NULL != getenv("secondserial")) {
	    puts("secondserial is set, switching to second serial port\n");
	    setenv("stderr", "serial1");
	    setenv("stdout", "serial1");
	    setenv("stdin", "serial1");
	}

	setenv("ubrelver", METROBOX_U_BOOT_REL_STR);

	memset(envstr, 0, 255);
	sprintf (envstr, "Built %s %s by %s",
		 U_BOOT_DATE, U_BOOT_TIME, BUILDUSER);
	setenv("bldstr", envstr);
	saveenv();

	if( getenv("autorecover")) {
		setenv("autorecover", NULL);
		saveenv();
		sernum = sbcommon_get_serial_number();

		printf("\nSetting up environment for automatic filesystem recovery\n");
		/*
		 * Setup default bootargs
		 */
		memset(envstr, 0, 255);
		sprintf(envstr, "console=ttyS0,9600 root=/dev/ram0 "
			"rw ip=10.100.60.%d:::255.255.0.0:metrobox%d:eth0:none idebus=33",
			sernum, sernum);
		setenv("bootargs", envstr);

		/*
		 * Setup Default boot command
		 */
		setenv("bootcmd", "fatload ide 0 8000000 pimage.metrobox;"
		       "fatload ide 0 8100000 pramdisk;"
		       "bootm 8000000 8100000");

		printf("Done.  Please type allow the system to continue to boot\n");
	}

	if( getenv("fakeled")) {
		setenv("bootdelay", "-1");
		saveenv();
		printf("fakeled is set. use 'setenv fakeled ; setenv bootdelay 5 ; saveenv' to recover\n");
		opto_rev = (unsigned char)((opto_ps->revision_ul &
					    SAND_HAL_XC_XCVR_CNTL_REVISION_REVISION_MASK)
					   >> SAND_HAL_XC_XCVR_CNTL_REVISION_REVISION_SHIFT);

		if(0x12 <= opto_rev) {
			opto_ps->control_ul &= ~ SAND_HAL_XC_XCVR_CNTL_CNTL_ERROR_LED_MASK;
		}
	}

#ifdef CONFIG_HAS_ETH0
	if (!eth_getenv_enetaddr("ethaddr", enetaddr)) {
		board_get_enetaddr(0, enetaddr);
		eth_setenv_enetaddr("ethaddr", enetaddr);
	}
#endif

#ifdef CONFIG_HAS_ETH1
	if (!eth_getenv_enetaddr("eth1addr", enetaddr)) {
		board_get_enetaddr(1, enetaddr);
		eth_setenv_enetaddr("eth1addr", enetaddr);
	}
#endif

#ifdef CONFIG_HAS_ETH2
	if (!eth_getenv_enetaddr("eth2addr", enetaddr)) {
		board_get_enetaddr(2, enetaddr);
		eth_setenv_enetaddr("eth2addr", enetaddr);
	}
#endif

#ifdef CONFIG_HAS_ETH3
	if (!eth_getenv_enetaddr("eth3addr", enetaddr)) {
		board_get_enetaddr(3, enetaddr);
		eth_setenv_enetaddr("eth3addr", enetaddr);
	}
#endif

	return (0);
}

/*************************************************************************
 *  ide_set_reset
 ************************************************************************/
#ifdef CONFIG_IDE_RESET
void ide_set_reset(int on)
{
	OPTO_FPGA_REGS_ST *opto_ps;
	opto_ps = (OPTO_FPGA_REGS_ST *)CONFIG_SYS_FPGA_BASE;

	if (on) {		/* assert RESET */
	    opto_ps->reset_ul &= ~SAND_HAL_XC_XCVR_CNTL_RESET_CF_RESET_N_MASK;
	} else {		/* release RESET */
	    opto_ps->reset_ul |= SAND_HAL_XC_XCVR_CNTL_RESET_CF_RESET_N_MASK;
	}
}
#endif /* CONFIG_IDE_RESET */

/*************************************************************************
 *  fpga_init
 ************************************************************************/
void fpga_init(void)
{
	OPTO_FPGA_REGS_ST *opto_ps;
	unsigned char opto_rev;
	unsigned long tmp;

	/* Ensure we have power all around */
	udelay(500);

	/*
	 * Take appropriate hw bits out of reset
	 */
	opto_ps = (OPTO_FPGA_REGS_ST *)CONFIG_SYS_FPGA_BASE;

	tmp =
	    SAND_HAL_XC_XCVR_CNTL_RESET_MAC1_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_MAC0_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_BME_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_ACE_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_CF_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_QE_A_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_IFE_A_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_EFE_A_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_QE_B_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_IFE_B_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_EFE_B_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_LOCK1_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_LOCK0_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_I2C_MUX1_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_I2C_MUX0_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_PHY0_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_PHY1_RESET_N_MASK |
	    SAND_HAL_XC_XCVR_CNTL_RESET_SLAVE_RESET_N_MASK;
	opto_ps->reset_ul = tmp;
	/*
	 * Turn on the 'Slow Blink' for the System Error Led.
	 * Ensure FPGA rev is up to at least rev 0x12
	 */
	opto_rev = (unsigned char)((opto_ps->revision_ul &
				    SAND_HAL_XC_XCVR_CNTL_REVISION_REVISION_MASK)
				   >> SAND_HAL_XC_XCVR_CNTL_REVISION_REVISION_SHIFT);
	if(0x12 <= opto_rev) {
	    opto_ps->control_ul |= 1 << SAND_HAL_XC_XCVR_CNTL_CNTL_ERROR_LED_SHIFT;
	}

	asm("eieio");

	return;
}

int metroboxSetupVars(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned short sernum;
	char envstr[255];

	sernum = sbcommon_get_serial_number();

	memset(envstr, 0, 255);
	/*
	 * Setup our ip address
	 */
	sprintf(envstr, "10.100.60.%d", sernum);

	setenv("ipaddr", envstr);
	/*
	 * Setup the host ip address
	 */
	setenv("serverip", "10.100.17.10");

	/*
	 * Setup default bootargs
	 */
	memset(envstr, 0, 255);

	sprintf(envstr, "console=ttyS0,9600 root=/dev/nfs "
		"rw nfsroot=10.100.17.10:/home/metrobox/mbc%d "
		"nfsaddrs=10.100.60.%d:10.100.17.10:10.100.1.1"
		":255.255.0.0:metrobox%d.sandburst.com:eth0:none idebus=33",
		sernum, sernum, sernum);

	setenv("bootargs_nfs", envstr);
	setenv("bootargs", envstr);

	/*
	 * Setup CF bootargs
	 */
	memset(envstr, 0, 255);
	sprintf(envstr, "console=ttyS0,9600 root=/dev/hda2 "
		"rw ip=10.100.60.%d:::255.255.0.0:metrobox%d:eth0:none idebus=33",
		sernum, sernum);

	setenv("bootargs_cf", envstr);

	/*
	 * Setup Default boot command
	 */
	setenv("bootcmd_tftp", "tftp 8000000 pImage.metrobox;bootm 8000000");
	setenv("bootcmd", "tftp 8000000 pImage.metrobox;bootm 8000000");

	/*
	 * Setup compact flash boot command
	 */
	setenv("bootcmd_cf", "fatload ide 0 8000000 pimage.metrobox;bootm 8000000");

	saveenv();


	return(1);
}

int metroboxRecover(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned short sernum;
	char envstr[255];

	sernum = sbcommon_get_serial_number();

	printf("\nSetting up environment for filesystem recovery\n");
	/*
	 * Setup default bootargs
	 */
	memset(envstr, 0, 255);
	sprintf(envstr, "console=ttyS0,9600 root=/dev/ram0 "
		"rw ip=10.100.60.%d:::255.255.0.0:metrobox%d:eth0:none",
		sernum, sernum);

	setenv("bootargs", envstr);

	/*
	 * Setup Default boot command
	 */
	setenv("bootcmd", "fatload ide 0 8000000 pimage.metrobox;"
	       "fatload ide 0 8100000 pramdisk;"
	       "bootm 8000000 8100000");

	printf("Done.  Please type boot<cr>.\nWhen the kernel has booted"
	       " please type fsrecover.sh<cr>\n");

	return(1);
}

U_BOOT_CMD(mbsetup, 1, 1, metroboxSetupVars,
	   "Set environment to factory defaults", "");

U_BOOT_CMD(mbrecover, 1, 1, metroboxRecover,
	   "Set environment to allow for fs recovery", "");
