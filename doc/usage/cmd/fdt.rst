.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: fdt (command)

fdt command
===========

Synopsis
--------

::

    fdt addr [-cq] [addr [len]]
    fdt fit <addr>[#<conf>] <dest> [<len>]

Description
-----------

The fdt command provides access to flat device tree blobs in memory. It has
many subcommands, some of which are not documented here.

Flags:

-c
    Select the control FDT (otherwise the working FDT is used).
-q
    Don't display errors

The control FDT is the one used by U-Boot itself to control various features,
including driver model. This should only be changed if you really know what you
are doing, since once U-Boot starts it maintains pointers into the FDT from the
various driver model data structures.

The working FDT is the one passed to the Operating System when booting. This
can be freely modified, so far as U-Boot is concerned, since it does not affect
U-Boot's operation.

fdt addr
~~~~~~~~

With no arguments, this shows the address of the current working or control
FDT.

If the `addr` argument is provided, then this sets the address of the working or
control FDT to the provided address.

If the `len` argument is provided, then the device tree is expanded to that
size. This can be used to make space for more nodes and properties. It is
assumed that there is enough space in memory for this expansion.

fdt fit
~~~~~~~

This loads the device tree of a FIT configuration and copies it to `dest`. The
FIT must already be in memory at `addr`.

The configuration `conf` is selected as for the bootm command. If it is
omitted, the configuration matching the control FDT is used when
CONFIG_FIT_BEST_MATCH is enabled, otherwise the default configuration of the
FIT. The `fdt` property of the configuration lists a base device tree followed
by any overlays: the base device tree is loaded and the overlays are applied to
it in order.

Each image is checked against its hashes, whatever the value of the `verify`
environment variable. With CONFIG_FIT_SIGNATURE, the configuration must also
satisfy the signature policy of the control FDT, so an unsigned configuration
is rejected when a key has `required = "conf"`. A configuration signature
covers the hashes of all the images listed by the configuration, so it
authenticates the base device tree, the overlays and their order as one unit.

Unlike bootm, the command does not accept further configurations appended to
`conf` with `#`. Each of them would be verified on its own, so a chain could
combine device trees in a way which no configuration lists and nobody signed.
List each valid combination of a base device tree and overlays in its own
configuration instead.

The command fails if any image cannot be loaded, verified or applied, and
nothing is written to `dest` in that case. Unlike bootm, the command ignores
FIT image `load` addresses: only the completed device tree is copied to
`dest`, after checking that the destination does not overlap reserved memory
when CONFIG_LMB is enabled. Alignment and decompression of the base device
tree are still handled as needed.

If the `len` argument is provided, then the device tree is expanded to that
size, leaving space for later changes. The command fails if `len` is smaller
than the device tree. Without `len`, the device tree is copied at its own size.
The working FDT is not changed; use `fdt addr` to select the result.

This subcommand requires CONFIG_CMD_FDT_FIT.

Example
-------

Get the control address and copy that FDT to free memory::

    => fdt addr -c
    Control fdt: 0aff9fd0
    => cp.b 0aff9fd0 10000 10000
    => md 10000 4
    00010000: edfe0dd0 5b3d0000 78000000 7c270000  ......=[...x..'|

The second word shows the size of the FDT. Now set the working FDT to that
address and expand it to 0xf000 in size::

    => fdt addr 10000 f000
    Working FDT set to 10000
    => md 10000 4
    00010000: edfe0dd0 00f00000 78000000 7c270000  ...........x..'|

Load a FIT holding a device tree and its overlays, then copy the device tree of
the `conf-a` configuration, a base device tree with one overlay, to 2000000 and
inspect it::

    => load mmc 0#firmware 1000000 fdt.itb
    1771 bytes read in 1 ms (1.7 MiB/s)
    => fdt fit 1000000#conf-a 2000000
    ## Loading fdt (any) from FIT Image at 01000000 ...
       Using 'conf-a' configuration
       Verifying Hash Integrity ... OK
       Trying 'base' fdt subimage
       ...
       Verifying Hash Integrity ... sha256+ OK
    ## Loading fdt (any) from FIT Image at 01000000 ...
       Trying 'ov-a' fdt subimage
       ...
       Verifying Hash Integrity ... sha256+ OK
    => fdt addr 2000000
    Working FDT set to 2000000
    => fdt print /test-node
    test-node {
            ov-a-prop = "ov-a-value";
            base-prop = "base-value";
            order = "ov-a";
    };

A boot script can prepare a device tree kept with the firmware in this way and
pass it to the boot command, stopping if any step fails::

    if load mmc 0#firmware ${loadaddr} fdt.itb &&
       fdt fit ${loadaddr}#conf-a ${fdt_addr_r}; then
            bootefi bootmgr ${fdt_addr_r}
    fi
