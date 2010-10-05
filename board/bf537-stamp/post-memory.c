#include <common.h>
#include <asm/io.h>

#include <post.h>
#include <watchdog.h>

#if CONFIG_POST & CONFIG_SYS_POST_MEMORY
#define CLKIN 25000000
#define PATTERN1 0x5A5A5A5A
#define PATTERN2 0xAAAAAAAA

#define CCLK_NUM	4
#define SCLK_NUM	3

void post_out_buff(char *buff);
int post_key_pressed(void);
void post_init_pll(int mult, int div);
int post_init_sdram(int sclk);
void post_init_uart(int sclk);

const int pll[CCLK_NUM][SCLK_NUM][2] = {
	{ {20, 4}, {20, 5}, {20, 10} },	/* CCLK = 500M */
	{ {16, 4}, {16, 5}, {16, 8} },	/* CCLK = 400M */
	{ {8, 2}, {8, 4}, {8, 5} },	/* CCLK = 200M */
	{ {4, 1}, {4, 2}, {4, 4} }	/* CCLK = 100M */
};
const char *const log[CCLK_NUM][SCLK_NUM] = {
	{"CCLK-500MHz SCLK-125MHz:    Writing...\0",
	 "CCLK-500MHz SCLK-100MHz:    Writing...\0",
	 "CCLK-500MHz SCLK- 50MHz:    Writing...\0",},
	{"CCLK-400MHz SCLK-100MHz:    Writing...\0",
	 "CCLK-400MHz SCLK- 80MHz:    Writing...\0",
	 "CCLK-400MHz SCLK- 50MHz:    Writing...\0",},
	{"CCLK-200MHz SCLK-100MHz:    Writing...\0",
	 "CCLK-200MHz SCLK- 50MHz:    Writing...\0",
	 "CCLK-200MHz SCLK- 40MHz:    Writing...\0",},
	{"CCLK-100MHz SCLK-100MHz:    Writing...\0",
	 "CCLK-100MHz SCLK- 50MHz:    Writing...\0",
	 "CCLK-100MHz SCLK- 25MHz:    Writing...\0",},
};

int memory_post_test(int flags)
{
	int addr;
	int m, n;
	int sclk, sclk_temp;
	int ret = 1;

	sclk_temp = CLKIN / 1000000;
	sclk_temp = sclk_temp * CONFIG_VCO_MULT;
	for (sclk = 0; sclk_temp > 0; sclk++)
		sclk_temp -= CONFIG_SCLK_DIV;
	sclk = sclk * 1000000;
	post_init_uart(sclk);
	if (post_key_pressed() == 0)
		return 0;

	for (m = 0; m < CCLK_NUM; m++) {
		for (n = 0; n < SCLK_NUM; n++) {
			/* Calculate the sclk */
			sclk_temp = CLKIN / 1000000;
			sclk_temp = sclk_temp * pll[m][n][0];
			for (sclk = 0; sclk_temp > 0; sclk++)
				sclk_temp -= pll[m][n][1];
			sclk = sclk * 1000000;

			post_init_pll(pll[m][n][0], pll[m][n][1]);
			post_init_sdram(sclk);
			post_init_uart(sclk);
			post_out_buff("\n\r\0");
			post_out_buff(log[m][n]);
			for (addr = 0x0; addr < CONFIG_SYS_MAX_RAM_SIZE; addr += 4)
				*(unsigned long *)addr = PATTERN1;
			post_out_buff("Reading...\0");
			for (addr = 0x0; addr < CONFIG_SYS_MAX_RAM_SIZE; addr += 4) {
				if ((*(unsigned long *)addr) != PATTERN1) {
					post_out_buff("Error\n\r\0");
					ret = 0;
				}
			}
			post_out_buff("OK\n\r\0");
		}
	}
	if (ret)
		post_out_buff("memory POST passed\n\r\0");
	else
		post_out_buff("memory POST failed\n\r\0");

	post_out_buff("\n\r\n\r\0");
	return 1;
}

void post_init_uart(int sclk)
{
	int divisor;

	for (divisor = 0; sclk > 0; divisor++)
		sclk -= 57600 * 16;

	bfin_write_PORTF_FER(0x000F);
	bfin_write_PORTH_FER(0xFFFF);

	bfin_write_UART_GCTL(0x00);
	bfin_write_UART_LCR(0x83);
	SSYNC();
	bfin_write_UART_DLL(divisor & 0xFF);
	SSYNC();
	bfin_write_UART_DLH((divisor >> 8) & 0xFF);
	SSYNC();
	bfin_write_UART_LCR(0x03);
	SSYNC();
	bfin_write_UART_GCTL(0x01);
	SSYNC();
}

void post_out_buff(char *buff)
{

	int i = 0;
	for (i = 0; i < 0x80000; i++)
		;
	i = 0;
	while ((buff[i] != '\0') && (i != 100)) {
		while (!(bfin_read_pUART_LSR() & 0x20)) ;
		bfin_write_UART_THR(buff[i]);
		SSYNC();
		i++;
	}
	for (i = 0; i < 0x80000; i++)
		;
}

/* Using sw10-PF5 as the hotkey */
#define KEY_LOOP 0x80000
#define KEY_DELAY 0x80
int post_key_pressed(void)
{
	int i, n;
	unsigned short value;

	bfin_write_PORTF_FER(bfin_read_PORTF_FER() & ~PF5);
	bfin_write_PORTFIO_DIR(bfin_read_PORTFIO_DIR() & ~PF5);
	bfin_write_PORTFIO_INEN(bfin_read_PORTFIO_INEN() | PF5);
	SSYNC();

	post_out_buff("########Press SW10 to enter Memory POST########: 3\0");
	for (i = 0; i < KEY_LOOP; i++) {
		value = bfin_read_PORTFIO() & PF5;
		if (bfin_read_UART0_RBR() == 0x0D) {
			value = 0;
			goto key_pressed;
		}
		if (value != 0)
			goto key_pressed;
		for (n = 0; n < KEY_DELAY; n++)
			asm("nop");
	}
	post_out_buff("\b2\0");

	for (i = 0; i < KEY_LOOP; i++) {
		value = bfin_read_PORTFIO() & PF5;
		if (bfin_read_UART0_RBR() == 0x0D) {
			value = 0;
			goto key_pressed;
		}
		if (value != 0)
			goto key_pressed;
		for (n = 0; n < KEY_DELAY; n++)
			asm("nop");
	}
	post_out_buff("\b1\0");

	for (i = 0; i < KEY_LOOP; i++) {
		value = bfin_read_PORTFIO() & PF5;
		if (bfin_read_UART0_RBR() == 0x0D) {
			value = 0;
			goto key_pressed;
		}
		if (value != 0)
			goto key_pressed;
		for (n = 0; n < KEY_DELAY; n++)
			asm("nop");
	}
      key_pressed:
	post_out_buff("\b0");
	post_out_buff("\n\r\0");
	if (value == 0)
		return 0;
	post_out_buff("Hotkey has been pressed, Enter POST . . . . . .\n\r\0");
	return 1;
}

void post_init_pll(int mult, int div)
{

	bfin_write_SIC_IWR(0x01);
	bfin_write_PLL_CTL((mult << 9));
	bfin_write_PLL_DIV(div);
	asm("CLI R2;");
	asm("IDLE;");
	asm("STI R2;");
	while (!(bfin_read_PLL_STAT() & 0x20)) ;
}

int post_init_sdram(int sclk)
{
	int SDRAM_tRP, SDRAM_tRP_num, SDRAM_tRAS, SDRAM_tRAS_num, SDRAM_tRCD,
	    SDRAM_tWR;
	int SDRAM_Tref, SDRAM_NRA, SDRAM_CL, SDRAM_SIZE, SDRAM_WIDTH,
	    mem_SDGCTL, mem_SDBCTL, mem_SDRRC;

	if ((sclk > 119402985)) {
		SDRAM_tRP = TRP_2;
		SDRAM_tRP_num = 2;
		SDRAM_tRAS = TRAS_7;
		SDRAM_tRAS_num = 7;
		SDRAM_tRCD = TRCD_2;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 104477612) && (sclk <= 119402985)) {
		SDRAM_tRP = TRP_2;
		SDRAM_tRP_num = 2;
		SDRAM_tRAS = TRAS_6;
		SDRAM_tRAS_num = 6;
		SDRAM_tRCD = TRCD_2;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 89552239) && (sclk <= 104477612)) {
		SDRAM_tRP = TRP_2;
		SDRAM_tRP_num = 2;
		SDRAM_tRAS = TRAS_5;
		SDRAM_tRAS_num = 5;
		SDRAM_tRCD = TRCD_2;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 74626866) && (sclk <= 89552239)) {
		SDRAM_tRP = TRP_2;
		SDRAM_tRP_num = 2;
		SDRAM_tRAS = TRAS_4;
		SDRAM_tRAS_num = 4;
		SDRAM_tRCD = TRCD_2;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 66666667) && (sclk <= 74626866)) {
		SDRAM_tRP = TRP_2;
		SDRAM_tRP_num = 2;
		SDRAM_tRAS = TRAS_3;
		SDRAM_tRAS_num = 3;
		SDRAM_tRCD = TRCD_2;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 59701493) && (sclk <= 66666667)) {
		SDRAM_tRP = TRP_1;
		SDRAM_tRP_num = 1;
		SDRAM_tRAS = TRAS_4;
		SDRAM_tRAS_num = 4;
		SDRAM_tRCD = TRCD_1;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 44776119) && (sclk <= 59701493)) {
		SDRAM_tRP = TRP_1;
		SDRAM_tRP_num = 1;
		SDRAM_tRAS = TRAS_3;
		SDRAM_tRAS_num = 3;
		SDRAM_tRCD = TRCD_1;
		SDRAM_tWR = TWR_2;
	} else if ((sclk > 29850746) && (sclk <= 44776119)) {
		SDRAM_tRP = TRP_1;
		SDRAM_tRP_num = 1;
		SDRAM_tRAS = TRAS_2;
		SDRAM_tRAS_num = 2;
		SDRAM_tRCD = TRCD_1;
		SDRAM_tWR = TWR_2;
	} else if (sclk <= 29850746) {
		SDRAM_tRP = TRP_1;
		SDRAM_tRP_num = 1;
		SDRAM_tRAS = TRAS_1;
		SDRAM_tRAS_num = 1;
		SDRAM_tRCD = TRCD_1;
		SDRAM_tWR = TWR_2;
	} else {
		SDRAM_tRP = TRP_1;
		SDRAM_tRP_num = 1;
		SDRAM_tRAS = TRAS_1;
		SDRAM_tRAS_num = 1;
		SDRAM_tRCD = TRCD_1;
		SDRAM_tWR = TWR_2;
	}
	/*SDRAM INFORMATION: */
	SDRAM_Tref = 64;	/* Refresh period in milliseconds */
	SDRAM_NRA = 4096;	/* Number of row addresses in SDRAM */
	SDRAM_CL = CL_3;	/* 2 */

	SDRAM_SIZE = EBSZ_64;
	SDRAM_WIDTH = EBCAW_10;

	mem_SDBCTL = SDRAM_WIDTH | SDRAM_SIZE | EBE;

	/* Equation from section 17 (p17-46) of BF533 HRM */
	mem_SDRRC =
	    (((CONFIG_SCLK_HZ / 1000) * SDRAM_Tref) / SDRAM_NRA) -
	    (SDRAM_tRAS_num + SDRAM_tRP_num);

	/* Enable SCLK Out */
	mem_SDGCTL =
	    (SCTLE | SDRAM_CL | SDRAM_tRAS | SDRAM_tRP | SDRAM_tRCD | SDRAM_tWR
	     | PSS);

	SSYNC();

	bfin_write_EBIU_SDGCTL(bfin_write_EBIU_SDGCTL() | 0x1000000);
	/* Set the SDRAM Refresh Rate control register based on SSCLK value */
	bfin_write_EBIU_SDRRC(mem_SDRRC);

	/* SDRAM Memory Bank Control Register */
	bfin_write_EBIU_SDBCTL(mem_SDBCTL);

	/* SDRAM Memory Global Control Register */
	bfin_write_EBIU_SDGCTL(mem_SDGCTL);
	SSYNC();
	return mem_SDRRC;
}

#endif				/* CONFIG_POST & CONFIG_SYS_POST_MEMORY */
