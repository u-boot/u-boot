#ifndef I2C_H
#define I2C_H

/****************************************************
 *
 * Copyright Motrola 1999
 *
 ****************************************************/
#define get_eumbbar() CFG_EUMB_ADDR

#define I2CADR    0x00003000
#define I2CFDR    0x00003004
#define I2CCR     0x00003008
#define I2CSR     0x0000300C
#define I2CDR     0x00003010

typedef enum _i2cstatus
{
 I2CSUCCESS     = 0x3000,
 I2CADDRESS,
 I2CERROR,
 I2CBUFFFULL,
 I2CBUFFEMPTY,
 I2CXMITERROR,
 I2CRCVERROR,
 I2CBUSBUSY,
 I2CALOSS,
 I2CNOEVENT,
} I2CStatus;

typedef enum i2c_control
{
 MEN  = 0x00000080,
 MIEN = 0x00000040,
 MSTA = 0x00000020,
 MTX  = 0x00000010,
 TXAK = 0x00000008,
 RSTA = 0x00000004,
} I2C_CONTROL;

typedef enum i2c_status
{
  MCF   =  0x00000080,
  MAAS  =  0x00000040,
  MBB   =  0x00000020,
  MAL   =  0x00000010,
  SRW   =  0x00000004,
  MIF   =  0x00000002,
  RXAK  =  0x00000001,
} I2C_STATUS;

typedef struct _i2c_ctrl
{
	unsigned int reserved0 : 24;
	unsigned int men       : 1;
	unsigned int mien      : 1;
	unsigned int msta      : 1;
	unsigned int mtx       : 1;
	unsigned int txak      : 1;
	unsigned int rsta      : 1;
	unsigned int reserved1 : 2;
} I2C_CTRL;

typedef struct _i2c_stat
{
	unsigned int rsrv0    : 24;
	unsigned int mcf      : 1;
	unsigned int maas     : 1;
	unsigned int mbb      : 1;
	unsigned int mal      : 1;
	unsigned int rsrv1    : 1;
	unsigned int srw      : 1;
	unsigned int mif      : 1;
	unsigned int rxak     : 1;
} I2C_STAT;

typedef enum _i2c_mode
{
	RCV =  0,
	XMIT = 1,
} I2C_MODE;

/******************** App. API ********************
 * The application API is for user level application
 * to use the funcitonality provided by I2C driver
 *
 * Note: Its App.s responsibility to swap the data
 *       byte. In our API, we just transfer whatever
 *       we are given
 **************************************************/
/**
 * Note:
 *
 * In all following functions,
 * the caller shall pass the configured embedded utility memory
 * block base, EUMBBAR.
 **/

/* Send a buffer of data to the intended rcv_addr.
 * If stop_flag is set, after the whole buffer
 * is sent, generate a STOP signal provided that the
 * receiver doesn't signal the STOP in the middle.
 * I2C is the master performing transmitting. If
 * no STOP signal is generated at the end of current
 * transaction, the master can generate a START signal
 * to another slave addr.
 *
 * return I2CSUCCESS if no error.
 */
static I2CStatus I2C_put( unsigned int  eumbbar,
						  unsigned char rcv_addr,    /* receiver's address */
	                      unsigned char *buffer_ptr, /* pointer of data to be sent */
					      unsigned int  length,      /* number of byte of in the buffer */
					      unsigned int  stop_flag,   /* 1 - signal STOP when buffer is empty
					                                  * 0 - no STOP signal when buffer is empty
											          */
						  unsigned int  is_cnt );    /* 1 - this is a restart, don't check MBB
                                                      * 0 - this is a new start, check MBB
													  */

/* Receive a buffer of data from the desired sender_addr
 * If stop_flag is set, when the buffer is full and the
 * sender does not signal STOP, generate a STOP signal.
 * I2C is the master performing receiving. If no STOP signal
 * is generated, the master can generate a START signal
 * to another slave addr.
 *
 * return I2CSUCCESS if no error.
 */
static I2CStatus I2C_get( unsigned int  eumbbar,
						  unsigned char sender_addr, /* sender's address */
					      unsigned char *buffer_ptr, /* pointer of receiving buffer */
				          unsigned int  length,      /* length of the receiving buffer */
					      unsigned int  stop_flag,   /* 1 - signal STOP when buffer is full
					                                  * 0 - no STOP signal when buffer is full
												      */
						  unsigned int  is_cnt );    /* 1 - this is a restart, don't check MBB
                                                      * 0 - this is a new start, check MBB
													  */

#if 0 /* the I2C_write and I2C_read functions are not active */
/* Send a buffer of data to the requiring master.
 * If stop_flag is set, after the whole buffer is sent,
 * generate a STOP signal provided that the requiring
 * receiver doesn't signal the STOP in the middle.
 * I2C is the slave performing transmitting.
 *
 * return I2CSUCCESS if no error.
 *
 * Note: due to the Kahlua design, slave transmitter
 *       shall not signal STOP since there is no way
 *       for master to detect it, causing I2C bus hung.
 *
 *       For the above reason, the stop_flag is always
 *       set, i.e., 1.
 *
 *       programmer shall use the timer on Kahlua to
 *       control the interval of data byte at the
 *       master side.
 */
static I2CStatus I2C_write( unsigned int eumbbar,
						    unsigned char *buffer_ptr, /* pointer of data to be sent */
					        unsigned int  length,      /* number of byte of in the buffer */
					        unsigned int  stop_flag ); /* 1 - signal STOP when buffer is empty
								                        * 0 - no STOP signal when buffer is empty
											            */

 /* Receive a buffer of data from the sending master.
 * If stop_flag is set, when the buffer is full and the
 * sender does not signal STOP, generate a STOP signal.
 * I2C is the slave performing receiving.
 *
 * return I2CSUCCESS if no error.
 */
static I2CStatus I2C_read(unsigned int  eumbbar,
						  unsigned char *buffer_ptr, /* pointer of receiving buffer */
					      unsigned int  length,      /* length of the receiving buffer */
				          unsigned int  stop_flag ); /* 1 - signal STOP when buffer is full
					                                  * 0 - no STOP signal when buffer is full
												      */
#endif /* of if0 for turning off I2C_read & I2C_write */

/* if interrupt is not used, this is the timer event handler.
 * After each fixed time interval, this function can be called
 * to check the I2C status and call appropriate function to
 * handle the status event.
 */
static I2CStatus I2C_Timer_Event( unsigned int eumbbar, I2CStatus (*handler)( unsigned int ) );

/********************* Kernel API ************************
 * Kernel APIs are functions I2C driver provides to the
 * O.S.
 *********************************************************/

/******************* device I/O function ***************/

/*  Generate a START signal in the desired mode.
 *  I2C is the master.
 *
 * return I2CSUCCESS if no error.
 *        I2CERROR   if i2c unit is not enabled.
 *        I2CBUSBUSY if bus cannot be granted
 */
static I2CStatus I2C_Start( unsigned int  eumbbar,
						    unsigned char slave_addr, /* address of the receiver */
	                        I2C_MODE     mode,       /* XMIT(1) - put (write)
							                          * RCV(0)  - get (read)
													  */
						    unsigned int is_cnt ); /* 1 - this is a restart, don't check MBB
													* 0 - this is a new start, check MBB
                                                    */

/* Generate a STOP signal to terminate the transaction. */
static I2CStatus I2C_Stop( unsigned int eumbbar );

/*  Do a one-byte master transmit.
 *
 *  return I2CBUFFEMPTY if this is the last byte.
 *  Otherwise return I2CSUCCESS
 */
static I2CStatus I2C_Master_Xmit( unsigned int eumbbar );

/*  Do a one-byte master receive.
 *
 *  return I2CBUFFFULL if this is the last byte.
 *  Otherwise return I2CSUCCESS
 */
static I2CStatus I2C_Master_Rcv( unsigned int eumbbar );

/*  Do a one-byte slave transmit.
 *
 *  return I2CBUFFEMPTY if this is the last byte.
 *  Otherwise return I2CSUCCESS
 *
 */
static I2CStatus I2C_Slave_Xmit( unsigned int eumbbar );

/* Do a one-byte slave receive.
 *
 *  return I2CBUFFFULL if this is the last byte.
 *  Otherwise return I2CSUCCESS
 */
static I2CStatus I2C_Slave_Rcv( unsigned int eumbbar  );

/* Process slave address phase.
 *
 * return I2CADDRESS if this is slave receiver's address phase
 * Otherwise return the result of slave xmit one byte.
 */
static I2CStatus I2C_Slave_Addr( unsigned int eumbbar );

/******************* Device Control Fucntion ****************/
/*  Initialize I2C unit with desired frequency divider,
 *  driver's slave address w/o interrupt enabled.
 *
 *  This function must be called before I2C unit can
 *  be used.
 */
static I2CStatus I2C_Init( unsigned int  eumbbar,
						   unsigned char fdr,       /* frequency divider */
	                       unsigned char addr,      /* driver's address used for receiving */
	 			           unsigned int en_int);    /* 1 - enable I2C interrupt
					                                 * 0 - disable I2C interrup
											         */

/* I2C interrupt service routine.
 *
 * return I2CADDRESS if it is receiver's (either master or slave) address phase.
 * return the result of xmit or receive one byte
 */
static I2CStatus I2C_ISR(unsigned int eumbbar  );

/* Set I2C Status, i.e., write to I2CSR */
static void I2C_Set_Stat( unsigned int eumbbar, I2C_STAT stat );

/* Query I2C Status, i.e., read I2CSR */
static I2C_STAT I2C_Get_Stat( unsigned int eumbbar );

/* Change I2C Control bits, i.e., write to I2CCR */
static void I2C_Set_Ctrl( unsigned int eumbbar, I2C_CTRL ); /* new control value */

/* Query I2C Control bits, i.e., read I2CCR */
static I2C_CTRL I2C_Get_Ctrl( unsigned int eumbbar );

/* This function performs the work for I2C_do_transaction.  The work is
 * split into this function to enable I2C_do_transaction to first transmit
 * the data address to the I2C slave device without putting the data address
 * into the first byte of the buffer.
 *
 * en_int controls interrupt/polling mode
 * act is the type of transaction
 * i2c_addr is the I2C address of the slave device
 * len is the length of data to send or receive
 * buffer is the address of the data buffer
 * stop = I2C_NO_STOP, don't signal STOP at end of transaction
 *        I2C_STOP, signal STOP at end of transaction
 * retry is the timeout retry value, currently ignored
 * rsta = I2C_NO_RESTART, this is not continuation of existing transaction
 *        I2C_RESTART, this is a continuation of existing transaction
 */
static I2C_Status I2C_do_buffer( I2C_INTERRUPT_MODE en_int,
                                 I2C_TRANSACTION_MODE act,
                                 unsigned char i2c_addr,
                                 int len,
                                 unsigned char *buffer,
                                 I2C_STOP_MODE stop,
                                 int retry,
                                 I2C_RESTART_MODE rsta);
#endif
