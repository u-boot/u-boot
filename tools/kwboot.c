/*
 * Boot a Marvell SoC, with Xmodem over UART0.
 *  supports Kirkwood, Dove, Armada 370, Armada XP, Armada 375, Armada 38x and
 *           Armada 39x
 *
 * (c) 2012 Daniel Stodden <daniel.stodden@gmail.com>
 * (c) 2021 Pali Rohár <pali@kernel.org>
 * (c) 2021 Marek Behún <marek.behun@nic.cz>
 *
 * References: marvell.com, "88F6180, 88F6190, 88F6192, and 88F6281
 *   Integrated Controller: Functional Specifications" December 2,
 *   2008. Chapter 24.2 "BootROM Firmware".
 */

#include "kwbimage.h"
#include "mkimage.h"
#include "version.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <image.h>
#include <libgen.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/stat.h>

#ifdef __linux__
#include "termios_linux.h"
#else
#include <termios.h>
#endif

/*
 * Marvell BootROM UART Sensing
 */

static unsigned char kwboot_msg_boot[] = {
	0xBB, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
};

static unsigned char kwboot_msg_debug[] = {
	0xDD, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77
};

/* Defines known to work on Kirkwood */
#define KWBOOT_MSG_REQ_DELAY	10 /* ms */
#define KWBOOT_MSG_RSP_TIMEO	50 /* ms */

/* Defines known to work on Armada XP */
#define KWBOOT_MSG_REQ_DELAY_AXP	1000 /* ms */
#define KWBOOT_MSG_RSP_TIMEO_AXP	1000 /* ms */

/*
 * Xmodem Transfers
 */

#define SOH	1	/* sender start of block header */
#define EOT	4	/* sender end of block transfer */
#define ACK	6	/* target block ack */
#define NAK	21	/* target block negative ack */
#define CAN	24	/* target/sender transfer cancellation */

#define KWBOOT_XM_BLKSZ	128 /* xmodem block size */

struct kwboot_block {
	uint8_t soh;
	uint8_t pnum;
	uint8_t _pnum;
	uint8_t data[KWBOOT_XM_BLKSZ];
	uint8_t csum;
} __packed;

#define KWBOOT_BLK_RSP_TIMEO 1000 /* ms */
#define KWBOOT_HDR_RSP_TIMEO 10000 /* ms */

/* ARM code making baudrate changing function return to original exec address */
static unsigned char kwboot_pre_baud_code[] = {
				/* exec_addr:                                 */
	0x00, 0x00, 0x00, 0x00, /* .word 0                                    */
	0x0c, 0xe0, 0x1f, 0xe5, /* ldr lr, exec_addr                          */
};

/* ARM code for binary header injection to change baudrate */
static unsigned char kwboot_baud_code[] = {
				/* ; #define UART_BASE 0xd0012000             */
				/* ; #define THR       0x00                   */
				/* ; #define DLL       0x00                   */
				/* ; #define DLH       0x04                   */
				/* ; #define LCR       0x0c                   */
				/* ; #define   DLAB    0x80                   */
				/* ; #define LSR       0x14                   */
				/* ; #define   THRE    0x20                   */
				/* ; #define   TEMT    0x40                   */
				/* ; #define DIV_ROUND(a, b) ((a + b/2) / b)  */
				/* ;                                          */
				/* ; u32 set_baudrate(u32 old_b, u32 new_b) { */
				/* ;   const u8 *str = "$baudratechange";     */
				/* ;   u8 c;                                  */
				/* ;   do {                                   */
				/* ;       c = *str++;                        */
				/* ;       writel(UART_BASE + THR, c);        */
				/* ;   } while (c);                           */
				/* ;   while                                  */
				/* ;      (!(readl(UART_BASE + LSR) & TEMT)); */
				/* ;   u32 lcr = readl(UART_BASE + LCR);      */
				/* ;   writel(UART_BASE + LCR, lcr | DLAB);   */
				/* ;   u8 old_dll = readl(UART_BASE + DLL);   */
				/* ;   u8 old_dlh = readl(UART_BASE + DLH);   */
				/* ;   u16 old_dl = old_dll | (old_dlh << 8); */
				/* ;   u32 clk = old_b * old_dl;              */
				/* ;   u16 new_dl = DIV_ROUND(clk, new_b);    */
				/* ;   u8 new_dll = new_dl & 0xff;            */
				/* ;   u8 new_dlh = (new_dl >> 8) & 0xff;     */
				/* ;   writel(UART_BASE + DLL, new_dll);      */
				/* ;   writel(UART_BASE + DLH, new_dlh);      */
				/* ;   writel(UART_BASE + LCR, lcr & ~DLAB);  */
				/* ;   msleep(1);                             */
				/* ;   return 0;                              */
				/* ; }                                        */

	0xfe, 0x5f, 0x2d, 0xe9, /* push  { r1 - r12, lr }                     */

				/*  ; r0 = UART_BASE                          */
	0x02, 0x0a, 0xa0, 0xe3, /* mov   r0, #0x2000                          */
	0x01, 0x00, 0x4d, 0xe3, /* movt  r0, #0xd001                          */

				/*  ; r2 = address of preamble string         */
	0xd0, 0x20, 0x8f, 0xe2, /* adr   r2, preamble                         */

				/*  ; Send preamble string over UART          */
				/* .Lloop_preamble:                           */
				/*                                            */
				/*  ; Wait until Transmitter Holding is Empty */
				/* .Lloop_thre:                               */
				/*  ; r1 = UART_BASE[LSR] & THRE              */
	0x14, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x14]                      */
	0x20, 0x00, 0x11, 0xe3, /* tst   r1, #0x20                            */
	0xfc, 0xff, 0xff, 0x0a, /* beq   .Lloop_thre                          */

				/*  ; Put character into Transmitter FIFO     */
				/*  ; r1 = *r2++                              */
	0x01, 0x10, 0xd2, 0xe4, /* ldrb  r1, [r2], #1                         */
				/*  ; UART_BASE[THR] = r1                     */
	0x00, 0x10, 0x80, 0xe5, /* str   r1, [r0, #0x0]                       */

				/*  ; Loop until end of preamble string       */
	0x00, 0x00, 0x51, 0xe3, /* cmp   r1, #0                               */
	0xf8, 0xff, 0xff, 0x1a, /* bne   .Lloop_preamble                      */

				/*  ; Wait until Transmitter FIFO is Empty    */
				/* .Lloop_txempty:                            */
				/*  ; r1 = UART_BASE[LSR] & TEMT              */
	0x14, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x14]                      */
	0x40, 0x00, 0x11, 0xe3, /* tst   r1, #0x40                            */
	0xfc, 0xff, 0xff, 0x0a, /* beq   .Lloop_txempty                       */

				/*  ; Set Divisor Latch Access Bit            */
				/*  ; UART_BASE[LCR] |= DLAB                  */
	0x0c, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x0c]                      */
	0x80, 0x10, 0x81, 0xe3, /* orr   r1, r1, #0x80                        */
	0x0c, 0x10, 0x80, 0xe5, /* str   r1, [r0, #0x0c]                      */

				/*  ; Read current Divisor Latch              */
				/*  ; r1 = UART_BASE[DLH]<<8 | UART_BASE[DLL] */
	0x00, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x00]                      */
	0xff, 0x10, 0x01, 0xe2, /* and   r1, r1, #0xff                        */
	0x01, 0x20, 0xa0, 0xe1, /* mov   r2, r1                               */
	0x04, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x04]                      */
	0xff, 0x10, 0x01, 0xe2, /* and   r1, r1, #0xff                        */
	0x41, 0x14, 0xa0, 0xe1, /* asr   r1, r1, #8                           */
	0x02, 0x10, 0x81, 0xe1, /* orr   r1, r1, r2                           */

				/*  ; Read old baudrate value                 */
				/*  ; r2 = old_baudrate                       */
	0x8c, 0x20, 0x9f, 0xe5, /* ldr   r2, old_baudrate                     */

				/*  ; Calculate base clock                    */
				/*  ; r1 = r2 * r1                            */
	0x92, 0x01, 0x01, 0xe0, /* mul   r1, r2, r1                           */

				/*  ; Read new baudrate value                 */
				/*  ; r2 = baudrate                           */
	0x88, 0x20, 0x9f, 0xe5, /* ldr   r2, baudrate                         */

				/*  ; Calculate new Divisor Latch             */
				/*  ; r1 = DIV_ROUND(r1, r2) =                */
				/*  ;    = (r1 + r2/2) / r2                   */
	0xa2, 0x10, 0x81, 0xe0, /* add   r1, r1, r2, lsr #1                   */
	0x02, 0x40, 0xa0, 0xe1, /* mov   r4, r2                               */
	0xa1, 0x00, 0x54, 0xe1, /* cmp   r4, r1, lsr #1                       */
				/* .Lloop_div1:                               */
	0x84, 0x40, 0xa0, 0x91, /* movls r4, r4, lsl #1                       */
	0xa1, 0x00, 0x54, 0xe1, /* cmp   r4, r1, lsr #1                       */
	0xfc, 0xff, 0xff, 0x9a, /* bls   .Lloop_div1                          */
	0x00, 0x30, 0xa0, 0xe3, /* mov   r3, #0                               */
				/* .Lloop_div2:                               */
	0x04, 0x00, 0x51, 0xe1, /* cmp   r1, r4                               */
	0x04, 0x10, 0x41, 0x20, /* subhs r1, r1, r4                           */
	0x03, 0x30, 0xa3, 0xe0, /* adc   r3, r3, r3                           */
	0xa4, 0x40, 0xa0, 0xe1, /* mov   r4, r4, lsr #1                       */
	0x02, 0x00, 0x54, 0xe1, /* cmp   r4, r2                               */
	0xf9, 0xff, 0xff, 0x2a, /* bhs   .Lloop_div2                          */
	0x03, 0x10, 0xa0, 0xe1, /* mov   r1, r3                               */

				/*  ; Set new Divisor Latch Low               */
				/*  ; UART_BASE[DLL] = r1 & 0xff              */
	0x01, 0x20, 0xa0, 0xe1, /* mov   r2, r1                               */
	0xff, 0x20, 0x02, 0xe2, /* and   r2, r2, #0xff                        */
	0x00, 0x20, 0x80, 0xe5, /* str   r2, [r0, #0x00]                      */

				/*  ; Set new Divisor Latch High              */
				/*  ; UART_BASE[DLH] = r1>>8 & 0xff           */
	0x41, 0x24, 0xa0, 0xe1, /* asr   r2, r1, #8                           */
	0xff, 0x20, 0x02, 0xe2, /* and   r2, r2, #0xff                        */
	0x04, 0x20, 0x80, 0xe5, /* str   r2, [r0, #0x04]                      */

				/*  ; Clear Divisor Latch Access Bit          */
				/*  ; UART_BASE[LCR] &= ~DLAB                 */
	0x0c, 0x10, 0x90, 0xe5, /* ldr   r1, [r0, #0x0c]                      */
	0x80, 0x10, 0xc1, 0xe3, /* bic   r1, r1, #0x80                        */
	0x0c, 0x10, 0x80, 0xe5, /* str   r1, [r0, #0x0c]                      */

				/*  ; Sleep 1ms ~~ 600000 cycles at 1200 MHz  */
				/*  ; r1 = 600000                             */
	0x9f, 0x1d, 0xa0, 0xe3, /* mov   r1, #0x27c0                          */
	0x09, 0x10, 0x40, 0xe3, /* movt  r1, #0x0009                          */
				/* .Lloop_sleep:                              */
	0x01, 0x10, 0x41, 0xe2, /* sub   r1, r1, #1                           */
	0x00, 0x00, 0x51, 0xe3, /* cmp   r1, #0                               */
	0xfc, 0xff, 0xff, 0x1a, /* bne   .Lloop_sleep                         */

				/*  ; Return 0 - no error                     */
	0x00, 0x00, 0xa0, 0xe3, /* mov   r0, #0                               */
	0xfe, 0x9f, 0xbd, 0xe8, /* pop   { r1 - r12, pc }                     */

				/*  ; Preamble string                         */
				/* preamble:                                  */
	0x24, 0x62, 0x61, 0x75, /* .asciz "$baudratechange"                   */
	0x64, 0x72, 0x61, 0x74,
	0x65, 0x63, 0x68, 0x61,
	0x6e, 0x67, 0x65, 0x00,

				/*  ; Placeholder for old baudrate value      */
				/* old_baudrate:                              */
	0x00, 0x00, 0x00, 0x00, /* .word 0                                    */

				/*  ; Placeholder for new baudrate value      */
				/* new_baudrate:                              */
	0x00, 0x00, 0x00, 0x00, /* .word 0                                    */
};

#define KWBOOT_BAUDRATE_BIN_HEADER_SZ (sizeof(kwboot_baud_code) + \
				       sizeof(struct opt_hdr_v1) + 8)

static const char kwb_baud_magic[16] = "$baudratechange";

static int kwboot_verbose;

static int msg_req_delay = KWBOOT_MSG_REQ_DELAY;
static int msg_rsp_timeo = KWBOOT_MSG_RSP_TIMEO;
static int blk_rsp_timeo = KWBOOT_BLK_RSP_TIMEO;

static ssize_t
kwboot_write(int fd, const char *buf, size_t len)
{
	size_t tot = 0;

	while (tot < len) {
		ssize_t wr = write(fd, buf + tot, len - tot);

		if (wr < 0)
			return -1;

		tot += wr;
	}

	return tot;
}

static void
kwboot_printv(const char *fmt, ...)
{
	va_list ap;

	if (kwboot_verbose) {
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
		fflush(stdout);
	}
}

static void
__spinner(void)
{
	const char seq[] = { '-', '\\', '|', '/' };
	const int div = 8;
	static int state, bs;

	if (state % div == 0) {
		fputc(bs, stdout);
		fputc(seq[state / div % sizeof(seq)], stdout);
		fflush(stdout);
	}

	bs = '\b';
	state++;
}

static void
kwboot_spinner(void)
{
	if (kwboot_verbose)
		__spinner();
}

static void
__progress(int pct, char c)
{
	const int width = 70;
	static const char *nl = "";
	static int pos;

	if (pos % width == 0)
		printf("%s%3d %% [", nl, pct);

	fputc(c, stdout);

	nl = "]\n";
	pos = (pos + 1) % width;

	if (pct == 100) {
		while (pos && pos++ < width)
			fputc(' ', stdout);
		fputs(nl, stdout);
		nl = "";
		pos = 0;
	}

	fflush(stdout);

}

static void
kwboot_progress(int _pct, char c)
{
	static int pct;

	if (_pct != -1)
		pct = _pct;

	if (kwboot_verbose)
		__progress(pct, c);

	if (pct == 100)
		pct = 0;
}

static int
kwboot_tty_recv(int fd, void *buf, size_t len, int timeo)
{
	int rc, nfds;
	fd_set rfds;
	struct timeval tv;
	ssize_t n;

	rc = -1;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = timeo * 1000;
	if (tv.tv_usec > 1000000) {
		tv.tv_sec += tv.tv_usec / 1000000;
		tv.tv_usec %= 1000000;
	}

	do {
		nfds = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (nfds < 0)
			goto out;
		if (!nfds) {
			errno = ETIMEDOUT;
			goto out;
		}

		n = read(fd, buf, len);
		if (n <= 0)
			goto out;

		buf = (char *)buf + n;
		len -= n;
	} while (len > 0);

	rc = 0;
out:
	return rc;
}

static int
kwboot_tty_send(int fd, const void *buf, size_t len)
{
	if (!buf)
		return 0;

	if (kwboot_write(fd, buf, len) < 0)
		return -1;

	return tcdrain(fd);
}

static int
kwboot_tty_send_char(int fd, unsigned char c)
{
	return kwboot_tty_send(fd, &c, 1);
}

static speed_t
kwboot_tty_baudrate_to_speed(int baudrate)
{
	switch (baudrate) {
#ifdef B4000000
	case 4000000:
		return B4000000;
#endif
#ifdef B3500000
	case 3500000:
		return B3500000;
#endif
#ifdef B3000000
	case 3000000:
		return B3000000;
#endif
#ifdef B2500000
	case 2500000:
		return B2500000;
#endif
#ifdef B2000000
	case 2000000:
		return B2000000;
#endif
#ifdef B1500000
	case 1500000:
		return B1500000;
#endif
#ifdef B1152000
	case 1152000:
		return B1152000;
#endif
#ifdef B1000000
	case 1000000:
		return B1000000;
#endif
#ifdef B921600
	case 921600:
		return B921600;
#endif
#ifdef B614400
	case 614400:
		return B614400;
#endif
#ifdef B576000
	case 576000:
		return B576000;
#endif
#ifdef B500000
	case 500000:
		return B500000;
#endif
#ifdef B460800
	case 460800:
		return B460800;
#endif
#ifdef B307200
	case 307200:
		return B307200;
#endif
#ifdef B230400
	case 230400:
		return B230400;
#endif
#ifdef B153600
	case 153600:
		return B153600;
#endif
#ifdef B115200
	case 115200:
		return B115200;
#endif
#ifdef B76800
	case 76800:
		return B76800;
#endif
#ifdef B57600
	case 57600:
		return B57600;
#endif
#ifdef B38400
	case 38400:
		return B38400;
#endif
#ifdef B19200
	case 19200:
		return B19200;
#endif
#ifdef B9600
	case 9600:
		return B9600;
#endif
#ifdef B4800
	case 4800:
		return B4800;
#endif
#ifdef B2400
	case 2400:
		return B2400;
#endif
#ifdef B1800
	case 1800:
		return B1800;
#endif
#ifdef B1200
	case 1200:
		return B1200;
#endif
#ifdef B600
	case 600:
		return B600;
#endif
#ifdef B300
	case 300:
		return B300;
#endif
#ifdef B200
	case 200:
		return B200;
#endif
#ifdef B150
	case 150:
		return B150;
#endif
#ifdef B134
	case 134:
		return B134;
#endif
#ifdef B110
	case 110:
		return B110;
#endif
#ifdef B75
	case 75:
		return B75;
#endif
#ifdef B50
	case 50:
		return B50;
#endif
	default:
#ifdef BOTHER
		return BOTHER;
#else
		return B0;
#endif
	}
}

static int
_is_within_tolerance(int value, int reference, int tolerance)
{
	return 100 * value >= reference * (100 - tolerance) &&
	       100 * value <= reference * (100 + tolerance);
}

static int
kwboot_tty_change_baudrate(int fd, int baudrate)
{
	struct termios tio;
	speed_t speed;
	int rc;

	rc = tcgetattr(fd, &tio);
	if (rc)
		return rc;

	speed = kwboot_tty_baudrate_to_speed(baudrate);
	if (speed == B0) {
		errno = EINVAL;
		return -1;
	}

#ifdef BOTHER
	if (speed == BOTHER)
		tio.c_ospeed = tio.c_ispeed = baudrate;
#endif

	rc = cfsetospeed(&tio, speed);
	if (rc)
		return rc;

	rc = cfsetispeed(&tio, speed);
	if (rc)
		return rc;

	rc = tcsetattr(fd, TCSANOW, &tio);
	if (rc)
		return rc;

	rc = tcgetattr(fd, &tio);
	if (rc)
		return rc;

	if (cfgetospeed(&tio) != speed || cfgetispeed(&tio) != speed)
		goto baud_fail;

#ifdef BOTHER
	/*
	 * Check whether set baudrate is within 3% tolerance.
	 * If BOTHER is defined, Linux always fills out c_ospeed / c_ispeed
	 * with real values.
	 */
	if (!_is_within_tolerance(tio.c_ospeed, baudrate, 3))
		goto baud_fail;

	if (!_is_within_tolerance(tio.c_ispeed, baudrate, 3))
		goto baud_fail;
#endif

	return 0;

baud_fail:
	fprintf(stderr, "Could not set baudrate to requested value\n");
	errno = EINVAL;
	return -1;
}

static int
kwboot_open_tty(const char *path, int baudrate)
{
	int rc, fd, flags;
	struct termios tio;

	rc = -1;

	fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd < 0)
		goto out;

	rc = tcgetattr(fd, &tio);
	if (rc)
		goto out;

	cfmakeraw(&tio);
	tio.c_cflag |= CREAD | CLOCAL;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	rc = tcsetattr(fd, TCSANOW, &tio);
	if (rc)
		goto out;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
		goto out;

	rc = fcntl(fd, F_SETFL, flags & ~O_NDELAY);
	if (rc)
		goto out;

	rc = kwboot_tty_change_baudrate(fd, baudrate);
	if (rc)
		goto out;

	rc = fd;
out:
	if (rc < 0) {
		if (fd >= 0)
			close(fd);
	}

	return rc;
}

static int
kwboot_bootmsg(int tty, void *msg)
{
	int rc;
	char c;
	int count;

	if (msg == NULL)
		kwboot_printv("Please reboot the target into UART boot mode...");
	else
		kwboot_printv("Sending boot message. Please reboot the target...");

	do {
		rc = tcflush(tty, TCIOFLUSH);
		if (rc)
			break;

		for (count = 0; count < 128; count++) {
			rc = kwboot_tty_send(tty, msg, 8);
			if (rc) {
				usleep(msg_req_delay * 1000);
				continue;
			}
		}

		rc = kwboot_tty_recv(tty, &c, 1, msg_rsp_timeo);

		kwboot_spinner();

	} while (rc || c != NAK);

	kwboot_printv("\n");

	return rc;
}

static int
kwboot_debugmsg(int tty, void *msg)
{
	int rc;

	kwboot_printv("Sending debug message. Please reboot the target...");

	do {
		char buf[16];

		rc = tcflush(tty, TCIOFLUSH);
		if (rc)
			break;

		rc = kwboot_tty_send(tty, msg, 8);
		if (rc) {
			usleep(msg_req_delay * 1000);
			continue;
		}

		rc = kwboot_tty_recv(tty, buf, 16, msg_rsp_timeo);

		kwboot_spinner();

	} while (rc);

	kwboot_printv("\n");

	return rc;
}

static size_t
kwboot_xm_makeblock(struct kwboot_block *block, const void *data,
		    size_t size, int pnum)
{
	size_t i, n;

	block->soh = SOH;
	block->pnum = pnum;
	block->_pnum = ~block->pnum;

	n = size < KWBOOT_XM_BLKSZ ? size : KWBOOT_XM_BLKSZ;
	memcpy(&block->data[0], data, n);
	memset(&block->data[n], 0, KWBOOT_XM_BLKSZ - n);

	block->csum = 0;
	for (i = 0; i < n; i++)
		block->csum += block->data[i];

	return n;
}

static uint64_t
_now(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_MONOTONIC, &ts)) {
		static int err_print;

		if (!err_print) {
			perror("clock_gettime() does not work");
			err_print = 1;
		}

		/* this will just make the timeout not work */
		return -1ULL;
	}

	return ts.tv_sec * 1000ULL + (ts.tv_nsec + 500000) / 1000000;
}

static int
_is_xm_reply(char c)
{
	return c == ACK || c == NAK || c == CAN;
}

static int
_xm_reply_to_error(int c)
{
	int rc = -1;

	switch (c) {
	case ACK:
		rc = 0;
		break;
	case NAK:
		errno = EBADMSG;
		break;
	case CAN:
		errno = ECANCELED;
		break;
	default:
		errno = EPROTO;
		break;
	}

	return rc;
}

static int
kwboot_baud_magic_handle(int fd, char c, int baudrate)
{
	static size_t rcv_len;

	if (rcv_len < sizeof(kwb_baud_magic)) {
		/* try to recognize whole magic word */
		if (c == kwb_baud_magic[rcv_len]) {
			rcv_len++;
		} else {
			printf("%.*s%c", (int)rcv_len, kwb_baud_magic, c);
			fflush(stdout);
			rcv_len = 0;
		}
	}

	if (rcv_len == sizeof(kwb_baud_magic)) {
		/* magic word received */
		kwboot_printv("\nChanging baudrate to %d Bd\n", baudrate);

		return kwboot_tty_change_baudrate(fd, baudrate) ? : 1;
	} else {
		return 0;
	}
}

static int
kwboot_xm_recv_reply(int fd, char *c, int allow_non_xm, int *non_xm_print,
		     int baudrate, int *baud_changed)
{
	int timeout = allow_non_xm ? KWBOOT_HDR_RSP_TIMEO : blk_rsp_timeo;
	uint64_t recv_until = _now() + timeout;
	int rc;

	if (non_xm_print)
		*non_xm_print = 0;
	if (baud_changed)
		*baud_changed = 0;

	while (1) {
		rc = kwboot_tty_recv(fd, c, 1, timeout);
		if (rc) {
			if (errno != ETIMEDOUT)
				return rc;
			else if (allow_non_xm && *non_xm_print)
				return -1;
			else
				*c = NAK;
		}

		/* If received xmodem reply, end. */
		if (_is_xm_reply(*c))
			break;

		/*
		 * If receiving/printing non-xmodem text output is allowed and
		 * such a byte was received, we want to increase receiving time
		 * and either:
		 * - print the byte, if it is not part of baudrate change magic
		 *   sequence while baudrate change was requested (-B option)
		 * - change baudrate
		 * Otherwise decrease timeout by time elapsed.
		 */
		if (allow_non_xm) {
			recv_until = _now() + timeout;

			if (baudrate && !*baud_changed) {
				rc = kwboot_baud_magic_handle(fd, *c, baudrate);
				if (rc == 1)
					*baud_changed = 1;
				else if (!rc)
					*non_xm_print = 1;
				else
					return rc;
			} else if (!baudrate || !*baud_changed) {
				putchar(*c);
				fflush(stdout);
				*non_xm_print = 1;
			}
		} else {
			timeout = recv_until - _now();
			if (timeout < 0) {
				errno = ETIMEDOUT;
				return -1;
			}
		}
	}

	return 0;
}

static int
kwboot_xm_sendblock(int fd, struct kwboot_block *block, int allow_non_xm,
		    int *done_print, int baudrate)
{
	int non_xm_print, baud_changed;
	int rc, err, retries;
	char c;

	*done_print = 0;

	retries = 16;
	do {
		rc = kwboot_tty_send(fd, block, sizeof(*block));
		if (rc)
			return rc;

		if (allow_non_xm && !*done_print) {
			kwboot_progress(100, '.');
			kwboot_printv("Done\n");
			*done_print = 1;
		}

		rc = kwboot_xm_recv_reply(fd, &c, allow_non_xm, &non_xm_print,
					  baudrate, &baud_changed);
		if (rc)
			goto can;

		if (!allow_non_xm && c != ACK)
			kwboot_progress(-1, '+');
	} while (c == NAK && retries-- > 0);

	if (non_xm_print)
		kwboot_printv("\n");

	if (allow_non_xm && baudrate && !baud_changed) {
		fprintf(stderr, "Baudrate was not changed\n");
		rc = -1;
		errno = EPROTO;
		goto can;
	}

	return _xm_reply_to_error(c);
can:
	err = errno;
	kwboot_tty_send_char(fd, CAN);
	kwboot_printv("\n");
	errno = err;
	return rc;
}

static int
kwboot_xm_finish(int fd)
{
	int rc, retries;
	char c;

	kwboot_printv("Finishing transfer\n");

	retries = 16;
	do {
		rc = kwboot_tty_send_char(fd, EOT);
		if (rc)
			return rc;

		rc = kwboot_xm_recv_reply(fd, &c, 0, NULL, 0, NULL);
		if (rc)
			return rc;
	} while (c == NAK && retries-- > 0);

	return _xm_reply_to_error(c);
}

static int
kwboot_xmodem_one(int tty, int *pnum, int header, const uint8_t *data,
		  size_t size, int baudrate)
{
	int done_print = 0;
	size_t sent, left;
	int rc;

	kwboot_printv("Sending boot image %s (%zu bytes)...\n",
		      header ? "header" : "data", size);

	left = size;
	sent = 0;

	while (sent < size) {
		struct kwboot_block block;
		int last_block;
		size_t blksz;

		blksz = kwboot_xm_makeblock(&block, data, left, (*pnum)++);
		data += blksz;

		last_block = (left <= blksz);

		rc = kwboot_xm_sendblock(tty, &block, header && last_block,
					 &done_print, baudrate);
		if (rc)
			goto out;

		sent += blksz;
		left -= blksz;

		if (!done_print)
			kwboot_progress(sent * 100 / size, '.');
	}

	if (!done_print)
		kwboot_printv("Done\n");

	return 0;
out:
	kwboot_printv("\n");
	return rc;
}

static int
kwboot_xmodem(int tty, const void *_img, size_t size, int baudrate)
{
	const uint8_t *img = _img;
	int rc, pnum;
	size_t hdrsz;

	hdrsz = kwbheader_size(img);

	kwboot_printv("Waiting 2s and flushing tty\n");
	sleep(2); /* flush isn't effective without it */
	tcflush(tty, TCIOFLUSH);

	pnum = 1;

	rc = kwboot_xmodem_one(tty, &pnum, 1, img, hdrsz, baudrate);
	if (rc)
		return rc;

	img += hdrsz;
	size -= hdrsz;

	rc = kwboot_xmodem_one(tty, &pnum, 0, img, size, 0);
	if (rc)
		return rc;

	rc = kwboot_xm_finish(tty);
	if (rc)
		return rc;

	if (baudrate) {
		char buf[sizeof(kwb_baud_magic)];

		/* Wait 1s for baudrate change magic */
		rc = kwboot_tty_recv(tty, buf, sizeof(buf), 1000);
		if (rc)
			return rc;

		if (memcmp(buf, kwb_baud_magic, sizeof(buf))) {
			errno = EPROTO;
			return -1;
		}

		kwboot_printv("\nChanging baudrate back to 115200 Bd\n\n");
		rc = kwboot_tty_change_baudrate(tty, 115200);
		if (rc)
			return rc;
	}

	return 0;
}

static int
kwboot_term_pipe(int in, int out, const char *quit, int *s)
{
	ssize_t nin;
	char _buf[128], *buf = _buf;

	nin = read(in, buf, sizeof(_buf));
	if (nin <= 0)
		return -1;

	if (quit) {
		int i;

		for (i = 0; i < nin; i++) {
			if (*buf == quit[*s]) {
				(*s)++;
				if (!quit[*s])
					return 0;
				buf++;
				nin--;
			} else {
				if (kwboot_write(out, quit, *s) < 0)
					return -1;
				*s = 0;
			}
		}
	}

	if (kwboot_write(out, buf, nin) < 0)
		return -1;

	return 0;
}

static int
kwboot_terminal(int tty)
{
	int rc, in, s;
	const char *quit = "\34c";
	struct termios otio, tio;

	rc = -1;

	in = STDIN_FILENO;
	if (isatty(in)) {
		rc = tcgetattr(in, &otio);
		if (!rc) {
			tio = otio;
			cfmakeraw(&tio);
			rc = tcsetattr(in, TCSANOW, &tio);
		}
		if (rc) {
			perror("tcsetattr");
			goto out;
		}

		kwboot_printv("[Type Ctrl-%c + %c to quit]\r\n",
			      quit[0] | 0100, quit[1]);
	} else
		in = -1;

	rc = 0;
	s = 0;

	do {
		fd_set rfds;
		int nfds = 0;

		FD_SET(tty, &rfds);
		nfds = nfds < tty ? tty : nfds;

		if (in >= 0) {
			FD_SET(in, &rfds);
			nfds = nfds < in ? in : nfds;
		}

		nfds = select(nfds + 1, &rfds, NULL, NULL, NULL);
		if (nfds < 0)
			break;

		if (FD_ISSET(tty, &rfds)) {
			rc = kwboot_term_pipe(tty, STDOUT_FILENO, NULL, NULL);
			if (rc)
				break;
		}

		if (in >= 0 && FD_ISSET(in, &rfds)) {
			rc = kwboot_term_pipe(in, tty, quit, &s);
			if (rc)
				break;
		}
	} while (quit[s] != 0);

	if (in >= 0)
		tcsetattr(in, TCSANOW, &otio);
	printf("\n");
out:
	return rc;
}

static void *
kwboot_read_image(const char *path, size_t *size, size_t reserve)
{
	int rc, fd;
	struct stat st;
	void *img;
	off_t tot;

	rc = -1;
	img = NULL;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		goto out;

	rc = fstat(fd, &st);
	if (rc)
		goto out;

	img = malloc(st.st_size + reserve);
	if (!img)
		goto out;

	tot = 0;
	while (tot < st.st_size) {
		ssize_t rd = read(fd, img + tot, st.st_size - tot);

		if (rd < 0)
			goto out;

		tot += rd;

		if (!rd && tot < st.st_size) {
			errno = EIO;
			goto out;
		}
	}

	rc = 0;
	*size = st.st_size;
out:
	if (rc && img) {
		free(img);
		img = NULL;
	}
	if (fd >= 0)
		close(fd);

	return img;
}

static uint8_t
kwboot_hdr_csum8(const void *hdr)
{
	const uint8_t *data = hdr;
	uint8_t csum;
	size_t size;

	size = kwbheader_size_for_csum(hdr);

	for (csum = 0; size-- > 0; data++)
		csum += *data;

	return csum;
}

static int
kwboot_img_is_secure(void *img)
{
	struct opt_hdr_v1 *ohdr;

	for_each_opt_hdr_v1 (ohdr, img)
		if (ohdr->headertype == OPT_HDR_V1_SECURE_TYPE)
			return 1;

	return 0;
}

static void *
kwboot_img_grow_data_left(void *img, size_t *size, size_t grow)
{
	uint32_t hdrsz, datasz, srcaddr;
	struct main_hdr_v1 *hdr = img;
	uint8_t *data;

	srcaddr = le32_to_cpu(hdr->srcaddr);

	hdrsz = kwbheader_size(hdr);
	data = (uint8_t *)img + srcaddr;
	datasz = *size - srcaddr;

	/* only move data if there is not enough space */
	if (hdrsz + grow > srcaddr) {
		size_t need = hdrsz + grow - srcaddr;

		/* move data by enough bytes */
		memmove(data + need, data, datasz);
		*size += need;
		srcaddr += need;
	}

	srcaddr -= grow;
	hdr->srcaddr = cpu_to_le32(srcaddr);
	hdr->destaddr = cpu_to_le32(le32_to_cpu(hdr->destaddr) - grow);
	hdr->blocksize = cpu_to_le32(le32_to_cpu(hdr->blocksize) + grow);

	return (uint8_t *)img + srcaddr;
}

static void
kwboot_img_grow_hdr(void *img, size_t *size, size_t grow)
{
	uint32_t hdrsz, datasz, srcaddr;
	struct main_hdr_v1 *hdr = img;
	uint8_t *data;

	srcaddr = le32_to_cpu(hdr->srcaddr);

	hdrsz = kwbheader_size(img);
	data = (uint8_t *)img + srcaddr;
	datasz = *size - srcaddr;

	/* only move data if there is not enough space */
	if (hdrsz + grow > srcaddr) {
		size_t need = hdrsz + grow - srcaddr;

		/* move data by enough bytes */
		memmove(data + need, data, datasz);

		hdr->srcaddr = cpu_to_le32(srcaddr + need);
		*size += need;
	}

	if (kwbimage_version(img) == 1) {
		hdrsz += grow;
		hdr->headersz_msb = hdrsz >> 16;
		hdr->headersz_lsb = cpu_to_le16(hdrsz & 0xffff);
	}
}

static void *
kwboot_add_bin_ohdr_v1(void *img, size_t *size, uint32_t binsz)
{
	struct main_hdr_v1 *hdr = img;
	struct opt_hdr_v1 *ohdr;
	uint32_t ohdrsz;

	ohdrsz = binsz + 8 + sizeof(*ohdr);
	kwboot_img_grow_hdr(img, size, ohdrsz);

	if (hdr->ext & 0x1) {
		for_each_opt_hdr_v1 (ohdr, img)
			if (opt_hdr_v1_next(ohdr) == NULL)
				break;

		*opt_hdr_v1_ext(ohdr) |= 1;
		ohdr = opt_hdr_v1_next(ohdr);
	} else {
		hdr->ext |= 1;
		ohdr = (void *)(hdr + 1);
	}

	ohdr->headertype = OPT_HDR_V1_BINARY_TYPE;
	ohdr->headersz_msb = ohdrsz >> 16;
	ohdr->headersz_lsb = cpu_to_le16(ohdrsz & 0xffff);

	memset(&ohdr->data[0], 0, ohdrsz - sizeof(*ohdr));

	return &ohdr->data[4];
}

static void
_copy_baudrate_change_code(struct main_hdr_v1 *hdr, void *dst, int pre,
			   int old_baud, int new_baud)
{
	size_t codesz = sizeof(kwboot_baud_code);
	uint8_t *code = dst;

	if (pre) {
		size_t presz = sizeof(kwboot_pre_baud_code);

		/*
		 * We need to prepend code that loads lr register with original
		 * value of hdr->execaddr. We do this by putting the original
		 * exec address before the code that loads it relatively from
		 * it's beginning.
		 * Afterwards we change the exec address to this code (which is
		 * at offset 4, because the first 4 bytes contain the original
		 * exec address).
		 */
		memcpy(code, kwboot_pre_baud_code, presz);
		*(uint32_t *)code = hdr->execaddr;

		hdr->execaddr = cpu_to_le32(le32_to_cpu(hdr->destaddr) + 4);

		code += presz;
	}

	memcpy(code, kwboot_baud_code, codesz - 8);
	*(uint32_t *)(code + codesz - 8) = cpu_to_le32(old_baud);
	*(uint32_t *)(code + codesz - 4) = cpu_to_le32(new_baud);
}

static int
kwboot_img_patch(void *img, size_t *size, int baudrate)
{
	struct main_hdr_v1 *hdr;
	uint32_t srcaddr;
	uint8_t csum;
	size_t hdrsz;
	int image_ver;
	int is_secure;

	hdr = img;

	if (*size < sizeof(struct main_hdr_v1))
		goto err;

	image_ver = kwbimage_version(img);
	if (image_ver != 0 && image_ver != 1) {
		fprintf(stderr, "Invalid image header version\n");
		goto err;
	}

	hdrsz = kwbheader_size(hdr);

	if (*size < hdrsz)
		goto err;

	csum = kwboot_hdr_csum8(hdr) - hdr->checksum;
	if (csum != hdr->checksum)
		goto err;

	if (image_ver == 0) {
		struct main_hdr_v0 *hdr_v0 = img;

		hdr_v0->nandeccmode = IBR_HDR_ECC_DISABLED;
		hdr_v0->nandpagesize = 0;
	}

	srcaddr = le32_to_cpu(hdr->srcaddr);

	switch (hdr->blockid) {
	case IBR_HDR_SATA_ID:
		if (srcaddr < 1)
			goto err;

		hdr->srcaddr = cpu_to_le32((srcaddr - 1) * 512);
		break;

	case IBR_HDR_SDIO_ID:
		hdr->srcaddr = cpu_to_le32(srcaddr * 512);
		break;

	case IBR_HDR_PEX_ID:
		if (srcaddr == 0xFFFFFFFF)
			hdr->srcaddr = cpu_to_le32(hdrsz);
		break;

	case IBR_HDR_SPI_ID:
		if (hdr->destaddr == cpu_to_le32(0xFFFFFFFF)) {
			kwboot_printv("Patching destination and execution addresses from SPI/NOR XIP area to DDR area 0x00800000\n");
			hdr->destaddr = cpu_to_le32(0x00800000);
			hdr->execaddr = cpu_to_le32(0x00800000);
		}
		break;
	}

	if (hdrsz > le32_to_cpu(hdr->srcaddr) ||
	    *size < le32_to_cpu(hdr->srcaddr) + le32_to_cpu(hdr->blocksize))
		goto err;

	is_secure = kwboot_img_is_secure(img);

	if (hdr->blockid != IBR_HDR_UART_ID) {
		if (is_secure) {
			fprintf(stderr,
				"Image has secure header with signature for non-UART booting\n");
			goto err;
		}

		kwboot_printv("Patching image boot signature to UART\n");
		hdr->blockid = IBR_HDR_UART_ID;
	}

	if (baudrate) {
		uint32_t codesz = sizeof(kwboot_baud_code);
		void *code;

		if (image_ver == 0) {
			fprintf(stderr,
				"Cannot inject code for changing baudrate into v0 image header\n");
			goto err;
		}

		if (is_secure) {
			fprintf(stderr,
				"Cannot inject code for changing baudrate into image with secure header\n");
			goto err;
		}

		/*
		 * First inject code that changes the baudrate from the default
		 * value of 115200 Bd to requested value. This code is inserted
		 * as a new opt hdr, so it is executed by BootROM after the
		 * header part is received.
		 */
		kwboot_printv("Injecting binary header code for changing baudrate to %d Bd\n",
			      baudrate);

		code = kwboot_add_bin_ohdr_v1(img, size, codesz);
		_copy_baudrate_change_code(hdr, code, 0, 115200, baudrate);

		/*
		 * Now inject code that changes the baudrate back to 115200 Bd.
		 * This code is prepended to the data part of the image, so it
		 * is executed before U-Boot proper.
		 */
		kwboot_printv("Injecting code for changing baudrate back\n");

		codesz += sizeof(kwboot_pre_baud_code);
		code = kwboot_img_grow_data_left(img, size, codesz);
		_copy_baudrate_change_code(hdr, code, 1, baudrate, 115200);

		/* recompute header size */
		hdrsz = kwbheader_size(hdr);
	}

	if (hdrsz % KWBOOT_XM_BLKSZ) {
		size_t offset = (KWBOOT_XM_BLKSZ - hdrsz % KWBOOT_XM_BLKSZ) %
				KWBOOT_XM_BLKSZ;

		if (is_secure) {
			fprintf(stderr, "Cannot align image with secure header\n");
			goto err;
		}

		kwboot_printv("Aligning image header to Xmodem block size\n");
		kwboot_img_grow_hdr(img, size, offset);
	}

	hdr->checksum = kwboot_hdr_csum8(hdr) - csum;

	*size = le32_to_cpu(hdr->srcaddr) + le32_to_cpu(hdr->blocksize);
	return 0;
err:
	errno = EINVAL;
	return -1;
}

static void
kwboot_usage(FILE *stream, char *progname)
{
	fprintf(stream, "kwboot version %s\n", PLAIN_VERSION);
	fprintf(stream,
		"Usage: %s [OPTIONS] [-b <image> | -D <image> ] [-B <baud> ] <TTY>\n",
		progname);
	fprintf(stream, "\n");
	fprintf(stream,
		"  -b <image>: boot <image> with preamble (Kirkwood, Armada 370/XP)\n");
	fprintf(stream,
		"  -D <image>: boot <image> without preamble (Dove)\n");
	fprintf(stream, "  -d: enter debug mode\n");
	fprintf(stream, "  -a: use timings for Armada XP\n");
	fprintf(stream, "  -q <req-delay>:  use specific request-delay\n");
	fprintf(stream, "  -s <resp-timeo>: use specific response-timeout\n");
	fprintf(stream,
		"  -o <block-timeo>: use specific xmodem block timeout\n");
	fprintf(stream, "\n");
	fprintf(stream, "  -t: mini terminal\n");
	fprintf(stream, "\n");
	fprintf(stream, "  -B <baud>: set baud rate\n");
	fprintf(stream, "\n");
}

int
main(int argc, char **argv)
{
	const char *ttypath, *imgpath;
	int rv, rc, tty, term;
	void *bootmsg;
	void *debugmsg;
	void *img;
	size_t size;
	size_t after_img_rsv;
	int baudrate;

	rv = 1;
	tty = -1;
	bootmsg = NULL;
	debugmsg = NULL;
	imgpath = NULL;
	img = NULL;
	term = 0;
	size = 0;
	after_img_rsv = KWBOOT_XM_BLKSZ;
	baudrate = 115200;

	kwboot_verbose = isatty(STDOUT_FILENO);

	do {
		int c = getopt(argc, argv, "hb:ptaB:dD:q:s:o:");
		if (c < 0)
			break;

		switch (c) {
		case 'b':
			bootmsg = kwboot_msg_boot;
			imgpath = optarg;
			break;

		case 'D':
			bootmsg = NULL;
			imgpath = optarg;
			break;

		case 'd':
			debugmsg = kwboot_msg_debug;
			break;

		case 'p':
			/* nop, for backward compatibility */
			break;

		case 't':
			term = 1;
			break;

		case 'a':
			msg_req_delay = KWBOOT_MSG_REQ_DELAY_AXP;
			msg_rsp_timeo = KWBOOT_MSG_RSP_TIMEO_AXP;
			break;

		case 'q':
			msg_req_delay = atoi(optarg);
			break;

		case 's':
			msg_rsp_timeo = atoi(optarg);
			break;

		case 'o':
			blk_rsp_timeo = atoi(optarg);
			break;

		case 'B':
			baudrate = atoi(optarg);
			break;

		case 'h':
			rv = 0;
		default:
			goto usage;
		}
	} while (1);

	if (!bootmsg && !term && !debugmsg)
		goto usage;

	if (argc - optind < 1)
		goto usage;

	ttypath = argv[optind++];

	tty = kwboot_open_tty(ttypath, imgpath ? 115200 : baudrate);
	if (tty < 0) {
		perror(ttypath);
		goto out;
	}

	if (baudrate == 115200)
		/* do not change baudrate during Xmodem to the same value */
		baudrate = 0;
	else
		/* ensure we have enough space for baudrate change code */
		after_img_rsv += KWBOOT_BAUDRATE_BIN_HEADER_SZ +
				 sizeof(kwboot_pre_baud_code) +
				 sizeof(kwboot_baud_code);

	if (imgpath) {
		img = kwboot_read_image(imgpath, &size, after_img_rsv);
		if (!img) {
			perror(imgpath);
			goto out;
		}

		rc = kwboot_img_patch(img, &size, baudrate);
		if (rc) {
			fprintf(stderr, "%s: Invalid image.\n", imgpath);
			goto out;
		}
	}

	if (debugmsg) {
		rc = kwboot_debugmsg(tty, debugmsg);
		if (rc) {
			perror("debugmsg");
			goto out;
		}
	} else if (bootmsg) {
		rc = kwboot_bootmsg(tty, bootmsg);
		if (rc) {
			perror("bootmsg");
			goto out;
		}
	}

	if (img) {
		rc = kwboot_xmodem(tty, img, size, baudrate);
		if (rc) {
			perror("xmodem");
			goto out;
		}
	}

	if (term) {
		rc = kwboot_terminal(tty);
		if (rc && !(errno == EINTR)) {
			perror("terminal");
			goto out;
		}
	}

	rv = 0;
out:
	if (tty >= 0)
		close(tty);

	if (img)
		free(img);

	return rv;

usage:
	kwboot_usage(rv ? stderr : stdout, basename(argv[0]));
	goto out;
}
