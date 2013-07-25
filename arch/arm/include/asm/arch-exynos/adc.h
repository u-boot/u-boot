/*
 *  Copyright (C) 2010 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *  MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARM_ARCH_ADC_H_
#define __ASM_ARM_ARCH_ADC_H_

#ifndef __ASSEMBLY__
struct s5p_adc {
	unsigned int adccon;
	unsigned int adctsc;
	unsigned int adcdly;
	unsigned int adcdat0;
	unsigned int adcdat1;
	unsigned int adcupdn;
	unsigned int adcclrint;
	unsigned int adcmux;
	unsigned int adcclrintpndnup;
};
#endif

#endif /* __ASM_ARM_ARCH_ADC_H_ */
