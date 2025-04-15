#ifndef __STDIO_H
#define __STDIO_H

#include <stdarg.h>
#include <linux/compiler.h>

/* stdin */
int getchar(void);
int tstc(void);

/* stdout */
#if !defined(CONFIG_XPL_BUILD) || CONFIG_IS_ENABLED(SERIAL)
void putc(const char c);
void puts(const char *s);
#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void flush(void);
#else
static inline void flush(void) {}
#endif
int __printf(1, 2) printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list args);
#else
static inline void putc(const char c)
{
}

static inline void puts(const char *s)
{
}

static inline void flush(void)
{
}

static inline int __printf(1, 2) printf(const char *fmt, ...)
{
	return 0;
}

static inline int vprintf(const char *fmt, va_list args)
{
	return 0;
}
#endif

/**
 * Format a string and place it in a buffer
 *
 * @buf: The buffer to place the result into
 * @size: The size of the buffer, including the trailing null space
 * @fmt: The format string to use
 * @...: Arguments for the format string
 * Return: the number of characters which would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.  If the return is greater than or equal to
 * @size, the resulting string is truncated.
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int snprintf(char *buf, size_t size, const char *fmt, ...)
	     __attribute__ ((format (__printf__, 3, 4)));

/*
 * FILE based functions (can only be used AFTER relocation!)
 */
#define stdin		0
#define stdout		1
#define stderr		2
#define MAX_FILES	3

/* stderr */
#define eputc(c)		fputc(stderr, c)
#define eputs(s)		fputs(stderr, s)
#define eflush()		fflush(stderr)
#define eprintf(fmt, args...)	fprintf(stderr, fmt, ##args)

int __printf(2, 3) fprintf(int file, const char *fmt, ...);
void fputs(int file, const char *s);
void fputc(int file, const char c);
#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
void fflush(int file);
#else
static inline void fflush(int file) {}
#endif
int ftstc(int file);
int fgetc(int file);

#endif /* __STDIO_H */
