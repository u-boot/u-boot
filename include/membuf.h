/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * Copyright (c) 1992 Simon Glass
 */

#ifndef _membuf_H
#define _membuf_H

#include <stdbool.h>

/**
 * @struct membuf: holds the state of a membuff - it is used for input and
 * output buffers. The buffer extends from @start to (@start + @size - 1).
 * Data in the buffer extends from @tail to @head: it is written in at
 * @head and read out from @tail. The membuff is empty when @head == @tail
 * and full when adding another character would make @head == @tail. We
 * therefore waste one character in the membuff to avoid having an extra flag
 * to determine whether (when @head == @tail) the membuff is empty or full.
 *
 * xxxxxx  data
 * ......  empty
 *
 * .............xxxxxxxxxxxxxxxx.........................
 *		^		^
 * ^start	tail		head                     ^end
 *
 * xxxxxxxxxxxxx................xxxxxxxxxxxxxxxxxxxxxxxxx
 *		^		^
 *		head		tail
 */
struct membuf {
	char *start;		/** the start of the buffer */
	char *end;		/** the end of the buffer (start + length) */
	char *head;		/** current buffer head */
	char *tail;		/** current buffer tail */
};

/**
 * membuf_purge() - reset a membuff to the empty state
 *
 * Initialise head and tail pointers so that the membuff becomes empty.
 *
 * @mb: membuff to purge
 */
void membuf_purge(struct membuf *mb);

/**
 * membuf_putraw() - find out where bytes can be written
 *
 * Work out where in the membuff some data could be written. Return a pointer
 * to the address and the number of bytes which can be written there. If
 * @update is true, the caller must then write the data immediately, since
 * the membuff is updated as if the write has been done,
 *
 * Note that because the spare space in a membuff may not be contiguous, this
 * function may not return @maxlen even if there is enough space in the
 * membuff. However, by calling this function twice (with @update == true),
 * you will get access to all the spare space.
 *
 * @mb: membuff to adjust
 * @maxlen: the number of bytes we want to write
 * @update: true to update the membuff as if the write happened, false to not
 * @data: the address data can be written to
 * Return: number of bytes which can be written
 */
int membuf_putraw(struct membuf *mb, int maxlen, bool update, char **data);

/**
 * membuf_getraw() - find and return a pointer to available bytes
 *
 * Returns a pointer to any valid input data in the given membuff and
 * optionally marks it as read. Note that not all input data may not be
 * returned, since data is not necessarily contiguous in the membuff. However,
 * if you call this function twice (with @update == true) you are guaranteed
 * to get all available data, in at most two installments.
 *
 * @mb: membuff to adjust
 * @maxlen: maximum number of bytes to get
 * @update: true to update the membuff as if the bytes have been read (use
 * false to check bytes without reading them)
 * @data: returns address of data in input membuff
 * Return: the number of bytes available at *@data
 */
int membuf_getraw(struct membuf *mb, int maxlen, bool update, char **data);

/**
 * membuf_putbyte() - Writes a byte to a membuff
 *
 * @mb: membuff to adjust
 * @ch: byte to write
 * Return: true on success, false if membuff is full
 */
bool membuf_putbyte(struct membuf *mb, int ch);

/**
 * @mb: membuff to adjust
 * membuf_getbyte() - Read a byte from the membuff
 * Return: the byte read, or -1 if the membuff is empty
 */
int membuf_getbyte(struct membuf *mb);

/**
 * membuf_peekbyte() - check the next available byte
 *
 * Return the next byte which membuf_getbyte() would return, without
 * removing it from the membuff.
 *
 * @mb: membuff to adjust
 * Return: the byte peeked, or -1 if the membuff is empty
 */
int membuf_peekbyte(struct membuf *mb);

/**
 * membuf_get() - get data from a membuff
 *
 * Copies any available data (up to @maxlen bytes) to @buff and removes it
 * from the membuff.
 *
 * @mb: membuff to adjust
 * @Buff: address of membuff to transfer bytes to
 * @maxlen: maximum number of bytes to read
 * Return: the number of bytes read
 */
int membuf_get(struct membuf *mb, char *buff, int maxlen);

/**
 * membuf_put() - write data to a membuff
 *
 * Writes some data to a membuff. Returns the number of bytes added. If this
 * is less than @lnehgt, then the membuff got full
 *
 * @mb: membuff to adjust
 * @data: the data to write
 * @length: number of bytes to write from 'data'
 * Return: the number of bytes added
 */
int membuf_put(struct membuf *mb, const char *buff, int length);

/**
 * membuf_isempty() - check if a membuff is empty
 *
 * @mb: membuff to check
 * Return: true if empty, else false
 */
bool membuf_isempty(struct membuf *mb);

/**
 * membuf_avail() - check available data in a membuff
 *
 * @mb: membuff to check
 * Return: number of bytes of data available
 */
int membuf_avail(struct membuf *mb);

/**
 * membuf_size() - get the size of a membuff
 *
 * Note that a membuff can only old data up to one byte less than its size.
 *
 * @mb: membuff to check
 * Return: total size
 */
int membuf_size(struct membuf *mb);

/**
 * membuf_makecontig() - adjust all membuff data to be contiguous
 *
 * This places all data in a membuff into a single contiguous lump, if
 * possible
 *
 * @mb: membuff to adjust
 * Return: true on success
 */
bool membuf_makecontig(struct membuf *mb);

/**
 * membuf_free() - find the number of bytes that can be written to a membuff
 *
 * @mb: membuff to check
 * Return: returns the number of bytes free in a membuff
 */
int membuf_free(struct membuf *mb);

/**
 * membuf_readline() - read a line of text from a membuff
 *
 * Reads a line of text of up to 'maxlen' characters from a membuff and puts
 * it in @str. Any character less than @minch is assumed to be the end of
 * line character
 *
 * @mb: membuff to adjust
 * @str: Place to put the line
 * @maxlen: Maximum line length (excluding terminator)
 * @minch: Minimum ASCII character to permit as part of the line (e.g. ' ')
 * @must_fit: If true then str is empty if line doesn't fit
 * Return: number of bytes read (including terminator) if a line has been
 *	   read, 0 if nothing was there or line didn't fit when must_fit is set
 */
int membuf_readline(struct membuf *mb, char *str, int maxlen, int minch,
		    bool must_fit);

/**
 * membuf_extend_by() - expand a membuff
 *
 * Extends a membuff by the given number of bytes
 *
 * @mb: membuff to adjust
 * @by: Number of bytes to increase the size by
 * @max: Maximum size to allow
 * Return: 0 if the expand succeeded, -ENOMEM if not enough memory, -E2BIG
 * if the the size would exceed @max
 */
int membuf_extend_by(struct membuf *mb, int by, int max);

/**
 * membuf_init() - set up a new membuff using an existing membuff
 *
 * @mb: membuff to set up
 * @buff: Address of buffer
 * @size: Size of buffer
 */
void membuf_init(struct membuf *mb, char *buff, int size);

/**
 * membuf_uninit() - clear a membuff so it can no longer be used
 *
 * @mb: membuff to uninit
 */
void membuf_uninit(struct membuf *mb);

/**
 * membuf_new() - create a new membuff
 *
 * @mb: membuff to init
 * @size: size of membuff to create
 * Return: 0 if OK, -ENOMEM if out of memory
 */
int membuf_new(struct membuf *mb, int size);

/**
 * membuf_dispose() - free memory allocated to a membuff and uninit it
 *
 * @mb: membuff to dispose
 */
void membuf_dispose(struct membuf *mb);

#endif
