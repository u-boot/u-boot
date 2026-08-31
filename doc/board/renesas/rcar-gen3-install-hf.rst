.. SPDX-License-Identifier: GPL-2.0+

Renesas R-Car Gen3 U-Boot HyperFlash installation
=================================================

U-Boot can be installed on R-Car Gen3 systems into HyperFlash from U-Boot.

.. note::

    This update mechanism is only available in case the TFA has been built
    with `RCAR_RPC_HYPERFLASH_LOCKED=0`.

Install U-Boot into HyperFlash using NOR write from U-Boot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to install U-Boot using write into HyperFlash, first build U-Boot
for this target and collect ``u-boot.bin`` build artifact. Then start the
target, drop into U-Boot shell, and load the build artifact into DRAM at
well known address:

.. code-block:: console

    => tftp 0x50000000 u-boot.bin

Finally, write U-Boot into HyperFlash:

.. code-block:: console

    => erase 0x8640000 +${filesize} && cp.w 0x50000000 0x8640000 0x80000

.. note::

    The `cp.w` size parameter is in 16-bit short word units, not in Bytes.
    The `cp.w` size parameter size in Bytes would be 0x100000 .

Install U-Boot into HyperFlash using dfu_tftp update from U-Boot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to install U-Boot using dfu_tftp update, first build U-Boot for
this target and collect ``u-boot.bin`` build artifact. Then bundle this
``u-boot.bin`` into an update fitImage using the following fitImage source
file. The update fitImage source file is named ``update.its`` in this example:

.. code-block:: dts

    // update.its
    /dts-v1/;

    / {
        description = "Update fitImage for U-Boot";

        images {
            bootparam {
                description = "U-Boot";
                data = /incbin/("u-boot.bin");
                type = "standalone";
                os = "U-Boot";
                arch = "arm64";
                compression = "none";
                load = <0x8640000>;
            };
        };
    };

Generate the update fitImage using the following command:

.. code-block:: console

    $ mkimage -f update.its update.itb

Then start the target, drop into U-Boot shell, and load the ``update.itb``
artifact into DRAM at well known address:

.. code-block:: console

    => tftp 0x50000000 update.itb && dfu tftp 0x50000000
