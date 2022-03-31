.. SPDX-License-Identifier: GPL-2.0+

Pre-Generated FIP file set
==========================

The Amlogic ARMv8 based SoCs uses a vendor variant of the Trusted Firmware-A
boot architecture.

You can find documentation on the Trusted Firmware-A architecture on: https://www.trustedfirmware.org/projects/tf-a/

The Trusted Firmware-A uses the following boot elements (simplified):

- BL1: First boot step, implemented in ROM on Amlogic SoCs
- BL2: Second boot step, used to initialize the SoC main clocks & DDR interface. The BL21 and ACS board-specific binaries are "inserted" in the BL32 binary before signing/packaging in order to be flashed on the platform.
- BL30: Amlogic Secure Co-Processor (SCP) firmware used to handle all the system management operations (DVFS, suspend/resume, ...)
- BL301: Amlogic Secure Co-Processor (SCP) board-specific firmware "plug-in" to handle custom DVFS & suspend-resume parameters
- BL31: Initializes the interrupt controller and the system management interface (PSCI)
- BL32 (Optional): Is the Trusted Environment Execution (TEE) Operating System to run secure Trusted Apps, e.g. OP-TEE
- BL33: Is the last non-secure step, usually U-Boot which loads Linux

Amlogic provides in binary form:

- bl2.bin
- bl30.bin
- bl30.bin
- bl31.img
- bl32.bin

And for lastest SoCs, Amlogic also provides the DDR drivers used by the BL2 binary.

The licence of these files wasn't clear until recently, the currently Amlogic distribution licence
is the following:

.. code-block:: C

    // Copyright (C) 2018 Amlogic, Inc. All rights reserved.
    //
    // All information contained herein is Amlogic confidential.
    //
    // This software is provided to you pursuant to Software License
    // Agreement (SLA) with Amlogic Inc ("Amlogic"). This software may be
    // used only in accordance with the terms of this agreement.
    //
    // Redistribution and use in source and binary forms, with or without
    // modification is strictly prohibited without prior written permission
    // from Amlogic.
    //
    // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    // A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    // OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    // SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    // LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    // DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    // THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    // (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    // OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The following files are generated from the Amlogic U-Boot fork:

- acs.bin: contains the PLL & DDR parameters for the board
- bl301.bin: contains the DVFS & suspend-resume handling code for the board
- bl33.bin: U-boot binary image

The acs.bin & bl301.bin uses the U-Boot GPL-2.0+ headers & build systems, thus those
are considered issued from GPL-2.0+ source code.

The tools used to sign & package those binary files are delivered in binary format
for Intel x86-64 and Python 2.x only.

A collection of pre-built with the corresponding Amlogic binaries for the common
commercially available boards were collected in the https://github.com/LibreELEC/amlogic-boot-fip
repository.

Using this collection for a commercially available board is very easy.

Here considering the Libre Computer AML-S905X-CC, which codename is `lepotato`:

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh lepotato /path/to/u-boot/u-boot.bin my-output-dir

and then write the image to SD with:

.. code-block:: bash

    $ DEV=/dev/your_sd_device
    $ dd if=my-output-dir/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=my-output-dir/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=444
