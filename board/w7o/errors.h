/*
 * (C) Copyright 2001
 * Bill Hunter, Wave 7 Optics, william.hunter@mediaone.net
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _ERRORS_H_
#define _ERRORS_H_

#define ERR_FF		-1	/* led test value(2) */
#define ERR_00		0x0000	/* led test value(2) */
#define ERR_LED		0x01	/* led test failed (1)(3)(4) */
#define ERR_RAMG	0x04	/* start SDRAM data bus test (2) */
#define ERR_RAML	0x05	/* SDRAM data bus fault in LSW chip (5) */
#define ERR_RAMH	0x06	/* SDRAM data bus fault in MSW chip (6) */
#define ERR_RAMB	0x07	/* SDRAM data bus fault both chips (5)(6)(7) */
#define ERR_ADDG	0x08	/* start Address ghosting test (13) */
#define ERR_ADDF	0x09	/* fault during Address ghosting test (13) */
#define ERR_POST1	0x0a	/* post1 tests complete */
#define ERR_TMP1	0x0b	/* */
#define ERR_R55G	0x0c	/* start SDRAM fill 55 test (2) */
#define ERR_R55L	0x0d	/* SDRAM fill test 55 failed in LSW chip (8) */
#define ERR_R55H	0x0e	/* SDRAM fill test 55 failed in MSW chip (9) */
#define ERR_R55B	0x0f	/* SDRAM fill test 55 fail in both chips (10) */
#define ERR_RAAG	0x10	/* start SDRAM fill aa test (2) */
#define ERR_RAAL	0x11	/* SDRAM fill test aa failed in LSW chip (8) */
#define ERR_RAAH	0x12	/* SDRAM fill test aa failed in MSW chip (9) */
#define ERR_RAAB	0x13	/* SDRAM fill test aa fail in both chips (10) */
#define ERR_R00G	0x14	/* start SDRAM fill 00 test (2) */
#define ERR_R00L	0x15	/* SDRAM fill test 00 failed in LSW chip (8) */
#define ERR_R00H	0x16	/* SDRAM fill test 00 failed in MSW chip (9) */
#define ERR_R00B	0x17	/* SDRAM fill test 00 fail in both chips (10) */
#define ERR_RTCG	0x18	/* start RTC test */
#define ERR_RTCBAT	0x19	/* RTC battery failure */
#define ERR_RTCTIM	0x1A	/* RTC invalid time/date values */
#define ERR_RTCVAL	0x1B	/* RTC NVRAM not accessable */
#define ERR_FPGAG	0x20	/* fault during FPGA programming */
#define ERR_XRW1	0x21	/* Xilinx - can't read/write regs on FPGA 1 */
#define ERR_XRW2	0x22	/* Xilinx - can't read/write regs on FPGA 2 */
#define ERR_XRW3	0x23	/* Xilinx - can't read/write regs on FPGA 3 */
#define ERR_XRW4	0x24	/* Xilinx - can't read/write regs on FPGA 4 */
#define ERR_XRW5	0x25	/* Xilinx - can't read/write regs on FPGA 5 */
#define ERR_XRW6	0x26	/* Xilinx - can't read/write regs on FPGA 6 */
#define ERR_XINIT0	0x27	/* Xilinx - INIT line failed to go low */
#define ERR_XINIT1	0x28	/* Xilinx - INIT line failed to go high */
#define ERR_XDONE1	0x29	/* Xilinx - DONE line failed to go high */
#define ERR_XIMAGE	0x2A	/* Xilinx - Bad FPGA image in Flash */
#define ERR_TempG	0x2b	/* start temp sensor tests */
#define ERR_Tinit0	0x2C	/* temp sensor 0 failed to init */
#define ERR_Tinit1	0x2D	/* temp sensor 1 failed to init */
#define ERR_Ttest0	0x2E	/* temp sensor 0 failed test */
#define ERR_Ttest1	0x2F	/* temp sensor 1 failed test */
#define ERR_lm75r	0x30	/* temp sensor read failure */
#define ERR_lm75w	0x31	/* temp sensor write failure */


#define ERR_POSTOK	0x55	/* PANIC: psych... OK */

#if !defined(__ASSEMBLY__)
extern void log_stat(int errcode);
extern void log_warn(int errcode);
extern void log_err(int errcode);
#endif

/*
Debugging suggestions:
(1) periferal data bus shorted or crossed
(2) general processor halt, check reset, watch dog, power supply ripple, processor clock.
(3) check p_we, p_r/w, p_oe, p_rdy lines.
(4) check LED buffers
(5) check SDRAM data bus bits 16-31, check LSW SDRAM chip.
(6) check SDRAM data bus bits 0-15, check MSW SDRAM chip.
(7) check SDRAM control lines and clocks
(8) check decoupling caps, replace LSW SDRAM
(9) check decoupling caps, replace MSW SDRAM
(10)
(11)
(12)
(13) SDRAM address shorted or unconnected, check sdram caps
*/
#endif /* _ERRORS_H_ */
