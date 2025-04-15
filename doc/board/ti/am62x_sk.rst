.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Vignesh Raghavendra <vigneshr@ti.com>

AM62 Platforms
==============

Introduction:
-------------
The AM62 SoC family is the follow on AM335x built on the K3 Multicore
SoC architecture platform, providing ultra-low-power modes, dual
display, multi-sensor edge compute, security and other BOM-saving
integrations.  The AM62 SoC targets a broad market to enable
applications such as Industrial HMI, PLC/CNC/Robot control, Medical
Equipment, Building Automation, Appliances and more.

Some highlights of this SoC are:

* Quad-Cortex-A53s (running up to 1.4GHz) in a single cluster.
  Pin-to-pin compatible options for single and quad core are available.
* Cortex-M4F for general-purpose or safety usage.
* Dual display support, providing 24-bit RBG parallel interface and
  OLDI/LVDS-4 Lane x2, up to 200MHz pixel clock support for 2K display
  resolution.
* Selectable GPU support, up to 8GFLOPS, providing better user experience
  in 3D graphic display case and Android.
* PRU(Programmable Realtime Unit) support for customized programmable
  interfaces/IOs.
* Integrated Giga-bit Ethernet switch supporting up to a total of two
  external ports (TSN capable).
* 9xUARTs, 5xSPI, 6xI2C, 2xUSB2, 3xCAN-FD, 3x eMMC and SD, GPMC for
  NAND/FPGA connection, OSPI memory controller, 3xMcASP for audio,
  1x CSI-RX-4L for Camera, eCAP/eQEP, ePWM, among other peripherals.
* Dedicated Centralized System Controller for Security, Power, and
  Resource Management.
* Multiple low power modes support, ex: Deep sleep, Standby, MCU-only,
  enabling battery powered system design.

More details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/spruiv7

Platform information:

* https://www.ti.com/tool/SK-AM62B

Boot Flow:
----------
Below is the pictorial representation of boot flow:

.. image:: img/boot_diagram_am62.svg
  :alt: Boot flow diagram

- Here TIFS acts as master and provides all the critical services. R5/A53
  requests TIFS to get these services done as shown in the above diagram.

Sources:
--------

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_firmwares
    :end-before: .. k3_rst_include_end_tifsstub

Build procedure:
----------------
0. Setup the environment variables:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. prompt:: bash $

  export UBOOT_CFG_CORTEXR=am62x_evm_r5_defconfig
  export UBOOT_CFG_CORTEXA=am62x_evm_a53_defconfig
  export TFA_BOARD=lite
  # we dont use any extra TFA parameters
  unset TFA_EXTRA_ARGS
  export OPTEE_PLATFORM=k3-am62x
  export OPTEE_EXTRA_ARGS="CFG_WITH_SOFTWARE_PRNG=y"

.. am62x_evm_rst_include_start_build_steps

1. Trusted Firmware-A:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_tfa
    :end-before: .. k3_rst_include_end_build_steps_tfa


2. OP-TEE:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_optee
    :end-before: .. k3_rst_include_end_build_steps_optee

3. U-Boot:

* 3.1 R5:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

* 3.1.1 Alternative build of R5 for DFU boot:

As the SPL size can get too big when building with support for booting both
from local storage *and* DFU an extra config fragment should be used to enable
DFU support (and disable storage support)

.. prompt:: bash $

  export UBOOT_CFG_CORTEXR="${UBOOT_CFG_CORTEXR} am62x_r5_usbdfu.config"

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

* 3.2 A53:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot

* 3.2.1 Alternative build of A53 for Android bootflow:

Since the Android requires many more dependencies, it is disabled by default.
An extra config fragment should be used to enable Android bootflow support.

.. prompt:: bash $

  export UBOOT_CFG_CORTEXR="${UBOOT_CFG_CORTEXA} am62x_a53_android.config"

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot

.. am62x_evm_rst_include_end_build_steps

Target Images
-------------

In order to boot we need tiboot3.bin, tispl.bin and u-boot.img.  Each SoC
variant (GP, HS-FS, HS-SE) requires a different source for these files.

 - GP

        * tiboot3-am62x-gp-evm.bin from step 3.1
        * tispl.bin_unsigned, u-boot.img_unsigned from step 3.2

 - HS-FS

        * tiboot3-am62x-hs-fs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

 - HS-SE

        * tiboot3-am62x-hs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

Image formats:
--------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg
  :alt: tiboot3.bin image format

- tispl.bin

.. image:: img/tifsstub_dm_tispl.bin.svg
  :alt: tispl.bin image format

OSPI:
-----
ROM supports booting from OSPI from offset 0x0.

Flashing images to OSPI:

Below commands can be used to download tiboot3.bin, tispl.bin, and u-boot.img,
over tftp and then flash those to OSPI at their respective addresses.

.. prompt:: bash =>

  sf probe
  tftp ${loadaddr} tiboot3.bin
  sf update $loadaddr 0x0 $filesize
  tftp ${loadaddr} tispl.bin
  sf update $loadaddr 0x80000 $filesize
  tftp ${loadaddr} u-boot.img
  sf update $loadaddr 0x280000 $filesize

Flash layout for OSPI:

.. image:: img/ospi_sysfw2.svg
  :alt: OSPI flash partition layout

A53 SPL DDR Memory Layout
-------------------------

.. am62x_evm_rst_include_start_ddr_mem_layout

This provides an overview memory usage in A53 SPL stage.

.. list-table::
   :widths: 16 16 16
   :header-rows: 1

   * - Region
     - Start Address
     - End Address

   * - EMPTY
     - 0x80000000
     - 0x80080000

   * - TEXT BASE
     - 0x80080000
     - 0x800d8000

   * - EMPTY
     - 0x800d8000
     - 0x80200000

   * - BMP IMAGE
     - 0x80200000
     - 0x80b77660

   * - STACK
     - 0x80b77660
     - 0x80b77e60

   * - GD
     - 0x80b77e60
     - 0x80b78000

   * - MALLOC
     - 0x80b78000
     - 0x80b80000

   * - EMPTY
     - 0x80b80000
     - 0x80c80000

   * - BSS
     - 0x80c80000
     - 0x80d00000

   * - BLOBS
     - 0x80d00000
     - 0x80d00400

   * - EMPTY
     - 0x80d00400
     - 0x81000000
.. am62x_evm_rst_include_end_ddr_mem_layout

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

The following table shows some common boot modes used on AM62 platform. More
details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/spruiv7 under the `Boot Mode Pins` section.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW2: 12345678
     - SW3: 12345678

   * - SD
     - 01000000
     - 11000010

   * - OSPI
     - 00000000
     - 11001110

   * - EMMC
     - 00000000
     - 11010010

   * - UART
     - 00000000
     - 11011100

   * - USB DFU
     - 00000000
     - 11001010

   * - Ethernet
     - 00110000
     - 11000100

For SW2 and SW1, the switch state in the "ON" position = 1.

DFU based boot
--------------

To boot the board over DFU, set the switches to DFU mode and connect to the
USB type C DRD port on the board. After power-on the build artifacts needs to be
uploaded one by one with a tool like dfu-util.

.. am62x_evm_rst_include_start_dfu_boot

The initial ROM will have a DFU alt named `bootloader` for the initial R5 spl
upload. The next stages as exposed by U-Boot have target alts matching the name
of the artifacts, for these a USB reset has to be done after each upload.

When using dfu-util the following commands can be used to boot to a U-Boot shell:

.. prompt:: bash $

  dfu-util -a bootloader -D tiboot3.bin
  dfu-util -R -a tispl -D tispl.bin
  dfu-util -R -a u-boot.img -D u-boot.img

.. am62x_evm_rst_include_end_dfu_boot

Ethernet based boot
-------------------

To boot the board via Ethernet, configure the BOOT MODE pins for Ethernet boot.

On powering on the device, ROM uses the Ethernet Port corresponding to CPSW3G's MAC
Port 1 to transmit "TI K3 Bootp Boot".

The TFTP server and DHCP server on the receiver device need to be configured such
that VCI string "TI K3 Bootp Boot" maps to the file `tiboot3.bin` and the TFTP
server should be capable of transferring it to the device.

**Configuring DHCP server includes following steps:**

* Install DHCP server:

.. prompt:: bash $

  sudo apt install isc-dhcp-server

* Disable services before configuring:

.. prompt:: bash $

  sudo systemctl disable --now isc-dhcp-server.service isc-dhcp-server6.service

* DHCP server setup

Run the ip link or ifconfig command to find the name of your network interface:

Example

.. code-block::

  eno1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 172.24.145.229  netmask 255.255.254.0  broadcast 172.24.145.255
        inet6 fe80::bbd5:34c8:3d4c:5de4  prefixlen 64  scopeid 0x20<link>
        ether c0:18:03:bd:b1:a6  txqueuelen 1000  (Ethernet)
        RX packets 2733979  bytes 1904440459 (1.9 GB)
        RX errors 0  dropped 3850  overruns 0  frame 0
        TX packets 796807  bytes 84534764 (84.5 MB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
        device interrupt 16  memory 0xe2200000-e2220000

  enxf8e43b8cffe8: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
          ether f8:e4:3b:8c:ff:e8  txqueuelen 1000  (Ethernet)
          RX packets 95  bytes 31160 (31.1 KB)
          RX errors 0  dropped 0  overruns 0  frame 0
          TX packets 89  bytes 17445 (17.4 KB)
          TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

  lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
          inet 127.0.0.1  netmask 255.0.0.0
          inet6 ::1  prefixlen 128  scopeid 0x10<host>
          loop  txqueuelen 1000  (Local Loopback)
          RX packets 85238  bytes 7244462 (7.2 MB)
          RX errors 0  dropped 0  overruns 0  frame 0
          TX packets 85238  bytes 7244462 (7.2 MB)
          TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

Suppose we are using enxf8e43b8cffe8 interface, one end of it is connected to host PC
and other to board.

* Do the following changes in /etc/dhcp/dhcpd.conf in host PC.

.. code-block::

  subnet 192.168.0.0 netmask 255.255.254.0
  {
  range dynamic-bootp 192.168.0.2 192.168.0.5;
  if substring (option vendor-class-identifier, 0, 16) = "TI K3 Bootp Boot"
  {
  filename "tiboot3.bin";
  } elsif substring (option vendor-class-identifier, 0, 20) = "AM62X U-Boot R5 SPL"
  {
  filename "tispl.bin";
  } elsif substring (option vendor-class-identifier, 0, 21) = "AM62X U-Boot A53 SPL"
  {
  filename "u-boot.img";
  }
  default-lease-time 60000;
  max-lease-time 720000;
  next-server 192.168.0.1;
  }

* Do following changes in /etc/default/isc-dhcp-server

.. code-block::

  DHCPDv4_CONF=/etc/dhcp/dhcpd.conf
  INTERFACESv4="enxf8e43b8cffe8"
  INTERFACESv6=""

* For your interface change ip address and netmask to next-server and your netmask

.. prompt:: bash $

  sudo ifconfig enxf8e43b8cffe8 192.168.0.1 netmask 255.255.254.0

* Enable DHCP

.. prompt:: bash $

  sudo systemctl enable --now isc-dhcp-server

* To see if there is any configuration error or if dhcp is running run

.. prompt:: bash $

  sudo service isc-dhcp-server status
  # If it shows error then something is wrong with configuration

**For TFTP setup follow below steps:**

* Install TFTP server:

.. prompt:: bash $

  sudo apt install tftpd-hpa

tftpd-hpa package should be installed.

Now, check whether the tftpd-hpa service is running with the following command:

.. prompt:: bash $

  sudo systemctl status tftpd-hpa

* Configuring TFTP server:

The default configuration file of tftpd-hpa server is /etc/default/tftpd-hpa.
If you want to configure the TFTP server, then you have to modify this configuration
file and restart the tftpd-hpa service afterword.

To modify the /etc/default/tftpd-hpa configuration file, run the following command

.. prompt:: bash $

  sudo vim /etc/default/tftpd-hpa

Configuration file may contain following configuration options by default:

.. code-block::

  # /etc/default/tftpd-hpa

  TFTP_USERNAME="tftp"
  TFTP_DIRECTORY="/var/lib/tftpboot"
  TFTP_ADDRESS=":69"
  TFTP_OPTIONS="--secure"

Now change the **TFTP_DIRECTORY** to **/tftp** and add the **--create** option to the
**TFTP_OPTIONS**. Without the **--create** option, you won't be able to create or upload
new files to the TFTP server. You will only be able to update existing files.

After above changes /etc/default/tftpd-hpa file would look like this:

.. code-block::

  # /etc/default/tftpd-hpa

  TFTP_USERNAME="tftp"
  TFTP_DIRECTORY="/tftp"
  TFTP_ADDRESS=":69"
  TFTP_OPTIONS="--secure --create"

Since we have configured tftp directory as /tftp, put tiboot3.bin, tispl.bin
and u-boot.img after building it using sdk or manually cloning all the repos.

To build binaries use following defconfig files:

.. code-block::

  am62x_evm_r5_ethboot_defconfig
  am62x_evm_a53_ethboot_defconfig

`tiboot3.bin` is expected to be built from `am62x_evm_r5_ethboot_defconfig` and
`tispl.bin` and `u-boot.img` are expected to be built from
`am62x_evm_a53_ethboot_defconfig`.

Images should get fetched in following sequence as a part of boot procedure:

.. code-block::

  tiboot3.bin => tispl.bin => u-boot.img

ROM loads and executes `tiboot3.bin` provided by the TFTP server.

Next, based on NET_VCI_STRING string mentioned in respective defconfig file `tiboot3.bin`
fetches `tispl.bin` and then `tispl.bin` fetches `u-boot.img` from TFTP server which
completes Ethernet boot on the device.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **OpenOCD support since**: v0.12.0

  If the default package version of OpenOCD in your development
  environment's distribution needs to be updated, it might be necessary to
  build OpenOCD from the source.

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to the board

.. prompt:: bash $

  openocd -f board/ti_am625evm.cfg
