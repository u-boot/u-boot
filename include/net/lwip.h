/* SPDX-License-Identifier: GPL-2.0 */

int do_lwip_dns(struct cmd_tbl *cmdtp, int flag, int argc,
		char *const argv[]);
int do_lwip_ping(struct cmd_tbl *cmdtp, int flag, int argc,
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

/**
 * ulwip_wget() - creates the HTTP request to download file
 *
 * This function creates the HTTP request to download file from url to the address
 * specified in parameters. After this function you need to invoke the polling
 * loop to process network communication.
 *
 *
 * @addr:  start address to download result
 * @url:   url in format http://host/url
 * Returns: 0 for success, !0 if error
 */
int ulwip_wget(ulong addr, char *url);

/**
 * ulwip_ping - create the ping request
 *
 * This function creates the ping for  address provided in parameters.
 * After this function you need to invoke the polling
 * loop to process network communication.
 *
 *
 * @ping_addr: IP address to ping
 * Returns: 0 for success, !0 if error
*/
int ulwip_ping(char *ping_addr);


/**
 * ulwip_list - print lwip interfaces information
 *
 * Print information about lwip interfaces, like name
 * MAC, state. Usefull for debugging.
 *
*/
void ulwip_list(void);

/**
 * ulwip_up_down - up/down lwip interface
 *
*/
void ulwip_up_down(char *name, int up);
