/*************************************************************
 *
 * Copyright @ Motorola, 1999
 *
 ************************************************************/
#include <common.h>

#ifdef CONFIG_HARD_I2C
#include <i2c.h>
#include "i2c_export.h"
#include "i2c.h"

#undef  I2CDBG0
#undef  DEBUG

/* Define a macro to use an optional application-layer print function, if
 * one was passed to the I2C library during initialization.  If there was
 * no function pointer passed, this protects against calling it.  Also define
 * the global variable that holds the passed pointer.
 */
#define TIMEOUT (CFG_HZ/4)
#define PRINT if ( app_print ) app_print
static int (*app_print) (char *, ...);

/******************* Internal to I2C Driver *****************/
static unsigned int ByteToXmit = 0;
static unsigned int XmitByte = 0;
static unsigned char *XmitBuf = 0;
static unsigned int XmitBufEmptyStop = 0;
static unsigned int ByteToRcv = 0;
static unsigned int RcvByte = 0;
static unsigned char *RcvBuf = 0;
static unsigned int RcvBufFulStop = 0;
static unsigned int MasterRcvAddress = 0;

/* Set by call to get_eumbbar during I2C_Initialize.
 * This could be globally available to the I2C library, but there is
 * an advantage to passing it as a parameter: it is already in a register
 * and doesn't have to be loaded from memory.  Also, that is the way the
 * I2C library was already implemented and I don't want to change it without
 * a more detailed analysis.
 * It is being set as a global variable in I2C_Initialize to hide it from
 * the DINK application layer, because it is Kahlua-specific.  I think that
 * get_eumbbar, load_runtime_reg, and store_runtime_reg should be defined in
 * a Kahlua-specific library dealing with the embedded utilities memory block.
 * Right now, get_eumbbar is defined in dink32/kahlua.s.  The other two are
 * defined in dink32/drivers/i2c/i2c2.s.
 */
static unsigned int Global_eumbbar = 0;

extern unsigned int load_runtime_reg (unsigned int eumbbar,
				      unsigned int reg);

extern unsigned int store_runtime_reg (unsigned int eumbbar,
				       unsigned int reg, unsigned int val);

/************************** API *****************/

/* Application Program Interface (API) are the calls provided by the I2C
 * library to upper layer applications (i.e., DINK) to access the Kahlua
 * I2C bus interface.  The functions and values that are part of this API
 * are declared in i2c_export.h.
 */

/*  Initialize I2C unit with the following:
 *  driver's slave address
 *  interrupt enabled
 *  optional pointer to application layer print function
 *
 *  These parameters may be added:
 *  desired clock rate
 *  digital filter frequency sampling rate
 *
 *  This function must be called before I2C unit can be used.
 */
I2C_Status I2C_Initialize (unsigned char addr,
			   I2C_INTERRUPT_MODE en_int,
			   int (*p) (char *, ...))
{
	I2CStatus status;

	/* establish the pointer, if there is one, to the application's "printf" */
	app_print = p;

	/* If this is the first call, get the embedded utilities memory block
	 * base address.  I'm not sure what to do about error handling here:
	 * if a non-zero value is returned, accept it.
	 */
	if (Global_eumbbar == 0)
		Global_eumbbar = get_eumbbar ();
	if (Global_eumbbar == 0) {
		PRINT ("I2C_Initialize: can't find EUMBBAR\n");
		return I2C_ERROR;
	}

	/* validate the I2C address */
	if (addr & 0x80) {
		PRINT ("I2C_Initialize, I2C address invalid:  %d 0x%x\n",
			   (unsigned int) addr, (unsigned int) addr);
		return I2C_ERROR;
	}

	/* Call the internal I2C library function to perform work.
	 * Accept the default frequency sampling rate (no way to set it currently,
	 * via I2C_Init) and set the clock frequency to something reasonable.
	 */
	status = I2C_Init (Global_eumbbar, (unsigned char) 0x31, addr, en_int);
	if (status != I2CSUCCESS) {
		PRINT ("I2C_Initialize: error in initiation\n");
		return I2C_ERROR;
	}

	/* all is well */
	return I2C_SUCCESS;
}


/* Perform the given I2C transaction, only MASTER_XMIT and MASTER_RCV
 * are implemented.  Both are only in polling mode.
 *
 * en_int controls interrupt/polling mode
 * act is the type of transaction
 * i2c_addr is the I2C address of the slave device
 * data_addr is the address of the data on the slave device
 * len is the length of data to send or receive
 * buffer is the address of the data buffer
 * stop = I2C_NO_STOP, don't signal STOP at end of transaction
 *        I2C_STOP, signal STOP at end of transaction
 * retry is the timeout retry value, currently ignored
 * rsta = I2C_NO_RESTART, this is not continuation of existing transaction
 *        I2C_RESTART, this is a continuation of existing transaction
 */
I2C_Status I2C_do_transaction ( I2C_INTERRUPT_MODE en_int,
				I2C_TRANSACTION_MODE act,
				unsigned char i2c_addr,
				unsigned char data_addr,
				int len,
				char *buffer,
				I2C_STOP_MODE stop,
				int retry, I2C_RESTART_MODE rsta)
{
	I2C_Status status;
	unsigned char data_addr_buffer[1];

#if 1
/* This is a temporary work-around.  The I2C library breaks the protocol
 * if it attempts to handle a data transmission in more than one
 * transaction, so the data address and the actual data bytes are put
 * into a single buffer before sending it to the library internal functions.
 * The problem is related to being able to restart a transaction without
 * sending the I2C device address or repeating the data address.  It may take
 * a day or two to sort it all out, so I'll have to get back to it later.
 * Look at I2C_Start to see about using some status flags (I'm not sure that
 * "stop" and "rsta" are enough to reflect the states, maybe so; but the logic
 * in the library is insufficient) to control correct handling of the protocol.
 */
	unsigned char dummy_buffer[257];

	if (act == I2C_MASTER_XMIT) {
		int i;

		if (len > 256)
			return I2C_ERROR;
		for (i = 1; i <= len; i++)
			dummy_buffer[i] = buffer[i - 1];
		dummy_buffer[0] = data_addr;
		status = I2C_do_buffer (en_int, act, i2c_addr, 1 + len,
					dummy_buffer, stop, retry, rsta);
		if (status != I2C_SUCCESS) {
			PRINT ("I2C_do_transaction: can't perform data transfer\n");
			return I2C_ERROR;
		}
		return I2C_SUCCESS;
	}
#endif	/* end of temp work-around */

	/* validate requested transaction type */
	if ((act != I2C_MASTER_XMIT) && (act != I2C_MASTER_RCV)) {
		PRINT ("I2C_do_transaction, invalid transaction request:  %d\n",
			act);
		return I2C_ERROR;
	}

	/* range check the I2C address */
	if (i2c_addr & 0x80) {
		PRINT ("I2C_do_transaction, I2C address out of range:  %d 0x%x\n",
			(unsigned int) i2c_addr, (unsigned int) i2c_addr);
		return I2C_ERROR;
	} else {
		data_addr_buffer[0] = data_addr;
	}

	/*
         * We first have to contact the slave device and transmit the
         * data address. Be careful about the STOP and restart stuff.
         * We don't want to signal STOP after sending the data
         * address, but this could be a continuation if the
         * application didn't release the bus after the previous
         * transaction, by not sending a STOP after it.
	 */
	status = I2C_do_buffer (en_int, I2C_MASTER_XMIT, i2c_addr, 1,
				data_addr_buffer, I2C_NO_STOP, retry, rsta);
	if (status != I2C_SUCCESS) {
		PRINT ("I2C_do_transaction: can't send data address for read\n");
		return I2C_ERROR;
	}

	/* The data transfer will be a continuation. */
	rsta = I2C_RESTART;

	/* now handle the user data */
	status = I2C_do_buffer (en_int, act, i2c_addr, len,
				buffer, stop, retry, rsta);
	if (status != I2C_SUCCESS) {
		PRINT ("I2C_do_transaction: can't perform data transfer\n");
		return I2C_ERROR;
	}

	/* all is well */
	return I2C_SUCCESS;
}

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
static I2C_Status I2C_do_buffer (I2C_INTERRUPT_MODE en_int,
				 I2C_TRANSACTION_MODE act,
				 unsigned char i2c_addr,
				 int len,
				 unsigned char *buffer,
				 I2C_STOP_MODE stop,
				 int retry, I2C_RESTART_MODE rsta)
{
	I2CStatus rval;
	unsigned int dev_stat;

	if (act == I2C_MASTER_RCV) {
		/* set up for master-receive transaction */
		rval = I2C_get (Global_eumbbar, i2c_addr, buffer, len, stop, rsta);
	} else {
		/* set up for master-transmit transaction */
		rval = I2C_put (Global_eumbbar, i2c_addr, buffer, len, stop, rsta);
	}

	/* validate the setup */
	if (rval != I2CSUCCESS) {
		dev_stat = load_runtime_reg (Global_eumbbar, I2CSR);
		PRINT ("Error(I2C_do_buffer): control phase, code(0x%08x), status(0x%08x)\n", rval, dev_stat);
		I2C_Stop (Global_eumbbar);
		return I2C_ERROR;
	}

	if (en_int == 1) {
		/* this should not happen, no interrupt handling yet */
		return I2C_SUCCESS;
	}

	/* this performs the polling action, when the transfer is completed,
	 * the status returned from I2C_Timer_Event will be I2CBUFFFULL or
	 * I2CBUFFEMPTY (rcv or xmit), I2CSUCCESS or I2CADDRESS indicates the
	 * transaction is not yet complete, anything else is an error.
	 */
	while (rval == I2CSUCCESS || rval == I2CADDRESS) {
		int timeval = get_timer (0);

		/* poll the device until something happens */
		do {
			rval = I2C_Timer_Event (Global_eumbbar, 0);
		}
		while (rval == I2CNOEVENT && get_timer (timeval) < TIMEOUT);

		/* check for error condition */
		if (rval == I2CSUCCESS   ||
		    rval == I2CBUFFFULL  ||
		    rval == I2CBUFFEMPTY ||
		    rval == I2CADDRESS) {
			;	/* do nothing */
		} else {
			/* report the error condition */
			dev_stat = load_runtime_reg (Global_eumbbar, I2CSR);
			PRINT ("Error(I2C_do_buffer):  code(0x%08x), status(0x%08x)\n",
				   rval, dev_stat);
			return I2C_ERROR;
		}
	}

	/* all is well */
	return I2C_SUCCESS;
}

/**
 * Note:
 *
 * In all following functions,
 * the caller shall pass the configured embedded utility memory
 * block base, EUMBBAR.
 **/

/***********************************************************
 * function: I2C_put
 *
 * description:
   Send a buffer of data to the intended rcv_addr.
 * If stop_flag is set, after the whole buffer
 * is sent, generate a STOP signal provided that the
 * receiver doesn't signal the STOP in the middle.
 * I2C is the master performing transmitting. If
 * no STOP signal is generated at the end of current
 * transaction, the master can generate a START signal
 * to another slave addr.
 *
 * note: this is master xmit API
 *********************************************************/
static I2CStatus I2C_put (unsigned int eumbbar, unsigned char rcv_addr,	/* receiver's address */
	  unsigned char *buffer_ptr,	/* pointer of data to be sent */
	  unsigned int length,	/* number of byte of in the buffer */
	  unsigned int stop_flag,	/* 1 - signal STOP when buffer is empty
					 * 0 - no STOP signal when buffer is empty
					 */
	  unsigned int is_cnt)
{					/* 1 - this is a restart, don't check MBB
					 * 0 - this is a new start, check MBB
					 */
	if (buffer_ptr == 0 || length == 0) {
		return I2CERROR;
	}
#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_put\n", __FILE__, __LINE__);
#endif

	XmitByte = 0;
	ByteToXmit = length;
	XmitBuf = buffer_ptr;
	XmitBufEmptyStop = stop_flag;

	RcvByte = 0;
	ByteToRcv = 0;
	RcvBuf = 0;

	/* we are the master, start transaction */
	return I2C_Start (eumbbar, rcv_addr, XMIT, is_cnt);
}

/***********************************************************
 * function: I2C_get
 *
 * description:
 * Receive a buffer of data from the desired sender_addr
 * If stop_flag is set, when the buffer is full and the
 * sender does not signal STOP, generate a STOP signal.
 * I2C is the master performing receiving. If no STOP signal
 * is generated, the master can generate a START signal
 * to another slave addr.
 *
 * note: this is master receive API
 **********************************************************/
static I2CStatus I2C_get (unsigned int eumbbar, unsigned char rcv_from,	/* sender's address */
		  unsigned char *buffer_ptr,	/* pointer of receiving buffer */
		  unsigned int length,	/* length of the receiving buffer */
		  unsigned int stop_flag,	/* 1 - signal STOP when buffer is full
						 * 0 - no STOP signal when buffer is full
						 */
		  unsigned int is_cnt)
{						/* 1 - this is a restart, don't check MBB
						 * 0 - this is a new start, check MBB
						 */
	if (buffer_ptr == 0 || length == 0) {
		return I2CERROR;
	}
#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_get\n", __FILE__, __LINE__);
#endif

	RcvByte = 0;
	ByteToRcv = length;
	RcvBuf = buffer_ptr;
	RcvBufFulStop = stop_flag;

	XmitByte = 0;
	ByteToXmit = 0;
	XmitBuf = 0;

	/* we are the master, start the transaction */
	return I2C_Start (eumbbar, rcv_from, RCV, is_cnt);

}

#if 0	/* turn off dead code */
/*********************************************************
 * function: I2C_write
 *
 * description:
 * Send a buffer of data to the requiring master.
 * If stop_flag is set, after the whole buffer is sent,
 * generate a STOP signal provided that the requiring
 * receiver doesn't signal the STOP in the middle.
 * I2C is the slave performing transmitting.
 *
 * Note: this is slave xmit API.
 *
 *       due to the current Kahlua design, slave transmitter
 *       shall not signal STOP since there is no way
 *       for master to detect it, causing I2C bus hung.
 *
 *       For the above reason, the stop_flag is always
 *       set, i.e., 0.
 *
 *       programmer shall use the timer on Kahlua to
 *       control the interval of data byte at the
 *       master side.
 *******************************************************/
static I2CStatus I2C_write (unsigned int eumbbar, unsigned char *buffer_ptr,	/* pointer of data to be sent */
		unsigned int length,	/* number of byte of in the buffer */
		unsigned int stop_flag)
{					/* 1 - signal STOP when buffer is empty
					 * 0 - no STOP signal when buffer is empty
					 */
	if (buffer_ptr == 0 || length == 0) {
		return I2CERROR;
	}

	XmitByte = 0;
	ByteToXmit = length;
	XmitBuf = buffer_ptr;
	XmitBufEmptyStop = 0;	/* in order to avoid bus hung, ignored the user's stop_flag */

	RcvByte = 0;
	ByteToRcv = 0;
	RcvBuf = 0;

	/* we are the slave, just wait for being called, or pull */
	/* I2C_Timer_Event( eumbbar ); */
}

/******************************************************
 * function: I2C_read
 *
 * description:
 * Receive a buffer of data from the sending master.
 * If stop_flag is set, when the buffer is full and the
 * sender does not signal STOP, generate a STOP signal.
 * I2C is the slave performing receiving.
 *
 * note: this is slave receive API
 ****************************************************/
static I2CStatus I2C_read (unsigned int eumbbar, unsigned char *buffer_ptr,	/* pointer of receiving buffer */
		   unsigned int length,	/* length of the receiving buffer */
		   unsigned int stop_flag)
{					/* 1 - signal STOP when buffer is full
					 * 0 - no STOP signal when buffer is full
					 */
	if (buffer_ptr == 0 || length == 0) {
		return I2CERROR;
	}

	RcvByte = 0;
	ByteToRcv = length;
	RcvBuf = buffer_ptr;
	RcvBufFulStop = stop_flag;

	XmitByte = 0;
	ByteToXmit = 0;
	XmitBuf = 0;

	/* wait for master to call us, or poll */
	/* I2C_Timer_Event( eumbbar ); */
}
#endif	/* turn off dead code */

/*********************************************************
 * function: I2c_Timer_Event
 *
 * description:
 * if interrupt is not used, this is the timer event handler.
 * After each fixed time interval, this function can be called
 * to check the I2C status and call appropriate function to
 * handle the status event.
 ********************************************************/
static I2CStatus I2C_Timer_Event (unsigned int eumbbar,
				  I2CStatus (*handler) (unsigned int))
{
	I2C_STAT stat;

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Timer_Event\n", __FILE__, __LINE__);
#endif

	stat = I2C_Get_Stat (eumbbar);

	if (stat.mif == 1) {
		if (handler == 0) {
			return I2C_ISR (eumbbar);
		} else {
			return (*handler) (eumbbar);
		}
	}

	return I2CNOEVENT;
}


/****************** Device I/O function *****************/

/******************************************************
 * function: I2C_Start
 *
 * description: Generate a START signal in the desired mode.
 *              I2C is the master.
 *
 *              Return I2CSUCCESS if no error.
 *
 * note:
 ****************************************************/
static I2CStatus I2C_Start (unsigned int eumbbar, unsigned char slave_addr,	/* address of the receiver */
			I2C_MODE mode,	/* XMIT(1) - put (write)
					 * RCV(0)  - get (read)
					 */
			unsigned int is_cnt)
{					/* 1 - this is a restart, don't check MBB
					 * 0 - this is a new start
					 */
	unsigned int tmp = 0;
	I2C_STAT stat;
	I2C_CTRL ctrl;

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Start addr 0x%x mode %d cnt %d\n", __FILE__,
		   __LINE__, slave_addr, mode, is_cnt);
#endif

	ctrl = I2C_Get_Ctrl (eumbbar);

	/* first make sure I2C has been initialized */
	if (ctrl.men == 0) {
		return I2CERROR;
	}

	/* next make sure bus is idle */
	stat = I2C_Get_Stat (eumbbar);

	if (is_cnt == 0 && stat.mbb == 1) {
		/* sorry, we lost */
		return I2CBUSBUSY;
	} else if (is_cnt == 1 && stat.mif == 1 && stat.mal == 0) {
		/* sorry, we lost the bus */
		return I2CALOSS;
	}


	/* OK, I2C is enabled and we have the bus */

	/* prepare to write the slave address */
	ctrl.msta = 1;
	ctrl.mtx = 1;
	ctrl.txak = 0;
	ctrl.rsta = is_cnt;		/* set the repeat start bit */
	I2C_Set_Ctrl (eumbbar, ctrl);

	/* write the slave address and xmit/rcv mode bit */
	tmp = load_runtime_reg (eumbbar, I2CDR);
	tmp = (tmp & 0xffffff00) |
	      ((slave_addr & 0x007f) << 1) |
	      (mode == XMIT ? 0x0 : 0x1);
	store_runtime_reg (eumbbar, I2CDR, tmp);

	if (mode == RCV) {
		MasterRcvAddress = 1;
	} else {
		MasterRcvAddress = 0;
	}

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Start exit\n", __FILE__, __LINE__);
#endif

	/* wait for the interrupt or poll  */
	return I2CSUCCESS;
}

/***********************************************************
 * function: I2c_Stop
 *
 * description: Generate a STOP signal to terminate the master
 *              transaction.
 *              return I2CSUCCESS
 *
 **********************************************************/
static I2CStatus I2C_Stop (unsigned int eumbbar)
{
	I2C_CTRL ctrl;

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Stop enter\n", __FILE__, __LINE__);
#endif

	ctrl = I2C_Get_Ctrl (eumbbar);
	ctrl.msta = 0;
	I2C_Set_Ctrl (eumbbar, ctrl);

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Stop exit\n", __FILE__, __LINE__);
#endif

	return I2CSUCCESS;
}

/****************************************************
 * function: I2C_Master_Xmit
 *
 * description: Master sends one byte of data to
 *              slave target
 *
 *              return I2CSUCCESS if the byte transmitted.
 *              Otherwise no-zero
 *
 * Note: condition must meet when this function is called:
 *       I2CSR(MIF) == 1 && I2CSR(MCF)  == 1  && I2CSR(RXAK) == 0
 *       I2CCR(MSTA) == 1  && I2CCR(MTX) == 1
 *
 ***************************************************/
static I2CStatus I2C_Master_Xmit (unsigned int eumbbar)
{
	unsigned int val;

	if (ByteToXmit > 0) {

		if (ByteToXmit == XmitByte) {
			/* all xmitted */
			ByteToXmit = 0;

			if (XmitBufEmptyStop == 1) {
				I2C_Stop (eumbbar);
			}

			return I2CBUFFEMPTY;

		}
#ifdef I2CDBG0
		PRINT ("%s(%d): xmit 0x%02x\n", __FILE__, __LINE__,
			   *(XmitBuf + XmitByte));
#endif

		val = *(XmitBuf + XmitByte);
		val &= 0x000000ff;
		store_runtime_reg (eumbbar, I2CDR, val);
		XmitByte++;

		return I2CSUCCESS;

	}

	return I2CBUFFEMPTY;
}

/***********************************************
 * function: I2C_Master_Rcv
 *
 * description: master reads one byte data
 *              from slave source
 *
 *              return I2CSUCCESS if no error
 *
 * Note: condition must meet when this function is called:
 *       I2CSR(MIF) == 1 && I2CSR(MCF) == 1 &&
 *       I2CCR(MSTA) == 1 && I2CCR(MTX) == 0
 *
 ***********************************************/
static I2CStatus I2C_Master_Rcv (unsigned int eumbbar)
{
	I2C_CTRL ctrl;
	unsigned int val;

	if (ByteToRcv > 0) {

		if (ByteToRcv - RcvByte == 2 && RcvBufFulStop == 1) {
			/* master requests more than or equal to 2 bytes
			 * we are reading 2nd to last byte
			 */

			/* we need to set I2CCR(TXAK) to generate a STOP */
			ctrl = I2C_Get_Ctrl (eumbbar);
			ctrl.txak = 1;
			I2C_Set_Ctrl (eumbbar, ctrl);

			/* Kahlua will automatically generate a STOP
			 * next time a transaction happens
			 */

			/* note: the case of master requesting one byte is
			 *       handled in I2C_ISR
			 */
		}

		/* generat a STOP before reading the last byte */
		if (RcvByte + 1 == ByteToRcv && RcvBufFulStop == 1) {
			I2C_Stop (eumbbar);
		}

		val = load_runtime_reg (eumbbar, I2CDR);
		*(RcvBuf + RcvByte) = val & 0xFF;

#ifdef I2CDBG0
		PRINT ("%s(%d): rcv 0x%02x\n", __FILE__, __LINE__,
			   *(RcvBuf + RcvByte));
#endif

		RcvByte++;

		if (ByteToRcv == RcvByte) {
			ByteToRcv = 0;

			return I2CBUFFFULL;
		}

		return I2CSUCCESS;
	}

	return I2CBUFFFULL;

}

/****************************************************
 * function: I2C_Slave_Xmit
 *
 * description: Slave sends one byte of data to
 *              requesting destination
 *
 *        return SUCCESS if the byte transmitted. Otherwise
 *        No-zero
 *
 * Note: condition must meet when this function is called:
 *       I2CSR(MIF) == 1 && I2CSR(MCF) == 1 &&  I2CSR(RXAK) = 0
 *       I2CCR(MSTA) == 0  && I2CCR(MTX) == 1
 *
 ***************************************************/
static I2CStatus I2C_Slave_Xmit (unsigned int eumbbar)
{
	unsigned int val;

	if (ByteToXmit > 0) {

		if (ByteToXmit == XmitByte) {
			/* no more data to send */
			ByteToXmit = 0;

			/*
                         * do not toggle I2CCR(MTX). Doing so will
                         * cause bus-hung since current Kahlua design
                         * does not give master a way to detect slave
                         * stop. It is always a good idea for master
                         * to use timer to prevent the long long
                         * delays
			 */

			return I2CBUFFEMPTY;
		}
#ifdef I2CDBG
		PRINT ("%s(%d): xmit 0x%02x\n", __FILE__, __LINE__,
			   *(XmitBuf + XmitByte));
#endif

		val = *(XmitBuf + XmitByte);
		val &= 0x000000ff;
		store_runtime_reg (eumbbar, I2CDR, val);
		XmitByte++;

		return I2CSUCCESS;
	}

	return I2CBUFFEMPTY;
}

/***********************************************
 * function: I2C_Slave_Rcv
 *
 * description: slave reads one byte data
 *              from master source
 *
 *              return I2CSUCCESS if no error otherwise non-zero
 *
 * Note: condition must meet when this function is called:
 *       I2CSR(MIF) == 1 && I2CSR(MCF) == 1 &&
 *       I2CCR(MSTA) == 0 && I2CCR(MTX)  = 0
 *
 ***********************************************/
static I2CStatus I2C_Slave_Rcv (unsigned int eumbbar)
{
	unsigned int val;
	I2C_CTRL ctrl;

	if (ByteToRcv > 0) {
		val = load_runtime_reg (eumbbar, I2CDR);
		*(RcvBuf + RcvByte) = val & 0xff;
#ifdef I2CDBG
		PRINT ("%s(%d): rcv 0x%02x\n", __FILE__, __LINE__,
			   *(RcvBuf + RcvByte));
#endif
		RcvByte++;

		if (ByteToRcv == RcvByte) {
			if (RcvBufFulStop == 1) {
				/* all done */
				ctrl = I2C_Get_Ctrl (eumbbar);
				ctrl.txak = 1;
				I2C_Set_Ctrl (eumbbar, ctrl);
			}

			ByteToRcv = 0;
			return I2CBUFFFULL;
		}

		return I2CSUCCESS;
	}

	return I2CBUFFFULL;
}

/****************** Device Control Function *************/

/*********************************************************
 * function: I2C_Init
 *
 * description: Initialize I2C unit with desired frequency divider,
 *              master's listening address, with interrupt enabled
 *              or disabled.
 *
 * note:
 ********************************************************/
static I2CStatus I2C_Init (unsigned int eumbbar, unsigned char fdr,	/* frequency divider */
		   unsigned char slave_addr,	/* driver's address used for receiving */
		   unsigned int en_int)
{						/* 1 - enable I2C interrupt
						 * 0 - disable I2C interrup
						 */
	I2C_CTRL ctrl;
	unsigned int tmp;

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Init enter\n", __FILE__, __LINE__);
#endif

	ctrl = I2C_Get_Ctrl (eumbbar);
	/* disable the I2C module before we change everything */
	ctrl.men = 0;
	I2C_Set_Ctrl (eumbbar, ctrl);

	/* set the frequency diver */
	tmp = load_runtime_reg (eumbbar, I2CFDR);
	tmp = (tmp & 0xffffffc0) | (fdr & 0x3f);
	store_runtime_reg (eumbbar, I2CFDR, tmp);

	/* Set our listening (slave) address */
	tmp = load_runtime_reg (eumbbar, I2CADR);
	tmp = (tmp & 0xffffff01) | ((slave_addr & 0x7f) << 1);
	store_runtime_reg (eumbbar, I2CADR, tmp);

	/* enable I2C with desired interrupt setting */
	ctrl.men = 1;
	ctrl.mien = en_int & 0x1;
	I2C_Set_Ctrl (eumbbar, ctrl);
#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_Init exit\n", __FILE__, __LINE__);
#endif

	return I2CSUCCESS;

}

/*****************************************
 * function I2c_Get_Stat
 *
 * description: Query I2C Status, i.e., read I2CSR
 *
 ****************************************/
static I2C_STAT I2C_Get_Stat (unsigned int eumbbar)
{
	unsigned int temp;
	I2C_STAT stat;

	temp = load_runtime_reg (eumbbar, I2CSR);

#ifdef I2CDBG0
	PRINT ("%s(%d): get stat = 0x%08x\n", __FILE__, __LINE__, temp);
#endif

	stat.rsrv0 = (temp & 0xffffff00) >> 8;
	stat.mcf   = (temp & 0x00000080) >> 7;
	stat.maas  = (temp & 0x00000040) >> 6;
	stat.mbb   = (temp & 0x00000020) >> 5;
	stat.mal   = (temp & 0x00000010) >> 4;
	stat.rsrv1 = (temp & 0x00000008) >> 3;
	stat.srw   = (temp & 0x00000004) >> 2;
	stat.mif   = (temp & 0x00000002) >> 1;
	stat.rxak  = (temp & 0x00000001);
	return stat;
}

/*********************************************
 * function: I2c_Set_Ctrl
 *
 * description: Change I2C Control bits,
 *              i.e., write to I2CCR
 *
 ********************************************/
static void I2C_Set_Ctrl (unsigned int eumbbar, I2C_CTRL ctrl)
{						/* new control value */
	unsigned int temp = load_runtime_reg (eumbbar, I2CCR);

	temp &= 0xffffff03;
	temp |= ((ctrl.men  & 0x1) << 7);
	temp |= ((ctrl.mien & 0x1) << 6);
	temp |= ((ctrl.msta & 0x1) << 5);
	temp |= ((ctrl.mtx  & 0x1) << 4);
	temp |= ((ctrl.txak & 0x1) << 3);
	temp |= ((ctrl.rsta & 0x1) << 2);
#ifdef I2CDBG0
	PRINT ("%s(%d): set ctrl = 0x%08x\n", __FILE__, __LINE__, temp);
#endif
	store_runtime_reg (eumbbar, I2CCR, temp);

}

/*****************************************
 * function: I2C_Get_Ctrl
 *
 * description: Query I2C Control bits,
 *              i.e., read I2CCR
 *****************************************/
static I2C_CTRL I2C_Get_Ctrl (unsigned int eumbbar)
{
	union {
		I2C_CTRL ctrl;
		unsigned int temp;
	} s;

	s.temp = load_runtime_reg (eumbbar, I2CCR);
#ifdef I2CDBG0
	PRINT ("%s(%d): get ctrl = 0x%08x\n", __FILE__, __LINE__, s.temp);
#endif

	return s.ctrl;
}


/****************************************
 * function: I2C_Slave_Addr
 *
 * description: Process slave address phase.
 *              return I2CSUCCESS if no error
 *
 * note: Precondition for calling this function:
 *       I2CSR(MIF) == 1 &&
 *       I2CSR(MAAS) == 1
 ****************************************/
static I2CStatus I2C_Slave_Addr (unsigned int eumbbar)
{
	I2C_STAT stat = I2C_Get_Stat (eumbbar);
	I2C_CTRL ctrl = I2C_Get_Ctrl (eumbbar);

	if (stat.srw == 1) {
		/* we are asked to xmit */
		ctrl.mtx = 1;
		I2C_Set_Ctrl (eumbbar, ctrl);	/* set MTX */
		return I2C_Slave_Xmit (eumbbar);
	}

	/* we are asked to receive data */
	ctrl.mtx = 0;
	I2C_Set_Ctrl (eumbbar, ctrl);
	(void) load_runtime_reg (eumbbar, I2CDR);	/* do a fake read to start */

	return I2CADDRESS;
}

/***********************************************
 * function: I2C_ISR
 *
 * description: I2C Interrupt service routine
 *
 * note: Precondition:
 *      I2CSR(MIF) == 1
 **********************************************/
static I2CStatus I2C_ISR (unsigned int eumbbar)
{
	I2C_STAT stat;
	I2C_CTRL ctrl;

#ifdef I2CDBG0
	PRINT ("%s(%d): I2C_ISR\n", __FILE__, __LINE__);
#endif

	stat = I2C_Get_Stat (eumbbar);
	ctrl = I2C_Get_Ctrl (eumbbar);

	/* clear MIF */
	stat.mif = 0;

	/* Now let see what kind of event this is */
	if (stat.mcf == 1) {
		/* transfer compete */

		/* clear the MIF bit */
		I2C_Set_Stat (eumbbar, stat);

		if (ctrl.msta == 1) {
			/* master */
			if (ctrl.mtx == 1) {
				/* check if this is the address phase for master receive */
				if (MasterRcvAddress == 1) {
					/* Yes, it is the address phase of master receive */
					ctrl.mtx = 0;
					/* now check how much we want to receive */
					if (ByteToRcv == 1 && RcvBufFulStop == 1) {
						ctrl.txak = 1;
					}

					I2C_Set_Ctrl (eumbbar, ctrl);
					(void) load_runtime_reg (eumbbar, I2CDR);	/* fake read first */

					MasterRcvAddress = 0;
					return I2CADDRESS;

				}

				/* master xmit */
				if (stat.rxak == 0) {
					/* slave has acknowledged */
					return I2C_Master_Xmit (eumbbar);
				}

				/* slave has not acknowledged yet, generate a STOP */
				if (XmitBufEmptyStop == 1) {
					ctrl.msta = 0;
					I2C_Set_Ctrl (eumbbar, ctrl);
				}

				return I2CSUCCESS;
			}

			/* master receive */
			return I2C_Master_Rcv (eumbbar);
		}

		/* slave */
		if (ctrl.mtx == 1) {
			/* slave xmit */
			if (stat.rxak == 0) {
				/* master has acknowledged */
				return I2C_Slave_Xmit (eumbbar);
			}

			/* master has not acknowledged, wait for STOP */
			/* do nothing for preventing bus from hung */
			return I2CSUCCESS;
		}

		/* slave rcv */
		return I2C_Slave_Rcv (eumbbar);

	} else if (stat.maas == 1) {
		/* received a call from master */

		/* clear the MIF bit */
		I2C_Set_Stat (eumbbar, stat);

		/* master is calling us, process the address phase */
		return I2C_Slave_Addr (eumbbar);
	} else {
		/* has to be arbitration lost */
		stat.mal = 0;
		I2C_Set_Stat (eumbbar, stat);

		ctrl.msta = 0;			/* return to receive mode */
		I2C_Set_Ctrl (eumbbar, ctrl);
	}

	return I2CSUCCESS;

}

/******************************************************
 * function: I2C_Set_Stat
 *
 * description: modify the I2CSR
 *
 *****************************************************/
static void I2C_Set_Stat (unsigned int eumbbar, I2C_STAT stat)
{
	union {
		unsigned int val;
		I2C_STAT stat;
	} s_tmp;
	union {
		unsigned int val;
		I2C_STAT stat;
	} s;

	s.val = load_runtime_reg (eumbbar, I2CSR);
	s.val &= 0xffffff08;
	s_tmp.stat = stat;
	s.val |= (s_tmp.val & 0xf7);

#ifdef I2CDBG0
	PRINT ("%s(%d): set stat = 0x%08x\n", __FILE__, __LINE__, s.val);
#endif

	store_runtime_reg (eumbbar, I2CSR, s.val);

}

/******************************************************
 * The following are routines to glue the rest of
 * U-Boot to the Sandpoint I2C driver.
 *****************************************************/

void i2c_init (int speed, int slaveadd)
{
#ifdef CFG_I2C_INIT_BOARD
	/*
	 * call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	 */
	i2c_init_board();
#endif

#ifdef DEBUG
	I2C_Initialize (0x7f, 0, (void *) printf);
#else
	I2C_Initialize (0x7f, 0, 0);
#endif
}

int i2c_probe (uchar chip)
{
	int tmp;

	/*
	 * Try to read the first location of the chip.  The underlying
	 * driver doesn't appear to support sending just the chip address
	 * and looking for an <ACK> back.
	 */
	udelay(10000);
	return i2c_read (chip, 0, 1, (char *)&tmp, 1);
}

int i2c_read (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	I2CStatus status;
	uchar xaddr[4];

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;

		status = I2C_do_buffer (0, I2C_MASTER_XMIT, chip, alen,
					&xaddr[4 - alen], I2C_NO_STOP, 1,
					I2C_NO_RESTART);
		if (status != I2C_SUCCESS) {
			PRINT ("i2c_read: can't send data address for read\n");
			return 1;
		}
	}

	/* The data transfer will be a continuation. */
	status = I2C_do_buffer (0, I2C_MASTER_RCV, chip, len,
				buffer, I2C_STOP, 1, (alen > 0 ? I2C_RESTART :
				I2C_NO_RESTART));

	if (status != I2C_SUCCESS) {
		PRINT ("i2c_read: can't perform data transfer\n");
		return 1;
	}

	return 0;
}

int i2c_write (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	I2CStatus status;
	unsigned char dummy_buffer[I2C_RXTX_LEN + 2];
	int i;

	dummy_buffer[0] = addr & 0xFF;
	if (alen == 2)
		dummy_buffer[1] = (addr >> 8) & 0xFF;
	for (i = 0; i < len; i++)
		dummy_buffer[i + alen] = buffer[i];

	status = I2C_do_buffer (0, I2C_MASTER_XMIT, chip, alen + len,
				dummy_buffer, I2C_STOP, 1, I2C_NO_RESTART);

#ifdef CFG_EEPROM_PAGE_WRITE_DELAY_MS
	udelay(CFG_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
	if (status != I2C_SUCCESS) {
		PRINT ("i2c_write: can't perform data transfer\n");
		return 1;
	}

	return 0;
}

uchar i2c_reg_read (uchar i2c_addr, uchar reg)
{
	char buf[1];

	i2c_init (0, 0);

	i2c_read (i2c_addr, reg, 1, buf, 1);

	return (buf[0]);
}

void i2c_reg_write (uchar i2c_addr, uchar reg, uchar val)
{
	i2c_init (0, 0);

	i2c_write (i2c_addr, reg, 1, &val, 1);
}

#endif	/* CONFIG_HARD_I2C */
