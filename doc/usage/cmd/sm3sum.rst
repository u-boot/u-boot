.. SPDX-License-Identifier: GPL-2.0-or-later
.. Copyright Nabla Software Engineering GmbH
   Written by Heiko Schocher <hs@nabladev.com>

.. index::
   single: sm3sum (command)

sm3sum command
==============

Synopsis
--------

::

    sm3sum - compute SM3 message digest

    Usage:
    sm3sum address count [[*]sum]
      - compute SM3 message digest [save to sum]
    sm3sum -v address count [*]sum
      - verify sm3sum of memory area


Description
-----------

The sm3sum command calculates the SM3 hash of data of ``count`` bytes
at address ``address``. If the ``-v`` option is passed to the command,
it compares the calculated hash with the hash found at address ``sum``.

The SM3 secure hash is calculated as specified by OSCCA GM/T
0004-2012 SM3 and described at

https://datatracker.ietf.org/doc/html/draft-sca-cfrg-sm3-02

Parameters
----------

address
    address from where the sm3 hash is calculated.
    Hexadecimal string, 0x prefix optional.

count
    length in bytes of memory area for which the sm3 hash is calculated
    Hexadecimal string, 0x prefix optional.

sum
    if it starts with ``*`` the string is interpreted as an address
    in hexadecimal format to which the calculated hash gets stored.

    else the string is interpreted as a name for an environment variable
    in which the calculated hash is stored as string.

    or if ``-v`` option is passed:

    address of hash with which the calculated hash gets compared.

Example
-------

create some data

::

    u-boot=> mw 0x100000000 0x426f6f46 1
    u-boot=> md.b 0x100000000 4
    00000000: 46 6f 6f 42                                      FooB

and calculate the sm3sum of 4 bytes starting from address ``0x100000000``
and store it in environment variable ``hashval``

::

    u-boot=> sm3sum 0x100000000 4 hashval
    sm3_256 for 100000000 ... 100000003 ==> cdf49da4e33017bf2d9fe87b885d80c9a7c920be7e10ffb8c89036a1eb1503b7
    u-boot=> print hashval
    hashval=cdf49da4e33017bf2d9fe87b885d80c9a7c920be7e10ffb8c89036a1eb1503b7
    u-boot=>

or calculate sm3sum of 4 bytes starting from address ``0x100000000`` and
store it at address ``0x110000000``

::

    u-boot=> sm3sum 0x100000000 4 *0x110000000
    sm3_256 for 100000000 ... 100000003 ==> cdf49da4e33017bf2d9fe87b885d80c9a7c920be7e10ffb8c89036a1eb1503b7

and now check if this hash is the expected sm3sum hash value with ``-v``
option

::

    u-boot=> sm3sum -v 0x100000000 4 *0x110000000
    u-boot=> echo $?
    0

example with wrong hash

::

    u-boot=> sm3sum -v 0x100000000 4 *0x110000004
    sm3_256 for 100000000 ... 100000003 ==> cdf49da4e33017bf2d9fe87b885d80c9a7c920be7e10ffb8c89036a1eb1503b7 != e33017bf2d9fe87b885d80c9a7c920be7e10ffb8c89036a1eb1503b7ffffffff ** ERROR **
    u-boot=>


Configuration
-------------

Enable the sm3sum command via Kconfig option ``CONFIG_CMD_SM3SUM``.
The ``-v`` option is separate enabled through Kconfig option
``CONFIG_SM3SUM_VERIFY``.
