/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <i2c.h>
#include <miiphy.h>

#ifdef CONFIG_TAISHAN

#define LCD_DELAY_NORMAL_US	100
#define LCD_DELAY_NORMAL_MS	2
#define LCD_CMD_ADDR		((volatile char *)(CONFIG_SYS_EBC2_LCM_BASE))
#define LCD_DATA_ADDR		((volatile char *)(CONFIG_SYS_EBC2_LCM_BASE+1))
#define LCD_BLK_CTRL		((volatile char *)(CONFIG_SYS_EBC1_FPGA_BASE+0x2))

#define mdelay(t)	({unsigned long msec=(t); while (msec--) { udelay(1000);}})

static int g_lcd_init_b = 0;
static char *amcc_logo = "  AMCC TAISHAN  440GX EvalBoard";
static char addr_flag = 0x80;

static void lcd_bl_ctrl(char val)
{
	char cpld_val;

	cpld_val = *LCD_BLK_CTRL;
	*LCD_BLK_CTRL = val | cpld_val;
}

static void lcd_putc(char val)
{
	int i = 100;
	char addr;

	while (i--) {
		if ((*LCD_CMD_ADDR & 0x80) != 0x80) {	/*BF = 1 ? */
			udelay(LCD_DELAY_NORMAL_US);
			break;
		}
		udelay(LCD_DELAY_NORMAL_US);
	}

	if (*LCD_CMD_ADDR & 0x80) {
		printf("LCD is busy\n");
		return;
	}

	addr = *LCD_CMD_ADDR;
	udelay(LCD_DELAY_NORMAL_US);
	if ((addr != 0) && (addr % 0x10 == 0)) {
		addr_flag ^= 0x40;
		*LCD_CMD_ADDR = addr_flag;
	}

	udelay(LCD_DELAY_NORMAL_US);
	*LCD_DATA_ADDR = val;
	udelay(LCD_DELAY_NORMAL_US);
}

static void lcd_puts(char *s)
{
	char *p = s;
	int i = 100;

	while (i--) {
		if ((*LCD_CMD_ADDR & 0x80) != 0x80) {	/*BF = 1 ? */
			udelay(LCD_DELAY_NORMAL_US);
			break;
		}
		udelay(LCD_DELAY_NORMAL_US);
	}

	if (*LCD_CMD_ADDR & 0x80) {
		printf("LCD is busy\n");
		return;
	}

	while (*p)
		lcd_putc(*p++);
}

static void lcd_put_logo(void)
{
	int i = 100;
	char *p = amcc_logo;

	while (i--) {
		if ((*LCD_CMD_ADDR & 0x80) != 0x80) {	/*BF = 1 ? */
			udelay(LCD_DELAY_NORMAL_US);
			break;
		}
		udelay(LCD_DELAY_NORMAL_US);
	}

	if (*LCD_CMD_ADDR & 0x80) {
		printf("LCD is busy\n");
		return;
	}

	*LCD_CMD_ADDR = 0x80;
	while (*p)
		lcd_putc(*p++);
}

int lcd_init(void)
{
	if (g_lcd_init_b == 0) {
		puts("LCD: ");
		mdelay(100);	/* Waiting for the LCD initialize */

		*LCD_CMD_ADDR = 0x38;	/*set function:8-bit,2-line,5x7 font type */
		udelay(LCD_DELAY_NORMAL_US);

		*LCD_CMD_ADDR = 0x0f;	/*set display on,cursor on,blink on */
		udelay(LCD_DELAY_NORMAL_US);

		*LCD_CMD_ADDR = 0x01;	/*display clear */
		mdelay(LCD_DELAY_NORMAL_MS);

		*LCD_CMD_ADDR = 0x06;	/*set entry */
		udelay(LCD_DELAY_NORMAL_US);

		lcd_bl_ctrl(0x02);
		lcd_put_logo();

		puts("  ready\n");
		g_lcd_init_b = 1;
	}

	return 0;
}

static int do_lcd_test(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	lcd_init();
	return 0;
}

static int do_lcd_clear(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	*LCD_CMD_ADDR = 0x01;
	mdelay(LCD_DELAY_NORMAL_MS);
	return 0;
}
static int do_lcd_puts(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}
	lcd_puts(argv[1]);
	return 0;
}
static int do_lcd_putc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}
	lcd_putc((char)argv[1][0]);
	return 0;
}
static int do_lcd_cur(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong count;
	ulong dir;
	char cur_addr;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	count = simple_strtoul(argv[1], NULL, 16);
	if (count > 31) {
		printf("unable to shift > 0x20\n");
		count = 0;
	}

	dir = simple_strtoul(argv[2], NULL, 16);
	cur_addr = *LCD_CMD_ADDR;
	udelay(LCD_DELAY_NORMAL_US);
	if (dir == 0x0) {
		if (addr_flag == 0x80) {
			if (count >= (cur_addr & 0xf)) {
				*LCD_CMD_ADDR = 0x80;
				udelay(LCD_DELAY_NORMAL_US);
				count = 0;
			}
		} else {
			if (count >= ((cur_addr & 0x0f) + 0x0f)) {
				*LCD_CMD_ADDR = 0x80;
				addr_flag = 0x80;
				udelay(LCD_DELAY_NORMAL_US);
				count = 0x0;
			} else if (count >= (cur_addr & 0xf)) {
				count -= cur_addr & 0xf;
				*LCD_CMD_ADDR = 0x80 | 0xf;
				addr_flag = 0x80;
				udelay(LCD_DELAY_NORMAL_US);
			}
		}
	} else {
		if (addr_flag == 0x80) {
			if (count >= (0x1f - (cur_addr & 0xf))) {
				count = 0x0;
				addr_flag = 0xc0;
				*LCD_CMD_ADDR = 0xc0 | 0xf;
				udelay(LCD_DELAY_NORMAL_US);
			} else if ((count + (cur_addr & 0xf)) >= 0x0f) {
				count = count + (cur_addr & 0xf) - 0x0f;
				addr_flag = 0xc0;
				*LCD_CMD_ADDR = 0xc0;
				udelay(LCD_DELAY_NORMAL_US);
			}
		} else if ((count + (cur_addr & 0xf)) >= 0x0f) {
			count = 0x0;
			*LCD_CMD_ADDR = 0xc0 | 0xf;
			udelay(LCD_DELAY_NORMAL_US);
		}
	}

	while (count--) {
		if (dir == 0) {
			*LCD_CMD_ADDR = 0x10;
		} else {
			*LCD_CMD_ADDR = 0x14;
		}
		udelay(LCD_DELAY_NORMAL_US);
	}

	return 0;
}

U_BOOT_CMD(lcd_test, 1, 1, do_lcd_test, "lcd test display", "");
U_BOOT_CMD(lcd_cls, 1, 1, do_lcd_clear, "lcd clear display", "");
U_BOOT_CMD(lcd_puts, 2, 1, do_lcd_puts,
	   "display string on lcd",
	   "<string> - <string> to be displayed");
U_BOOT_CMD(lcd_putc, 2, 1, do_lcd_putc,
	   "display char on lcd",
	   "<char> - <char> to be displayed");
U_BOOT_CMD(lcd_cur, 3, 1, do_lcd_cur,
	   "shift cursor on lcd",
	   "<count> <dir>- shift cursor on lcd <count> times, direction is <dir> \n"
	   " <count> - 0~31\n" " <dir> - 0,backward; 1, forward");

#if 0 /* test-only */
void set_phy_loopback_mode(void)
{
	char devemac2[32];
	char devemac3[32];

	sprintf(devemac2, "%s2", CONFIG_EMAC_DEV_NAME);
	sprintf(devemac3, "%s3", CONFIG_EMAC_DEV_NAME);

#if 0
	unsigned short reg_short;

	miiphy_read(devemac2, 0x1, 1, &reg_short);
	if (reg_short & 0x04) {
		/*
		 * printf("EMAC2 link up,do nothing\n");
		 */
	} else {
		udelay(1000);
		miiphy_write(devemac2, 0x1, 0, 0x6000);
		udelay(1000);
		miiphy_read(devemac2, 0x1, 0, &reg_short);
		if (reg_short != 0x6000) {
			printf
			    ("\nEMAC2 error set LOOPBACK mode error,reg2[0]=%x\n",
			     reg_short);
		}
	}

	miiphy_read(devemac3, 0x3, 1, &reg_short);
	if (reg_short & 0x04) {
		/*
		 * printf("EMAC3 link up,do nothing\n");
		 */
	} else {
		udelay(1000);
		miiphy_write(devemac3, 0x3, 0, 0x6000);
		udelay(1000);
		miiphy_read(devemac3, 0x3, 0, &reg_short);
		if (reg_short != 0x6000) {
			printf
			    ("\nEMAC3 error set LOOPBACK mode error,reg2[0]=%x\n",
			     reg_short);
		}
	}
#else
	/* Set PHY as LOOPBACK MODE, for Linux emac initializing */
	miiphy_write(devemac2, CONFIG_PHY2_ADDR, 0, 0x6000);
	udelay(1000);
	miiphy_write(devemac3, CONFIG_PHY3_ADDR, 0, 0x6000);
	udelay(1000);
#endif	/* 0 */
}

void set_phy_normal_mode(void)
{
	char devemac2[32];
	char devemac3[32];
	unsigned short reg_short;

	sprintf(devemac2, "%s2", CONFIG_EMAC_DEV_NAME);
	sprintf(devemac3, "%s3", CONFIG_EMAC_DEV_NAME);

	/* Set phy of EMAC2 */
	miiphy_read(devemac2, CONFIG_PHY2_ADDR, 0x16, &reg_short);
	reg_short &= ~(0x7);
	reg_short |= 0x6;	/* RGMII DLL Delay */
	miiphy_write(devemac2, CONFIG_PHY2_ADDR, 0x16, reg_short);

	miiphy_read(devemac2, CONFIG_PHY2_ADDR, 0x17, &reg_short);
	reg_short &= ~(0x40);
	miiphy_write(devemac2, CONFIG_PHY2_ADDR, 0x17, reg_short);

	miiphy_write(devemac2, CONFIG_PHY2_ADDR, 0x1c, 0x74f0);

	/* Set phy of EMAC3 */
	miiphy_read(devemac3, CONFIG_PHY3_ADDR, 0x16, &reg_short);
	reg_short &= ~(0x7);
	reg_short |= 0x6;	/* RGMII DLL Delay */
	miiphy_write(devemac3, CONFIG_PHY3_ADDR, 0x16, reg_short);

	miiphy_read(devemac3, CONFIG_PHY3_ADDR, 0x17, &reg_short);
	reg_short &= ~(0x40);
	miiphy_write(devemac3, CONFIG_PHY3_ADDR, 0x17, reg_short);

	miiphy_write(devemac3, CONFIG_PHY3_ADDR, 0x1c, 0x74f0);
}
#endif	/* 0 - test only */

static int do_led_test_off(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	volatile unsigned int *GpioOr =
		(volatile unsigned int *)(CONFIG_SYS_PERIPHERAL_BASE + 0x700);
	*GpioOr |= 0x00300000;
	return 0;
}

static int do_led_test_on(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	volatile unsigned int *GpioOr =
		(volatile unsigned int *)(CONFIG_SYS_PERIPHERAL_BASE + 0x700);
	*GpioOr &= ~0x00300000;
	return 0;
}

U_BOOT_CMD(ledon, 1, 1, do_led_test_on,
	   "led test light on", "");

U_BOOT_CMD(ledoff, 1, 1, do_led_test_off,
	   "led test light off", "");
#endif
