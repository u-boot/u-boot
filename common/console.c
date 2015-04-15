/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <stdarg.h>
#include <iomux.h>
#include <malloc.h>
#include <os.h>
#include <serial.h>
#include <stdio_dev.h>
#include <exports.h>
#include <environment.h>

DECLARE_GLOBAL_DATA_PTR;

static int on_console(const char *name, const char *value, enum env_op op,
	int flags)
{
	int console = -1;

	/* Check for console redirection */
	if (strcmp(name, "stdin") == 0)
		console = stdin;
	else if (strcmp(name, "stdout") == 0)
		console = stdout;
	else if (strcmp(name, "stderr") == 0)
		console = stderr;

	/* if not actually setting a console variable, we don't care */
	if (console == -1 || (gd->flags & GD_FLG_DEVINIT) == 0)
		return 0;

	switch (op) {
	case env_op_create:
	case env_op_overwrite:

#ifdef CONFIG_CONSOLE_MUX
		if (iomux_doenv(console, value))
			return 1;
#else
		/* Try assigning specified device */
		if (console_assign(console, value) < 0)
			return 1;
#endif /* CONFIG_CONSOLE_MUX */
		return 0;

	case env_op_delete:
		if ((flags & H_FORCE) == 0)
			printf("Can't delete \"%s\"\n", name);
		return 1;

	default:
		return 0;
	}
}
U_BOOT_ENV_CALLBACK(console, on_console);

#ifdef CONFIG_SILENT_CONSOLE
static int on_silent(const char *name, const char *value, enum env_op op,
	int flags)
{
#ifndef CONFIG_SILENT_CONSOLE_UPDATE_ON_SET
	if (flags & H_INTERACTIVE)
		return 0;
#endif
#ifndef CONFIG_SILENT_CONSOLE_UPDATE_ON_RELOC
	if ((flags & H_INTERACTIVE) == 0)
		return 0;
#endif

	if (value != NULL)
		gd->flags |= GD_FLG_SILENT;
	else
		gd->flags &= ~GD_FLG_SILENT;

	return 0;
}
U_BOOT_ENV_CALLBACK(silent, on_silent);
#endif

#ifdef CONFIG_SYS_CONSOLE_IS_IN_ENV
/*
 * if overwrite_console returns 1, the stdin, stderr and stdout
 * are switched to the serial port, else the settings in the
 * environment are used
 */
#ifdef CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
extern int overwrite_console(void);
#define OVERWRITE_CONSOLE overwrite_console()
#else
#define OVERWRITE_CONSOLE 0
#endif /* CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE */

#endif /* CONFIG_SYS_CONSOLE_IS_IN_ENV */

static int console_setfile(int file, struct stdio_dev * dev)
{
	int error = 0;

	if (dev == NULL)
		return -1;

	switch (file) {
	case stdin:
	case stdout:
	case stderr:
		/* Start new device */
		if (dev->start) {
			error = dev->start(dev);
			/* If it's not started dont use it */
			if (error < 0)
				break;
		}

		/* Assign the new device (leaving the existing one started) */
		stdio_devices[file] = dev;

		/*
		 * Update monitor functions
		 * (to use the console stuff by other applications)
		 */
		switch (file) {
		case stdin:
			gd->jt->getc = getc;
			gd->jt->tstc = tstc;
			break;
		case stdout:
			gd->jt->putc  = putc;
			gd->jt->puts  = puts;
			gd->jt->printf = printf;
			break;
		}
		break;

	default:		/* Invalid file ID */
		error = -1;
	}
	return error;
}

#if defined(CONFIG_CONSOLE_MUX)
/** Console I/O multiplexing *******************************************/

static struct stdio_dev *tstcdev;
struct stdio_dev **console_devices[MAX_FILES];
int cd_count[MAX_FILES];

/*
 * This depends on tstc() always being called before getc().
 * This is guaranteed to be true because this routine is called
 * only from fgetc() which assures it.
 * No attempt is made to demultiplex multiple input sources.
 */
static int console_getc(int file)
{
	unsigned char ret;

	/* This is never called with testcdev == NULL */
	ret = tstcdev->getc(tstcdev);
	tstcdev = NULL;
	return ret;
}

static int console_tstc(int file)
{
	int i, ret;
	struct stdio_dev *dev;

	disable_ctrlc(1);
	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->tstc != NULL) {
			ret = dev->tstc(dev);
			if (ret > 0) {
				tstcdev = dev;
				disable_ctrlc(0);
				return ret;
			}
		}
	}
	disable_ctrlc(0);

	return 0;
}

static void console_putc(int file, const char c)
{
	int i;
	struct stdio_dev *dev;

	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->putc != NULL)
			dev->putc(dev, c);
	}
}

#ifdef CONFIG_PRE_CONSOLE_BUFFER
static void console_putc_noserial(int file, const char c)
{
	int i;
	struct stdio_dev *dev;

	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->putc != NULL && strcmp(dev->name, "serial") != 0)
			dev->putc(dev, c);
	}
}
#endif

static void console_puts(int file, const char *s)
{
	int i;
	struct stdio_dev *dev;

	for (i = 0; i < cd_count[file]; i++) {
		dev = console_devices[file][i];
		if (dev->puts != NULL)
			dev->puts(dev, s);
	}
}

static inline void console_printdevs(int file)
{
	iomux_printdevs(file);
}

static inline void console_doenv(int file, struct stdio_dev *dev)
{
	iomux_doenv(file, dev->name);
}
#else
static inline int console_getc(int file)
{
	return stdio_devices[file]->getc(stdio_devices[file]);
}

static inline int console_tstc(int file)
{
	return stdio_devices[file]->tstc(stdio_devices[file]);
}

static inline void console_putc(int file, const char c)
{
	stdio_devices[file]->putc(stdio_devices[file], c);
}

#ifdef CONFIG_PRE_CONSOLE_BUFFER
static inline void console_putc_noserial(int file, const char c)
{
	if (strcmp(stdio_devices[file]->name, "serial") != 0)
		stdio_devices[file]->putc(stdio_devices[file], c);
}
#endif

static inline void console_puts(int file, const char *s)
{
	stdio_devices[file]->puts(stdio_devices[file], s);
}

static inline void console_printdevs(int file)
{
	printf("%s\n", stdio_devices[file]->name);
}

static inline void console_doenv(int file, struct stdio_dev *dev)
{
	console_setfile(file, dev);
}
#endif /* defined(CONFIG_CONSOLE_MUX) */

/** U-Boot INITIAL CONSOLE-NOT COMPATIBLE FUNCTIONS *************************/

int serial_printf(const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	serial_puts(printbuffer);
	return i;
}

int fgetc(int file)
{
	if (file < MAX_FILES) {
#if defined(CONFIG_CONSOLE_MUX)
		/*
		 * Effectively poll for input wherever it may be available.
		 */
		for (;;) {
			/*
			 * Upper layer may have already called tstc() so
			 * check for that first.
			 */
			if (tstcdev != NULL)
				return console_getc(file);
			console_tstc(file);
#ifdef CONFIG_WATCHDOG
			/*
			 * If the watchdog must be rate-limited then it should
			 * already be handled in board-specific code.
			 */
			 udelay(1);
#endif
		}
#else
		return console_getc(file);
#endif
	}

	return -1;
}

int ftstc(int file)
{
	if (file < MAX_FILES)
		return console_tstc(file);

	return -1;
}

void fputc(int file, const char c)
{
	if (file < MAX_FILES)
		console_putc(file, c);
}

void fputs(int file, const char *s)
{
	if (file < MAX_FILES)
		console_puts(file, s);
}

int fprintf(int file, const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Send to desired file */
	fputs(file, printbuffer);
	return i;
}

/** U-Boot INITIAL CONSOLE-COMPATIBLE FUNCTION *****************************/

int getc(void)
{
#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return 0;
#endif

	if (!gd->have_console)
		return 0;

	if (gd->flags & GD_FLG_DEVINIT) {
		/* Get from the standard input */
		return fgetc(stdin);
	}

	/* Send directly to the handler */
	return serial_getc();
}

int tstc(void)
{
#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return 0;
#endif

	if (!gd->have_console)
		return 0;

	if (gd->flags & GD_FLG_DEVINIT) {
		/* Test the standard input */
		return ftstc(stdin);
	}

	/* Send directly to the handler */
	return serial_tstc();
}

#define PRE_CONSOLE_FLUSHPOINT1_SERIAL			0
#define PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL	1

#ifdef CONFIG_PRE_CONSOLE_BUFFER
#define CIRC_BUF_IDX(idx) ((idx) % (unsigned long)CONFIG_PRE_CON_BUF_SZ)

static void pre_console_putc(const char c)
{
	char *buffer = (char *)CONFIG_PRE_CON_BUF_ADDR;

	buffer[CIRC_BUF_IDX(gd->precon_buf_idx++)] = c;
}

static void pre_console_puts(const char *s)
{
	while (*s)
		pre_console_putc(*s++);
}

static void print_pre_console_buffer(int flushpoint)
{
	unsigned long i = 0;
	char *buffer = (char *)CONFIG_PRE_CON_BUF_ADDR;

	if (gd->precon_buf_idx > CONFIG_PRE_CON_BUF_SZ)
		i = gd->precon_buf_idx - CONFIG_PRE_CON_BUF_SZ;

	while (i < gd->precon_buf_idx)
		switch (flushpoint) {
		case PRE_CONSOLE_FLUSHPOINT1_SERIAL:
			putc(buffer[CIRC_BUF_IDX(i++)]);
			break;
		case PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL:
			console_putc_noserial(stdout,
					      buffer[CIRC_BUF_IDX(i++)]);
			break;
		}
}
#else
static inline void pre_console_putc(const char c) {}
static inline void pre_console_puts(const char *s) {}
static inline void print_pre_console_buffer(int flushpoint) {}
#endif

void putc(const char c)
{
#ifdef CONFIG_SANDBOX
	if (!gd || !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_putc(c);
		return;
	}
#endif
#ifdef CONFIG_SILENT_CONSOLE
	if (gd->flags & GD_FLG_SILENT)
		return;
#endif

#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return;
#endif

	if (!gd->have_console)
		return pre_console_putc(c);

	if (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputc(stdout, c);
	} else {
		/* Send directly to the handler */
		pre_console_putc(c);
		serial_putc(c);
	}
}

void puts(const char *s)
{
#ifdef CONFIG_SANDBOX
	if (!gd || !(gd->flags & GD_FLG_SERIAL_READY)) {
		os_puts(s);
		return;
	}
#endif

#ifdef CONFIG_SILENT_CONSOLE
	if (gd->flags & GD_FLG_SILENT)
		return;
#endif

#ifdef CONFIG_DISABLE_CONSOLE
	if (gd->flags & GD_FLG_DISABLE_CONSOLE)
		return;
#endif

	if (!gd->have_console)
		return pre_console_puts(s);

	if (gd->flags & GD_FLG_DEVINIT) {
		/* Send to the standard output */
		fputs(stdout, s);
	} else {
		/* Send directly to the handler */
		pre_console_puts(s);
		serial_puts(s);
	}
}

int printf(const char *fmt, ...)
{
	va_list args;
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

#if !defined(CONFIG_SANDBOX) && !defined(CONFIG_PRE_CONSOLE_BUFFER)
	if (!gd->have_console)
		return 0;
#endif

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	/* Print the string */
	puts(printbuffer);
	return i;
}

int vprintf(const char *fmt, va_list args)
{
	uint i;
	char printbuffer[CONFIG_SYS_PBSIZE];

#if defined(CONFIG_PRE_CONSOLE_BUFFER) && !defined(CONFIG_SANDBOX)
	if (!gd->have_console)
		return 0;
#endif

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vscnprintf(printbuffer, sizeof(printbuffer), fmt, args);

	/* Print the string */
	puts(printbuffer);
	return i;
}

/* test if ctrl-c was pressed */
static int ctrlc_disabled = 0;	/* see disable_ctrl() */
static int ctrlc_was_pressed = 0;
int ctrlc(void)
{
#ifndef CONFIG_SANDBOX
	if (!ctrlc_disabled && gd->have_console) {
		if (tstc()) {
			switch (getc()) {
			case 0x03:		/* ^C - Control C */
				ctrlc_was_pressed = 1;
				return 1;
			default:
				break;
			}
		}
	}
#endif

	return 0;
}
/* Reads user's confirmation.
   Returns 1 if user's input is "y", "Y", "yes" or "YES"
*/
int confirm_yesno(void)
{
	int i;
	char str_input[5];

	/* Flush input */
	while (tstc())
		getc();
	i = 0;
	while (i < sizeof(str_input)) {
		str_input[i] = getc();
		putc(str_input[i]);
		if (str_input[i] == '\r')
			break;
		i++;
	}
	putc('\n');
	if (strncmp(str_input, "y\r", 2) == 0 ||
	    strncmp(str_input, "Y\r", 2) == 0 ||
	    strncmp(str_input, "yes\r", 4) == 0 ||
	    strncmp(str_input, "YES\r", 4) == 0)
		return 1;
	return 0;
}
/* pass 1 to disable ctrlc() checking, 0 to enable.
 * returns previous state
 */
int disable_ctrlc(int disable)
{
	int prev = ctrlc_disabled;	/* save previous state */

	ctrlc_disabled = disable;
	return prev;
}

int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

#ifdef CONFIG_MODEM_SUPPORT_DEBUG
char	screen[1024];
char *cursor = screen;
int once = 0;
inline void dbg(const char *fmt, ...)
{
	va_list	args;
	uint	i;
	char	printbuffer[CONFIG_SYS_PBSIZE];

	if (!once) {
		memset(screen, 0, sizeof(screen));
		once++;
	}

	va_start(args, fmt);

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsnprintf(printbuffer, sizeof(printbuffer), fmt, args);
	va_end(args);

	if ((screen + sizeof(screen) - 1 - cursor)
	    < strlen(printbuffer) + 1) {
		memset(screen, 0, sizeof(screen));
		cursor = screen;
	}
	sprintf(cursor, printbuffer);
	cursor += strlen(printbuffer);

}
#else
static inline void dbg(const char *fmt, ...)
{
}
#endif

/** U-Boot INIT FUNCTIONS *************************************************/

struct stdio_dev *search_device(int flags, const char *name)
{
	struct stdio_dev *dev;

	dev = stdio_get_by_name(name);

	if (dev && (dev->flags & flags))
		return dev;

	return NULL;
}

int console_assign(int file, const char *devname)
{
	int flag;
	struct stdio_dev *dev;

	/* Check for valid file */
	switch (file) {
	case stdin:
		flag = DEV_FLAGS_INPUT;
		break;
	case stdout:
	case stderr:
		flag = DEV_FLAGS_OUTPUT;
		break;
	default:
		return -1;
	}

	/* Check for valid device name */

	dev = search_device(flag, devname);

	if (dev)
		return console_setfile(file, dev);

	return -1;
}

/* Called before relocation - use serial functions */
int console_init_f(void)
{
	gd->have_console = 1;

#ifdef CONFIG_SILENT_CONSOLE
	if (getenv("silent") != NULL)
		gd->flags |= GD_FLG_SILENT;
#endif

	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT1_SERIAL);

	return 0;
}

void stdio_print_current_devices(void)
{
	/* Print information */
	puts("In:    ");
	if (stdio_devices[stdin] == NULL) {
		puts("No input devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stdin]->name);
	}

	puts("Out:   ");
	if (stdio_devices[stdout] == NULL) {
		puts("No output devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stdout]->name);
	}

	puts("Err:   ");
	if (stdio_devices[stderr] == NULL) {
		puts("No error devices available!\n");
	} else {
		printf ("%s\n", stdio_devices[stderr]->name);
	}
}

#ifdef CONFIG_SYS_CONSOLE_IS_IN_ENV
/* Called after the relocation - use desired console functions */
int console_init_r(void)
{
	char *stdinname, *stdoutname, *stderrname;
	struct stdio_dev *inputdev = NULL, *outputdev = NULL, *errdev = NULL;
#ifdef CONFIG_SYS_CONSOLE_ENV_OVERWRITE
	int i;
#endif /* CONFIG_SYS_CONSOLE_ENV_OVERWRITE */
#ifdef CONFIG_CONSOLE_MUX
	int iomux_err = 0;
#endif

	/* set default handlers at first */
	gd->jt->getc  = serial_getc;
	gd->jt->tstc  = serial_tstc;
	gd->jt->putc  = serial_putc;
	gd->jt->puts  = serial_puts;
	gd->jt->printf = serial_printf;

	/* stdin stdout and stderr are in environment */
	/* scan for it */
	stdinname  = getenv("stdin");
	stdoutname = getenv("stdout");
	stderrname = getenv("stderr");

	if (OVERWRITE_CONSOLE == 0) {	/* if not overwritten by config switch */
		inputdev  = search_device(DEV_FLAGS_INPUT,  stdinname);
		outputdev = search_device(DEV_FLAGS_OUTPUT, stdoutname);
		errdev    = search_device(DEV_FLAGS_OUTPUT, stderrname);
#ifdef CONFIG_CONSOLE_MUX
		iomux_err = iomux_doenv(stdin, stdinname);
		iomux_err += iomux_doenv(stdout, stdoutname);
		iomux_err += iomux_doenv(stderr, stderrname);
		if (!iomux_err)
			/* Successful, so skip all the code below. */
			goto done;
#endif
	}
	/* if the devices are overwritten or not found, use default device */
	if (inputdev == NULL) {
		inputdev  = search_device(DEV_FLAGS_INPUT,  "serial");
	}
	if (outputdev == NULL) {
		outputdev = search_device(DEV_FLAGS_OUTPUT, "serial");
	}
	if (errdev == NULL) {
		errdev    = search_device(DEV_FLAGS_OUTPUT, "serial");
	}
	/* Initializes output console first */
	if (outputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdout, outputdev);
	}
	if (errdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stderr, errdev);
	}
	if (inputdev != NULL) {
		/* need to set a console if not done above. */
		console_doenv(stdin, inputdev);
	}

#ifdef CONFIG_CONSOLE_MUX
done:
#endif

#ifndef CONFIG_SYS_CONSOLE_INFO_QUIET
	stdio_print_current_devices();
#endif /* CONFIG_SYS_CONSOLE_INFO_QUIET */

#ifdef CONFIG_SYS_CONSOLE_ENV_OVERWRITE
	/* set the environment variables (will overwrite previous env settings) */
	for (i = 0; i < 3; i++) {
		setenv(stdio_names[i], stdio_devices[i]->name);
	}
#endif /* CONFIG_SYS_CONSOLE_ENV_OVERWRITE */

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

#if 0
	/* If nothing usable installed, use only the initial console */
	if ((stdio_devices[stdin] == NULL) && (stdio_devices[stdout] == NULL))
		return 0;
#endif
	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL);
	return 0;
}

#else /* CONFIG_SYS_CONSOLE_IS_IN_ENV */

/* Called after the relocation - use desired console functions */
int console_init_r(void)
{
	struct stdio_dev *inputdev = NULL, *outputdev = NULL;
	int i;
	struct list_head *list = stdio_get_list();
	struct list_head *pos;
	struct stdio_dev *dev;

#ifdef CONFIG_SPLASH_SCREEN
	/*
	 * suppress all output if splash screen is enabled and we have
	 * a bmp to display. We redirect the output from frame buffer
	 * console to serial console in this case or suppress it if
	 * "silent" mode was requested.
	 */
	if (getenv("splashimage") != NULL) {
		if (!(gd->flags & GD_FLG_SILENT))
			outputdev = search_device (DEV_FLAGS_OUTPUT, "serial");
	}
#endif

	/* Scan devices looking for input and output devices */
	list_for_each(pos, list) {
		dev = list_entry(pos, struct stdio_dev, list);

		if ((dev->flags & DEV_FLAGS_INPUT) && (inputdev == NULL)) {
			inputdev = dev;
		}
		if ((dev->flags & DEV_FLAGS_OUTPUT) && (outputdev == NULL)) {
			outputdev = dev;
		}
		if(inputdev && outputdev)
			break;
	}

	/* Initializes output console first */
	if (outputdev != NULL) {
		console_setfile(stdout, outputdev);
		console_setfile(stderr, outputdev);
#ifdef CONFIG_CONSOLE_MUX
		console_devices[stdout][0] = outputdev;
		console_devices[stderr][0] = outputdev;
#endif
	}

	/* Initializes input console */
	if (inputdev != NULL) {
		console_setfile(stdin, inputdev);
#ifdef CONFIG_CONSOLE_MUX
		console_devices[stdin][0] = inputdev;
#endif
	}

#ifndef CONFIG_SYS_CONSOLE_INFO_QUIET
	stdio_print_current_devices();
#endif /* CONFIG_SYS_CONSOLE_INFO_QUIET */

	/* Setting environment variables */
	for (i = 0; i < 3; i++) {
		setenv(stdio_names[i], stdio_devices[i]->name);
	}

	gd->flags |= GD_FLG_DEVINIT;	/* device initialization completed */

#if 0
	/* If nothing usable installed, use only the initial console */
	if ((stdio_devices[stdin] == NULL) && (stdio_devices[stdout] == NULL))
		return 0;
#endif
	print_pre_console_buffer(PRE_CONSOLE_FLUSHPOINT2_EVERYTHING_BUT_SERIAL);
	return 0;
}

#endif /* CONFIG_SYS_CONSOLE_IS_IN_ENV */
