#include <common.h>
#include <mpc8xx.h>
#include <commproc.h>

#include "atm.h"
#include <linux/stddef.h>

#define SYNC __asm__("sync")
#define MY_ALIGN(p, a) ((char *)(((uint32)(p)+(a)-1) & ~((uint32)(a)-1)))

#define FALSE  1
#define TRUE   0
#define OK     0
#define ERROR -1

struct atm_connection_t g_conn[NUM_CONNECTIONS] =
{
  { NULL, 10, NULL, 10,  NULL, NULL, NULL, NULL }, /* OAM  */
};

struct atm_driver_t g_atm =
{
  FALSE,   /* loaded */
  FALSE,   /* started */
  NULL,    /* csram */
  0,       /* csram_size */
  NULL,    /* am_top */
  NULL,    /* ap_top */
  NULL,    /* int_reload_ptr */
  NULL,    /* int_serv_ptr */
  NULL,    /* rbd_base_ptr */
  NULL,    /* tbd_base_ptr */
  0        /* linerate */
};

char csram[1024]; /* more than enough for doing nothing*/

int    atmLoad(void);
void   atmUnload(void);
int    atmMemInit(void);
void   atmIntInit(void);
void   atmApcInit(void);
void   atmAmtInit(void);
void   atmCpmInit(void);
void   atmUtpInit(void);

/*****************************************************************************
 *
 * FUNCTION NAME: atmLoad
 *
 * DESCRIPTION: Basic ATM initialization.
 *
 * PARAMETERS: none
 *
 * RETURNS: OK or ERROR
 *
 ****************************************************************************/
int atmLoad()
{
  volatile immap_t       *immap  = (immap_t *)CONFIG_SYS_IMMR;
  volatile cpmtimer8xx_t *timers = &immap->im_cpmtimer;
  volatile iop8xx_t      *iop    = &immap->im_ioport;

  timers->cpmt_tgcr &=  0x0FFF; SYNC;             /* Disable Timer 4 */
  immap->im_cpm.cp_scc[3].scc_gsmrl = 0x0; SYNC; /* Disable SCC4 */
  iop->iop_pdpar &= 0x3FFF; SYNC;                 /* Disable SAR and UTOPIA */

  if ( atmMemInit() != OK ) return ERROR;

  atmIntInit();
  atmApcInit();
  atmAmtInit();
  atmCpmInit();
  atmUtpInit();

  g_atm.loaded = TRUE;

  return OK;
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmUnload
 *
 * DESCRIPTION: Disables ATM and UTOPIA.
 *
 * PARAMETERS: none
 *
 * RETURNS: void
 *
 ****************************************************************************/
void atmUnload()
{
  volatile immap_t       *immap  = (immap_t *)CONFIG_SYS_IMMR;
  volatile cpmtimer8xx_t *timers = &immap->im_cpmtimer;
  volatile iop8xx_t      *iop    = &immap->im_ioport;

  timers->cpmt_tgcr &=  0x0FFF; SYNC;             /* Disable Timer 4 */
  immap->im_cpm.cp_scc[3].scc_gsmrl = 0x0; SYNC;  /* Disable SCC4 */
  iop->iop_pdpar &= 0x3FFF; SYNC;                 /* Disable SAR and UTOPIA */
  g_atm.loaded = FALSE;
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmMemInit
 *
 * DESCRIPTION:
 *
 * The ATM driver uses the following resources:
 *
 * A. Memory in DPRAM to hold
 *
 *     1/ CT          = Connection Table ( RCT & TCT )
 *     2/ TCTE        = Transmit Connection Table Extension
 *     3/ MPHYPT      = Multi-PHY Pointing Table
 *     4/ APCP        = APC Parameter Table
 *     5/ APCT_PRIO_1 = APC Table ( priority 1 for AAL1/2 )
 *     6/ APCT_PRIO_2 = APC Table ( priority 2 for VBR )
 *     7/ APCT_PRIO_3 = APC Table ( priority 3 for UBR )
 *     8/ TQ          = Transmit Queue
 *     9/ AM          = Address Matching Table
 *    10/ AP          = Address Pointing Table
 *
 * B. Memory in cache safe RAM to hold
 *
 *     1/ INT         = Interrupt Queue
 *     2/ RBD         = Receive Buffer Descriptors
 *     3/ TBD         = Transmit Buffer Descriptors
 *
 * This function
 * 1. clears the ATM DPRAM area,
 * 2. Allocates and clears cache safe memory,
 * 3. Initializes 'g_conn'.
 *
 * PARAMETERS: none
 *
 * RETURNS: OK or ERROR
 *
 ****************************************************************************/
int atmMemInit()
{
  int i;
  unsigned immr = CONFIG_SYS_IMMR;
  int total_num_rbd = 0;
  int total_num_tbd = 0;

  memset((char *)CONFIG_SYS_IMMR + 0x2000 + ATM_DPRAM_BEGIN, 0x00, ATM_DPRAM_SIZE);

  g_atm.csram_size = NUM_INT_ENTRIES * SIZE_OF_INT_ENTRY;

  for ( i = 0; i < NUM_CONNECTIONS; ++i ) {
    total_num_rbd += g_conn[i].num_rbd;
    total_num_tbd += g_conn[i].num_tbd;
  }

  g_atm.csram_size += total_num_rbd * SIZE_OF_RBD + total_num_tbd * SIZE_OF_TBD + 4;

  g_atm.csram = &csram[0];
  memset(&(g_atm.csram), 0x00, g_atm.csram_size);

  g_atm.int_reload_ptr = (uint32 *)MY_ALIGN(g_atm.csram, 4);
  g_atm.rbd_base_ptr = (struct atm_bd_t *)(g_atm.int_reload_ptr + NUM_INT_ENTRIES);
  g_atm.tbd_base_ptr = (struct atm_bd_t *)(g_atm.rbd_base_ptr + total_num_rbd);

  g_conn[0].rbd_ptr = g_atm.rbd_base_ptr;
  g_conn[0].tbd_ptr = g_atm.tbd_base_ptr;
  g_conn[0].ct_ptr = CT_PTR(immr);
  g_conn[0].tcte_ptr = TCTE_PTR(immr);

  return OK;
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmIntInit
 *
 * DESCRIPTION:
 *
 * Initialization of the MPC860 ESAR Interrupt Queue.
 * This function
 * - clears all entries in the INT,
 * - sets the WRAP bit of the last INT entry,
 * - initializes the 'int_serv_ptr' attribuut of the AtmDriver structure
 *   to the first INT entry.
 *
 * PARAMETERS: none
 *
 * RETURNS: void
 *
 * REMARKS:
 *
 * - The INT resides in external cache safe memory.
 * - The base address of the INT is stored in g_atm.int_reload_ptr.
 * - The number of entries in the INT is given by NUM_INT_ENTRIES.
 * - The INTBASE field in SAR Parameter RAM is set by atmCpmInit().
 *
 ****************************************************************************/
void atmIntInit()
{
  int i;
  for ( i = 0; i < NUM_INT_ENTRIES - 1; ++i) g_atm.int_reload_ptr[i] = 0;
  g_atm.int_reload_ptr[i] = INT_WRAP;
  g_atm.int_serv_ptr = g_atm.int_reload_ptr;
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmApcInit
 *
 * DESCRIPTION:
 *
 * This function initializes the following ATM Pace Controller related
 * data structures:
 *
 * - 1 MPHY Pointing Table (contains only one entry)
 * - 3 APC Parameter Tables (one PHY with 3 priorities)
 * - 3 APC Tables (one table for each priority)
 * - 1 Transmit Queue (one transmit queue per PHY)
 *
 * PARAMETERS: none
 *
 * RETURNS: void
 *
 ****************************************************************************/
void atmApcInit()
{
  int i;
  /* unsigned immr = CONFIG_SYS_IMMR; */
  uint16 * mphypt_ptr = MPHYPT_PTR(CONFIG_SYS_IMMR);
  struct apc_params_t * apcp_ptr = APCP_PTR(CONFIG_SYS_IMMR);
  uint16 * apct_prio1_ptr = APCT1_PTR(CONFIG_SYS_IMMR);
  uint16 * tq_ptr = TQ_PTR(CONFIG_SYS_IMMR);
  /***************************************************/
  /* Initialize MPHY Pointing Table (only one entry) */
  /***************************************************/
  *mphypt_ptr = APCP_BASE;

  /********************************************/
  /* Initialize APC parameters for priority 1 */
  /********************************************/
  apcp_ptr->apct_base1 = APCT_PRIO_1_BASE;
  apcp_ptr->apct_end1  =  APCT_PRIO_1_BASE + NUM_APCT_PRIO_1_ENTRIES * 2;
  apcp_ptr->apct_ptr1  =  APCT_PRIO_1_BASE;
  apcp_ptr->apct_sptr1 = APCT_PRIO_1_BASE;
  apcp_ptr->etqbase    = TQ_BASE;
  apcp_ptr->etqend     =  TQ_BASE + ( NUM_TQ_ENTRIES - 1 ) * 2;
  apcp_ptr->etqaptr    = TQ_BASE;
  apcp_ptr->etqtptr    = TQ_BASE;
  apcp_ptr->apc_mi     = 8;
  apcp_ptr->ncits      = 0x0100;   /* NCITS = 1 */
  apcp_ptr->apcnt      = 0;
  apcp_ptr->reserved1  = 0;
  apcp_ptr->eapcst     = 0x2009;  /* LAST, ESAR, MPHY */
  apcp_ptr->ptp_counter = 0;
  apcp_ptr->ptp_txch   = 0;
  apcp_ptr->reserved2  = 0;


  /***************************************************/
  /* Initialize APC Tables with empty slots (0xFFFF) */
  /***************************************************/
  for ( i = 0; i < NUM_APCT_PRIO_1_ENTRIES; ++i ) *(apct_prio1_ptr++) = 0xFFFF;

  /************************/
  /* Clear Transmit Queue */
  /************************/
  for ( i = 0; i < NUM_TQ_ENTRIES; ++i ) *(tq_ptr++) = 0;
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmAmtInit
 *
 * DESCRIPTION:
 *
 * This function clears the first entry in the Address Matching Table and
 * lets the first entry in the Address Pointing table point to the first
 * entry in the TCT table (i.e. the raw cell channel).
 *
 * PARAMETERS: none
 *
 * RETURNS: void
 *
 * REMARKS:
 *
 * The values for the AMBASE, AMEND and APBASE registers in SAR parameter
 * RAM are initialized by atmCpmInit().
 *
 ****************************************************************************/
void atmAmtInit()
{
  unsigned immr = CONFIG_SYS_IMMR;

  g_atm.am_top = AM_PTR(immr);
  g_atm.ap_top = AP_PTR(immr);

  *(g_atm.ap_top--) = CT_BASE;
  *(g_atm.am_top--) = 0;
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmCpmInit
 *
 * DESCRIPTION:
 *
 * This function initializes the Utopia Interface Parameter RAM Map
 * (SCC4, ATM Protocol) of the Communication Processor Modudule.
 *
 * PARAMETERS: none
 *
 * RETURNS: void
 *
 ****************************************************************************/
void atmCpmInit()
{
  unsigned immr = CONFIG_SYS_IMMR;

  memset((char *)immr + 0x3F00, 0x00, 0xC0);

  /*-----------------------------------------------------------------*/
  /* RBDBASE - Receive buffer descriptors base address               */
  /* The RBDs reside in cache safe external memory.                  */
  /*-----------------------------------------------------------------*/
  *RBDBASE(immr) = (uint32)g_atm.rbd_base_ptr;

  /*-----------------------------------------------------------------*/
  /* SRFCR - SAR receive function code                               */
  /* 0-2 rsvd = 000                                                  */
  /* 3-4 BO   = 11  Byte ordering (big endian).                      */
  /* 5-7 FC   = 000 Value driven on the address type signals AT[1-3] */
  /*                when the SDMA channel accesses memory.           */
  /*-----------------------------------------------------------------*/
  *SRFCR(immr) = 0x18;

  /*-----------------------------------------------------------------*/
  /* SRSTATE - SAR receive status                                    */
  /* 0 EXT  = 0 Extended mode off.                                   */
  /* 1 ACP  = 0 Valid only if EXT = 1.                               */
  /* 2 EC   = 0 Standard 53-byte ATM cell.                           */
  /* 3 SNC  = 0 In sync. Must be set to 0 during initialization.     */
  /* 4 ESAR = 1 Enhanced SAR functionality enabled.                  */
  /* 5 MCF  = 1 Management Cell Filter active.                       */
  /* 6 SER  = 0 UTOPIA mode.                                         */
  /* 7 MPY  = 1 Multiple PHY mode.                                   */
  /*-----------------------------------------------------------------*/
  *SRSTATE(immr) = 0x0D;

  /*-----------------------------------------------------------------*/
  /* MRBLR - Maximum receive buffer length register.                 */
  /* Must be cleared for ATM operation (see also SMRBLR).            */
  /*-----------------------------------------------------------------*/
  *MRBLR(immr) = 0;

  /*-----------------------------------------------------------------*/
  /* RSTATE - SCC internal receive state parameters                  */
  /* The first byte must be initialized with the value of SRFCR.     */
  /*-----------------------------------------------------------------*/
  *RSTATE(immr) = (uint32)(*SRFCR(immr)) << 24;

  /*-----------------------------------------------------------------*/
  /* STFCR - SAR transmit function code                              */
  /* 0-2 rsvd = 000                                                  */
  /* 3-4 BO   = 11  Byte ordering (big endian).                      */
  /* 5-7 FC   = 000 Value driven on the address type signals AT[1-3] */
  /*                when the SDMA channel accesses memory.           */
  /*-----------------------------------------------------------------*/
  *STFCR(immr) = 0x18;

  /*-----------------------------------------------------------------*/
  /* SRSTATE - SAR transmit status                                   */
  /* 0 EXT  = 0 : Extended mode off                                  */
  /* 1 rsvd = 0 :                                                    */
  /* 2 EC   = 0 : Standard 53-byte ATM cell                          */
  /* 3 rsvd = 0 :                                                    */
  /* 4 ESAR = 1 : Enhanced SAR functionality enabled                 */
  /* 5 rsvd = 0 :                                                    */
  /* 6 SER  = 0 : UTOPIA mode                                        */
  /* 7 MPY  = 1 : Multiple PHY mode                                  */
  /*-----------------------------------------------------------------*/
  *STSTATE(immr) = 0x09;

  /*-----------------------------------------------------------------*/
  /* TBDBASE - Transmit buffer descriptors base address              */
  /* The TBDs reside in cache safe external memory.                  */
  /*-----------------------------------------------------------------*/
  *TBDBASE(immr) = (uint32)g_atm.tbd_base_ptr;

  /*-----------------------------------------------------------------*/
  /* TSTATE - SCC internal transmit state parameters                 */
  /* The first byte must be initialized with the value of STFCR.     */
  /*-----------------------------------------------------------------*/
  *TSTATE(immr) = (uint32)(*STFCR(immr)) << 24;

  /*-----------------------------------------------------------------*/
  /* CTBASE - Connection table base address                          */
  /* Offset from the beginning of DPRAM (64-byte aligned).           */
  /*-----------------------------------------------------------------*/
  *CTBASE(immr) = CT_BASE;

  /*-----------------------------------------------------------------*/
  /* INTBASE - Interrupt queue base pointer.                         */
  /* The interrupt queue resides in cache safe external memory.      */
  /*-----------------------------------------------------------------*/
  *INTBASE(immr) = (uint32)g_atm.int_reload_ptr;

  /*-----------------------------------------------------------------*/
  /* INTPTR - Pointer into interrupt queue.                          */
  /* Initialize to INTBASE.                                          */
  /*-----------------------------------------------------------------*/
  *INTPTR(immr) = *INTBASE(immr);

  /*-----------------------------------------------------------------*/
  /* C_MASK - Constant mask for CRC32                                */
  /* Must be initialized to 0xDEBB20E3.                              */
  /*-----------------------------------------------------------------*/
  *C_MASK(immr) = 0xDEBB20E3;

  /*-----------------------------------------------------------------*/
  /* INT_ICNT - Interrupt threshold value                            */
  /*-----------------------------------------------------------------*/
  *INT_ICNT(immr) = 1;

  /*-----------------------------------------------------------------*/
  /* INT_CNT - Interrupt counter                                     */
  /* Initalize to INT_ICNT. Decremented for each interrupt entry     */
  /* reported in the interrupt queue. On zero an interrupt is        */
  /* signaled to the host by setting the GINT bit in the event       */
  /* register. The counter is reinitialized with INT_ICNT.           */
  /*-----------------------------------------------------------------*/
  *INT_CNT(immr) = *INT_ICNT(immr);

  /*-----------------------------------------------------------------*/
  /* SMRBLR - SAR maximum receive buffer length register.            */
  /* Must be a multiple of 48 bytes. Common for all ATM connections. */
  /*-----------------------------------------------------------------*/
  *SMRBLR(immr) = SAR_RXB_SIZE;

  /*-----------------------------------------------------------------*/
  /* APCST - APC status register.                                    */
  /* 0     rsvd 0                                                    */
  /* 1-2   CSER 11  Initialize with the same value as NSER.          */
  /* 3-4   NSER 11  Next serial or UTOPIA channel.                   */
  /* 5-7   rsvd 000                                                  */
  /* 8-10  rsvd 000                                                  */
  /* 11    rsvd 0                                                    */
  /* 12    ESAR 1   UTOPIA Level 2 MPHY enabled.                     */
  /* 13    DIS  0   APC disable. Must be initiazed to 0.             */
  /* 14    PL2  0   Not used.                                        */
  /* 15    MPY  1   Multiple PHY mode on.                            */
  /*-----------------------------------------------------------------*/
  *APCST(immr) = 0x7809;

  /*-----------------------------------------------------------------*/
  /* APCPTR - Pointer to the APC parameter table                     */
  /* In MPHY master mode this parameter points to the MPHY pointing  */
  /* table. 2-byte aligned.                                          */
  /*-----------------------------------------------------------------*/
  *APCPTR(immr) = MPHYPT_BASE;

  /*-----------------------------------------------------------------*/
  /* HMASK - Header mask                                             */
  /* Each incoming cell is masked with HMASK before being compared   */
  /* to the entries in the address matching table.                   */
  /*-----------------------------------------------------------------*/
  *HMASK(immr) = AM_HMASK;

  /*-----------------------------------------------------------------*/
  /* AMBASE - Address matching table base address                    */
  /*-----------------------------------------------------------------*/
  *AMBASE(immr) = AM_BASE;

  /*-----------------------------------------------------------------*/
  /* AMEND - Address matching table end address                      */
  /*-----------------------------------------------------------------*/
  *AMEND(immr) = AM_BASE;

  /*-----------------------------------------------------------------*/
  /* APBASE - Address pointing table base address                    */
  /*-----------------------------------------------------------------*/
  *APBASE(immr) = AP_BASE;

  /*-----------------------------------------------------------------*/
  /* MPHYST - MPHY status register                                   */
  /* 0-1   rsvd  00                                                  */
  /* 2-6   NMPHY 00000 1 PHY                                         */
  /* 7-9   rsvd  000                                                 */
  /* 10-14 CMPHY 00000 Initialize with same value as NMPHY           */
  /*-----------------------------------------------------------------*/
  *MPHYST(immr) = 0x0000;

  /*-----------------------------------------------------------------*/
  /* TCTEBASE - Transmit connection table extension base address     */
  /* Offset from the beginning of DPRAM (32-byte aligned).           */
  /*-----------------------------------------------------------------*/
  *TCTEBASE(immr) = TCTE_BASE;

  /*-----------------------------------------------------------------*/
  /* Clear not used registers.                                       */
  /*-----------------------------------------------------------------*/
}

/*****************************************************************************
 *
 * FUNCTION NAME: atmUtpInit
 *
 * DESCRIPTION:
 *
 * This function initializes the ATM interface for
 *
 * - UTOPIA mode
 * - muxed bus
 * - master operation
 * - multi PHY (because of a bug in the MPC860P rev. E.0)
 * - internal clock = SYSCLK / 2
 *
 * EXTERNAL EFFECTS:
 *
 * After calling this function, the MPC860ESAR UTOPIA bus is
 * active and uses the following ports/pins:
 *
 * Port    Pin  Signal   Description
 * ------  ---  -------  -------------------------------------------
 * PB[15]  R17  TxClav   Transmit cell available input/output signal
 * PC[15]  D16  RxClav   Receive cell available input/output signal
 * PD[15]  U17  UTPB[0]  UTOPIA bus bit 0 input/output signal
 * PD[14]  V19  UTPB[1]  UTOPIA bus bit 1 input/output signal
 * PD[13]  V18  UTPB[2]  UTOPIA bus bit 2 input/output signal
 * PD[12]  R16  UTPB[3]  UTOPIA bus bit 3 input/output signal
 * PD[11]  T16  RXENB    Receive enable input/output signal
 * PD[10]  W18  TXENB    Transmit enable input/output signal
 * PD[9]   V17  UTPCLK   UTOPIA clock input/output signal
 * PD[7]   T15  UTPB[4]  UTOPIA bus bit 4 input/output signal
 * PD[6]   V16  UTPB[5]  UTOPIA bus bit 5 input/output signal
 * PD[5]   U15  UTPB[6]  UTOPIA bus bit 6 input/output signal
 * PD[4]   U16  UTPB[7]  UTOPIA bus bit 7 input/output signal
 * PD[3]   W16  SOC      Start of cell input/output signal
 *
 * PARAMETERS: none
 *
 * RETURNS: void
 *
 * REMARK:
 *
 * The ATM parameters and data structures must be configured before
 * initializing the UTOPIA port. The UTOPIA port activates immediately
 * upon initialization, and if its associated data structures are not
 * initialized, the CPM will lock up.
 *
 ****************************************************************************/
void atmUtpInit()
{
  volatile immap_t       *immap  = (immap_t *)CONFIG_SYS_IMMR;
  volatile iop8xx_t      *iop    = &immap->im_ioport;
  volatile car8xx_t	 *car    = &immap->im_clkrst;
  volatile cpm8xx_t	 *cpm    = &immap->im_cpm;
  int flag;

  flag = disable_interrupts();

  /*-----------------------------------------------------------------*/
  /* SCCR - System Clock Control Register                            */
  /*                                                                 */
  /* The UTOPIA clock can be selected to be internal clock or        */
  /* external clock (selected by the UTOPIA mode register).          */
  /* In case of internal clock, the UTOPIA clock is derived from     */
  /* the system frequency divided by two dividers.                   */
  /* Bits 27-31 of the SCCR register are defined to control the      */
  /* UTOPIA clock.                                                   */
  /*                                                                 */
  /* SCCR[27:29] DFUTP  Division factor. Divide the system clock     */
  /*                    by 2^DFUTP.                                  */
  /* SCCR[30:31] DFAUTP Additional division factor. Divide the       */
  /*                    system clock by the following value:         */
  /*                    00 = divide by 1                             */
  /*                    00 = divide by 3                             */
  /*                    10 = divide by 5                             */
  /*                    11 = divide by 7                             */
  /*                                                                 */
  /* Note that the UTOPIA clock must be programmed as to operate     */
  /* within the range SYSCLK/10 .. 50MHz.                            */
  /*-----------------------------------------------------------------*/
  car->car_sccr &= 0xFFFFFFE0;
  car->car_sccr |= 0x00000008; /* UTPCLK = SYSCLK / 4 */

  /*-----------------------------------------------------------------*/
  /* RCCR - RISC Controller Configuration Register                   */
  /*                                                                 */
  /* RCCR[8]     DR1M IDMA Request 0 Mode                            */
  /*                  0 = edge sensitive                             */
  /*                  1 = level sensitive                            */
  /* RCCR[9]     DR0M IDMA Request 0 Mode                            */
  /*                  0 = edge sensitive                             */
  /*                  1 = level sensitive                            */
  /* RCCR[10:11] DRQP IDMA Request Priority                          */
  /*                  00 = IDMA req. have more prio. than SCCs       */
  /*                  01 = IDMA req. have less prio. then SCCs       */
  /*                  10 = IDMA requests have the lowest prio.       */
  /*                  11 = reserved                                  */
  /*                                                                 */
  /* The RCCR[DR0M] and RCCR[DR1M] bits must be set to enable UTOPIA */
  /* operation. Also, program RCCR[DPQP] to 01 to give SCC transfers */
  /* higher priority.                                                */
  /*-----------------------------------------------------------------*/
  cpm->cp_rccr &= 0xFF0F;
  cpm->cp_rccr |= 0x00D0;

  /*-----------------------------------------------------------------*/
  /* Port B - TxClav Signal                                          */
  /*-----------------------------------------------------------------*/
  cpm->cp_pbpar |= 0x00010000; /* PBPAR[15] = 1 */
  cpm->cp_pbdir &= 0xFFFEFFFF; /* PBDIR[15] = 0 */

  /*-----------------------------------------------------------------*/
  /* UTOPIA Mode Register                                            */
  /*                                                                 */
  /* - muxed bus (master operation only)                             */
  /* - multi PHY (because of a bug in the MPC860P rev.E.0)           */
  /* - internal clock                                                */
  /* - no loopback                                                   */
  /* - do no activate statistical counters                           */
  /*-----------------------------------------------------------------*/
  iop->utmode = 0x00000004; SYNC;

  /*-----------------------------------------------------------------*/
  /* Port D - UTOPIA Data and Control Signals                        */
  /*                                                                 */
  /* 15-12 UTPB[0:3] UTOPIA bus bit 0 - 3 input/output signals       */
  /* 11    RXENB     UTOPIA receive enable input/output signal       */
  /* 10    TXENB     UTOPIA transmit enable input/output signal      */
  /* 9     TUPCLK    UTOPIA clock input/output signal                */
  /* 8     MII-MDC   Used by MII in simult. MII and UTOPIA operation */
  /* 7-4   UTPB[4:7] UTOPIA bus bit 4 - 7 input/output signals       */
  /* 3     SOC       UTOPIA Start of cell input/output signal        */
  /* 2               Reserved                                        */
  /* 1               Enable UTOPIA mode                              */
  /* 0               Enable SAR                                      */
  /*-----------------------------------------------------------------*/
  iop->iop_pdpar |= 0xDF7F; SYNC;
  iop->iop_pddir &= 0x2080; SYNC;

  /*-----------------------------------------------------------------*/
  /* Port C - RxClav Signal                                          */
  /*-----------------------------------------------------------------*/
  iop->iop_pcpar  |= 0x0001; /* PCPAR[15] = 1 */
  iop->iop_pcdir  &= 0xFFFE; /* PCDIR[15] = 0 */
  iop->iop_pcso   &= 0xFFFE; /* PCSO[15]  = 0 */

  if (flag)
    enable_interrupts();
}
