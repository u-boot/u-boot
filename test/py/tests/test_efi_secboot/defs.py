# SPDX-License-Identifier:      GPL-2.0+

# Disk image name
EFI_SECBOOT_IMAGE_NAME='test_efi_secboot.img'

# Size in MiB
EFI_SECBOOT_IMAGE_SIZE=16
EFI_SECBOOT_PART_SIZE=8

# Partition file system type
EFI_SECBOOT_FS_TYPE='vfat'

# Owner guid
GUID='11111111-2222-3333-4444-123456789abc'

# v1.5.1 or earlier of efitools has a bug in sha256 calculation, and
# you need build a newer version on your own.
EFITOOLS_PATH=''

# Hello World application for sandbox
HELLO_PATH=''
