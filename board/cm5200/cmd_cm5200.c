/*
 * (C) Copyright 2007 Markus Kappeler <markus.kappeler@objectxp.com>
 *
 * Adapted for U-Boot 1.2 by Piotr Kruszynski <ppk@semihalf.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <usb.h>

#ifdef CONFIG_CMD_BSP

static int do_i2c_test(char *argv[])
{
	unsigned char temp, temp1;

	printf("Starting I2C Test\n"
		"Please set Jumper:\nI2C SDA 2-3\nI2C SCL 2-3\n\n"
		"Please press any key to start\n\n");
	getc();

	temp = 0xf0; /* set io 0-4 as output */
	i2c_write(CONFIG_SYS_I2C_IO, 3, 1, (uchar *)&temp, 1);

	printf("Press I2C4-7. LED I2C0-3 should have the same state\n\n"
		"Press any key to stop\n\n");

	while (!tstc()) {
		i2c_read(CONFIG_SYS_I2C_IO, 0, 1, (uchar *)&temp, 1);
		temp1 = (temp >> 4) & 0x03;
		temp1 |= (temp >> 3) & 0x08; /* S302 -> LED303 */
		temp1 |= (temp >> 5) & 0x04; /* S303 -> LED302 */
		temp = temp1;
		i2c_write(CONFIG_SYS_I2C_IO, 1, 1, (uchar *)&temp, 1);
	}
	getc();

	return 0;
}

static int do_usb_test(char *argv[])
{
	int i;
	static int usb_stor_curr_dev = -1; /* current device */

	printf("Starting USB Test\n"
		"Please insert USB Memmory Stick\n\n"
		"Please press any key to start\n\n");
	getc();

	usb_stop();
	printf("(Re)start USB...\n");
	i = usb_init();
#ifdef CONFIG_USB_STORAGE
		/* try to recognize storage devices immediately */
		if (i >= 0)
			usb_stor_curr_dev = usb_stor_scan(1);
#endif /* CONFIG_USB_STORAGE */
	if (usb_stor_curr_dev >= 0)
		printf("Found USB Storage Dev continue with Test...\n");
	else {
		printf("No USB Storage Device detected.. Stop Test\n");
		return 1;
	}

	usb_stor_info();

	printf("stopping USB..\n");
	usb_stop();

	return 0;
}

static int do_led_test(char *argv[])
{
	int i = 0;
	struct mpc5xxx_gpt_0_7 *gpt = (struct mpc5xxx_gpt_0_7 *)MPC5XXX_GPT;

	printf("Starting LED Test\n"
		"Please set Switch S500 all off\n\n"
		"Please press any key to start\n\n");
	getc();

	/* configure timer 2-3 for simple GPIO output High */
	gpt->gpt2.emsr |= 0x00000034;
	gpt->gpt3.emsr |= 0x00000034;

	(*(vu_long *)MPC5XXX_WU_GPIO_ENABLE) |= 0x80000000;
	(*(vu_long *)MPC5XXX_WU_GPIO_DIR) |= 0x80000000;
	printf("Please press any key to stop\n\n");
	while (!tstc()) {
		if (i == 1) {
			(*(vu_long *)MPC5XXX_WU_GPIO_DATA_O) |= 0x80000000;
			gpt->gpt2.emsr &= ~0x00000010;
			gpt->gpt3.emsr &= ~0x00000010;
		} else if (i == 2) {
			(*(vu_long *)MPC5XXX_WU_GPIO_DATA_O) &= ~0x80000000;
			gpt->gpt2.emsr &= ~0x00000010;
			gpt->gpt3.emsr |= 0x00000010;
		} else if (i >= 3) {
			(*(vu_long *)MPC5XXX_WU_GPIO_DATA_O) &= ~0x80000000;
			gpt->gpt3.emsr &= ~0x00000010;
			gpt->gpt2.emsr |= 0x00000010;
			i = 0;
		}
		i++;
		udelay(200000);
	}
	getc();

	(*(vu_long *)MPC5XXX_WU_GPIO_DATA_O) |= 0x80000000;
	gpt->gpt2.emsr |= 0x00000010;
	gpt->gpt3.emsr |= 0x00000010;

	return 0;
}

static int do_rs232_test(char *argv[])
{
	int error_status = 0;
	struct mpc5xxx_gpio *gpio = (struct mpc5xxx_gpio *)MPC5XXX_GPIO;
	struct mpc5xxx_psc *psc1 = (struct mpc5xxx_psc *)MPC5XXX_PSC1;

	/* Configure PSC 2-3-6 as GPIO */
	gpio->port_config &= 0xFF0FF80F;

	switch (simple_strtoul(argv[2], NULL, 10)) {
	case 1:
		/* check RTS <-> CTS loop */
		/* set rts to 0 */
		printf("Uart 1 test: RX TX tested by using U-Boot\n"
			"Please connect RTS with CTS on Uart1 plug\n\n"
			"Press any key to start\n\n");
		getc();

		psc1->op1 |= 0x01;

		/* wait some time before requesting status */
		udelay(10);

		/* check status at cts */
		if ((psc1->ip & 0x01) != 0) {
			error_status = 3;
			printf("%s: failure at rs232_1, cts status is %d "
				"(should be 0)\n",
				__FUNCTION__, (psc1->ip & 0x01));
		}

		/* set rts to 1 */
		psc1->op0 |= 0x01;

		/* wait some time before requesting status */
		udelay(10);

		/* check status at cts */
		if ((psc1->ip & 0x01) != 1) {
			error_status = 3;
			printf("%s: failure at rs232_1, cts status is %d "
				"(should be 1)\n",
				__FUNCTION__, (psc1->ip & 0x01));
		}
		break;
	case 2:
		/* set PSC2_0, PSC2_2 as output and PSC2_1, PSC2_3 as input */
		printf("Uart 2 test: Please use RS232 Loopback plug on UART2\n"
			"\nPress any key to start\n\n");
		getc();

		gpio->simple_gpioe &= ~(0x000000F0);
		gpio->simple_gpioe |= 0x000000F0;
		gpio->simple_ddr &= ~(0x000000F0);
		gpio->simple_ddr |= 0x00000050;

		/* check TXD <-> RXD loop */
		/* set TXD to 1 */
		gpio->simple_dvo |= (1 << 4);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000020) != 0x00000020) {
			error_status = 2;
			printf("%s: failure at rs232_2, rxd status is %d "
				"(should be 1)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000020) >> 5);
		}

		/* set TXD to 0 */
		gpio->simple_dvo &= ~(1 << 4);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000020) != 0x00000000) {
			error_status = 2;
			printf("%s: failure at rs232_2, rxd status is %d "
				"(should be 0)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000020) >> 5);
		}

		/* check RTS <-> CTS loop */
		/* set RTS to 1 */
		gpio->simple_dvo |= (1 << 6);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000080) != 0x00000080) {
			error_status = 3;
			printf("%s: failure at rs232_2, cts status is %d "
				"(should be 1)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000080) >> 7);
		}

		/* set RTS to 0 */
		gpio->simple_dvo &= ~(1 << 6);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000080) != 0x00000000) {
			error_status = 3;
			printf("%s: failure at rs232_2, cts status is %d "
				"(should be 0)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000080) >> 7);
		}
		break;
	case 3:
		/* set PSC3_0, PSC3_2 as output and PSC3_1, PSC3_3 as input */
		printf("Uart 3 test: Please use RS232 Loopback plug on UART2\n"
			"\nPress any key to start\n\n");
		getc();

		gpio->simple_gpioe &= ~(0x00000F00);
		gpio->simple_gpioe |= 0x00000F00;

		gpio->simple_ddr &= ~(0x00000F00);
		gpio->simple_ddr |= 0x00000500;

		/* check TXD <-> RXD loop */
		/* set TXD to 1 */
		gpio->simple_dvo |= (1 << 8);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000200) != 0x00000200) {
			error_status = 2;
			printf("%s: failure at rs232_3, rxd status is %d "
				"(should be 1)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000200) >> 9);
		}

		/* set TXD to 0 */
		gpio->simple_dvo &= ~(1 << 8);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000200) != 0x00000000) {
			error_status = 2;
			printf("%s: failure at rs232_3, rxd status is %d "
				"(should be 0)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000200) >> 9);
		}

		/* check RTS <-> CTS loop */
		/* set RTS to 1 */
		gpio->simple_dvo |= (1 << 10);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000800) != 0x00000800) {
			error_status = 3;
			printf("%s: failure at rs232_3, cts status is %d "
				"(should be 1)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000800) >> 11);
		}

		/* set RTS to 0 */
		gpio->simple_dvo &= ~(1 << 10);

		/* wait some time before requesting status */
		udelay(10);

		if ((gpio->simple_ival & 0x00000800) != 0x00000000) {
			error_status = 3;
			printf("%s: failure at rs232_3, cts status is %d "
				"(should be 0)\n", __FUNCTION__,
				(gpio->simple_ival & 0x00000800) >> 11);
		}
		break;
	case 4:
		/* set PSC6_2, PSC6_3 as output and PSC6_0, PSC6_1 as input */
		printf("Uart 4 test: Please use RS232 Loopback plug on UART2\n"
			"\nPress any key to start\n\n");
		getc();

		gpio->simple_gpioe &= ~(0xF0000000);
		gpio->simple_gpioe |= 0x30000000;

		gpio->simple_ddr &= ~(0xf0000000);
		gpio->simple_ddr |= 0x30000000;

		(*(vu_long *)MPC5XXX_WU_GPIO_ENABLE) |= 0x30000000;
		(*(vu_long *)MPC5XXX_WU_GPIO_DIR) &= ~(0x30000000);

		/* check TXD <-> RXD loop */
		/* set TXD to 1 */
		gpio->simple_dvo |= (1 << 28);

		/* wait some time before requesting status */
		udelay(10);

		if (((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) & 0x10000000) !=
				0x10000000) {
			error_status = 2;
			printf("%s: failure at rs232_4, rxd status is %lu "
				"(should be 1)\n", __FUNCTION__,
				((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) &
					0x10000000) >> 28);
		}

		/* set TXD to 0 */
		gpio->simple_dvo &= ~(1 << 28);

		/* wait some time before requesting status */
		udelay(10);

		if (((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) & 0x10000000) !=
				0x00000000) {
			error_status = 2;
			printf("%s: failure at rs232_4, rxd status is %lu "
				"(should be 0)\n", __FUNCTION__,
				((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) &
					0x10000000) >> 28);
		}

		/* check RTS <-> CTS loop */
		/* set RTS to 1 */
		gpio->simple_dvo |= (1 << 29);

		/* wait some time before requesting status */
		udelay(10);

		if (((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) & 0x20000000) !=
				0x20000000) {
			error_status = 3;
			printf("%s: failure at rs232_4, cts status is %lu "
				"(should be 1)\n", __FUNCTION__,
				((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) &
					0x20000000) >> 29);
		}

		/* set RTS to 0 */
		gpio->simple_dvo &= ~(1 << 29);

		/* wait some time before requesting status */
		udelay(10);

		if (((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) & 0x20000000) !=
				0x00000000) {
			error_status = 3;
			printf("%s: failure at rs232_4, cts status is %lu "
				"(should be 0)\n", __FUNCTION__,
				((*(vu_long *)MPC5XXX_WU_GPIO_DATA_I) &
					0x20000000) >> 29);
		}
		break;
	default:
		printf("%s: invalid rs232 number %s\n", __FUNCTION__, argv[2]);
		error_status = 1;
		break;
	}
	gpio->port_config |= (CONFIG_SYS_GPS_PORT_CONFIG & 0xFF0FF80F);

	return error_status;
}

static int cmd_fkt(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = -1;

	switch (argc) {
	case 2:
		if (strncmp(argv[1], "i2c", 3) == 0)
			rcode = do_i2c_test(argv);
		else if (strncmp(argv[1], "led", 3) == 0)
			rcode = do_led_test(argv);
		else if (strncmp(argv[1], "usb", 3) == 0)
			rcode = do_usb_test(argv);
		break;
	case 3:
		if (strncmp(argv[1], "rs232", 3) == 0)
			rcode = do_rs232_test(argv);
		break;
	}

	switch (rcode) {
	case -1:
		printf("Usage:\n"
			"fkt { i2c | led | usb }\n"
			"fkt rs232 number\n");
		rcode = 1;
		break;
	case 0:
		printf("Test passed\n");
		break;
	default:
		printf("Test failed with code: %d\n", rcode);
	}

	return rcode;
}

U_BOOT_CMD(
	fkt,	4,	1,	cmd_fkt,
	"Function test routines",
	"i2c\n"
	"     - Test I2C communication\n"
	"fkt led\n"
	"     - Test LEDs\n"
	"fkt rs232 number\n"
	"     - Test RS232 (loopback plug(s) for RS232 required)\n"
	"fkt usb\n"
	"     - Test USB communication"
);
#endif /* CONFIG_CMD_BSP */
