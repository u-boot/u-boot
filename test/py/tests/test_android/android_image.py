# SPDX-License-Identifier: GPL-2.0
"""Helpers to create Android disk images for sandbox testing."""

import collections
import os

import utils
from test_android import test_abootimg

def create_gpt_image(ubman, mmc_dev, required_parts, images):
    """Create a 20MB GPT disk image and write it to mmc{mmc_dev}.img

    Args:
        ubman: U-Boot console fixture
        mmc_dev (int): MMC device number, used to name the output image
            file mmc{mmc_dev}.img in the source directory
        required_parts (list of dict): Partitions to create, each with
            'num' (int) partition number, 'label' (str) partition name,
            and 'size' (str) either a sector count or 'NM' for N
            megabytes (e.g. '4M')
        images (list of tuple): Data to write into partitions, each a
            (partnum, filename, hex_data) tuple where partnum (int) is
            the target partition, filename (str) the image file name,
            and hex_data (str) the image content as hex
    """
    Partition = collections.namedtuple('part', 'start,size,name')
    parts = {}
    fname = os.path.join(ubman.config.source_dir, f'mmc{mmc_dev}.img')
    utils.run_and_log(ubman, f'qemu-img create {fname} 20M')
    utils.run_and_log(ubman, f'cgpt create {fname}')

    ptr = 40

    # Number of sectors in 1MB
    sect_size = 512
    sect_1mb = (1 << 20) // sect_size

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

    for partnum, img_name, hex_data in images:
        test_abootimg.AbootimgTestDiskImage(ubman, img_name, hex_data)
        img_path = os.path.join(ubman.config.result_dir, img_name)
        with open(img_path, 'rb') as inf:
            data = inf.read()
        start = parts[partnum].start * sect_size
        disk_data = disk_data[:start] + data + disk_data[start + len(data):]

    with open(fname, 'wb') as outf:
        outf.write(disk_data)

def setup_android_v4_image(ubman):
    """Create mmc7.img: boot image v4 with vendor_boot partitions."""
    required_parts = [
        {'num': 1, 'label':'misc', 'size': '1M'},
        {'num': 2, 'label':'boot_a', 'size': '4M'},
        {'num': 3, 'label':'boot_b', 'size': '4M'},
        {'num': 4, 'label':'vendor_boot_a', 'size': '4M'},
        {'num': 5, 'label':'vendor_boot_b', 'size': '4M'},
    ]
    images = [
        (2, 'bootv4.img', test_abootimg.boot_img_hex),
        (4, 'vendor_boot.img', test_abootimg.vboot_img_hex),
    ]
    create_gpt_image(ubman, 7, required_parts, images)

def setup_android_v2_image(ubman):
    """Create mmc8.img: boot image v2 without vendor_boot."""
    required_parts = [
        {'num': 1, 'label':'misc', 'size': '1M'},
        {'num': 2, 'label':'boot_a', 'size': '4M'},
        {'num': 3, 'label':'boot_b', 'size': '4M'},
    ]
    images = [
        (2, 'boot.img', test_abootimg.img_hex),
    ]
    create_gpt_image(ubman, 8, required_parts, images)

def setup_android_avb_image(ubman):
    """Create mmc12.img: boot image v2 with vbmeta.img."""

    required_parts = [
        {'num': 1, 'label':'boot', 'size': '4M'},
        {'num': 2, 'label':'system', 'size': '4M'},
        {'num': 3, 'label':'vbmeta', 'size': '4M'},
    ]
    images = [
        (1, 'boot.img', test_abootimg.img_hex),
        (3, 'vbmeta.img', test_abootimg.vbmeta_img_hex),
    ]
    create_gpt_image(ubman, 12, required_parts, images)

def setup_android_image(ubman):
    """Create disk images with Android partitions."""
    setup_android_v4_image(ubman)
    setup_android_v2_image(ubman)
    setup_android_avb_image(ubman)
