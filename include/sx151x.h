/*
 * (C) Copyright 2013
 * Viktar Palstsiuk, Promwad, viktar.palstsiuk@promwad.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SX151X_H_
#define __SX151X_H_

int sx151x_get_value(int chip, int gpio);
int sx151x_set_value(int chip, int gpio, int val);
int sx151x_direction_input(int chip, int gpio);
int sx151x_direction_output(int chip, int gpio);
int sx151x_reset(int chip);

#endif /* __SX151X_H_ */
