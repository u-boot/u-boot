/*
 *	Copied from LiMon - BOOTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Paolo Scaffardi
 */

#ifndef __NET_RAND_H__
#define __NET_RAND_H__

#define RAND_MAX 0xffffffff

/*
 * Seed the random number generator using the eth0 MAC address
 */
void srand_mac(void);

/*
 * Get a random number (after seeding with MAC address)
 *
 * @return random number
 */
unsigned long rand(void);

#endif /* __NET_RAND_H__ */
