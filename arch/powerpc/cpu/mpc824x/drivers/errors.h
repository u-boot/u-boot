/*	Copyright Motorola, Inc. 1993, 1994
	ALL RIGHTS RESERVED

	You are hereby granted a copyright license to use, modify, and
	distribute the SOFTWARE so long as this entire notice is retained
	without alteration in any modified and/or redistributed versions,
	and that such modified versions are clearly identified as such.
	No licenses are granted by implication, estoppel or otherwise under
	any patents or trademarks of Motorola, Inc.

	The SOFTWARE is provided on an "AS IS" basis and without warranty.
	To the maximum extent permitted by applicable law, MOTOROLA DISCLAIMS
	ALL WARRANTIES WHETHER EXPRESS OR IMPLIED, INCLUDING IMPLIED
	WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR
	PURPOSE AND ANY WARRANTY AGAINST INFRINGEMENT WITH
	REGARD TO THE SOFTWARE (INCLUDING ANY MODIFIED VERSIONS
	THEREOF) AND ANY ACCOMPANYING WRITTEN MATERIALS.

	To the maximum extent permitted by applicable law, IN NO EVENT SHALL
	MOTOROLA BE LIABLE FOR ANY DAMAGES WHATSOEVER
	(INCLUDING WITHOUT LIMITATION, DAMAGES FOR LOSS OF
	BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS
	INFORMATION, OR OTHER PECUNIARY LOSS) ARISING OF THE USE OR
	INABILITY TO USE THE SOFTWARE.   Motorola assumes no responsibility
	for the maintenance and support of the SOFTWARE.

*/


#include "config.h"

/*
	 1         2         3         4         5         6         7         8
01234567890123456789012345678901234567890123456789012345678901234567890123456789
*/
/* List define statements here */

/* These are for all the toolboxes and functions to use. These will help
to standardize the error handling in the current project */

				/* this is the "data type" for the error
				messages in the system */
#define STATUS unsigned int

				/* this is a success status code */
#define SUCCESS 1

				/* likewise this is failure */
#define FAILURE 0

#define NUM_ERRORS 47

/* This first section of "defines" are for error codes ONLY.  The called
   routine will return one of these error codes to the caller.  If the final
   returned code is "VALID", then everything is a-okay.  However, if one
   of the functions returns a non-valid status, that error code should be
   propogated back to all the callers.  At the end, the last caller will
   call an error_processing function, and send in the status which was
   returned.  It's up to the error_processing function to determine which
   error occured (as indicated by the status), and print an appropriate
   message back to the user.
*/
/*----------------------------------------------------------------------*/
/* these are specifically for the parser routines			*/

#define UNKNOWN_COMMAND		0xfb00 /* "unrecognized command " */
#define UNKNOWN_REGISTER	0xfb01 /* "unknown register "*/
#define ILLEGAL_RD_STAGE	0xfb02 /* cannot specify reg. family in range*/
#define ILLEGAL_REG_FAMILY	0xfb03 /* "cannot specify a range of special
					or miscellaneous registers"*/
#define RANGE_CROSS_FAMILY	0xfb04 /* "cannot specify a range across
					register families" */
#define UNIMPLEMENTED_STAGE	0xfb05 /* invalid rd or rmm parameter format */
#define REG_NOT_WRITEABLE	0xfb06 /* "unknown operator in arguements"*/
#define INVALID_FILENAME	0xfb07 /* "invalid download filename" */
#define INVALID_BAUD_RATE	0xfb08	/* invalid baud rate from sb command */
#define UNSUPPORTED_REGISTER	0xfb09	/* Special register is not supported */
#define FOR_BOARD_ONLY		0xfb0a  /* "Not available for Unix." */


/*----------------------------------------------------------------------*/
/* these are for the error checking toolbox				*/

#define INVALID			0xfd00 /* NOT valid */
#define VALID			0xfd01 /* valid */

					/* This error is found in the fcn:
					is_right_size_input() to indicate
					that the input was not 8 characters
					long.  */
#define INVALID_SIZE		0xfd02

					/* This error is found in the fcn:
					is_valid_address_range() to indicate
					that the address given falls outside
					of valid memory defined by MEM_START
					to MEM_END.
					*/
#define OUT_OF_BOUNDS_ADDRESS	0xfd03

					/* This error is found in the fcn:
					is_valid_hex_input() to indicate that
					one of more of the characters entered
					are not valid hex characters.  Valid
					hex characters are 0-9, A-F, a-f.
					*/
#define INVALID_HEX_INPUT	0xfd04

					/* This error is found in the fcn:
					is_valid_register_number() to indicate
					that a given register does not exist.
					*/
#define REG_NOT_READABLE	0xfd05

					/* This error is found in the fcn:
					is_word_aligned_address() to indicate
					that the given address is not word-
					aligned.  A word-aligned address ends
					in 0x0,0x4,0x8,0xc.
					*/
#define	NOT_WORD_ALIGNED	0xfd07

					/* This error is found in the fcn:
					is_valid_address_range() to indicate
					that the starting address is greater
					than the ending address.
					*/
#define REVERSED_ADDRESS	0xfd08

					/* this error tells us that the address
					specified as the destination is within
					the source addresses  */
#define RANGE_OVERLAP		0xfd09


#define	ERROR			0xfd0a /* An error occured */
#define INVALID_PARAM		0xfd0b /* "invalid input parameter " */


#define INVALID_FLAG		0xfd0c	/* invalid flag */

/*----------------------------------------------------------------------*/
/* these are for the getarg toolbox					*/

#define INVALID_NUMBER_ARGS	0xFE00 /* invalid number of commd arguements */
#define UNKNOWN_PARAMETER	0xFE01 /* "unknown type of parameter "*/


/*----------------------------------------------------------------------*/
/* these are for the tokenizer toolbox					*/

#define ILLEGAL_CHARACTER	0xFF00 /* unrecognized char. in input stream*/
#define TTL_NOT_SORTED		0xFF01 /* token translation list not sorted */
#define TTL_NOT_DEFINED		0xFF02 /* token translation list not assigned*/
#define INVALID_STRING		0xFF03 /* unable to extract string from input */
#define BUFFER_EMPTY		0xFF04 /* "input buffer is empty" */
#define INVALID_MODE		0xFF05 /* input buf is in an unrecognized mode*/
#define TOK_INTERNAL_ERROR	0xFF06 /* "internal tokenizer error" */
#define TOO_MANY_IBS		0xFF07 /* "too many open input buffers" */
#define NO_OPEN_IBS		0xFF08 /* "no open input buffers" */


/* these are for the read from screen toolbox */

#define RESERVED_WORD		0xFC00 /* used a reserved word as an arguement*/


/* these are for the breakpoint routines */

#define FULL_BPDS		0xFA00 /* breakpoint data structure is full */


/* THESE are for the downloader */

#define NOT_IN_S_RECORD_FORMAT	0xf900 /* "not in S-Record Format" */
#define UNREC_RECORD_TYPE	0xf901 /* "unrecognized record type" */
#define CONVERSION_ERROR	0xf902 /* "ascii to int conversion error" */
#define INVALID_MEMORY		0xf903 /* "bad s-record memory address " */


/* these are for the compression and decompression stuff */

#define COMP_UNK_CHARACTER	0xf800 /* "unknown compressed character " */

#define COMP_UNKNOWN_STATE	0xf801 /* "unknown binary state" */

#define NOT_IN_COMPRESSED_FORMAT 0xf802 /* not in compressed S-Record format */


/* these are for the DUART handling things */

					/* "unrecognized serial port configuration" */
#define UNKNOWN_PORT_STATE	0xf700


/* these are for the register toolbox */

					/* "cannot find register in special
					 purpose register file " */
#define SPR_NOT_FOUND		0xf600


/* these are for the duart specific stuff */

					/* "transparent mode needs access to
						two serial ports" */
#define TM_NEEDS_BOTH_PORTS	0xf500


/*----------------------------------------------------------------------*/
/* these are specifically for the flash routines			*/
#define FLASH_ERROR		0xf100		/* general flash error */
