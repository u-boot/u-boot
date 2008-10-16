/**************************************************
 *
 * copyright @ motorola, 1999
 *
 *************************************************/
#include <mpc824x.h>
#include <common.h>
#include "epic.h"


#define PRINT(format, args...) printf(format , ## args)

typedef void (*VOIDFUNCPTR)  (void);  /* ptr to function returning void */
struct SrcVecTable SrcVecTable[MAXVEC] = /* Addr/Vector cross-reference tbl */
    {
    { EPIC_EX_INT0_VEC_REG,  "External Direct/Serial Source 0"},
    { EPIC_EX_INT1_VEC_REG,  "External Direct/Serial Source 1"},
    { EPIC_EX_INT2_VEC_REG,  "External Direct/Serial Source 2"},
    { EPIC_EX_INT3_VEC_REG,  "External Direct/Serial Source 3"},
    { EPIC_EX_INT4_VEC_REG,  "External Direct/Serial Source 4"},

    { EPIC_SR_INT5_VEC_REG,  "External Serial Source 5"},
    { EPIC_SR_INT6_VEC_REG,  "External Serial Source 6"},
    { EPIC_SR_INT7_VEC_REG,  "External Serial Source 7"},
    { EPIC_SR_INT8_VEC_REG,  "External Serial Source 8"},
    { EPIC_SR_INT9_VEC_REG,  "External Serial Source 9"},
    { EPIC_SR_INT10_VEC_REG, "External Serial Source 10"},
    { EPIC_SR_INT11_VEC_REG, "External Serial Source 11"},
    { EPIC_SR_INT12_VEC_REG, "External Serial Source 12"},
    { EPIC_SR_INT13_VEC_REG, "External Serial Source 13"},
    { EPIC_SR_INT14_VEC_REG, "External Serial Source 14"},
    { EPIC_SR_INT15_VEC_REG, "External Serial Source 15"},

    { EPIC_I2C_INT_VEC_REG,  "Internal I2C Source"},
    { EPIC_DMA0_INT_VEC_REG, "Internal DMA0 Source"},
    { EPIC_DMA1_INT_VEC_REG, "Internal DMA1 Source"},
    { EPIC_MSG_INT_VEC_REG,  "Internal Message Source"},
    };

VOIDFUNCPTR intVecTbl[MAXVEC];    /* Interrupt vector table */


/****************************************************************************
*  epicInit - Initialize the EPIC registers
*
*  This routine resets the Global Configuration Register, thus it:
*     -  Disables all interrupts
*     -  Sets epic registers to reset values
*     -  Sets the value of the Processor Current Task Priority to the
*        highest priority (0xF).
*  epicInit then sets the EPIC operation mode to Mixed Mode (vs. Pass
*  Through or 8259 compatible mode).
*
*  If IRQType (input) is Direct IRQs:
*     - IRQType is written to the SIE bit of the EPIC Interrupt
*       Configuration register (ICR).
*     - clkRatio is ignored.
*  If IRQType is Serial IRQs:
*     - both IRQType and clkRatio will be written to the ICR register
*/

void epicInit
    (
    unsigned int IRQType,      /* Direct or Serial */
    unsigned int clkRatio      /* Clk Ratio for Serial IRQs */
    )
    {
    ULONG tmp;

    tmp = sysEUMBBARRead(EPIC_GLOBAL_REG);
    tmp |= 0xa0000000;                  /* Set the Global Conf. register */
    sysEUMBBARWrite(EPIC_GLOBAL_REG, tmp);
	/*
	 * Wait for EPIC to reset - CLH
	 */
    while( (sysEUMBBARRead(EPIC_GLOBAL_REG) & 0x80000000) == 1);
    sysEUMBBARWrite(EPIC_GLOBAL_REG, 0x20000000);
    tmp = sysEUMBBARRead(EPIC_INT_CONF_REG);    /* Read interrupt conf. reg */

    if (IRQType == EPIC_DIRECT_IRQ)             /* direct mode */
	sysEUMBBARWrite(EPIC_INT_CONF_REG, tmp & 0xf7ffffff);
    else                                        /* Serial mode */
	{
	tmp = (clkRatio << 28) | 0x08000000;    /* Set clock ratio */
	sysEUMBBARWrite(EPIC_INT_CONF_REG, tmp);
	}

    while (epicIntAck() != 0xff)       /* Clear all pending interrupts */
		epicEOI();
}

/****************************************************************************
 *  epicIntEnable - Enable an interrupt source
 *
 *  This routine clears the mask bit of an external, an internal or
 *  a Timer register to enable the interrupt.
 *
 *  RETURNS:  None
 */
void epicIntEnable(int intVec)
{
    ULONG tmp;
    ULONG srAddr;

    srAddr = SrcVecTable[intVec].srcAddr;  /* Retrieve src Vec/Prio register */
    tmp = sysEUMBBARRead(srAddr);
    tmp &= ~EPIC_VEC_PRI_MASK;             /* Clear the mask bit */
    tmp |= (EPIC_VEC_PRI_DFLT_PRI << 16);   /* Set priority to Default - CLH */
    tmp |= intVec;				           /* Set Vector number */
    sysEUMBBARWrite(srAddr, tmp);

    return;
    }

/****************************************************************************
 *  epicIntDisable - Disable an interrupt source
 *
 *  This routine sets the mask bit of an external, an internal or
 *  a Timer register to disable the interrupt.
 *
 *  RETURNS:  OK or ERROR
 *
 */

void epicIntDisable
    (
    int intVec        /* Interrupt vector number */
    )
    {

    ULONG tmp, srAddr;

    srAddr = SrcVecTable[intVec].srcAddr;
    tmp = sysEUMBBARRead(srAddr);
    tmp |= 0x80000000;                      /* Set the mask bit */
    sysEUMBBARWrite(srAddr, tmp);
    return;
    }

/****************************************************************************
 * epicIntSourceConfig - Set properties of an interrupt source
 *
 * This function sets interrupt properites (Polarity, Sense, Interrupt
 * Prority, and Interrupt Vector) of an Interrupt Source.  The properties
 * can be set when the current source is not in-request or in-service,
 * which is determined by the Activity bit.  This routine return ERROR
 * if the the Activity bit is 1 (in-request or in-service).
 *
 * This function assumes that the Source Vector/Priority register (input)
 * is a valid address.
 *
 * RETURNS:  OK or ERROR
 */

int epicIntSourceConfig
    (
    int   Vect,                         /* interrupt source vector number */
    int   Polarity,                     /* interrupt source polarity */
    int   Sense,                        /* interrupt source Sense */
    int   Prio                          /* interrupt source priority */
    )

    {
    ULONG tmp, newVal;
    ULONG actBit, srAddr;

    srAddr = SrcVecTable[Vect].srcAddr;
    tmp = sysEUMBBARRead(srAddr);
    actBit = (tmp & 40000000) >> 30;    /* retrieve activity bit - bit 30 */
    if (actBit == 1)
	return ERROR;

    tmp &= 0xff30ff00;     /* Erase previously set P,S,Prio,Vector bits */
    newVal = (Polarity << 23) | (Sense << 22) | (Prio << 16) | Vect;
    sysEUMBBARWrite(srAddr, tmp | newVal );
    return (OK);
    }

/****************************************************************************
 * epicIntAck - acknowledge an interrupt
 *
 * This function reads the Interrupt acknowldge register and return
 * the vector number of the highest pending interrupt.
 *
 * RETURNS: Interrupt Vector number.
 */

unsigned int epicIntAck(void)
{
    return(sysEUMBBARRead( EPIC_PROC_INT_ACK_REG ));
}

/****************************************************************************
 * epicEOI - signal an end of interrupt
 *
 * This function writes 0x0 to the EOI register to signal end of interrupt.
 * It is usually called after an interrupt routine is served.
 *
 * RETURNS: None
 */

void epicEOI(void)
    {
    sysEUMBBARWrite(EPIC_PROC_EOI_REG, 0x0);
    }

/****************************************************************************
 *  epicCurTaskPrioSet - sets the priority of the Processor Current Task
 *
 *  This function should be called after epicInit() to lower the priority
 *  of the processor current task.
 *
 *  RETURNS:  OK or ERROR
 */

int epicCurTaskPrioSet
    (
    int prioNum                 /* New priority value */
    )
    {

    if ( (prioNum < 0) || (prioNum > 0xF))
	return ERROR;
    sysEUMBBARWrite(EPIC_PROC_CTASK_PRI_REG, prioNum);
    return OK;
    }


/************************************************************************
 * function: epicIntTaskGet
 *
 * description: Get value of processor current interrupt task priority register
 *
 * note:
 ***********************************************************************/
unsigned char epicIntTaskGet()
{
  /* get the interrupt task priority register */
    ULONG reg;
    unsigned char rec;

    reg = sysEUMBBARRead( EPIC_PROC_CTASK_PRI_REG );
    rec = ( reg & 0x0F );
    return rec;
}


/**************************************************************
 * function: epicISR
 *
 * description: EPIC service routine called by the core exception
 *              at 0x500
 *
 * note:
 **************************************************************/
unsigned int epicISR(void)
{
   return 0;
}


/************************************************************
 * function: epicModeGet
 *
 * description: query EPIC mode, return 0 if pass through mode
 *                               return 1 if mixed mode
 *
 * note:
 *************************************************************/
unsigned int epicModeGet(void)
{
    ULONG val;

    val = sysEUMBBARRead( EPIC_GLOBAL_REG );
    return (( val & 0x20000000 ) >> 29);
}


/*********************************************
 * function: epicConfigGet
 *
 * description: Get the EPIC interrupt Configuration
 *              return 0 if not error, otherwise return 1
 *
 * note:
 ********************************************/
void epicConfigGet( unsigned int *clkRatio, unsigned int *serEnable)
{
    ULONG val;

    val = sysEUMBBARRead( EPIC_INT_CONF_REG );
    *clkRatio = ( val & 0x70000000 ) >> 28;
    *serEnable = ( val & 0x8000000 ) >> 27;
}


/*******************************************************************
 *  sysEUMBBARRead - Read a 32-bit EUMBBAR register
 *
 *  This routine reads the content of a register in the Embedded
 *  Utilities Memory Block, and swaps to big endian before returning
 *  the value.
 *
 *  RETURNS:  The content of the specified EUMBBAR register.
 */

ULONG sysEUMBBARRead
    (
    ULONG regNum
    )
    {
    ULONG temp;

    temp = *(ULONG *) (CONFIG_SYS_EUMB_ADDR + regNum);
    return ( LONGSWAP(temp));
    }

/*******************************************************************
 *  sysEUMBBARWrite - Write a 32-bit EUMBBAR register
 *
 *  This routine swaps the value to little endian then writes it to
 *  a register in the Embedded Utilities Memory Block address space.
 *
 *  RETURNS: N/A
 */

void sysEUMBBARWrite
    (
    ULONG regNum,               /* EUMBBAR register address */
    ULONG regVal                /* Value to be written */
    )
    {

    *(ULONG *) (CONFIG_SYS_EUMB_ADDR + regNum) = LONGSWAP(regVal);
    return ;
    }


/********************************************************
 * function: epicVendorId
 *
 * description: return the EPIC Vendor Identification
 *              register:
 *
 *              siliccon version, device id, and vendor id
 *
 * note:
 ********************************************************/
void epicVendorId
   (
    unsigned int *step,
    unsigned int *devId,
    unsigned int *venId
   )
   {
    ULONG val;
    val = sysEUMBBARRead( EPIC_VENDOR_ID_REG );
    *step  = ( val & 0x00FF0000 ) >> 16;
    *devId = ( val & 0x0000FF00 ) >> 8;
    *venId = ( val & 0x000000FF );
    }

/**************************************************
 * function: epicFeatures
 *
 * description: return the number of IRQ supported,
 *              number of CPU, and the version of the
 *              OpenEPIC
 *
 * note:
 *************************************************/
void epicFeatures
    (
    unsigned int *noIRQs,
    unsigned int *noCPUs,
    unsigned int *verId
    )
    {
    ULONG val;

    val = sysEUMBBARRead( EPIC_FEATURES_REG );
    *noIRQs  = ( val & 0x07FF0000 ) >> 16;
    *noCPUs  = ( val & 0x00001F00 ) >> 8;
    *verId   = ( val & 0x000000FF );
}


/*********************************************************
 * function: epciTmFrequncySet
 *
 * description: Set the timer frequency reporting register
 ********************************************************/
void epicTmFrequencySet( unsigned int frq )
{
    sysEUMBBARWrite(EPIC_TM_FREQ_REG, frq);
}

/*******************************************************
 * function: epicTmFrequncyGet
 *
 * description: Get the current value of the Timer Frequency
 * Reporting register
 *
 ******************************************************/
unsigned int epicTmFrequencyGet(void)
{
    return( sysEUMBBARRead(EPIC_TM_FREQ_REG)) ;
}


/****************************************************
 * function: epicTmBaseSet
 *
 * description: Set the #n global timer base count register
 *              return 0 if no error, otherwise return 1.
 *
 * note:
 ****************************************************/
unsigned int epicTmBaseSet
    (
    ULONG srcAddr,         /* Address of the Timer Base register */
    unsigned int cnt,    /* Base count */
    unsigned int inhibit   /* 1 - count inhibit */
    )
{

    unsigned int val = 0x80000000;
    /* First inhibit counting the timer */
    sysEUMBBARWrite(srcAddr, val) ;

    /* set the new value */
    val = (cnt & 0x7fffffff) | ((inhibit & 0x1) << 31);
    sysEUMBBARWrite(srcAddr, val) ;
    return 0;
}

/***********************************************************************
 * function: epicTmBaseGet
 *
 * description: Get the current value of the global timer base count register
 *              return 0 if no error, otherwise return 1.
 *
 * note:
 ***********************************************************************/
unsigned int epicTmBaseGet( ULONG srcAddr, unsigned int *val )
{
    *val = sysEUMBBARRead( srcAddr );
    *val = *val & 0x7fffffff;
    return 0;
}

/***********************************************************
 * function: epicTmCountGet
 *
 * description: Get the value of a given global timer
 *              current count register
 *              return 0 if no error, otherwise return 1
 * note:
 **********************************************************/
unsigned int epicTmCountGet( ULONG srcAddr, unsigned int *val )
{
    *val = sysEUMBBARRead( srcAddr );
    *val = *val & 0x7fffffff;
    return 0;
}


/***********************************************************
 * function: epicTmInhibit
 *
 * description: Stop counting of a given global timer
 *              return 0 if no error, otherwise return 1
 *
 * note:
 ***********************************************************/
unsigned int epicTmInhibit( unsigned int srcAddr )
{
    ULONG val;

    val = sysEUMBBARRead( srcAddr );
    val |= 0x80000000;
    sysEUMBBARWrite( srcAddr, val );
    return 0;
}

/******************************************************************
 * function: epicTmEnable
 *
 * description: Enable counting of a given global timer
 *              return 0 if no error, otherwise return 1
 *
 * note:
 *****************************************************************/
unsigned int epicTmEnable( ULONG srcAddr )
{
    ULONG val;

    val = sysEUMBBARRead( srcAddr );
    val &= 0x7fffffff;
    sysEUMBBARWrite( srcAddr, val );
    return 0;
}

void epicSourcePrint(int Vect)
    {
    ULONG srcVal;

    srcVal = sysEUMBBARRead(SrcVecTable[Vect].srcAddr);
    PRINT("%s\n", SrcVecTable[Vect].srcName);
    PRINT("Address   = 0x%lx\n", SrcVecTable[Vect].srcAddr);
    PRINT("Vector    = %ld\n", (srcVal & 0x000000FF) );
    PRINT("Mask      = %ld\n", srcVal >> 31);
    PRINT("Activitiy = %ld\n", (srcVal & 40000000) >> 30);
    PRINT("Polarity  = %ld\n", (srcVal & 0x00800000) >> 23);
    PRINT("Sense     = %ld\n", (srcVal & 0x00400000) >> 22);
    PRINT("Priority  = %ld\n", (srcVal & 0x000F0000) >> 16);
    }
