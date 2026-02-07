.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: memsize (command)

memsize command
===============

Synopsis
--------

::

    memsize [name]

Description
-----------

The memsize command shows the amount of RAM in MiB in decimal notation.
Optionally same value can be assigned to an environment variable.

Examples
--------

This first example shows printing of ram size:

::

  => memsize
  8192 MiB

This second example shows assign ram size to environment variable:

::

  => memsize memsz
  => printenv memsz
  memsz=8192

Return value
------------

The return value is always 0 except error happens on setting environment variable.
