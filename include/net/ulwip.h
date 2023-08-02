/* SPDX-License-Identifier: GPL-2.0 */

/**
 * ulwip_init() - initialize lwIP network stack
 *
 * @return 0 if success, !0 if error
 */
int ulwip_init(void);

/**
 * ulwip_active() - check if lwIP network stack was initialized
 *
 * @return 1 lwip processing, 0 if not
 */
int ulwip_active(void);

/**
 * ulwip_in_loop() - lwIP controls packet net loop
 *
 * @return 1 lwIP owns packet loop, 0 lwip does not own packet loop
 */
int ulwip_in_loop(void);

/**
 * ulwip_loop_set() - make loop to be used by lwIP
 *
 * Function is used to make lwIP control network pool.
 *
 * @loop: 1. Rx packets go to lwIP 2. Rx packets go to the original stack.
 */
void ulwip_loop_set(int loop);

/**
 * ulwip_exit() - exit from lwIP with a return code
 *
 * Exit from lwIP application back to the U-Boot with specific error code.
 *
 * @err: Error code to return
 */
void ulwip_exit(int err);

/**
 * ulwip_poll() - polling function to feed lwIP with ethernet packet
 *
 * Function takes network packet and passes it to lwIP network stack
 *
 * @in_packet:  Pointer to the network packet data
 * @len: Length of the network packet
 */
void ulwip_poll(uchar *in_packet, int len);

/**
 * ulwip_app_get_err() - return error code from lwIP application
 *
 * @return error code
 */
int ulwip_app_get_err(void);

/**
 * ulwip_loop() - enter to packet polling loop
 *
 * When lwIP application did it's initialization stage, then it needs to enter
 * to packet polling loop to grab rx packets.
 *
 * Returns:  0 if success, !0 if error
 */
int ulwip_loop(void);
