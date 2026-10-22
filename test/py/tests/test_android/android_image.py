# SPDX-License-Identifier: GPL-2.0
"""Helpers to create Android disk images for sandbox testing."""

import collections
import os

import utils
from test_android import test_abootimg

def setup_android_image(ubman):
    """Create a 20MB disk image with Android partitions"""
    Partition = collections.namedtuple('part', 'start,size,name')
    parts = {}
    disk_data = None

    def set_part_data(partnum, data):
        """Set the contents of a disk partition

        This updates disk_data by putting data in the right place

        Args:
            partnum (int): Partition number to set
            data (bytes): Data for that partition
        """
        nonlocal disk_data

        start = parts[partnum].start * sect_size
        disk_data = disk_data[:start] + data + disk_data[start + len(data):]

    mmc_dev = 7
    fname = os.path.join(ubman.config.source_dir, f'mmc{mmc_dev}.img')
    utils.run_and_log(ubman, f'qemu-img create {fname} 20M')
    utils.run_and_log(ubman, f'cgpt create {fname}')

    ptr = 40

    # Number of sectors in 1MB
    sect_size = 512
    sect_1mb = (1 << 20) // sect_size

    required_parts = [
        {'num': 1, 'label':'misc', 'size': '1M'},
        {'num': 2, 'label':'boot_a', 'size': '4M'},
        {'num': 3, 'label':'boot_b', 'size': '4M'},
        {'num': 4, 'label':'vendor_boot_a', 'size': '4M'},
        {'num': 5, 'label':'vendor_boot_b', 'size': '4M'},
    ]

    for part in required_parts:
        size_str = part['size']
        if 'M' in size_str:
            size = int(size_str[:-1]) * sect_1mb
        else:
            size = int(size_str)
        utils.run_and_log(
            ubman,
            f"cgpt add -i {part['num']} -b {ptr} -s {size} -l {part['label']} -t basicdata {fname}")
        ptr += size

    utils.run_and_log(ubman, f'cgpt boot -p {fname}')
    out = utils.run_and_log(ubman, f'cgpt show -q {fname}')

    # Create a dict (indexed by partition number) containing the above info
    for line in out.splitlines():
        start, size, num, name = line.split(maxsplit=3)
        parts[int(num)] = Partition(int(start), int(size), name)

    with open(fname, 'rb') as inf:
        disk_data = inf.read()

    test_abootimg.AbootimgTestDiskImage(ubman, 'bootv4.img', test_abootimg.boot_img_hex)
    boot_img = os.path.join(ubman.config.result_dir, 'bootv4.img')
    with open(boot_img, 'rb') as inf:
        set_part_data(2, inf.read())

    test_abootimg.AbootimgTestDiskImage(ubman, 'vendor_boot.img', test_abootimg.vboot_img_hex)
    vendor_boot_img = os.path.join(ubman.config.result_dir, 'vendor_boot.img')
    with open(vendor_boot_img, 'rb') as inf:
        set_part_data(4, inf.read())

    with open(fname, 'wb') as outf:
        outf.write(disk_data)

    mmc_dev = 8
    fname = os.path.join(ubman.config.source_dir, f'mmc{mmc_dev}.img')
    utils.run_and_log(ubman, f'qemu-img create {fname} 20M')
    utils.run_and_log(ubman, f'cgpt create {fname}')

    ptr = 40

    # Number of sectors in 1MB
    sect_size = 512
    sect_1mb = (1 << 20) // sect_size

    required_parts = [
        {'num': 1, 'label':'misc', 'size': '1M'},
        {'num': 2, 'label':'boot_a', 'size': '4M'},
        {'num': 3, 'label':'boot_b', 'size': '4M'},
    ]

    for part in required_parts:
        size_str = part['size']
        if 'M' in size_str:
            size = int(size_str[:-1]) * sect_1mb
        else:
            size = int(size_str)
        utils.run_and_log(
            ubman,
            f"cgpt add -i {part['num']} -b {ptr} -s {size} -l {part['label']} -t basicdata {fname}")
        ptr += size

    utils.run_and_log(ubman, f'cgpt boot -p {fname}')
    out = utils.run_and_log(ubman, f'cgpt show -q {fname}')

    # Create a dict (indexed by partition number) containing the above info
    for line in out.splitlines():
        start, size, num, name = line.split(maxsplit=3)
        parts[int(num)] = Partition(int(start), int(size), name)

    with open(fname, 'rb') as inf:
        disk_data = inf.read()

    test_abootimg.AbootimgTestDiskImage(ubman, 'boot.img', test_abootimg.img_hex)
    boot_img = os.path.join(ubman.config.result_dir, 'boot.img')
    with open(boot_img, 'rb') as inf:
        set_part_data(2, inf.read())

    with open(fname, 'wb') as outf:
        outf.write(disk_data)
