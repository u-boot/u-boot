/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 *
 * (C) Copyright 2000 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CONFIG_HARD_I2C)

#include <asm/cpm_8260.h>
#include <i2c.h>

/* define to enable debug messages */
#undef  DEBUG_I2C

DECLARE_GLOBAL_DATA_PTR;

/* uSec to wait between polls of the i2c */
#define DELAY_US	100
/* uSec to wait for the CPM to start processing the buffer */
#define START_DELAY_US	1000

/*
 * tx/rx per-byte timeout: we delay DELAY_US uSec between polls so the
 * timeout will be (tx_length + rx_length) * DELAY_US * TOUT_LOOP
 */
#define TOUT_LOOP 5

/*-----------------------------------------------------------------------
 * Set default values
 */
#ifndef	CFG_I2C_SPEED
#define	CFG_I2C_SPEED	50000
#endif

#ifndef	CFG_I2C_SLAVE
#define	CFG_I2C_SLAVE	0xFE
#endif
/*-----------------------------------------------------------------------
 */

typedef void (*i2c_ecb_t)(int, int, void *);    /* error callback function */

/* This structure keeps track of the bd and buffer space usage. */
typedef struct i2c_state {
	int		rx_idx;		/* index   to next free Rx BD */
	int		tx_idx;		/* index   to next free Tx BD */
	void		*rxbd;		/* pointer to next free Rx BD */
	void		*txbd;		/* pointer to next free Tx BD */
	int		tx_space;	/* number  of Tx bytes left   */
	unsigned char	*tx_buf;	/* pointer to free Tx area    */
	i2c_ecb_t	err_cb;		/* error callback function    */
	void		*cb_data;	/* private data to be passed  */
} i2c_state_t;

/* flags for i2c_send() and i2c_receive() */
#define	I2CF_ENABLE_SECONDARY	0x01	/* secondary_address is valid	*/
#define	I2CF_START_COND		0x02	/* tx: generate start condition	*/
#define I2CF_STOP_COND		0x04	/* tx: generate stop  condition	*/

/* return codes */
#define I2CERR_NO_BUFFERS	1	/* no more BDs or buffer space	*/
#define I2CERR_MSG_TOO_LONG	2	/* tried to send/receive to much data   */
#define I2CERR_TIMEOUT		3	/* timeout in i2c_doio()	*/
#define I2CERR_QUEUE_EMPTY	4	/* i2c_doio called without send/receive */
#define I2CERR_IO_ERROR		5	/* had an error during comms	*/

/* error callback flags */
#define I2CECB_RX_ERR		0x10	/* this is a receive error	*/
#define     I2CECB_RX_OV	0x02	/* receive overrun error	*/
#define     I2CECB_RX_MASK	0x0f	/* mask for error bits		*/
#define I2CECB_TX_ERR		0x20	/* this is a transmit error	*/
#define     I2CECB_TX_CL	0x01	/* transmit collision error	*/
#define     I2CECB_TX_UN	0x02	/* transmit underflow error	*/
#define     I2CECB_TX_NAK	0x04	/* transmit no ack error	*/
#define     I2CECB_TX_MASK	0x0f	/* mask for error bits		*/
#define I2CECB_TIMEOUT		0x40	/* this is a timeout error	*/

#define ERROR_I2C_NONE		0
#define ERROR_I2C_LENGTH	1

#define I2C_WRITE_BIT		0x00
#define I2C_READ_BIT		0x01

#define I2C_RXTX_LEN	128	/* maximum tx/rx buffer length */


#define NUM_RX_BDS 4
#define NUM_TX_BDS 4
#define MAX_TX_SPACE 256

typedef struct I2C_BD
{
  unsigned short status;
  unsigned short length;
  unsigned char *addr;
} I2C_BD;
#define BD_I2C_TX_START 0x0400  /* special status for i2c: Start condition */

#define BD_I2C_TX_CL	0x0001	/* collision error */
#define BD_I2C_TX_UN	0x0002	/* underflow error */
#define BD_I2C_TX_NAK	0x0004	/* no acknowledge error */
#define BD_I2C_TX_ERR	(BD_I2C_TX_NAK|BD_I2C_TX_UN|BD_I2C_TX_CL)

#define BD_I2C_RX_ERR	BD_SC_OV

#ifdef DEBUG_I2C
#define PRINTD(x) printf x
#else
#define PRINTD(x)
#endif

/*
 * Returns the best value of I2BRG to meet desired clock speed of I2C with
 * input parameters (clock speed, filter, and predivider value).
 * It returns computer speed value and the difference between it and desired
 * speed.
 */
static inline int
i2c_roundrate(int hz, int speed, int filter, int modval,
		int *brgval, int *totspeed)
{
    int moddiv = 1 << (5-(modval & 3)), brgdiv, div;

    PRINTD(("\t[I2C] trying hz=%d, speed=%d, filter=%d, modval=%d\n",
	hz, speed, filter, modval));

    div = moddiv * speed;
    brgdiv = (hz + div - 1) / div;

    PRINTD(("\t\tmoddiv=%d, brgdiv=%d\n", moddiv, brgdiv));

    *brgval = ((brgdiv + 1) / 2) - 3 - (2*filter);

    if ((*brgval < 0) || (*brgval > 255)) {
	  PRINTD(("\t\trejected brgval=%d\n", *brgval));
	  return -1;
    }

    brgdiv = 2 * (*brgval + 3 + (2 * filter));
    div = moddiv * brgdiv ;
    *totspeed = hz / div;

    PRINTD(("\t\taccepted brgval=%d, totspeed=%d\n", *brgval, *totspeed));

    return  0;
}

/*
 * Sets the I2C clock predivider and divider to meet required clock speed.
 */
static int i2c_setrate(int hz, int speed)
{
    immap_t	*immap = (immap_t *)CFG_IMMR ;
    volatile i2c8260_t *i2c = (i2c8260_t *)&immap->im_i2c;
    int brgval,
	  modval,	/* 0-3 */
	  bestspeed_diff = speed,
	  bestspeed_brgval=0,
	  bestspeed_modval=0,
	  bestspeed_filter=0,
	  totspeed,
	  filter = 0; /* Use this fixed value */

	for (modval = 0; modval < 4; modval++)
	{
		if (i2c_roundrate (hz, speed, filter, modval, &brgval, &totspeed) == 0)
		{
			int diff = speed - totspeed ;

			if ((diff >= 0) && (diff < bestspeed_diff))
			{
				bestspeed_diff 	= diff ;
				bestspeed_modval 	= modval;
				bestspeed_brgval 	= brgval;
				bestspeed_filter 	= filter;
			}
		}
	}

    PRINTD(("[I2C] Best is:\n"));
    PRINTD(("[I2C] CPU=%dhz RATE=%d F=%d I2MOD=%08x I2BRG=%08x DIFF=%dhz\n",
		   hz, speed,
		   bestspeed_filter, bestspeed_modval, bestspeed_brgval,
		   bestspeed_diff));

    i2c->i2c_i2mod |= ((bestspeed_modval & 3) << 1) | (bestspeed_filter << 3);
    i2c->i2c_i2brg = bestspeed_brgval & 0xff;

    PRINTD(("[I2C] i2mod=%08x i2brg=%08x\n", i2c->i2c_i2mod, i2c->i2c_i2brg));

    return 1 ;
}

void i2c_init(int speed, int slaveadd)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR ;
	volatile cpm8260_t *cp = (cpm8260_t *)&immap->im_cpm;
	volatile i2c8260_t *i2c	= (i2c8260_t *)&immap->im_i2c;
	volatile iic_t *iip;
	ulong rbase, tbase;
	volatile I2C_BD *rxbd, *txbd;
	uint dpaddr;

#ifdef CFG_I2C_INIT_BOARD
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#endif

	dpaddr = *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE]));
	if (dpaddr == 0) {
	    /* need to allocate dual port ram */
	    dpaddr = m8260_cpm_dpalloc(64 +
		(NUM_RX_BDS * sizeof(I2C_BD)) + (NUM_TX_BDS * sizeof(I2C_BD)) +
		MAX_TX_SPACE, 64);
	    *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE])) = dpaddr;
	}

	/*
	 * initialise data in dual port ram:
	 *
	 * 	  dpaddr -> parameter ram (64 bytes)
	 *         rbase -> rx BD         (NUM_RX_BDS * sizeof(I2C_BD) bytes)
	 *         tbase -> tx BD         (NUM_TX_BDS * sizeof(I2C_BD) bytes)
	 *                  tx buffer     (MAX_TX_SPACE bytes)
	 */

	iip = (iic_t *)&immap->im_dprambase[dpaddr];
	memset((void*)iip, 0, sizeof(iic_t));

	rbase = dpaddr + 64;
	tbase = rbase + NUM_RX_BDS * sizeof(I2C_BD);

	/* Disable interrupts */
	i2c->i2c_i2mod = 0x00;
	i2c->i2c_i2cmr = 0x00;
	i2c->i2c_i2cer = 0xff;
	i2c->i2c_i2add = slaveadd;

	/*
	 * Set the I2C BRG Clock division factor from desired i2c rate
	 * and current CPU rate (we assume sccr dfbgr field is 0;
	 * divide BRGCLK by 1)
	 */
	PRINTD(("[I2C] Setting rate...\n"));
	i2c_setrate (gd->brg_clk, CFG_I2C_SPEED) ;

	/* Set I2C controller in master mode */
	i2c->i2c_i2com = 0x01;

	/* Initialize Tx/Rx parameters */
	iip->iic_rbase = rbase;
	iip->iic_tbase = tbase;
	rxbd = (I2C_BD *)((unsigned char *)&immap->im_dprambase[iip->iic_rbase]);
	txbd = (I2C_BD *)((unsigned char *)&immap->im_dprambase[iip->iic_tbase]);

	PRINTD(("[I2C] rbase = %04x\n", iip->iic_rbase));
	PRINTD(("[I2C] tbase = %04x\n", iip->iic_tbase));
	PRINTD(("[I2C] rxbd = %08x\n", (int)rxbd));
	PRINTD(("[I2C] txbd = %08x\n", (int)txbd));

	/* Set big endian byte order */
	iip->iic_tfcr = 0x10;
	iip->iic_rfcr = 0x10;

	/* Set maximum receive size. */
	iip->iic_mrblr = I2C_RXTX_LEN;

    cp->cp_cpcr = mk_cr_cmd(CPM_CR_I2C_PAGE,
							CPM_CR_I2C_SBLOCK,
							0x00,
							CPM_CR_INIT_TRX) | CPM_CR_FLG;
    do {
		__asm__ __volatile__ ("eieio");
    } while (cp->cp_cpcr & CPM_CR_FLG);

	/* Clear events and interrupts */
	i2c->i2c_i2cer = 0xff;
	i2c->i2c_i2cmr = 0x00;
}

static
void i2c_newio(i2c_state_t *state)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR ;
	volatile iic_t *iip;
	uint dpaddr;

	PRINTD(("[I2C] i2c_newio\n"));

	dpaddr = *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE]));
	iip = (iic_t *)&immap->im_dprambase[dpaddr];
	state->rx_idx = 0;
	state->tx_idx = 0;
	state->rxbd = (void*)&immap->im_dprambase[iip->iic_rbase];
	state->txbd = (void*)&immap->im_dprambase[iip->iic_tbase];
	state->tx_space = MAX_TX_SPACE;
	state->tx_buf = (uchar*)state->txbd + NUM_TX_BDS * sizeof(I2C_BD);
	state->err_cb = NULL;
	state->cb_data = NULL;

	PRINTD(("[I2C] rxbd = %08x\n", (int)state->rxbd));
	PRINTD(("[I2C] txbd = %08x\n", (int)state->txbd));
	PRINTD(("[I2C] tx_buf = %08x\n", (int)state->tx_buf));

	/* clear the buffer memory */
	memset((char *)state->tx_buf, 0, MAX_TX_SPACE);
}

static
int i2c_send(i2c_state_t *state,
			 unsigned char address,
			 unsigned char secondary_address,
			 unsigned int flags,
			 unsigned short size,
			 unsigned char *dataout)
{
	volatile I2C_BD *txbd;
	int i,j;

	PRINTD(("[I2C] i2c_send add=%02d sec=%02d flag=%02d size=%d\n",
			address, secondary_address, flags, size));

	/* trying to send message larger than BD */
	if (size > I2C_RXTX_LEN)
	  return I2CERR_MSG_TOO_LONG;

	/* no more free bds */
	if (state->tx_idx >= NUM_TX_BDS || state->tx_space < (2 + size))
	  return I2CERR_NO_BUFFERS;

	txbd = (I2C_BD *)state->txbd;
	txbd->addr = state->tx_buf;

	PRINTD(("[I2C] txbd = %08x\n", (int)txbd));

    if (flags & I2CF_START_COND)
    {
	PRINTD(("[I2C] Formatting addresses...\n"));
	if (flags & I2CF_ENABLE_SECONDARY)
	{
		txbd->length = size + 2;  /* Length of message plus dest addresses */
		txbd->addr[0] = address << 1;
		txbd->addr[1] = secondary_address;
		i = 2;
	}
	else
	{
		txbd->length = size + 1;  /* Length of message plus dest address */
		txbd->addr[0] = address << 1;  /* Write destination address to BD */
		i = 1;
	}
    }
    else
    {
	txbd->length = size;  /* Length of message */
	i = 0;
    }

	/* set up txbd */
	txbd->status = BD_SC_READY;
	if (flags & I2CF_START_COND)
	  txbd->status |= BD_I2C_TX_START;
	if (flags & I2CF_STOP_COND)
	  txbd->status |= BD_SC_LAST | BD_SC_WRAP;

	/* Copy data to send into buffer */
	PRINTD(("[I2C] copy data...\n"));
	for(j = 0; j < size; i++, j++)
	  txbd->addr[i] = dataout[j];

	PRINTD(("[I2C] txbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		   txbd->length,
		   txbd->status,
		   txbd->addr[0],
		   txbd->addr[1]));

	/* advance state */
	state->tx_buf += txbd->length;
	state->tx_space -= txbd->length;
	state->tx_idx++;
	state->txbd = (void*)(txbd + 1);

	return 0;
}

static
int i2c_receive(i2c_state_t *state,
				unsigned char address,
				unsigned char secondary_address,
				unsigned int flags,
				unsigned short size_to_expect,
				unsigned char *datain)
{
	volatile I2C_BD *rxbd, *txbd;

	PRINTD(("[I2C] i2c_receive %02d %02d %02d\n", address, secondary_address, flags));

	/* Expected to receive too much */
	if (size_to_expect > I2C_RXTX_LEN)
	  return I2CERR_MSG_TOO_LONG;

	/* no more free bds */
	if (state->tx_idx >= NUM_TX_BDS || state->rx_idx >= NUM_RX_BDS
		 || state->tx_space < 2)
	  return I2CERR_NO_BUFFERS;

	rxbd = (I2C_BD *)state->rxbd;
	txbd = (I2C_BD *)state->txbd;

	PRINTD(("[I2C] rxbd = %08x\n", (int)rxbd));
	PRINTD(("[I2C] txbd = %08x\n", (int)txbd));

	txbd->addr = state->tx_buf;

	/* set up TXBD for destination address */
	if (flags & I2CF_ENABLE_SECONDARY)
	{
		txbd->length = 2;
		txbd->addr[0] = address << 1;   /* Write data */
		txbd->addr[1] = secondary_address;  /* Internal address */
		txbd->status = BD_SC_READY;
	}
	else
	{
		txbd->length = 1 + size_to_expect;
		txbd->addr[0] = (address << 1) | 0x01;
		txbd->status = BD_SC_READY;
		memset(&txbd->addr[1], 0, txbd->length);
	}

	/* set up rxbd for reception */
	rxbd->status = BD_SC_EMPTY;
	rxbd->length = size_to_expect;
	rxbd->addr = datain;

	txbd->status |= BD_I2C_TX_START;
	if (flags & I2CF_STOP_COND)
	{
		txbd->status |= BD_SC_LAST | BD_SC_WRAP;
		rxbd->status |= BD_SC_WRAP;
	}

	PRINTD(("[I2C] txbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		   txbd->length,
		   txbd->status,
		   txbd->addr[0],
		   txbd->addr[1]));
	PRINTD(("[I2C] rxbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		   rxbd->length,
		   rxbd->status,
		   rxbd->addr[0],
		   rxbd->addr[1]));

	/* advance state */
	state->tx_buf += txbd->length;
	state->tx_space -= txbd->length;
	state->tx_idx++;
	state->txbd = (void*)(txbd + 1);
	state->rx_idx++;
	state->rxbd = (void*)(rxbd + 1);

	return 0;
}


static
int i2c_doio(i2c_state_t *state)
{
	volatile immap_t *immap = (immap_t *)CFG_IMMR ;
	volatile iic_t *iip;
	volatile i2c8260_t *i2c	= (i2c8260_t *)&immap->im_i2c;
	volatile I2C_BD *txbd, *rxbd;
	int  n, i, b, rxcnt = 0, rxtimeo = 0, txcnt = 0, txtimeo = 0, rc = 0;
	uint dpaddr;

	PRINTD(("[I2C] i2c_doio\n"));

	if (state->tx_idx <= 0 && state->rx_idx <= 0) {
		PRINTD(("[I2C] No I/O is queued\n"));
		return I2CERR_QUEUE_EMPTY;
	}

	dpaddr = *((unsigned short*)(&immap->im_dprambase[PROFF_I2C_BASE]));
	iip = (iic_t *)&immap->im_dprambase[dpaddr];
	iip->iic_rbptr = iip->iic_rbase;
	iip->iic_tbptr = iip->iic_tbase;

	/* Enable I2C */
	PRINTD(("[I2C] Enabling I2C...\n"));
	i2c->i2c_i2mod |= 0x01;

	/* Begin transmission */
	i2c->i2c_i2com |= 0x80;

	/* Loop until transmit & receive completed */

	if ((n = state->tx_idx) > 0) {

		txbd = ((I2C_BD*)state->txbd) - n;
		for (i = 0; i < n; i++) {
			txtimeo += TOUT_LOOP * txbd->length;
			txbd++;
		}

		txbd--; /* wait until last in list is done */

		PRINTD(("[I2C] Transmitting...(txbd=0x%08lx)\n", (ulong)txbd));

		udelay(START_DELAY_US);	/* give it time to start */
		while((txbd->status & BD_SC_READY) && (++txcnt < txtimeo)) {
			udelay(DELAY_US);
			if (ctrlc())
				return (-1);
			__asm__ __volatile__ ("eieio");
		}
	}

	if (txcnt < txtimeo && (n = state->rx_idx) > 0) {

		rxbd = ((I2C_BD*)state->rxbd) - n;
		for (i = 0; i < n; i++) {
			rxtimeo += TOUT_LOOP * rxbd->length;
			rxbd++;
		}

		rxbd--; /* wait until last in list is done */

		PRINTD(("[I2C] Receiving...(rxbd=0x%08lx)\n", (ulong)rxbd));

		udelay(START_DELAY_US);	/* give it time to start */
		while((rxbd->status & BD_SC_EMPTY) && (++rxcnt < rxtimeo)) {
			udelay(DELAY_US);
			if (ctrlc())
				return (-1);
			__asm__ __volatile__ ("eieio");
		}
	}

	/* Turn off I2C */
	i2c->i2c_i2mod &= ~0x01;

	if ((n = state->tx_idx) > 0) {
		for (i = 0; i < n; i++) {
			txbd = ((I2C_BD*)state->txbd) - (n - i);
			if ((b = txbd->status & BD_I2C_TX_ERR) != 0) {
				if (state->err_cb != NULL)
					(*state->err_cb)(I2CECB_TX_ERR|b, i,
						state->cb_data);
				if (rc == 0)
					rc = I2CERR_IO_ERROR;
			}
		}
	}

	if ((n = state->rx_idx) > 0) {
		for (i = 0; i < n; i++) {
			rxbd = ((I2C_BD*)state->rxbd) - (n - i);
			if ((b = rxbd->status & BD_I2C_RX_ERR) != 0) {
				if (state->err_cb != NULL)
					(*state->err_cb)(I2CECB_RX_ERR|b, i,
						state->cb_data);
				if (rc == 0)
					rc = I2CERR_IO_ERROR;
			}
		}
	}

	if ((txtimeo > 0 && txcnt >= txtimeo) || \
	    (rxtimeo > 0 && rxcnt >= rxtimeo)) {
		if (state->err_cb != NULL)
			(*state->err_cb)(I2CECB_TIMEOUT, -1, state->cb_data);
		if (rc == 0)
			rc = I2CERR_TIMEOUT;
	}

	return (rc);
}

static void
i2c_probe_callback(int flags, int xnum, void *data)
{
	/*
	 * the only acceptable errors are a transmit NAK or a receive
	 * overrun - tx NAK means the device does not exist, rx OV
	 * means the device must have responded to the slave address
	 * even though the transfer failed
	 */
	if (flags == (I2CECB_TX_ERR|I2CECB_TX_NAK))
		*(int *)data |= 1;
	if (flags == (I2CECB_RX_ERR|I2CECB_RX_OV))
		*(int *)data |= 2;
}

int
i2c_probe(uchar chip)
{
	i2c_state_t state;
	int rc, err_flag;
	uchar buf[1];

	i2c_newio(&state);

	state.err_cb = i2c_probe_callback;
	state.cb_data = (void *) &err_flag;
	err_flag = 0;

	rc = i2c_receive(&state, chip, 0, I2CF_START_COND|I2CF_STOP_COND, 1, buf);

	if (rc != 0)
		return (rc);	/* probe failed */

	rc = i2c_doio(&state);

	if (rc == 0)
		return (0);	/* device exists - read succeeded */

	if (rc == I2CERR_TIMEOUT)
		return (-1);	/* device does not exist - timeout */

	if (rc != I2CERR_IO_ERROR || err_flag == 0)
		return (rc);	/* probe failed */

	if (err_flag & 1)
		return (-1);	/* device does not exist - had transmit NAK */

	return (0);	/* device exists - had receive overrun */
}


int
i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	i2c_state_t state;
	uchar xaddr[4];
	int rc;

	xaddr[0] = (addr >> 24) & 0xFF;
	xaddr[1] = (addr >> 16) & 0xFF;
	xaddr[2] = (addr >>  8) & 0xFF;
	xaddr[3] =  addr        & 0xFF;

#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
	 /*
	  * EEPROM chips that implement "address overflow" are ones
	  * like Catalyst 24WC04/08/16 which has 9/10/11 bits of address
	  * and the extra bits end up in the "chip address" bit slots.
	  * This makes a 24WC08 (1Kbyte) chip look like four 256 byte
	  * chips.
	  *
	  * Note that we consider the length of the address field to still
	  * be one byte because the extra address bits are hidden in the
	  * chip address.
	  */
	chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	i2c_newio(&state);

	rc = i2c_send(&state, chip, 0, I2CF_START_COND, alen, &xaddr[4-alen]);
	if (rc != 0) {
		printf("i2c_read: i2c_send failed (%d)\n", rc);
		return 1;
	}

	rc = i2c_receive(&state, chip, 0, I2CF_STOP_COND, len, buffer);
	if (rc != 0) {
		printf("i2c_read: i2c_receive failed (%d)\n", rc);
		return 1;
	}

	rc = i2c_doio(&state);
	if (rc != 0) {
		printf("i2c_read: i2c_doio failed (%d)\n", rc);
		return 1;
	}
	return 0;
}

int
i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	i2c_state_t state;
	uchar xaddr[4];
	int rc;

	xaddr[0] = (addr >> 24) & 0xFF;
	xaddr[1] = (addr >> 16) & 0xFF;
	xaddr[2] = (addr >>  8) & 0xFF;
	xaddr[3] =  addr        & 0xFF;

#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
	 /*
	  * EEPROM chips that implement "address overflow" are ones
	  * like Catalyst 24WC04/08/16 which has 9/10/11 bits of address
	  * and the extra bits end up in the "chip address" bit slots.
	  * This makes a 24WC08 (1Kbyte) chip look like four 256 byte
	  * chips.
	  *
	  * Note that we consider the length of the address field to still
	  * be one byte because the extra address bits are hidden in the
	  * chip address.
	  */
	chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	i2c_newio(&state);

	rc = i2c_send(&state, chip, 0, I2CF_START_COND, alen, &xaddr[4-alen]);
	if (rc != 0) {
		printf("i2c_write: first i2c_send failed (%d)\n", rc);
		return 1;
	}

	rc = i2c_send(&state, 0, 0, I2CF_STOP_COND, len, buffer);
	if (rc != 0) {
		printf("i2c_write: second i2c_send failed (%d)\n", rc);
		return 1;
	}

	rc = i2c_doio(&state);
	if (rc != 0) {
		printf("i2c_write: i2c_doio failed (%d)\n", rc);
		return 1;
	}
	return 0;
}

uchar
i2c_reg_read(uchar chip, uchar reg)
{
	uchar buf;

	i2c_read(chip, reg, 1, &buf, 1);

	return (buf);
}

void
i2c_reg_write(uchar chip, uchar reg, uchar val)
{
	i2c_write(chip, reg, 1, &val, 1);
}

#endif	/* CONFIG_HARD_I2C */
