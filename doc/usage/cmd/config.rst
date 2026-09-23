.. SPDX-License-Identifier: GPL-2.0

.. index::
   single: config (command)

config command
==============

Synopsis
--------

::

    config [<str>]

Description
-----------

The config command prints the `.config` file used when building U-Boot
to the console. Note that the `.config` file is usually several 1000
lines long.

If the optional argument is given, only lines containing that string
(case insensitively) are printed. That can be useful if one wants to
check whether a specific option is enabled, or just to limit the
output to the subsystem of interest.

The `.config` file is stored inside the U-Boot binary in
gzip-compressed format.

Examples
--------

.. code-block:: bash

    # Print the entire .config
    config

    # Print all lines related to pinctrl
    config pinctrl
