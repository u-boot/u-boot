/*
 * General Purpose Timer Masks
 */

#ifndef __BFIN_PERIPHERAL_TIMER__
#define __BFIN_PERIPHERAL_TIMER__

/* TIMER_ENABLE Masks */
#define TIMEN0			0x0001		/* Enable Timer 0					*/
#define TIMEN1			0x0002		/* Enable Timer 1					*/
#define TIMEN2			0x0004		/* Enable Timer 2					*/
#define TIMEN3			0x0008		/* Enable Timer 3					*/
#define TIMEN4			0x0010		/* Enable Timer 4					*/
#define TIMEN5			0x0020		/* Enable Timer 5					*/
#define TIMEN6			0x0040		/* Enable Timer 6					*/
#define TIMEN7			0x0080		/* Enable Timer 7					*/

/* TIMER_DISABLE Masks */
#define TIMDIS0			TIMEN0		/* Disable Timer 0					*/
#define TIMDIS1			TIMEN1		/* Disable Timer 1					*/
#define TIMDIS2			TIMEN2		/* Disable Timer 2					*/
#define TIMDIS3			TIMEN3		/* Disable Timer 3					*/
#define TIMDIS4			TIMEN4		/* Disable Timer 4					*/
#define TIMDIS5			TIMEN5		/* Disable Timer 5					*/
#define TIMDIS6			TIMEN6		/* Disable Timer 6					*/
#define TIMDIS7			TIMEN7		/* Disable Timer 7					*/

/* TIMER_STATUS Masks */
#define TIMIL0			0x00000001	/* Timer 0 Interrupt				*/
#define TIMIL1			0x00000002	/* Timer 1 Interrupt				*/
#define TIMIL2			0x00000004	/* Timer 2 Interrupt				*/
#define TIMIL3			0x00000008	/* Timer 3 Interrupt				*/
#define TOVF_ERR0		0x00000010	/* Timer 0 Counter Overflow			*/
#define TOVF_ERR1		0x00000020	/* Timer 1 Counter Overflow			*/
#define TOVF_ERR2		0x00000040	/* Timer 2 Counter Overflow			*/
#define TOVF_ERR3		0x00000080	/* Timer 3 Counter Overflow			*/
#define TRUN0			0x00001000	/* Timer 0 Slave Enable Status		*/
#define TRUN1			0x00002000	/* Timer 1 Slave Enable Status		*/
#define TRUN2			0x00004000	/* Timer 2 Slave Enable Status		*/
#define TRUN3			0x00008000	/* Timer 3 Slave Enable Status		*/
#define TIMIL4			0x00010000	/* Timer 4 Interrupt				*/
#define TIMIL5			0x00020000	/* Timer 5 Interrupt				*/
#define TIMIL6			0x00040000	/* Timer 6 Interrupt				*/
#define TIMIL7			0x00080000	/* Timer 7 Interrupt				*/
#define TOVF_ERR4		0x00100000	/* Timer 4 Counter Overflow			*/
#define TOVF_ERR5		0x00200000	/* Timer 5 Counter Overflow			*/
#define TOVF_ERR6		0x00400000	/* Timer 6 Counter Overflow			*/
#define TOVF_ERR7		0x00800000	/* Timer 7 Counter Overflow			*/
#define TRUN4			0x10000000	/* Timer 4 Slave Enable Status		*/
#define TRUN5			0x20000000	/* Timer 5 Slave Enable Status		*/
#define TRUN6			0x40000000	/* Timer 6 Slave Enable Status		*/
#define TRUN7			0x80000000	/* Timer 7 Slave Enable Status		*/

/* Alternate Deprecated Macros Provided For Backwards Code Compatibility */
#define TOVL_ERR0 TOVF_ERR0
#define TOVL_ERR1 TOVF_ERR1
#define TOVL_ERR2 TOVF_ERR2
#define TOVL_ERR3 TOVF_ERR3
#define TOVL_ERR4 TOVF_ERR4
#define TOVL_ERR5 TOVF_ERR5
#define TOVL_ERR6 TOVF_ERR6
#define TOVL_ERR7 TOVF_ERR7

/* TIMERx_CONFIG Masks */
#define PWM_OUT			0x0001	/* Pulse-Width Modulation Output Mode	*/
#define WDTH_CAP		0x0002	/* Width Capture Input Mode				*/
#define EXT_CLK			0x0003	/* External Clock Mode					*/
#define PULSE_HI		0x0004	/* Action Pulse (Positive/Negative*)	*/
#define PERIOD_CNT		0x0008	/* Period Count							*/
#define IRQ_ENA			0x0010	/* Interrupt Request Enable				*/
#define TIN_SEL			0x0020	/* Timer Input Select					*/
#define OUT_DIS			0x0040	/* Output Pad Disable					*/
#define CLK_SEL			0x0080	/* Timer Clock Select					*/
#define TOGGLE_HI		0x0100	/* PWM_OUT PULSE_HI Toggle Mode			*/
#define EMU_RUN			0x0200	/* Emulation Behavior Select			*/
#define ERR_TYP			0xC000	/* Error Type							*/

#endif
