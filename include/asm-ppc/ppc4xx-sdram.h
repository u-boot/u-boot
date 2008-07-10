/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _PPC4xx_SDRAM_H_
#define _PPC4xx_SDRAM_H_

#if defined(CONFIG_SDRAM_PPC4xx_IBM_SDRAM)

/*
 * SDRAM Controller
 */
/*
 * XXX - ToDo: Revisit file to change all these lower case defines into
 * upper case. Also needs to be done in the controller setup code too
 * of course. sr, 2008-06-02
 */
#ifndef CONFIG_405EP
#define mem_besra	0x00	/* bus error syndrome reg a		*/
#define mem_besrsa	0x04	/* bus error syndrome reg set a		*/
#define mem_besrb	0x08	/* bus error syndrome reg b		*/
#define mem_besrsb	0x0c	/* bus error syndrome reg set b		*/
#define mem_bear	0x10	/* bus error address reg		*/
#endif
#define mem_mcopt1	0x20	/* memory controller options 1		*/
#define mem_status	0x24	/* memory status			*/
#define mem_rtr		0x30	/* refresh timer reg			*/
#define mem_pmit	0x34	/* power management idle timer		*/
#define mem_mb0cf	0x40	/* memory bank 0 configuration		*/
#define mem_mb1cf	0x44	/* memory bank 1 configuration		*/
#ifndef CONFIG_405EP
#define mem_mb2cf	0x48	/* memory bank 2 configuration		*/
#define mem_mb3cf	0x4c	/* memory bank 3 configuration		*/
#endif
#define mem_sdtr1	0x80	/* timing reg 1				*/
#ifndef CONFIG_405EP
#define mem_ecccf	0x94	/* ECC configuration			*/
#define mem_eccerr	0x98	/* ECC error status			*/
#endif

#endif /* CONFIG_SDRAM_PPC4xx_IBM_SDRAM */

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR)

/*
 * Memory controller registers
 */
#define SDRAM_CFG0	0x20	/* memory controller options 0		*/
#define SDRAM_CFG1	0x21	/* memory controller options 1		*/

/*
 * XXX - ToDo: Revisit file to change all these lower case defines into
 * upper case. Also needs to be done in the controller setup code too
 * of course. sr, 2008-06-02
 */
#define mem_besr0_clr	0x0000	/* bus error status reg 0 (clr)		*/
#define mem_besr0_set	0x0004	/* bus error status reg 0 (set)		*/
#define mem_besr1_clr	0x0008	/* bus error status reg 1 (clr)		*/
#define mem_besr1_set	0x000c	/* bus error status reg 1 (set)		*/
#define mem_bear	0x0010	/* bus error address reg		*/
#define mem_mirq_clr	0x0011	/* bus master interrupt (clr)		*/
#define mem_mirq_set	0x0012	/* bus master interrupt (set)		*/
#define mem_slio	0x0018	/* ddr sdram slave interface options	*/
#define mem_cfg0	0x0020	/* ddr sdram options 0			*/
#define mem_cfg1	0x0021	/* ddr sdram options 1			*/
#define mem_devopt	0x0022	/* ddr sdram device options		*/
#define mem_mcsts	0x0024	/* memory controller status		*/
#define mem_rtr		0x0030	/* refresh timer register		*/
#define mem_pmit	0x0034	/* power management idle timer		*/
#define mem_uabba	0x0038	/* plb UABus base address		*/
#define mem_b0cr	0x0040	/* ddr sdram bank 0 configuration	*/
#define mem_b1cr	0x0044	/* ddr sdram bank 1 configuration	*/
#define mem_b2cr	0x0048	/* ddr sdram bank 2 configuration	*/
#define mem_b3cr	0x004c	/* ddr sdram bank 3 configuration	*/
#define mem_tr0		0x0080	/* sdram timing register 0		*/
#define mem_tr1		0x0081	/* sdram timing register 1		*/
#define mem_clktr	0x0082	/* ddr clock timing register		*/
#define mem_wddctr	0x0083	/* write data/dm/dqs clock timing reg	*/
#define mem_dlycal	0x0084	/* delay line calibration register	*/
#define mem_eccesr	0x0098	/* ECC error status			*/

/*
 * Memory Controller Options 0
 */
#define SDRAM_CFG0_DCEN		0x80000000	/* SDRAM Controller Enable	*/
#define SDRAM_CFG0_MCHK_MASK	0x30000000	/* Memory data errchecking mask */
#define SDRAM_CFG0_MCHK_NON	0x00000000	/* No ECC generation		*/
#define SDRAM_CFG0_MCHK_GEN	0x20000000	/* ECC generation		*/
#define SDRAM_CFG0_MCHK_CHK	0x30000000	/* ECC generation and checking	*/
#define SDRAM_CFG0_RDEN		0x08000000	/* Registered DIMM enable	*/
#define SDRAM_CFG0_PMUD		0x04000000	/* Page management unit		*/
#define SDRAM_CFG0_DMWD_MASK	0x02000000	/* DRAM width mask		*/
#define SDRAM_CFG0_DMWD_32	0x00000000	/* 32 bits			*/
#define SDRAM_CFG0_DMWD_64	0x02000000	/* 64 bits			*/
#define SDRAM_CFG0_UIOS_MASK	0x00C00000	/* Unused IO State		*/
#define SDRAM_CFG0_PDP		0x00200000	/* Page deallocation policy	*/

/*
 * Memory Controller Options 1
 */
#define SDRAM_CFG1_SRE		0x80000000	/* Self-Refresh Entry		*/
#define SDRAM_CFG1_PMEN		0x40000000	/* Power Management Enable	*/

/*
 * SDRAM DEVPOT Options
 */
#define SDRAM_DEVOPT_DLL	0x80000000
#define SDRAM_DEVOPT_DS		0x40000000

/*
 * SDRAM MCSTS Options
 */
#define SDRAM_MCSTS_MRSC	0x80000000
#define SDRAM_MCSTS_SRMS	0x40000000
#define SDRAM_MCSTS_CIS		0x20000000

/*
 * SDRAM Refresh Timer Register
 */
#define SDRAM_RTR_RINT_MASK	  0xFFFF0000
#define SDRAM_RTR_RINT_ENCODE(n)  (((n) << 16) & SDRAM_RTR_RINT_MASK)

/*
 * SDRAM UABus Base Address Reg
 */
#define SDRAM_UABBA_UBBA_MASK	0x0000000F

/*
 * Memory Bank 0-7 configuration
 */
#define SDRAM_BXCR_SDBA_MASK	0xff800000	  /* Base address	      */
#define SDRAM_BXCR_SDSZ_MASK	0x000e0000	  /* Size		      */
#define SDRAM_BXCR_SDSZ_8	0x00020000	  /*   8M		      */
#define SDRAM_BXCR_SDSZ_16	0x00040000	  /*  16M		      */
#define SDRAM_BXCR_SDSZ_32	0x00060000	  /*  32M		      */
#define SDRAM_BXCR_SDSZ_64	0x00080000	  /*  64M		      */
#define SDRAM_BXCR_SDSZ_128	0x000a0000	  /* 128M		      */
#define SDRAM_BXCR_SDSZ_256	0x000c0000	  /* 256M		      */
#define SDRAM_BXCR_SDSZ_512	0x000e0000	  /* 512M		      */
#define SDRAM_BXCR_SDAM_MASK	0x0000e000	  /* Addressing mode	      */
#define SDRAM_BXCR_SDAM_1	0x00000000	  /*   Mode 1		      */
#define SDRAM_BXCR_SDAM_2	0x00002000	  /*   Mode 2		      */
#define SDRAM_BXCR_SDAM_3	0x00004000	  /*   Mode 3		      */
#define SDRAM_BXCR_SDAM_4	0x00006000	  /*   Mode 4		      */
#define SDRAM_BXCR_SDBE		0x00000001	  /* Memory Bank Enable	      */

/*
 * SDRAM TR0 Options
 */
#define SDRAM_TR0_SDWR_MASK	0x80000000
#define	 SDRAM_TR0_SDWR_2_CLK	0x00000000
#define	 SDRAM_TR0_SDWR_3_CLK	0x80000000
#define SDRAM_TR0_SDWD_MASK	0x40000000
#define	 SDRAM_TR0_SDWD_0_CLK	0x00000000
#define	 SDRAM_TR0_SDWD_1_CLK	0x40000000
#define SDRAM_TR0_SDCL_MASK	0x01800000
#define	 SDRAM_TR0_SDCL_2_0_CLK 0x00800000
#define	 SDRAM_TR0_SDCL_2_5_CLK 0x01000000
#define	 SDRAM_TR0_SDCL_3_0_CLK 0x01800000
#define SDRAM_TR0_SDPA_MASK	0x000C0000
#define	 SDRAM_TR0_SDPA_2_CLK	0x00040000
#define	 SDRAM_TR0_SDPA_3_CLK	0x00080000
#define	 SDRAM_TR0_SDPA_4_CLK	0x000C0000
#define SDRAM_TR0_SDCP_MASK	0x00030000
#define	 SDRAM_TR0_SDCP_2_CLK	0x00000000
#define	 SDRAM_TR0_SDCP_3_CLK	0x00010000
#define	 SDRAM_TR0_SDCP_4_CLK	0x00020000
#define	 SDRAM_TR0_SDCP_5_CLK	0x00030000
#define SDRAM_TR0_SDLD_MASK	0x0000C000
#define	 SDRAM_TR0_SDLD_1_CLK	0x00000000
#define	 SDRAM_TR0_SDLD_2_CLK	0x00004000
#define SDRAM_TR0_SDRA_MASK	0x0000001C
#define	 SDRAM_TR0_SDRA_6_CLK	0x00000000
#define	 SDRAM_TR0_SDRA_7_CLK	0x00000004
#define	 SDRAM_TR0_SDRA_8_CLK	0x00000008
#define	 SDRAM_TR0_SDRA_9_CLK	0x0000000C
#define	 SDRAM_TR0_SDRA_10_CLK	0x00000010
#define	 SDRAM_TR0_SDRA_11_CLK	0x00000014
#define	 SDRAM_TR0_SDRA_12_CLK	0x00000018
#define	 SDRAM_TR0_SDRA_13_CLK	0x0000001C
#define SDRAM_TR0_SDRD_MASK	0x00000003
#define	 SDRAM_TR0_SDRD_2_CLK	0x00000001
#define	 SDRAM_TR0_SDRD_3_CLK	0x00000002
#define	 SDRAM_TR0_SDRD_4_CLK	0x00000003

/*
 * SDRAM TR1 Options
 */
#define SDRAM_TR1_RDSS_MASK	0xC0000000
#define	 SDRAM_TR1_RDSS_TR0	0x00000000
#define	 SDRAM_TR1_RDSS_TR1	0x40000000
#define	 SDRAM_TR1_RDSS_TR2	0x80000000
#define	 SDRAM_TR1_RDSS_TR3	0xC0000000
#define SDRAM_TR1_RDSL_MASK	0x00C00000
#define	 SDRAM_TR1_RDSL_STAGE1	0x00000000
#define	 SDRAM_TR1_RDSL_STAGE2	0x00400000
#define	 SDRAM_TR1_RDSL_STAGE3	0x00800000
#define SDRAM_TR1_RDCD_MASK	0x00000800
#define	 SDRAM_TR1_RDCD_RCD_0_0 0x00000000
#define	 SDRAM_TR1_RDCD_RCD_1_2 0x00000800
#define SDRAM_TR1_RDCT_MASK	0x000001FF
#define	 SDRAM_TR1_RDCT_ENCODE(x)  (((x) << 0) & SDRAM_TR1_RDCT_MASK)
#define	 SDRAM_TR1_RDCT_DECODE(x)  (((x) & SDRAM_TR1_RDCT_MASK) >> 0)
#define	 SDRAM_TR1_RDCT_MIN	0x00000000
#define	 SDRAM_TR1_RDCT_MAX	0x000001FF

/*
 * SDRAM WDDCTR Options
 */
#define SDRAM_WDDCTR_WRCP_MASK	0xC0000000
#define	 SDRAM_WDDCTR_WRCP_0DEG	  0x00000000
#define	 SDRAM_WDDCTR_WRCP_90DEG  0x40000000
#define	 SDRAM_WDDCTR_WRCP_180DEG 0x80000000
#define SDRAM_WDDCTR_DCD_MASK	0x000001FF

/*
 * SDRAM CLKTR Options
 */
#define SDRAM_CLKTR_CLKP_MASK	0xC0000000
#define	 SDRAM_CLKTR_CLKP_0DEG	  0x00000000
#define	 SDRAM_CLKTR_CLKP_90DEG	  0x40000000
#define	 SDRAM_CLKTR_CLKP_180DEG  0x80000000
#define SDRAM_CLKTR_DCDT_MASK	0x000001FF

/*
 * SDRAM DLYCAL Options
 */
#define SDRAM_DLYCAL_DLCV_MASK	0x000003FC
#define	 SDRAM_DLYCAL_DLCV_ENCODE(x) (((x)<<2) & SDRAM_DLYCAL_DLCV_MASK)
#define	 SDRAM_DLYCAL_DLCV_DECODE(x) (((x) & SDRAM_DLYCAL_DLCV_MASK)>>2)

#endif /* CONFIG_SDRAM_PPC4xx_IBM_DDR */

#if defined(CONFIG_SDRAM_PPC4xx_IBM_DDR2)

#define SDRAM_DLYCAL_DLCV_MASK		0x000003FC
#define SDRAM_DLYCAL_DLCV_ENCODE(x)	(((x)<<2) & SDRAM_DLYCAL_DLCV_MASK)
#define SDRAM_DLYCAL_DLCV_DECODE(x)	(((x) & SDRAM_DLYCAL_DLCV_MASK)>>2)

/*
 * Memory queue defines
 */
#define SDRAMQ_DCR_BASE	0x040

#define SDRAM_R0BAS	(SDRAMQ_DCR_BASE+0x0)	/* rank 0 base address & size  */
#define SDRAM_R1BAS	(SDRAMQ_DCR_BASE+0x1)	/* rank 1 base address & size  */
#define SDRAM_R2BAS	(SDRAMQ_DCR_BASE+0x2)	/* rank 2 base address & size  */
#define SDRAM_R3BAS	(SDRAMQ_DCR_BASE+0x3)	/* rank 3 base address & size  */
#define SDRAM_CONF1HB	(SDRAMQ_DCR_BASE+0x5)	/* configuration 1 HB          */
#define SDRAM_ERRSTATHB	(SDRAMQ_DCR_BASE+0x7)	/* error status HB             */
#define SDRAM_ERRADDUHB	(SDRAMQ_DCR_BASE+0x8)	/* error address upper 32 HB   */
#define SDRAM_ERRADDLHB	(SDRAMQ_DCR_BASE+0x9)	/* error address lower 32 HB   */
#define SDRAM_PLBADDULL	(SDRAMQ_DCR_BASE+0xA)	/* PLB base address upper 32 LL */
#define SDRAM_CONF1LL	(SDRAMQ_DCR_BASE+0xB)	/* configuration 1 LL          */
#define SDRAM_ERRSTATLL	(SDRAMQ_DCR_BASE+0xC)	/* error status LL             */
#define SDRAM_ERRADDULL	(SDRAMQ_DCR_BASE+0xD)	/* error address upper 32 LL   */
#define SDRAM_ERRADDLLL	(SDRAMQ_DCR_BASE+0xE)	/* error address lower 32 LL   */
#define SDRAM_CONFPATHB	(SDRAMQ_DCR_BASE+0xF)	/* configuration between paths */
#define SDRAM_PLBADDUHB	(SDRAMQ_DCR_BASE+0x10)	/* PLB base address upper 32 LL */

#if !defined(CONFIG_405EX)
/*
 * Memory Bank 0-7 configuration
 */
#if defined(CONFIG_440SPE) || \
    defined(CONFIG_460EX) || defined(CONFIG_460GT)
#define SDRAM_RXBAS_SDBA_MASK		0xFFE00000	/* Base address	*/
#define SDRAM_RXBAS_SDBA_ENCODE(n)	((u32)(((phys_size_t)(n) >> 2) & 0xFFE00000))
#define SDRAM_RXBAS_SDBA_DECODE(n)	((((phys_size_t)(n)) & 0xFFE00000) << 2)
#endif /* CONFIG_440SPE */
#if defined(CONFIG_440SP)
#define SDRAM_RXBAS_SDBA_MASK		0xFF800000	/* Base address	*/
#define SDRAM_RXBAS_SDBA_ENCODE(n)	((((u32)(n))&0xFF800000))
#define SDRAM_RXBAS_SDBA_DECODE(n)	((((u32)(n))&0xFF800000))
#endif /* CONFIG_440SP */
#define SDRAM_RXBAS_SDSZ_MASK		0x0000FFC0	/* Size		*/
#define SDRAM_RXBAS_SDSZ_ENCODE(n)	((((u32)(n))&0x3FF)<<6)
#define SDRAM_RXBAS_SDSZ_DECODE(n)	((((u32)(n))>>6)&0x3FF)
#define SDRAM_RXBAS_SDSZ_0		0x00000000	/*   0M		*/
#define SDRAM_RXBAS_SDSZ_8		0x0000FFC0	/*   8M		*/
#define SDRAM_RXBAS_SDSZ_16		0x0000FF80	/*  16M		*/
#define SDRAM_RXBAS_SDSZ_32		0x0000FF00	/*  32M		*/
#define SDRAM_RXBAS_SDSZ_64		0x0000FE00	/*  64M		*/
#define SDRAM_RXBAS_SDSZ_128		0x0000FC00	/* 128M		*/
#define SDRAM_RXBAS_SDSZ_256		0x0000F800	/* 256M		*/
#define SDRAM_RXBAS_SDSZ_512		0x0000F000	/* 512M		*/
#define SDRAM_RXBAS_SDSZ_1024		0x0000E000	/* 1024M	*/
#define SDRAM_RXBAS_SDSZ_2048		0x0000C000	/* 2048M	*/
#define SDRAM_RXBAS_SDSZ_4096		0x00008000	/* 4096M	*/
#else /* CONFIG_405EX */
/*
 * XXX - ToDo:
 * Revisit this file to check if all these 405EX defines are correct and
 * can be used in the common 44x_spd_ddr2 code as well. sr, 2008-06-02
 */
#define SDRAM_RXBAS_SDSZ_MASK		PPC_REG_VAL(19, 0xF)
#define SDRAM_RXBAS_SDSZ_4MB	   	PPC_REG_VAL(19, 0x0)
#define SDRAM_RXBAS_SDSZ_8MB	   	PPC_REG_VAL(19, 0x1)
#define SDRAM_RXBAS_SDSZ_16MB	   	PPC_REG_VAL(19, 0x2)
#define SDRAM_RXBAS_SDSZ_32MB	   	PPC_REG_VAL(19, 0x3)
#define SDRAM_RXBAS_SDSZ_64MB	   	PPC_REG_VAL(19, 0x4)
#define SDRAM_RXBAS_SDSZ_128MB	   	PPC_REG_VAL(19, 0x5)
#define SDRAM_RXBAS_SDSZ_256MB	   	PPC_REG_VAL(19, 0x6)
#define SDRAM_RXBAS_SDSZ_512MB	   	PPC_REG_VAL(19, 0x7)
#define SDRAM_RXBAS_SDSZ_1024MB	   	PPC_REG_VAL(19, 0x8)
#define SDRAM_RXBAS_SDSZ_2048MB	   	PPC_REG_VAL(19, 0x9)
#define SDRAM_RXBAS_SDSZ_4096MB		PPC_REG_VAL(19, 0xA)
#define SDRAM_RXBAS_SDSZ_8192MB		PPC_REG_VAL(19, 0xB)
#define SDRAM_RXBAS_SDSZ_8      	SDRAM_RXBAS_SDSZ_8MB
#define SDRAM_RXBAS_SDSZ_16     	SDRAM_RXBAS_SDSZ_16MB
#define SDRAM_RXBAS_SDSZ_32     	SDRAM_RXBAS_SDSZ_32MB
#define SDRAM_RXBAS_SDSZ_64     	SDRAM_RXBAS_SDSZ_64MB
#define SDRAM_RXBAS_SDSZ_128    	SDRAM_RXBAS_SDSZ_128MB
#define SDRAM_RXBAS_SDSZ_256    	SDRAM_RXBAS_SDSZ_256MB
#define SDRAM_RXBAS_SDSZ_512    	SDRAM_RXBAS_SDSZ_512MB
#define SDRAM_RXBAS_SDSZ_1024		SDRAM_RXBAS_SDSZ_1024MB
#define SDRAM_RXBAS_SDSZ_2048		SDRAM_RXBAS_SDSZ_2048MB
#define SDRAM_RXBAS_SDSZ_4096		SDRAM_RXBAS_SDSZ_4096MB
#define SDRAM_RXBAS_SDSZ_8192		SDRAM_RXBAS_SDSZ_8192MB
#define SDRAM_RXBAS_SDAM_MODE0		PPC_REG_VAL(23, 0x0)
#define SDRAM_RXBAS_SDAM_MODE1		PPC_REG_VAL(23, 0x1)
#define SDRAM_RXBAS_SDAM_MODE2		PPC_REG_VAL(23, 0x2)
#define SDRAM_RXBAS_SDAM_MODE3		PPC_REG_VAL(23, 0x3)
#define SDRAM_RXBAS_SDAM_MODE4		PPC_REG_VAL(23, 0x4)
#define SDRAM_RXBAS_SDAM_MODE5		PPC_REG_VAL(23, 0x5)
#define SDRAM_RXBAS_SDAM_MODE6		PPC_REG_VAL(23, 0x6)
#define SDRAM_RXBAS_SDAM_MODE7		PPC_REG_VAL(23, 0x7)
#define SDRAM_RXBAS_SDAM_MODE8		PPC_REG_VAL(23, 0x8)
#define SDRAM_RXBAS_SDAM_MODE9		PPC_REG_VAL(23, 0x9)
#define SDRAM_RXBAS_SDBE_DISABLE	PPC_REG_VAL(31, 0x0)
#define SDRAM_RXBAS_SDBE_ENABLE		PPC_REG_VAL(31, 0x1)
#endif /* CONFIG_405EX */

/*
 * Memory controller registers
 */
#define SDRAM_MCSTAT	0x14	/* memory controller status                  */
#define SDRAM_MCOPT1	0x20	/* memory controller options 1               */
#define SDRAM_MCOPT2	0x21	/* memory controller options 2               */
#define SDRAM_MODT0	0x22	/* on die termination for bank 0             */
#define SDRAM_MODT1	0x23	/* on die termination for bank 1             */
#define SDRAM_MODT2	0x24	/* on die termination for bank 2             */
#define SDRAM_MODT3	0x25	/* on die termination for bank 3             */
#define SDRAM_CODT	0x26	/* on die termination for controller         */
#define SDRAM_VVPR	0x27	/* variable VRef programmming                */
#define SDRAM_OPARS	0x28	/* on chip driver control setup              */
#define SDRAM_OPART	0x29	/* on chip driver control trigger            */
#define SDRAM_RTR	0x30	/* refresh timer                             */
#define SDRAM_PMIT	0x34	/* power management idle timer               */
#define SDRAM_MB0CF	0x40	/* memory bank 0 configuration               */
#define SDRAM_MB1CF	0x44	/* memory bank 1 configuration               */
#define SDRAM_MB2CF	0x48
#define SDRAM_MB3CF	0x4C
#define SDRAM_INITPLR0	0x50	/* manual initialization control             */
#define SDRAM_INITPLR1	0x51	/* manual initialization control             */
#define SDRAM_INITPLR2	0x52	/* manual initialization control             */
#define SDRAM_INITPLR3	0x53	/* manual initialization control             */
#define SDRAM_INITPLR4	0x54	/* manual initialization control             */
#define SDRAM_INITPLR5	0x55	/* manual initialization control             */
#define SDRAM_INITPLR6	0x56	/* manual initialization control             */
#define SDRAM_INITPLR7	0x57	/* manual initialization control             */
#define SDRAM_INITPLR8	0x58	/* manual initialization control             */
#define SDRAM_INITPLR9	0x59	/* manual initialization control             */
#define SDRAM_INITPLR10	0x5a	/* manual initialization control             */
#define SDRAM_INITPLR11	0x5b	/* manual initialization control             */
#define SDRAM_INITPLR12	0x5c	/* manual initialization control             */
#define SDRAM_INITPLR13	0x5d	/* manual initialization control             */
#define SDRAM_INITPLR14	0x5e	/* manual initialization control             */
#define SDRAM_INITPLR15	0x5f	/* manual initialization control             */
#define SDRAM_RQDC	0x70	/* read DQS delay control                    */
#define SDRAM_RFDC	0x74	/* read feedback delay control               */
#define SDRAM_RDCC	0x78	/* read data capture control                 */
#define SDRAM_DLCR	0x7A	/* delay line calibration                    */
#define SDRAM_CLKTR	0x80	/* DDR clock timing                          */
#define SDRAM_WRDTR	0x81	/* write data, DQS, DM clock, timing         */
#define SDRAM_SDTR1	0x85	/* DDR SDRAM timing 1                        */
#define SDRAM_SDTR2	0x86	/* DDR SDRAM timing 2                        */
#define SDRAM_SDTR3	0x87	/* DDR SDRAM timing 3                        */
#define SDRAM_MMODE	0x88	/* memory mode                               */
#define SDRAM_MEMODE	0x89	/* memory extended mode                      */
#define SDRAM_ECCCR	0x98	/* ECC error status                          */
#define SDRAM_CID	0xA4	/* core ID                                   */
#define SDRAM_RID	0xA8	/* revision ID                               */
#define SDRAM_RTSR	0xB1	/* run time status tracking                  */

/*
 * Memory Controller Status
 */
#define SDRAM_MCSTAT_MIC_MASK		0x80000000	/* Memory init status mask	*/
#define SDRAM_MCSTAT_MIC_NOTCOMP	0x00000000	/* Mem init not complete	*/
#define SDRAM_MCSTAT_MIC_COMP		0x80000000	/* Mem init complete		*/
#define SDRAM_MCSTAT_SRMS_MASK		0x40000000	/* Mem self refresh stat mask	*/
#define SDRAM_MCSTAT_SRMS_NOT_SF	0x00000000	/* Mem not in self refresh	*/
#define SDRAM_MCSTAT_SRMS_SF		0x40000000	/* Mem in self refresh		*/
#define SDRAM_MCSTAT_IDLE_MASK		0x20000000	/* Mem self refresh stat mask	*/
#define SDRAM_MCSTAT_IDLE_NOT		0x00000000	/* Mem contr not idle		*/
#define SDRAM_MCSTAT_IDLE		0x20000000	/* Mem contr idle		*/

/*
 * Memory Controller Options 1
 */
#define SDRAM_MCOPT1_MCHK_MASK		0x30000000 /* Memory data err check mask*/
#define SDRAM_MCOPT1_MCHK_NON		0x00000000 /* No ECC generation		*/
#define SDRAM_MCOPT1_MCHK_GEN		0x20000000 /* ECC generation		*/
#define SDRAM_MCOPT1_MCHK_CHK		0x10000000 /* ECC generation and check	*/
#define SDRAM_MCOPT1_MCHK_CHK_REP	0x30000000 /* ECC generation, chk, report*/
#define SDRAM_MCOPT1_MCHK_CHK_DECODE(n)	((((u32)(n))>>28)&0x3)
#define SDRAM_MCOPT1_RDEN_MASK		0x08000000 /* Registered DIMM mask	*/
#define SDRAM_MCOPT1_RDEN		0x08000000 /* Registered DIMM enable	*/
#define SDRAM_MCOPT1_PMU_MASK		0x06000000 /* Page management unit mask	*/
#define SDRAM_MCOPT1_PMU_CLOSE		0x00000000 /* PMU Close			*/
#define SDRAM_MCOPT1_PMU_OPEN		0x04000000 /* PMU Open			*/
#define SDRAM_MCOPT1_PMU_AUTOCLOSE	0x02000000 /* PMU AutoClose		*/
#define SDRAM_MCOPT1_DMWD_MASK		0x01000000 /* DRAM width mask		*/
#define SDRAM_MCOPT1_DMWD_32		0x00000000 /* 32 bits			*/
#define SDRAM_MCOPT1_DMWD_64		0x01000000 /* 64 bits			*/
#define SDRAM_MCOPT1_UIOS_MASK		0x00C00000 /* Unused IO State		*/
#define SDRAM_MCOPT1_BCNT_MASK		0x00200000 /* Bank count		*/
#define SDRAM_MCOPT1_4_BANKS		0x00000000 /* 4 Banks			*/
#define SDRAM_MCOPT1_8_BANKS		0x00200000 /* 8 Banks			*/
#define SDRAM_MCOPT1_DDR_TYPE_MASK	0x00100000 /* DDR Memory Type mask	*/
#define SDRAM_MCOPT1_DDR1_TYPE		0x00000000 /* DDR1 Memory Type		*/
#define SDRAM_MCOPT1_DDR2_TYPE		0x00100000 /* DDR2 Memory Type		*/
#define SDRAM_MCOPT1_QDEP		0x00020000 /* 4 commands deep		*/
#define SDRAM_MCOPT1_RWOO_MASK		0x00008000 /* Out of Order Read mask	*/
#define SDRAM_MCOPT1_RWOO_DISABLED	0x00000000 /* disabled			*/
#define SDRAM_MCOPT1_RWOO_ENABLED	0x00008000 /* enabled			*/
#define SDRAM_MCOPT1_WOOO_MASK		0x00004000 /* Out of Order Write mask	*/
#define SDRAM_MCOPT1_WOOO_DISABLED	0x00000000 /* disabled			*/
#define SDRAM_MCOPT1_WOOO_ENABLED	0x00004000 /* enabled			*/
#define SDRAM_MCOPT1_DCOO_MASK		0x00002000 /* All Out of Order mask	*/
#define SDRAM_MCOPT1_DCOO_DISABLED	0x00002000 /* disabled			*/
#define SDRAM_MCOPT1_DCOO_ENABLED	0x00000000 /* enabled			*/
#define SDRAM_MCOPT1_DREF_MASK		0x00001000 /* Deferred refresh mask	*/
#define SDRAM_MCOPT1_DREF_NORMAL	0x00000000 /* normal refresh		*/
#define SDRAM_MCOPT1_DREF_DEFER_4	0x00001000 /* defer up to 4 refresh cmd	*/

/*
 * Memory Controller Options 2
 */
#define SDRAM_MCOPT2_SREN_MASK		0x80000000 /* Self Test mask		*/
#define SDRAM_MCOPT2_SREN_EXIT		0x00000000 /* Self Test exit		*/
#define SDRAM_MCOPT2_SREN_ENTER		0x80000000 /* Self Test enter		*/
#define SDRAM_MCOPT2_PMEN_MASK		0x40000000 /* Power Management mask	*/
#define SDRAM_MCOPT2_PMEN_DISABLE	0x00000000 /* disable			*/
#define SDRAM_MCOPT2_PMEN_ENABLE	0x40000000 /* enable			*/
#define SDRAM_MCOPT2_IPTR_MASK		0x20000000 /* Init Trigger Reg mask	*/
#define SDRAM_MCOPT2_IPTR_IDLE		0x00000000 /* idle			*/
#define SDRAM_MCOPT2_IPTR_EXECUTE	0x20000000 /* execute preloaded init	*/
#define SDRAM_MCOPT2_XSRP_MASK		0x10000000 /* Exit Self Refresh Prevent	*/
#define SDRAM_MCOPT2_XSRP_ALLOW		0x00000000 /* allow self refresh exit	*/
#define SDRAM_MCOPT2_XSRP_PREVENT	0x10000000 /* prevent self refresh exit	*/
#define SDRAM_MCOPT2_DCEN_MASK		0x08000000 /* SDRAM Controller Enable	*/
#define SDRAM_MCOPT2_DCEN_DISABLE	0x00000000 /* SDRAM Controller Enable	*/
#define SDRAM_MCOPT2_DCEN_ENABLE	0x08000000 /* SDRAM Controller Enable	*/
#define SDRAM_MCOPT2_ISIE_MASK		0x04000000 /* Init Seq Interruptable mas*/
#define SDRAM_MCOPT2_ISIE_DISABLE	0x00000000 /* disable			*/
#define SDRAM_MCOPT2_ISIE_ENABLE	0x04000000 /* enable			*/

/*
 * SDRAM Refresh Timer Register
 */
#define SDRAM_RTR_RINT_MASK		0xFFF80000
#define SDRAM_RTR_RINT_ENCODE(n)	((((u32)(n))&0xFFF8)<<16)
#define SDRAM_RTR_RINT_DECODE(n)	((((u32)(n))>>16)&0xFFF8)

/*
 * SDRAM Read DQS Delay Control Register
 */
#define SDRAM_RQDC_RQDE_MASK		0x80000000
#define SDRAM_RQDC_RQDE_DISABLE		0x00000000
#define SDRAM_RQDC_RQDE_ENABLE		0x80000000
#define SDRAM_RQDC_RQFD_MASK		0x000001FF
#define SDRAM_RQDC_RQFD_ENCODE(n)	((((u32)(n))&0x1FF)<<0)

#define SDRAM_RQDC_RQFD_MAX		0x1FF

/*
 * SDRAM Read Data Capture Control Register
 */
#define SDRAM_RDCC_RDSS_MASK		0xC0000000
#define SDRAM_RDCC_RDSS_T1		0x00000000
#define SDRAM_RDCC_RDSS_T2		0x40000000
#define SDRAM_RDCC_RDSS_T3		0x80000000
#define SDRAM_RDCC_RDSS_T4		0xC0000000
#define SDRAM_RDCC_RSAE_MASK		0x00000001
#define SDRAM_RDCC_RSAE_DISABLE		0x00000001
#define SDRAM_RDCC_RSAE_ENABLE		0x00000000

/*
 * SDRAM Read Feedback Delay Control Register
 */
#define SDRAM_RFDC_ARSE_MASK		0x80000000
#define SDRAM_RFDC_ARSE_DISABLE		0x80000000
#define SDRAM_RFDC_ARSE_ENABLE		0x00000000
#define SDRAM_RFDC_RFOS_MASK		0x007F0000
#define SDRAM_RFDC_RFOS_ENCODE(n)	((((u32)(n))&0x7F)<<16)
#define SDRAM_RFDC_RFFD_MASK		0x000007FF
#define SDRAM_RFDC_RFFD_ENCODE(n)	((((u32)(n))&0x7FF)<<0)

#define SDRAM_RFDC_RFFD_MAX		0x7FF

/*
 * SDRAM Delay Line Calibration Register
 */
#define SDRAM_DLCR_DCLM_MASK		0x80000000
#define SDRAM_DLCR_DCLM_MANUEL		0x80000000
#define SDRAM_DLCR_DCLM_AUTO		0x00000000
#define SDRAM_DLCR_DLCR_MASK		0x08000000
#define SDRAM_DLCR_DLCR_CALIBRATE	0x08000000
#define SDRAM_DLCR_DLCR_IDLE		0x00000000
#define SDRAM_DLCR_DLCS_MASK		0x07000000
#define SDRAM_DLCR_DLCS_NOT_RUN		0x00000000
#define SDRAM_DLCR_DLCS_IN_PROGRESS	0x01000000
#define SDRAM_DLCR_DLCS_COMPLETE	0x02000000
#define SDRAM_DLCR_DLCS_CONT_DONE	0x03000000
#define SDRAM_DLCR_DLCS_ERROR		0x04000000
#define SDRAM_DLCR_DLCV_MASK		0x000001FF
#define SDRAM_DLCR_DLCV_ENCODE(n)	((((u32)(n))&0x1FF)<<0)
#define SDRAM_DLCR_DLCV_DECODE(n)	((((u32)(n))>>0)&0x1FF)

/*
 * SDRAM Controller On Die Termination Register
 */
#define SDRAM_CODT_ODT_ON			0x80000000
#define SDRAM_CODT_ODT_OFF			0x00000000
#define SDRAM_CODT_DQS_VOLTAGE_DDR_MASK		0x00000020
#define SDRAM_CODT_DQS_2_5_V_DDR1		0x00000000
#define SDRAM_CODT_DQS_1_8_V_DDR2		0x00000020
#define SDRAM_CODT_DQS_MASK			0x00000010
#define SDRAM_CODT_DQS_DIFFERENTIAL		0x00000000
#define SDRAM_CODT_DQS_SINGLE_END		0x00000010
#define SDRAM_CODT_CKSE_DIFFERENTIAL		0x00000000
#define SDRAM_CODT_CKSE_SINGLE_END		0x00000008
#define SDRAM_CODT_FEEBBACK_RCV_SINGLE_END	0x00000004
#define SDRAM_CODT_FEEBBACK_DRV_SINGLE_END	0x00000002
#define SDRAM_CODT_IO_HIZ			0x00000000
#define SDRAM_CODT_IO_NMODE			0x00000001

/*
 * SDRAM Mode Register
 */
#define SDRAM_MMODE_WR_MASK		0x00000E00
#define SDRAM_MMODE_WR_DDR1		0x00000000
#define SDRAM_MMODE_WR_DDR2_3_CYC	0x00000400
#define SDRAM_MMODE_WR_DDR2_4_CYC	0x00000600
#define SDRAM_MMODE_WR_DDR2_5_CYC	0x00000800
#define SDRAM_MMODE_WR_DDR2_6_CYC	0x00000A00
#define SDRAM_MMODE_DCL_MASK		0x00000070
#define SDRAM_MMODE_DCL_DDR1_2_0_CLK	0x00000020
#define SDRAM_MMODE_DCL_DDR1_2_5_CLK	0x00000060
#define SDRAM_MMODE_DCL_DDR1_3_0_CLK	0x00000030
#define SDRAM_MMODE_DCL_DDR2_2_0_CLK	0x00000020
#define SDRAM_MMODE_DCL_DDR2_3_0_CLK	0x00000030
#define SDRAM_MMODE_DCL_DDR2_4_0_CLK	0x00000040
#define SDRAM_MMODE_DCL_DDR2_5_0_CLK	0x00000050
#define SDRAM_MMODE_DCL_DDR2_6_0_CLK	0x00000060
#define SDRAM_MMODE_DCL_DDR2_7_0_CLK	0x00000070

/*
 * SDRAM Extended Mode Register
 */
#define SDRAM_MEMODE_DIC_MASK		0x00000002
#define SDRAM_MEMODE_DIC_NORMAL		0x00000000
#define SDRAM_MEMODE_DIC_WEAK		0x00000002
#define SDRAM_MEMODE_DLL_MASK		0x00000001
#define SDRAM_MEMODE_DLL_DISABLE	0x00000001
#define SDRAM_MEMODE_DLL_ENABLE		0x00000000
#define SDRAM_MEMODE_RTT_MASK		0x00000044
#define SDRAM_MEMODE_RTT_DISABLED	0x00000000
#define SDRAM_MEMODE_RTT_75OHM		0x00000004
#define SDRAM_MEMODE_RTT_150OHM		0x00000040
#define SDRAM_MEMODE_DQS_MASK		0x00000400
#define SDRAM_MEMODE_DQS_DISABLE	0x00000400
#define SDRAM_MEMODE_DQS_ENABLE		0x00000000

/*
 * SDRAM Clock Timing Register
 */
#define SDRAM_CLKTR_CLKP_MASK		0xC0000000
#define SDRAM_CLKTR_CLKP_0_DEG		0x00000000
#define SDRAM_CLKTR_CLKP_180_DEG_ADV	0x80000000
#define SDRAM_CLKTR_CLKP_90_DEG_ADV	0x40000000
#define SDRAM_CLKTR_CLKP_270_DEG_ADV	0xC0000000

/*
 * SDRAM Write Timing Register
 */
#define SDRAM_WRDTR_LLWP_MASK		0x10000000
#define SDRAM_WRDTR_LLWP_DIS		0x10000000
#define SDRAM_WRDTR_LLWP_1_CYC		0x00000000
#define SDRAM_WRDTR_WTR_MASK		0x0E000000
#define SDRAM_WRDTR_WTR_0_DEG		0x06000000
#define SDRAM_WRDTR_WTR_90_DEG_ADV	0x04000000
#define SDRAM_WRDTR_WTR_180_DEG_ADV	0x02000000
#define SDRAM_WRDTR_WTR_270_DEG_ADV	0x00000000

/*
 * SDRAM SDTR1 Options
 */
#define SDRAM_SDTR1_LDOF_MASK		0x80000000
#define SDRAM_SDTR1_LDOF_1_CLK		0x00000000
#define SDRAM_SDTR1_LDOF_2_CLK		0x80000000
#define SDRAM_SDTR1_RTW_MASK		0x00F00000
#define SDRAM_SDTR1_RTW_2_CLK		0x00200000
#define SDRAM_SDTR1_RTW_3_CLK		0x00300000
#define SDRAM_SDTR1_WTWO_MASK		0x000F0000
#define SDRAM_SDTR1_WTWO_0_CLK		0x00000000
#define SDRAM_SDTR1_WTWO_1_CLK		0x00010000
#define SDRAM_SDTR1_RTRO_MASK		0x0000F000
#define SDRAM_SDTR1_RTRO_1_CLK		0x00001000
#define SDRAM_SDTR1_RTRO_2_CLK		0x00002000

/*
 * SDRAM SDTR2 Options
 */
#define SDRAM_SDTR2_RCD_MASK		0xF0000000
#define SDRAM_SDTR2_RCD_1_CLK		0x10000000
#define SDRAM_SDTR2_RCD_2_CLK		0x20000000
#define SDRAM_SDTR2_RCD_3_CLK		0x30000000
#define SDRAM_SDTR2_RCD_4_CLK		0x40000000
#define SDRAM_SDTR2_RCD_5_CLK		0x50000000
#define SDRAM_SDTR2_WTR_MASK		0x0F000000
#define SDRAM_SDTR2_WTR_1_CLK		0x01000000
#define SDRAM_SDTR2_WTR_2_CLK		0x02000000
#define SDRAM_SDTR2_WTR_3_CLK		0x03000000
#define SDRAM_SDTR2_WTR_4_CLK		0x04000000
#define SDRAM_SDTR3_WTR_ENCODE(n)	((((u32)(n))&0xF)<<24)
#define SDRAM_SDTR2_XSNR_MASK		0x00FF0000
#define SDRAM_SDTR2_XSNR_8_CLK		0x00080000
#define SDRAM_SDTR2_XSNR_16_CLK		0x00100000
#define SDRAM_SDTR2_XSNR_32_CLK		0x00200000
#define SDRAM_SDTR2_XSNR_64_CLK		0x00400000
#define SDRAM_SDTR2_WPC_MASK		0x0000F000
#define SDRAM_SDTR2_WPC_2_CLK		0x00002000
#define SDRAM_SDTR2_WPC_3_CLK		0x00003000
#define SDRAM_SDTR2_WPC_4_CLK		0x00004000
#define SDRAM_SDTR2_WPC_5_CLK		0x00005000
#define SDRAM_SDTR2_WPC_6_CLK		0x00006000
#define SDRAM_SDTR3_WPC_ENCODE(n)	((((u32)(n))&0xF)<<12)
#define SDRAM_SDTR2_RPC_MASK		0x00000F00
#define SDRAM_SDTR2_RPC_2_CLK		0x00000200
#define SDRAM_SDTR2_RPC_3_CLK		0x00000300
#define SDRAM_SDTR2_RPC_4_CLK		0x00000400
#define SDRAM_SDTR2_RP_MASK		0x000000F0
#define SDRAM_SDTR2_RP_3_CLK		0x00000030
#define SDRAM_SDTR2_RP_4_CLK		0x00000040
#define SDRAM_SDTR2_RP_5_CLK		0x00000050
#define SDRAM_SDTR2_RP_6_CLK		0x00000060
#define SDRAM_SDTR2_RP_7_CLK		0x00000070
#define SDRAM_SDTR2_RRD_MASK		0x0000000F
#define SDRAM_SDTR2_RRD_2_CLK		0x00000002
#define SDRAM_SDTR2_RRD_3_CLK		0x00000003

/*
 * SDRAM SDTR3 Options
 */
#define SDRAM_SDTR3_RAS_MASK		0x1F000000
#define SDRAM_SDTR3_RAS_ENCODE(n)	((((u32)(n))&0x1F)<<24)
#define SDRAM_SDTR3_RC_MASK		0x001F0000
#define SDRAM_SDTR3_RC_ENCODE(n)	((((u32)(n))&0x1F)<<16)
#define SDRAM_SDTR3_XCS_MASK		0x00001F00
#define SDRAM_SDTR3_XCS			0x00000D00
#define SDRAM_SDTR3_RFC_MASK		0x0000003F
#define SDRAM_SDTR3_RFC_ENCODE(n)	((((u32)(n))&0x3F)<<0)

/*
 * Memory Bank 0-1 configuration
 */
#define SDRAM_BXCF_M_AM_MASK		0x00000F00	/* Addressing mode	*/
#define SDRAM_BXCF_M_AM_0		0x00000000	/*   Mode 0		*/
#define SDRAM_BXCF_M_AM_1		0x00000100	/*   Mode 1		*/
#define SDRAM_BXCF_M_AM_2		0x00000200	/*   Mode 2		*/
#define SDRAM_BXCF_M_AM_3		0x00000300	/*   Mode 3		*/
#define SDRAM_BXCF_M_AM_4		0x00000400	/*   Mode 4		*/
#define SDRAM_BXCF_M_AM_5		0x00000500	/*   Mode 5		*/
#define SDRAM_BXCF_M_AM_6		0x00000600	/*   Mode 6		*/
#define SDRAM_BXCF_M_AM_7		0x00000700	/*   Mode 7		*/
#define SDRAM_BXCF_M_AM_8		0x00000800	/*   Mode 8		*/
#define SDRAM_BXCF_M_AM_9		0x00000900	/*   Mode 9		*/
#define SDRAM_BXCF_M_BE_MASK		0x00000001	/* Memory Bank Enable	*/
#define SDRAM_BXCF_M_BE_DISABLE		0x00000000	/* Memory Bank Enable	*/
#define SDRAM_BXCF_M_BE_ENABLE		0x00000001	/* Memory Bank Enable	*/

#define SDRAM_RTSR_TRK1SM_MASK		0xC0000000	/* Tracking State Mach 1*/
#define SDRAM_RTSR_TRK1SM_ATBASE	0x00000000	/* atbase state		*/
#define SDRAM_RTSR_TRK1SM_MISSED	0x40000000	/* missed state		*/
#define SDRAM_RTSR_TRK1SM_ATPLS1	0x80000000	/* atpls1 state		*/
#define SDRAM_RTSR_TRK1SM_RESET		0xC0000000	/* reset  state		*/

#define SDR0_MFR_FIXD			0x10000000	/* Workaround for PCI/DMA */

#endif /* CONFIG_SDRAM_PPC4xx_IBM_DDR2 */

#if defined(CONFIG_SDRAM_PPC4xx_DENALI_DDR2)
/*
 * SDRAM Controller
 */
#define DDR0_00				0x00
#define DDR0_00_INT_ACK_MASK		0x7F000000	/* Write only */
#define DDR0_00_INT_ACK_ALL		0x7F000000
#define DDR0_00_INT_ACK_ENCODE(n)	((((u32)(n))&0x7F)<<24)
#define DDR0_00_INT_ACK_DECODE(n)	((((u32)(n))>>24)&0x7F)
/* Status */
#define DDR0_00_INT_STATUS_MASK		0x00FF0000	/* Read only */
/* Bit0. A single access outside the defined PHYSICAL memory space detected. */
#define DDR0_00_INT_STATUS_BIT0		0x00010000
/* Bit1. Multiple accesses outside the defined PHYSICAL memory space detected. */
#define DDR0_00_INT_STATUS_BIT1		0x00020000
/* Bit2. Single correctable ECC event detected */
#define DDR0_00_INT_STATUS_BIT2		0x00040000
/* Bit3. Multiple correctable ECC events detected. */
#define DDR0_00_INT_STATUS_BIT3		0x00080000
/* Bit4. Single uncorrectable ECC event detected. */
#define DDR0_00_INT_STATUS_BIT4		0x00100000
/* Bit5. Multiple uncorrectable ECC events detected. */
#define DDR0_00_INT_STATUS_BIT5		0x00200000
/* Bit6. DRAM initialization complete. */
#define DDR0_00_INT_STATUS_BIT6		0x00400000
/* Bit7. Logical OR of all lower bits. */
#define DDR0_00_INT_STATUS_BIT7		0x00800000

#define DDR0_00_INT_STATUS_ENCODE(n)	((((u32)(n))&0xFF)<<16)
#define DDR0_00_INT_STATUS_DECODE(n)	((((u32)(n))>>16)&0xFF)
#define DDR0_00_DLL_INCREMENT_MASK	0x00007F00
#define DDR0_00_DLL_INCREMENT_ENCODE(n)	((((u32)(n))&0x7F)<<8)
#define DDR0_00_DLL_INCREMENT_DECODE(n)	((((u32)(n))>>8)&0x7F)
#define DDR0_00_DLL_START_POINT_MASK	0x0000007F
#define DDR0_00_DLL_START_POINT_ENCODE(n) ((((u32)(n))&0x7F)<<0)
#define DDR0_00_DLL_START_POINT_DECODE(n) ((((u32)(n))>>0)&0x7F)

#define DDR0_01				0x01
#define DDR0_01_PLB0_DB_CS_LOWER_MASK	0x1F000000
#define DDR0_01_PLB0_DB_CS_LOWER_ENCODE(n) ((((u32)(n))&0x1F)<<24)
#define DDR0_01_PLB0_DB_CS_LOWER_DECODE(n) ((((u32)(n))>>24)&0x1F)
#define DDR0_01_PLB0_DB_CS_UPPER_MASK	0x001F0000
#define DDR0_01_PLB0_DB_CS_UPPER_ENCODE(n) ((((u32)(n))&0x1F)<<16)
#define DDR0_01_PLB0_DB_CS_UPPER_DECODE(n) ((((u32)(n))>>16)&0x1F)
#define DDR0_01_OUT_OF_RANGE_TYPE_MASK	0x00000700	/* Read only */
#define DDR0_01_OUT_OF_RANGE_TYPE_ENCODE(n) ((((u32)(n))&0x7)<<8)
#define DDR0_01_OUT_OF_RANGE_TYPE_DECODE(n) ((((u32)(n))>>8)&0x7)
#define DDR0_01_INT_MASK_MASK		0x000000FF
#define DDR0_01_INT_MASK_ENCODE(n)	((((u32)(n))&0xFF)<<0)
#define DDR0_01_INT_MASK_DECODE(n)	((((u32)(n))>>0)&0xFF)
#define DDR0_01_INT_MASK_ALL_ON		0x000000FF
#define DDR0_01_INT_MASK_ALL_OFF	0x00000000

#define DDR0_02				0x02
#define DDR0_02_MAX_CS_REG_MASK		0x02000000	/* Read only */
#define DDR0_02_MAX_CS_REG_ENCODE(n)	((((u32)(n))&0x2)<<24)
#define DDR0_02_MAX_CS_REG_DECODE(n)	((((u32)(n))>>24)&0x2)
#define DDR0_02_MAX_COL_REG_MASK	0x000F0000	/* Read only */
#define DDR0_02_MAX_COL_REG_ENCODE(n)	((((u32)(n))&0xF)<<16)
#define DDR0_02_MAX_COL_REG_DECODE(n)	((((u32)(n))>>16)&0xF)
#define DDR0_02_MAX_ROW_REG_MASK	0x00000F00	/* Read only */
#define DDR0_02_MAX_ROW_REG_ENCODE(n)	((((u32)(n))&0xF)<<8)
#define DDR0_02_MAX_ROW_REG_DECODE(n)	((((u32)(n))>>8)&0xF)
#define DDR0_02_START_MASK		0x00000001
#define DDR0_02_START_ENCODE(n)		((((u32)(n))&0x1)<<0)
#define DDR0_02_START_DECODE(n)		((((u32)(n))>>0)&0x1)
#define DDR0_02_START_OFF		0x00000000
#define DDR0_02_START_ON		0x00000001

#define DDR0_03				0x03
#define DDR0_03_BSTLEN_MASK		0x07000000
#define DDR0_03_BSTLEN_ENCODE(n)	((((u32)(n))&0x7)<<24)
#define DDR0_03_BSTLEN_DECODE(n)	((((u32)(n))>>24)&0x7)
#define DDR0_03_CASLAT_MASK		0x00070000
#define DDR0_03_CASLAT_ENCODE(n)	((((u32)(n))&0x7)<<16)
#define DDR0_03_CASLAT_DECODE(n)	((((u32)(n))>>16)&0x7)
#define DDR0_03_CASLAT_LIN_MASK		0x00000F00
#define DDR0_03_CASLAT_LIN_ENCODE(n)	((((u32)(n))&0xF)<<8)
#define DDR0_03_CASLAT_LIN_DECODE(n)	((((u32)(n))>>8)&0xF)
#define DDR0_03_INITAREF_MASK		0x0000000F
#define DDR0_03_INITAREF_ENCODE(n)	((((u32)(n))&0xF)<<0)
#define DDR0_03_INITAREF_DECODE(n)	((((u32)(n))>>0)&0xF)

#define DDR0_04				0x04
#define DDR0_04_TRC_MASK		0x1F000000
#define DDR0_04_TRC_ENCODE(n)		((((u32)(n))&0x1F)<<24)
#define DDR0_04_TRC_DECODE(n)		((((u32)(n))>>24)&0x1F)
#define DDR0_04_TRRD_MASK		0x00070000
#define DDR0_04_TRRD_ENCODE(n)		((((u32)(n))&0x7)<<16)
#define DDR0_04_TRRD_DECODE(n)		((((u32)(n))>>16)&0x7)
#define DDR0_04_TRTP_MASK		0x00000700
#define DDR0_04_TRTP_ENCODE(n)		((((u32)(n))&0x7)<<8)
#define DDR0_04_TRTP_DECODE(n)		((((u32)(n))>>8)&0x7)

#define DDR0_05				0x05
#define DDR0_05_TMRD_MASK		0x1F000000
#define DDR0_05_TMRD_ENCODE(n)		((((u32)(n))&0x1F)<<24)
#define DDR0_05_TMRD_DECODE(n)		((((u32)(n))>>24)&0x1F)
#define DDR0_05_TEMRS_MASK		0x00070000
#define DDR0_05_TEMRS_ENCODE(n)		((((u32)(n))&0x7)<<16)
#define DDR0_05_TEMRS_DECODE(n)		((((u32)(n))>>16)&0x7)
#define DDR0_05_TRP_MASK		0x00000F00
#define DDR0_05_TRP_ENCODE(n)		((((u32)(n))&0xF)<<8)
#define DDR0_05_TRP_DECODE(n)		((((u32)(n))>>8)&0xF)
#define DDR0_05_TRAS_MIN_MASK		0x000000FF
#define DDR0_05_TRAS_MIN_ENCODE(n)	((((u32)(n))&0xFF)<<0)
#define DDR0_05_TRAS_MIN_DECODE(n)	((((u32)(n))>>0)&0xFF)

#define DDR0_06				0x06
#define DDR0_06_WRITEINTERP_MASK	0x01000000
#define DDR0_06_WRITEINTERP_ENCODE(n)	((((u32)(n))&0x1)<<24)
#define DDR0_06_WRITEINTERP_DECODE(n)	((((u32)(n))>>24)&0x1)
#define DDR0_06_TWTR_MASK		0x00070000
#define DDR0_06_TWTR_ENCODE(n)		((((u32)(n))&0x7)<<16)
#define DDR0_06_TWTR_DECODE(n)		((((u32)(n))>>16)&0x7)
#define DDR0_06_TDLL_MASK		0x0000FF00
#define DDR0_06_TDLL_ENCODE(n)		((((u32)(n))&0xFF)<<8)
#define DDR0_06_TDLL_DECODE(n)		((((u32)(n))>>8)&0xFF)
#define DDR0_06_TRFC_MASK		0x0000007F
#define DDR0_06_TRFC_ENCODE(n)		((((u32)(n))&0x7F)<<0)
#define DDR0_06_TRFC_DECODE(n)		((((u32)(n))>>0)&0x7F)

#define DDR0_07				0x07
#define DDR0_07_NO_CMD_INIT_MASK	0x01000000
#define DDR0_07_NO_CMD_INIT_ENCODE(n)	((((u32)(n))&0x1)<<24)
#define DDR0_07_NO_CMD_INIT_DECODE(n)	((((u32)(n))>>24)&0x1)
#define DDR0_07_TFAW_MASK		0x001F0000
#define DDR0_07_TFAW_ENCODE(n)		((((u32)(n))&0x1F)<<16)
#define DDR0_07_TFAW_DECODE(n)		((((u32)(n))>>16)&0x1F)
#define DDR0_07_AUTO_REFRESH_MODE_MASK	0x00000100
#define DDR0_07_AUTO_REFRESH_MODE_ENCODE(n) ((((u32)(n))&0x1)<<8)
#define DDR0_07_AUTO_REFRESH_MODE_DECODE(n) ((((u32)(n))>>8)&0x1)
#define DDR0_07_AREFRESH_MASK		0x00000001
#define DDR0_07_AREFRESH_ENCODE(n)	((((u32)(n))&0x1)<<0)
#define DDR0_07_AREFRESH_DECODE(n)	((((u32)(n))>>0)&0x1)

#define DDR0_08				0x08
#define DDR0_08_WRLAT_MASK		0x07000000
#define DDR0_08_WRLAT_ENCODE(n)		((((u32)(n))&0x7)<<24)
#define DDR0_08_WRLAT_DECODE(n)		((((u32)(n))>>24)&0x7)
#define DDR0_08_TCPD_MASK		0x00FF0000
#define DDR0_08_TCPD_ENCODE(n)		((((u32)(n))&0xFF)<<16)
#define DDR0_08_TCPD_DECODE(n)		((((u32)(n))>>16)&0xFF)
#define DDR0_08_DQS_N_EN_MASK		0x00000100
#define DDR0_08_DQS_N_EN_ENCODE(n)	((((u32)(n))&0x1)<<8)
#define DDR0_08_DQS_N_EN_DECODE(n)	((((u32)(n))>>8)&0x1)
#define DDR0_08_DDRII_SDRAM_MODE_MASK	0x00000001
#define DDR0_08_DDRII_ENCODE(n)		((((u32)(n))&0x1)<<0)
#define DDR0_08_DDRII_DECODE(n)		((((u32)(n))>>0)&0x1)

#define DDR0_09				0x09
#define DDR0_09_OCD_ADJUST_PDN_CS_0_MASK 0x1F000000
#define DDR0_09_OCD_ADJUST_PDN_CS_0_ENCODE(n) ((((u32)(n))&0x1F)<<24)
#define DDR0_09_OCD_ADJUST_PDN_CS_0_DECODE(n) ((((u32)(n))>>24)&0x1F)
#define DDR0_09_RTT_0_MASK		0x00030000
#define DDR0_09_RTT_0_ENCODE(n)		((((u32)(n))&0x3)<<16)
#define DDR0_09_RTT_0_DECODE(n)		((((u32)(n))>>16)&0x3)
#define DDR0_09_WR_DQS_SHIFT_BYPASS_MASK  0x00007F00
#define DDR0_09_WR_DQS_SHIFT_BYPASS_ENCODE(n) ((((u32)(n))&0x7F)<<8)
#define DDR0_09_WR_DQS_SHIFT_BYPASS_DECODE(n) ((((u32)(n))>>8)&0x7F)
#define DDR0_09_WR_DQS_SHIFT_MASK	0x0000007F
#define DDR0_09_WR_DQS_SHIFT_ENCODE(n)	((((u32)(n))&0x7F)<<0)
#define DDR0_09_WR_DQS_SHIFT_DECODE(n)	((((u32)(n))>>0)&0x7F)

#define DDR0_10				0x0A
#define DDR0_10_WRITE_MODEREG_MASK	0x00010000	/* Write only */
#define DDR0_10_WRITE_MODEREG_ENCODE(n)	((((u32)(n))&0x1)<<16)
#define DDR0_10_WRITE_MODEREG_DECODE(n)	((((u32)(n))>>16)&0x1)
#define DDR0_10_CS_MAP_MASK		0x00000300
#define DDR0_10_CS_MAP_NO_MEM		0x00000000
#define DDR0_10_CS_MAP_RANK0_INSTALLED	0x00000100
#define DDR0_10_CS_MAP_RANK1_INSTALLED	0x00000200
#define DDR0_10_CS_MAP_ENCODE(n)	((((u32)(n))&0x3)<<8)
#define DDR0_10_CS_MAP_DECODE(n)	((((u32)(n))>>8)&0x3)
#define DDR0_10_OCD_ADJUST_PUP_CS_0_MASK  0x0000001F
#define DDR0_10_OCD_ADJUST_PUP_CS_0_ENCODE(n) ((((u32)(n))&0x1F)<<0)
#define DDR0_10_OCD_ADJUST_PUP_CS_0_DECODE(n) ((((u32)(n))>>0)&0x1F)

#define DDR0_11				0x0B
#define DDR0_11_SREFRESH_MASK		0x01000000
#define DDR0_11_SREFRESH_ENCODE(n)	((((u32)(n))&0x1)<<24)
#define DDR0_11_SREFRESH_DECODE(n)	((((u32)(n))>>24)&0x1F)
#define DDR0_11_TXSNR_MASK		0x00FF0000
#define DDR0_11_TXSNR_ENCODE(n)		((((u32)(n))&0xFF)<<16)
#define DDR0_11_TXSNR_DECODE(n)		((((u32)(n))>>16)&0xFF)
#define DDR0_11_TXSR_MASK		0x0000FF00
#define DDR0_11_TXSR_ENCODE(n)		((((u32)(n))&0xFF)<<8)
#define DDR0_11_TXSR_DECODE(n)		((((u32)(n))>>8)&0xFF)

#define DDR0_12				0x0C
#define DDR0_12_TCKE_MASK		0x0000007
#define DDR0_12_TCKE_ENCODE(n)		((((u32)(n))&0x7)<<0)
#define DDR0_12_TCKE_DECODE(n)		((((u32)(n))>>0)&0x7)

#define DDR0_14				0x0E
#define DDR0_14_DLL_BYPASS_MODE_MASK	0x01000000
#define DDR0_14_DLL_BYPASS_MODE_ENCODE(n) ((((u32)(n))&0x1)<<24)
#define DDR0_14_DLL_BYPASS_MODE_DECODE(n) ((((u32)(n))>>24)&0x1)
#define DDR0_14_REDUC_MASK		0x00010000
#define DDR0_14_REDUC_64BITS		0x00000000
#define DDR0_14_REDUC_32BITS		0x00010000
#define DDR0_14_REDUC_ENCODE(n)		((((u32)(n))&0x1)<<16)
#define DDR0_14_REDUC_DECODE(n)		((((u32)(n))>>16)&0x1)
#define DDR0_14_REG_DIMM_ENABLE_MASK	0x00000100
#define DDR0_14_REG_DIMM_ENABLE_ENCODE(n) ((((u32)(n))&0x1)<<8)
#define DDR0_14_REG_DIMM_ENABLE_DECODE(n) ((((u32)(n))>>8)&0x1)

#define DDR0_17				0x11
#define DDR0_17_DLL_DQS_DELAY_0_MASK	0x7F000000
#define DDR0_17_DLL_DQS_DELAY_0_ENCODE(n) ((((u32)(n))&0x7F)<<24)
#define DDR0_17_DLL_DQS_DELAY_0_DECODE(n) ((((u32)(n))>>24)&0x7F)
#define DDR0_17_DLLLOCKREG_MASK		0x00010000	/* Read only */
#define DDR0_17_DLLLOCKREG_LOCKED	0x00010000
#define DDR0_17_DLLLOCKREG_UNLOCKED	0x00000000
#define DDR0_17_DLLLOCKREG_ENCODE(n)	((((u32)(n))&0x1)<<16)
#define DDR0_17_DLLLOCKREG_DECODE(n)	((((u32)(n))>>16)&0x1)
#define DDR0_17_DLL_LOCK_MASK		0x00007F00	/* Read only */
#define DDR0_17_DLL_LOCK_ENCODE(n)	((((u32)(n))&0x7F)<<8)
#define DDR0_17_DLL_LOCK_DECODE(n)	((((u32)(n))>>8)&0x7F)

#define DDR0_18				0x12
#define DDR0_18_DLL_DQS_DELAY_X_MASK	0x7F7F7F7F
#define DDR0_18_DLL_DQS_DELAY_4_MASK	0x7F000000
#define DDR0_18_DLL_DQS_DELAY_4_ENCODE(n) ((((u32)(n))&0x7F)<<24)
#define DDR0_18_DLL_DQS_DELAY_4_DECODE(n) ((((u32)(n))>>24)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_3_MASK	0x007F0000
#define DDR0_18_DLL_DQS_DELAY_3_ENCODE(n) ((((u32)(n))&0x7F)<<16)
#define DDR0_18_DLL_DQS_DELAY_3_DECODE(n) ((((u32)(n))>>16)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_2_MASK	0x00007F00
#define DDR0_18_DLL_DQS_DELAY_2_ENCODE(n) ((((u32)(n))&0x7F)<<8)
#define DDR0_18_DLL_DQS_DELAY_2_DECODE(n) ((((u32)(n))>>8)&0x7F)
#define DDR0_18_DLL_DQS_DELAY_1_MASK	0x0000007F
#define DDR0_18_DLL_DQS_DELAY_1_ENCODE(n) ((((u32)(n))&0x7F)<<0)
#define DDR0_18_DLL_DQS_DELAY_1_DECODE(n) ((((u32)(n))>>0)&0x7F)

#define DDR0_19				0x13
#define DDR0_19_DLL_DQS_DELAY_X_MASK	0x7F7F7F7F
#define DDR0_19_DLL_DQS_DELAY_8_MASK	0x7F000000
#define DDR0_19_DLL_DQS_DELAY_8_ENCODE(n) ((((u32)(n))&0x7F)<<24)
#define DDR0_19_DLL_DQS_DELAY_8_DECODE(n) ((((u32)(n))>>24)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_7_MASK	0x007F0000
#define DDR0_19_DLL_DQS_DELAY_7_ENCODE(n) ((((u32)(n))&0x7F)<<16)
#define DDR0_19_DLL_DQS_DELAY_7_DECODE(n) ((((u32)(n))>>16)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_6_MASK	0x00007F00
#define DDR0_19_DLL_DQS_DELAY_6_ENCODE(n) ((((u32)(n))&0x7F)<<8)
#define DDR0_19_DLL_DQS_DELAY_6_DECODE(n) ((((u32)(n))>>8)&0x7F)
#define DDR0_19_DLL_DQS_DELAY_5_MASK	0x0000007F
#define DDR0_19_DLL_DQS_DELAY_5_ENCODE(n) ((((u32)(n))&0x7F)<<0)
#define DDR0_19_DLL_DQS_DELAY_5_DECODE(n) ((((u32)(n))>>0)&0x7F)

#define DDR0_20				0x14
#define DDR0_20_DLL_DQS_BYPASS_3_MASK	0x7F000000
#define DDR0_20_DLL_DQS_BYPASS_3_ENCODE(n) ((((u32)(n))&0x7F)<<24)
#define DDR0_20_DLL_DQS_BYPASS_3_DECODE(n) ((((u32)(n))>>24)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_2_MASK	0x007F0000
#define DDR0_20_DLL_DQS_BYPASS_2_ENCODE(n) ((((u32)(n))&0x7F)<<16)
#define DDR0_20_DLL_DQS_BYPASS_2_DECODE(n) ((((u32)(n))>>16)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_1_MASK	0x00007F00
#define DDR0_20_DLL_DQS_BYPASS_1_ENCODE(n) ((((u32)(n))&0x7F)<<8)
#define DDR0_20_DLL_DQS_BYPASS_1_DECODE(n) ((((u32)(n))>>8)&0x7F)
#define DDR0_20_DLL_DQS_BYPASS_0_MASK	0x0000007F
#define DDR0_20_DLL_DQS_BYPASS_0_ENCODE(n) ((((u32)(n))&0x7F)<<0)
#define DDR0_20_DLL_DQS_BYPASS_0_DECODE(n) ((((u32)(n))>>0)&0x7F)

#define DDR0_21				0x15
#define DDR0_21_DLL_DQS_BYPASS_7_MASK	0x7F000000
#define DDR0_21_DLL_DQS_BYPASS_7_ENCODE(n) ((((u32)(n))&0x7F)<<24)
#define DDR0_21_DLL_DQS_BYPASS_7_DECODE(n) ((((u32)(n))>>24)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_6_MASK	0x007F0000
#define DDR0_21_DLL_DQS_BYPASS_6_ENCODE(n) ((((u32)(n))&0x7F)<<16)
#define DDR0_21_DLL_DQS_BYPASS_6_DECODE(n) ((((u32)(n))>>16)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_5_MASK	0x00007F00
#define DDR0_21_DLL_DQS_BYPASS_5_ENCODE(n) ((((u32)(n))&0x7F)<<8)
#define DDR0_21_DLL_DQS_BYPASS_5_DECODE(n) ((((u32)(n))>>8)&0x7F)
#define DDR0_21_DLL_DQS_BYPASS_4_MASK	0x0000007F
#define DDR0_21_DLL_DQS_BYPASS_4_ENCODE(n) ((((u32)(n))&0x7F)<<0)
#define DDR0_21_DLL_DQS_BYPASS_4_DECODE(n) ((((u32)(n))>>0)&0x7F)

#define DDR0_22				0x16
#define DDR0_22_CTRL_RAW_MASK		0x03000000
#define DDR0_22_CTRL_RAW_ECC_DISABLE	0x00000000
#define DDR0_22_CTRL_RAW_ECC_CHECK_ONLY	0x01000000
#define DDR0_22_CTRL_RAW_NO_ECC_RAM	0x02000000
#define DDR0_22_CTRL_RAW_ECC_ENABLE	0x03000000
#define DDR0_22_CTRL_RAW_ENCODE(n)	((((u32)(n))&0x3)<<24)
#define DDR0_22_CTRL_RAW_DECODE(n)	((((u32)(n))>>24)&0x3)
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_MASK 0x007F0000
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_ENCODE(n) ((((u32)(n))&0x7F)<<16)
#define DDR0_22_DQS_OUT_SHIFT_BYPASS_DECODE(n) ((((u32)(n))>>16)&0x7F)
#define DDR0_22_DQS_OUT_SHIFT_MASK	0x00007F00
#define DDR0_22_DQS_OUT_SHIFT_ENCODE(n)	((((u32)(n))&0x7F)<<8)
#define DDR0_22_DQS_OUT_SHIFT_DECODE(n)	((((u32)(n))>>8)&0x7F)
#define DDR0_22_DLL_DQS_BYPASS_8_MASK	0x0000007F
#define DDR0_22_DLL_DQS_BYPASS_8_ENCODE(n) ((((u32)(n))&0x7F)<<0)
#define DDR0_22_DLL_DQS_BYPASS_8_DECODE(n) ((((u32)(n))>>0)&0x7F)

#define DDR0_23				0x17
#define DDR0_23_ODT_RD_MAP_CS0_MASK	0x03000000
#define DDR0_23_ODT_RD_MAP_CS0_ENCODE(n) ((((u32)(n))&0x3)<<24)
#define DDR0_23_ODT_RD_MAP_CS0_DECODE(n) ((((u32)(n))>>24)&0x3)
#define DDR0_23_ECC_C_SYND_MASK		0x00FF0000	/* Read only */
#define DDR0_23_ECC_C_SYND_ENCODE(n)	((((u32)(n))&0xFF)<<16)
#define DDR0_23_ECC_C_SYND_DECODE(n)	((((u32)(n))>>16)&0xFF)
#define DDR0_23_ECC_U_SYND_MASK		0x0000FF00	/* Read only */
#define DDR0_23_ECC_U_SYND_ENCODE(n)	((((u32)(n))&0xFF)<<8)
#define DDR0_23_ECC_U_SYND_DECODE(n)	((((u32)(n))>>8)&0xFF)
#define DDR0_23_FWC_MASK		0x00000001	/* Write only */
#define DDR0_23_FWC_ENCODE(n)		((((u32)(n))&0x1)<<0)
#define DDR0_23_FWC_DECODE(n)		((((u32)(n))>>0)&0x1)

#define DDR0_24				0x18
#define DDR0_24_RTT_PAD_TERMINATION_MASK 0x03000000
#define DDR0_24_RTT_PAD_TERMINATION_ENCODE(n) ((((u32)(n))&0x3)<<24)
#define DDR0_24_RTT_PAD_TERMINATION_DECODE(n) ((((u32)(n))>>24)&0x3)
#define DDR0_24_ODT_WR_MAP_CS1_MASK	0x00030000
#define DDR0_24_ODT_WR_MAP_CS1_ENCODE(n) ((((u32)(n))&0x3)<<16)
#define DDR0_24_ODT_WR_MAP_CS1_DECODE(n) ((((u32)(n))>>16)&0x3)
#define DDR0_24_ODT_RD_MAP_CS1_MASK	0x00000300
#define DDR0_24_ODT_RD_MAP_CS1_ENCODE(n) ((((u32)(n))&0x3)<<8)
#define DDR0_24_ODT_RD_MAP_CS1_DECODE(n) ((((u32)(n))>>8)&0x3)
#define DDR0_24_ODT_WR_MAP_CS0_MASK	0x00000003
#define DDR0_24_ODT_WR_MAP_CS0_ENCODE(n) ((((u32)(n))&0x3)<<0)
#define DDR0_24_ODT_WR_MAP_CS0_DECODE(n) ((((u32)(n))>>0)&0x3)

#define DDR0_25				0x19
#define DDR0_25_VERSION_MASK		0xFFFF0000	/* Read only */
#define DDR0_25_VERSION_ENCODE(n)	((((u32)(n))&0xFFFF)<<16)
#define DDR0_25_VERSION_DECODE(n)	((((u32)(n))>>16)&0xFFFF)
#define DDR0_25_OUT_OF_RANGE_LENGTH_MASK  0x000003FF	/* Read only */
#define DDR0_25_OUT_OF_RANGE_LENGTH_ENCODE(n) ((((u32)(n))&0x3FF)<<0)
#define DDR0_25_OUT_OF_RANGE_LENGTH_DECODE(n) ((((u32)(n))>>0)&0x3FF)

#define DDR0_26				0x1A
#define DDR0_26_TRAS_MAX_MASK		0xFFFF0000
#define DDR0_26_TRAS_MAX_ENCODE(n)	((((u32)(n))&0xFFFF)<<16)
#define DDR0_26_TRAS_MAX_DECODE(n)	((((u32)(n))>>16)&0xFFFF)
#define DDR0_26_TREF_MASK		0x00003FFF
#define DDR0_26_TREF_ENCODE(n)		((((u32)(n))&0x3FFF)<<0)
#define DDR0_26_TREF_DECODE(n)		((((u32)(n))>>0)&0x3FFF)

#define DDR0_27				0x1B
#define DDR0_27_EMRS_DATA_MASK		0x3FFF0000
#define DDR0_27_EMRS_DATA_ENCODE(n)	((((u32)(n))&0x3FFF)<<16)
#define DDR0_27_EMRS_DATA_DECODE(n)	((((u32)(n))>>16)&0x3FFF)
#define DDR0_27_TINIT_MASK		0x0000FFFF
#define DDR0_27_TINIT_ENCODE(n)		((((u32)(n))&0xFFFF)<<0)
#define DDR0_27_TINIT_DECODE(n)		((((u32)(n))>>0)&0xFFFF)

#define DDR0_28				0x1C
#define DDR0_28_EMRS3_DATA_MASK		0x3FFF0000
#define DDR0_28_EMRS3_DATA_ENCODE(n)	((((u32)(n))&0x3FFF)<<16)
#define DDR0_28_EMRS3_DATA_DECODE(n)	((((u32)(n))>>16)&0x3FFF)
#define DDR0_28_EMRS2_DATA_MASK		0x00003FFF
#define DDR0_28_EMRS2_DATA_ENCODE(n)	((((u32)(n))&0x3FFF)<<0)
#define DDR0_28_EMRS2_DATA_DECODE(n)	((((u32)(n))>>0)&0x3FFF)

#define DDR0_31				0x1F
#define DDR0_31_XOR_CHECK_BITS_MASK	0x0000FFFF
#define DDR0_31_XOR_CHECK_BITS_ENCODE(n) ((((u32)(n))&0xFFFF)<<0)
#define DDR0_31_XOR_CHECK_BITS_DECODE(n) ((((u32)(n))>>0)&0xFFFF)

#define DDR0_32				0x20
#define DDR0_32_OUT_OF_RANGE_ADDR_MASK	0xFFFFFFFF	/* Read only */
#define DDR0_32_OUT_OF_RANGE_ADDR_ENCODE(n) ((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_32_OUT_OF_RANGE_ADDR_DECODE(n) ((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_33				0x21
#define DDR0_33_OUT_OF_RANGE_ADDR_MASK	0x00000001	/* Read only */
#define DDR0_33_OUT_OF_RANGE_ADDR_ENCODE(n) ((((u32)(n))&0x1)<<0)
#define DDR0_33_OUT_OF_RANGE_ADDR_DECODE(n) ((((u32)(n))>>0)&0x1)

#define DDR0_34				0x22
#define DDR0_34_ECC_U_ADDR_MASK		0xFFFFFFFF	/* Read only */
#define DDR0_34_ECC_U_ADDR_ENCODE(n)	((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_34_ECC_U_ADDR_DECODE(n)	((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_35				0x23
#define DDR0_35_ECC_U_ADDR_MASK		0x00000001	/* Read only */
#define DDR0_35_ECC_U_ADDR_ENCODE(n)	((((u32)(n))&0x1)<<0)
#define DDR0_35_ECC_U_ADDR_DECODE(n)	((((u32)(n))>>0)&0x1)

#define DDR0_36				0x24
#define DDR0_36_ECC_U_DATA_MASK		0xFFFFFFFF	/* Read only */
#define DDR0_36_ECC_U_DATA_ENCODE(n)	((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_36_ECC_U_DATA_DECODE(n)	((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_37				0x25
#define DDR0_37_ECC_U_DATA_MASK		0xFFFFFFFF	/* Read only */
#define DDR0_37_ECC_U_DATA_ENCODE(n)	((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_37_ECC_U_DATA_DECODE(n)	((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_38				0x26
#define DDR0_38_ECC_C_ADDR_MASK		0xFFFFFFFF	/* Read only */
#define DDR0_38_ECC_C_ADDR_ENCODE(n)	((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_38_ECC_C_ADDR_DECODE(n)	((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_39				0x27
#define DDR0_39_ECC_C_ADDR_MASK		0x00000001	/* Read only */
#define DDR0_39_ECC_C_ADDR_ENCODE(n)	((((u32)(n))&0x1)<<0)
#define DDR0_39_ECC_C_ADDR_DECODE(n)	((((u32)(n))>>0)&0x1)

#define DDR0_40				0x28
#define DDR0_40_ECC_C_DATA_MASK		0xFFFFFFFF	/* Read only */
#define DDR0_40_ECC_C_DATA_ENCODE(n)	((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_40_ECC_C_DATA_DECODE(n)	((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_41				0x29
#define DDR0_41_ECC_C_DATA_MASK		0xFFFFFFFF	/* Read only */
#define DDR0_41_ECC_C_DATA_ENCODE(n)	((((u32)(n))&0xFFFFFFFF)<<0)
#define DDR0_41_ECC_C_DATA_DECODE(n)	((((u32)(n))>>0)&0xFFFFFFFF)

#define DDR0_42				0x2A
#define DDR0_42_ADDR_PINS_MASK		0x07000000
#define DDR0_42_ADDR_PINS_ENCODE(n)	((((u32)(n))&0x7)<<24)
#define DDR0_42_ADDR_PINS_DECODE(n)	((((u32)(n))>>24)&0x7)
#define DDR0_42_CASLAT_LIN_GATE_MASK	0x0000000F
#define DDR0_42_CASLAT_LIN_GATE_ENCODE(n) ((((u32)(n))&0xF)<<0)
#define DDR0_42_CASLAT_LIN_GATE_DECODE(n) ((((u32)(n))>>0)&0xF)

#define DDR0_43				0x2B
#define DDR0_43_TWR_MASK		0x07000000
#define DDR0_43_TWR_ENCODE(n)		((((u32)(n))&0x7)<<24)
#define DDR0_43_TWR_DECODE(n)		((((u32)(n))>>24)&0x7)
#define DDR0_43_APREBIT_MASK		0x000F0000
#define DDR0_43_APREBIT_ENCODE(n)	((((u32)(n))&0xF)<<16)
#define DDR0_43_APREBIT_DECODE(n)	((((u32)(n))>>16)&0xF)
#define DDR0_43_COLUMN_SIZE_MASK	0x00000700
#define DDR0_43_COLUMN_SIZE_ENCODE(n)	((((u32)(n))&0x7)<<8)
#define DDR0_43_COLUMN_SIZE_DECODE(n)	((((u32)(n))>>8)&0x7)
#define DDR0_43_EIGHT_BANK_MODE_MASK	0x00000001
#define DDR0_43_EIGHT_BANK_MODE_8_BANKS	0x00000001
#define DDR0_43_EIGHT_BANK_MODE_4_BANKS	0x00000000
#define DDR0_43_EIGHT_BANK_MODE_ENCODE(n) ((((u32)(n))&0x1)<<0)
#define DDR0_43_EIGHT_BANK_MODE_DECODE(n) ((((u32)(n))>>0)&0x1)

#define DDR0_44				0x2C
#define DDR0_44_TRCD_MASK		0x000000FF
#define DDR0_44_TRCD_ENCODE(n)		((((u32)(n))&0xFF)<<0)
#define DDR0_44_TRCD_DECODE(n)		((((u32)(n))>>0)&0xFF)

#endif /* CONFIG_SDRAM_PPC4xx_DENALI_DDR2 */

#endif /* _PPC4xx_SDRAM_H_ */
