.. SPDX-License-Identifier: GPL-2.0+:

font command
============

Synopis
-------

::

    font list
    font select <name> [<size>]
    font size <size>


Description
-----------

The *font* command allows selection of the font to use on the video console.
This is available when the Truetype console is in use. This is the case when
`CONFIG_CONSOLE_TRUETYPE` is enabled.


font list
~~~~~~~~~

This lists the available fonts, using the name of the font file in the build.


font select
~~~~~~~~~~~

This selects a new font and optionally changes the size.


font size
~~~~~~~~~

This changes the font size only.


Examples
--------

::

    => font list
    nimbus_sans_l_regular
    cantoraone_regular
    => font size 40
    => font select cantoraone_regular 20
    =>
