/*********************************************************************
 * mpc8240epic.h - EPIC module of the MPC8240 micro-controller
 *
 * Copyrigh 1999  Motorola Inc.
 *
 * Modification History:
 * =====================
 * 01a,04Feb99,My  Created.
 * 15Nov200, robt -modified to use in U-Boot
 *
*/

#ifndef __INCEPICh
#define __INCEPICh

#define ULONG unsigned long
#define MAXVEC             20
#define MAXIRQ        5 /* IRQs */
#define EPIC_DIRECT_IRQ     0 /* Direct interrupt type */

/* EPIC register addresses */

#define EPIC_EUMBBAR      0x40000     /* EUMBBAR of EPIC  */
#define EPIC_FEATURES_REG (EPIC_EUMBBAR + 0x01000)/* Feature reporting */
#define EPIC_GLOBAL_REG   (EPIC_EUMBBAR + 0x01020)/* Global config.  */
#define EPIC_INT_CONF_REG (EPIC_EUMBBAR + 0x01030)/* Interrupt config. */
#define EPIC_VENDOR_ID_REG  (EPIC_EUMBBAR + 0x01080)/* Vendor id */
#define EPIC_PROC_INIT_REG  (EPIC_EUMBBAR + 0x01090)/* Processor init. */
#define EPIC_SPUR_VEC_REG (EPIC_EUMBBAR + 0x010e0)/* Spurious vector */
#define EPIC_TM_FREQ_REG  (EPIC_EUMBBAR + 0x010f0)/* Timer Frequency */

#define EPIC_TM0_CUR_COUNT_REG  (EPIC_EUMBBAR + 0x01100)/* Gbl TM0 Cur. Count*/
#define EPIC_TM0_BASE_COUNT_REG (EPIC_EUMBBAR + 0x01110)/* Gbl TM0 Base Count*/
#define EPIC_TM0_VEC_REG  (EPIC_EUMBBAR + 0x01120)/* Gbl TM0 Vector Pri*/
#define EPIC_TM0_DES_REG  (EPIC_EUMBBAR + 0x01130)/* Gbl TM0 Dest. */

#define EPIC_TM1_CUR_COUNT_REG  (EPIC_EUMBBAR + 0x01140)/* Gbl TM1 Cur. Count*/
#define EPIC_TM1_BASE_COUNT_REG (EPIC_EUMBBAR + 0x01150)/* Gbl TM1 Base Count*/
#define EPIC_TM1_VEC_REG  (EPIC_EUMBBAR + 0x01160)/* Gbl TM1 Vector Pri*/
#define EPIC_TM1_DES_REG  (EPIC_EUMBBAR + 0x01170)/* Gbl TM1 Dest. */

#define EPIC_TM2_CUR_COUNT_REG  (EPIC_EUMBBAR + 0x01180)/* Gbl TM2 Cur. Count*/
#define EPIC_TM2_BASE_COUNT_REG (EPIC_EUMBBAR + 0x01190)/* Gbl TM2 Base Count*/
#define EPIC_TM2_VEC_REG  (EPIC_EUMBBAR + 0x011a0)/* Gbl TM2 Vector Pri*/
#define EPIC_TM2_DES_REG  (EPIC_EUMBBAR + 0x011b0)/* Gbl TM2 Dest */

#define EPIC_TM3_CUR_COUNT_REG  (EPIC_EUMBBAR + 0x011c0)/* Gbl TM3 Cur. Count*/
#define EPIC_TM3_BASE_COUNT_REG (EPIC_EUMBBAR + 0x011d0)/* Gbl TM3 Base Count*/
#define EPIC_TM3_VEC_REG  (EPIC_EUMBBAR + 0x011e0)/* Gbl TM3 Vector Pri*/
#define EPIC_TM3_DES_REG  (EPIC_EUMBBAR + 0x011f0)/* Gbl TM3 Dest. */

#define EPIC_EX_INT0_VEC_REG  (EPIC_EUMBBAR + 0x10200)/* Ext. Int. Sr0 Des */
#define EPIC_EX_INT0_DES_REG  (EPIC_EUMBBAR + 0x10210)/* Ext. Int. Sr0 Vect*/
#define EPIC_EX_INT1_VEC_REG  (EPIC_EUMBBAR + 0x10220)/* Ext. Int. Sr1 Des */
#define EPIC_EX_INT1_DES_REG  (EPIC_EUMBBAR + 0x10230)/* Ext. Int. Sr1 Vect*/
#define EPIC_EX_INT2_VEC_REG  (EPIC_EUMBBAR + 0x10240)/* Ext. Int. Sr2 Des */
#define EPIC_EX_INT2_DES_REG  (EPIC_EUMBBAR + 0x10250)/* Ext. Int. Sr2 Vect*/
#define EPIC_EX_INT3_VEC_REG  (EPIC_EUMBBAR + 0x10260)/* Ext. Int. Sr3 Des */
#define EPIC_EX_INT3_DES_REG  (EPIC_EUMBBAR + 0x10270)/* Ext. Int. Sr3 Vect*/
#define EPIC_EX_INT4_VEC_REG  (EPIC_EUMBBAR + 0x10280)/* Ext. Int. Sr4 Des */
#define EPIC_EX_INT4_DES_REG  (EPIC_EUMBBAR + 0x10290)/* Ext. Int. Sr4 Vect*/

#define EPIC_SR_INT0_VEC_REG  (EPIC_EUMBBAR + 0x10200)/* Sr. Int. Sr0 Des */
#define EPIC_SR_INT0_DES_REG  (EPIC_EUMBBAR + 0x10210)/* Sr. Int. Sr0 Vect */
#define EPIC_SR_INT1_VEC_REG  (EPIC_EUMBBAR + 0x10220)/* Sr. Int. Sr1 Des */
#define EPIC_SR_INT1_DES_REG  (EPIC_EUMBBAR + 0x10230)/* Sr. Int. Sr1 Vect.*/
#define EPIC_SR_INT2_VEC_REG  (EPIC_EUMBBAR + 0x10240)/* Sr. Int. Sr2 Des */
#define EPIC_SR_INT2_DES_REG  (EPIC_EUMBBAR + 0x10250)/* Sr. Int. Sr2 Vect.*/
#define EPIC_SR_INT3_VEC_REG  (EPIC_EUMBBAR + 0x10260)/* Sr. Int. Sr3 Des */
#define EPIC_SR_INT3_DES_REG  (EPIC_EUMBBAR + 0x10270)/* Sr. Int. Sr3 Vect.*/
#define EPIC_SR_INT4_VEC_REG  (EPIC_EUMBBAR + 0x10280)/* Sr. Int. Sr4 Des */
#define EPIC_SR_INT4_DES_REG  (EPIC_EUMBBAR + 0x10290)/* Sr. Int. Sr4 Vect.*/

#define EPIC_SR_INT5_VEC_REG  (EPIC_EUMBBAR + 0x102a0)/* Sr. Int. Sr5 Des */
#define EPIC_SR_INT5_DES_REG  (EPIC_EUMBBAR + 0x102b0)/* Sr. Int. Sr5 Vect.*/
#define EPIC_SR_INT6_VEC_REG  (EPIC_EUMBBAR + 0x102c0)/* Sr. Int. Sr6 Des */
#define EPIC_SR_INT6_DES_REG  (EPIC_EUMBBAR + 0x102d0)/* Sr. Int. Sr6 Vect.*/
#define EPIC_SR_INT7_VEC_REG  (EPIC_EUMBBAR + 0x102e0)/* Sr. Int. Sr7 Des */
#define EPIC_SR_INT7_DES_REG  (EPIC_EUMBBAR + 0x102f0)/* Sr. Int. Sr7 Vect.*/
#define EPIC_SR_INT8_VEC_REG  (EPIC_EUMBBAR + 0x10300)/* Sr. Int. Sr8 Des */
#define EPIC_SR_INT8_DES_REG  (EPIC_EUMBBAR + 0x10310)/* Sr. Int. Sr8 Vect.*/
#define EPIC_SR_INT9_VEC_REG  (EPIC_EUMBBAR + 0x10320)/* Sr. Int. Sr9 Des */
#define EPIC_SR_INT9_DES_REG  (EPIC_EUMBBAR + 0x10330)/* Sr. Int. Sr9 Vect.*/

#define EPIC_SR_INT10_VEC_REG (EPIC_EUMBBAR + 0x10340)/* Sr. Int. Sr10 Des */
#define EPIC_SR_INT10_DES_REG (EPIC_EUMBBAR + 0x10350)/* Sr. Int. Sr10 Vect*/
#define EPIC_SR_INT11_VEC_REG (EPIC_EUMBBAR + 0x10360)/* Sr. Int. Sr11 Des */
#define EPIC_SR_INT11_DES_REG (EPIC_EUMBBAR + 0x10370)/* Sr. Int. Sr11 Vect*/
#define EPIC_SR_INT12_VEC_REG (EPIC_EUMBBAR + 0x10380)/* Sr. Int. Sr12 Des */
#define EPIC_SR_INT12_DES_REG (EPIC_EUMBBAR + 0x10390)/* Sr. Int. Sr12 Vect*/
#define EPIC_SR_INT13_VEC_REG (EPIC_EUMBBAR + 0x103a0)/* Sr. Int. Sr13 Des */
#define EPIC_SR_INT13_DES_REG (EPIC_EUMBBAR + 0x103b0)/* Sr. Int. Sr13 Vect*/
#define EPIC_SR_INT14_VEC_REG (EPIC_EUMBBAR + 0x103c0)/* Sr. Int. Sr14 Des */
#define EPIC_SR_INT14_DES_REG (EPIC_EUMBBAR + 0x103d0)/* Sr. Int. Sr14 Vect*/
#define EPIC_SR_INT15_VEC_REG (EPIC_EUMBBAR + 0x103e0)/* Sr. Int. Sr15 Des */
#define EPIC_SR_INT15_DES_REG (EPIC_EUMBBAR + 0x103f0)/* Sr. Int. Sr15 Vect*/

#define EPIC_I2C_INT_VEC_REG  (EPIC_EUMBBAR + 0x11020)/* I2C Int. Vect Pri.*/
#define EPIC_I2C_INT_DES_REG  (EPIC_EUMBBAR + 0x11030)/* I2C Int. Dest */
#define EPIC_DMA0_INT_VEC_REG (EPIC_EUMBBAR + 0x11040)/* DMA0 Int. Vect Pri*/
#define EPIC_DMA0_INT_DES_REG (EPIC_EUMBBAR + 0x11050)/* DMA0 Int. Dest */
#define EPIC_DMA1_INT_VEC_REG (EPIC_EUMBBAR + 0x11060)/* DMA1 Int. Vect Pri*/
#define EPIC_DMA1_INT_DES_REG (EPIC_EUMBBAR + 0x11070)/* DMA1 Int. Dest */
#define EPIC_MSG_INT_VEC_REG  (EPIC_EUMBBAR + 0x110c0)/* Msg Int. Vect Pri*/
#define EPIC_MSG_INT_DES_REG  (EPIC_EUMBBAR + 0x110d0)/* Msg Int. Dest  */

#define EPIC_PROC_CTASK_PRI_REG (EPIC_EUMBBAR + 0x20080)/* Proc. current task*/
#define EPIC_PROC_INT_ACK_REG (EPIC_EUMBBAR + 0x200a0)/* Int. acknowledge */
#define EPIC_PROC_EOI_REG (EPIC_EUMBBAR + 0x200b0)/* End of interrupt */

#define EPIC_VEC_PRI_MASK      0x80000000 /* Mask Interrupt bit in IVPR */
#define EPIC_VEC_PRI_DFLT_PRI  8          /* Interrupt Priority in IVPR */

/* Error code */

#define OK       0
#define ERROR    -1

/* function prototypes */

void epicVendorId( unsigned int *step,
       unsigned int *devId,
       unsigned int *venId
     );
void epicFeatures( unsigned int *noIRQs,
	     unsigned int *noCPUs,
       unsigned int *VerId );
extern void epicInit( unsigned int IRQType, unsigned int clkRatio);
ULONG sysEUMBBARRead ( ULONG regNum );
void sysEUMBBARWrite ( ULONG regNum, ULONG regVal);
extern void epicTmFrequencySet( unsigned int frq );
extern unsigned int epicTmFrequencyGet(void);
extern unsigned int epicTmBaseSet( ULONG srcAddr,
		 unsigned int cnt,
		 unsigned int inhibit );
extern unsigned int epicTmBaseGet ( ULONG srcAddr, unsigned int *val );
extern unsigned int epicTmCountGet( ULONG srcAddr, unsigned int *val );
extern unsigned int epicTmInhibit( unsigned int timer );
extern unsigned int epicTmEnable( ULONG srcAdr );
extern void CoreExtIntEnable(void);  /* Enable 603e external interrupts */
extern void CoreExtIntDisable(void); /* Disable 603e external interrupts */
extern unsigned char epicIntTaskGet(void);
extern void epicIntTaskSet( unsigned char val );
extern unsigned int epicIntAck(void);
extern void epicSprSet( unsigned int eumbbar, unsigned char );
extern void epicConfigGet( unsigned int *clkRatio,
	       unsigned int *serEnable );
extern void SrcVecTableInit(void);
extern unsigned int  epicModeGet(void);
extern void epicIntEnable(int Vect);
extern void epicIntDisable(int Vect);
extern int epicIntSourceConfig(int Vect, int Polarity, int Sense, int Prio);
extern unsigned int epicIntAck(void);
extern void epicEOI(void);
extern int epicCurTaskPrioSet(int Vect);

struct SrcVecTable
    {
     ULONG srcAddr;
     char  srcName[40];
    };

#endif   /*  EPIC_H */
