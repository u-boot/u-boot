/************************************************************
 *
 * copyright @ Motorola, 1999
 *
 * App. API
 *
 * App. API are the APIs Kernel provides for the application
 * level program
 *
 ************************************************************/
#include "dma_export.h"
#include "dma.h"

/* Define a macro to use an optional application-layer print function, if
 * one was passed to the library during initialization.  If there was no
 * function pointer passed, this protects against referencing a NULL pointer.
 * Also define The global variable that holds the passed pointer.
 */
#define PRINT if ( app_print ) app_print
static int (*app_print)(char *,...);

/* Set by call to get_eumbbar during DMA_Initialize.
 * This could be globally available to the library, but there is
 * an advantage to passing it as a parameter: it is already in a register
 * and doesn't have to be loaded from memory.  Also, that is the way the
 * library was already implemented and I don't want to change it without
 * a more detailed analysis.
 * It is being set as a global variable during initialization to hide it from
 * the DINK application layer, because it is Kahlua-specific.  I think that
 * get_eumbbar, load_runtime_reg, and store_runtime_reg should be defined in
 * a Kahlua-specific library dealing with the embedded utilities memory block.
 * Right now, get_eumbbar is defined in dink32/kahlua.s.  The other two are
 * defined in dink32/drivers/i2c/i2c2.s, drivers/dma/dma2.s, etc.
 */
static unsigned int Global_eumbbar = 0;
extern unsigned int get_eumbbar();


extern unsigned int load_runtime_reg( unsigned int eumbbar, unsigned int reg );
#pragma Alias( load_runtime_reg, "load_runtime_reg" );

extern void store_runtime_reg( unsigned int eumbbar, unsigned int reg, unsigned int val );
#pragma Alias( store_runtime_reg, "store_runtime_reg" );

unsigned int dma_reg_tb[][14] = {
	/* local DMA registers */
	{
      /* DMA_0_MR   */  0x00001100,
      /* DMA_0_SR   */  0x00001104,
      /* DMA_0_CDAR */  0x00001108,
      /* DMA_0_SAR  */  0x00001110,
      /* DMA_0_DAR  */  0x00001118,
      /* DMA_0_BCR  */  0x00001120,
      /* DMA_0_NDAR */  0x00001124,
      /* DMA_1_MR   */  0x00001200,
      /* DMA_1_SR   */  0x00001204,
      /* DMA_1_CDAR */  0x00001208,
      /* DMA_1_SAR  */  0x00001210,
      /* DMA_1_DAR  */  0x00001218,
      /* DMA_1_BCR  */  0x00001220,
      /* DMA_1_NDAR */  0x00001224,
	},
	/* remote DMA registers */
	{
      /* DMA_0_MR   */  0x00000100,
      /* DMA_0_SR   */  0x00000104,
      /* DMA_0_CDAR */  0x00000108,
      /* DMA_0_SAR  */  0x00000110,
      /* DMA_0_DAR  */  0x00000118,
      /* DMA_0_BCR  */  0x00000120,
      /* DMA_0_NDAR */  0x00000124,
      /* DMA_1_MR   */  0x00000200,
      /* DMA_1_SR   */  0x00000204,
      /* DMA_1_CDAR */  0x00000208,
      /* DMA_1_SAR  */  0x00000210,
      /* DMA_1_DAR  */  0x00000218,
      /* DMA_1_BCR  */  0x00000220,
      /* DMA_1_NDAR */  0x00000224,
	},
};

/* API functions */

/*  Initialize DMA unit with the following:
 *  optional pointer to application layer print function
 *
 *  These parameters may be added:
 *  ???
 *  Interrupt enables, modes, etc. are set for each transfer.
 *
 *  This function must be called before DMA unit can be used.
 */
extern
DMA_Status DMA_Initialize( int (*p)(char *,...))
{
  DMAStatus status;
  /* establish the pointer, if there is one, to the application's "printf" */
  app_print = p;

  /* If this is the first call, get the embedded utilities memory block
   * base address.  I'm not sure what to do about error handling here:
   * if a non-zero value is returned, accept it.
   */
  if ( Global_eumbbar == 0)
     Global_eumbbar = get_eumbbar();
  if ( Global_eumbbar == 0)
  {
    PRINT( "DMA_Initialize: can't find EUMBBAR\n" );
    return DMA_ERROR;
  }

  return DMA_SUCCESS;
}


/* Perform the DMA transfer, only direct mode is currently implemented.
 * At this point, I think it would be better to define a different
 * function for chaining mode.
 * Also, I'm not sure if it is appropriate to have the "generic" API
 * accept snoop and int_steer parameters.  The DINK user interface allows
 * them, so for now I'll leave them.
 *
 * int_steer controls DMA interrupt steering to PCI or local processor
 * type is the type of transfer: M2M, M2P, P2M, P2P
 * source is the source address of the data
 * dest is the destination address of the data
 * len is the length of data to transfer
 * channel is the DMA channel to use for the transfer
 * snoop is the snoop enable control
 */
extern DMA_Status DMA_direct_transfer( DMA_INTERRUPT_STEER int_steer,
				       DMA_TRANSFER_TYPE type,
				       unsigned int source,
				       unsigned int dest,
				       unsigned int len,
				       DMA_CHANNEL channel,
				       DMA_SNOOP_MODE snoop)
{
    DMA_MR md;
    DMA_CDAR cdar;
    /* it's inappropriate for curr to be a struct, but I'll leave it */
    DMA_CURR curr;

    DMAStatus stat;

	/* The rest of this code was moved from device.c test_dma to here.
	 * It needs to be cleaned up and validated, but at least it is removed
	 * from the application and API.  Most of the mode is left hard coded.
	 * This should be changed after the final API is defined and the user
	 * application has a way to control the transfer.
	 *
	 */

	if ( DMA_Get_Mode( LOCAL, Global_eumbbar, channel, &md ) != DMASUCCESS )
	{
		return DMA_ERROR;
	}

	md.irqs = int_steer;
	md.pde = 0;
	md.dahts = 3; /* 8 - byte */
	md.sahts = 3; /* 8 - byte */
	md.dahe = 0;
	md.sahe = 0;
	md.prc = 0;
	/* if steering interrupts to local processor, use polling mode */
	if ( int_steer == DMA_INT_STEER_PCI )
	{
		md.eie = 1;
		md.eotie = 1;
	} else {
		md.eie = 0;
		md.eotie = 0;
	}
	md.dl = 0;
	md.ctm = 1;   /* direct mode */
    md.cc = 0;

	/* validate the length range */
	if (len > 0x3ffffff )
	{
		PRINT( "dev DMA: length of transfer too large: %d\n", len );
		return DMA_ERROR;
	}

	/* inappropriate to use a struct, but leave as is for now */
	curr.src_addr = source;
	curr.dest_addr = dest;
	curr.byte_cnt = len;

	(void)DMA_Poke_Desp( LOCAL, Global_eumbbar, channel, &cdar );
	cdar.snen = snoop;
	cdar.ctt = type;

	if ( ( stat = DMA_Bld_Desp( LOCAL, Global_eumbbar, channel, cdar ))
			!= DMASUCCESS ||
		 ( stat = DMA_Bld_Curr( LOCAL, Global_eumbbar, channel, curr ))
			!= DMASUCCESS ||
	     ( stat = DMA_Set_Mode( LOCAL, Global_eumbbar, channel, md ))
			!= DMASUCCESS ||
		 ( stat = DMA_Start( LOCAL, Global_eumbbar, channel ))
			!= DMASUCCESS )
	{
		if ( stat == DMACHNBUSY )
		{
			PRINT( "dev DMA: channel %d busy.\n", channel );
		}
		else
		{
			PRINT( "dev DMA: invalid channel request.\n", channel );
		}

		return DMA_ERROR;
	}

/* Since we are interested at the DMA performace right now,
   we are going to do as less as possible to burden the
   603e core.

   if you have epic enabled or don't care the return from
   DMA operation, you can just return SUCCESS.

   if you don't have epic enabled and care the DMA result,
   you can use the polling method below.

   Note: I'll attempt to activate the code for handling polling.
 */

#if 0
	/* if steering interrupt to local processor, let it handle results */
	if ( int_steer == DMA_INT_STEER_LOCAL )
	{
	    return DMA_SUCCESS;
	}

	/* polling since interrupt goes to PCI */
	do
	{
		stat = DMA_ISR( Global_eumbbar, channel, dma_error_func,
			dma_error_func, dma_error_func, dma_error_func );
	}
	while ( stat == DMANOEVENT );
#endif

    return DMA_SUCCESS;
}

/* DMA library internal functions */

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
 *
 *              if error, reserved0 field all 1s.
 **************************************************************/
static
DMAStatus DMA_Get_Stat( LOCATION host, unsigned int eumbbar, unsigned int channel, DMA_SR *stat )
{
    unsigned int tmp;

   if ( channel != 0 && channel != 1 || stat == 0 )
   {
       return DMAINVALID;
   }

    tmp = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_SR_REG] );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) stat = 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_SR_REG], tmp );
#endif

	 stat->reserved0 = ( tmp & 0xffffff00 ) >> 8;
	 stat->lme       = ( tmp & 0x00000080 ) >> 7;
	 stat->reserved1 = ( tmp & 0x00000060 ) >> 5;
	 stat->pe        = ( tmp & 0x00000010 ) >> 4;
	 stat->reserved2 = ( tmp & 0x00000008 ) >> 3;
	 stat->cb        = ( tmp & 0x00000004 ) >> 2;
	 stat->eosi      = ( tmp & 0x00000002 ) >> 1;
	 stat->eocai     = ( tmp & 0x00000001 );

   return DMASUCCESS;
}

/**************************************************************
 * function: DMA_Get_Mode
 *
 * description: return the content of mode register of the
 *              given DMA channel
 *
 *              if error, return DMAINVALID, otherwise return
 *              DMASUCCESS
 **************************************************************/
static
DMAStatus DMA_Get_Mode( LOCATION host, unsigned eumbbar, unsigned int channel, DMA_MR *mode )
{
    unsigned int tmp;
   if ( channel != 0 && channel != 1 || mode == 0 )
   {
     return DMAINVALID;
   }

    tmp = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_MR_REG] );

#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) mode = 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_MR_REG], tmp );
#endif

	 mode->reserved0 = (tmp & 0xfff00000) >> 20;
	 mode->irqs      = (tmp & 0x00080000) >> 19;
	 mode->pde       = (tmp & 0x00040000) >> 18;
	 mode->dahts     = (tmp & 0x00030000) >> 16;
     mode->sahts     = (tmp & 0x0000c000) >> 14;
	 mode->dahe      = (tmp & 0x00002000) >> 13;
	 mode->sahe      = (tmp & 0x00001000) >> 12;
	 mode->prc       = (tmp & 0x00000c00) >> 10;
	 mode->reserved1 = (tmp & 0x00000200) >> 9;
	 mode->eie       = (tmp & 0x00000100) >> 8;
	 mode->eotie     = (tmp & 0x00000080) >> 7;
	 mode->reserved2 = (tmp & 0x00000070) >> 4;
	 mode->dl        = (tmp & 0x00000008) >> 3;
	 mode->ctm       = (tmp & 0x00000004) >> 2;
	 mode->cc        = (tmp & 0x00000002) >> 1;
	 mode->cs        = (tmp & 0x00000001);

   return DMASUCCESS;
}

/**************************************************************
 * function: DMA_Set_Mode
 *
 * description: Set a new mode to a given DMA channel
 *
 * note: It is not a good idea of changing the DMA mode during
 *       the middle of a transaction.
 **************************************************************/
static
DMAStatus DMA_Set_Mode( LOCATION host, unsigned eumbbar, unsigned int channel, DMA_MR mode )
{
    unsigned int tmp;
   if ( channel != 0 && channel != 1 )
   {
	   return DMAINVALID;
   }

   tmp = ( mode.reserved0 & 0xfff ) << 20;
   tmp |= ( ( mode.irqs  & 0x1 ) << 19);
   tmp |= ( ( mode.pde   & 0x1 ) << 18 );
   tmp |= ( ( mode.dahts & 0x3 ) << 16 );
   tmp |= ( ( mode.sahts & 0x3 ) << 14 );
   tmp |= ( ( mode.dahe  & 0x1 ) << 13 );
   tmp |= ( ( mode.sahe  & 0x1 ) << 12 );
   tmp |= ( ( mode.prc   & 0x3 ) << 10 );
   tmp |= ( ( mode.reserved1 & 0x1 ) << 9 );
   tmp |= ( ( mode.eie   & 0x1 ) << 8 );
   tmp |= ( ( mode.eotie & 0x1 ) << 7 );
   tmp |= ( ( mode.reserved2 & 0x7 ) << 4 );
   tmp |= ( ( mode.dl    & 0x1 ) << 3 );
   tmp |= ( ( mode.ctm   & 0x1 ) << 2 );
   tmp |= ( ( mode.cc    & 0x1 ) << 1 ) ;
   tmp |= ( mode.cs    & 0x1 );

   store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG + DMA_MR_REG], tmp );
   return DMASUCCESS;
}

/************************************************************
 * function: DMA_Start
 *
 * description: start a given DMA channel transaction
 *              return DMASUCCESS if success otherwise return
 *              DMAStatus value
 *
 * note: this function will clear DMA_MR(CC) first, then
 *       set DMA_MR(CC).
 ***********************************************************/
static
DMAStatus DMA_Start( LOCATION host, unsigned int eumbbar, unsigned int channel )
{
   DMA_SR stat;
   unsigned int mode;

   if ( channel != 0 && channel != 1 )
   {
	   return DMAINVALID;
   }

   if ( DMA_Get_Stat( host, eumbbar, channel, &stat ) != DMASUCCESS )
   {
		   return DMAINVALID;
   }

   if ( stat.cb == 1 )
   {
	   /* DMA is not free */
	   return DMACHNBUSY;
   }

   mode = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG + DMA_MR_REG] );
   /* clear DMA_MR(CS) */
   mode &= 0xfffffffe;
   store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG + DMA_MR_REG], mode );

   /* set DMA_MR(CS) */
   mode |= CS;
   store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG + DMA_MR_REG], mode );
   return DMASUCCESS;
}

/***********************************************************
 * function: DMA_Halt
 *
 * description: halt the current dma transaction on the specified
 *              channel.
 *              return DMASUCCESS if success otherwise return DMAINVALID
 *
 * note: if the specified DMA channel is idle, nothing happens
 *************************************************************/
static
DMAStatus DMA_Halt( LOCATION host, unsigned int eumbbar, unsigned int channel )
{
   unsigned int mode;
   if ( channel != 0 && channel != 1 )
   {
	   return DMAINVALID;
   }

   mode = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG + DMA_MR_REG]);

   /* clear DMA_MR(CS) */
   mode &= 0xfffffffe;
   store_runtime_reg(eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG + DMA_MR_REG], mode );
   return DMASUCCESS;
}

/*************************************************************
 * function: DMA_Chn_Cnt
 *
 * description: set the DMA_MR(CC) bit for a given channel
 *              that is in chaining mode.
 *              return DMASUCCESS if successfule, otherwise return
 *              DMAINVALID.
 *
 * note: if the given channel is not in chaining mode, nothing
 *       happen.
 *
 *************************************************************/
static
DMAStatus DMA_Chn_Cnt( LOCATION host, unsigned int eumbbar, unsigned int channel )
{
	DMA_MR mode;
	if ( channel != 0 && channel != 1 )
	{
		return DMAINVALID;
	}

	if ( DMA_Get_Mode( host, eumbbar, channel, &mode ) != DMASUCCESS )
	{
			return DMAINVALID;
	}

	if ( mode.ctm == 0 )
	{
		/* either illegal mode or not chaining mode */
		return DMAINVALID;
	}

	mode.cc = 1;
	return DMA_Set_Mode( host, eumbbar, channel, mode );
}

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
static
DMAStatus DMA_Bld_Desp( LOCATION host,
						   unsigned int eumbbar,
						   unsigned int channel,
						   DMA_CDAR     desp )
{
	DMA_SR status;
	unsigned int temp;

	if ( channel != 0 && channel != 1 )
	{
		/* channel number out of range */
		return DMAINVALID;
	}

	if ( DMA_Get_Stat( host, eumbbar, channel, &status ) != DMASUCCESS )
	{
			return DMAINVALID;
	}

	if ( status.cb == 1 )
	{
		/* channel busy */
		return DMACHNBUSY;
	}

	temp = ( desp.cda & 0x7ffffff ) << 5;
	temp |= (( desp.snen & 0x1 ) << 4 );
	temp |= (( desp.eosie & 0x1 ) << 3 );
	temp |= (( desp.ctt   & 0x3 ) << 1 );
    temp |= ( desp.eotd  & 0x1 );

    store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], temp );

#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) cdar := 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], temp );
#endif

	return DMASUCCESS;
}

/**************************************************************
 * function: DMA_Poke_Desp
 *
 * description: poke the current descriptor address register
 *              for a given channel
 *
 *              return DMASUCCESS if no error
 *
 * note: Due to the undeterministic parallellism of DMA operation,
 *       the value returned by this function shall be taken as
 *       the most recently used descriptor when the last time
 *       DMA starts a chaining mode operation.
 **************************************************************/
static
DMAStatus DMA_Poke_Desp( LOCATION host,
						    unsigned int eumbbar,
						    unsigned int channel,
						    DMA_CDAR     *desp )
{
	unsigned int cdar;
	if ( channel != 0 && channel != 1 || desp == 0 )
	{
			return DMAINVALID;
	}

    cdar = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG] );

#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) cdar : 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], cdar );
#endif


	desp->cda   = ( cdar & 0xffffffe0 ) >> 5;
	desp->snen  = ( cdar & 0x00000010 ) >> 4;
	desp->eosie = ( cdar & 0x00000008 ) >> 3;
	desp->ctt   = ( cdar & 0x00000006 ) >> 1;
	desp->eotd  = ( cdar & 0x00000001 );

	return DMASUCCESS;
}

/**************************************************************
 * function: DMA_Bld_Curr
 *
 * description: set current src, dest, byte count registers
 *              according to the desp for a given channel
 *              return DMASUCCESS if no error.
 *
 * note:
 **************************************************************/
static
DMAStatus DMA_Bld_Curr( LOCATION host,
					   unsigned int eumbbar,
					   unsigned int channel,
					   DMA_CURR     desp )
{
	DMA_SR status;
	if ( channel != 0 && channel != 1 )
	{
		/* channel number out of range */
		return DMAINVALID;
	}

	if ( DMA_Get_Stat( host, eumbbar, channel, &status ) != DMASUCCESS )
	{
		 return DMAINVALID;
	}

	if ( status.cb == 1  )
	{
		/* channel busy */
		return DMACHNBUSY;
	}

	desp.byte_cnt &= 0x03ffffff; /* upper 6-bits are 0s */

    store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_SAR_REG], desp.src_addr );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) src := 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], desp.src_addr );
#endif

    store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_DAR_REG], desp.dest_addr );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) dest := 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], desp.dest_addr );
#endif

    store_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_BCR_REG], desp.byte_cnt );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) count := 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], desp.byte_cnt );
#endif


	return DMASUCCESS;

}

/**************************************************************
 * function: DMA_Poke_Curr
 *
 * description: poke the current src, dest, byte count registers
 *              for a given channel.
 *
 *              return DMASUCCESS if no error
 *
 * note:        Due to the undeterministic parallelism, in chaining
 *              mode, the value returned by this function shall
 *              be taken as reference when the query is made rather
 *              than the absolute snapshot when the value is returned.
 **************************************************************/
static
DMAStatus DMA_Poke_Curr( LOCATION host,
					    unsigned int eumbbar,
					    unsigned int channel,
					    DMA_CURR*    desp )
{
	if ( channel != 0 && channel != 1 || desp == 0 )
	{
			return DMAINVALID;
	}

	desp->src_addr = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_SAR_REG] );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) src : 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], desp->src_addr );
#endif

	desp->dest_addr = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_DAR_REG] );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) dest : 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], desp->dest_addr );
#endif

    desp->byte_cnt = load_runtime_reg( eumbbar, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_BCR_REG] );
#ifdef DMADBG0
   PRINT( "%s(%d): %s DMA %d (0x%08x) count : 0x%08x\n", __FILE__, __LINE__,
		  ( host == LOCAL ? "local" : "remote" ), channel, dma_reg_tb[host][channel*NUM_DMA_REG+DMA_CDAR_REG], desp->byte_cnt );
#endif


	return DMASUCCESS;
}

/*****************************************************************
 * function: dma_error_func
 *
 * description: display the error information
 *
 * note: This seems like a highly convoluted way to handle messages,
 * but I'll leave it as it was in device.c when I moved it into the
 * DMA library source.
 ****************************************************************/
static
DMAStatus dma_error_func( unsigned int eumbbar, unsigned int chn, DMAStatus err)
{
	unsigned char *msg[] =
		{
			"Local Memory Error",
			"PCI Error",
			"Channel Busy",
			"End-of-Segment Interrupt",
			"End-of-Chain/Direct Interrupt",
		};

	   if ( err >= DMALMERROR && err <= DMAEOCAINT )
	   {
	     PRINT( "DMA Status: channel %d  %s\n", chn, msg[err-DMASUCCESS-1] );
	   }

	   return err;

}

/*************************************************************
 * function: DMA_ISR
 *
 * description: DMA interrupt service routine
 *              return DMAStatus value based on
 *              the status
 *
 *************************************************************/
static
DMAStatus DMA_ISR( unsigned int eumbbar,
				  unsigned int channel,
				  DMAStatus (*lme_func)( unsigned int, unsigned int, DMAStatus ),
				  DMAStatus (*pe_func) ( unsigned int, unsigned int, DMAStatus ),
				  DMAStatus (*eosi_func)( unsigned int, unsigned int, DMAStatus ),
				  DMAStatus (*eocai_func)(unsigned int, unsigned int, DMAStatus ))
{

	DMA_SR stat;
	DMAStatus rval = DMANOEVENT;
    unsigned int temp;

	if ( channel != 0 && channel != 1 )
	{
		return DMAINVALID;
	}

	if ( DMA_Get_Stat( LOCAL, eumbbar, channel, &stat ) != DMASUCCESS )
	{
			return DMAINVALID;
	}

	if ( stat.lme == 1 )
	{
		/* local memory error */
		rval = DMALMERROR;
		if ( lme_func != 0 )
		{
		  rval = (*lme_func)(eumbbar, channel, DMALMERROR );
	    }

	}
	else if ( stat.pe == 1 )
	{
	/* PCI error */
		rval = DMAPERROR;
		if ( pe_func != 0 )
		{
		  rval = (*pe_func)(eumbbar, channel, DMAPERROR );
	    }

	}
	else if ( stat.eosi == 1 )
	{
		/* end-of-segment interrupt */
		rval = DMAEOSINT;
		if ( eosi_func != 0 )
		{
		  rval = (*eosi_func)(eumbbar, channel, DMAEOSINT );
	    }
	}
	else
	{
		/* End-of-chain/direct interrupt */
		rval = DMAEOCAINT;
		if ( eocai_func != 0 )
		{
		  rval = (*eocai_func)(eumbbar, channel, DMAEOCAINT );
	    }
	}

    temp = ( stat.reserved0 & 0xffffff ) << 8;
	temp |= ( ( stat.lme       & 0x1 ) << 7 );  /* write one to clear */
	temp |= ( ( stat.reserved1 & 0x3 ) << 5 );
    temp |= ( ( stat.pe        & 0x1 ) << 4 );  /* write one to clear */
    temp |= ( ( stat.reserved2 & 0x1 ) << 3 );
	temp |= ( ( stat.cb        & 0x1 ) << 2 );  /* write one to clear */
    temp |= ( ( stat.eosi      & 0x1 ) << 1 );  /* write one to clear */
    temp |= ( stat.eocai & 0x1 );               /* write one to clear */

    store_runtime_reg( eumbbar, dma_reg_tb[LOCAL][channel*NUM_DMA_REG + DMA_SR_REG], temp );

#ifdef DMADBG0
	PRINT( "%s(%d): DMA channel %d SR := 0x%08x\n", __FILE__, __LINE__, channel, temp );
#endif

	return rval;
}
