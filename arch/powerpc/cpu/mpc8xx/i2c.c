/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 *
 * (C) Copyright 2000 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

#include <common.h>

#ifdef CONFIG_HARD_I2C

#include <commproc.h>
#include <i2c.h>
#ifdef CONFIG_LWMON
#include <watchdog.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* tx/rx timeout (we need the i2c early, so we don't use get_timer()) */
#define TOUT_LOOP 1000000

#define NUM_RX_BDS 4
#define NUM_TX_BDS 4
#define MAX_TX_SPACE 256
#define I2C_RXTX_LEN 128	/* maximum tx/rx buffer length */

typedef struct I2C_BD {
	unsigned short status;
	unsigned short length;
	unsigned char *addr;
} I2C_BD;

#define BD_I2C_TX_START 0x0400	/* special status for i2c: Start condition */

#define BD_I2C_TX_CL	0x0001	/* collision error */
#define BD_I2C_TX_UN	0x0002	/* underflow error */
#define BD_I2C_TX_NAK	0x0004	/* no acknowledge error */
#define BD_I2C_TX_ERR	(BD_I2C_TX_NAK|BD_I2C_TX_UN|BD_I2C_TX_CL)

#define BD_I2C_RX_ERR	BD_SC_OV

typedef void (*i2c_ecb_t) (int, int);	/* error callback function */

/* This structure keeps track of the bd and buffer space usage. */
typedef struct i2c_state {
	int rx_idx;		/* index   to next free Rx BD */
	int tx_idx;		/* index   to next free Tx BD */
	void *rxbd;		/* pointer to next free Rx BD */
	void *txbd;		/* pointer to next free Tx BD */
	int tx_space;		/* number  of Tx bytes left   */
	unsigned char *tx_buf;	/* pointer to free Tx area    */
	i2c_ecb_t err_cb;	/* error callback function    */
} i2c_state_t;


/* flags for i2c_send() and i2c_receive() */
#define I2CF_ENABLE_SECONDARY	0x01  /* secondary_address is valid           */
#define I2CF_START_COND		0x02  /* tx: generate start condition         */
#define I2CF_STOP_COND		0x04  /* tx: generate stop  condition         */

/* return codes */
#define I2CERR_NO_BUFFERS	0x01  /* no more BDs or buffer space          */
#define I2CERR_MSG_TOO_LONG	0x02  /* tried to send/receive to much data   */
#define I2CERR_TIMEOUT		0x03  /* timeout in i2c_doio()                */
#define I2CERR_QUEUE_EMPTY	0x04  /* i2c_doio called without send/receive */

/* error callback flags */
#define I2CECB_RX_ERR		0x10  /* this is a receive error              */
#define     I2CECB_RX_ERR_OV	0x02  /* receive overrun error                */
#define     I2CECB_RX_MASK	0x0f  /* mask for error bits                  */
#define I2CECB_TX_ERR		0x20  /* this is a transmit error             */
#define     I2CECB_TX_CL	0x01  /* transmit collision error             */
#define     I2CECB_TX_UN	0x02  /* transmit underflow error             */
#define     I2CECB_TX_NAK	0x04  /* transmit no ack error                */
#define     I2CECB_TX_MASK	0x0f  /* mask for error bits                  */
#define I2CECB_TIMEOUT		0x40  /* this is a timeout error              */

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
	int moddiv = 1 << (5 - (modval & 3)), brgdiv, div;

	debug("\t[I2C] trying hz=%d, speed=%d, filter=%d, modval=%d\n",
		hz, speed, filter, modval);

	div = moddiv * speed;
	brgdiv = (hz + div - 1) / div;

	debug("\t\tmoddiv=%d, brgdiv=%d\n", moddiv, brgdiv);

	*brgval = ((brgdiv + 1) / 2) - 3 - (2 * filter);

	if ((*brgval < 0) || (*brgval > 255)) {
		debug("\t\trejected brgval=%d\n", *brgval);
		return -1;
	}

	brgdiv = 2 * (*brgval + 3 + (2 * filter));
	div = moddiv * brgdiv;
	*totspeed = hz / div;

	debug("\t\taccepted brgval=%d, totspeed=%d\n", *brgval, *totspeed);

	return 0;
}

/*
 * Sets the I2C clock predivider and divider to meet required clock speed.
 */
static int i2c_setrate(int hz, int speed)
{
	immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile i2c8xx_t *i2c = (i2c8xx_t *) & immap->im_i2c;
	int	brgval,
		modval,	/* 0-3 */
		bestspeed_diff = speed,
		bestspeed_brgval = 0,
		bestspeed_modval = 0,
		bestspeed_filter = 0,
		totspeed,
		filter = 0;	/* Use this fixed value */

	for (modval = 0; modval < 4; modval++) {
		if (i2c_roundrate
		    (hz, speed, filter, modval, &brgval, &totspeed) == 0) {
			int diff = speed - totspeed;

			if ((diff >= 0) && (diff < bestspeed_diff)) {
				bestspeed_diff = diff;
				bestspeed_modval = modval;
				bestspeed_brgval = brgval;
				bestspeed_filter = filter;
			}
		}
	}

	debug("[I2C] Best is:\n");
	debug("[I2C] CPU=%dhz RATE=%d F=%d I2MOD=%08x I2BRG=%08x DIFF=%dhz\n",
		hz,
		speed,
		bestspeed_filter,
		bestspeed_modval,
		bestspeed_brgval,
		bestspeed_diff);

	i2c->i2c_i2mod |=
		((bestspeed_modval & 3) << 1) | (bestspeed_filter << 3);
	i2c->i2c_i2brg = bestspeed_brgval & 0xff;

	debug("[I2C] i2mod=%08x i2brg=%08x\n",
		i2c->i2c_i2mod,
		i2c->i2c_i2brg);

	return 1;
}

void i2c_init(int speed, int slaveaddr)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = (cpm8xx_t *)&immap->im_cpm;
	volatile i2c8xx_t *i2c = (i2c8xx_t *)&immap->im_i2c;
	volatile iic_t *iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];
	ulong rbase, tbase;
	volatile I2C_BD *rxbd, *txbd;
	uint dpaddr;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#endif

#ifdef CONFIG_SYS_I2C_UCODE_PATCH
	iip = (iic_t *)&cp->cp_dpmem[iip->iic_rpbase];
#else
	/* Disable relocation */
	iip->iic_rpbase = 0;
#endif

#ifdef CONFIG_SYS_ALLOC_DPRAM
	dpaddr = iip->iic_rbase;
	if (dpaddr == 0) {
		/* need to allocate dual port ram */
		dpaddr = dpram_alloc_align((NUM_RX_BDS * sizeof(I2C_BD)) +
					   (NUM_TX_BDS * sizeof(I2C_BD)) +
					   MAX_TX_SPACE, 8);
	}
#else
	dpaddr = CPM_I2C_BASE;
#endif

	/*
	 * initialise data in dual port ram:
	 *
	 * dpaddr->rbase -> rx BD         (NUM_RX_BDS * sizeof(I2C_BD) bytes)
	 *         tbase -> tx BD         (NUM_TX_BDS * sizeof(I2C_BD) bytes)
	 *                  tx buffer     (MAX_TX_SPACE bytes)
	 */

	rbase = dpaddr;
	tbase = rbase + NUM_RX_BDS * sizeof(I2C_BD);

	/* Initialize Port B I2C pins. */
	cp->cp_pbpar |= 0x00000030;
	cp->cp_pbdir |= 0x00000030;
	cp->cp_pbodr |= 0x00000030;

	/* Disable interrupts */
	i2c->i2c_i2mod = 0x00;
	i2c->i2c_i2cmr = 0x00;
	i2c->i2c_i2cer = 0xff;
	i2c->i2c_i2add = slaveaddr;

	/*
	 * Set the I2C BRG Clock division factor from desired i2c rate
	 * and current CPU rate (we assume sccr dfbgr field is 0;
	 * divide BRGCLK by 1)
	 */
	debug("[I2C] Setting rate...\n");
	i2c_setrate(gd->cpu_clk, CONFIG_SYS_I2C_SPEED);

	/* Set I2C controller in master mode */
	i2c->i2c_i2com = 0x01;

	/* Set SDMA bus arbitration level to 5 (SDCR) */
	immap->im_siu_conf.sc_sdcr = 0x0001;

	/* Initialize Tx/Rx parameters */
	iip->iic_rbase = rbase;
	iip->iic_tbase = tbase;
	rxbd = (I2C_BD *) ((unsigned char *) &cp->cp_dpmem[iip->iic_rbase]);
	txbd = (I2C_BD *) ((unsigned char *) &cp->cp_dpmem[iip->iic_tbase]);

	debug("[I2C] rbase = %04x\n", iip->iic_rbase);
	debug("[I2C] tbase = %04x\n", iip->iic_tbase);
	debug("[I2C] rxbd = %08x\n", (int)rxbd);
	debug("[I2C] txbd = %08x\n", (int)txbd);

	/* Set big endian byte order */
	iip->iic_tfcr = 0x10;
	iip->iic_rfcr = 0x10;

	/* Set maximum receive size. */
	iip->iic_mrblr = I2C_RXTX_LEN;

#ifdef CONFIG_SYS_I2C_UCODE_PATCH
	/*
	 *  Initialize required parameters if using microcode patch.
	 */
	iip->iic_rbptr = iip->iic_rbase;
	iip->iic_tbptr = iip->iic_tbase;
	iip->iic_rstate = 0;
	iip->iic_tstate = 0;
#else
	cp->cp_cpcr = mk_cr_cmd(CPM_CR_CH_I2C, CPM_CR_INIT_TRX) | CPM_CR_FLG;
	do {
		__asm__ __volatile__("eieio");
	} while (cp->cp_cpcr & CPM_CR_FLG);
#endif

	/* Clear events and interrupts */
	i2c->i2c_i2cer = 0xff;
	i2c->i2c_i2cmr = 0x00;
}

static void i2c_newio(i2c_state_t *state)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = (cpm8xx_t *)&immap->im_cpm;
	volatile iic_t *iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];

	debug("[I2C] i2c_newio\n");

#ifdef CONFIG_SYS_I2C_UCODE_PATCH
	iip = (iic_t *)&cp->cp_dpmem[iip->iic_rpbase];
#endif
	state->rx_idx = 0;
	state->tx_idx = 0;
	state->rxbd = (void *)&cp->cp_dpmem[iip->iic_rbase];
	state->txbd = (void *)&cp->cp_dpmem[iip->iic_tbase];
	state->tx_space = MAX_TX_SPACE;
	state->tx_buf = (uchar *)state->txbd + NUM_TX_BDS * sizeof(I2C_BD);
	state->err_cb = NULL;

	debug("[I2C] rxbd = %08x\n", (int)state->rxbd);
	debug("[I2C] txbd = %08x\n", (int)state->txbd);
	debug("[I2C] tx_buf = %08x\n", (int)state->tx_buf);

	/* clear the buffer memory */
	memset((char *)state->tx_buf, 0, MAX_TX_SPACE);
}

static int
i2c_send(i2c_state_t *state,
	 unsigned char address,
	 unsigned char secondary_address,
	 unsigned int flags, unsigned short size, unsigned char *dataout)
{
	volatile I2C_BD *txbd;
	int i, j;

	debug("[I2C] i2c_send add=%02d sec=%02d flag=%02d size=%d\n",
		address, secondary_address, flags, size);

	/* trying to send message larger than BD */
	if (size > I2C_RXTX_LEN)
		return I2CERR_MSG_TOO_LONG;

	/* no more free bds */
	if (state->tx_idx >= NUM_TX_BDS || state->tx_space < (2 + size))
		return I2CERR_NO_BUFFERS;

	txbd = (I2C_BD *) state->txbd;
	txbd->addr = state->tx_buf;

	debug("[I2C] txbd = %08x\n", (int)txbd);

	if (flags & I2CF_START_COND) {
		debug("[I2C] Formatting addresses...\n");
		if (flags & I2CF_ENABLE_SECONDARY) {
			/* Length of msg + dest addr */
			txbd->length = size + 2;

			txbd->addr[0] = address << 1;
			txbd->addr[1] = secondary_address;
			i = 2;
		} else {
			/* Length of msg + dest addr */
			txbd->length = size + 1;
			/* Write dest addr to BD */
			txbd->addr[0] = address << 1;
			i = 1;
		}
	} else {
		txbd->length = size;	/* Length of message */
		i = 0;
	}

	/* set up txbd */
	txbd->status = BD_SC_READY;
	if (flags & I2CF_START_COND)
		txbd->status |= BD_I2C_TX_START;
	if (flags & I2CF_STOP_COND)
		txbd->status |= BD_SC_LAST | BD_SC_WRAP;

	/* Copy data to send into buffer */
	debug("[I2C] copy data...\n");
	for(j = 0; j < size; i++, j++)
		txbd->addr[i] = dataout[j];

	debug("[I2C] txbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		txbd->length,
		txbd->status,
		txbd->addr[0],
		txbd->addr[1]);

	/* advance state */
	state->tx_buf += txbd->length;
	state->tx_space -= txbd->length;
	state->tx_idx++;
	state->txbd = (void *) (txbd + 1);

	return 0;
}

static int
i2c_receive(i2c_state_t *state,
	    unsigned char address,
	    unsigned char secondary_address,
	    unsigned int flags,
	    unsigned short size_to_expect, unsigned char *datain)
{
	volatile I2C_BD *rxbd, *txbd;

	debug("[I2C] i2c_receive %02d %02d %02d\n",
		address, secondary_address, flags);

	/* Expected to receive too much */
	if (size_to_expect > I2C_RXTX_LEN)
		return I2CERR_MSG_TOO_LONG;

	/* no more free bds */
	if (state->tx_idx >= NUM_TX_BDS || state->rx_idx >= NUM_RX_BDS
	    || state->tx_space < 2)
		return I2CERR_NO_BUFFERS;

	rxbd = (I2C_BD *) state->rxbd;
	txbd = (I2C_BD *) state->txbd;

	debug("[I2C] rxbd = %08x\n", (int)rxbd);
	debug("[I2C] txbd = %08x\n", (int)txbd);

	txbd->addr = state->tx_buf;

	/* set up TXBD for destination address */
	if (flags & I2CF_ENABLE_SECONDARY) {
		txbd->length = 2;
		txbd->addr[0] = address << 1;	/* Write data */
		txbd->addr[1] = secondary_address;	/* Internal address */
		txbd->status = BD_SC_READY;
	} else {
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
	if (flags & I2CF_STOP_COND) {
		txbd->status |= BD_SC_LAST | BD_SC_WRAP;
		rxbd->status |= BD_SC_WRAP;
	}

	debug("[I2C] txbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		txbd->length,
		txbd->status,
		txbd->addr[0],
		txbd->addr[1]);
	debug("[I2C] rxbd: length=0x%04x status=0x%04x addr[0]=0x%02x addr[1]=0x%02x\n",
		rxbd->length,
		rxbd->status,
		rxbd->addr[0],
		rxbd->addr[1]);

	/* advance state */
	state->tx_buf += txbd->length;
	state->tx_space -= txbd->length;
	state->tx_idx++;
	state->txbd = (void *) (txbd + 1);
	state->rx_idx++;
	state->rxbd = (void *) (rxbd + 1);

	return 0;
}


static int i2c_doio(i2c_state_t *state)
{
	volatile immap_t *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile cpm8xx_t *cp = (cpm8xx_t *)&immap->im_cpm;
	volatile i2c8xx_t *i2c = (i2c8xx_t *)&immap->im_i2c;
	volatile iic_t *iip = (iic_t *)&cp->cp_dparam[PROFF_IIC];
	volatile I2C_BD *txbd, *rxbd;
	volatile int j = 0;

	debug("[I2C] i2c_doio\n");

#ifdef CONFIG_SYS_I2C_UCODE_PATCH
	iip = (iic_t *)&cp->cp_dpmem[iip->iic_rpbase];
#endif

	if (state->tx_idx <= 0 && state->rx_idx <= 0) {
		debug("[I2C] No I/O is queued\n");
		return I2CERR_QUEUE_EMPTY;
	}

	iip->iic_rbptr = iip->iic_rbase;
	iip->iic_tbptr = iip->iic_tbase;

	/* Enable I2C */
	debug("[I2C] Enabling I2C...\n");
	i2c->i2c_i2mod |= 0x01;

	/* Begin transmission */
	i2c->i2c_i2com |= 0x80;

	/* Loop until transmit & receive completed */

	if (state->tx_idx > 0) {
		txbd = ((I2C_BD*)state->txbd) - 1;

		debug("[I2C] Transmitting...(txbd=0x%08lx)\n",
			(ulong)txbd);

		while ((txbd->status & BD_SC_READY) && (j++ < TOUT_LOOP)) {
			if (ctrlc())
				return (-1);

			__asm__ __volatile__("eieio");
		}
	}

	if ((state->rx_idx > 0) && (j < TOUT_LOOP)) {
		rxbd = ((I2C_BD*)state->rxbd) - 1;

		debug("[I2C] Receiving...(rxbd=0x%08lx)\n",
			(ulong)rxbd);

		while ((rxbd->status & BD_SC_EMPTY) && (j++ < TOUT_LOOP)) {
			if (ctrlc())
				return (-1);

			__asm__ __volatile__("eieio");
		}
	}

	/* Turn off I2C */
	i2c->i2c_i2mod &= ~0x01;

	if (state->err_cb != NULL) {
		int n, i, b;

		/*
		 * if we have an error callback function, look at the
		 * error bits in the bd status and pass them back
		 */

		if ((n = state->tx_idx) > 0) {
			for (i = 0; i < n; i++) {
				txbd = ((I2C_BD *) state->txbd) - (n - i);
				if ((b = txbd->status & BD_I2C_TX_ERR) != 0)
					(*state->err_cb) (I2CECB_TX_ERR | b,
							  i);
			}
		}

		if ((n = state->rx_idx) > 0) {
			for (i = 0; i < n; i++) {
				rxbd = ((I2C_BD *) state->rxbd) - (n - i);
				if ((b = rxbd->status & BD_I2C_RX_ERR) != 0)
					(*state->err_cb) (I2CECB_RX_ERR | b,
							  i);
			}
		}

		if (j >= TOUT_LOOP)
			(*state->err_cb) (I2CECB_TIMEOUT, 0);
	}

	return (j >= TOUT_LOOP) ? I2CERR_TIMEOUT : 0;
}

static int had_tx_nak;

static void i2c_test_callback(int flags, int xnum)
{
	if ((flags & I2CECB_TX_ERR) && (flags & I2CECB_TX_NAK))
		had_tx_nak = 1;
}

int i2c_probe(uchar chip)
{
	i2c_state_t state;
	int rc;
	uchar buf[1];

	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);

	i2c_newio(&state);

	state.err_cb = i2c_test_callback;
	had_tx_nak = 0;

	rc = i2c_receive(&state, chip, 0, I2CF_START_COND | I2CF_STOP_COND, 1,
			 buf);

	if (rc != 0)
		return (rc);

	rc = i2c_doio(&state);

	if ((rc != 0) && (rc != I2CERR_TIMEOUT))
		return (rc);

	return (had_tx_nak);
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	i2c_state_t state;
	uchar xaddr[4];
	int rc;

#ifdef CONFIG_LWMON
	WATCHDOG_RESET();
#endif

	xaddr[0] = (addr >> 24) & 0xFF;
	xaddr[1] = (addr >> 16) & 0xFF;
	xaddr[2] = (addr >> 8) & 0xFF;
	xaddr[3] = addr & 0xFF;

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones like
	 * Catalyst 24WC04/08/16 which has 9/10/11 bits of address and the
	 * extra bits end up in the "chip address" bit slots.  This makes
	 * a 24WC08 (1Kbyte) chip look like four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to still
	 * be one byte because the extra address bits are hidden in the
	 * chip address.
	 */
	chip |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	i2c_newio(&state);

	rc = i2c_send(&state, chip, 0, I2CF_START_COND, alen,
		      &xaddr[4 - alen]);
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

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	i2c_state_t state;
	uchar xaddr[4];
	int rc;

	xaddr[0] = (addr >> 24) & 0xFF;
	xaddr[1] = (addr >> 16) & 0xFF;
	xaddr[2] = (addr >> 8) & 0xFF;
	xaddr[3] = addr & 0xFF;

#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones like
	 * Catalyst 24WC04/08/16 which has 9/10/11 bits of address and the
	 * extra bits end up in the "chip address" bit slots.  This makes
	 * a 24WC08 (1Kbyte) chip look like four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to still
	 * be one byte because the extra address bits are hidden in the
	 * chip address.
	 */
	chip |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	i2c_newio(&state);

	rc = i2c_send(&state, chip, 0, I2CF_START_COND, alen,
		      &xaddr[4 - alen]);
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

#endif /* CONFIG_HARD_I2C */
