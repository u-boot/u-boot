.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: binop (command)

binop command
=============

Synopsis
--------

::

    binop op count src1 src2 [dest]

Description
-----------

The binop command applies a bitwise binary operation to two byte arrays of
count bytes. Bytes are processed independently; no endian or word-size
conversion is performed. The result is written to dest or, if dest is
omitted, printed to the console.

op
    Operation to perform:

    * ``xor`` - set each bit where the operands differ
    * ``or`` - set each bit that is set in either operand
    * ``and`` - set each bit that is set in both operands

count
    Number of bytes to process, specified in decimal. Must be
    greater than zero.

src1, src2
    Input operands. Each operand is either:

    - an environment variable name, whose value is parsed as a hex string
    - a hex string, if no environment variable with that name exists
    - a memory address prefixed with '*', in which case count bytes are
      read from that address

    An existing environment variable takes precedence over interpreting
    the argument as a hex string. Prefixing an argument with ``*``
    explicitly selects memory access. Hex strings must contain an even
    number of hexadecimal digits and may not describe more than count
    bytes. Shorter values are zero-padded at the end.

dest
    Optional destination. If prefixed with '*', the result is written to
    that memory address. Otherwise dest is the name of an environment
    variable which receives the result as a lowercase hex string with two
    hex digits per byte. If dest is omitted, the result is printed as hex
    bytes separated by spaces, 16 bytes per line.

Addresses are parsed as hexadecimal numbers. The caller is responsible
for ensuring that the specified memory is accessible and, for
destinations, writable.

Invalid counts, operators, addresses or hex strings are rejected and the
destination is left unchanged.

Examples
--------

XOR two hex strings and store the result in an environment variable::

    => binop xor 4 01020304 ffffffff binop_out
    => printenv binop_out
    binop_out=fefdfcfb

AND two hex strings::

    => binop and 4 0f0f0f0f 00ff00ff binop_out
    => printenv binop_out
    binop_out=000f000f

Read an operand from an environment variable. The short value is
zero-padded::

    => setenv binop_a 0102
    => binop or 4 binop_a 03 binop_out
    => printenv binop_out
    binop_out=03020000

Print the result to the console::

    => binop xor 2 0102 0304
    02 06

Read from and write to memory (addresses are hexadecimal)::

    => binop xor 4 *0x80000000 01010101 *0x80000010

Configuration
-------------

The binop command is available if CONFIG_CMD_BINOP=y.

Return value
------------

The return value $? is 0 on success and 1 on failure.
