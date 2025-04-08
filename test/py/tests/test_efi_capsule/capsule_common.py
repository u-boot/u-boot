# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2023, Linaro Limited


"""Common function for UEFI capsule test."""

from capsule_defs import CAPSULE_DATA_DIR, CAPSULE_INSTALL_DIR

def capsule_setup(ubman, disk_img, osindications):
    """setup the test

    Args:
        ubman -- A console connection to U-Boot.
        disk_img -- A path to disk image to be used for testing.
        osindications -- String of osindications value.
    """
    ubman.run_command_list([
        f'host bind 0 {disk_img}',
        'printenv -e PlatformLangCodes', # workaround for terminal size determination
        'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi',
        'efidebug boot order 1',
        'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;'
        'u-boot-env raw 0x150000 0x200000"'])

    if osindications is None:
        ubman.run_command('env set -e OsIndications')
    else:
        ubman.run_command(f'env set -e -nv -bs -rt OsIndications ={osindications}')

    ubman.run_command('env save')

def init_content(ubman, target, filename, expected):
    """initialize test content

    Args:
        ubman -- A console connection to U-Boot.
        target -- Target address to place the content.
        filename -- File name of the content.
        expected -- Expected string of the content.
    """
    output = ubman.run_command_list([
        'sf probe 0:0',
        f'fatload host 0:1 4000000 {CAPSULE_DATA_DIR}/{filename}',
        f'sf write 4000000 {target} 10',
        'sf read 5000000 100000 10',
        'md.b 5000000 10'])
    assert expected in ''.join(output)

def place_capsule_file(ubman, filenames):
    """place the capsule file

    Args:
        ubman -- A console connection to U-Boot.
        filenames -- File name array of the target capsule files.
    """
    for name in filenames:
        ubman.run_command_list([
            f'fatload host 0:1 4000000 {CAPSULE_DATA_DIR}/{name}',
            f'fatwrite host 0:1 4000000 {CAPSULE_INSTALL_DIR}/{name} $filesize'])

    output = ubman.run_command(f'fatls host 0:1 {CAPSULE_INSTALL_DIR}')
    for name in filenames:
        assert name in ''.join(output)

def exec_manual_update(ubman, disk_img, filenames, need_reboot = True):
    """execute capsule update manually

    Args:
        ubman -- A console connection to U-Boot.
        disk_img -- A path to disk image to be used for testing.
        filenames -- File name array of the target capsule files.
        need_reboot -- Flag indicates whether system reboot is required.
    """
    # make sure that dfu_alt_info exists even persistent variables
    # are not available.
    output = ubman.run_command_list([
        'env set dfu_alt_info '
                '"sf 0:0=u-boot-bin raw 0x100000 0x50000;'
                'u-boot-env raw 0x150000 0x200000"',
        f'host bind 0 {disk_img}',
        f'fatls host 0:1 {CAPSULE_INSTALL_DIR}'])
    for name in filenames:
        assert name in ''.join(output)

    # need to run uefi command to initiate capsule handling
    ubman.run_command(
        'env print -e Capsule0000', wait_for_reboot = need_reboot)

def check_file_removed(ubman, disk_img, filenames):
    """check files are removed

    Args:
        ubman -- A console connection to U-Boot.
        disk_img -- A path to disk image to be used for testing.
        filenames -- File name array of the target capsule files.
    """
    output = ubman.run_command_list([
        f'host bind 0 {disk_img}',
        f'fatls host 0:1 {CAPSULE_INSTALL_DIR}'])
    for name in filenames:
        assert name not in ''.join(output)

def check_file_exist(ubman, disk_img, filenames):
    """check files exist

    Args:
        ubman -- A console connection to U-Boot.
        disk_img -- A path to disk image to be used for testing.
        filenames -- File name array of the target capsule files.
    """
    output = ubman.run_command_list([
        f'host bind 0 {disk_img}',
        f'fatls host 0:1 {CAPSULE_INSTALL_DIR}'])
    for name in filenames:
        assert name in ''.join(output)

def verify_content(ubman, target, expected):
    """verify the content

    Args:
        ubman -- A console connection to U-Boot.
        target -- Target address to verify.
        expected -- Expected string of the content.
    """
    output = ubman.run_command_list([
        'sf probe 0:0',
        f'sf read 4000000 {target} 10',
        'md.b 4000000 10'])
    assert expected in ''.join(output)

def do_reboot_dtb_specified(u_boot_config, ubman, dtb_filename):
    """do reboot with specified DTB

    Args:
        u_boot_config -- U-boot configuration.
        ubman -- A console connection to U-Boot.
        dtb_filename -- DTB file name.
    """
    mnt_point = u_boot_config.persistent_data_dir + '/scratch'
    ubman.config.dtb = mnt_point + CAPSULE_DATA_DIR \
                                + f'/{dtb_filename}'
    ubman.restart_uboot()
