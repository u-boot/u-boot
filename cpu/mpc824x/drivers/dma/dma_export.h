#ifndef DMA_EXPORT_H
#define DMA_EXPORT_H

/****************************************************
 * $Id:
 *
 * Copyright Motorola 1999
 *
 * $Log:
 *
 ****************************************************/

/* These are the defined return values for the DMA_* functions.
 * Any non-zero value indicates failure.  Failure modes can be added for
 * more detailed error reporting.
 */
typedef enum _dma_status
{
 DMA_SUCCESS     = 0,
 DMA_ERROR,
} DMA_Status;

/* These are the defined channel transfer types.  */
typedef enum _dma_transfer_types
{
	DMA_M2M =  0,	/* local memory to local memory */
	DMA_M2P =  1,	/* local memory to PCI */
	DMA_P2M =  2,	/* PCI to local memory */
	DMA_P2P =  3,	/* PCI to PCI */
} DMA_TRANSFER_TYPE;

typedef enum _dma_interrupt_steer
{
	DMA_INT_STEER_LOCAL =  0, /* steer DMA int to local processor */
	DMA_INT_STEER_PCI = 1,    /* steer DMA int to PCI bus through INTA_ */
} DMA_INTERRUPT_STEER;

typedef enum _dma_channel
{
	DMA_CHN_0 =  0, /* kahlua has two dma channels: 0 and 1 */
	DMA_CHN_1 =  1,
} DMA_CHANNEL;

typedef enum _dma_snoop_mode
{
	DMA_SNOOP_DISABLE =  0,
	DMA_SNOOP_ENABLE = 1,
} DMA_SNOOP_MODE;

/******************** App. API ********************
 * The application API is for user level application
 * to use the functionality provided by DMA driver.
 * This is a "generic" DMA interface, it should contain
 * nothing specific to the Kahlua implementation.
 * Only the generic functions are exported by the library.
 *
 * Note: Its App.s responsibility to swap the data
 *       byte. In our API, we currently transfer whatever
 *       we are given - Big/Little Endian.  This could
 *       become part of the DMA config, though.
 **************************************************/


/*  Initialize DMA unit with the following:
 *  optional pointer to application layer print function
 *
 *  These parameters may be added:
 *  ???
 *  Interrupt enables, modes, etc. are set for each transfer.
 *
 *  This function must be called before DMA unit can be used.
 */
extern DMA_Status DMA_Initialize(
	int (*app_print_function)(char *,...)); /* pointer to optional "printf"
						 * provided by application
						 */

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
				       DMA_SNOOP_MODE snoop);
#endif
