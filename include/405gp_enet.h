/*----------------------------------------------------------------------------+
|
|       This source code has been made available to you by IBM on an AS-IS
|       basis.  Anyone receiving this source is licensed under IBM
|       copyrights to use it in any way he or she deems fit, including
|       copying it, modifying it, compiling it, and redistributing it either
|       with or without modifications.  No license under IBM patents or
|       patent applications is to be implied by the copyright license.
|
|       Any user of this software should understand that IBM cannot provide
|       technical support for this software and will not be responsible for
|       any consequences resulting from the use of this software.
|
|       Any person who transfers this source code or any derivative work
|       must include the IBM copyright notice, this paragraph, and the
|       preceding two paragraphs in the transferred software.
|
|       COPYRIGHT   I B M   CORPORATION 1999
|       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|
|  File Name:   enetemac.h
|
|  Function:    Header file for the EMAC3 macro on the 405GP.
|
|  Author:      Mark Wisner
|
|  Change Activity-
|
|  Date        Description of Change                                       BY
|  ---------   ---------------------                                       ---
|  29-Apr-99   Created                                                     MKW
|
+----------------------------------------------------------------------------*/
#ifndef _enetemac_h_
#define _enetemac_h_
#include <net.h>
#include <405_mal.h>

/*-----------------------------------------------------------------------------+
| General enternet defines.  802 frames are not supported.
+-----------------------------------------------------------------------------*/
#define ENET_ADDR_LENGTH                6
#define ENET_ARPTYPE                    0x806
#define ARP_REQUEST                     1
#define ARP_REPLY                       2
#define ENET_IPTYPE                     0x800
#define ARP_CACHE_SIZE                  5


struct enet_frame {
   unsigned char        dest_addr[ENET_ADDR_LENGTH];
   unsigned char        source_addr[ENET_ADDR_LENGTH];
   unsigned short       type;
   unsigned char        enet_data[1];
};

struct arp_entry {
   unsigned long        inet_address;
   unsigned char        mac_address[ENET_ADDR_LENGTH];
   unsigned long        valid;
   unsigned long        sec;
   unsigned long        nsec;
};


			/*Register addresses */
#if defined(CONFIG_440)
#define ZMII_BASE			(CFG_PERIPHERAL_BASE + 0x0780)
#define ZMII_FER			(ZMII_BASE)
#define ZMII_SSR			(ZMII_BASE + 4)
#define ZMII_SMIISR			(ZMII_BASE + 8)

#define ZMII_RMII			0x22000000
#define ZMII_MDI0			0x80000000
#endif /* CONFIG_440 */

#if defined(CONFIG_440)
#define EMAC_BASE 			(CFG_PERIPHERAL_BASE + 0x0800)
#else
#define EMAC_BASE 			0xEF600800
#endif

#define EMAC_M0 			(EMAC_BASE)
#define EMAC_M1 			(EMAC_BASE + 4)
#define EMAC_TXM0				(EMAC_BASE + 8)
#define EMAC_TXM1		 		(EMAC_BASE + 12)
#define EMAC_RXM		 		(EMAC_BASE + 16)
#define EMAC_ISR		 		(EMAC_BASE + 20)
#define EMAC_IER		 		(EMAC_BASE + 24)
#define EMAC_IAH		 		(EMAC_BASE + 28)
#define EMAC_IAL		 		(EMAC_BASE + 32)
#define EMAC_VLAN_TPID_REG 		(EMAC_BASE + 36)
#define EMAC_VLAN_TCI_REG 		(EMAC_BASE + 40)
#define EMAC_PAUSE_TIME_REG 		(EMAC_BASE + 44)
#define EMAC_IND_HASH_1			(EMAC_BASE + 48)
#define EMAC_IND_HASH_2			(EMAC_BASE + 52)
#define EMAC_IND_HASH_3			(EMAC_BASE + 56)
#define EMAC_IND_HASH_4			(EMAC_BASE + 60)
#define EMAC_GRP_HASH_1			(EMAC_BASE + 64)
#define EMAC_GRP_HASH_2			(EMAC_BASE + 68)
#define EMAC_GRP_HASH_3			(EMAC_BASE + 72)
#define EMAC_GRP_HASH_4			(EMAC_BASE + 76)
#define EMAC_LST_SRC_LOW		(EMAC_BASE + 80)
#define EMAC_LST_SRC_HI			(EMAC_BASE + 84)
#define EMAC_I_FRAME_GAP_REG		(EMAC_BASE + 88)
#define EMAC_STACR			(EMAC_BASE + 92)
#define EMAC_TRTR				(EMAC_BASE + 96)
#define EMAC_RX_HI_LO_WMARK		(EMAC_BASE + 100)

/* bit definitions */
/* MODE REG 0 */
#define EMAC_M0_RXI			0x80000000
#define EMAC_M0_TXI			0x40000000
#define EMAC_M0_SRST			0x20000000
#define EMAC_M0_TXE			0x10000000
#define EMAC_M0_RXE			0x08000000
#define EMAC_M0_WKE			0x04000000

/* MODE Reg 1 */
#define EMAC_M1_FDE			0x80000000
#define EMAC_M1_ILE			0x40000000
#define EMAC_M1_VLE			0x20000000
#define EMAC_M1_EIFC			0x10000000
#define EMAC_M1_APP			0x08000000
#define EMAC_M1_AEMI			0x02000000
#define EMAC_M1_IST			0x01000000
#define EMAC_M1_MF_1000MBPS		0x00800000	/* 0's for 10MBPS */
#define EMAC_M1_MF_100MBPS		0x00400000
#define EMAC_M1_RFS_4K			0x00300000	/* ~4k for 512 byte */
#define EMAC_M1_RFS_2K			0x00200000
#define EMAC_M1_RFS_1K			0x00100000
#define EMAC_M1_TX_FIFO_2K		0x00080000	/* 0's for 512 byte */
#define EMAC_M1_TX_FIFO_1K		0x00040000
#define EMAC_M1_TR0_DEPEND		0x00010000	/* 0'x for single packet */
#define EMAC_M1_TR0_MULTI		0x00008000
#define EMAC_M1_TR1_DEPEND		0x00004000
#define EMAC_M1_TR1_MULTI		0x00002000
#define EMAC_M1_JUMBO_ENABLE		0x00001000

/* Transmit Mode Register 0 */
#define EMAC_TXM0_GNP0			0x80000000
#define EMAC_TXM0_GNP1			0x40000000
#define EMAC_TXM0_GNPD			0x20000000
#define EMAC_TXM0_FC			0x10000000

/* Receive Mode Register */
#define EMAC_RMR_SP			0x80000000
#define EMAC_RMR_SFCS			0x40000000
#define EMAC_RMR_ARRP			0x20000000
#define EMAC_RMR_ARP			0x10000000
#define EMAC_RMR_AROP			0x08000000
#define EMAC_RMR_ARPI			0x04000000
#define EMAC_RMR_PPP			0x02000000
#define EMAC_RMR_PME			0x01000000
#define EMAC_RMR_PMME			0x00800000
#define EMAC_RMR_IAE			0x00400000
#define EMAC_RMR_MIAE			0x00200000
#define EMAC_RMR_BAE			0x00100000
#define EMAC_RMR_MAE			0x00080000

/* Interrupt Status & enable Regs */
#define EMAC_ISR_OVR			0x02000000
#define EMAC_ISR_PP			0x01000000
#define EMAC_ISR_BP			0x00800000
#define EMAC_ISR_RP			0x00400000
#define EMAC_ISR_SE			0x00200000
#define EMAC_ISR_SYE			0x00100000
#define EMAC_ISR_BFCS			0x00080000
#define EMAC_ISR_PTLE			0x00040000
#define EMAC_ISR_ORE			0x00020000
#define EMAC_ISR_IRE			0x00010000
#define EMAC_ISR_DBDM			0x00000200
#define EMAC_ISR_DB0			0x00000100
#define EMAC_ISR_SE0			0x00000080
#define EMAC_ISR_TE0			0x00000040
#define EMAC_ISR_DB1			0x00000020
#define EMAC_ISR_SE1			0x00000010
#define EMAC_ISR_TE1			0x00000008
#define EMAC_ISR_MOS			0x00000002
#define EMAC_ISR_MOF			0x00000001


/* STA CONTROL REG */
#define EMAC_STACR_OC			0x00008000
#define EMAC_STACR_PHYE			0x00004000
#define EMAC_STACR_WRITE		0x00002000
#define EMAC_STACR_READ			0x00001000
#define EMAC_STACR_CLK_83MHZ		0x00000800  /* 0's for 50Mhz */
#define EMAC_STACR_CLK_66MHZ		0x00000400
#define EMAC_STACR_CLK_100MHZ		0x00000C00

/* Transmit Request Threshold Register */
#define EMAC_TRTR_256			0x18000000   /* 0's for 64 Bytes */
#define EMAC_TRTR_192			0x10000000
#define EMAC_TRTR_128			0x01000000

/* the follwing defines are for the MadMAL status and control registers. */
/* For bits 0..5 look at the mal.h file                                  */
#define EMAC_TX_CTRL_GFCS 	0x0200
#define EMAC_TX_CTRL_GP		0x0100
#define EMAC_TX_CTRL_ISA	0x0080
#define EMAC_TX_CTRL_RSA	0x0040
#define EMAC_TX_CTRL_IVT	0x0020
#define EMAC_TX_CTRL_RVT	0x0010

#define EMAC_TX_CTRL_DEFAULT (EMAC_TX_CTRL_GFCS |EMAC_TX_CTRL_GP)

#define EMAC_TX_ST_BFCS		0x0200
#define EMAC_TX_ST_BPP		0x0100
#define EMAC_TX_ST_LCS		0x0080
#define EMAC_TX_ST_ED		0x0040
#define EMAC_TX_ST_EC		0x0020
#define EMAC_TX_ST_LC		0x0010
#define EMAC_TX_ST_MC		0x0008
#define EMAC_TX_ST_SC		0x0004
#define EMAC_TX_ST_UR		0x0002
#define EMAC_TX_ST_SQE		0x0001

#define EMAC_TX_ST_DEFAULT    0x03F3


/* madmal receive status / Control bits */

#define EMAC_RX_ST_OE		0x0200
#define EMAC_RX_ST_PP		0x0100
#define EMAC_RX_ST_BP		0x0080
#define EMAC_RX_ST_RP		0x0040
#define EMAC_RX_ST_SE		0x0020
#define EMAC_RX_ST_AE		0x0010
#define EMAC_RX_ST_BFCS		0x0008
#define EMAC_RX_ST_PTL		0x0004
#define EMAC_RX_ST_ORE		0x0002
#define EMAC_RX_ST_IRE		0x0001
/* all the errors we care about */
#define EMAC_RX_ERRORS		0x03FF

#define NUM_RX_BUFF PKTBUFSRX
#define NUM_TX_BUFF 1

#define MAX_ERR_LOG 10
typedef struct emac_stats_st{	/* Statistic Block */
	int data_len_err;
	int rx_frames;
	int rx;
	int rx_prot_err;
	int int_err;
	int pkts_tx;
	int pkts_rx;
	int pkts_handled;
	short tx_err_log[MAX_ERR_LOG];
	short rx_err_log[MAX_ERR_LOG];
} EMAC_STATS_ST, *EMAC_STATS_PST;

/* Structure containing variables used by the shared code (440gx_enet.c) */
typedef struct emac_440gx_hw_st {
	uint32_t		hw_addr;		/* EMAC offset */
	uint32_t		tah_addr;		/* TAH offset */
	uint32_t		phy_id;
	uint32_t		phy_addr;
	uint32_t		original_fc;
	uint32_t		txcw;
	uint32_t		autoneg_failed;
	uint32_t		emac_ier;
	volatile mal_desc_t *tx;
	volatile mal_desc_t *rx;
	bd_t		*bis;	/* for eth_init upon mal error */
	mal_desc_t		*alloc_tx_buf;
	mal_desc_t		*alloc_rx_buf;
	char		*txbuf_ptr;
	uint16_t		devnum;
	int			get_link_status;
	int			tbi_compatibility_en;
	int			tbi_compatibility_on;
	int			fc_send_xon;
	int			report_tx_early;
	int			first_init;
	int			tx_err_index;
	int			rx_err_index;
	int			rx_slot;			/* MAL Receive Slot */
	int			rx_i_index;		/* Receive Interrupt Queue Index */
	int			rx_u_index;		/* Receive User Queue Index */
	int			tx_slot;			/* MAL Transmit Slot */
	int			tx_i_index;		/* Transmit Interrupt Queue Index */
	int			tx_u_index;		/* Transmit User Queue Index */
	int			rx_ready[NUM_RX_BUFF];	/* Receive Ready Queue */
	int			tx_run[NUM_TX_BUFF];	/* Transmit Running Queue */
	int			is_receiving;	/* sync with eth interrupt */
	int			print_speed;	/* print speed message upon start */
	EMAC_STATS_ST	stats;
} EMAC_405_HW_ST, *EMAC_405_HW_PST;

/*-----------------------------------------------------------------------------+
| Function prototypes for device table.
+-----------------------------------------------------------------------------*/
#endif /* _enetLib_h_ */
