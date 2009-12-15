/*
 * This file is derived from various .h and .c files from the zlib-1.2.3
 * distribution by Jean-loup Gailly and Mark Adler, with some additions
 * by Paul Mackerras to aid in implementing Deflate compression and
 * decompression for PPP packets.  See zlib.h for conditions of
 * distribution and use.
 *
 * Changes that have been made include:
 * - changed functions not used outside this file to "local"
 * - added minCompression parameter to deflateInit2
 * - added Z_PACKET_FLUSH (see zlib.h for details)
 * - added inflateIncomp
 */

/*+++++*/
/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

#define ZUTIL_H
#define ZLIB_INTERNAL

#include <common.h>
#include <compiler.h>
#include <asm/unaligned.h>
#include "u-boot/zlib.h"
#undef	OFF				/* avoid conflicts */

/* To avoid a build time warning */
#ifdef STDC
#include <malloc.h>
#endif

#ifndef local
#define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

typedef unsigned char uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long ulg;

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]
#define ERR_RETURN(strm,err) return (strm->msg = (char*)ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

#ifndef NULL
#define NULL	((void *) 0)
#endif

	/* common constants */

#ifndef DEF_WBITS
#define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#define DEF_MEM_LEVEL 8
#else
#define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH 3
#define MAX_MATCH 258
/* The minimum and maximum match lengths */

	 /* functions */

#include <linux/string.h>
#define zmemcpy memcpy
#define zmemcmp memcmp
#define zmemzero(dest, len) memset(dest, 0, len)

/* Diagnostic functions */
#ifdef DEBUG
	extern int z_verbose;
	extern void z_error    OF((char *m));
#define Assert(cond,msg) {if(!(cond)) z_error(msg);}
#define fprintf(fp,...)	printf(__VA_ARGS__)
#define Trace(x) {if (z_verbose>=0) fprintf x ;}
#define Tracev(x) {if (z_verbose>0) fprintf x ;}
#define Tracevv(x) {if (z_verbose>1) fprintf x ;}
#define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;}
#define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;}
#else
#define Assert(cond,msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c,x)
#define Tracecv(c,x)
#endif

voidpf zcalloc OF((voidpf opaque, unsigned items, unsigned size));
void zcfree  OF((voidpf opaque, voidpf ptr, unsigned size));

#define ZALLOC(strm, items, size) \
	(*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr), 0)

/*+++++*/
/* inftrees.h -- header to use inftrees.c
 * Copyright (C) 1995-2005 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Structure for decoding tables.  Each entry provides either the
   information needed to do the operation requested by the code that
   indexed that table entry, or it provides a pointer to another
   table that indexes more bits of the code.  op indicates whether
   the entry is a pointer to another table, a literal, a length or
   distance, an end-of-block, or an invalid code.  For a table
   pointer, the low four bits of op is the number of index bits of
   that table.  For a length or distance, the low four bits of op
   is the number of extra bits to get after the code.  bits is
   the number of bits in this code or part of the code to drop off
   of the bit buffer.  val is the actual byte to output in the case
   of a literal, the base length or distance, or the offset from
   the current table to the next table.  Each entry is four bytes. */

typedef struct {
	unsigned char op;           /* operation, extra bits, table bits */
	unsigned char bits;         /* bits in this part of the code */
	unsigned short val;         /* offset in table or code value */
} code;

/* Maximum size of dynamic tree.  The maximum found in a long but non-
   exhaustive search was 1444 code structures (852 for length/literals
   and 592 for distances, the latter actually the result of an
   exhaustive search).  The true maximum is not known, but the value
   below is more than safe. */
#define ENOUGH 2048
#define MAXD 592

/* Type of code to build for inftable() */
typedef enum {
	CODES,
	LENS,
	DISTS
} codetype;

extern int inflate_table OF((codetype type, unsigned short FAR *lens,
				unsigned codes, code FAR * FAR *table,
				unsigned FAR *bits, unsigned short FAR *work));
/*+++++*/
/* inflate.h -- internal inflate state definition
 * Copyright (C) 1995-2004 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

#define GUNZIP

/* Possible inflate modes between inflate() calls */
typedef enum {
	HEAD, /* i: waiting for magic header */
	FLAGS, /* i: waiting for method and flags (gzip) */
	TIME, /* i: waiting for modification time (gzip) */
	OS, /* i: waiting for extra flags and operating system (gzip) */
	EXLEN, /* i: waiting for extra length (gzip) */
	EXTRA, /* i: waiting for extra bytes (gzip) */
	NAME, /* i: waiting for end of file name (gzip) */
	COMMENT, /* i: waiting for end of comment (gzip) */
	HCRC, /* i: waiting for header crc (gzip) */
	DICTID, /* i: waiting for dictionary check value */
	DICT, /* waiting for inflateSetDictionary() call */
	TYPE, /* i: waiting for type bits, including last-flag bit */
	TYPEDO, /* i: same, but skip check to exit inflate on new block */
	STORED, /* i: waiting for stored size (length and complement) */
	COPY, /* i/o: waiting for input or output to copy stored block */
	TABLE, /* i: waiting for dynamic block table lengths */
	LENLENS, /* i: waiting for code length code lengths */
	CODELENS, /* i: waiting for length/lit and distance code lengths */
	LEN, /* i: waiting for length/lit code */
	LENEXT, /* i: waiting for length extra bits */
	DIST, /* i: waiting for distance code */
	DISTEXT, /* i: waiting for distance extra bits */
	MATCH, /* o: waiting for output space to copy string */
	LIT, /* o: waiting for output space to write literal */
	CHECK, /* i: waiting for 32-bit check value */
	LENGTH, /* i: waiting for 32-bit length (gzip) */
	DONE, /* finished check, done -- remain here until reset */
	BAD, /* got a data error -- remain here until reset */
	MEM, /* got an inflate() memory error -- remain here until reset */
	SYNC, /* looking for synchronization bytes to restart inflate() */
	START,
	WASH,
	END,
	BADCODE
} inflate_mode;

/*
    State transitions between above modes -

    (most modes can go to the BAD or MEM mode -- not shown for clarity)

    Process header:
        HEAD -> (gzip) or (zlib)
        (gzip) -> FLAGS -> TIME -> OS -> EXLEN -> EXTRA -> NAME
        NAME -> COMMENT -> HCRC -> TYPE
        (zlib) -> DICTID or TYPE
        DICTID -> DICT -> TYPE
    Read deflate blocks:
            TYPE -> STORED or TABLE or LEN or CHECK
            STORED -> COPY -> TYPE
            TABLE -> LENLENS -> CODELENS -> LEN
    Read deflate codes:
                LEN -> LENEXT or LIT or TYPE
                LENEXT -> DIST -> DISTEXT -> MATCH -> LEN
                LIT -> LEN
    Process trailer:
        CHECK -> LENGTH -> DONE
 */

/* state maintained between inflate() calls.  Approximately 7K bytes. */
struct inflate_state {
	inflate_mode mode; /* current inflate mode */
	int last; /* true if processing last block */
	int wrap; /* bit 0 true for zlib, bit 1 true for gzip */
	int havedict; /* true if dictionary provided */
	int flags; /* gzip header method and flags (0 if zlib) */
	unsigned dmax; /* zlib header max distance (INFLATE_STRICT) */
	unsigned long check; /* protected copy of check value */
	unsigned long total; /* protected copy of output count */
	gz_headerp head; /* where to save gzip header information */
        /* sliding window */
	unsigned wbits; /* log base 2 of requested window size */
	unsigned wsize; /* window size or zero if not using window */
	unsigned whave; /* valid bytes in the window */
	unsigned write; /* window write index */
	unsigned char FAR *window; /* allocated sliding window, if needed */
        /* bit accumulator */
	unsigned long hold; /* input bit accumulator */
	unsigned bits; /* number of bits in "in" */
        /* for string and stored block copying */
	unsigned length; /* literal or length of data to copy */
	unsigned offset; /* distance back to copy string from */
        /* for table and code decoding */
	unsigned extra; /* extra bits needed */
        /* fixed and dynamic code tables */
	code const FAR *lencode; /* starting table for length/literal codes */
	code const FAR *distcode; /* starting table for distance codes */
	unsigned lenbits; /* index bits for lencode */
	unsigned distbits; /* index bits for distcode */
        /* dynamic table building */
	unsigned ncode; /* number of code length code lengths */
	unsigned nlen; /* number of length code lengths */
	unsigned ndist; /* number of distance code lengths */
	unsigned have; /* number of code lengths in lens[] */
	code FAR *next; /* next available space in codes[] */
	unsigned short lens[320]; /* temporary storage for code lengths */
	unsigned short work[288]; /* work area for code table building */
	code codes[ENOUGH]; /* space for code tables */
};

/*+++++*/
/* inffast.h -- header to use inffast.c
 * Copyright (C) 1995-2003 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

void inflate_fast OF((z_streamp strm, unsigned start));
/*+++++*/
    /* inffixed.h -- table for decoding fixed codes
     * Generated automatically by makefixed().
     */

    /* WARNING: this file should *not* be used by applications. It
       is part of the implementation of the compression library and
       is subject to change. Applications should only use zlib.h.
     */

	static const code lenfix[512] = {
	{96,7,0},{0,8,80},{0,8,16},{20,8,115},{18,7,31},{0,8,112},{0,8,48},
	{0,9,192},{16,7,10},{0,8,96},{0,8,32},{0,9,160},{0,8,0},{0,8,128},
	{0,8,64},{0,9,224},{16,7,6},{0,8,88},{0,8,24},{0,9,144},{19,7,59},
	{0,8,120},{0,8,56},{0,9,208},{17,7,17},{0,8,104},{0,8,40},{0,9,176},
	{0,8,8},{0,8,136},{0,8,72},{0,9,240},{16,7,4},{0,8,84},{0,8,20},
	{21,8,227},{19,7,43},{0,8,116},{0,8,52},{0,9,200},{17,7,13},{0,8,100},
	{0,8,36},{0,9,168},{0,8,4},{0,8,132},{0,8,68},{0,9,232},{16,7,8},
	{0,8,92},{0,8,28},{0,9,152},{20,7,83},{0,8,124},{0,8,60},{0,9,216},
	{18,7,23},{0,8,108},{0,8,44},{0,9,184},{0,8,12},{0,8,140},{0,8,76},
	{0,9,248},{16,7,3},{0,8,82},{0,8,18},{21,8,163},{19,7,35},{0,8,114},
	{0,8,50},{0,9,196},{17,7,11},{0,8,98},{0,8,34},{0,9,164},{0,8,2},
	{0,8,130},{0,8,66},{0,9,228},{16,7,7},{0,8,90},{0,8,26},{0,9,148},
	{20,7,67},{0,8,122},{0,8,58},{0,9,212},{18,7,19},{0,8,106},{0,8,42},
	{0,9,180},{0,8,10},{0,8,138},{0,8,74},{0,9,244},{16,7,5},{0,8,86},
	{0,8,22},{64,8,0},{19,7,51},{0,8,118},{0,8,54},{0,9,204},{17,7,15},
	{0,8,102},{0,8,38},{0,9,172},{0,8,6},{0,8,134},{0,8,70},{0,9,236},
	{16,7,9},{0,8,94},{0,8,30},{0,9,156},{20,7,99},{0,8,126},{0,8,62},
	{0,9,220},{18,7,27},{0,8,110},{0,8,46},{0,9,188},{0,8,14},{0,8,142},
	{0,8,78},{0,9,252},{96,7,0},{0,8,81},{0,8,17},{21,8,131},{18,7,31},
	{0,8,113},{0,8,49},{0,9,194},{16,7,10},{0,8,97},{0,8,33},{0,9,162},
	{0,8,1},{0,8,129},{0,8,65},{0,9,226},{16,7,6},{0,8,89},{0,8,25},
	{0,9,146},{19,7,59},{0,8,121},{0,8,57},{0,9,210},{17,7,17},{0,8,105},
	{0,8,41},{0,9,178},{0,8,9},{0,8,137},{0,8,73},{0,9,242},{16,7,4},
	{0,8,85},{0,8,21},{16,8,258},{19,7,43},{0,8,117},{0,8,53},{0,9,202},
	{17,7,13},{0,8,101},{0,8,37},{0,9,170},{0,8,5},{0,8,133},{0,8,69},
	{0,9,234},{16,7,8},{0,8,93},{0,8,29},{0,9,154},{20,7,83},{0,8,125},
	{0,8,61},{0,9,218},{18,7,23},{0,8,109},{0,8,45},{0,9,186},{0,8,13},
	{0,8,141},{0,8,77},{0,9,250},{16,7,3},{0,8,83},{0,8,19},{21,8,195},
	{19,7,35},{0,8,115},{0,8,51},{0,9,198},{17,7,11},{0,8,99},{0,8,35},
	{0,9,166},{0,8,3},{0,8,131},{0,8,67},{0,9,230},{16,7,7},{0,8,91},
	{0,8,27},{0,9,150},{20,7,67},{0,8,123},{0,8,59},{0,9,214},{18,7,19},
	{0,8,107},{0,8,43},{0,9,182},{0,8,11},{0,8,139},{0,8,75},{0,9,246},
	{16,7,5},{0,8,87},{0,8,23},{64,8,0},{19,7,51},{0,8,119},{0,8,55},
	{0,9,206},{17,7,15},{0,8,103},{0,8,39},{0,9,174},{0,8,7},{0,8,135},
	{0,8,71},{0,9,238},{16,7,9},{0,8,95},{0,8,31},{0,9,158},{20,7,99},
	{0,8,127},{0,8,63},{0,9,222},{18,7,27},{0,8,111},{0,8,47},{0,9,190},
	{0,8,15},{0,8,143},{0,8,79},{0,9,254},{96,7,0},{0,8,80},{0,8,16},
	{20,8,115},{18,7,31},{0,8,112},{0,8,48},{0,9,193},{16,7,10},{0,8,96},
	{0,8,32},{0,9,161},{0,8,0},{0,8,128},{0,8,64},{0,9,225},{16,7,6},
	{0,8,88},{0,8,24},{0,9,145},{19,7,59},{0,8,120},{0,8,56},{0,9,209},
	{17,7,17},{0,8,104},{0,8,40},{0,9,177},{0,8,8},{0,8,136},{0,8,72},
	{0,9,241},{16,7,4},{0,8,84},{0,8,20},{21,8,227},{19,7,43},{0,8,116},
	{0,8,52},{0,9,201},{17,7,13},{0,8,100},{0,8,36},{0,9,169},{0,8,4},
	{0,8,132},{0,8,68},{0,9,233},{16,7,8},{0,8,92},{0,8,28},{0,9,153},
	{20,7,83},{0,8,124},{0,8,60},{0,9,217},{18,7,23},{0,8,108},{0,8,44},
	{0,9,185},{0,8,12},{0,8,140},{0,8,76},{0,9,249},{16,7,3},{0,8,82},
	{0,8,18},{21,8,163},{19,7,35},{0,8,114},{0,8,50},{0,9,197},{17,7,11},
	{0,8,98},{0,8,34},{0,9,165},{0,8,2},{0,8,130},{0,8,66},{0,9,229},
	{16,7,7},{0,8,90},{0,8,26},{0,9,149},{20,7,67},{0,8,122},{0,8,58},
	{0,9,213},{18,7,19},{0,8,106},{0,8,42},{0,9,181},{0,8,10},{0,8,138},
	{0,8,74},{0,9,245},{16,7,5},{0,8,86},{0,8,22},{64,8,0},{19,7,51},
	{0,8,118},{0,8,54},{0,9,205},{17,7,15},{0,8,102},{0,8,38},{0,9,173},
	{0,8,6},{0,8,134},{0,8,70},{0,9,237},{16,7,9},{0,8,94},{0,8,30},
	{0,9,157},{20,7,99},{0,8,126},{0,8,62},{0,9,221},{18,7,27},{0,8,110},
	{0,8,46},{0,9,189},{0,8,14},{0,8,142},{0,8,78},{0,9,253},{96,7,0},
	{0,8,81},{0,8,17},{21,8,131},{18,7,31},{0,8,113},{0,8,49},{0,9,195},
	{16,7,10},{0,8,97},{0,8,33},{0,9,163},{0,8,1},{0,8,129},{0,8,65},
	{0,9,227},{16,7,6},{0,8,89},{0,8,25},{0,9,147},{19,7,59},{0,8,121},
	{0,8,57},{0,9,211},{17,7,17},{0,8,105},{0,8,41},{0,9,179},{0,8,9},
	{0,8,137},{0,8,73},{0,9,243},{16,7,4},{0,8,85},{0,8,21},{16,8,258},
	{19,7,43},{0,8,117},{0,8,53},{0,9,203},{17,7,13},{0,8,101},{0,8,37},
	{0,9,171},{0,8,5},{0,8,133},{0,8,69},{0,9,235},{16,7,8},{0,8,93},
	{0,8,29},{0,9,155},{20,7,83},{0,8,125},{0,8,61},{0,9,219},{18,7,23},
	{0,8,109},{0,8,45},{0,9,187},{0,8,13},{0,8,141},{0,8,77},{0,9,251},
	{16,7,3},{0,8,83},{0,8,19},{21,8,195},{19,7,35},{0,8,115},{0,8,51},
	{0,9,199},{17,7,11},{0,8,99},{0,8,35},{0,9,167},{0,8,3},{0,8,131},
	{0,8,67},{0,9,231},{16,7,7},{0,8,91},{0,8,27},{0,9,151},{20,7,67},
	{0,8,123},{0,8,59},{0,9,215},{18,7,19},{0,8,107},{0,8,43},{0,9,183},
	{0,8,11},{0,8,139},{0,8,75},{0,9,247},{16,7,5},{0,8,87},{0,8,23},
	{64,8,0},{19,7,51},{0,8,119},{0,8,55},{0,9,207},{17,7,15},{0,8,103},
	{0,8,39},{0,9,175},{0,8,7},{0,8,135},{0,8,71},{0,9,239},{16,7,9},
	{0,8,95},{0,8,31},{0,9,159},{20,7,99},{0,8,127},{0,8,63},{0,9,223},
	{18,7,27},{0,8,111},{0,8,47},{0,9,191},{0,8,15},{0,8,143},{0,8,79},
	{0,9,255}
	};

	static const code distfix[32] = {
	{16,5,1},{23,5,257},{19,5,17},{27,5,4097},{17,5,5},{25,5,1025},
	{21,5,65},{29,5,16385},{16,5,3},{24,5,513},{20,5,33},{28,5,8193},
	{18,5,9},{26,5,2049},{22,5,129},{64,5,0},{16,5,2},{23,5,385},
	{19,5,25},{27,5,6145},{17,5,7},{25,5,1537},{21,5,97},{29,5,24577},
	{16,5,4},{24,5,769},{20,5,49},{28,5,12289},{18,5,13},{26,5,3073},
	{22,5,193},{64,5,0}
	};

/*+++++*/
/* inffast.c -- fast decoding
 * Copyright (C) 1995-2004 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* Allow machine dependent optimization for post-increment or pre-increment.
   Based on testing to date,
   Pre-increment preferred for:
   - PowerPC G3 (Adler)
   - MIPS R5000 (Randers-Pehrson)
   Post-increment preferred for:
   - none
   No measurable difference:
   - Pentium III (Anderson)
   - M68060 (Nikl)
 */
#define OFF 1
#define PUP(a) *++(a)
#define UP_UNALIGNED(a) get_unaligned(++(a))

/*
   Decode literal, length, and distance codes and write out the resulting
   literal and match bytes until either not enough input or output is
   available, an end-of-block is encountered, or a data error is encountered.
   When large enough input and output buffers are supplied to inflate(), for
   example, a 16K input buffer and a 64K output buffer, more than 95% of the
   inflate execution time is spent in this routine.

   Entry assumptions:

        state->mode == LEN
        strm->avail_in >= 6
        strm->avail_out >= 258
        start >= strm->avail_out
        state->bits < 8

   On return, state->mode is one of:

        LEN -- ran out of enough output space or enough available input
        TYPE -- reached end of block code, inflate() to interpret next block
        BAD -- error in block data

   Notes:

    - The maximum input bits used by a length/distance pair is 15 bits for the
      length code, 5 bits for the length extra, 15 bits for the distance code,
      and 13 bits for the distance extra.  This totals 48 bits, or six bytes.
      Therefore if strm->avail_in >= 6, then there is enough input to avoid
      checking for available input while decoding.

    - The maximum bytes that a single length/distance pair can output is 258
      bytes, which is the maximum length that can be coded.  inflate_fast()
      requires strm->avail_out >= 258 for each loop to avoid checking for
      output space.
 */
void inflate_fast(strm, start)
z_streamp strm;
unsigned start;         /* inflate()'s starting value for strm->avail_out */
{
    struct inflate_state FAR *state;
    unsigned char FAR *in;      /* local strm->next_in */
    unsigned char FAR *last;    /* while in < last, enough input available */
    unsigned char FAR *out;     /* local strm->next_out */
    unsigned char FAR *beg;     /* inflate()'s initial strm->next_out */
    unsigned char FAR *end;     /* while out < end, enough space available */
#ifdef INFLATE_STRICT
    unsigned dmax;              /* maximum distance from zlib header */
#endif
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned write;             /* window write index */
    unsigned char FAR *window;  /* allocated sliding window, if wsize != 0 */
    unsigned long hold;         /* local strm->hold */
    unsigned bits;              /* local strm->bits */
    code const FAR *lcode;      /* local strm->lencode */
    code const FAR *dcode;      /* local strm->distcode */
    unsigned lmask;             /* mask for first level of length codes */
    unsigned dmask;             /* mask for first level of distance codes */
    code this;                  /* retrieved table entry */
    unsigned op;                /* code bits, operation, extra bits, or */
                                /*  window position, window bytes to copy */
    unsigned len;               /* match length, unused bytes */
    unsigned dist;              /* match distance */
    unsigned char FAR *from;    /* where to copy match from */

    /* copy state to local variables */
    state = (struct inflate_state FAR *)strm->state;
    in = strm->next_in - OFF;
    last = in + (strm->avail_in - 5);
    out = strm->next_out - OFF;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - 257);
#ifdef INFLATE_STRICT
    dmax = state->dmax;
#endif
    wsize = state->wsize;
    whave = state->whave;
    write = state->write;
    window = state->window;
    hold = state->hold;
    bits = state->bits;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    /* decode literals and length/distances until end-of-block or not enough
       input data or output space */
    do {
        if (bits < 15) {
            hold += (unsigned long)(PUP(in)) << bits;
            bits += 8;
            hold += (unsigned long)(PUP(in)) << bits;
            bits += 8;
        }
        this = lcode[hold & lmask];
      dolen:
        op = (unsigned)(this.bits);
        hold >>= op;
        bits -= op;
        op = (unsigned)(this.op);
        if (op == 0) {                          /* literal */
            Tracevv((stderr, this.val >= 0x20 && this.val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", this.val));
            PUP(out) = (unsigned char)(this.val);
        }
        else if (op & 16) {                     /* length base */
            len = (unsigned)(this.val);
            op &= 15;                           /* number of extra bits */
            if (op) {
                if (bits < op) {
                    hold += (unsigned long)(PUP(in)) << bits;
                    bits += 8;
                }
                len += (unsigned)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            if (bits < 15) {
                hold += (unsigned long)(PUP(in)) << bits;
                bits += 8;
                hold += (unsigned long)(PUP(in)) << bits;
                bits += 8;
            }
            this = dcode[hold & dmask];
          dodist:
            op = (unsigned)(this.bits);
            hold >>= op;
            bits -= op;
            op = (unsigned)(this.op);
            if (op & 16) {                      /* distance base */
                dist = (unsigned)(this.val);
                op &= 15;                       /* number of extra bits */
                if (bits < op) {
                    hold += (unsigned long)(PUP(in)) << bits;
                    bits += 8;
                    if (bits < op) {
                        hold += (unsigned long)(PUP(in)) << bits;
                        bits += 8;
                    }
                }
                dist += (unsigned)hold & ((1U << op) - 1);
#ifdef INFLATE_STRICT
                if (dist > dmax) {
                    strm->msg = (char *)"invalid distance too far back";
                    state->mode = BAD;
                    break;
                }
#endif
                hold >>= op;
                bits -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (unsigned)(out - beg);     /* max distance in output */
                if (dist > op) {                /* see if copy from window */
                    op = dist - op;             /* distance back in window */
                    if (op > whave) {
                        strm->msg = (char *)"invalid distance too far back";
                        state->mode = BAD;
                        break;
                    }
                    from = window - OFF;
                    if (write == 0) {           /* very common case */
                        from += wsize - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                PUP(out) = PUP(from);
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    else if (write < op) {      /* wrap around window */
                        from += wsize + write - op;
                        op -= write;
                        if (op < len) {         /* some from end of window */
                            len -= op;
                            do {
                                PUP(out) = PUP(from);
                            } while (--op);
                            from = window - OFF;
                            if (write < len) {  /* some from start of window */
                                op = write;
                                len -= op;
                                do {
                                    PUP(out) = PUP(from);
                                } while (--op);
                                from = out - dist;      /* rest from output */
                            }
                        }
                    }
                    else {                      /* contiguous in window */
                        from += write - op;
                        if (op < len) {         /* some from window */
                            len -= op;
                            do {
                                PUP(out) = PUP(from);
                            } while (--op);
                            from = out - dist;  /* rest from output */
                        }
                    }
                    while (len > 2) {
                        PUP(out) = PUP(from);
                        PUP(out) = PUP(from);
                        PUP(out) = PUP(from);
                        len -= 3;
                    }
                    if (len) {
                        PUP(out) = PUP(from);
                        if (len > 1)
                            PUP(out) = PUP(from);
                    }
                }
                else {
		    unsigned short *sout;
		    unsigned long loops;

                    from = out - dist;          /* copy direct from output */
                    /* minimum length is three */
		    /* Align out addr */
		    if (!((long)(out - 1 + OFF) & 1)) {
			PUP(out) = PUP(from);
			len--;
		    }
		    sout = (unsigned short *)(out - OFF);
		    if (dist > 2 ) {
			unsigned short *sfrom;

			sfrom = (unsigned short *)(from - OFF);
			loops = len >> 1;
			do
			    PUP(sout) = UP_UNALIGNED(sfrom);
			while (--loops);
			out = (unsigned char *)sout + OFF;
			from = (unsigned char *)sfrom + OFF;
		    } else { /* dist == 1 or dist == 2 */
			unsigned short pat16;

			pat16 = *(sout-2+2*OFF);
			if (dist == 1)
#if defined(__BIG_ENDIAN)
			    pat16 = (pat16 & 0xff) | ((pat16 & 0xff ) << 8);
#elif defined(__LITTLE_ENDIAN)
			    pat16 = (pat16 & 0xff00) | ((pat16 & 0xff00 ) >> 8);
#else
#error __BIG_ENDIAN nor __LITTLE_ENDIAN is defined
#endif
			loops = len >> 1;
			do
			    PUP(sout) = pat16;
			while (--loops);
			out = (unsigned char *)sout + OFF;
		    }
		    if (len & 1)
			PUP(out) = PUP(from);
                }
            }
            else if ((op & 64) == 0) {          /* 2nd level distance code */
                this = dcode[this.val + (hold & ((1U << op) - 1))];
                goto dodist;
            }
            else {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
        }
        else if ((op & 64) == 0) {              /* 2nd level length code */
            this = lcode[this.val + (hold & ((1U << op) - 1))];
            goto dolen;
        }
        else if (op & 32) {                     /* end-of-block */
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            break;
        }
        else {
            strm->msg = (char *)"invalid literal/length code";
            state->mode = BAD;
            break;
        }
    } while (in < last && out < end);

    /* return unused bytes (on entry, bits < 8, so in won't go too far back) */
    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    /* update state and return */
    strm->next_in = in + OFF;
    strm->next_out = out + OFF;
    strm->avail_in = (unsigned)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (unsigned)(out < end ?
                                 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
    return;
}

/*
   inflate_fast() speedups that turned out slower (on a PowerPC G3 750CXe):
   - Using bit fields for code structure
   - Different op definition to avoid & for extra bits (do & for table bits)
   - Three separate decoding do-loops for direct, window, and write == 0
   - Special case for distance > 1 copies to do overlapped load and store copy
   - Explicit branch predictions (based on measured branch probabilities)
   - Deferring match copy and interspersed it with decoding subsequent codes
   - Swapping literal/length else
   - Swapping window/direct else
   - Larger unrolled copy loops (three is about right)
   - Moving len -= 3 statement into middle of loop
 */

/*+++++*/
/* inftrees.c -- generate Huffman trees for efficient decoding
 * Copyright (C) 1995-2005 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

#define MAXBITS 15
/*
  If you use the zlib library in a product, an acknowledgment is welcome
  in the documentation of your product. If for some reason you cannot
  include such an acknowledgment, I would appreciate that you keep this
  copyright string in the executable of your product.
 */

/*
   Build a set of tables to decode the provided canonical Huffman code.
   The code lengths are lens[0..codes-1].  The result starts at *table,
   whose indices are 0..2^bits-1.  work is a writable array of at least
   lens shorts, which is used as a work area.  type is the type of code
   to be generated, CODES, LENS, or DISTS.  On return, zero is success,
   -1 is an invalid code, and +1 means that ENOUGH isn't enough.  table
   on return points to the next available entry's address.  bits is the
   requested root table index bits, and on return it is the actual root
   table index bits.  It will differ if the request is greater than the
   longest code or if it is less than the shortest code.
 */
int inflate_table(type, lens, codes, table, bits, work)
codetype type;
unsigned short FAR *lens;
unsigned codes;
code FAR * FAR *table;
unsigned FAR *bits;
unsigned short FAR *work;
{
    unsigned len;               /* a code's length in bits */
    unsigned sym;               /* index of code symbols */
    unsigned min, max;          /* minimum and maximum code lengths */
    unsigned root;              /* number of index bits for root table */
    unsigned curr;              /* number of index bits for current table */
    unsigned drop;              /* code bits to drop for sub-table */
    int left;                   /* number of prefix codes available */
    unsigned used;              /* code entries in table used */
    unsigned huff;              /* Huffman code */
    unsigned incr;              /* for incrementing code, index */
    unsigned fill;              /* index for replicating entries */
    unsigned low;               /* low bits for current root entry */
    unsigned mask;              /* mask for low root bits */
    code this;                  /* table entry for duplication */
    code FAR *next;             /* next available space in table */
    const unsigned short FAR *base;     /* base value table to use */
    const unsigned short FAR *extra;    /* extra bits table to use */
    int end;                    /* use base and extra for symbol > end */
    unsigned short count[MAXBITS+1];    /* number of codes of each length */
    unsigned short offs[MAXBITS+1];     /* offsets in table for each length */
    static const unsigned short lbase[31] = { /* Length codes 257..285 base */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
    static const unsigned short lext[31] = { /* Length codes 257..285 extra */
        16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18,
        19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 16, 201, 196};
    static const unsigned short dbase[32] = { /* Distance codes 0..29 base */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577, 0, 0};
    static const unsigned short dext[32] = { /* Distance codes 0..29 extra */
        16, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22,
        23, 23, 24, 24, 25, 25, 26, 26, 27, 27,
        28, 28, 29, 29, 64, 64};

    /*
       Process a set of code lengths to create a canonical Huffman code.  The
       code lengths are lens[0..codes-1].  Each length corresponds to the
       symbols 0..codes-1.  The Huffman code is generated by first sorting the
       symbols by length from short to long, and retaining the symbol order
       for codes with equal lengths.  Then the code starts with all zero bits
       for the first code of the shortest length, and the codes are integer
       increments for the same length, and zeros are appended as the length
       increases.  For the deflate format, these bits are stored backwards
       from their more natural integer increment ordering, and so when the
       decoding tables are built in the large loop below, the integer codes
       are incremented backwards.

       This routine assumes, but does not check, that all of the entries in
       lens[] are in the range 0..MAXBITS.  The caller must assure this.
       1..MAXBITS is interpreted as that code length.  zero means that that
       symbol does not occur in this code.

       The codes are sorted by computing a count of codes for each length,
       creating from that a table of starting indices for each length in the
       sorted table, and then entering the symbols in order in the sorted
       table.  The sorted table is work[], with that space being provided by
       the caller.

       The length counts are used for other purposes as well, i.e. finding
       the minimum and maximum length codes, determining if there are any
       codes at all, checking for a valid set of lengths, and looking ahead
       at length counts to determine sub-table sizes when building the
       decoding tables.
     */

    /* accumulate lengths for codes (assumes lens[] all in 0..MAXBITS) */
    for (len = 0; len <= MAXBITS; len++)
        count[len] = 0;
    for (sym = 0; sym < codes; sym++)
        count[lens[sym]]++;

    /* bound code lengths, force root to be within code lengths */
    root = *bits;
    for (max = MAXBITS; max >= 1; max--)
        if (count[max] != 0) break;
    if (root > max) root = max;
    if (max == 0) {                     /* no symbols to code at all */
        this.op = (unsigned char)64;    /* invalid code marker */
        this.bits = (unsigned char)1;
        this.val = (unsigned short)0;
        *(*table)++ = this;             /* make a table to force an error */
        *(*table)++ = this;
        *bits = 1;
        return 0;     /* no symbols, but wait for decoding to report error */
    }
    for (min = 1; min <= MAXBITS; min++)
        if (count[min] != 0) break;
    if (root < min) root = min;

    /* check for an over-subscribed or incomplete set of lengths */
    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= count[len];
        if (left < 0) return -1;        /* over-subscribed */
    }
    if (left > 0 && (type == CODES || max != 1))
        return -1;                      /* incomplete set */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + count[len];

    /* sort symbols by length, by symbol order within each length */
    for (sym = 0; sym < codes; sym++)
        if (lens[sym] != 0) work[offs[lens[sym]]++] = (unsigned short)sym;

    /*
       Create and fill in decoding tables.  In this loop, the table being
       filled is at next and has curr index bits.  The code being used is huff
       with length len.  That code is converted to an index by dropping drop
       bits off of the bottom.  For codes where len is less than drop + curr,
       those top drop + curr - len bits are incremented through all values to
       fill the table with replicated entries.

       root is the number of index bits for the root table.  When len exceeds
       root, sub-tables are created pointed to by the root entry with an index
       of the low root bits of huff.  This is saved in low to check for when a
       new sub-table should be started.  drop is zero when the root table is
       being filled, and drop is root when sub-tables are being filled.

       When a new sub-table is needed, it is necessary to look ahead in the
       code lengths to determine what size sub-table is needed.  The length
       counts are used for this, and so count[] is decremented as codes are
       entered in the tables.

       used keeps track of how many table entries have been allocated from the
       provided *table space.  It is checked when a LENS table is being made
       against the space in *table, ENOUGH, minus the maximum space needed by
       the worst case distance code, MAXD.  This should never happen, but the
       sufficiency of ENOUGH has not been proven exhaustively, hence the check.
       This assumes that when type == LENS, bits == 9.

       sym increments through all symbols, and the loop terminates when
       all codes of length max, i.e. all codes, have been processed.  This
       routine permits incomplete codes, so another loop after this one fills
       in the rest of the decoding tables with invalid code markers.
     */

    /* set up for code type */
    switch (type) {
    case CODES:
        base = extra = work;    /* dummy value--not used */
        end = 19;
        break;
    case LENS:
        base = lbase;
        base -= 257;
        extra = lext;
        extra -= 257;
        end = 256;
        break;
    default:            /* DISTS */
        base = dbase;
        extra = dext;
        end = -1;
    }

    /* initialize state for loop */
    huff = 0;                   /* starting code */
    sym = 0;                    /* starting code symbol */
    len = min;                  /* starting code length */
    next = *table;              /* current table to fill in */
    curr = root;                /* current table index bits */
    drop = 0;                   /* current bits to drop from code for index */
    low = (unsigned)(-1);       /* trigger new sub-table when len > root */
    used = 1U << root;          /* use root table entries */
    mask = used - 1;            /* mask for comparing low */

    /* check available table space */
    if (type == LENS && used >= ENOUGH - MAXD)
        return 1;

    /* process all codes and make table entries */
    for (;;) {
        /* create table entry */
        this.bits = (unsigned char)(len - drop);
        if ((int)(work[sym]) < end) {
            this.op = (unsigned char)0;
            this.val = work[sym];
        }
        else if ((int)(work[sym]) > end) {
            this.op = (unsigned char)(extra[work[sym]]);
            this.val = base[work[sym]];
        }
        else {
            this.op = (unsigned char)(32 + 64);         /* end of block */
            this.val = 0;
        }

        /* replicate for those indices with low len bits equal to huff */
        incr = 1U << (len - drop);
        fill = 1U << curr;
        min = fill;                 /* save offset to next table */
        do {
            fill -= incr;
            next[(huff >> drop) + fill] = this;
        } while (fill != 0);

        /* backwards increment the len-bit code huff */
        incr = 1U << (len - 1);
        while (huff & incr)
            incr >>= 1;
        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        }
        else
            huff = 0;

        /* go to next symbol, update count, len */
        sym++;
        if (--(count[len]) == 0) {
            if (len == max) break;
            len = lens[work[sym]];
        }

        /* create new sub-table if needed */
        if (len > root && (huff & mask) != low) {
            /* if first time, transition to sub-tables */
            if (drop == 0)
                drop = root;

            /* increment past last table */
            next += min;            /* here min is 1 << curr */

            /* determine length of next table */
            curr = len - drop;
            left = (int)(1 << curr);
            while (curr + drop < max) {
                left -= count[curr + drop];
                if (left <= 0) break;
                curr++;
                left <<= 1;
            }

            /* check for enough space */
            used += 1U << curr;
            if (type == LENS && used >= ENOUGH - MAXD)
                return 1;

            /* point entry in root table to sub-table */
            low = huff & mask;
            (*table)[low].op = (unsigned char)curr;
            (*table)[low].bits = (unsigned char)root;
            (*table)[low].val = (unsigned short)(next - *table);
        }
    }

    /*
       Fill in rest of table for incomplete codes.  This loop is similar to the
       loop above in incrementing huff for table indices.  It is assumed that
       len is equal to curr + drop, so there is no loop needed to increment
       through high index bits.  When the current sub-table is filled, the loop
       drops back to the root table to fill in any remaining entries there.
     */
    this.op = (unsigned char)64;                /* invalid code marker */
    this.bits = (unsigned char)(len - drop);
    this.val = (unsigned short)0;
    while (huff != 0) {
        /* when done with sub-table, drop back to root table */
        if (drop != 0 && (huff & mask) != low) {
            drop = 0;
            len = root;
            next = *table;
            this.bits = (unsigned char)len;
        }

        /* put invalid code marker in table */
        next[huff >> drop] = this;

        /* backwards increment the len-bit code huff */
        incr = 1U << (len - 1);
        while (huff & incr)
            incr >>= 1;
        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        }
        else
            huff = 0;
    }

    /* set return parameters */
    *table += used;
    *bits = root;
    return 0;
}

/*+++++*/
/* inflate.c -- zlib decompression
 * Copyright (C) 1995-2005 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
local void fixedtables OF((struct inflate_state FAR *state));
local int updatewindow OF((z_streamp strm, unsigned out));

int ZEXPORT inflateReset(strm)
z_streamp strm;
{
    struct inflate_state FAR *state;

    if (strm == Z_NULL || strm->state == Z_NULL) return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    strm->total_in = strm->total_out = state->total = 0;
    strm->msg = Z_NULL;
    strm->adler = 1;        /* to support ill-conceived Java test suite */
    state->mode = HEAD;
    state->last = 0;
    state->havedict = 0;
    state->dmax = 32768U;
    state->head = Z_NULL;
    state->wsize = 0;
    state->whave = 0;
    state->write = 0;
    state->hold = 0;
    state->bits = 0;
    state->lencode = state->distcode = state->next = state->codes;
    if (strm->outcb != Z_NULL)
	(*strm->outcb)(Z_NULL, 0);
    Tracev((stderr, "inflate: reset\n"));
    return Z_OK;
}

int ZEXPORT inflateInit2_(strm, windowBits, version, stream_size)
z_streamp strm;
int windowBits;
const char *version;
int stream_size;
{
    struct inflate_state FAR *state;

    if (version == Z_NULL || version[0] != ZLIB_VERSION[0] ||
        stream_size != (int)(sizeof(z_stream)))
        return Z_VERSION_ERROR;
    if (strm == Z_NULL) return Z_STREAM_ERROR;
    strm->msg = Z_NULL;                 /* in case we return an error */
    if (strm->zalloc == (alloc_func)0) {
        strm->zalloc = zcalloc;
        strm->opaque = (voidpf)0;
    }
    if (strm->zfree == (free_func)0) strm->zfree = zcfree;
    state = (struct inflate_state FAR *)
            ZALLOC(strm, 1, sizeof(struct inflate_state));
    if (state == Z_NULL) return Z_MEM_ERROR;
    Tracev((stderr, "inflate: allocated\n"));
    strm->state = (struct internal_state FAR *)state;
    if (windowBits < 0) {
        state->wrap = 0;
        windowBits = -windowBits;
    }
    else {
        state->wrap = (windowBits >> 4) + 1;
#ifdef GUNZIP
        if (windowBits < 48) windowBits &= 15;
#endif
    }
    if (windowBits < 8 || windowBits > 15) {
        ZFREE(strm, state);
        strm->state = Z_NULL;
        return Z_STREAM_ERROR;
    }
    state->wbits = (unsigned)windowBits;
    state->window = Z_NULL;
    return inflateReset(strm);
}

int ZEXPORT inflateInit_(strm, version, stream_size)
z_streamp strm;
const char *version;
int stream_size;
{
    return inflateInit2_(strm, DEF_WBITS, version, stream_size);
}

local void fixedtables(state)
struct inflate_state FAR *state;
{
    state->lencode = lenfix;
    state->lenbits = 9;
    state->distcode = distfix;
    state->distbits = 5;
}

/*
   Update the window with the last wsize (normally 32K) bytes written before
   returning.  If window does not exist yet, create it.  This is only called
   when a window is already in use, or when output has been written during this
   inflate call, but the end of the deflate stream has not been reached yet.
   It is also called to create a window for dictionary data when a dictionary
   is loaded.

   Providing output buffers larger than 32K to inflate() should provide a speed
   advantage, since only the last 32K of output is copied to the sliding window
   upon return from inflate(), and since all distances after the first 32K of
   output will fall in the output data, making match copies simpler and faster.
   The advantage may be dependent on the size of the processor's data caches.
 */
local int updatewindow(strm, out)
z_streamp strm;
unsigned out;
{
    struct inflate_state FAR *state;
    unsigned copy, dist;

    state = (struct inflate_state FAR *)strm->state;

    /* if it hasn't been done already, allocate space for the window */
    if (state->window == Z_NULL) {
        state->window = (unsigned char FAR *)
                        ZALLOC(strm, 1U << state->wbits,
                               sizeof(unsigned char));
        if (state->window == Z_NULL) return 1;
    }

    /* if window not in use yet, initialize */
    if (state->wsize == 0) {
        state->wsize = 1U << state->wbits;
        state->write = 0;
        state->whave = 0;
    }

    /* copy state->wsize or less output bytes into the circular window */
    copy = out - strm->avail_out;
    if (copy >= state->wsize) {
        zmemcpy(state->window, strm->next_out - state->wsize, state->wsize);
        state->write = 0;
        state->whave = state->wsize;
    }
    else {
        dist = state->wsize - state->write;
        if (dist > copy) dist = copy;
        zmemcpy(state->window + state->write, strm->next_out - copy, dist);
        copy -= dist;
        if (copy) {
            zmemcpy(state->window, strm->next_out - copy, copy);
            state->write = copy;
            state->whave = state->wsize;
        }
        else {
            state->write += dist;
            if (state->write == state->wsize) state->write = 0;
            if (state->whave < state->wsize) state->whave += dist;
        }
    }
    return 0;
}

/* Macros for inflate(): */

/* check function to use adler32() for zlib or crc32() for gzip */
#define UPDATE(check, buf, len) \
	(state->flags ? crc32(check, buf, len) : adler32(check, buf, len))

/* check macros for header crc */
#define CRC2(check, word) \
	do { \
		hbuf[0] = (unsigned char)(word); \
		hbuf[1] = (unsigned char)((word) >> 8); \
		check = crc32(check, hbuf, 2); \
	} while (0)

#define CRC4(check, word) \
	do { \
		hbuf[0] = (unsigned char)(word); \
		hbuf[1] = (unsigned char)((word) >> 8); \
		hbuf[2] = (unsigned char)((word) >> 16); \
		hbuf[3] = (unsigned char)((word) >> 24); \
		check = crc32(check, hbuf, 4); \
	} while (0)

/* Load registers with state in inflate() for speed */
#define LOAD() \
	do { \
		put = strm->next_out; \
		left = strm->avail_out; \
		next = strm->next_in; \
		have = strm->avail_in; \
		hold = state->hold; \
		bits = state->bits; \
	} while (0)

/* Restore state from registers in inflate() */
#define RESTORE() \
	do { \
		strm->next_out = put; \
		strm->avail_out = left; \
		strm->next_in = next; \
		strm->avail_in = have; \
		state->hold = hold; \
		state->bits = bits; \
	} while (0)

/* Clear the input bit accumulator */
#define INITBITS() \
	do { \
		hold = 0; \
		bits = 0; \
	} while (0)

/* Get a byte of input into the bit accumulator, or return from inflate()
   if there is no input available. */
#define PULLBYTE() \
	do { \
		if (have == 0) goto inf_leave; \
		have--; \
		hold += (unsigned long)(*next++) << bits; \
		bits += 8; \
	} while (0)

/* Assure that there are at least n bits in the bit accumulator.  If there is
   not enough available input to do that, then return from inflate(). */
#define NEEDBITS(n) \
	do { \
		while (bits < (unsigned)(n)) \
			PULLBYTE(); \
	} while (0)

/* Return the low n bits of the bit accumulator (n < 16) */
#define BITS(n) \
	((unsigned)hold & ((1U << (n)) - 1))

/* Remove n bits from the bit accumulator */
#define DROPBITS(n) \
	do { \
		hold >>= (n); \
		bits -= (unsigned)(n); \
	} while (0)

/* Remove zero to seven bits as needed to go to a byte boundary */
#define BYTEBITS() \
	do { \
		hold >>= bits & 7; \
		bits -= bits & 7; \
	} while (0)

/* Reverse the bytes in a 32-bit value */
#define REVERSE(q) \
	((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
		(((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

/*
   inflate() uses a state machine to process as much input data and generate as
   much output data as possible before returning.  The state machine is
   structured roughly as follows:

    for (;;) switch (state) {
    ...
    case STATEn:
        if (not enough input data or output space to make progress)
            return;
        ... make progress ...
        state = STATEm;
        break;
    ...
    }

   so when inflate() is called again, the same case is attempted again, and
   if the appropriate resources are provided, the machine proceeds to the
   next state.  The NEEDBITS() macro is usually the way the state evaluates
   whether it can proceed or should return.  NEEDBITS() does the return if
   the requested bits are not available.  The typical use of the BITS macros
   is:

        NEEDBITS(n);
        ... do something with BITS(n) ...
        DROPBITS(n);

   where NEEDBITS(n) either returns from inflate() if there isn't enough
   input left to load n bits into the accumulator, or it continues.  BITS(n)
   gives the low n bits in the accumulator.  When done, DROPBITS(n) drops
   the low n bits off the accumulator.  INITBITS() clears the accumulator
   and sets the number of available bits to zero.  BYTEBITS() discards just
   enough bits to put the accumulator on a byte boundary.  After BYTEBITS()
   and a NEEDBITS(8), then BITS(8) would return the next byte in the stream.

   NEEDBITS(n) uses PULLBYTE() to get an available byte of input, or to return
   if there is no input available.  The decoding of variable length codes uses
   PULLBYTE() directly in order to pull just enough bytes to decode the next
   code, and no more.

   Some states loop until they get enough input, making sure that enough
   state information is maintained to continue the loop where it left off
   if NEEDBITS() returns in the loop.  For example, want, need, and keep
   would all have to actually be part of the saved state in case NEEDBITS()
   returns:

    case STATEw:
        while (want < need) {
            NEEDBITS(n);
            keep[want++] = BITS(n);
            DROPBITS(n);
        }
        state = STATEx;
    case STATEx:

   As shown above, if the next state is also the next case, then the break
   is omitted.

   A state may also return if there is not enough output space available to
   complete that state.  Those states are copying stored data, writing a
   literal byte, and copying a matching string.

   When returning, a "goto inf_leave" is used to update the total counters,
   update the check value, and determine whether any progress has been made
   during that inflate() call in order to return the proper return code.
   Progress is defined as a change in either strm->avail_in or strm->avail_out.
   When there is a window, goto inf_leave will update the window with the last
   output written.  If a goto inf_leave occurs in the middle of decompression
   and there is no window currently, goto inf_leave will create one and copy
   output to the window for the next call of inflate().

   In this implementation, the flush parameter of inflate() only affects the
   return code (per zlib.h).  inflate() always writes as much as possible to
   strm->next_out, given the space available and the provided input--the effect
   documented in zlib.h of Z_SYNC_FLUSH.  Furthermore, inflate() always defers
   the allocation of and copying into a sliding window until necessary, which
   provides the effect documented in zlib.h for Z_FINISH when the entire input
   stream available.  So the only thing the flush parameter actually does is:
   when flush is set to Z_FINISH, inflate() cannot return Z_OK.  Instead it
   will return Z_BUF_ERROR if it has not reached the end of the stream.
 */
int ZEXPORT inflate(strm, flush)
z_streamp strm;
int flush;
{
    struct inflate_state FAR *state;
    unsigned char FAR *next;    /* next input */
    unsigned char FAR *put;     /* next output */
    unsigned have, left;        /* available input and output */
    unsigned long hold;         /* bit buffer */
    unsigned bits;              /* bits in bit buffer */
    unsigned in, out;           /* save starting available input and output */
    unsigned copy;              /* number of stored or match bytes to copy */
    unsigned char FAR *from;    /* where to copy match bytes from */
    code this;                  /* current decoding table entry */
    code last;                  /* parent table entry */
    unsigned len;               /* length to copy for repeats, bits to drop */
    int ret;                    /* return code */
#ifdef GUNZIP
    unsigned char hbuf[4];      /* buffer for gzip header crc calculation */
#endif
    static const unsigned short order[19] = /* permutation of code lengths */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    if (strm == Z_NULL || strm->state == Z_NULL ||
        (strm->next_in == Z_NULL && strm->avail_in != 0))
        return Z_STREAM_ERROR;

    state = (struct inflate_state FAR *)strm->state;
    if (state->mode == TYPE) state->mode = TYPEDO;      /* skip check */
    LOAD();
    in = have;
    out = left;
    ret = Z_OK;
    for (;;)
        switch (state->mode) {
        case HEAD:
            if (state->wrap == 0) {
                state->mode = TYPEDO;
                break;
            }
            NEEDBITS(16);
#ifdef GUNZIP
            if ((state->wrap & 2) && hold == 0x8b1f) {  /* gzip header */
                state->check = crc32(0L, Z_NULL, 0);
                CRC2(state->check, hold);
                INITBITS();
                state->mode = FLAGS;
                break;
            }
            state->flags = 0;           /* expect zlib header */
            if (state->head != Z_NULL)
                state->head->done = -1;
            if (!(state->wrap & 1) ||   /* check if zlib header allowed */
#else
            if (
#endif
                ((BITS(8) << 8) + (hold >> 8)) % 31) {
                strm->msg = (char *)"incorrect header check";
                state->mode = BAD;
                break;
            }
            if (BITS(4) != Z_DEFLATED) {
                strm->msg = (char *)"unknown compression method";
                state->mode = BAD;
                break;
            }
            DROPBITS(4);
            len = BITS(4) + 8;
            if (len > state->wbits) {
                strm->msg = (char *)"invalid window size";
                state->mode = BAD;
                break;
            }
            state->dmax = 1U << len;
            Tracev((stderr, "inflate:   zlib header ok\n"));
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = hold & 0x200 ? DICTID : TYPE;
            INITBITS();
            break;
#ifdef GUNZIP
        case FLAGS:
            NEEDBITS(16);
            state->flags = (int)(hold);
            if ((state->flags & 0xff) != Z_DEFLATED) {
                strm->msg = (char *)"unknown compression method";
                state->mode = BAD;
                break;
            }
            if (state->flags & 0xe000) {
                strm->msg = (char *)"unknown header flags set";
                state->mode = BAD;
                break;
            }
            if (state->head != Z_NULL)
                state->head->text = (int)((hold >> 8) & 1);
            if (state->flags & 0x0200) CRC2(state->check, hold);
            INITBITS();
            state->mode = TIME;
        case TIME:
            NEEDBITS(32);
            if (state->head != Z_NULL)
                state->head->time = hold;
            if (state->flags & 0x0200) CRC4(state->check, hold);
            INITBITS();
            state->mode = OS;
        case OS:
            NEEDBITS(16);
            if (state->head != Z_NULL) {
                state->head->xflags = (int)(hold & 0xff);
                state->head->os = (int)(hold >> 8);
            }
            if (state->flags & 0x0200) CRC2(state->check, hold);
            INITBITS();
            state->mode = EXLEN;
        case EXLEN:
            if (state->flags & 0x0400) {
                NEEDBITS(16);
                state->length = (unsigned)(hold);
                if (state->head != Z_NULL)
                    state->head->extra_len = (unsigned)hold;
                if (state->flags & 0x0200) CRC2(state->check, hold);
                INITBITS();
            }
            else if (state->head != Z_NULL)
                state->head->extra = Z_NULL;
            state->mode = EXTRA;
        case EXTRA:
            if (state->flags & 0x0400) {
                copy = state->length;
                if (copy > have) copy = have;
                if (copy) {
                    if (state->head != Z_NULL &&
                        state->head->extra != Z_NULL) {
                        len = state->head->extra_len - state->length;
                        zmemcpy(state->head->extra + len, next,
                                len + copy > state->head->extra_max ?
                                state->head->extra_max - len : copy);
                    }
                    if (state->flags & 0x0200)
                        state->check = crc32(state->check, next, copy);
                    have -= copy;
                    next += copy;
                    state->length -= copy;
                }
                if (state->length) goto inf_leave;
            }
            state->length = 0;
            state->mode = NAME;
        case NAME:
            if (state->flags & 0x0800) {
                if (have == 0) goto inf_leave;
                copy = 0;
                do {
                    len = (unsigned)(next[copy++]);
                    if (state->head != Z_NULL &&
                            state->head->name != Z_NULL &&
                            state->length < state->head->name_max)
                        state->head->name[state->length++] = len;
                } while (len && copy < have);
                if (state->flags & 0x0200)
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                if (len) goto inf_leave;
            }
            else if (state->head != Z_NULL)
                state->head->name = Z_NULL;
            state->length = 0;
            state->mode = COMMENT;
        case COMMENT:
            if (state->flags & 0x1000) {
                if (have == 0) goto inf_leave;
                copy = 0;
                do {
                    len = (unsigned)(next[copy++]);
                    if (state->head != Z_NULL &&
                            state->head->comment != Z_NULL &&
                            state->length < state->head->comm_max)
                        state->head->comment[state->length++] = len;
                } while (len && copy < have);
                if (state->flags & 0x0200)
                    state->check = crc32(state->check, next, copy);
                have -= copy;
                next += copy;
                if (len) goto inf_leave;
            }
            else if (state->head != Z_NULL)
                state->head->comment = Z_NULL;
            state->mode = HCRC;
        case HCRC:
            if (state->flags & 0x0200) {
                NEEDBITS(16);
                if (hold != (state->check & 0xffff)) {
                    strm->msg = (char *)"header crc mismatch";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
            }
            if (state->head != Z_NULL) {
                state->head->hcrc = (int)((state->flags >> 9) & 1);
                state->head->done = 1;
            }
            strm->adler = state->check = crc32(0L, Z_NULL, 0);
            state->mode = TYPE;
            break;
#endif
        case DICTID:
            NEEDBITS(32);
            strm->adler = state->check = REVERSE(hold);
            INITBITS();
            state->mode = DICT;
        case DICT:
            if (state->havedict == 0) {
                RESTORE();
                return Z_NEED_DICT;
            }
            strm->adler = state->check = adler32(0L, Z_NULL, 0);
            state->mode = TYPE;
        case TYPE:
            if (flush == Z_BLOCK) goto inf_leave;
        case TYPEDO:
            if (state->last) {
                BYTEBITS();
                state->mode = CHECK;
                break;
            }
            NEEDBITS(3);
            state->last = BITS(1);
            DROPBITS(1);
            switch (BITS(2)) {
            case 0:                             /* stored block */
                Tracev((stderr, "inflate:     stored block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = STORED;
                break;
            case 1:                             /* fixed block */
                fixedtables(state);
                Tracev((stderr, "inflate:     fixed codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = LEN;              /* decode codes */
                break;
            case 2:                             /* dynamic block */
                Tracev((stderr, "inflate:     dynamic codes block%s\n",
                        state->last ? " (last)" : ""));
                state->mode = TABLE;
                break;
            case 3:
                strm->msg = (char *)"invalid block type";
                state->mode = BAD;
            }
            DROPBITS(2);
            break;
        case STORED:
            BYTEBITS();                         /* go to byte boundary */
            NEEDBITS(32);
            if ((hold & 0xffff) != ((hold >> 16) ^ 0xffff)) {
                strm->msg = (char *)"invalid stored block lengths";
                state->mode = BAD;
                break;
            }
            state->length = (unsigned)hold & 0xffff;
            Tracev((stderr, "inflate:       stored length %u\n",
                    state->length));
            INITBITS();
            state->mode = COPY;
        case COPY:
            copy = state->length;
            if (copy) {
                if (copy > have) copy = have;
                if (copy > left) copy = left;
                if (copy == 0) goto inf_leave;
                zmemcpy(put, next, copy);
                have -= copy;
                next += copy;
                left -= copy;
                put += copy;
                state->length -= copy;
                break;
            }
            Tracev((stderr, "inflate:       stored end\n"));
            state->mode = TYPE;
            break;
        case TABLE:
            NEEDBITS(14);
            state->nlen = BITS(5) + 257;
            DROPBITS(5);
            state->ndist = BITS(5) + 1;
            DROPBITS(5);
            state->ncode = BITS(4) + 4;
            DROPBITS(4);
#ifndef PKZIP_BUG_WORKAROUND
            if (state->nlen > 286 || state->ndist > 30) {
                strm->msg = (char *)"too many length or distance symbols";
                state->mode = BAD;
                break;
            }
#endif
            Tracev((stderr, "inflate:       table sizes ok\n"));
            state->have = 0;
            state->mode = LENLENS;
        case LENLENS:
            while (state->have < state->ncode) {
                NEEDBITS(3);
                state->lens[order[state->have++]] = (unsigned short)BITS(3);
                DROPBITS(3);
            }
            while (state->have < 19)
                state->lens[order[state->have++]] = 0;
            state->next = state->codes;
            state->lencode = (code const FAR *)(state->next);
            state->lenbits = 7;
            ret = inflate_table(CODES, state->lens, 19, &(state->next),
                                &(state->lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid code lengths set";
                state->mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       code lengths ok\n"));
            state->have = 0;
            state->mode = CODELENS;
        case CODELENS:
            while (state->have < state->nlen + state->ndist) {
                for (;;) {
                    this = state->lencode[BITS(state->lenbits)];
                    if ((unsigned)(this.bits) <= bits) break;
                    PULLBYTE();
                }
                if (this.val < 16) {
                    NEEDBITS(this.bits);
                    DROPBITS(this.bits);
                    state->lens[state->have++] = this.val;
                }
                else {
                    if (this.val == 16) {
                        NEEDBITS(this.bits + 2);
                        DROPBITS(this.bits);
                        if (state->have == 0) {
                            strm->msg = (char *)"invalid bit length repeat";
                            state->mode = BAD;
                            break;
                        }
                        len = state->lens[state->have - 1];
                        copy = 3 + BITS(2);
                        DROPBITS(2);
                    }
                    else if (this.val == 17) {
                        NEEDBITS(this.bits + 3);
                        DROPBITS(this.bits);
                        len = 0;
                        copy = 3 + BITS(3);
                        DROPBITS(3);
                    }
                    else {
                        NEEDBITS(this.bits + 7);
                        DROPBITS(this.bits);
                        len = 0;
                        copy = 11 + BITS(7);
                        DROPBITS(7);
                    }
                    if (state->have + copy > state->nlen + state->ndist) {
                        strm->msg = (char *)"invalid bit length repeat";
                        state->mode = BAD;
                        break;
                    }
                    while (copy--)
                        state->lens[state->have++] = (unsigned short)len;
                }
            }

            /* handle error breaks in while */
            if (state->mode == BAD) break;

            /* build code tables */
            state->next = state->codes;
            state->lencode = (code const FAR *)(state->next);
            state->lenbits = 9;
            ret = inflate_table(LENS, state->lens, state->nlen, &(state->next),
                                &(state->lenbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid literal/lengths set";
                state->mode = BAD;
                break;
            }
            state->distcode = (code const FAR *)(state->next);
            state->distbits = 6;
            ret = inflate_table(DISTS, state->lens + state->nlen, state->ndist,
                            &(state->next), &(state->distbits), state->work);
            if (ret) {
                strm->msg = (char *)"invalid distances set";
                state->mode = BAD;
                break;
            }
            Tracev((stderr, "inflate:       codes ok\n"));
            state->mode = LEN;
        case LEN:
            if (strm->outcb != Z_NULL) /* for watchdog (U-Boot) */
                (*strm->outcb)(Z_NULL, 0);
            if (have >= 6 && left >= 258) {
                RESTORE();
                inflate_fast(strm, out);
                LOAD();
                break;
            }
            for (;;) {
                this = state->lencode[BITS(state->lenbits)];
                if ((unsigned)(this.bits) <= bits) break;
                PULLBYTE();
            }
            if (this.op && (this.op & 0xf0) == 0) {
                last = this;
                for (;;) {
                    this = state->lencode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + this.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(this.bits);
            state->length = (unsigned)this.val;
            if ((int)(this.op) == 0) {
                Tracevv((stderr, this.val >= 0x20 && this.val < 0x7f ?
                        "inflate:         literal '%c'\n" :
                        "inflate:         literal 0x%02x\n", this.val));
                state->mode = LIT;
                break;
            }
            if (this.op & 32) {
                Tracevv((stderr, "inflate:         end of block\n"));
                state->mode = TYPE;
                break;
            }
            if (this.op & 64) {
                strm->msg = (char *)"invalid literal/length code";
                state->mode = BAD;
                break;
            }
            state->extra = (unsigned)(this.op) & 15;
            state->mode = LENEXT;
        case LENEXT:
            if (state->extra) {
                NEEDBITS(state->extra);
                state->length += BITS(state->extra);
                DROPBITS(state->extra);
            }
            Tracevv((stderr, "inflate:         length %u\n", state->length));
            state->mode = DIST;
        case DIST:
            for (;;) {
                this = state->distcode[BITS(state->distbits)];
                if ((unsigned)(this.bits) <= bits) break;
                PULLBYTE();
            }
            if ((this.op & 0xf0) == 0) {
                last = this;
                for (;;) {
                    this = state->distcode[last.val +
                            (BITS(last.bits + last.op) >> last.bits)];
                    if ((unsigned)(last.bits + this.bits) <= bits) break;
                    PULLBYTE();
                }
                DROPBITS(last.bits);
            }
            DROPBITS(this.bits);
            if (this.op & 64) {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
            state->offset = (unsigned)this.val;
            state->extra = (unsigned)(this.op) & 15;
            state->mode = DISTEXT;
        case DISTEXT:
            if (state->extra) {
                NEEDBITS(state->extra);
                state->offset += BITS(state->extra);
                DROPBITS(state->extra);
            }
#ifdef INFLATE_STRICT
            if (state->offset > state->dmax) {
                strm->msg = (char *)"invalid distance too far back";
                state->mode = BAD;
                break;
            }
#endif
            if (state->offset > state->whave + out - left) {
                strm->msg = (char *)"invalid distance too far back";
                state->mode = BAD;
                break;
            }
            Tracevv((stderr, "inflate:         distance %u\n", state->offset));
            state->mode = MATCH;
        case MATCH:
            if (left == 0) goto inf_leave;
            copy = out - left;
            if (state->offset > copy) {         /* copy from window */
                copy = state->offset - copy;
                if (copy > state->write) {
                    copy -= state->write;
                    from = state->window + (state->wsize - copy);
                }
                else
                    from = state->window + (state->write - copy);
                if (copy > state->length) copy = state->length;
            }
            else {                              /* copy from output */
                from = put - state->offset;
                copy = state->length;
            }
            if (copy > left) copy = left;
            left -= copy;
            state->length -= copy;
            do {
                *put++ = *from++;
            } while (--copy);
            if (state->length == 0) state->mode = LEN;
            break;
        case LIT:
            if (left == 0) goto inf_leave;
            *put++ = (unsigned char)(state->length);
            left--;
            state->mode = LEN;
            break;
        case CHECK:
            if (state->wrap) {
                NEEDBITS(32);
                out -= left;
                strm->total_out += out;
                state->total += out;
                if (out)
                    strm->adler = state->check =
                        UPDATE(state->check, put - out, out);
                out = left;
                if ((
#ifdef GUNZIP
                     state->flags ? hold :
#endif
                     REVERSE(hold)) != state->check) {
                    strm->msg = (char *)"incorrect data check";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
                Tracev((stderr, "inflate:   check matches trailer\n"));
            }
#ifdef GUNZIP
            state->mode = LENGTH;
        case LENGTH:
            if (state->wrap && state->flags) {
                NEEDBITS(32);
                if (hold != (state->total & 0xffffffffUL)) {
                    strm->msg = (char *)"incorrect length check";
                    state->mode = BAD;
                    break;
                }
                INITBITS();
                Tracev((stderr, "inflate:   length matches trailer\n"));
            }
#endif
            state->mode = DONE;
        case DONE:
            ret = Z_STREAM_END;
            goto inf_leave;
        case BAD:
            ret = Z_DATA_ERROR;
            goto inf_leave;
        case MEM:
            return Z_MEM_ERROR;
        case SYNC:
        default:
            return Z_STREAM_ERROR;
        }

    /*
       Return from inflate(), updating the total counts and the check value.
       If there was no progress during the inflate() call, return a buffer
       error.  Call updatewindow() to create and/or update the window state.
       Note: a memory error from inflate() is non-recoverable.
     */
  inf_leave:
    RESTORE();
    if (state->wsize || (state->mode < CHECK && out != strm->avail_out))
        if (updatewindow(strm, out)) {
            state->mode = MEM;
            return Z_MEM_ERROR;
        }
    in -= strm->avail_in;
    out -= strm->avail_out;
    strm->total_in += in;
    strm->total_out += out;
    state->total += out;
    if (state->wrap && out)
        strm->adler = state->check =
            UPDATE(state->check, strm->next_out - out, out);
    strm->data_type = state->bits + (state->last ? 64 : 0) +
                      (state->mode == TYPE ? 128 : 0);
    if (((in == 0 && out == 0) || flush == Z_FINISH) && ret == Z_OK)
        ret = Z_BUF_ERROR;
    return ret;
}

int ZEXPORT inflateEnd(strm)
z_streamp strm;
{
    struct inflate_state FAR *state;
    if (strm == Z_NULL || strm->state == Z_NULL || strm->zfree == (free_func)0)
        return Z_STREAM_ERROR;
    state = (struct inflate_state FAR *)strm->state;
    if (state->window != Z_NULL) {
	if (strm->outcb != Z_NULL)
		(*strm->outcb)(Z_NULL, 0);
	ZFREE(strm, state->window);
    }
    ZFREE(strm, strm->state);
    strm->state = Z_NULL;
    Tracev((stderr, "inflate: end\n"));
    return Z_OK;
}

/*+++++*/
/* zutil.c -- target dependent utility functions for the compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ifndef NO_DUMMY_DECL
struct internal_state	{int dummy;}; /* for buggy compilers */
#endif

const char * const z_errmsg[10] = {
"need dictionary",     /* Z_NEED_DICT       2  */
"stream end",          /* Z_STREAM_END      1  */
"",                    /* Z_OK              0  */
"file error",          /* Z_ERRNO         (-1) */
"stream error",        /* Z_STREAM_ERROR  (-2) */
"data error",          /* Z_DATA_ERROR    (-3) */
"insufficient memory", /* Z_MEM_ERROR     (-4) */
"buffer error",        /* Z_BUF_ERROR     (-5) */
"incompatible version",/* Z_VERSION_ERROR (-6) */
""};

#ifdef DEBUG

#ifndef verbose
#define verbose 0
#endif
int z_verbose = verbose;

void z_error (m)
    char *m;
{
	fprintf(stderr, "%s\n", m);
	hang ();
}
#endif

/* exported to allow conversion of error code to string for compress() and
 * uncompress()
 */
#ifndef MY_ZCALLOC /* Any system without a special alloc function */

#ifndef STDC
extern voidp    malloc OF((uInt size));
extern voidp    calloc OF((uInt items, uInt size));
extern void     free   OF((voidpf ptr));
#endif

voidpf zcalloc (opaque, items, size)
	voidpf opaque;
	unsigned items;
	unsigned size;
{
	if (opaque)
		items += size - size; /* make compiler happy */
	return sizeof(uInt) > 2 ? (voidpf)malloc(items * size) :
		(voidpf)calloc(items, size);
}

void  zcfree (opaque, ptr, nb)
	voidpf opaque;
	voidpf ptr;
	unsigned nb;
{
	free(ptr);
	if (opaque)
		return; /* make compiler happy */
}

#endif /* MY_ZCALLOC */
/*+++++*/
/* adler32.c -- compute the Adler-32 checksum of a data stream
 * Copyright (C) 1995-2004 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define BASE 65521UL    /* largest prime smaller than 65536 */
#define NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

/* use NO_DIVIDE if your processor does not do division in hardware */
#ifdef NO_DIVIDE
#define MOD(a) \
	do { \
		if (a >= (BASE << 16)) \
			a -= (BASE << 16); \
		if (a >= (BASE << 15)) \
			a -= (BASE << 15); \
		if (a >= (BASE << 14)) \
			a -= (BASE << 14); \
		if (a >= (BASE << 13)) \
			a -= (BASE << 13); \
		if (a >= (BASE << 12)) \
			a -= (BASE << 12); \
		if (a >= (BASE << 11)) \
			a -= (BASE << 11); \
		if (a >= (BASE << 10)) \
			a -= (BASE << 10); \
		if (a >= (BASE << 9)) \
			a -= (BASE << 9); \
		if (a >= (BASE << 8)) \
			a -= (BASE << 8); \
		if (a >= (BASE << 7)) \
			a -= (BASE << 7); \
		if (a >= (BASE << 6)) \
			a -= (BASE << 6); \
		if (a >= (BASE << 5)) \
			a -= (BASE << 5); \
		if (a >= (BASE << 4)) \
			a -= (BASE << 4); \
		if (a >= (BASE << 3)) \
			a -= (BASE << 3); \
		if (a >= (BASE << 2)) \
			a -= (BASE << 2); \
		if (a >= (BASE << 1)) \
			a -= (BASE << 1); \
		if (a >= BASE) \
			a -= BASE; \
	} while (0)
#define MOD4(a) \
	do { \
		if (a >= (BASE << 4)) \
			a -= (BASE << 4); \
		if (a >= (BASE << 3)) \
			a -= (BASE << 3); \
		if (a >= (BASE << 2)) \
			a -= (BASE << 2); \
		if (a >= (BASE << 1)) \
			a -= (BASE << 1); \
		if (a >= BASE) \
			a -= BASE; \
	} while (0)
#else
#define MOD(a) a %= BASE
#define MOD4(a) a %= BASE
#endif

/* ========================================================================= */
uLong ZEXPORT adler32(adler, buf, len)
    uLong adler;
    const Bytef *buf;
    uInt len;
{
    unsigned long sum2;
    unsigned n;

    /* split Adler-32 into component sums */
    sum2 = (adler >> 16) & 0xffff;
    adler &= 0xffff;

    /* in case user likes doing a byte at a time, keep it fast */
    if (len == 1) {
        adler += buf[0];
        if (adler >= BASE)
            adler -= BASE;
        sum2 += adler;
        if (sum2 >= BASE)
            sum2 -= BASE;
        return adler | (sum2 << 16);
    }

    /* initial Adler-32 value (deferred check for len == 1 speed) */
    if (buf == Z_NULL)
        return 1L;

    /* in case short lengths are provided, keep it somewhat fast */
    if (len < 16) {
        while (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        if (adler >= BASE)
            adler -= BASE;
        MOD4(sum2);             /* only added so many BASE's */
        return adler | (sum2 << 16);
    }

    /* do length NMAX blocks -- requires just one modulo operation */
    while (len >= NMAX) {
        len -= NMAX;
        n = NMAX / 16;          /* NMAX is divisible by 16 */
        do {
            DO16(buf);          /* 16 sums unrolled */
            buf += 16;
        } while (--n);
        MOD(adler);
        MOD(sum2);
    }

    /* do remaining bytes (less than NMAX, still just one modulo) */
    if (len) {                  /* avoid modulos if none remaining */
        while (len >= 16) {
            len -= 16;
            DO16(buf);
            buf += 16;
        }
        while (len--) {
            adler += *buf++;
            sum2 += adler;
        }
        MOD(adler);
        MOD(sum2);
    }

    /* return recombined sums */
    return adler | (sum2 << 16);
}
