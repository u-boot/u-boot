.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Wadim Egorov <w.egorov@phytec.de>

Boot Flow
---------

The default `bootcmd` performs three steps:

.. code-block::

  run start_watchdog; bootflow scan -lb; run ${boot}boot

Boot devices are scanned in the order given by `boot_targets`:

.. code-block::

  mmc1 mmc0 spi_flash dhcp

For each device, U-Boot tries the boot methods listed in `bootmeths`:

.. code-block::

  [rauc] script efi extlinux pxe

The `rauc` bootmeth is only present when `CONFIG_BOOTMETH_RAUC=y` is set in
the A53 defconfig. RAUC slot selection is handled entirely by the bootmeth;
no environment-side configuration is required.

The legacy `${boot}boot` chain (`mmcboot`, `spiboot`, `netboot`) is kept for
backwards compatibility and prints a deprecation warning when run. New
deployments should rely on the standard boot mechanism (`bootflow`) only.


Watchdog
--------

`bootcmd` runs `start_watchdog` before starting the boot flow. When
`CONFIG_WATCHDOG_TIMEOUT_MSECS` is set to a non-zero value and the
`watchdog` environment variable points to a watchdog device, U-Boot enables
the watchdog with that timeout.

After this point the OS is responsible for servicing the watchdog. If it
does not feed the watchdog before the timeout expires, the SoC will reset.
Make sure the watchdog driver is enabled and configured in the kernel and
userspace before relying on this.

To skip the watchdog start, either build with `CONFIG_WATCHDOG_TIMEOUT_MSECS=0`
or set `watchdog_timeout_ms=0` in the environment.


Environment
-----------


Variables Set at Runtime
~~~~~~~~~~~~~~~~~~~~~~~~

At runtime the `boot` environment variable is set to reflect the source from which the board was booted. This ensures that the correct boot path is followed for further system initialization.


Environment Storage Selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The environment is loaded from a storage location based on the boot device:

* If booted from eMMC or uSD card, the environment is retrieved from FAT or a fixed offset if configured.

* If booted from SPI, the environment is retrieved from SPI flash if enabled.

For all other boot devices, the environment is not stored persistently (ENVL_NOWHERE).


Saving the Environment
~~~~~~~~~~~~~~~~~~~~~~

The `saveenv` command will store the environment on the same device the board was booted from, ensuring consistency between boot sources and stored configurations.


Capsule Updates
---------------

Capsules for each of these binaries are automatically generated as part of the build process and are named `<binary>-capsule.bin`. For example, the capsule for `u-boot.img` is named `uboot-capsule.bin`.



Performing an Update
~~~~~~~~~~~~~~~~~~~~

Each board has a dynamically generated GUID. To retrieve it, run:

.. code-block::

 efidebug capsule esrt

To update the firmware, follow these steps on the board. Ensure the capsule binaries are available on a uSD card.

.. code-block:: bash

 load mmc 1:1 $loadaddr tiboot3-capsule.bin
 efidebug capsule update $loadaddr

 load mmc 1:1 $loadaddr tispl-capsule.bin
 efidebug capsule update $loadaddr

 load mmc 1:1 $loadaddr uboot-capsule.bin
 efidebug capsule update $loadaddr

These commands load the capsule binaries into memory and trigger the EFI capsule update process.


Important Notes
~~~~~~~~~~~~~~~

The updates are applied to the boot device from which the board is currently running. For eMMC, updates are always applied to the first boot partition. Capsule updates can be performed on eMMC, OSPI NOR, or a uSD card, depending on the boot device. For any additional configuration or troubleshooting, refer to :ref:`uefi_capsule_update_ref`.
