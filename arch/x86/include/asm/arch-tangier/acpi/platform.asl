/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Partially based on platform.asl for other x86 platforms
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/acpi/statdef.asl>

/*
 * The _PTS method (Prepare To Sleep) is called before the OS is
 * entering a sleep state. The sleep state number is passed in Arg0.
 */
Method(_PTS, 1)
{
}

/* The _WAK method is called on system wakeup */
Method(_WAK, 1)
{
    Return (Package() {0, 0})
}

/* ACPI global NVS */
#include "global_nvs.asl"

Scope (\_SB)
{
    #include "southcluster.asl"
}
