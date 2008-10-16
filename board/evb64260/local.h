/*
 * include/local.h - local configuration options, board specific
 */

#ifndef __LOCAL_H
#define __LOCAL_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

/* This tells U-Boot that the config options are compiled in */
/* #undef ENV_IS_EMBEDDED */
/* Don't touch this! U-Boot figures this out  based on other
 * magic. */

/* Uncomment and define any of the below options */

/* #define CONFIG_750CX */ /* The 750CX doesn't support as many things in L2CR */
			/* Note: If you defined CONFIG_EVB64260_750CX this */
			/* gets defined automatically. */

/* These want string arguments */
/* #define CONFIG_BOOTARGS */
/* #define CONFIG_BOOTCOMMAND */
/* #define CONFIG_RAMBOOTCOMMAND */
/* #define CONFIG_NFSBOOTCOMMAND */
/* #define CONFIG_SYS_AUTOLOAD */
/* #define CONFIG_PREBOOT */

/* These don't */

/* #define CONFIG_BOOTDELAY  */
/* #define CONFIG_BAUDRATE */
/* #define CONFIG_LOADS_ECHO */
/* #define CONFIG_ETHADDR */
/* #define CONFIG_ETH2ADDR */
/* #define CONFIG_ETH3ADDR */
/* #define CONFIG_IPADDR */
/* #define CONFIG_SERVERIP */
/* #define CONFIG_ROOTPATH */
/* #define CONFIG_GATEWAYIP */
/* #define CONFIG_NETMASK */
/* #define CONFIG_HOSTNAME */
/* #define CONFIG_BOOTFILE */
/* #define CONFIG_LOADADDR */

/* these hardware addresses are pretty bogus, please change them to
   suit your needs */

/* first ethernet */
#define CONFIG_ETHADDR          00:11:22:33:44:55

/* next two ethernet hwaddrs */
#define CONFIG_HAS_ETH1
#define CONFIG_ETH1ADDR		00:11:22:33:44:66
#define CONFIG_HAS_ETH2
#define CONFIG_ETH2ADDR		00:11:22:33:44:77

#define CONFIG_ENV_OVERWRITE
#endif	/* __CONFIG_H */
