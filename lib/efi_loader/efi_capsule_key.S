/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * .esl cert for capsule authentication
 *
 * Copyright (c) 2021, Ilias Apalodimas <ilias.apalodimas@linaro.org>
 */

#include <config.h>

.section .rodata.capsule_key.init,"a"
.balign 16
.global __efi_capsule_sig_begin
__efi_capsule_sig_begin:
.incbin CONFIG_EFI_CAPSULE_KEY_PATH
__efi_capsule_sig_end:
.global __efi_capsule_sig_end
.balign 16
