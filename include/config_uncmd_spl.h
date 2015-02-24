/*
 * (C) Copyright 2012
 * Ilya Yanok, ilya.yanok@gmail.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_UNCMD_SPL_H__
#define __CONFIG_UNCMD_SPL_H__

#ifdef CONFIG_SPL_BUILD
/* SPL needs only BOOTP + TFTP so undefine other stuff to save space */
#undef CONFIG_CMD_CDP
#undef CONFIG_CMD_DHCP
#undef CONFIG_CMD_DNS
#undef CONFIG_CMD_LINK_LOCAL
#undef CONFIG_CMD_NFS
#undef CONFIG_CMD_PING
#undef CONFIG_CMD_RARP
#undef CONFIG_CMD_SNTP
#undef CONFIG_CMD_TFTPPUT
#undef CONFIG_CMD_TFTPSRV
#undef CONFIG_OF_CONTROL

#ifndef CONFIG_SPL_DM
#undef CONFIG_DM_SERIAL
#undef CONFIG_DM_GPIO
#undef CONFIG_DM_I2C
#undef CONFIG_DM_SPI
#endif

#undef CONFIG_DM_WARN
#undef CONFIG_DM_DEVICE_REMOVE
#undef CONFIG_DM_STDIO

#endif /* CONFIG_SPL_BUILD */
#endif /* __CONFIG_UNCMD_SPL_H__ */
