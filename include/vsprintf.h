/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __VSPRINTF_H
#define __VSPRINTF_H

#include <stdarg.h>
#include <linux/types.h>

/**
 * simple_strtoul - convert a string to an unsigned long
 *
 * @param cp	The string to be converted
 * @param endp	Updated to point to the first character not converted
 * @param base	The number base to use (0 for the default)
 * @return value decoded from string (0 if invalid)
 *
 * Converts a string to an unsigned long. If there are invalid characters at
 * the end these are ignored. In the worst case, if all characters are invalid,
 * 0 is returned
 *
 * A hex prefix is supported (e.g. 0x123) regardless of the value of @base.
 * If found, the base is set to hex (16).
 *
 * If @base is 0:
 *    - an octal '0' prefix (e.g. 0777) sets the base to octal (8).
 *    - otherwise the base defaults to decimal (10).
 */
ulong simple_strtoul(const char *cp, char **endp, unsigned int base);

/**
 * hex_strtoul - convert a string in hex to an unsigned long
 *
 * @param cp	The string to be converted
 * @param endp	Updated to point to the first character not converted
 * @return value decoded from string (0 if invalid)
 *
 * Converts a hex string to an unsigned long. If there are invalid characters at
 * the end these are ignored. In the worst case, if all characters are invalid,
 * 0 is returned
 */
unsigned long hextoul(const char *cp, char **endp);

/**
 * dec_strtoul - convert a string in decimal to an unsigned long
 *
 * @param cp	The string to be converted
 * @param endp	Updated to point to the first character not converted
 * @return value decoded from string (0 if invalid)
 *
 * Converts a decimal string to an unsigned long. If there are invalid
 * characters at the end these are ignored. In the worst case, if all characters
 * are invalid, 0 is returned
 */
unsigned long dectoul(const char *cp, char **endp);

/**
 * strict_strtoul - convert a string to an unsigned long strictly
 * @param cp	The string to be converted
 * @param base	The number base to use (0 for the default)
 * @param res	The converted result value
 * @return 0 if conversion is successful and *res is set to the converted
 * value, otherwise it returns -EINVAL and *res is set to 0.
 *
 * strict_strtoul converts a string to an unsigned long only if the
 * string is really an unsigned long string, any string containing
 * any invalid char at the tail will be rejected and -EINVAL is returned,
 * only a newline char at the tail is acceptible because people generally
 * change a module parameter in the following way:
 *
 *      echo 1024 > /sys/module/e1000/parameters/copybreak
 *
 * echo will append a newline to the tail.
 *
 * A hex prefix is supported (e.g. 0x123) regardless of the value of @base.
 * If found, the base is set to hex (16).
 *
 * If @base is 0:
 *    - an octal '0' prefix (e.g. 0777) sets the base to octal (8).
 *    - otherwise the base defaults to decimal (10).
 *
 * Copied this function from Linux 2.6.38 commit ID:
 * 521cb40b0c44418a4fd36dc633f575813d59a43d
 *
 */
int strict_strtoul(const char *cp, unsigned int base, unsigned long *res);
unsigned long long simple_strtoull(const char *cp, char **endp,
					unsigned int base);
long simple_strtol(const char *cp, char **endp, unsigned int base);
long long simple_strtoll(const char *cp, char **endp, unsigned int base);

/**
 * trailing_strtol() - extract a trailing integer from a string
 *
 * Given a string this finds a trailing number on the string and returns it.
 * For example, "abc123" would return 123.
 *
 * @str:	String to exxamine
 * @return training number if found, else -1
 */
long trailing_strtol(const char *str);

/**
 * trailing_strtoln() - extract a trailing integer from a fixed-length string
 *
 * Given a fixed-length string this finds a trailing number on the string
 * and returns it. For example, "abc123" would return 123. Only the
 * characters between @str and @end - 1 are examined. If @end is NULL, it is
 * set to str + strlen(str).
 *
 * @str:	String to exxamine
 * @end:	Pointer to end of string to examine, or NULL to use the
 *		whole string
 * @return training number if found, else -1
 */
long trailing_strtoln(const char *str, const char *end);

/**
 * panic() - Print a message and reset/hang
 *
 * Prints a message on the console(s) and then resets. If CONFIG_PANIC_HANG is
 * defined, then it will hang instead of resetting.
 *
 * @param fmt:	printf() format string for message, which should not include
 *		\n, followed by arguments
 */
void panic(const char *fmt, ...)
		__attribute__ ((format (__printf__, 1, 2), noreturn));

/**
 * panic_str() - Print a message and reset/hang
 *
 * Prints a message on the console(s) and then resets. If CONFIG_PANIC_HANG is
 * defined, then it will hang instead of resetting.
 *
 * This function can be used instead of panic() when your board does not
 * already use printf(), * to keep code size small.
 *
 * @param fmt:	string to display, which should not include \n
 */
void panic_str(const char *str) __attribute__ ((noreturn));

/**
 * Format a string and place it in a buffer
 *
 * @param buf	The buffer to place the result into
 * @param fmt	The format string to use
 * @param ...	Arguments for the format string
 *
 * The function returns the number of characters written
 * into @buf.
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int sprintf(char *buf, const char *fmt, ...)
		__attribute__ ((format (__printf__, 2, 3)));

/**
 * Format a string and place it in a buffer (va_list version)
 *
 * @param buf	The buffer to place the result into
 * @param fmt	The format string to use
 * @param args	Arguments for the format string
 * @return the number of characters which have been written into
 * the @buf not including the trailing '\0'.
 *
 * If you're not already dealing with a va_list consider using scnprintf().
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int vsprintf(char *buf, const char *fmt, va_list args);

/**
 * simple_itoa() - convert an unsigned integer to a string
 *
 * This returns a static string containing the decimal representation of the
 * given value. The returned value may be overwritten by other calls to other
 * simple_... functions, so should be used immediately
 *
 * @val: Value to convert
 * @return string containing the decimal representation of @val
 */
char *simple_itoa(ulong val);

/**
 * simple_xtoa() - convert an unsigned integer to a hex string
 *
 * This returns a static string containing the hexadecimal representation of the
 * given value. The returned value may be overwritten by other calls to other
 * simple_... functions, so should be used immediately
 *
 * @val: Value to convert
 * @return string containing the hexecimal representation of @val
 */
char *simple_xtoa(ulong num);

/**
 * Format a string and place it in a buffer
 *
 * @param buf	The buffer to place the result into
 * @param size	The size of the buffer, including the trailing null space
 * @param fmt	The format string to use
 * @param ...	Arguments for the format string
 * @return the number of characters which would be
 * generated for the given input, excluding the trailing null,
 * as per ISO C99.  If the return is greater than or equal to
 * @size, the resulting string is truncated.
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int snprintf(char *buf, size_t size, const char *fmt, ...)
		__attribute__ ((format (__printf__, 3, 4)));

/**
 * Format a string and place it in a buffer
 *
 * @param buf	The buffer to place the result into
 * @param size	The size of the buffer, including the trailing null space
 * @param fmt	The format string to use
 * @param ...	Arguments for the format string
 *
 * The return value is the number of characters written into @buf not including
 * the trailing '\0'. If @size is == 0 the function returns 0.
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int scnprintf(char *buf, size_t size, const char *fmt, ...)
		__attribute__ ((format (__printf__, 3, 4)));

/**
 * Format a string and place it in a buffer (base function)
 *
 * @param buf	The buffer to place the result into
 * @param size	The size of the buffer, including the trailing null space
 * @param fmt	The format string to use
 * @param args	Arguments for the format string
 * @return The number characters which would be generated for the given
 * input, excluding the trailing '\0', as per ISO C99. Note that fewer
 * characters may be written if this number of characters is >= size.
 *
 * This function follows C99 vsnprintf, but has some extensions:
 * %pS output the name of a text symbol
 * %pF output the name of a function pointer
 * %pR output the address range in a struct resource
 *
 * The function returns the number of characters which would be
 * generated for the given input, excluding the trailing '\0',
 * as per ISO C99.
 *
 * Call this function if you are already dealing with a va_list.
 * You probably want snprintf() instead.
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

/**
 * Format a string and place it in a buffer (va_list version)
 *
 * @param buf	The buffer to place the result into
 * @param size	The size of the buffer, including the trailing null space
 * @param fmt	The format string to use
 * @param args	Arguments for the format string
 * @return the number of characters which have been written into
 * the @buf not including the trailing '\0'. If @size is == 0 the function
 * returns 0.
 *
 * If you're not already dealing with a va_list consider using scnprintf().
 *
 * See the vsprintf() documentation for format string extensions over C99.
 */
int vscnprintf(char *buf, size_t size, const char *fmt, va_list args);

/**
 * print_grouped_ull() - print a value with digits grouped by ','
 *
 * This prints a value with grouped digits, like 12,345,678 to make it easier
 * to read.
 *
 * @val:	Value to print
 * @digits:	Number of digiits to print
 */
void print_grouped_ull(unsigned long long int_val, int digits);

bool str2off(const char *p, loff_t *num);
bool str2long(const char *p, ulong *num);

/**
 * strmhz() - Convert a value to a Hz string
 *
 * This creates a string indicating the number of MHz of a value. For example,
 * 2700000 produces "2.7".
 * @buf: Buffer to hold output string, which must be large enough
 * @hz: Value to convert
 */
char *strmhz(char *buf, unsigned long hz);

/**
 * str_to_upper() - Convert a string to upper case
 *
 * This simply uses toupper() on each character of the string.
 *
 * @in: String to convert (must be large enough to hold the output string)
 * @out: Buffer to put converted string
 * @len: Number of bytes available in @out (SIZE_MAX for all)
 */
void str_to_upper(const char *in, char *out, size_t len);

/**
 * sscanf - Unformat a buffer into a list of arguments
 * @buf:	input buffer
 * @fmt:	formatting of buffer
 * @...:	resulting arguments
 */
int sscanf(const char *buf, const char *fmt, ...);

#endif
