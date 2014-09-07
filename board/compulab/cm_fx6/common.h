/*
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/mx6-pins.h>
#include <asm/arch/clock.h>

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |	\
			PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |	\
			PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define CM_FX6_ECSPI_BUS0_CS0	IMX_GPIO_NR(2, 30)
#define CM_FX6_GREEN_LED	IMX_GPIO_NR(2, 31)

void cm_fx6_set_usdhc_iomux(void);
void cm_fx6_set_ecspi_iomux(void);
