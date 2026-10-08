.. SPDX-License-Identifier: GPL-2.0+

Measured Boot
=============

U-Boot can perform a measured boot, the process of hashing various components
of the boot process, extending the results in the TPM and logging the
component's measurement in memory for the operating system to consume.

The functionality is available when booting via the EFI subsystem or 'bootm'
command.

UEFI measured boot
------------------

The EFI subsystem implements the `EFI TCG protocol
<https://trustedcomputinggroup.org/resource/tcg-efi-protocol-specification/>`_
and the `TCG PC Client Specific Platform Firmware Profile Specification
<https://trustedcomputinggroup.org/resource/pc-client-specific-platform-firmware-profile-specification/>`_
which defines the binaries to be measured and the corresponding PCRs to be used.

Requirements
~~~~~~~~~~~~

* A hardware TPM 2.0 supported by an enabled U-Boot driver
* CONFIG_EFI_TCG2_PROTOCOL=y
* optional CONFIG_EFI_TCG2_PROTOCOL_MEASURE_DTB=y will measure the loaded DTB
  in PCR 1

Legacy measured boot
--------------------

The commands booti, bootm, and bootz can be used for measured boot
using the legacy entry point of the Linux kernel.

By default, U-Boot will measure the operating system (linux) image, the
initrd image, and the "bootargs" environment variable. By enabling
CONFIG_MEASURE_DEVICETREE, U-Boot will also measure the devicetree image in PCR1.

The operating system typically would verify that the hashes found in the
TPM PCRs match the contents of the event log. This can further be checked
against the hash results of previous boots.

Requirements
~~~~~~~~~~~~

* A hardware TPM 2.0 supported by an enabled U-Boot driver
* CONFIG_TPMv2=y
* CONFIG_MEASURED_BOOT=y
* Device-tree configuration of the TPM device to specify the memory area
  for event logging. The TPM device node must either contain a phandle to
  a reserved memory region or "linux,sml-base" and "linux,sml-size"
  indicating the address and size of the memory region. An example can be
  found in arch/sandbox/dts/test.dts
* The operating system must also be configured to use the memory regions
  specified in the U-Boot device-tree in order to make use of the event
  log.

Starting the TPM without measured boot
--------------------------------------

Measured boot starts the TPM as a side effect, but only on the boot paths
listed above. A plain booti/bootm without CONFIG_MEASURED_BOOT, or a boot
which does not go through the EFI subsystem, leaves the chip uninitialised
and the operating system has to start it itself. Some kernels report this as
an error.

CONFIG_TPM_AUTOSTART runs the same startup and self-test sequence as the
'tpm autostart' command from an EVT_LAST_STAGE_INIT handler, independently of
the boot path, the environment and any boot script. When no TPM device is
found the startup is skipped, so a board which shares one binary between
variants with and without a TPM fitted can enable it safely.

Note that a full self-test can take a noticeable amount of time on a TPMv2,
which is added to every boot even when the TPM is not used afterwards.
