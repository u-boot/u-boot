typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef volatile unsigned char vuint8;
typedef volatile unsigned short vuint16;
typedef volatile unsigned int vuint32;


#define DPRAM_ATM CFG_IMMR + 0x3000

#define ATM_DPRAM_BEGIN  (DPRAM_ATM - CFG_IMMR - 0x2000)
#define NUM_CONNECTIONS  1
#define SAR_RXB_SIZE     1584
#define AM_HMASK         0x0FFFFFF0

#define NUM_CT_ENTRIES           (NUM_CONNECTIONS)
#define NUM_TCTE_ENTRIES         (NUM_CONNECTIONS)
#define NUM_AM_ENTRIES           (NUM_CONNECTIONS+1)
#define NUM_AP_ENTRIES           (NUM_CONNECTIONS+1)
#define NUM_MPHYPT_ENTRIES       1
#define NUM_APCP_ENTRIES         1
#define NUM_APCT_PRIO_1_ENTRIES  146	/* Determines minimum rate */
#define NUM_TQ_ENTRIES           12

#define SIZE_OF_CT_ENTRY         64
#define SIZE_OF_TCTE_ENTRY       32
#define SIZE_OF_AM_ENTRY         4
#define SIZE_OF_AP_ENTRY         2
#define SIZE_OF_MPHYPT_ENTRY     2
#define SIZE_OF_APCP_ENTRY       32
#define SIZE_OF_APCT_ENTRY       2
#define SIZE_OF_TQ_ENTRY         2

#define CT_BASE           ((ATM_DPRAM_BEGIN + 63) & 0xFFC0)	/*64 */
#define TCTE_BASE         (CT_BASE + NUM_CT_ENTRIES * SIZE_OF_CT_ENTRY)	/*32 */
#define APCP_BASE         (TCTE_BASE + NUM_TCTE_ENTRIES * SIZE_OF_TCTE_ENTRY)	/*32 */
#define AM_BEGIN          (APCP_BASE + NUM_APCP_ENTRIES * SIZE_OF_APCP_ENTRY)	/*4 */
#define AM_BASE           (AM_BEGIN + (NUM_AM_ENTRIES - 1) * SIZE_OF_AM_ENTRY)
#define AP_BEGIN          (AM_BEGIN + NUM_AM_ENTRIES * SIZE_OF_AM_ENTRY)	/*2 */
#define AP_BASE           (AP_BEGIN + (NUM_AP_ENTRIES - 1) * SIZE_OF_AP_ENTRY)
#define MPHYPT_BASE       (AP_BEGIN + NUM_AP_ENTRIES * SIZE_OF_AP_ENTRY)	/*2 */
#define APCT_PRIO_1_BASE  (MPHYPT_BASE + NUM_MPHYPT_ENTRIES * SIZE_OF_MPHYPT_ENTRY)	/*2 */
#define TQ_BASE           (APCT_PRIO_1_BASE + NUM_APCT_PRIO_1_ENTRIES * SIZE_OF_APCT_ENTRY)	/*2 */
#define ATM_DPRAM_SIZE    ((TQ_BASE + NUM_TQ_ENTRIES * SIZE_OF_TQ_ENTRY) - ATM_DPRAM_BEGIN)

#define CT_PTR(base)      ((struct ct_entry_t *)((char *)(base) + 0x2000 + CT_BASE))
#define TCTE_PTR(base)    ((struct tcte_entry_t *)((char *)(base) + 0x2000 + TCTE_BASE))
#define AM_PTR(base)      ((uint32 *)((char *)(base) + 0x2000 + AM_BASE))
#define AP_PTR(base)      ((uint16 *)((char *)(base) + 0x2000 + AP_BASE))
#define MPHYPT_PTR(base)  ((uint16 *)((char *)(base) + 0x2000 + MPHYPT_BASE))
#define APCP_PTR(base)    ((struct apc_params_t *)((char*)(base) + 0x2000 + APCP_BASE))
#define APCT1_PTR(base)   ((uint16 *)((char *)(base) + 0x2000 + APCT_PRIO_1_BASE))
#define APCT2_PTR(base)   ((uint16 *)((char *)(base) + 0x2000 + APCT_PRIO_2_BASE))
#define APCT3_PTR(base)   ((uint16 *)((char *)(base) + 0x2000 + APCT_PRIO_3_BASE))
#define TQ_PTR(base)      ((uint16 *)((char *)(base) + 0x2000 + TQ_BASE))

/* SAR registers */
#define RBDBASE(base)	  ((vuint32 *)(base + 0x3F00))	/* Base address of RxBD-List */
#define SRFCR(base)	  ((vuint8 *)(base + 0x3F04))	/* DMA Receive function code */
#define SRSTATE(base)	  ((vuint8 *)(base + 0x3F05))	/* DMA Receive status */
#define MRBLR(base)	  ((vuint16 *)(base + 0x3F06))	/* Init to 0 for ATM */
#define RSTATE(base)	  ((vuint32 *)(base + 0x3F08))	/* Do not write to */
#define R_CNT(base)	  ((vuint16 *)(base + 0x3F10))	/* Do not write to */
#define STFCR(base)	  ((vuint8 *)(base + 0x3F12))	/* DMA Transmit function code */
#define STSTATE(base)	  ((vuint8 *)(base + 0x3F13))	/* DMA Transmit status */
#define TBDBASE(base)	  ((vuint32 *)(base + 0x3F14))	/* Base address of TxBD-List */
#define TSTATE(base)	  ((vuint32 *)(base + 0x3F18))	/* Do not write to */
#define COMM_CH(base)	  ((vuint16 *)(base + 0x3F1C))	/* Command channel */
#define STCHNUM(base)	  ((vuint16 *)(base + 0x3F1E))	/* Do not write to */
#define T_CNT(base)	  ((vuint16 *)(base + 0x3F20))	/* Do not write to */
#define CTBASE(base)	  ((vuint16 *)(base + 0x3F22))	/* Base address of Connection-table */
#define ECTBASE(base)	  ((vuint32 *)(base + 0x3F24))	/* Valid only for external Conn.-table */
#define INTBASE(base)	  ((vuint32 *)(base + 0x3F28))	/* Base address of Interrupt-table */
#define INTPTR(base)	  ((vuint32 *)(base + 0x3F2C))	/* Pointer to Interrupt-queue */
#define C_MASK(base)	  ((vuint32 *)(base + 0x3F30))	/* CRC-mask */
#define SRCHNUM(base)	  ((vuint16 *)(base + 0x3F34))	/* Do not write to */
#define INT_CNT(base)	  ((vuint16 *)(base + 0x3F36))	/* Interrupt-Counter */
#define INT_ICNT(base)	  ((vuint16 *)(base + 0x3F38))	/* Interrupt threshold */
#define TSTA(base)	  ((vuint16 *)(base + 0x3F3A))	/* Time-stamp-address */
#define OLDLEN(base)	  ((vuint16 *)(base + 0x3F3C))	/* Do not write to */
#define SMRBLR(base)	  ((vuint16 *)(base + 0x3F3E))	/* SAR max RXBuffer length */
#define EHEAD(base)	  ((vuint32 *)(base + 0x3F40))	/* Valid for serial mode */
#define EPAYLOAD(base)	  ((vuint32 *)(base + 0x3F44))	/* Valid for serial mode */
#define TQBASE(base)	  ((vuint16 *)(base + 0x3F48))	/* Base address of Tx queue */
#define TQEND(base)	  ((vuint16 *)(base + 0x3F4A))	/* End address of Tx queue */
#define TQAPTR(base)	  ((vuint16 *)(base + 0x3F4C))	/* TQ APC pointer */
#define TQTPTR(base)	  ((vuint16 *)(base + 0x3F4E))	/* TQ Tx pointer */
#define APCST(base)	  ((vuint16 *)(base + 0x3F50))	/* APC status */
#define APCPTR(base)	  ((vuint16 *)(base + 0x3F52))	/* APC parameter pointer */
#define HMASK(base)	  ((vuint32 *)(base + 0x3F54))	/* Header mask */
#define AMBASE(base)	  ((vuint16 *)(base + 0x3F58))	/* Address match table base */
#define AMEND(base)	  ((vuint16 *)(base + 0x3F5A))	/* Address match table end */
#define APBASE(base)	  ((vuint16 *)(base + 0x3F5C))	/* Address match parameter */
#define FLBASE(base)	  ((vuint32 *)(base + 0x3F54))	/* First-level table base */
#define SLBASE(base)	  ((vuint32 *)(base + 0x3F58))	/* Second-level table base */
#define FLMASK(base)	  ((vuint16 *)(base + 0x3F5C))	/* First-level mask */
#define ECSIZE(base)	  ((vuint16 *)(base + 0x3F5E))	/* Valid for extended mode */
#define APCT_REAL(base)	  ((vuint32 *)(base + 0x3F60))	/* APC 32 bit counter */
#define R_PTR(base)	  ((vuint32 *)(base + 0x3F64))	/* Do not write to */
#define RTEMP(base)	  ((vuint32 *)(base + 0x3F68))	/* Do not write to */
#define T_PTR(base)	  ((vuint32 *)(base + 0x3F6C))	/* Do not write to */
#define TTEMP(base)	  ((vuint32 *)(base + 0x3F70))	/* Do not write to */

/* ESAR registers */
#define FMCTIMESTMP(base) ((vuint32 *)(base + 0x3F80))	/* Perf.Mon.Timestamp */
#define FMCTEMPLATE(base) ((vuint32 *)(base + 0x3F84))	/* Perf.Mon.Template */
#define PMPTR(base)       ((vuint16 *)(base + 0x3F88))	/* Perf.Mon.Table */
#define PMCHANNEL(base)	  ((vuint16 *)(base + 0x3F8A))	/* Perf.Mon.Channel */
#define MPHYST(base)	  ((vuint16 *)(base + 0x3F90))	/* Multi-PHY Status */
#define TCTEBASE(base)	  ((vuint16 *)(base + 0x3F92))	/* Internal TCT Extension Base */
#define ETCTEBASE(base)	  ((vuint32 *)(base + 0x3F94))	/* External TCT Extension Base */
#define COMM_CH2(base)	  ((vuint32 *)(base + 0x3F98))	/* 2nd command channel word */
#define STATBASE(base)	  ((vuint16 *)(base + 0x3F9C))	/* Statistics table pointer */

/* UTOPIA Mode Register */
#define UTMODE(base)      (CAST(vuint32 *)(base + 0x0978))

/* SAR commands */
#define TRANSMIT_CHANNEL_ACTIVATE_CMD   0x0FC1
#define TRANSMIT_CHANNEL_DEACTIVATE_CMD 0x1FC1
#define STOP_TRANSMIT_CMD               0x2FC1
#define RESTART_TRANSMIT_CMD            0x3FC1
#define STOP_RECEIVE_CMD                0x4FC1
#define RESTART_RECEIVE_CMD             0x5FC1
#define APC_BYPASS_CMD                  0x6FC1
#define MEM_WRITE_CMD                   0x7FC1
#define CPCR_FLG                        0x0001

/* INT flags */
#define INT_VALID	0x80000000
#define INT_WRAP	0x40000000
#define INT_APCO	0x00800000
#define INT_TQF		0x00200000
#define INT_RXF		0x00080000
#define INT_BSY		0x00040000
#define INT_TXB		0x00020000
#define INT_RXB		0x00010000

#define NUM_INT_ENTRIES   80
#define SIZE_OF_INT_ENTRY 4

struct apc_params_t {
	vuint16 apct_base1;	/* APC Table - First Priority Base pointer */
	vuint16 apct_end1;	/* First APC Table - Length */
	vuint16 apct_ptr1;	/* First APC Table Pointer */
	vuint16 apct_sptr1;	/* APC Table First Priority Service pointer */
	vuint16 etqbase;	/* Enhanced Transmit Queue Base pointer */
	vuint16 etqend;		/* Enhanced Transmit Queue End pointer */
	vuint16 etqaptr;	/* Enhanced Transmit Queue APC pointer */
	vuint16 etqtptr;	/* Enhanced Transmit Queue Transmitter pointer */
	vuint16 apc_mi;		/* APC - Max Iteration */
	vuint16 ncits;		/* Number of Cells In TimeSlot  */
	vuint16 apcnt;		/* APC - N Timer */
	vuint16 reserved1;	/* reserved */
	vuint16 eapcst;		/* APC status */
	vuint16 ptp_counter;	/* PTP queue length */
	vuint16 ptp_txch;	/* PTP channel */
	vuint16 reserved2;	/* reserved */
};

struct ct_entry_t {
	/* RCT */
	unsigned fhnt:1;
	unsigned pm_rct:1;
	unsigned reserved0:6;
	unsigned hec:1;
	unsigned clp:1;
	unsigned cng_ncrc:1;
	unsigned inf_rct:1;
	unsigned cngi_ptp:1;
	unsigned cdis_rct:1;
	unsigned aal_rct:2;
	uint16 rbalen;
	uint32 rcrc;
	uint32 rb_ptr;
	uint16 rtmlen;
	uint16 rbd_ptr;
	uint16 rbase;
	uint16 tstamp;
	uint16 imask;
	unsigned ft:2;
	unsigned nim:1;
	unsigned reserved1:2;
	unsigned rpmt:6;
	unsigned reserved2:5;
	uint8 reserved3[8];
	/* TCT */
	unsigned reserved4:1;
	unsigned pm_tct:1;
	unsigned reserved5:6;
	unsigned pc:1;
	unsigned reserved6:2;
	unsigned inf_tct:1;
	unsigned cr10:1;
	unsigned cdis_tct:1;
	unsigned aal_tct:2;
	uint16 tbalen;
	uint32 tcrc;
	uint32 tb_ptr;
	uint16 ttmlen;
	uint16 tbd_ptr;
	uint16 tbase;
	unsigned reserved7:5;
	unsigned tpmt:6;
	unsigned reserved8:3;
	unsigned avcf:1;
	unsigned act:1;
	uint32 chead;
	uint16 apcl;
	uint16 apcpr;
	unsigned out:1;
	unsigned bnr:1;
	unsigned tservice:2;
	unsigned apcp:12;
	uint16 apcpf;
};

struct tcte_entry_t {
	unsigned res1:4;
	unsigned scr:12;
	uint16 scrf;
	uint16 bt;
	uint16 buptrh;
	uint32 buptrl;
	unsigned vbr2:1;
	unsigned res2:15;
	uint16 oobr;
	uint16 res3[8];
};

#define SIZE_OF_RBD  12
#define SIZE_OF_TBD  12

struct atm_bd_t {
	vuint16 flags;
	vuint16 length;
	unsigned char *buffer_ptr;
	vuint16 cpcs_uu_cpi;
	vuint16 reserved;
};

/* BD flags */
#define EMPTY		0x8000
#define READY		0x8000
#define WRAP		0x2000
#define INTERRUPT	0x1000
#define LAST		0x0800
#define FIRST		0x0400
#define OAM             0x0400
#define CONTINUOUS	0x0200
#define HEC_ERROR	0x0080
#define CELL_LOSS	0x0040
#define CONGESTION	0x0020
#define ABORT		0x0010
#define LEN_ERROR	0x0002
#define CRC_ERROR	0x0001

struct atm_connection_t {
	struct atm_bd_t *rbd_ptr;
	int num_rbd;
	struct atm_bd_t *tbd_ptr;
	int num_tbd;
	struct ct_entry_t *ct_ptr;
	struct tcte_entry_t *tcte_ptr;
	void *drv;
	void (*notify) (void *drv, int event);
};

struct atm_driver_t {
	int loaded;
	int started;
	char *csram;
	int csram_size;
	uint32 *am_top;
	uint16 *ap_top;
	uint32 *int_reload_ptr;
	uint32 *int_serv_ptr;
	struct atm_bd_t *rbd_base_ptr;
	struct atm_bd_t *tbd_base_ptr;
	unsigned linerate_in_bps;
};

extern struct atm_connection_t g_conn[NUM_CONNECTIONS];
extern struct atm_driver_t g_atm;

extern int atmLoad (void);
extern void atmUnload (void);
