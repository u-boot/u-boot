# SPDX-License-Identifier: GPL-2.0+

"""Test the measurements made by the EFI_TCG2_PROTOCOL on sandbox

The sandbox TPM starts with every PCR clear, and the events measured when an
EFI application is started (boot variables, GPT, separators) are only measured
the first time one is. So each case restarts U-Boot, starts one application
and checks PCR 5 against the events it should then hold.
"""

import hashlib
import os
import re
import shutil
import struct
import zlib

import pytest

import utils
from tests.fs_helper import FsHelper

# EFI System Partition
ESP_TYPE = 'C12A7328-F81F-11D2-BA4B-00A0C93EC93B'
SECTOR = 512

# Fields of the GPT header, by offset
GPT_HEADER_SIZE = 92
GPT_HDR_SIZE_OFF = 12
GPT_HDR_CRC_OFF = 16
GPT_ALTERNATE_OFF = 32
GPT_ENTRIES_OFF = 72    # partition_entry_lba, num_partition_entries, ...
GPT_NUM_ENTRIES_OFF = 80
GPT_ENTRY_SIZE = 128


def extend(pcr, data):
    """Extend a SHA256 PCR value with the digest of some data

    Args:
        pcr (bytes): PCR value before
        data (bytes): Event data

    Returns:
        bytes: PCR value after
    """
    return hashlib.sha256(pcr + hashlib.sha256(data).digest()).digest()


def make_gpt_disk(ubman, fname):
    """Write a GPT disk image with helloworld.efi on its only partition

    Args:
        ubman (ConsoleBase): U-Boot console
        fname (str): Filename of the disk image
    """
    with FsHelper(ubman.config, 'vfat', 1, 'tcg2') as fsh:
        shutil.copy(os.path.join(ubman.config.build_dir,
                                 'lib/efi_loader/helloworld.efi'),
                    fsh.srcdir)
        fsh.mk_fs()
        with open(fsh.fs_img, 'rb') as inf:
            fs_data = inf.read()

    # 1MiB for the primary GPT, the partition, 1MiB for the backup GPT
    with open(fname, 'wb') as outf:
        outf.truncate((2 << 20) + len(fs_data))
    utils.run_and_log(ubman, f'sfdisk {fname}',
                      stdin=(f'label: gpt\nstart=2048, '
                             f'size={len(fs_data) // SECTOR}, '
                             f'type={ESP_TYPE}\n').encode())
    with open(fname, 'r+b') as outf:
        outf.seek(1 << 20)
        outf.write(fs_data)


def gpt_event(fname, backup=False):
    """Get the EV_EFI_GPT_EVENT data for a GPT of a disk image

    Args:
        fname (str): Filename of the disk image
        backup (bool): True for the backup GPT, as it is on the disk, else
            the primary one

    Returns:
        bytes: The header, the number of partitions, then their entries
    """
    with open(fname, 'rb') as inf:
        disk = inf.read()
    start = (len(disk) - SECTOR) if backup else SECTOR
    hdr = disk[start:start + GPT_HEADER_SIZE]
    entry_lba, num, size = struct.unpack_from('<QII', hdr, GPT_ENTRIES_OFF)
    entries = []
    for i in range(num):
        start = entry_lba * SECTOR + i * size
        entry = disk[start:start + size]
        if any(entry[:16]):
            entries.append(entry)
    return hdr + struct.pack('<Q', len(entries)) + b''.join(entries)


def set_num_entries(fname, num):
    """Set num_partition_entries in the primary GPT header and fix its CRC

    Args:
        fname (str): Filename of the disk image
        num (int): Number of partition entries
    """
    with open(fname, 'r+b') as outf:
        outf.seek(SECTOR)
        hdr = bytearray(outf.read(SECTOR))
        struct.pack_into('<I', hdr, GPT_NUM_ENTRIES_OFF, num)
        struct.pack_into('<I', hdr, GPT_HDR_CRC_OFF, 0)
        hdr_size = struct.unpack_from('<I', hdr, GPT_HDR_SIZE_OFF)[0]
        struct.pack_into('<I', hdr, GPT_HDR_CRC_OFF,
                         zlib.crc32(hdr[:hdr_size]))
        outf.seek(SECTOR)
        outf.write(hdr)


def ignore_primary(fname):
    """Mark the primary GPT header to be ignored, as ChromeOS does

    Args:
        fname (str): Filename of the disk image
    """
    with open(fname, 'r+b') as outf:
        outf.seek(SECTOR)
        outf.write(b'IGNOREME')


def corrupt_entries(fname):
    """Change an unused entry of the primary GPT, leaving the header as it is

    The CRC of the entries in the header no longer matches them.

    Args:
        fname (str): Filename of the disk image
    """
    with open(fname, 'r+b') as outf:
        outf.seek(SECTOR)
        hdr = outf.read(GPT_HEADER_SIZE)
        entry_lba = struct.unpack_from('<Q', hdr, GPT_ENTRIES_OFF)[0]
        # the name of the second entry, whose type GUID stays zero
        outf.seek(entry_lba * SECTOR + GPT_ENTRY_SIZE + 56)
        outf.write(b'x')


def copy_backup_gpt(src, dst):
    """Copy the backup GPT of a disk image to another of the same size

    Args:
        src (str): Filename of the disk image to copy it from
        dst (str): Filename of the disk image to copy it to
    """
    with open(src, 'rb') as inf:
        inf.seek(SECTOR)
        hdr = inf.read(GPT_HEADER_SIZE)
        backup_lba = struct.unpack_from('<Q', hdr, GPT_ALTERNATE_OFF)[0]
        inf.seek(backup_lba * SECTOR)
        hdr = inf.read(GPT_HEADER_SIZE)
        entry_lba = struct.unpack_from('<Q', hdr, GPT_ENTRIES_OFF)[0]
        inf.seek(entry_lba * SECTOR)
        data = inf.read()
    with open(dst, 'r+b') as outf:
        outf.seek(entry_lba * SECTOR)
        outf.write(data)


def boot_and_read_pcr5(ubman, fname):
    """Start helloworld.efi from a disk image on a fresh U-Boot

    Args:
        ubman (ConsoleBase): U-Boot console
        fname (str): Filename of the disk image

    Returns:
        bytes: PCR 5 after the application has run
    """
    addr = utils.find_ram_base(ubman) + (4 << 20)
    ubman.restart_uboot()
    ubman.run_command(f'host bind 0 {fname}')
    output = ubman.run_command(f'load host 0:1 {addr:x} helloworld.efi')
    assert 'bytes read' in output
    output = ubman.run_command(f'bootefi {addr:x}')
    assert 'Hello, world!' in output
    output = ubman.run_command(f'tpm2 pcr_read 5 {addr:x}')
    assert 'PCR #5 sha256 32 byte content' in output
    digest = output.split('content', 1)[1].split('\n', 1)[1]
    return bytes.fromhex(''.join(re.findall(r'\b[0-9a-f]{2}\b', digest)))


@pytest.mark.boardspec('sandbox')
# EFI applications are built with the sanitizer but cannot run with it
@pytest.mark.notbuildconfigspec('asan')
@pytest.mark.buildconfigspec('efi_tcg2_protocol')
@pytest.mark.buildconfigspec('efi_partition')
@pytest.mark.buildconfigspec('cmd_tpm_v2')
@pytest.mark.buildconfigspec('bootefi_hello_compile')
def test_efi_tcg2_gpt(ubman):
    """Check that PCR 5 holds the GPT the partitions are found in

    PCR 5 holds the GPT event and then the separator. When the partition
    driver refuses the primary GPT, it finds the partitions in the backup
    one, and the GPT event holds that, with the primary header it restores,
    or the backup header as it is when the primary one is marked to be
    ignored.
    """
    fname = os.path.join(ubman.config.persistent_data_dir, 'tcg2_gpt.img')
    other = os.path.join(ubman.config.persistent_data_dir, 'tcg2_other.img')
    separator = struct.pack('<I', 0)
    try:
        make_gpt_disk(ubman, fname)
        expect = extend(extend(bytes(32), gpt_event(fname)), separator)
        assert boot_and_read_pcr5(ubman, fname) == expect

        # an entry array whose size comes to one block in 32 bits: the
        # backup GPT is the same GPT, so PCR 5 is too
        set_num_entries(fname, (1 << 32) // GPT_ENTRY_SIZE +
                        SECTOR // GPT_ENTRY_SIZE)
        assert boot_and_read_pcr5(ubman, fname) == expect

        # a backup GPT whose partition has another GUID, and primary entries
        # that do not match their CRC, though the change is in an unused
        # entry, which is not measured
        make_gpt_disk(ubman, fname)
        shutil.copy(fname, other)
        utils.run_and_log(ubman, f'sfdisk --part-uuid {other} 1 '
                          '0a6a1f54-1b55-4d0c-9a52-6d1e5e0e5d01')
        expect = extend(extend(bytes(32), gpt_event(other)), separator)
        copy_backup_gpt(other, fname)
        corrupt_entries(fname)
        assert boot_and_read_pcr5(ubman, fname) == expect

        # a primary header marked to be ignored, which is never repaired:
        # PCR 5 holds the backup GPT as it is
        make_gpt_disk(ubman, fname)
        ignore_primary(fname)
        expect = extend(extend(bytes(32), gpt_event(fname, backup=True)),
                        separator)
        assert boot_and_read_pcr5(ubman, fname) == expect
    finally:
        for path in (fname, other):
            if os.path.exists(path):
                os.remove(path)
        ubman.restart_uboot()
