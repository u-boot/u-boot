/*
 * Boot a Marvell SoC, with Xmodem over UART0.
 *  supports Kirkwood, Dove, Armada 370, Armada XP
 *
 * (c) 2012 Daniel Stodden <daniel.stodden@gmail.com>
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
#include <termios.h>
#include <time.h>
#include <sys/stat.h>

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
kwboot_tty_speed(int baudrate)
{
	switch (baudrate) {
	case 115200:
		return B115200;
	case 57600:
		return B57600;
	case 38400:
		return B38400;
	case 19200:
		return B19200;
	case 9600:
		return B9600;
	}

	return -1;
}

static int
kwboot_open_tty(const char *path, speed_t speed)
{
	int rc, fd;
	struct termios tio;

	rc = -1;

	fd = open(path, O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0)
		goto out;

	memset(&tio, 0, sizeof(tio));

	tio.c_iflag = 0;
	tio.c_cflag = CREAD|CLOCAL|CS8;

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 10;

	cfsetospeed(&tio, speed);
	cfsetispeed(&tio, speed);

	rc = tcsetattr(fd, TCSANOW, &tio);
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
kwboot_xm_recv_reply(int fd, char *c, int allow_non_xm, int *non_xm_print)
{
	int timeout = allow_non_xm ? KWBOOT_HDR_RSP_TIMEO : blk_rsp_timeo;
	uint64_t recv_until = _now() + timeout;
	int rc;

	if (non_xm_print)
		*non_xm_print = 0;

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
		 * If printing non-xmodem text output is allowed and such a byte
		 * was received, print it and increase receiving time.
		 * Otherwise decrease timeout by time elapsed.
		 */
		if (allow_non_xm) {
			recv_until = _now() + timeout;
			putchar(*c);
			fflush(stdout);
			*non_xm_print = 1;
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
		    int *done_print)
{
	int non_xm_print;
	int rc, retries;
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

		rc = kwboot_xm_recv_reply(fd, &c, allow_non_xm, &non_xm_print);
		if (rc)
			return rc;

		if (!allow_non_xm && c != ACK)
			kwboot_progress(-1, '+');
	} while (c == NAK && retries-- > 0);

	if (non_xm_print)
		kwboot_printv("\n");

	return _xm_reply_to_error(c);
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

		rc = kwboot_xm_recv_reply(fd, &c, 0, NULL);
		if (rc)
			return rc;
	} while (c == NAK && retries-- > 0);

	return _xm_reply_to_error(c);
}

static int
kwboot_xmodem_one(int tty, int *pnum, int header, const uint8_t *data,
		  size_t size)
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
					 &done_print);
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
kwboot_xmodem(int tty, const void *_img, size_t size)
{
	const uint8_t *img = _img;
	int rc, pnum;
	size_t hdrsz;

	hdrsz = kwbheader_size(img);

	kwboot_printv("Waiting 2s and flushing tty\n");
	sleep(2); /* flush isn't effective without it */
	tcflush(tty, TCIOFLUSH);

	pnum = 1;

	rc = kwboot_xmodem_one(tty, &pnum, 1, img, hdrsz);
	if (rc)
		return rc;

	img += hdrsz;
	size -= hdrsz;

	rc = kwboot_xmodem_one(tty, &pnum, 0, img, size);
	if (rc)
		return rc;

	return kwboot_xm_finish(tty);
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
			      quit[0]|0100, quit[1]);
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

static int
kwboot_img_patch_hdr(void *img, size_t *size)
{
	int rc;
	struct main_hdr_v1 *hdr;
	uint32_t srcaddr;
	uint8_t csum;
	size_t hdrsz = sizeof(*hdr);
	int image_ver;
	int is_secure;

	rc = -1;
	hdr = img;

	if (*size < hdrsz) {
		errno = EINVAL;
		goto out;
	}

	image_ver = kwbimage_version(img);
	if (image_ver != 0 && image_ver != 1) {
		fprintf(stderr, "Invalid image header version\n");
		errno = EINVAL;
		goto out;
	}

	hdrsz = kwbheader_size(hdr);

	if (*size < hdrsz) {
		errno = EINVAL;
		goto out;
	}

	csum = kwboot_hdr_csum8(hdr) - hdr->checksum;
	if (csum != hdr->checksum) {
		errno = EINVAL;
		goto out;
	}

	if (image_ver == 0) {
		struct main_hdr_v0 *hdr_v0 = img;

		hdr_v0->nandeccmode = IBR_HDR_ECC_DISABLED;
		hdr_v0->nandpagesize = 0;
	}

	srcaddr = le32_to_cpu(hdr->srcaddr);

	switch (hdr->blockid) {
	case IBR_HDR_SATA_ID:
		if (srcaddr < 1) {
			errno = EINVAL;
			goto out;
		}
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
	    *size < le32_to_cpu(hdr->srcaddr) + le32_to_cpu(hdr->blocksize)) {
		errno = EINVAL;
		goto out;
	}

	is_secure = kwboot_img_is_secure(img);

	if (hdr->blockid != IBR_HDR_UART_ID) {
		if (is_secure) {
			fprintf(stderr,
				"Image has secure header with signature for non-UART booting\n");
			errno = EINVAL;
			goto out;
		}

		kwboot_printv("Patching image boot signature to UART\n");
		hdr->blockid = IBR_HDR_UART_ID;
	}

	if (hdrsz % KWBOOT_XM_BLKSZ) {
		size_t offset = (KWBOOT_XM_BLKSZ - hdrsz % KWBOOT_XM_BLKSZ) %
				KWBOOT_XM_BLKSZ;

		if (is_secure) {
			fprintf(stderr, "Cannot align image with secure header\n");
			errno = EINVAL;
			goto out;
		}

		kwboot_printv("Aligning image header to Xmodem block size\n");
		kwboot_img_grow_hdr(img, size, offset);
	}

	hdr->checksum = kwboot_hdr_csum8(hdr) - csum;

	*size = le32_to_cpu(hdr->srcaddr) + le32_to_cpu(hdr->blocksize);
	rc = 0;
out:
	return rc;
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
	speed_t speed;

	rv = 1;
	tty = -1;
	bootmsg = NULL;
	debugmsg = NULL;
	imgpath = NULL;
	img = NULL;
	term = 0;
	size = 0;
	speed = B115200;

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
			speed = kwboot_tty_speed(atoi(optarg));
			if (speed == -1)
				goto usage;
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

	tty = kwboot_open_tty(ttypath, speed);
	if (tty < 0) {
		perror(ttypath);
		goto out;
	}

	if (imgpath) {
		img = kwboot_read_image(imgpath, &size, KWBOOT_XM_BLKSZ);
		if (!img) {
			perror(imgpath);
			goto out;
		}

		rc = kwboot_img_patch_hdr(img, &size);
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
		rc = kwboot_xmodem(tty, img, size);
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
