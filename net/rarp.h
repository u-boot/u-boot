/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#if defined(CONFIG_CMD_RARP)

#ifndef __RARP_H__
#define __RARP_H__

#include <net.h>

/**********************************************************************/
/*
 *	Global functions and variables.
 */

extern int RarpTry;

/* Process the receipt of a RARP packet */
extern void rarp_receive(struct ip_udp_hdr *ip, unsigned len);
extern void RarpRequest(void);	/* Send a RARP request */

/**********************************************************************/

#endif /* __RARP_H__ */
#endif
