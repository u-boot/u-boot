/****************************************************************************
 * Copyright(c) 2000-2001 Broadcom Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * Name:        nicext.h
 *
 * Description: Broadcom Network Interface Card Extension (NICE) is an
 *              extension to Linux NET device kernel mode drivers.
 *              NICE is designed to provide additional functionalities,
 *              such as receive packet intercept. To support Broadcom NICE,
 *              the network device driver can be modified by adding an
 *              device ioctl handler and by indicating receiving packets
 *              to the NICE receive handler. Broadcom NICE will only be
 *              enabled by a NICE-aware intermediate driver, such as
 *              Broadcom Advanced Server Program Driver (BASP). When NICE
 *              is not enabled, the modified network device drivers
 *              functions exactly as other non-NICE aware drivers.
 *
 * Author:      Frankie Fan
 *
 * Created:     September 17, 2000
 *
 ****************************************************************************/
#ifndef _nicext_h_
#define _nicext_h_

/*
 * ioctl for NICE
 */
#define SIOCNICE                   	SIOCDEVPRIVATE+7

/*
 * SIOCNICE:
 *
 * The following structure needs to be less than IFNAMSIZ (16 bytes) because
 * we're overloading ifreq.ifr_ifru.
 *
 * If 16 bytes is not enough, we should consider relaxing this because
 * this is no field after ifr_ifru in the ifreq structure. But we may
 * run into future compatiability problem in case of changing struct ifreq.
 */
struct nice_req
{
    __u32 cmd;

    union
    {
#ifdef __KERNEL__
	/* cmd = NICE_CMD_SET_RX or NICE_CMD_GET_RX */
	struct
	{
	    void (*nrqus1_rx)( struct sk_buff*, void* );
	    void* nrqus1_ctx;
	} nrqu_nrqus1;

	/* cmd = NICE_CMD_QUERY_SUPPORT */
	struct
	{
	    __u32 nrqus2_magic;
	    __u32 nrqus2_support_rx:1;
	    __u32 nrqus2_support_vlan:1;
	    __u32 nrqus2_support_get_speed:1;
	} nrqu_nrqus2;
#endif

	/* cmd = NICE_CMD_GET_SPEED */
	struct
	{
	    unsigned int nrqus3_speed; /* 0 if link is down, */
				       /* otherwise speed in Mbps */
	} nrqu_nrqus3;

	/* cmd = NICE_CMD_BLINK_LED */
	struct
	{
	    unsigned int nrqus4_blink_time; /* blink duration in seconds */
	} nrqu_nrqus4;

    } nrq_nrqu;
};

#define nrq_rx           nrq_nrqu.nrqu_nrqus1.nrqus1_rx
#define nrq_ctx          nrq_nrqu.nrqu_nrqus1.nrqus1_ctx
#define nrq_support_rx   nrq_nrqu.nrqu_nrqus2.nrqus2_support_rx
#define nrq_magic        nrq_nrqu.nrqu_nrqus2.nrqus2_magic
#define nrq_support_vlan nrq_nrqu.nrqu_nrqus2.nrqus2_support_vlan
#define nrq_support_get_speed nrq_nrqu.nrqu_nrqus2.nrqus2_support_get_speed
#define nrq_speed        nrq_nrqu.nrqu_nrqus3.nrqus3_speed
#define nrq_blink_time   nrq_nrqu.nrqu_nrqus4.nrqus4_blink_time

/*
 * magic constants
 */
#define NICE_REQUESTOR_MAGIC            0x4543494E /* NICE in ascii */
#define NICE_DEVICE_MAGIC               0x4E494345 /* ECIN in ascii */

/*
 * command field
 */
#define NICE_CMD_QUERY_SUPPORT          0x00000001
#define NICE_CMD_SET_RX                 0x00000002
#define NICE_CMD_GET_RX                 0x00000003
#define NICE_CMD_GET_SPEED              0x00000004
#define NICE_CMD_BLINK_LED              0x00000005

#endif  /* _nicext_h_ */
