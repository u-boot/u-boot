/* Module for handling DALLAS DS2438, smart battery monitor
   Chip can store up to 40 bytes of user data in EEPROM,
   perform temp, voltage and current measurements.
   Chip also contains a unique serial number.

   Always read/write LSb first

   For documentaion, see data sheet for DS2438, 2438.pdf

   By Thomas.Lange@corelatus.com 001025 */

#include <common.h>
#include <config.h>
#include <mpc8xx.h>

#include <../board/gth/ee_dev.h>

/* We dont have kernel functions */
#define printk printf
#define KERN_DEBUG
#define KERN_ERR
#define EIO 1

static int Debug = 0;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*
 * lookup table ripped from DS app note 17, understanding and using
 * cyclic redundancy checks...
 */

static u8 crc_lookup[256] = {
	0,	94,	188,	226,	97,	63,	221,	131,
	194,	156,	126,	32,	163,	253,	31,	65,
	157,	195,	33,	127,	252,	162,	64,	30,
	95,	1,	227,	189,	62,	96,	130,	220,
	35,	125,	159,	193,	66,	28,	254,	160,
	225,	191,	93,	3,	128,	222,	60,	98,
	190,	224,	2,	92,	223,	129,	99,	61,
	124,	34,	192,	158,	29,	67,	161,	255,
	70,	24,	250,	164,	39,	121,	155,	197,
	132,	218,	56,	102,	229,	187,	89,	7,
	219,	133,	103,	57,	186,	228,	6,	88,
	25,	71,	165,	251,	120,	38,	196,	154,
	101,	59,	217,	135,	4,	90,	184,	230,
	167,	249,	27,	69,	198,	152,	122,	36,
	248,	166,	68,	26,	153,	199,	37,	123,
	58,	100,	134,	216,	91,	5,	231,	185,
	140,	210,	48,	110,	237,	179,	81,	15,
	78,	16,	242,	172,	47,	113,	147,	205,
	17,	79,	173,	243,	112,	46,	204,	146,
	211,	141,	111,	49,	178,	236,	14,	80,
	175,	241,	19,	77,	206,	144,	114,	44,
	109,	51,	209,	143,	12,	82,	176,	238,
	50,	108,	142,	208,	83,	13,	239,	177,
	240,	174,	76,	18,	145,	207,	45,	115,
	202,	148,	118,	40,	171,	245,	23,	73,
	8,	86,	180,	234,	105,	55,	213,	139,
	87,	9,	235,	181,	54,	104,	138,	212,
	149,	203,	41,	119,	244,	170,	72,	22,
	233,	183,	85,	11,	136,	214,	52,	106,
	43,	117,	151,	201,	74,	20,	246,	168,
	116,	42,	200,	150,	21,	75,	169,	247,
	182,	232,	10,	84,	215,	137,	107,	53
};

static u8 make_new_crc( u8 Old_crc, u8 New_value ){
  /* Compute a new checksum with new byte, using previous checksum as input
     See DS app note 17, understanding and using cyclic redundancy checks...
     Also see DS2438, page 11 */
  return( crc_lookup[Old_crc ^ New_value ]);
}

int ee_crc_ok( u8 *Buffer, int Len, u8 Crc ){
  /* Check if the checksum for this buffer is correct */
  u8 Curr_crc=0;
  int i;
  u8 *Curr_byte = Buffer;

  for(i=0;i<Len;i++){
    Curr_crc = make_new_crc( Curr_crc, *Curr_byte);
    Curr_byte++;
  }
  E_DEBUG("Calculated CRC = 0x%x, read = 0x%x\n", Curr_crc, Crc);

  if(Curr_crc == Crc){
    /* Good */
    return(TRUE);
  }
  printk(KERN_ERR"EE checksum error, Calculated CRC = 0x%x, read = 0x%x\n",
	Curr_crc, Crc);
  return(FALSE);
}

static void
set_idle(void){
  /* Send idle and keep start time
     Continous 1 is idle */
  WRITE_PORT(1);
}

static int
do_reset(void){
  /* Release reset and verify that chip responds with presence pulse */
  int Retries = 0;
  while(Retries<5){
    udelay(RESET_LOW_TIME);

    /* Send reset */
    WRITE_PORT(0);
    udelay(RESET_LOW_TIME);

    /* Release reset */
    WRITE_PORT(1);

    /* Wait for EEPROM to drive output */
    udelay(PRESENCE_TIMEOUT);
    if(!READ_PORT){
      /* Ok, EEPROM is driving a 0 */
      E_DEBUG("Presence detected\n");
      if(Retries){
	E_DEBUG("Retries %d\n",Retries);
      }
      /* Make sure chip releases pin */
      udelay(PRESENCE_LOW_TIME);
      return 0;
    }
    Retries++;
  }

  printk(KERN_ERR"EEPROM did not respond when releasing reset\n");

    /* Make sure chip releases pin */
  udelay(PRESENCE_LOW_TIME);

  /* Set to idle again */
  set_idle();

  return(-EIO);
}

static u8
read_byte(void){
  /* Read a single byte from EEPROM
     Read LSb first */
  int i;
  int Value;
  u8 Result=0;
#ifndef CFG_IMMR
  u32 Flags;
#endif

  E_DEBUG("Reading byte\n");

  for(i=0;i<8;i++){
    /* Small delay between pulses */
    udelay(1);

#ifndef CFG_IMMR
    /* Disable irq */
    save_flags(Flags);
    cli();
#endif

    /* Pull down pin short time to start read
       See page 26 in data sheet */

    WRITE_PORT(0);
    udelay(READ_LOW);
    WRITE_PORT(1);

    /* Wait for chip to drive pin */
    udelay(READ_TIMEOUT);

    Value = READ_PORT;
    if(Value)
      Value=1;

#ifndef CFG_IMMR
    /* Enable irq */
    restore_flags(Flags);
#endif

    /* Wait for chip to release pin */
    udelay(TOTAL_READ_LOW-READ_TIMEOUT);

    /* LSb first */
    Result|=Value<<i;
  }

  E_DEBUG("Read byte 0x%x\n",Result);

  return(Result);
}

static void
write_byte(u8 Byte){
  /* Write a single byte to EEPROM
     Write LSb first */
  int i;
  int Value;
#ifndef CFG_IMMR
  u32 Flags;
#endif

  E_DEBUG("Writing byte 0x%x\n",Byte);

  for(i=0;i<8;i++){
    /* Small delay between pulses */
    udelay(1);
    Value = Byte&1;

#ifndef CFG_IMMR
    /* Disable irq */
    save_flags(Flags);
    cli();
#endif

    /* Pull down pin short time for a 1, long time for a 0
       See page 26 in data sheet */

    WRITE_PORT(0);
    if(Value){
      /* Write a 1 */
      udelay(WRITE_1_LOW);
    }
    else{
      /* Write a 0 */
      udelay(WRITE_0_LOW);
    }

    WRITE_PORT(1);

#ifndef CFG_IMMR
    /* Enable irq */
    restore_flags(Flags);
#endif

    if(Value)
      /* Wait for chip to read the 1 */
      udelay(TOTAL_WRITE_LOW-WRITE_1_LOW);
    Byte>>=1;
  }
}

int ee_do_command( u8 *Tx, int Tx_len, u8 *Rx, int Rx_len, int Send_skip ){
  /* Execute this command string, including
     giving reset and setting to idle after command
     if Rx_len is set, we read out data from EEPROM */
  int i;

  E_DEBUG("Command, Tx_len %d, Rx_len %d\n", Tx_len, Rx_len );

  if(do_reset()){
    /* Failed! */
    return(-EIO);
  }

  if(Send_skip)
    /* Always send SKIP_ROM first to tell chip we are sending a command,
       except when we read out rom data for chip */
    write_byte(SKIP_ROM);

  /* Always have Tx data */
  for(i=0;i<Tx_len;i++){
    write_byte(Tx[i]);
  }

  if(Rx_len){
    for(i=0;i<Rx_len;i++){
      Rx[i]=read_byte();
    }
  }

  set_idle();

  E_DEBUG("Command done\n");

  return(0);
}

int ee_init_data(void){
  int i;
  u8 Tx[10];
  int tmp;
  volatile immap_t *immap = (immap_t *)CFG_IMMR;

  while(0){
    tmp = 1-tmp;
    if(tmp)
      immap->im_ioport.iop_padat &= ~PA_FRONT_LED;
    else
      immap->im_ioport.iop_padat |= PA_FRONT_LED;
    udelay(1);
  }

  /* Set port to open drain to be able to read data from
     port without setting it to input */
  PORT_B_PAR &= ~PB_EEPROM;
  PORT_B_ODR |= PB_EEPROM;
  SET_PORT_B_OUTPUT(PB_EEPROM);

  /* Set idle mode */
  set_idle();

  /* Copy all User EEPROM data to scratchpad */
  for(i=0;i<USER_PAGES;i++){
    Tx[0]=RECALL_MEMORY;
    Tx[1]=EE_USER_PAGE_0+i;
    if(ee_do_command(Tx,2,NULL,0,TRUE)) return(-EIO);
  }

  /* Make sure chip doesnt store measurements in NVRAM */
  Tx[0]=WRITE_SCRATCHPAD;
  Tx[1]=0; /* Page */
  Tx[2]=9;
  if(ee_do_command(Tx,3,NULL,0,TRUE)) return(-EIO);

  Tx[0]=COPY_SCRATCHPAD;
  if(ee_do_command(Tx,2,NULL,0,TRUE)) return(-EIO);

  /* FIXME check status bit instead
     Could take 10 ms to store in EEPROM */
  for(i=0;i<10;i++){
    udelay(1000);
  }

  return(0);
}
