.. SPDX-License-Identifier: GPL-2.0+:

cedit command
=============

Synopis
-------

::

    cedit load <interface> <dev[:part]> <filename>
    cedit run

Description
-----------

The *cedit* command is used to load a configuration-editor description and allow
the user to interact with it.

It makes use of the expo subsystem.

The description is in the form of a devicetree file, as documented at
:ref:`expo_format`.

Example
-------

::

    => cedit load hostfs - fred.dtb
    => cedit run
