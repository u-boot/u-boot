.. SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause
.. sectionauthor:: Apurva Nandan <a-nandan@ti.com>

J742S2, J784S4 and AM69 Platforms
=================================

Introduction
------------
The J784S4 SoC belongs to the K3 Multicore SoC architecture
platform, providing advanced system integration in automotive,
ADAS and industrial applications requiring AI at the network edge.
This SoC extends the K3 Jacinto 7 family of SoCs with focus on
raising performance and integration while providing interfaces,
memory architecture and compute performance for multi-sensor, high
concurrency applications.

The device is partitioned into three functional domains, each containing
specific processing cores and peripherals:

1. Wake-up (WKUP) domain
    * ARM Cortex-M4F processor, runs TI Foundational Security (TIFS)

2. Microcontroller (MCU) domain
    * Dual core ARM Cortex-R5F processor, runs device management
      and SoC early boot

3. MAIN domain
    * Two clusters of quad core 64-bit ARM Cortex-A72, runs HLOS
    * Dual core ARM Cortex-R5F processor used for RTOS applications
    * Four C7x DSPs used for Machine Learning applications.


More info can be found in TRM: http://www.ti.com/lit/zip/spruj52

Platform information:

* https://www.ti.com/tool/J784S4XEVM
* https://www.ti.com/tool/SK-AM69

J742S2 is derivative of J784S24 SOC, More info can be found in

* TRM : https://www.ti.com/lit/ug/spruje3/spruje3.pdf
* Platform Information : https://www.ti.com/tool/J742S2XH01EVM

Boot Flow
---------
Below is the pictorial representation of boot flow:

.. image:: img/boot_diagram_k3_current.svg
    :alt: K3 boot flow

- On this platform, "TI Foundational Security" (TIFS) functions as the
  security enclave master. While "Device Manager" (DM), also known as the
  "TISCI server" in TI terminology, offers all the essential services.

- As illustrated in the diagram above, R5 SPL manages power and clock
  services independently before handing over control to DM. The A72 or
  the C7x (Aux core) software components request TIFS/DM to handle
  security or device management services.

Sources
-------

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_boot_firmwares
    :end-before: .. k3_rst_include_end_boot_firmwares

Build procedure
---------------
0. Setup the environment variables:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. prompt:: bash

   export UBOOT_CFG_CORTEXR=j784s4_evm_r5_defconfig
   export UBOOT_CFG_CORTEXA=j784s4_evm_a72_defconfig
   export TFA_BOARD=j784s4
   export TFA_EXTRA_ARGS="K3_USART=0x8"
   export OPTEE_PLATFORM=k3-j784s4
   export OPTEE_EXTRA_ARGS="CFG_CONSOLE_UART=0x8"

.. note::

   For AM69-SK, use the following U_BOOT_CFG instead:

   .. prompt:: bash

      export UBOOT_CFG_CORTEXR=am69_sk_r5_defconfig
      export UBOOT_CFG_CORTEXA=am69_sk_a72_defconfig

   For J742S2-EVM, use the following U_BOOT_CFG instead:

   .. prompt:: bash

      export UBOOT_CFG_CORTEXR=j742s2_evm_r5_defconfig
      export UBOOT_CFG_CORTEXA=j742s2_evm_a72_defconfig

.. j784s4_evm_rst_include_start_build_steps

1. Trusted Firmware-A

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_tfa
    :end-before: .. k3_rst_include_end_build_steps_tfa


2. OP-TEE

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_optee
    :end-before: .. k3_rst_include_end_build_steps_optee

3. U-Boot

.. _j784s4_evm_rst_u_boot_r5:

* 3.1 R5

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

.. _j784s4_evm_rst_u_boot_a72:

* 3.2 A72

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot
.. j784s4_evm_rst_include_end_build_steps

Target Images
-------------
In order to boot we need tiboot3.bin, tispl.bin and u-boot.img. Each SoC
variant (GP, HS-FS, HS-SE) requires a different source for these files.

 - GP

    * tiboot3-j784s4-gp-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tispl.bin_unsigned, u-boot.img_unsigned from :ref:`step 3.2 <j784s4_evm_rst_u_boot_a72>`

  .. note::

    For J742S2, GP variant is not available.


 - HS-FS

    * tiboot3-j784s4-hs-fs-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tiboot3-j742s2-hs-fs-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tispl.bin, u-boot.img from :ref:`step 3.2 <j784s4_evm_rst_u_boot_a72>`

 - HS-SE

    * tiboot3-j784s4-hs-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tiboot3-j742s2-hs-evm.bin from :ref:`step 3.1 <j784s4_evm_rst_u_boot_r5>`
    * tispl.bin, u-boot.img from :ref:`step 3.2 <j784s4_evm_rst_u_boot_a72>`

Image formats
-------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg
    :alt: tiboot3.bin format

- tispl.bin

.. image:: img/dm_tispl.bin.svg
    :alt: tispl.bin format

OSPI:
-----
ROM supports booting from OSPI from offset 0x0.

Flashing images to OSPI NOR:

Below commands can be used to download tiboot3.bin, tispl.bin, and
u-boot.img over tftp and then flash those to OSPI at their respective
addresses.

.. prompt:: bash =>

  sf probe
  tftp ${loadaddr} tiboot3.bin
  sf update $loadaddr 0x0 $filesize
  tftp ${loadaddr} tispl.bin
  sf update $loadaddr 0x80000 $filesize
  tftp ${loadaddr} u-boot.img
  sf update $loadaddr 0x280000 $filesize

Flash layout for OSPI NOR:

.. image:: img/ospi_sysfw3.svg
  :alt: OSPI NOR flash partition layout

R5 Memory Map
-------------

.. list-table::
   :widths: 16 16 16
   :header-rows: 1

   * - Region
     - Start Address
     - End Address

   * - SPL
     - 0x41c00000
     - 0x41c40000

   * - EMPTY
     - 0x41c40000
     - 0x41c61f20

   * - STACK
     - 0x41c65f20
     - 0x41c61f20

   * - Global data
     - 0x41c65f20
     - 0x41c66000

   * - Heap
     - 0x41c66000
     - 0x41c76000

   * - BSS
     - 0x41c76000
     - 0x41c80000

   * - DM DATA
     - 0x41c80000
     - 0x41c84130

   * - EMPTY
     - 0x41c84130
     - 0x41cff9fc

   * - MCU Scratchpad
     - 0x41cff9fc
     - 0x41cffbfc

   * - ROM DATA
     - 0x41cffbfc
     - 0x41cfffff

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

Boot Mode Pins for J784S4-EVM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following tables show some common boot modes used on J784S4 EVM platform.
More details can be found in the Technical Reference Manual:
http://www.ti.com/lit/zip/spruj52 under the `Boot Mode Pins` section.

.. list-table:: J784S4 EVM Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW11: 12345678
     - SW7: 12345678

   * - SD
     - 10000010
     - 00000000

   * - EMMC
     - 10000000
     - 01000000

   * - OSPI
     - 00000110
     - 01000000

   * - UART
     - 00000000
     - 01110000

   * - PCIe
     - 10001000
     - 01010000

For SW7 and SW11, the switch state in the "ON" position = 1.

Boot Mode Pins for AM69-SK
^^^^^^^^^^^^^^^^^^^^^^^^^^

The following table show some common boot modes used on AM69-SK platform.
More details can be found in the User Guide for AM69-SK:
https://www.ti.com/lit/ug/spruj70/spruj70.pdf under the `Bootmode Settings`
section.

.. list-table:: AM69 SK Boot Modes
   :widths: 16 16
   :header-rows: 1

   * - Switch Label
     - SW2: 1234

   * - SD
     - 0000

   * - OSPI
     - 0010

   * - EMMC
     - 0110

   * - UART
     - 1010

For SW2, the switch state in the "ON" position = 1.

PCIe Boot
---------

The J784S4 SoC supports booting over PCIe, allowing the device to function
as a PCIe endpoint and receive boot loader images from a PCIe Root Complex.
The PCIe1 instance of PCIe is configured by Boot ROM for Endpoint Mode of
operation. Hence, the PCIe Connector on the EVM corresponding to PCIe1
should be utilized for PCIe Boot.

Hardware Setup
^^^^^^^^^^^^^^

To boot the J784S4 EVM via PCIe, the following hardware setup is required:

1. Configure the boot mode switches on J784S4-EVM for PCIe boot:

   .. code-block:: text

      SW7:  01010000
      SW11: 10001000

2. Connect the J784S4-EVM (endpoint) to a PCIe Root Complex (e.g., x86 host)
   using a PCIe cable. Both boards should be powered off before making the
   connection.

Endpoint Configuration
^^^^^^^^^^^^^^^^^^^^^^

The following configuration options are enabled by default in
``j784s4_evm_r5_defconfig`` and ``j784s4_evm_a72_defconfig``:

- ``CONFIG_SPL_PCI_DFU_BAR_SIZE``: Size of the PCIe BAR for DFU/boot image download
- ``CONFIG_SPL_PCI_DFU_VENDOR_ID``: PCIe vendor ID advertised by the endpoint
- ``CONFIG_SPL_PCI_DFU_DEVICE_ID``: PCIe device ID advertised by the endpoint
- ``CONFIG_SPL_PCI_DFU_MAGIC_WORD``: Magic word written by Root Complex to signal image transfer completion
- ``CONFIG_SPL_PCI_DFU_BOOT_PHASE``: Current boot phase indicator for Root Complex

By default, PCIe Root Complex mode is enabled in the device tree. For PCIe Boot,
build the Bootloaders with the following content added to k3-j784s4-evm-u-boot.dtsi:

.. code-block:: devicetree

   &serdes0 {
           /delete-property/ serdes0_usb_link;
   };

   &serdes_refclk {
           bootph-all;
   };

   &serdes0_pcie1_link {
           bootph-all;
   };

   &serdes_ln_ctrl {
           bootph-all;
   };

   &pcie1_ctrl {
           bootph-all;
   };

   &pcie1_rc {
           status = "disabled";
   };

   &cbass_main {
           pcie1_ep: pcie-ep@2910000 {
                   compatible = "ti,j784s4-pcie-ep";
                   reg = <0x00 0x02910000 0x00 0x1000>,
                         <0x00 0x02917000 0x00 0x400>,
                         <0x00 0x0d800000 0x00 0x00800000>,
                         <0x00 0x18000000 0x00 0x08000000>;
                   reg-names = "intd_cfg", "user_cfg", "reg", "mem";
                   interrupt-names = "link_state";
                   interrupts = <GIC_SPI 330 IRQ_TYPE_EDGE_RISING>;
                   ti,syscon-pcie-ctrl = <&pcie1_ctrl 0x0>;
                   max-link-speed = <3>;
                   num-lanes = <2>;
                   power-domains = <&k3_pds 333 TI_SCI_PD_EXCLUSIVE>;
                   clocks = <&k3_clks 333 0>;
                   clock-names = "fck";
                   max-functions = /bits/ 8 <6>;
                   max-virtual-functions = /bits/ 8 <4 4 4 4 0 0>;
                   dma-coherent;
                   phys = <&serdes0_pcie1_link>;
                   phy-names = "pcie-phy";
                   bootph-all;
           };
   };

PCIe Boot Procedure
^^^^^^^^^^^^^^^^^^^

The following steps describe the process of booting J784S4-EVM over PCIe:

1. Compile the sample host program (provided after this section):

   .. prompt:: bash

      gcc -o pcie_boot_util pcie_boot_util.c

2. Power on the J784S4-EVM (endpoint) after configuring boot mode switches
   for PCIe Boot.

3. Copy the compiled sample host program (pcie_boot_util) and the bootloader
   images to the Root Complex. Check PCIe enumeration on Root Complex to ensure
   that the J784S4 EVM shows up as the PCIe Endpoint:

   .. prompt:: bash

      lspci

   The endpoint will appear as a RAM device or with multiple functions:

   .. code-block:: text

      0000:00:00.0 PCI bridge: Texas Instruments Device b012
      0000:01:00.0 RAM memory: Texas Instruments Device b012
      0000:01:00.1 Non-VGA unclassified device: Texas Instruments Device 0100
      0000:01:00.2 Non-VGA unclassified device: Texas Instruments Device 0100

4. Copy ``tiboot3.bin`` to the endpoint. Use ``lspci -vv`` to identify the BAR
   address:

   .. prompt:: bash

      sudo ./pcie_boot_util 0x4007100000 tiboot3.bin

   The sample program automatically writes the image start address to
   ``0x41CF3FE0`` and the magic word ``0xB17CEAD9`` to ``0x41CF3FE4``.

5. After ``tiboot3.bin`` is processed, the PCIe link will go down briefly.
   Remove the PCIe device and rescan the bus:

   .. prompt:: bash

      echo 1 > /sys/bus/pci/devices/0000\:01\:00.0/remove
      echo 1 > /sys/bus/pci/devices/0000\:00\:00.0/rescan
      lspci

   The enumeration will change to something similar:

   .. code-block:: text

      0000:00:00.0 PCI bridge: Texas Instruments Device b012
      0000:01:00.0 RAM memory: Texas Instruments Device b010 (rev dc)

   .. note::

      When the Root-Complex enumerates the PCIe Endpoint after a 'remove-rescan' sequence,
      it is possible that the 'BAR' appears 'disabled'. If so, writing to the BAR via the
      'pcie_boot_util' to transfer the bootloader image will have no effect. In such cases,
      run 'setpci -s 0000:01:00.0 COMMAND=0x02' on the Root-Complex after enumeration
      (with appropriate DOMAIN:BUS:DEVICE.FUNCTION corresponding to the Endpoint) to enable
      the BAR.

6. Copy ``tispl.bin`` to the new BAR address (use ``lspci -vv`` to find):

   .. prompt:: bash

      sudo ./pcie_boot_util 0x4000400000 tispl.bin

7. After ``tispl.bin`` is processed, the PCIe link will go down again. Remove
   and rescan the PCIe device:

   .. prompt:: bash

      echo 1 > /sys/bus/pci/devices/0000\:01\:00.0/remove
      echo 1 > /sys/bus/pci/devices/0000\:00\:00.0/rescan

8. Copy ``u-boot.img``:

   .. prompt:: bash

      sudo ./pcie_boot_util 0x4000400000 u-boot.img

9. After ``u-boot.img`` is successfully loaded, the boot process is complete
   and endpoint should boot till U-Boot prompt.

.. note::

   During the boot process, "PCIe LINK DOWN" messages might appear in kernel
   logs. This is expected as the endpoint resets and re-initializes the PCIe
   link after processing each boot stage.

Sample Host Program
^^^^^^^^^^^^^^^^^^^

The following C program can be used on the Root Complex to copy bootloader images
to the J784S4 endpoint:

.. code-block:: c

   #include <stdio.h>
   #include <stdlib.h>
   #include <fcntl.h>
   #include <sys/mman.h>
   #include <unistd.h>
   #include <string.h>

   #define MAP_SIZE 0x400000

   /*
    * bootloader_file: Path to the bootloader image (tiboot3.bin, tispl.bin and u-boot.img)
    * bootloader_mem: Memory allocated in RAM for reading the bootloader image file
    * bar_address: Address of BAR to which bootloader image will be written
    * bar_map_base: Mapping of the BAR Base Address for the program
    * load_address: Address in BAR region where bootloader is being transferred
    * transfer_completion_offset: Offset in BAR region to write to notify completion of transfer
    * fd_mem: File descriptor for opening /dev/mem
    * fptr: File pointer for bootloader image in filesystem
    * magic_word: Magic word to notify completion of tiboot3.bin transfer to Boot ROM
    * use_magic_word: Flag to indicate if Magic Word has to be written
    * file_size: Size of bootloader image
    * i: Iterator used during bootloader image transfer
    */
   int main(int argc, char *argv[])
   {
      off_t bar_address, load_address, transfer_completion_offset;
      unsigned char *bootloader_mem;
      const char *bootloader_file;
      int fd_mem, i, use_magic_word;
      unsigned int magic_word;
      void *bar_map_base;
      long file_size;
      FILE * fptr;

      if (argc != 3) {
          printf("Usage: %s <bar_address> <bootloader_file>\n", argv[0]);
          return 0;
      }

      bar_address = strtoul(argv[1], NULL, 16);
      bootloader_file = argv[2];

      printf("Bootloader File: %s\n", bootloader_file);
      printf("BAR Address: 0x%lx\n", bar_address);

      if(!strcmp(bootloader_file,"tiboot3.bin")) {
          transfer_completion_offset = 0xF3FE0;
          load_address = 0x41C00000;
          magic_word = 0xB17CEAD9;
          use_magic_word = 1;
      } else {
          transfer_completion_offset = MAP_SIZE - 0x4;
          load_address = 0xDEADBEEF;
          use_magic_word = 0;
      }

      fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
      if(fd_mem == -1) {
          printf("failed to open /dev/mem\n");
          return -1;
      }

      bar_map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, bar_address);
      if(bar_map_base == (void *)-1) {
          printf("failed to map BAR\n");
          return -1;
      }

      fptr = fopen(bootloader_file, "rb");
      if (!fptr) {
          printf("failed to read bootloader file\n");
          return -1;
      }

      fseek(fptr, 0, SEEK_END);
      file_size = ftell(fptr);
      rewind(fptr);

      bootloader_mem = (unsigned char *)malloc(sizeof(char) * file_size);
      if(!bootloader_mem) {
         printf("failed to allocate local memory for bootloader file\n");
         return -1;
      }

      if (fread(bootloader_mem, 1, file_size, fptr) != file_size) {
          printf("failed to read bootloader file into local memory\n");
          return -1;
      }

      for(i = 0; i < file_size; i++) {
          *((char *)(bar_map_base) + i) = bootloader_mem[i];
      }

      *(unsigned int *)(bar_map_base + transfer_completion_offset) = (unsigned int)(load_address);

      if(use_magic_word) {
          *(unsigned int *)(bar_map_base + transfer_completion_offset + 4) = magic_word;
          printf("Magic word written for Boot ROM\n");
      }

      printf("Transferred %s to Endpoint\n", bootloader_file);
      return 0;
   }

This program copies the boot image to the PCIe endpoint's memory region and
writes the necessary control words to signal image transfer completion.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **OpenOCD support since**: September 2023 (git master)

  Until the next stable release of OpenOCD is available in your development
  environment's distribution, it might be necessary to build OpenOCD `from the
  source <https://github.com/openocd-org/openocd>`_.

Debugging U-Boot on J784S4-EVM and AM69-SK
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to J784S4-EVM or AM69-SK board, use the
following.

.. prompt:: bash

  openocd -f board/ti_j784s4evm.cfg
