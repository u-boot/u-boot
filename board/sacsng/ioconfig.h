/*
 * I/O Port configuration table
 *
 * If conf is 1, then that port pin will be configured at boot time
 * according to the five values podr/pdir/ppar/psor/pdat for that entry
 */

#ifdef SKIP
#undef SKIP
#endif

#ifdef CONF
#undef CONF
#endif

#ifdef DIN
#undef DIN
#endif

#ifdef DOUT
#undef DOUT
#endif

#ifdef GPIO
#undef GPIO
#endif

#ifdef SPEC
#undef SPEC
#endif

#ifdef ACTV
#undef ACTV
#endif

#ifdef OPEN
#undef OPEN
#endif

#define SKIP 0  /* SKIP over this port */
#define CONF 1  /* CONFiguration the port */

#define DIN  0  /* PDIRx 0: Direction IN  */
#define DOUT 1  /* PDIRx 1: Direction OUT */

#define GPIO 0  /* PPARx 0: General Purpose I/O */
#define SPEC 1  /* PPARx 1: dedicated to a peripheral function, */
		/*          i.e. the port has a SPECial use. */

#define ACTV 0  /* PODRx 0: ACTiVely driven as an output */
#define OPEN 1  /* PODRx 1: OPEN-drain driver */

const iop_conf_t iop_conf_tab[4][32] = {

    /* Port A configuration */
    {	/*	     conf  ppar  psor  pdir  podr  pdat */
	/* PA31 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS8*        */
	/* PA30 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS7*        */
	/* PA29 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS6*        */
	/* PA28 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS5*        */
	/* PA27 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS4*        */
	/* PA26 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS3*        */
	/* PA25 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS2*        */
	/* PA24 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* RODIS1*        */
	/* PA23 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* ODIS_EN*       */
	/* PA22 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* STLED2_EN*     */
	/* PA21 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* STLED1_EN*     */
	/* PA20 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* PLED3_EN*      */
	/* PA19 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* PLED2_EN*      */
	/* PA18 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* PLED1_EN*      */
	/* PA17 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PA16 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* DAC_RST*       */
	/* PA15 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* CH34SDATA_PU   */
	/* PA14 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* CH12SDATA_PU   */
	/* PA13 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* SLRCLK_EN*     */
	/* PA12 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_4ACDC*    */
	/* PA11 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_4TEDS*    */
	/* PA10 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_4XTDS*    */
	/* PA9  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_3ACDC*    */
	/* PA8  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_3TEDS*    */
	/* PA7  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_3XTDS*    */
	/* PA6  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_2ACDC*    */
	/* PA5  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_2TEDS*    */
	/* PA4  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_2XTDS*    */
	/* PA3  */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PA2  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_1ACDC*    */
	/* PA1  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* MTRX_1TEDS*    */
	/* PA0  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }  /* MTRX_1XTDS*    */
    },

    /* Port B configuration */
    {	/*	     conf  ppar  psor  pdir  podr  pdat */
	/* PB31 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* FCC2 MII_TX_ER */
	/* PB30 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_RX_DV */
	/* PB29 */ { CONF, SPEC,   1,  DOUT, ACTV,   0   }, /* FCC2 MII_TX_EN */
	/* PB28 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_RX_ER */
	/* PB27 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_COL   */
	/* PB26 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_CRS   */
	/* PB25 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* FCC2 MII_TXD3  */
	/* PB24 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* FCC2 MII_TXD2  */
	/* PB23 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* FCC2 MII_TXD1  */
	/* PB22 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* FCC2 MII_TXD0  */
	/* PB21 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_RXD0  */
	/* PB20 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_RXD1  */
	/* PB19 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_RXD2  */
	/* PB18 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* FCC2 MII_RXD3  */
	/* PB17 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PB16 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PB15 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PB14 */ { CONF, SPEC,   1,  DIN,  ACTV,   0   }, /* L1RXDC1,   BSDATA_ADC12 */
	/* PB13 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PB12 */ { CONF, SPEC,   1,  DIN,  ACTV,   0   }, /* L1RSYNCC1, LRCLK  */
	/* PB11 */ { CONF, SPEC,   1,  DIN,  ACTV,   0   }, /* L1TXDD1,   RSDATA_DAC12 */
	/* PB10 */ { CONF, SPEC,   1,  DIN,  ACTV,   0   }, /* L1RXDD1,   BSDATA_ADC34 */
	/* PB9  */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PB8  */ { CONF, SPEC,   1,  DIN,  ACTV,   0   }, /* L1RSYNCD1, LRCLK  */
	/* PB7  */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PB6  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* XCITE_SHDN     */
	/* PB5  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* TRIGGER        */
	/* PB4  */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* ARM            */
	/* PB3  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }, /* pin doesn't exist */
	/* PB2  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }, /* pin doesn't exist */
	/* PB1  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }, /* pin doesn't exist */
	/* PB0  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }  /* pin doesn't exist */
    },

    /* Port C */
    {	/*	      conf ppar  psor  pdir  podr  pdat */
	/* PC31 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC30 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC29 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* CLK3,  MCLK    */
	/* PC28 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* TOUT2*         */
#ifdef QQQ
	/* PC28 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* TOUT2*         */
#endif
	/* PC27 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* CLK5,  SCLK    */
	/* PC26 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC25 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* CLK7,  SCLK    */
	/* PC24 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC23 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* CLK9,  MCLK    */
	/* PC22 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC21 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* BRGO6 (LRCLK)  */
	/* PC20 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC19 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* CLK13, MII_RXCLK  */
	/* PC18 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* CLK14, MII_TXCLK  */
	/* PC17 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* BRGO8 (SCLK)   */
	/* PC16 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC15 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* SMC2_TX        */
	/* PC14 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC13 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC12 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* TDM_STRB3      */
	/* PC11 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC10 */ { CONF, SPEC,   1,  DOUT, ACTV,   0   }, /* TDM_STRB4      */
	/* PC9  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* BPDIS_IN3      */
	/* PC8  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* BPDIS_IN2      */
	/* PC7  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* BPDIS_IN1      */
	/* PC6  */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PC5  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* BTST_IN2*      */
	/* PC4  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* BTST_IN1*      */
	/* PC3  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* MUSH_STAT      */
	/* PC2  */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* OUTDRV_STAT    */
	/* PC1  */ { CONF, GPIO,   0,  DOUT, OPEN,   1   }, /* PHY_MDIO       */
	/* PC0  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* PHY_MDC        */
    },

    /* Port D */
    {	/*	      conf ppar  psor  pdir  podr  pdat */
	/* PD31 */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* SCC1_RX        */
	/* PD30 */ { CONF, SPEC,   1,  DOUT, ACTV,   0   }, /* SCC1_TX        */
	/* PD29 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD28 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD27 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD26 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD25 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD24 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD23 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD22 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD21 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD20 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* SPI_ADC_CS*    */
	/* PD19 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* SPI_DAC_CS*    */
#if defined(CONFIG_SOFT_SPI)
	/* PD18 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* SPI_CLK        */
	/* PD17 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* SPI_MOSI       */
	/* PD16 */ { CONF, GPIO,   0,  DIN,  ACTV,   0   }, /* SPI_MISO       */
#else
	/* PD18 */ { CONF, SPEC,   1,  DOUT, ACTV,   0   }, /* SPI_CLK        */
	/* PD17 */ { CONF, SPEC,   1,  DOUT, ACTV,   0   }, /* SPI_MOSI       */
	/* PD16 */ { CONF, SPEC,   1,  DIN,  ACTV,   0   }, /* SPI_MISO       */
#endif
#if defined(CONFIG_SOFT_I2C)
	/* PD15 */ { CONF, GPIO,   0,  DOUT, OPEN,   1   }, /* I2C_SDA        */
	/* PD14 */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* I2C_SCL        */
#else
#if defined(CONFIG_HARD_I2C)
	/* PD15 */ { CONF, SPEC,   1,  DIN,  OPEN,   0   }, /* I2C_SDA        */
	/* PD14 */ { CONF, SPEC,   1,  DIN,  OPEN,   0   }, /* I2C_SCL        */
#else /* normal I/O port pins */
	/* PD15 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* I2C_SDA        */
	/* PD14 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* I2C_SCL        */
#endif
#endif
	/* PD13 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* TDM_STRB1      */
	/* PD12 */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* TDM_STRB2      */
	/* PD11 */ { CONF, GPIO,   0,  DOUT, ACTV,   0   }, /* N/C            */
	/* PD10 */ { CONF, SPEC,   1,  DOUT, ACTV,   0   }, /* BRGO4 (MCLK)   */
	/* PD9  */ { CONF, SPEC,   0,  DOUT, ACTV,   0   }, /* SMC1_TX        */
	/* PD8  */ { CONF, SPEC,   0,  DIN,  ACTV,   0   }, /* SMC1_RX        */
	/* PD7  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* N/C            */
	/* PD6  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* N/C            */
	/* PD5  */ { CONF, GPIO,   0,  DOUT, ACTV,   1   }, /* N/C            */
	/* PD4  */ { CONF, SPEC,   1,  DOUT, ACTV,   1   }, /* SMC2_RX        */
	/* PD3  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }, /* pin doesn't exist */
	/* PD2  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }, /* pin doesn't exist */
	/* PD1  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }, /* pin doesn't exist */
	/* PD0  */ { SKIP, GPIO,   0,  DIN,  ACTV,   0   }  /* pin doesn't exist */
    }
};
