.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: fuse (command)

fuse command
============

Synopsis
--------

::

    fuse read <bank> <word> [<cnt>]
    fuse cmp <bank> <word> <hexval>
    fuse readm <bank> <word> <addr> [<cnt>]
    fuse sense <bank> <word> [<cnt>]
    fuse prog [-y] <bank> <word> <hexval> [<hexval>...]
    fuse override <bank> <word> <hexval> [<hexval>...]
    fuse writebuff [-y] <addr>

Description
-----------

The fuse API allows to control a fusebox and how it is used by the upper
hardware layers.

A fuse corresponds to a single non-volatile memory bit that can be programmed
(i.e., blown, set to 1) only once. The programming operation is irreversible.
A fuse that has not been programmed reads as 0.

Fuses can be used by SoCs to store various permanent configurations and data,
such as boot configurations, security settings, MAC addresses, etc.

A fuse 'word' is the smallest group of fuses that can be read at once from
the fusebox control IP registers. In the current API, this is limited to 32 bits.

A fuse 'bank' is the smallest group of fuse words having a common ID, as
defined by each SoC.

Upon startup, the fusebox control IP reads the fuse values and stores them in a
volatile shadow cache.

Commands
--------

- **fuse read <bank> <word> [<cnt>]**
  Reads 1 or 'cnt' fuse words, starting at 'word' from the shadow cache.

- **fuse cmp <bank> <word> <hexval>**
  Compares 'hexval' to fuse at 'word'.

- **fuse readm <bank> <word> <addr> [<cnt>]**
  Reads 1 or 'cnt' fuse words, starting at 'word' into memory at 'addr'.

- **fuse sense <bank> <word> [<cnt>]**
  Sense 1 or 'cnt' fuse words, starting at 'word'.
  Sense - i.e. read directly from the fusebox, skipping the shadow cache -
  fuse words. This operation does not update the shadow cache. This is
  useful to know the true value of fuses if an override has been
  performed (see below).

- **fuse prog [-y] <bank> <word> <hexval> [<hexval>...]**
  Permanently programs 1 or several fuse words, starting at 'word'.
  This operation directly affects the fusebox and is irreversible. The
  shadow cache is updated accordingly or not, depending on each IP.
  Only the bits to be programmed should be set in the input value (i.e.
  for fuse bits that have already been programmed and hence should be
  left unchanged by a further programming, it is preferable to clear
  the corresponding bits in the input value in order not to perform a
  new hardware programming operation on these fuse bits).

- **fuse override <bank> <word> <hexval> [<hexval>...]**
  Override 1 or several fuse words, starting at 'word' in the shadow cache.
  The fusebox is unaffected, so following this operation, the shadow cache
  may differ from the fusebox values. Read or sense operations can then be
  used to get the values from the shadow cache or from the fusebox.
  This is useful to change the behaviours linked to some cached fuse values,
  either because this is needed only temporarily, or because some of the
  fuses have already been programmed or are locked (if the SoC allows to
  override a locked fuse).

- **fuse writebuff [-y] <addr>**
  Programs fuse data using a structured buffer in memory starting at 'addr'.
  This operation directly affects the fusebox and is irreversible.

  The structure of the buffer should contain all necessary details for
  programming fuses, such as the values to be written to the fuse, optional
  metadata for validation or programming constraints and any configuration
  data required for the operation. Define CONFIG_CMD_FUSE_WRITEBUFF to
  enable the fuse writebuff command.

Examples
--------

fuse read
~~~~~~~~~

::

    u-boot=> fuse read 0 1
      Reading bank 0:

      Word 0x00000001: 00000001

fuse cmp
~~~~~~~~

::

    u-boot=> fuse cmp 0 1 0x1
      Comparing bank 0:

      Word 0x00000001:
      Value 0x00000001:0x00000001
      passed

fuse readm
~~~~~~~~~~

::

    u-boot=> fuse readm 0 1 0x83000000
      Reading bank 0 len 1 to 0x83000000

fuse sense
~~~~~~~~~~

::

    u-boot=> fuse sense 0 1
      Sensing bank 0:

      Word 0x00000001: 00000001

fuse prog
~~~~~~~~~

::

    u-boot=> fuse prog 0 1 0x00000002
      Programming bank 0 word 0x00000001 to 0x00000002...
      Warning: Programming fuses is an irreversible operation!
               This may brick your system.
               Use this command only if you are sure of what you are doing!

      Really perform this fuse programming? <y/N>
      y

fuse override
~~~~~~~~~~~~~

::

    u-boot=> fuse override 0 1 0x00000003
      Overriding bank 0 word 0x00000001 with 0x00000003...

fuse writebuff
~~~~~~~~~~~~~~

::

    u-boot=> fuse writebuff -y 0x84000000
      Programming fuses with buffer at addr 0x84000000

Configuration
-------------

The fuse commands are available if CONFIG_CMD_FUSE=y.
The fuse writebuff command is available if CONFIG_CMD_FUSE_WRITEBUFF=y.

Return code
-----------

The return value $? is set to 0 (true) if the command is successful,
1 (false) otherwise.
