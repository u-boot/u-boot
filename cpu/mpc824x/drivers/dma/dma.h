#ifndef DMA_H
#define DMA_H
/*******************************************************
 *
 * copyright @ Motorola 1999
 *
 *******************************************************/
#define NUM_DMA_REG   7
#define DMA_MR_REG    0
#define DMA_SR_REG    1
#define DMA_CDAR_REG  2
#define DMA_SAR_REG   3
#define DMA_DAR_REG   4
#define DMA_BCR_REG   5
#define DMA_NDAR_REG  6

typedef enum _dmastatus
{
	DMASUCCESS = 0x1000,
	DMALMERROR,
	DMAPERROR,
	DMACHNBUSY,
	DMAEOSINT,
	DMAEOCAINT,
	DMAINVALID,
	DMANOEVENT,
} DMAStatus;

typedef enum _location
{
	LOCAL = 0,     /* local processor accesses on board DMA,
			          local processor's eumbbar is required */
	REMOTE = 1,    /* PCI master accesses DMA on I/O board,
			          I/O processor's pcsrbar is required */
} LOCATION;

typedef enum dma_mr_bit
{
	IRQS    = 0x00080000,
    PDE     = 0x00040000,
	DAHTS   = 0x00030000,
	SAHTS   = 0x0000c000,
	DAHE    = 0x00002000,
	SAHE    = 0x00001000,
	PRC     = 0x00000c00,
	EIE     = 0x00000080,
	EOTIE   = 0x00000040,
	DL      = 0x00000008,
	CTM     = 0x00000004,
	CC      = 0x00000002,
	CS      = 0x00000001,
} DMA_MR_BIT;

typedef enum dma_sr_bit
{
	LME     = 0x00000080,
	PE      = 0x00000010,
	CB      = 0x00000004,
	EOSI    = 0x00000002,
	EOCAI   = 0x00000001,
} DMA_SR_BIT;

/* structure for DMA Mode Register */
typedef struct _dma_mr
{
	unsigned int  reserved0 : 12;
	unsigned int  irqs      : 1;
	unsigned int  pde       : 1;
	unsigned int  dahts     : 2;
    unsigned int  sahts     : 2;
	unsigned int  dahe      : 1;
	unsigned int  sahe      : 1;
	unsigned int  prc       : 2;
	unsigned int  reserved1 : 1;
	unsigned int  eie       : 1;
	unsigned int  eotie     : 1;
	unsigned int  reserved2 : 3;
	unsigned int  dl        : 1;
	unsigned int  ctm       : 1;
	/* if chaining mode is enabled, any time, user can modify the
	 * descriptor and does not need to halt the current DMA transaction.
	 * Set CC bit, enable DMA to process the modified descriptors
	 * Hardware will clear this bit each time, DMA starts.
	 */
	unsigned int  cc        : 1;
	/* cs bit has dua role, halt the current DMA transaction and
	 * (re)start DMA transaction. In chaining mode, if the descriptor
	 * needs modification, cs bit shall be used not the cc bit.
	 * Hardware will not set/clear this bit each time DMA transaction
	 * stops or starts. Software shall do it.
	 *
	 * cs bit shall not be used to halt chaining DMA transaction for
	 * modifying the descriptor. That is the role of CC bit.
	 */
	unsigned int  cs        : 1;
} DMA_MR;

/* structure for DMA Status register */
typedef struct _dma_sr
{
	unsigned int  reserved0 : 24;
	unsigned int  lme       : 1;
	unsigned int  reserved1 : 2;
	unsigned int  pe        : 1;
	unsigned int  reserved2 : 1;
	unsigned int  cb        : 1;
	unsigned int  eosi      : 1;
	unsigned int  eocai     : 1;
} DMA_SR;

/* structure for DMA current descriptor address register */
typedef struct _dma_cdar
{
	unsigned int  cda    : 27;
	unsigned int snen    : 1;
	unsigned int eosie   : 1;
	unsigned int ctt     : 2;
	unsigned int eotd    : 1;
} DMA_CDAR;

/* structure for DMA byte count register */
typedef struct _dma_bcr
{
	unsigned int reserved : 6;
	unsigned int  bcr      : 26;
} DMA_BCR;

/* structure for DMA Next Descriptor Address register */
typedef struct _dma_ndar
{
	unsigned int nda    : 27;
	unsigned int ndsnen : 1;
	unsigned int ndeosie: 1;
	unsigned int ndctt  : 2;
	unsigned int eotd   : 1;
} DMA_NDAR;

/* structure for DMA current transaction info */
typedef struct _dma_curr
{
	unsigned int src_addr;
	unsigned int dest_addr;
	unsigned int byte_cnt;
} DMA_CURR;

/************************* Kernel API********************
 * Kernel APIs are used to interface with O.S. kernel.
 * They are the functions required by O.S. kernel to
 * provide I/O service.
 ********************************************************/

/**************DMA Device Control Functions ********/

/**
 * Note:
 *
 * In all following functions, the host (KAHLUA) processor has a
 * choice of accessing on board local DMA (LOCAL),
 * or DMA on a distributed KAHLUA (REMOTE). In either case,
 * the caller shall pass the configured embedded utility memory
 * block base address relative to the DMA. If LOCAL DMA is used,
 * this parameter shall be EUMBBAR, if REMOTE is used, the
 * parameter shall be the corresponding PCSRBAR.
 **/

/**************************************************************
 * function: DMA_Get_Stat
 *
 * description: return the content of status register of
 *              the given DMA channel
 *              if error, return DMAINVALID. Otherwise return
 *              DMASUCCESS.
 *
 **************************************************************/
static DMAStatus DMA_Get_Stat( LOCATION, unsigned int eumbbar, unsigned int channel, DMA_SR * );

/**************************************************************
 * function: DMA_Get_Mode
 *
 * description: return the content of mode register of the
 *              given DMA channel
 *              if error, return DMAINVALID. Otherwise return DMASUCCESS.
 *
 **************************************************************/
static DMAStatus DMA_Get_Mode( LOCATION, unsigned int eumbbar, unsigned int channel, DMA_MR * );

/**************************************************************
 * function: DMA_Set_Mode
 *
 * description: Set a new mode to a given DMA channel
 *              return DMASUCCESS if success, otherwise return DMACHNINVALID
 *
 * note: It is not a good idea of changing the DMA mode during
 *       the middle of a transaction.
 **************************************************************/
static DMAStatus DMA_Set_Mode( LOCATION, unsigned int eumbbar, unsigned int channel, DMA_MR mode );

/*************************************************************
 * function: DMA_ISR
 *
 * description: DMA interrupt service routine
 *              return DMAStatus based on the status
 *
 *************************************************************/
static DMAStatus    DMA_ISR( unsigned int eumbbar,
							 unsigned int channel,
						     DMAStatus (*lme_func)( unsigned int, unsigned int, DMAStatus ),
					         DMAStatus (*pe_func) ( unsigned int, unsigned int, DMAStatus ),
					         DMAStatus (*eosi_func)( unsigned int, unsigned int, DMAStatus ),
					         DMAStatus (*eocai_func)(unsigned int, unsigned int, DMAStatus ));

static DMAStatus dma_error_func( unsigned int, unsigned int, DMAStatus );

/********************* DMA I/O function ********************/

/************************************************************
 * function: DMA_Start
 *
 * description: start a given DMA channel transaction
 *              return DMASUCCESS if success, otherwise return DMACHNINVALID
 *
 * note: this function will clear DMA_MR(CC) first, then
 *       set DMA_MR(CC).
 ***********************************************************/
static DMAStatus DMA_Start( LOCATION, unsigned int eumbbar,unsigned int channel );

/***********************************************************
 * function: DMA_Halt
 *
 * description: halt the current dma transaction on the specified
 *              channel.
 *              return DMASUCCESS if success, otherwise return DMACHNINVALID
 *
 * note: if the specified DMA channel is idle, nothing happens
 *************************************************************/
static DMAStatus DMA_Halt( LOCATION, unsigned int eumbbar,unsigned int channel );

/*************************************************************
 * function: DMA_Chn_Cnt
 *
 * description: set the DMA_MR(CC) bit for a given channel
 *              that is in chaining mode.
 *              return DMASUCCESS if successfule, otherwise return DMACHNINVALID
 *
 * note: if the given channel is not in chaining mode, nothing
 *       happen.
 *
 *************************************************************/
static DMAStatus DMA_Chn_Cnt( LOCATION, unsigned int eumbbar,unsigned int channel );

/*********************** App. API ***************************
 * App. API are the APIs Kernel provides for the application
 * level program
 ************************************************************/
/**************************************************************
 * function: DMA_Bld_Curr
 *
 * description: set current src, dest, byte count registers
 *              according to the desp for a given channel
 *
 *              if the given channel is busy,  no change made,
 *              return DMACHNBUSY.
 *
 *              otherwise return DMASUCCESS.
 *
 * note:
 **************************************************************/
static DMAStatus DMA_Bld_Curr( LOCATION,
								  unsigned int eumbbar,
								  unsigned int channel,
							      DMA_CURR     desp );

/**************************************************************
 * function: DMA_Poke_Curr
 *
 * description: poke the current src, dest, byte count registers
 *              for a given channel.
 *
 *              return DMASUCCESS if no error otherwise return DMACHNERROR
 *
 * note:        Due to the undeterministic parallelism, in chaining
 *              mode, the value returned by this function shall
 *              be taken as reference when the query is made rather
 *              than the absolute snapshot when the value is returned.
 **************************************************************/
static DMAStatus DMA_Poke_Curr( LOCATION,
							   unsigned int eumbbar,
							   unsigned int channel,
						       DMA_CURR*    desp );

/**************************************************************
 * function: DMA_Bld_Desp
 *
 * description: set current descriptor address register
 *              according to the desp for a given channel
 *
 *              if the given channel is busy return DMACHNBUSY
 *              and no change made, otherwise return DMASUCCESS.
 *
 * note:
 **************************************************************/
static DMAStatus DMA_Bld_Desp( LOCATION host,
					          unsigned int eumbbar,
					          unsigned int channel,
					          DMA_CDAR     desp );

/**************************************************************
 * function: DMA_Poke_Desp
 *
 * description: poke the current descriptor address register
 *              for a given channel
 *
 *              return DMASUCCESS if no error otherwise return
 *              DMAINVALID
 *
 * note: Due to the undeterministic parallellism of DMA operation,
 *       the value returned by this function shall be taken as
 *       the most recently used descriptor when the last time
 *       DMA starts a chaining mode operation.
 **************************************************************/
static DMAStatus DMA_Poke_Desp( LOCATION,
							   unsigned int eumbbar,
							   unsigned int channel,
						       DMA_CDAR     *desp );

#endif
