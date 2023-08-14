.. SPDX-License-Identifier: GPL-2.0+:

cedit command
=============

Synopis
-------

::

    cedit load <interface> <dev[:part]> <filename>
    cedit run
    cedit write_fdt <dev[:part]> <filename>
    cedit read_fdt <dev[:part]> <filename>
    cedit write_env [-v]

Description
-----------

The *cedit* command is used to load a configuration-editor description and allow
the user to interact with it.

It makes use of the expo subsystem.

The description is in the form of a devicetree file, as documented at
:ref:`expo_format`.

See :doc:`../../develop/cedit` for information about the configuration editor.

cedit load
~~~~~~~~~~

Loads a configuration-editor description from a file. It creates a new cedit
structure ready for use. Initially no settings are read, so default values are
used for each object.

cedit run
~~~~~~~~~

Runs the default configuration-editor event loop. This is very simple, just
accepting character input and moving through the objects under user control.
The implementation is at `cedit_run()`.

cedit write_fdt
~~~~~~~~~~~~~~~

Writes the current user settings to a devicetree file. For each menu item the
selected ID and its text string are written.

cedit read_fdt
~~~~~~~~~~~~~~

Reads the user settings from a devicetree file and updates the cedit with those
settings.

cedit write_env
~~~~~~~~~~~~~~~

Writes the settings to environment variables. For each menu item the selected
ID and its text string are written, similar to:

   setenv c.<name> <selected_id>
   setenv c.<name>-str <selected_id's text string>

The `-v` flag enables verbose mode, where each variable is printed before it is
set.


Example
-------

::

    => cedit load hostfs - fred.dtb
    => cedit run
    => cedit write_fdt hostfs - settings.dtb

That results in::

    / {
        cedit-values {
            cpu-speed = <0x00000006>;
            cpu-speed-str = "2 GHz";
            power-loss = <0x0000000a>;
            power-loss-str = "Always Off";
        };
    }

    => cedit read_fdt hostfs - settings.dtb

This shows settings being stored in the environment::

    => cedit write_env -v
    => print
    ...
    c.cpu-speed=6
    c.cpu-speed-str=2 GHz
    c.power-loss=10
    c.power-loss-str=Always Off
    ...
