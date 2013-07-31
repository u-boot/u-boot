/*
 * (C) Copyright 2011
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PCA9698_H_
#define __PCA9698_H_

int pca9698_request(unsigned gpio, const char *label);
void pca9698_free(unsigned gpio);
int pca9698_direction_input(u8 addr, unsigned gpio);
int pca9698_direction_output(u8 addr, unsigned gpio, int value);
int pca9698_get_value(u8 addr, unsigned gpio);
int pca9698_set_value(u8 addr, unsigned gpio, int value);

#endif /* __PCA9698_H_ */
