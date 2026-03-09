/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written by Timesys Corporation
 *
 *
 */

#ifndef DT_BINDINGS_PINCTRL_ADI_ADSP
#define DT_BINDINGS_PINCTRL_ADI_ADSP

#define ADI_ADSP_PIN(port, pin) (16 * ((port) - 'A') + (pin))
#define ADI_ADSP_PINFUNC_ALT0 0
#define ADI_ADSP_PINFUNC_ALT1 1
#define ADI_ADSP_PINFUNC_ALT2 2
#define ADI_ADSP_PINFUNC_ALT3 3

#endif
