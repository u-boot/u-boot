/*
 * (C) Copyright 2000 - 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __U_BOOT_H__
#define __U_BOOT_H__

/*
 * Board information passed to Linux kernel from U-Boot
 *
 * include/asm-ppc/u-boot.h
 */

#ifndef __ASSEMBLY__

typedef struct bd_info {
	unsigned long	bi_memstart;	/* start of DRAM memory */
	unsigned long	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size	 of FLASH memory */
	unsigned long	bi_flashoffset; /* reserved area for startup monitor */
	unsigned long	bi_sramstart;	/* start of SRAM memory */
	unsigned long	bi_sramsize;	/* size	 of SRAM memory */
#if defined(CONFIG_5xx) || defined(CONFIG_8xx) || defined(CONFIG_8260) \
	|| defined(CONFIG_E500) || defined(CONFIG_MPC86xx)
	unsigned long	bi_immr_base;	/* base of IMMR register */
#endif
#if defined(CONFIG_MPC5xxx)
	unsigned long	bi_mbar_base;	/* base of internal registers */
#endif
#if defined(CONFIG_MPC83XX)
	unsigned long	bi_immrbar;
#endif
#if defined(CONFIG_MPC8220)
	unsigned long	bi_mbar_base;	/* base of internal registers */
	unsigned long   bi_inpfreq;     /* Input Freq, In MHz */
	unsigned long   bi_pcifreq;     /* PCI Freq, in MHz */
	unsigned long   bi_pevfreq;     /* PEV Freq, in MHz */
	unsigned long   bi_flbfreq;     /* Flexbus Freq, in MHz */
	unsigned long   bi_vcofreq;     /* VCO Freq, in MHz */
#endif
	unsigned long	bi_bootflags;	/* boot / reboot flag (for LynxOS) */
	unsigned long	bi_ip_addr;	/* IP Address */
	unsigned char	bi_enetaddr[6];	/* Ethernet adress */
	unsigned short	bi_ethspeed;	/* Ethernet speed in Mbps */
	unsigned long	bi_intfreq;	/* Internal Freq, in MHz */
	unsigned long	bi_busfreq;	/* Bus Freq, in MHz */
#if defined(CONFIG_CPM2)
	unsigned long	bi_cpmfreq;	/* CPM_CLK Freq, in MHz */
	unsigned long	bi_brgfreq;	/* BRG_CLK Freq, in MHz */
	unsigned long	bi_sccfreq;	/* SCC_CLK Freq, in MHz */
	unsigned long	bi_vco;		/* VCO Out from PLL, in MHz */
#endif
#if defined(CONFIG_MPC5xxx)
	unsigned long	bi_ipbfreq;	/* IPB Bus Freq, in MHz */
	unsigned long	bi_pcifreq;	/* PCI Bus Freq, in MHz */
#endif
	unsigned long	bi_baudrate;	/* Console Baudrate */
#if defined(CONFIG_405)   || \
    defined(CONFIG_405GP) || \
    defined(CONFIG_405CR) || \
    defined(CONFIG_405EP) || \
    defined(CONFIG_405EZ) || \
    defined(CONFIG_440)
	unsigned char	bi_s_version[4];	/* Version of this structure */
	unsigned char	bi_r_version[32];	/* Version of the ROM (AMCC) */
	unsigned int	bi_procfreq;	/* CPU (Internal) Freq, in Hz */
	unsigned int	bi_plb_busfreq;	/* PLB Bus speed, in Hz */
	unsigned int	bi_pci_busfreq;	/* PCI Bus speed, in Hz */
	unsigned char	bi_pci_enetaddr[6];	/* PCI Ethernet MAC address */
#endif
#if defined(CONFIG_HYMOD)
	hymod_conf_t	bi_hymod_conf;	/* hymod configuration information */
#endif

#ifdef CONFIG_HAS_ETH1
	/* second onboard ethernet port */
	unsigned char   bi_enet1addr[6];
#endif
#ifdef CONFIG_HAS_ETH2
	/* third onboard ethernet port */
	unsigned char	bi_enet2addr[6];
#endif
#ifdef CONFIG_HAS_ETH3
	unsigned char   bi_enet3addr[6];
#endif

#if defined(CONFIG_405GP) || defined(CONFIG_405EP) || \
    defined(CONFIG_405EZ) || defined(CONFIG_440GX) || \
    defined(CONFIG_440EP) || defined(CONFIG_440GR) || \
    defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
	unsigned int	bi_opbfreq;		/* OPB clock in Hz */
	int		bi_iic_fast[2];		/* Use fast i2c mode */
#endif
#if defined(CONFIG_NX823)
	unsigned char	bi_sernum[8];
#endif
#if defined(CONFIG_4xx)
#if defined(CONFIG_440GX)
	int 		bi_phynum[4];           /* Determines phy mapping */
	int 		bi_phymode[4];          /* Determines phy mode */
#elif defined(CONFIG_405EP) || defined(CONFIG_440)
	int 		bi_phynum[2];           /* Determines phy mapping */
	int 		bi_phymode[2];          /* Determines phy mode */
#else
	int 		bi_phynum[1];           /* Determines phy mapping */
	int 		bi_phymode[1];          /* Determines phy mode */
#endif
#endif /* defined(CONFIG_4xx) */
} bd_t;

#endif /* __ASSEMBLY__ */
#endif	/* __U_BOOT_H__ */
