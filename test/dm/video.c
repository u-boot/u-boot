// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <bzlib.h>
#include <dm.h>
#include <gzip.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <os.h>
#include <video.h>
#include <video_console.h>
#include <asm/test.h>
#include <asm/sdl.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>
#include <test/video.h>

/*
 * These tests use the standard sandbox frame buffer, the resolution of which
 * is defined in the device tree. This only supports 16bpp so the tests only
 * test that code path. It would be possible to adjust this fairly easily,
 * by adjusting the bpix value in struct sandbox_sdl_plat. However the code
 * in sandbox_sdl_sync() would also need to change to handle the different
 * surface depth.
 */
/* Basic test of the video uclass */
static int dm_test_video_base(struct unit_test_state *uts)
{
	struct video_priv *priv;
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_VIDEO, 0, &dev));
	ut_asserteq(1366, video_get_xsize(dev));
	ut_asserteq(768, video_get_ysize(dev));
	priv = dev_get_uclass_priv(dev);
	ut_asserteq(priv->fb_size, 1366 * 768 * 2);

	return 0;
}
DM_TEST(dm_test_video_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

int video_compress_fb(struct unit_test_state *uts, struct udevice *dev,
		      bool use_copy)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);
	uint destlen;
	void *dest;
	int ret;

	if (!IS_ENABLED(CONFIG_VIDEO_COPY))
		use_copy = false;

	destlen = priv->fb_size;
	dest = malloc(priv->fb_size);
	if (!dest)
		return -ENOMEM;
	ret = BZ2_bzBuffToBuffCompress(dest, &destlen,
				       use_copy ? priv->copy_fb : priv->fb,
				       priv->fb_size,
				       3, 0, 0);
	free(dest);
	if (ret)
		return ret;

	return destlen;
}

int video_check_copy_fb(struct unit_test_state *uts, struct udevice *dev)
{
	struct video_priv *priv = dev_get_uclass_priv(dev);

	if (!IS_ENABLED(CONFIG_VIDEO_COPY))
		return 0;

	video_sync(dev, false);
	ut_assertf(!memcmp(priv->fb, priv->copy_fb, priv->fb_size),
		   "Copy framebuffer does not match fb");

	return 0;
}

/*
 * Call this function at any point to halt and show the current display. Be
 * sure to run the test with the -l flag.
 */
static void __maybe_unused see_output(void)
{
	video_sync_all();
	while (1);
}

/* Select the video console driver to use for a video device */
static int select_vidconsole(struct unit_test_state *uts, const char *drv_name)
{
	struct sandbox_sdl_plat *plat;
	struct udevice *dev;

	ut_assertok(uclass_find_device(UCLASS_VIDEO, 0, &dev));
	ut_assert(!device_active(dev));
	plat = dev_get_plat(dev);
	plat->vidconsole_drv_name = "vidconsole0";

	return 0;
}

/**
 * video_get_nologo() - Disable the logo on the video device and return it
 *
 * @uts: Test state
 * @devp: Returns video device
 * Return: 0 if OK, -ve on error
 */
static int video_get_nologo(struct unit_test_state *uts, struct udevice **devp)
{
	struct video_uc_plat *uc_plat;
	struct udevice *dev;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	uc_plat = dev_get_uclass_plat(dev);
	uc_plat->hide_logo = true;

	/* now probe it */
	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	*devp = dev;

	return 0;
}

/* Test text output works on the video console */
static int dm_test_video_text(struct unit_test_state *uts)
{
	struct udevice *dev, *con;
	int i;

#define WHITE		0xffff
#define SCROLL_LINES	100

	ut_assertok(select_vidconsole(uts, "vidconsole0"));
	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	ut_assertok(vidconsole_select_font(con, "8x16", 0));
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_putc_xy(con, 0, 0, 'a');
	ut_asserteq(79, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	vidconsole_putc_xy(con, 0, 0, ' ');
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	for (i = 0; i < 20; i++)
		vidconsole_putc_xy(con, VID_TO_POS(i * 8), 0, ' ' + i);
	ut_asserteq(273, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	vidconsole_set_row(con, 0, WHITE);
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	for (i = 0; i < 20; i++)
		vidconsole_putc_xy(con, VID_TO_POS(i * 8), 0, ' ' + i);
	ut_asserteq(273, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_text, UTF_SCAN_PDATA | UTF_SCAN_FDT);

static int dm_test_video_text_12x22(struct unit_test_state *uts)
{
	struct udevice *dev, *con;
	int i;

#define WHITE		0xffff
#define SCROLL_LINES	100

	ut_assertok(select_vidconsole(uts, "vidconsole0"));
	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	ut_assertok(vidconsole_select_font(con, "12x22", 0));
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_putc_xy(con, 0, 0, 'a');
	ut_asserteq(89, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	vidconsole_putc_xy(con, 0, 0, ' ');
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	for (i = 0; i < 20; i++)
		vidconsole_putc_xy(con, VID_TO_POS(i * 8), 0, ' ' + i);
	ut_asserteq(363, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	vidconsole_set_row(con, 0, WHITE);
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	for (i = 0; i < 20; i++)
		vidconsole_putc_xy(con, VID_TO_POS(i * 8), 0, ' ' + i);
	ut_asserteq(363, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_text_12x22, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test handling of special characters in the console */
static int dm_test_video_chars(struct unit_test_state *uts)
{
	struct udevice *dev, *con;
	const char *test_string = "Well\b\b\b\bxhe is\r \n\ta very \amodest  \bman\n\t\tand Has much to\b\bto be modest about.";

	ut_assertok(select_vidconsole(uts, "vidconsole0"));
	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	ut_assertok(vidconsole_select_font(con, "8x16", 0));
	vidconsole_put_string(con, test_string);
	ut_asserteq(466, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_chars, UTF_SCAN_PDATA | UTF_SCAN_FDT);

#ifdef CONFIG_VIDEO_ANSI
#define ANSI_ESC "\x1b"
/* Test handling of ANSI escape sequences */
static int dm_test_video_ansi(struct unit_test_state *uts)
{
	struct udevice *dev, *con;

	ut_assertok(select_vidconsole(uts, "vidconsole0"));
	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	ut_assertok(vidconsole_select_font(con, "8x16", 0));

	/* reference clear: */
	video_clear(con->parent);
	video_sync(con->parent, false);
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* test clear escape sequence: [2J */
	vidconsole_put_string(con, "A\tB\tC"ANSI_ESC"[2J");
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* test set-cursor: [%d;%df */
	vidconsole_put_string(con, "abc"ANSI_ESC"[2;2fab"ANSI_ESC"[4;4fcd");
	ut_asserteq(143, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* test colors (30-37 fg color, 40-47 bg color) */
	vidconsole_put_string(con, ANSI_ESC"[30;41mfoo"); /* black on red */
	vidconsole_put_string(con, ANSI_ESC"[33;44mbar"); /* yellow on blue */
	ut_asserteq(272, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_ansi, UTF_SCAN_PDATA | UTF_SCAN_FDT);
#endif

/**
 * check_vidconsole_output() - Run a text console test
 *
 * @uts:	Test state
 * @rot:	Console rotation (0=normal orientation, 1=90 degrees clockwise,
 *		2=upside down, 3=90 degree counterclockwise)
 * @wrap_size:	Expected size of compressed frame buffer for the wrap test
 * @scroll_size: Same for the scroll test
 * Return: 0 on success
 */
static int check_vidconsole_output(struct unit_test_state *uts, int rot,
				   int wrap_size, int scroll_size)
{
	struct udevice *dev, *con;
	struct sandbox_sdl_plat *plat;
	int i;

	ut_assertok(uclass_find_device(UCLASS_VIDEO, 0, &dev));
	ut_assert(!device_active(dev));
	plat = dev_get_plat(dev);
	plat->rot = rot;

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	ut_assertok(vidconsole_select_font(con, "8x16", 0));
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* Check display wrap */
	for (i = 0; i < 120; i++)
		vidconsole_put_char(con, 'A' + i % 50);
	ut_asserteq(wrap_size, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* Check display scrolling */
	for (i = 0; i < SCROLL_LINES; i++) {
		vidconsole_put_char(con, 'A' + i % 50);
		vidconsole_put_char(con, '\n');
	}
	ut_asserteq(scroll_size, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* If we scroll enough, the screen becomes blank again */
	for (i = 0; i < SCROLL_LINES; i++)
		vidconsole_put_char(con, '\n');
	ut_asserteq(46, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}

/* Test text output through the console uclass */
static int dm_test_video_context(struct unit_test_state *uts)
{
	ut_assertok(select_vidconsole(uts, "vidconsole0"));
	ut_assertok(check_vidconsole_output(uts, 0, 788, 453));

	return 0;
}
DM_TEST(dm_test_video_context, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test rotated text output through the console uclass */
static int dm_test_video_rotation1(struct unit_test_state *uts)
{
	ut_assertok(check_vidconsole_output(uts, 1, 1112, 680));

	return 0;
}
DM_TEST(dm_test_video_rotation1, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test rotated text output through the console uclass */
static int dm_test_video_rotation2(struct unit_test_state *uts)
{
	ut_assertok(check_vidconsole_output(uts, 2, 783, 445));

	return 0;
}
DM_TEST(dm_test_video_rotation2, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test rotated text output through the console uclass */
static int dm_test_video_rotation3(struct unit_test_state *uts)
{
	ut_assertok(check_vidconsole_output(uts, 3, 1134, 681));

	return 0;
}
DM_TEST(dm_test_video_rotation3, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Read a file into memory and return a pointer to it */
static int read_file(struct unit_test_state *uts, const char *fname,
		     ulong *addrp)
{
	int buf_size = 100000;
	ulong addr = 0;
	int size, fd;
	char *buf;

	buf = map_sysmem(addr, 0);
	ut_assert(buf != NULL);
	fd = os_open(fname, OS_O_RDONLY);
	ut_assert(fd >= 0);
	size = os_read(fd, buf, buf_size);
	os_close(fd);
	ut_assert(size >= 0);
	ut_assert(size < buf_size);
	*addrp = addr;

	return 0;
}

/* Test drawing a bitmap file */
static int dm_test_video_bmp(struct unit_test_state *uts)
{
	struct udevice *dev;
	ulong addr;

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(read_file(uts, "tools/logos/denx.bmp", &addr));

	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));
	ut_asserteq(1368, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a bitmap file on a 8bpp display */
static int dm_test_video_bmp8(struct unit_test_state *uts)
{
	struct udevice *dev;
	ulong addr;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP8));

	ut_assertok(read_file(uts, "tools/logos/denx.bmp", &addr));

	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));
	ut_asserteq(1247, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp8, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a bitmap file on a 16bpp display */
static int dm_test_video_bmp16(struct unit_test_state *uts)
{
	ulong src, src_len = ~0UL;
	uint dst_len = ~0U;
	struct udevice *dev;
	ulong dst = 0x10000;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP16));

	ut_assertok(read_file(uts, "tools/logos/denx-16bpp.bmp.gz", &src));
	ut_assertok(gunzip(map_sysmem(dst, 0), dst_len, map_sysmem(src, 0),
			   &src_len));

	ut_assertok(video_bmp_display(dev, dst, 0, 0, false));
	ut_asserteq(3700, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp16, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a 24bpp bitmap file on a 16bpp display */
static int dm_test_video_bmp24(struct unit_test_state *uts)
{
	ulong src, src_len = ~0UL;
	uint dst_len = ~0U;
	struct udevice *dev;
	ulong dst = 0x10000;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP16));

	ut_assertok(read_file(uts, "tools/logos/denx-24bpp.bmp.gz", &src));
	ut_assertok(gunzip(map_sysmem(dst, 0), dst_len, map_sysmem(src, 0),
			   &src_len));

	ut_assertok(video_bmp_display(dev, dst, 0, 0, false));
	ut_asserteq(3656, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp24, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a 24bpp bitmap file on a 32bpp display */
static int dm_test_video_bmp24_32(struct unit_test_state *uts)
{
	ulong src, src_len = ~0UL;
	uint dst_len = ~0U;
	struct udevice *dev;
	ulong dst = 0x10000;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP32));

	ut_assertok(read_file(uts, "tools/logos/denx-24bpp.bmp.gz", &src));
	ut_assertok(gunzip(map_sysmem(dst, 0), dst_len, map_sysmem(src, 0),
			   &src_len));

	ut_assertok(video_bmp_display(dev, dst, 0, 0, false));
	ut_asserteq(6827, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp24_32, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a bitmap file on a 32bpp display */
static int dm_test_video_bmp32(struct unit_test_state *uts)
{
	struct udevice *dev;
	ulong addr;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP32));
	ut_assertok(read_file(uts, "tools/logos/denx.bmp", &addr));

	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));
	ut_asserteq(2024, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp32, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a compressed bitmap file */
static int dm_test_video_bmp_comp(struct unit_test_state *uts)
{
	struct udevice *dev;
	ulong addr;

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(read_file(uts, "tools/logos/denx-comp.bmp", &addr));

	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));
	ut_asserteq(1368, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_bmp_comp, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a bitmap file on a 32bpp display */
static int dm_test_video_comp_bmp32(struct unit_test_state *uts)
{
	struct udevice *dev;
	ulong addr;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP32));

	ut_assertok(read_file(uts, "tools/logos/denx.bmp", &addr));

	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));
	ut_asserteq(2024, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_comp_bmp32, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test drawing a bitmap file on a 8bpp display */
static int dm_test_video_comp_bmp8(struct unit_test_state *uts)
{
	struct udevice *dev;
	ulong addr;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	ut_assertok(sandbox_sdl_set_bpp(dev, VIDEO_BPP8));

	ut_assertok(read_file(uts, "tools/logos/denx.bmp", &addr));

	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));
	ut_asserteq(1247, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_comp_bmp8, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test TrueType console */
static int dm_test_video_truetype(struct unit_test_state *uts)
{
	struct udevice *dev, *con;
	const char *test_string = "Criticism may not be agreeable, but it is necessary. It fulfils the same function as pain in the human body. It calls attention to an unhealthy state of things. Some see private enterprise as a predatory target to be shot, others as a cow to be milked, but few are those who see it as a sturdy horse pulling the wagon. The \aprice OF\b\bof greatness\n\tis responsibility.\n\nBye";

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_put_string(con, test_string);
	vidconsole_put_stringn(con, test_string, 30);
	ut_asserteq(13184, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_truetype, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test scrolling TrueType console */
static int dm_test_video_truetype_scroll(struct unit_test_state *uts)
{
	struct sandbox_sdl_plat *plat;
	struct udevice *dev, *con;
	const char *test_string = "Criticism may not be agreeable, but it is necessary. It fulfils the same function as pain in the human body. It calls attention to an unhealthy state of things. Some see private enterprise as a predatory target to be shot, others as a cow to be milked, but few are those who see it as a sturdy horse pulling the wagon. The \aprice OF\b\bof greatness\n\tis responsibility.\n\nBye";

	ut_assertok(uclass_find_device(UCLASS_VIDEO, 0, &dev));
	ut_assert(!device_active(dev));
	plat = dev_get_plat(dev);
	plat->font_size = 100;

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_put_string(con, test_string);
	ut_asserteq(34287, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_truetype_scroll, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test TrueType backspace, within and across lines */
static int dm_test_video_truetype_bs(struct unit_test_state *uts)
{
	struct sandbox_sdl_plat *plat;
	struct udevice *dev, *con;
	const char *test_string = "...Criticism may or may\b\b\b\b\b\bnot be agreeable, but seldom it is necessary\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bit is necessary. It fulfils the same function as pain in the human body. It calls attention to an unhealthy state of things.";

	ut_assertok(uclass_find_device(UCLASS_VIDEO, 0, &dev));
	ut_assert(!device_active(dev));
	plat = dev_get_plat(dev);
	plat->font_size = 100;

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_put_string(con, test_string);
	ut_asserteq(29471, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_truetype_bs, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test partial rendering onto hardware frame buffer */
static int dm_test_video_copy(struct unit_test_state *uts)
{
	struct sandbox_sdl_plat *plat;
	struct video_uc_plat *uc_plat;
	struct udevice *dev, *con;
	struct video_priv *priv;
	const char *test_string = "\n\tCriticism may not be agreeable, but it is necessary.\t";
	ulong addr;

	if (!IS_ENABLED(CONFIG_VIDEO_COPY))
		return -EAGAIN;

	ut_assertok(uclass_find_first_device(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	uc_plat = dev_get_uclass_plat(dev);
	uc_plat->hide_logo = true;
	plat = dev_get_plat(dev);
	plat->font_size = 32;
	ut_assert(!device_active(dev));
	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	ut_assertnonnull(dev);
	priv = dev_get_uclass_priv(dev);

	ut_assertok(read_file(uts, "tools/logos/denx.bmp", &addr));
	ut_assertok(video_bmp_display(dev, addr, 0, 0, false));

	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_put_string(con, "\n\n\n\n\n");
	vidconsole_put_string(con, test_string);
	vidconsole_put_string(con, test_string);

	ut_asserteq(6678, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/*
	 * Secretly clear the hardware frame buffer, but in a different
	 * color (black) to see which parts will be overwritten.
	 */
	memset(priv->copy_fb, 0, priv->fb_size);

	/*
	 * We should have the full content on the main buffer, but only
	 * 'damage' should have been copied to the copy buffer. This consists
	 * of a while rectangle with the Denx logo and four lines of text. The
	 * rest of the display is black.
	 *
	 * An easy way to try this is by changing video_sync() to call
	 * sandbox_sdl_sync(priv->copy_fb) instead of priv->fb then running the
	 * unit test:
	 *
	 *   ./u-boot -Tl
	 *   ut dm dm_test_video_copy
	 */
	vidconsole_put_string(con, test_string);
	vidconsole_put_string(con, test_string);
	video_sync(dev, true);
	ut_asserteq(7589, video_compress_fb(uts, dev, false));
	ut_asserteq(7704, video_compress_fb(uts, dev, true));

	return 0;
}
DM_TEST(dm_test_video_copy, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test video damage tracking */
static int dm_test_video_damage(struct unit_test_state *uts)
{
	struct sandbox_sdl_plat *plat;
	struct udevice *dev, *con;
	struct video_priv *priv;
	const char *test_string_1 = "Criticism may not be agreeable, ";
	const char *test_string_2 = "but it is necessary.";
	const char *test_string_3 = "It fulfils the same function as pain in the human body.";

	if (!IS_ENABLED(CONFIG_VIDEO_DAMAGE))
		return -EAGAIN;

	ut_assertok(uclass_find_device(UCLASS_VIDEO, 0, &dev));
	ut_assert(!device_active(dev));
	plat = dev_get_plat(dev);
	plat->font_size = 32;

	ut_assertok(video_get_nologo(uts, &dev));
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	priv = dev_get_uclass_priv(dev);

	vidconsole_position_cursor(con, 14, 10);
	vidconsole_put_string(con, test_string_2);
	ut_asserteq(449, priv->damage.xstart);
	ut_asserteq(325, priv->damage.ystart);
	ut_asserteq(661, priv->damage.xend);
	ut_asserteq(350, priv->damage.yend);

	vidconsole_position_cursor(con, 7, 5);
	vidconsole_put_string(con, test_string_1);
	ut_asserteq(225, priv->damage.xstart);
	ut_asserteq(164, priv->damage.ystart);
	ut_asserteq(661, priv->damage.xend);
	ut_asserteq(350, priv->damage.yend);

	vidconsole_position_cursor(con, 21, 15);
	vidconsole_put_string(con, test_string_3);
	ut_asserteq(225, priv->damage.xstart);
	ut_asserteq(164, priv->damage.ystart);
	ut_asserteq(1280, priv->damage.xend);
	ut_asserteq(510, priv->damage.yend);

	video_sync(dev, true);
	ut_asserteq(priv->xsize, priv->damage.xstart);
	ut_asserteq(priv->ysize, priv->damage.ystart);
	ut_asserteq(0, priv->damage.xend);
	ut_asserteq(0, priv->damage.yend);

	ut_asserteq(7339, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_damage, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test font measurement */
static int dm_test_font_measure(struct unit_test_state *uts)
{
	const char *test_string = "There is always much\nto be said for not "
		"attempting more than you can do and for making a certainty of "
		"what you try. But this principle, like others in life and "
		"war, has its exceptions.";
	const struct vidconsole_mline *line;
	struct vidconsole_bbox bbox;
	struct video_priv *priv;
	struct udevice *dev, *con;
	const int limit = 0x320;
	struct alist lines;
	int nl;

	ut_assertok(uclass_get_device(UCLASS_VIDEO, 0, &dev));
	priv = dev_get_uclass_priv(dev);
	ut_asserteq(1366, priv->xsize);
	ut_asserteq(768, priv->ysize);

	/* this is using the Nimbus font with size of 18 pixels */
	ut_assertok(uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con));
	vidconsole_position_cursor(con, 0, 0);
	alist_init_struct(&lines, struct vidconsole_mline);
	ut_assertok(vidconsole_measure(con, NULL, 0, test_string, -1, &bbox,
				       &lines));
	ut_asserteq(0, bbox.x0);
	ut_asserteq(0, bbox.y0);
	ut_asserteq(0x3ea, bbox.x1);
	ut_asserteq(0x24, bbox.y1);
	ut_asserteq(2, lines.count);

	nl = strchr(test_string, '\n') - test_string;

	line = alist_get(&lines, 0, struct vidconsole_mline);
	ut_assertnonnull(line);
	ut_asserteq(0, line->bbox.x0);
	ut_asserteq(0, line->bbox.y0);
	ut_asserteq(0x8c, line->bbox.x1);
	ut_asserteq(0x12, line->bbox.y1);
	ut_asserteq(0, line->start);
	ut_asserteq(20, line->len);
	ut_asserteq(nl, line->len);

	line++;
	ut_asserteq(0x0, line->bbox.x0);
	ut_asserteq(0x12, line->bbox.y0);
	ut_asserteq(0x3ea, line->bbox.x1);
	ut_asserteq(0x24, line->bbox.y1);
	ut_asserteq(21, line->start);
	ut_asserteq(nl + 1, line->start);
	ut_asserteq(163, line->len);
	ut_asserteq(strlen(test_string + nl + 1), line->len);

	/* now use a limit on the width */
	ut_assertok(vidconsole_measure(con, NULL, 0, test_string, limit, &bbox,
				       &lines));
	ut_asserteq(0, bbox.x0);
	ut_asserteq(0, bbox.y0);
	ut_asserteq(0x31e, bbox.x1);
	ut_asserteq(0x36, bbox.y1);
	ut_asserteq(3, lines.count);

	nl = strchr(test_string, '\n') - test_string;

	line = alist_get(&lines, 0, struct vidconsole_mline);
	ut_assertnonnull(line);
	ut_asserteq(0, line->bbox.x0);
	ut_asserteq(0, line->bbox.y0);
	ut_asserteq(0x8c, line->bbox.x1);
	ut_asserteq(0x12, line->bbox.y1);
	ut_asserteq(0, line->start);
	ut_asserteq(20, line->len);
	ut_asserteq(nl, line->len);
	printf("line0 '%.*s'\n", line->len, test_string + line->start);
	ut_asserteq_strn("There is always much",
			 test_string + line->start);

	line++;
	ut_asserteq(0x0, line->bbox.x0);
	ut_asserteq(0x12, line->bbox.y0);
	ut_asserteq(0x31e, line->bbox.x1);
	ut_asserteq(0x24, line->bbox.y1);
	ut_asserteq(21, line->start);
	ut_asserteq(nl + 1, line->start);
	ut_asserteq(129, line->len);
	printf("line1 '%.*s'\n", line->len, test_string + line->start);
	ut_asserteq_strn("to be said for not attempting more than you can do "
			 "and for making a certainty of what you try. But this "
			 "principle, like others in",
			 test_string + line->start);

	line++;
	ut_asserteq(0x0, line->bbox.x0);
	ut_asserteq(0x24, line->bbox.y0);
	ut_asserteq(0xc8, line->bbox.x1);
	ut_asserteq(0x36, line->bbox.y1);
	ut_asserteq(21 + 130, line->start);
	ut_asserteq(33, line->len);
	printf("line2 '%.*s'\n", line->len, test_string + line->start);
	ut_asserteq_strn("life and war, has its exceptions.",
			 test_string + line->start);

	/*
	 * all characters should be accounted for, except the newline and the
	 * space which is consumed in the wordwrap
	 */
	ut_asserteq(strlen(test_string) - 2,
		    line[-2].len + line[-1].len + line->len);

	return 0;
}
DM_TEST(dm_test_font_measure, UTF_SCAN_FDT);

/* Test silencing the video console */
static int dm_test_video_silence(struct unit_test_state *uts)
{
	struct udevice *dev, *con;
	struct stdio_dev *sdev;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));

	/*
	 * use the old console device from before when dm_test_pre_run() was
	 * called, since that is what is in stdio / console
	 */
	sdev = stdio_get_by_name("vidconsole");
	ut_assertnonnull(sdev);
	con = sdev->priv;
	ut_assertok(vidconsole_clear_and_reset(con));
	ut_unsilence_console(uts);

	printf("message 1: console\n");
	vidconsole_put_string(con, "message 1: video\n");

	vidconsole_set_quiet(con, true);
	printf("second message: console\n");
	vidconsole_put_string(con, "second message: video\n");

	vidconsole_set_quiet(con, false);
	printf("final message: console\n");
	vidconsole_put_string(con, "final message: video\n");

	ut_asserteq(3892, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_silence, UTF_SCAN_FDT);

/* test drawing a box */
static int dm_test_video_box(struct unit_test_state *uts)
{
	struct video_priv *priv;
	struct udevice *dev;

	ut_assertok(video_get_nologo(uts, &dev));
	priv = dev_get_uclass_priv(dev);
	video_draw_box(dev, 100, 100, 200, 200, 3,
		       video_index_to_colour(priv, VID_LIGHT_BLUE));
	video_draw_box(dev, 300, 100, 400, 200, 1,
		       video_index_to_colour(priv, VID_MAGENTA));
	video_draw_box(dev, 500, 100, 600, 200, 20,
		       video_index_to_colour(priv, VID_LIGHT_RED));
	ut_asserteq(133, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	return 0;
}
DM_TEST(dm_test_video_box, UTF_SCAN_FDT);
