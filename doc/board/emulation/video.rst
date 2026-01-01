.. SPDX-License-Identifier: GPL-2.0-or-later

Emulation of a graphics adapter
===============================

Currently QEMU's ``bochs-display`` is the only emulated GPU that U-Boot's
generic QEMU defconfigs support. It emulates a PCIe-connected graphics card
with VESA 2.0 VBE extensions.

The following parameters can be specified:

edid
    controls whether EDID information is available. Allowable values are
    ``off`` and ``on``. The default is ``on``.

vgamem
    specifies the display memory size. The default value is 16 MiB.
    For UHD resolution 32 MiB is required.

The following configuration parameters are relevant:

CONFIG_VIDEO_BOCHS
    enable support for the Bochs GPU.

CONFIG_VIDEO_BOCHS_SIZE_X
    defines the display width.

CONFIG_VIDEO_BOCHS_SIZE_Y
    defines the display height.

CONFIG_VIDEO_PCI_DEFAULT_FB_SIZE
    sets the framebuffer size that QEMU can use for a device driver that is
    enabled after relocation. This should be chosen as

    .. code-block::

        CONFIG_VIDEO_PCI_DEFAULT_FB_SIZE >=
        4 * CONFIG_VIDEO_BOCHS_SIZE_X * CONFIG_VIDEO_BOCHS_SIZE_Y

Here is an example QEMU invocation for qemu-riscv64_smode_defconfig:

.. code-block:: bash

    qemu-system-riscv64 \
      -M virt \
      -device bochs-display,vgamem=33554432 \
      -serial mon:stdio \
      -device qemu-xhci \
      -device usb-kbd \
      -kernel u-boot

``mon:stdio`` provides a serial console, ``qemu-xhci`` a USB root hub,
``usb-kbd`` a USB keyboard.

To start QEMU without GPU emulation, use parameter ``-nographic``, as shown in
the following example:

.. code-block:: bash

    qemu-system-riscv64 \
      -M virt \
      -nographic \
      -kernel u-boot
