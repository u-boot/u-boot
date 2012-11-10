/*
 *	LiMon - BOOTP/TFTP.
 *
 *	Copyright 1994, 1995, 2000 Neil Russell.
 *	Copyright 2011 Comelit Group SpA
 *	               Luca Ceresoli <luca.ceresoli@comelit.it>
 *	(See License)
 */

#ifndef __TFTP_H__
#define __TFTP_H__

/**********************************************************************/
/*
 *	Global functions and variables.
 */

/* tftp.c */
void TftpStart(enum proto_t protocol);	/* Begin TFTP get/put */

#ifdef CONFIG_CMD_TFTPSRV
extern void TftpStartServer(void);	/* Wait for incoming TFTP put */
#endif

extern ulong TftpRRQTimeoutMSecs;
extern int TftpRRQTimeoutCountMax;

/**********************************************************************/

#endif /* __TFTP_H__ */
