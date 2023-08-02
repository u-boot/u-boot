/* SPDX-License-Identifier: GPL-2.0 */

int do_lwip_dns(struct cmd_tbl *cmdtp, int flag, int argc,
		char *const argv[]);

/**
 * ulwip_dns() - creates the DNS request to resolve a domain host name
 *
 * This function creates the DNS request to resolve a domain host name. Function
 * can return immediately if previous request was cached or it might require
 * entering the polling loop for a request to a remote server.
 *
 * @name:    dns name to resolve
 * @varname: (optional) U-Boot variable name to store the result
 * Returns: ERR_OK(0) for fetching entry from the cache
 *          -EINPROGRESS success, can go to the polling loop
 *          Other value < 0, if error
 */
int ulwip_dns(char *name, char *varname);

/**
 * ulwip_dhcp() -  create the DHCP request to obtain IP address.
 *
 * This function creates the DHCP request to obtain IP address. If DHCP server
 * returns file name, this file will be downloaded with tftp.  After this
 * function you need to invoke the polling loop to process network communication.
 *
 * Returns: 0 if success
 *         Other value < 0, if error
 **/
int ulwip_dhcp(void);

/**
 * ulwip_tftp() - load file with tftp
 *
 * Load file with tftp to specific address
 *
 * @addr: Address to store downloaded file
 * @filename: File name on remote tftp server to download
 *
 *
 * Returns:  0 if success, !0 if error
 */
int ulwip_tftp(ulong addr, const char *filename);
