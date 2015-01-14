/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PHY_H_
#define _PHY_H_

int setup_88e1514(const char *bus, unsigned char addr);
int setup_88e1518(const char *bus, unsigned char addr);

#endif
