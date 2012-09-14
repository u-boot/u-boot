/*
 * (C) Copyright 2012 Xilinx
 *
 * Xilinx hardware interface to the Timer.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef XSCUTIMER_HW_H		/* prevent circular inclusions */
#define XSCUTIMER_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifdef NOTNOW_BHILL
#include "xil_types.h"
#include "xil_io.h"
#endif
/************************** Constant Definitions *****************************/

/** @name Register Map
 * Offsets of registers from the start of the device
 * @{
 */

#define XSCUTIMER_LOAD_OFFSET		0x00 /**< Timer Load Register */
#define XSCUTIMER_COUNTER_OFFSET	0x04 /**< Timer Counter Register */
#define XSCUTIMER_CONTROL_OFFSET	0x08 /**< Timer Control Register */
#define XSCUTIMER_ISR_OFFSET		0x0C /**< Timer Interrupt
						  Status Register */
/* @} */

/** @name Timer Control register
 * This register bits control the prescaler, Intr enable,
 * auto-reload and timer enable.
 * @{
 */

#define XSCUTIMER_CONTROL_PRESCALER_MASK	0x0000FF00 /**< Prescaler */
#define XSCUTIMER_CONTROL_PRESCALER_SHIFT	8
#define XSCUTIMER_CONTROL_IRQ_ENABLE_MASK	0x00000004 /**< Intr enable */
#define XSCUTIMER_CONTROL_AUTO_RELOAD_MASK	0x00000002 /**< Auto-reload */
#define XSCUTIMER_CONTROL_ENABLE_MASK		0x00000001 /**< Timer enable */
/* @} */

/** @name Interrupt Status register
 * This register indicates the Timer counter register has reached zero.
 * @{
 */

#define XSCUTIMER_ISR_EVENT_FLAG_MASK		0x00000001 /**< Event flag */
/*@}*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif	/* end of protection macro */
