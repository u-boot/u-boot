/*
 * BF533-STAMP splash driver
 *
 * Copyright (c) 2006-2008 Analog Devices Inc.
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 * (C) Copyright 2002
 * Wolfgang Denk, wd@denx.de
 *
 * Licensed under the GPL-2 or later.
 */

#include <stdarg.h>
#include <common.h>
#include <config.h>
#include <malloc.h>
#include <asm/blackfin.h>
#include <asm/mach-common/bits/dma.h>
#include <i2c.h>
#include <linux/types.h>
#include <stdio_dev.h>

#define DMA_SIZE16	2

#include <asm/mach-common/bits/ppi.h>

#define NTSC_FRAME_ADDR 0x06000000
#include "video.h"

/* NTSC OUTPUT SIZE  720 * 240 */
#define VERTICAL	2
#define HORIZONTAL	4

int is_vblank_line(const int line)
{
	/*
	 *  This array contains a single bit for each line in
	 *  an NTSC frame.
	 */
	if ((line <= 18) || (line >= 264 && line <= 281) || (line == 528))
		return true;

	return false;
}

int NTSC_framebuffer_init(char *base_address)
{
	const int NTSC_frames = 1;
	const int NTSC_lines = 525;
	char *dest = base_address;
	int frame_num, line_num;

	for (frame_num = 0; frame_num < NTSC_frames; ++frame_num) {
		for (line_num = 1; line_num <= NTSC_lines; ++line_num) {
			unsigned int code;
			int offset = 0;
			int i;

			if (is_vblank_line(line_num))
				offset++;

			if (line_num > 266 || line_num < 3)
				offset += 2;

			/* Output EAV code */
			code = system_code_map[offset].eav;
			write_dest_byte((char)(code >> 24) & 0xff);
			write_dest_byte((char)(code >> 16) & 0xff);
			write_dest_byte((char)(code >> 8) & 0xff);
			write_dest_byte((char)(code) & 0xff);

			/* Output horizontal blanking */
			for (i = 0; i < 67 * 2; ++i) {
				write_dest_byte(0x80);
				write_dest_byte(0x10);
			}

			/* Output SAV */
			code = system_code_map[offset].sav;
			write_dest_byte((char)(code >> 24) & 0xff);
			write_dest_byte((char)(code >> 16) & 0xff);
			write_dest_byte((char)(code >> 8) & 0xff);
			write_dest_byte((char)(code) & 0xff);

			/* Output empty horizontal data */
			for (i = 0; i < 360 * 2; ++i) {
				write_dest_byte(0x80);
				write_dest_byte(0x10);
			}
		}
	}

	return dest - base_address;
}

void fill_frame(char *Frame, int Value)
{
	int *OddPtr32;
	int OddLine;
	int *EvenPtr32;
	int EvenLine;
	int i;
	int *data;
	int m, n;

	/* fill odd and even frames */
	for (OddLine = 22, EvenLine = 285; OddLine < 263; OddLine++, EvenLine++) {
		OddPtr32 = (int *)((Frame + (OddLine * 1716)) + 276);
		EvenPtr32 = (int *)((Frame + (EvenLine * 1716)) + 276);
		for (i = 0; i < 360; i++, OddPtr32++, EvenPtr32++) {
			*OddPtr32 = Value;
			*EvenPtr32 = Value;
		}
	}

	for (m = 0; m < VERTICAL; m++) {
		data = (int *)u_boot_logo.data;
		for (OddLine = (22 + m), EvenLine = (285 + m);
		     OddLine < (u_boot_logo.height * VERTICAL) + (22 + m);
		     OddLine += VERTICAL, EvenLine += VERTICAL) {
			OddPtr32 = (int *)((Frame + ((OddLine) * 1716)) + 276);
			EvenPtr32 =
			    (int *)((Frame + ((EvenLine) * 1716)) + 276);
			for (i = 0; i < u_boot_logo.width / 2; i++) {
				/* enlarge one pixel to m x n */
				for (n = 0; n < HORIZONTAL; n++) {
					*OddPtr32++ = *data;
					*EvenPtr32++ = *data;
				}
				data++;
			}
		}
	}
}

static void video_init(char *NTSCFrame)
{
	NTSC_framebuffer_init(NTSCFrame);
	fill_frame(NTSCFrame, BLUE);

	bfin_write_PPI_CONTROL(0x0082);
	bfin_write_PPI_FRAME(0x020D);

	bfin_write_DMA0_START_ADDR(NTSCFrame);
	bfin_write_DMA0_X_COUNT(0x035A);
	bfin_write_DMA0_X_MODIFY(0x0002);
	bfin_write_DMA0_Y_COUNT(0x020D);
	bfin_write_DMA0_Y_MODIFY(0x0002);
	bfin_write_DMA0_CONFIG(0x1015);
	bfin_write_PPI_CONTROL(0x0083);
}

void video_stop(void)
{
	bfin_write_PPI_CONTROL(0);
	bfin_write_DMA0_CONFIG(0);
}

int drv_video_init(void)
{
	struct stdio_dev videodev;

	video_init((void *)NTSC_FRAME_ADDR);

	memset(&videodev, 0, sizeof(videodev));
	strcpy(videodev.name, "video");

	return stdio_register(&videodev);
}
