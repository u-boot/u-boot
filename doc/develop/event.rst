.. SPDX-License-Identifier: GPL-2.0+

Events
======

U-Boot supports a way for various events to be handled by interested
subsystems. This provide a generic way to handle 'hooks' like setting up the
CPUs after driver model is active, or reading a partition table after a new
block device is probed.

Rather than using weak functions and direct calls across subsystemss, it is
often easier to use an event.

An event consists of a type (e.g. EVT_DM_POST_INIT) and some optional data,
in `union event_data`. An event spy can be creasted to watch for events of a
particular type. When the event is created, it is sent to each spy in turn.


Declaring a spy
---------------

To declare a spy, use something like this::

    static int snow_setup_cpus(void *ctx, struct event *event)
    {
        /* do something */
        return 0;
    }
    EVENT_SPY(EVT_DM_POST_INIT, snow_setup_cpus);

Your function is called when EVT_DM_POST_INIT is emitted, i.e. after driver
model is inited (in SPL, or in U-Boot proper before and after relocation).


Debugging
---------

To assist with debugging events, enable `CONFIG_EVENT_DEBUG` and
`CONFIG_CMD_EVENT`. The :doc:`../usage/cmd/event` command can then be used to
provide a spy list.

It is also possible to list spy information from the U-Boot executable,, using
the `event_dump.py` script::

    $ scripts/event_dump.py /tmp/b/sandbox/u-boot
    Event type            Id                              Source location
    --------------------  ------------------------------  ------------------------------
    EVT_MISC_INIT_F       f:sandbox_misc_init_f           arch/sandbox/cpu/start.c:125

This shows each event spy in U-Boot, along with the event type, function name
(or ID) and source location.

Note that if `CONFIG_EVENT_DEBUG` is not enabled, the event ID is missing, so
the function is shown instead (with an `f:` prefix as above). Since the ID is
generally the same as the function name, this does not matter much.

The event type is decoded by the symbol used by U-Boot for the event linker
list. Symbols have the form::

    _u_boot_list_2_evspy_info_2_EVT_MISC_INIT_F

so the event type can be read from the end. To manually list spy information
in an image, use $(CROSS_COMPILE)nm::

    nm u-boot |grep evspy |grep list
    00000000002d6300 D _u_boot_list_2_evspy_info_2_EVT_MISC_INIT_F
