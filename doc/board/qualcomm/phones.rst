.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Casey Connolly <casey.connolly@linaro.org>

======================================
Booting U-Boot on Qualcomm smartphones
======================================

About this
----------

This page attempts to the describe U-Boot support for Qualcomm phones, as a user guide but also a
technical introduction to How Stuff Works to help new porters.

In broad strokes, U-Boot should boot if the SoC is supported, and the device is already capable of
booting an upstream Linux kernel.

The list of supported Qualcomm SoCs changes often, for now it is best to look in
``drivers/clk/qcom/`` to get a rough idea.

For building instructions, see :doc:`board`.

Phone bringup
-------------

It is usually easier to get Linux booting first, there are many good resources for this such as the
`postmarketOS wiki`_. Once the device can boot Linux with logs on the display and ideally USB gadget
support, it is highly likely that U-Boot will boot as well.

For logs on display, you should have a simple framebuffer node defined in your DT, newer devices
require that this follow the downstream naming scheme (that the DTB is compiled with labels enabled
and the framebuffer reserved-memory region is labelled ``cont_splash``). Once this is working in
Linux it should also work in U-Boot.

In practise, U-Boot still has many more papercuts than Linux, which can be sticking points when
porting a new device. In particular, drivers failing to bind/probe (especially pre-relocation) can
be tricky to debug without UART since U-Boot will simply panic with no way to inform you of
the error. As a result, bringing up a new device can be quite frustrating, but there are quite a few
things you can try.

The phone config
^^^^^^^^^^^^^^^^

Since most phones lack a physical keyboard or serial port, a special config fragment and environment
file can be used to provide a more seamless experience. This can be enabled by generating the config
with::

	make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig qcom-phone.config

The config and associated environment file can be found in board/qualcomm/. The main changes are:

- Panic on hang (so the panic message can be read on the display)
- Boot retry (to automatically open and re-open the bootmenu)
- A boot menu with helpful shortcuts (including USB console gadget)
- Launch the boot menu if power is held during boot or on boot failure

Fastboot mode
-------------

U-Boot's fastboot implementation is much more limited than Qualcomm's, and currently does not have a
backend for UFS storage. If your device uses eMMC or has an sdcard slot, fastboot will use that by
default.

You may need to run the fastboot command on your PC as root since the USB product/vendor ID may not
match the android udev rules.

You can also use fastboot to run arbitrary U-Boot commands with ``fastboot oem run``

Retrieving early logs
^^^^^^^^^^^^^^^^^^^^^

U-Boot is configured to save it's internal log to a buffer, this can help with debugging some driver
bind/probe issues. If your device can boot and has working USB, you can enable fastboot mode (either
via the U-Boot menu or by adding ``run fastboot`` to the end of the ``preboot=`` config in
``board/qualcomm/qcom-phone.env``).

You can then retrieve U-Boot's log buffer with the ``fastboot oem log`` command on your PC.

Hang/crash bisection
--------------------

Without a way to get logs, we can still get quite far with only a few bits of information: what
happens when you ``fastboot boot u-boot.img``?

Does the device disconnect?
^^^^^^^^^^^^^^^^^^^^^^^^^^^

This can be verified by watching ``dmesg -w``. If it stays connected, it likely means the boot image
doesn't match what the bootloader expected, use ``unpack_bootimg`` to compare it with a known-good
boot image (ideally one with an upstream kernel).

Does the device hang?
^^^^^^^^^^^^^^^^^^^^^

If it stays on a black screen and does nothing, then that's a hang! Since ``qcom-phone.config``
enables CONFIG_PANIC_HANG, this likely means that you're successfully executing U-Boot code (yay!),
but something is causing a panic.

It could also be due to a bad memory or register access triggering a secure interrupt, it's worth
waiting for around a minute to see if the device eventually reboots or goes to crashdump mode. You
can also disable CONFIG_PANIC_HANG and see if that causes the device to reboot instead, if so then
it is definitely a U-Boot panic.

With enough time and patience, it should be possible to narrow down the cause of the panic by
inserting calls to ``reset_cpu()`` (with CONFIG_PANIC_HANG enabled). Then if the device resets you
know it executed the ``reset_cpu()`` call.

A good place to start is ``board_fdt_blob_setup()`` in ``arch/arm/mach-snapdragon/board.c``, this
function is called extremely early so adding a reset call is a good way to validate that U-Boot is
definitely running.

You can then do a binary search starting from the end of ``board_init_f()`` / start of
``board_init_r()`` and work from there using the init sequences for reference.

The Qualcomm RAM parsing code is a likely culprit, as ABL is known to sometimes give bogus entries
in the memory node which can trip U-Boot up.

To rule out crashes that might be caused by specific drivers, it's a good idea to disable them and
re-enable them one by one. Here is a non-exhaustive list of drivers to disable:

- pinctrl
- mmc
- scsi/ufs
- usb (dwc3)
- phy (usb, ufs)
- clk (remove clock references from your framebuffer node in DT)

Ideally, it would be possible to use the framebuffer as an early console / debug output, at the time
of writing there are out of tree patches for this but they haven't been submitted upstream yet.

Does the device reboot or go to crashdump mode?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On many devices crashdump mode is disabled, so they will reboot instead (maybe after some delay).
The same approach as suggested above can be used to figure out where the crash occurs.

If the device is rebooting, you can insert calls to ``hang()`` instead of ``reset_cpu()`` when
following the instructions above.

The most likely cause of a crashdump is the pinctrl/gpio driver or the SMMU driver, ensure that the
``apps_smmu`` node in your SoCs devicetree file has one of its compatible strings referenced in
``drivers/iommu/qcom-hyp-smmu.c``, you can also try disabling the pinctrl driver for your SoC (or
``CONFIG_PINCTRL`` altogether).

.. _`postmarketOS wiki`: https://wiki.postmarketos.org/wiki/Mainlining
