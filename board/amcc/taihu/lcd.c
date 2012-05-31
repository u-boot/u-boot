/*
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

#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>

#define LCD_CMD_ADDR	0x50100002
#define LCD_DATA_ADDR	0x50100003
#define LCD_BLK_CTRL	CPLD_REG1_ADDR

static char *amcc_logo = "AMCC 405EP TAIHU EVALUATION KIT";
static int addr_flag = 0x80;

static void lcd_bl_ctrl(char val)
{
	out_8((u8 *) LCD_BLK_CTRL, in_8((u8 *) LCD_BLK_CTRL) | val);
}

static void lcd_putc(int val)
{
	int i = 100;
	char addr;

	while (i--) {
		if ((in_8((u8 *) LCD_CMD_ADDR) & 0x80) != 0x80) { /*BF = 1 ?*/
			udelay(50);
			break;
		}
		udelay(50);
	}

	if (in_8((u8 *) LCD_CMD_ADDR) & 0x80) {
		printf("LCD is busy\n");
		return;
	}

	addr = in_8((u8 *) LCD_CMD_ADDR);
	udelay(50);
	if ((addr != 0) && (addr % 0x10 == 0)) {
		addr_flag ^= 0x40;
		out_8((u8 *) LCD_CMD_ADDR, addr_flag);
	}

	udelay(50);
	out_8((u8 *) LCD_DATA_ADDR, val);
	udelay(50);
}

static void lcd_puts(char *s)
{
	char *p = s;
	int i = 100;

	while (i--) {
		if ((in_8((u8 *) LCD_CMD_ADDR) & 0x80) != 0x80) { /*BF = 1 ?*/
			udelay(50);
			break;
		}
		udelay(50);
	}

	if (in_8((u8 *) LCD_CMD_ADDR) & 0x80) {
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
		if ((in_8((u8 *) LCD_CMD_ADDR) & 0x80) != 0x80) { /*BF = 1 ?*/
			udelay(50);
			break;
		}
		udelay(50);
	}

	if (in_8((u8 *) LCD_CMD_ADDR) & 0x80) {
		printf("LCD is busy\n");
		return;
	}

	out_8((u8 *) LCD_CMD_ADDR, 0x80);
	while (*p)
		lcd_putc(*p++);
}

int lcd_init(void)
{
	puts("LCD: ");
	out_8((u8 *) LCD_CMD_ADDR, 0x38); /* set function:8-bit,2-line,5x7 font type */
	udelay(50);
	out_8((u8 *) LCD_CMD_ADDR, 0x0f); /* set display on,cursor on,blink on */
	udelay(50);
	out_8((u8 *) LCD_CMD_ADDR, 0x01); /* display clear */
	udelay(2000);
	out_8((u8 *) LCD_CMD_ADDR, 0x06); /* set entry */
	udelay(50);
	lcd_bl_ctrl(0x02);		/* set backlight on */
	lcd_put_logo();
	puts("ready\n");

	return 0;
}

static int do_lcd_clear (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	out_8((u8 *) LCD_CMD_ADDR, 0x01);
	udelay(2000);

	return 0;
}

static int do_lcd_puts (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	lcd_puts(argv[1]);

	return 0;
}

static int do_lcd_putc (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return cmd_usage(cmdtp);

	lcd_putc((char)argv[1][0]);

	return 0;
}

static int do_lcd_cur (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	ulong count;
	ulong dir;
	char cur_addr;

	if (argc < 3)
		return cmd_usage(cmdtp);

	count = simple_strtoul(argv[1], NULL, 16);
	if (count > 31) {
		printf("unable to shift > 0x20\n");
		count = 0;
	}

	dir = simple_strtoul(argv[2], NULL, 16);
	cur_addr = in_8((u8 *) LCD_CMD_ADDR);
	udelay(50);

	if (dir == 0x0) {
		if (addr_flag == 0x80) {
			if (count >= (cur_addr & 0xf)) {
				out_8((u8 *) LCD_CMD_ADDR, 0x80);
				udelay(50);
				count = 0;
			}
		} else {
			if (count >= ((cur_addr & 0x0f) + 0x0f)) {
				out_8((u8 *) LCD_CMD_ADDR, 0x80);
				addr_flag = 0x80;
				udelay(50);
				count = 0x0;
			} else if (count >= ( cur_addr & 0xf)) {
				count -= cur_addr & 0xf ;
				out_8((u8 *) LCD_CMD_ADDR, 0x80 | 0xf);
				addr_flag = 0x80;
				udelay(50);
			}
		}
	} else {
		if (addr_flag == 0x80) {
			if (count >= (0x1f - (cur_addr & 0xf))) {
				count = 0x0;
				addr_flag = 0xc0;
				out_8((u8 *) LCD_CMD_ADDR, 0xc0 | 0xf);
				udelay(50);
			} else if ((count + (cur_addr & 0xf ))>=  0x0f) {
				count = count + (cur_addr & 0xf) - 0x0f;
				addr_flag = 0xc0;
				out_8((u8 *) LCD_CMD_ADDR, 0xc0);
				udelay(50);
			}
		} else if ((count + (cur_addr & 0xf )) >= 0x0f) {
			count = 0x0;
			out_8((u8 *) LCD_CMD_ADDR, 0xC0 | 0x0F);
			udelay(50);
		}
	}
	while (count--) {
		if (dir == 0)
			out_8((u8 *) LCD_CMD_ADDR, 0x10);
		else
			out_8((u8 *) LCD_CMD_ADDR, 0x14);
		udelay(50);
	}

	return 0;
}

U_BOOT_CMD(
	lcd_cls, 1, 1, do_lcd_clear,
	"lcd clear display",
	""
);

U_BOOT_CMD(
	lcd_puts, 2, 1, do_lcd_puts,
	"display string on lcd",
	"<string> - <string> to be displayed"
);

U_BOOT_CMD(
	lcd_putc, 2, 1, do_lcd_putc,
	"display char on lcd",
	"<char> - <char> to be displayed"
);

U_BOOT_CMD(
	lcd_cur, 3, 1, do_lcd_cur,
	"shift cursor on lcd",
	"<count> <dir> - shift cursor on lcd <count> times, direction is <dir> \n"
	" <count> - 0..31\n"
	" <dir>   - 0=backward 1=forward"
);
