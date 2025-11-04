.. SPDX-License-Identifier: GPL-2.0+
   Copyright 2025 NXP
   Written by Peng Fan <peng.fan@nxp.com>

i.MX remoteproc usage guide
===========================

Introduction
------------

This guide is for giving user how to use the Remote Processors found on
various i.MX Chips. The term remote processor is indicating the Cortex-M
[4,7,33] cores inside i.MX family.

i.MX8MM-EVK
-----------

Steps to start the Cortex-M4 core

    load mmc 2:2 0x90000000 /lib/firmware/imx8mm_m4_TCM_rpmsg_lite_str_echo_rtos.elf

    rproc load 0 0x90000000 ${filesize}

    rproc start 0

i.MX8MN-EVK
-----------

Steps to start the Cortex-M7 core

    load mmc 2:2 0x90000000 /lib/firmware/imx8mn_m7_TCM_rpmsg_lite_str_echo_rtos.elf

    rproc load 0 0x90000000 ${filesize}

    rproc start 0

i.MX8MQ-EVK
-----------

Steps to start the Cortex-M4 core

    load mmc 0:2 0x90000000 /lib/firmware/imx8mq_m4_TCM_rpmsg_lite_str_echo_rtos.elf

    rproc load 0 0x90000000 ${filesize}

    rproc start 0

i.MX8MP-EVK
-----------

Steps to start the Cortex-M7 core

    load mmc 2:2 0x90000000 /lib/firmware/imx8mp_m7_TCM_rpmsg_lite_str_echo_rtos.elf

    rproc load 0 0x90000000 ${filesize}

    rproc start 0

i.MX93-FRDM/QSB/EVK
-------------------

Steps to start the Cortex-M33 core, need to choose the correct file for
corresponding board.

    load mmc 0:2 0x90000000 /lib/firmware/imx93-11x11-evk_m33_TCM_rpmsg_lite_str_echo_rtos.elf

    rproc load 0 0x90000000 ${filesize}

    rproc start 0
