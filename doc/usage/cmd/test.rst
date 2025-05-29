.. SPDX-License-Identifier: GPL-2.0-or-later

.. index::
   single: test (command)

test command
============

Synopsis
--------

::

    test <str-op> <s>
    test <s1> <str-cmp> <s2>
    test <n1> <num-cmp> <n2>
    test ! <expr>
    test <expr1> -o <expr2>
    test <expr1> -a <expr2>
    test -e <interface> <dev[:part]> <path>
    test <s> =~ <re>

Description
-----------

The ``test`` command is similar to the ordinary shell built-in by the
same name. Unlike in ordinary shells, it cannot be spelled ``[``.

Strings
~~~~~~~

The string tests ``-n`` and ``-z``, and string comparison operators
``=``, ``!=``, ``<`` and ``>``, work exactly as in ordinary shells.

Numbers
~~~~~~~

The number comparison operators ``-lt``, ``-le``, ``-gt``, ``-gt``,
``-eq`` and ``-ne`` work as in ordinary shells.

.. note::
  Numbers are parsed with ``simple_strtol(, 0)``, meaning that they
  are treated as decimal unless there is a `0x` prefix, any errors in
  parsing are ignored, and parsing stops as soon as a non-digit (for
  the selected base) is encountered. And most U-Boot commands that
  generate "numeric" environment variables store them as hexadecimal
  *without* a `0x` prefix.

For example, this is not a correct way of testing whether a given file
has a size less than 4KiB::

  # Assuming readme.txt exists, sets 'filesize' environment variable
  $ size mmc 0:1 readme.txt
  $ if test "$filesize" -lt 4096 ; then ...

If the file size is actually 8000 (decimal), its hexadecimal
representation, and thus the value of ``$filesize``, is ``1f40``, so
the comparison that is done ends up being "1 < 4096".

Logic
~~~~~

The ``!`` operator negates the sense of the test of the expression
``<expr>``.

The ``-o`` and ``-a`` operators perform logical OR and logical AND,
respectively, of the two expressions.

File existence
~~~~~~~~~~~~~~

Like ordinary shells, the ``-e`` operator can be used to test for
existence of a file. However, the U-Boot version takes three
arguments:

- The interface (e.g. ``mmc``).
- The device number, possibly including a partition specification.
- The usual path argument, which is interpreted relative to the root
  of the filesystem.

Regular expressions
~~~~~~~~~~~~~~~~~~~

When ``CONFIG_REGEX`` is enabled, an additional operator ``=~`` is
available. This is similar to the same operator available with bash's
extended test command ``[[ ]]``. The left operand is a string which is
matched against the regular expression described by the right operand.

The regular expression engine supports these features:

- Anchoring ``^`` and ``$``, matching at the beginning/end of the
  string.
- Matching any single character (including whitespace) using ``.``.
- Character classes ``[ ]``, including ranges ``[0-9]`` and negation
  ``[^ /.]``.
- Grouping ``( )``.
- Alternation ``|``.
- Postfix qualifiers ``*``, ``+`` and ``?`` and their non-greedy
  variants ``*?``, ``+?`` and ``??``

For extracting the parts matching a capture group and/or performing
substitutions, including back references, see :doc:`setexpr`.
