/************************************************
 * NAME	    : arm925t.h
 * Version  : 23 June 2003			*
 ************************************************/

#ifndef __ARM925T_H__
#define __ARM925T_H__

void gpioreserve(ushort mask);
void gpiosetdir(ushort mask, ushort in);
void gpiosetout(ushort mask, ushort out);
void gpioinit(void);
void archflashwp(void *archdata, int wp);

#endif /*__ARM925T_H__*/
