# SPDX-License-Identifier: GPL-2.0+
# Copyright 2026 Canonical Ltd.
#
# Test mkimage handling of the 'image-data' FIT property

"""Tests for the FIT 'image-data' property.

An image node may name another image node under /images in an
'image-data' property to share its binary data. mkimage resolves the
reference by copying the data when building an inline FIT, and by
pointing both nodes at the same region when building with external data
(-E). Invalid references (undefined target, chained references,
mismatching data or incompatible cipher settings) must be rejected.
"""

import os
import struct
import subprocess

import pytest

import fit_util

BASE_ITS = '''
/dts-v1/;

/ {
    description = "image-data test";

    images {
        kernel-1 {
            description = "test kernel";
            data = /incbin/("%(kernel)s");
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x40000>;
            entry = <0x40000>;
            hash-1 {
                algo = "sha256";
            };
        };
        kernel-alt {
            description = "same kernel at another address";
            %(alt_props)s
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x80000>;
            entry = <0x80000>;
            hash-1 {
                algo = "sha256";
            };
        };
        %(extra_images)s
    };

    configurations {
        default = "conf-1";
        conf-1 {
            kernel = "kernel-1";
        };
        conf-2 {
            kernel = "kernel-alt";
        };
    };
};
'''

CHAIN_IMAGE = '''
        kernel-mid {
            description = "middle image in a chain";
            image-data = "kernel-1";
            type = "kernel";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x60000>;
            entry = <0x60000>;
        };
'''

CIPHER = '''
            cipher {
                algo = "aes256";
                key-name-hint = "kern";
                iv-name-hint = "kiv";
            };
'''

CIPHER_NO_IV = '''
            cipher {
                algo = "aes256";
                key-name-hint = "kern";
            };
'''

CIPHER_IV = '''
            cipher {
                algo = "aes256";
                key-name-hint = "kern";
                iv = [11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00];
            };
'''

CIPHER_IV_ALT = '''
            cipher {
                algo = "aes256";
                key-name-hint = "kern";
                iv = [00 ff ee dd cc bb aa 99 88 77 66 55 44 33 22 11];
            };
'''

OVERLAP_SHARER = '''
        filler {
            description = "second sharer overlapping kernel-alt";
            image-data = "kernel-1";
            type = "loadable";
            arch = "sandbox";
            os = "linux";
            compression = "none";
            load = <0x80400>;
        };
'''


def mkimage(ubman, *args):
    """Run mkimage with the given arguments, returning the process result"""
    tool = os.path.join(ubman.config.build_dir, 'tools/mkimage')
    return subprocess.run([tool] + list(args), capture_output=True, text=True)


def fdtget(itb, node, prop, ftype=None):
    """Read a property from the FIT, returning None if it is missing"""
    cmd = ['fdtget'] + (['-t', ftype] if ftype else []) + [itb, node, prop]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode:
        return None
    return result.stdout.strip()


def fdt_bytes(itb, node, prop):
    """Read a property from the FIT as bytes, returning None if missing"""
    value = fdtget(itb, node, prop, 'bx')
    if value is None:
        return None
    return bytes(int(tok, 16) for tok in value.split())


def build_fit_text(ubman, its_text, basename, *args, alt_props=None,
                   extra_images=''):
    """Build a FIT from a full .its template, returning (result, itb, kdata)"""
    kernel = fit_util.make_kernel(ubman, f'{basename}-kernel.bin', 'kernel')
    params = {
        'kernel': kernel,
        'alt_props': alt_props or 'image-data = "kernel-1";',
        'extra_images': extra_images,
    }
    its = fit_util.make_its(ubman, its_text, params, f'{basename}.its')
    itb = fit_util.make_fname(ubman, f'{basename}.itb')
    result = mkimage(ubman, *args, '-f', its, itb)
    with open(kernel, 'rb') as inf:
        kdata = inf.read()
    return result, itb, kdata


def build_fit(ubman, alt_props, extra_images='', basename='idata', *args):
    """Build a FIT from BASE_ITS, returning (result, itb, kernel_data)"""
    return build_fit_text(ubman, BASE_ITS, basename, *args,
                          alt_props=alt_props, extra_images=extra_images)


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
def test_fit_image_data_inline(ubman):
    """Inline FIT: the data is copied and the reference is retained"""
    result, itb, kdata = build_fit(ubman, 'image-data = "kernel-1";')
    assert result.returncode == 0, result.stderr

    assert fdt_bytes(itb, '/images/kernel-1', 'data') == kdata
    assert fdt_bytes(itb, '/images/kernel-alt', 'data') == kdata

    # The reference and the node's own properties are retained
    assert fdtget(itb, '/images/kernel-alt', 'image-data', 's') == 'kernel-1'
    assert fdtget(itb, '/images/kernel-alt', 'load', 'x') == '80000'

    # Each node hashes the same bytes
    hash1 = fdtget(itb, '/images/kernel-1/hash-1', 'value', 'bx')
    hash2 = fdtget(itb, '/images/kernel-alt/hash-1', 'value', 'bx')
    assert hash1 and hash1 == hash2


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
def test_fit_image_data_external(ubman):
    """External FIT (-E): both nodes reference a single copy of the data"""
    result, itb, kdata = build_fit(ubman, 'image-data = "kernel-1";',
                                   '', 'idata-ext', '-E')
    assert result.returncode == 0, result.stderr

    assert fdtget(itb, '/images/kernel-1', 'data-offset') == \
        fdtget(itb, '/images/kernel-alt', 'data-offset')
    assert fdtget(itb, '/images/kernel-1', 'data-size') == \
        fdtget(itb, '/images/kernel-alt', 'data-size') == str(len(kdata))
    assert fdtget(itb, '/images/kernel-alt', 'data') is None

    # The payload appears exactly once, at the shared offset
    with open(itb, 'rb') as inf:
        blob = inf.read()
    assert blob.count(kdata) == 1
    totalsize = struct.unpack('>I', blob[4:8])[0]
    offset = int(fdtget(itb, '/images/kernel-1', 'data-offset'))
    assert blob[totalsize + offset:totalsize + offset + len(kdata)] == kdata


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
def test_fit_image_data_reprocess(ubman):
    """A resolved FIT can be re-processed with -F, including to -E"""
    result, itb, kdata = build_fit(ubman, 'image-data = "kernel-1";',
                                   '', 'idata-re')
    assert result.returncode == 0, result.stderr

    result = mkimage(ubman, '-F', itb)
    assert result.returncode == 0, result.stderr

    result = mkimage(ubman, '-F', '-E', itb)
    assert result.returncode == 0, result.stderr
    assert fdtget(itb, '/images/kernel-1', 'data-offset') == \
        fdtget(itb, '/images/kernel-alt', 'data-offset')
    with open(itb, 'rb') as inf:
        assert inf.read().count(kdata) == 1


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
def test_fit_image_data_invalid(ubman):
    """Invalid image-data references are rejected"""
    # Reference to an undefined image
    result, itb, kdata = build_fit(ubman, 'image-data = "nope";',
                                   '', 'idata-undef')
    assert result.returncode != 0
    assert "references undefined image 'nope'" in result.stderr

    # Chained reference
    result, itb, kdata = build_fit(ubman, 'image-data = "kernel-mid";',
                                   CHAIN_IMAGE, 'idata-chain')
    assert result.returncode != 0
    assert 'chained references are not permitted' in result.stderr

    # Own data disagreeing with the target's data
    other = fit_util.make_kernel(ubman, 'idata-other.bin', 'other')
    props = f'image-data = "kernel-1";\ndata = /incbin/("{other}");'
    result, itb, kdata = build_fit(ubman, props, '', 'idata-mismatch')
    assert result.returncode != 0
    assert "data does not match image-data target 'kernel-1'" in result.stderr

    # image-data combined with a partial external data reference
    result, itb, kdata = build_fit(
        ubman, 'image-data = "kernel-1";\ndata-size = <0x10>;',
        '', 'idata-extprop')
    assert result.returncode != 0
    assert 'must not be combined with external data properties' in \
        result.stderr

    # A complete reference is rejected before the import when compiling
    # a source file, whether it uses an offset or a position
    result, itb, kdata = build_fit(
        ubman,
        'image-data = "kernel-1";\ndata-offset = <0>;\ndata-size = <0x10>;',
        '', 'idata-extpair')
    assert result.returncode != 0
    assert 'must not be combined with external data properties' in \
        result.stderr
    result, itb, kdata = build_fit(
        ubman,
        'image-data = "kernel-1";\ndata-position = <0x1000>;\n'
        'data-size = <0x10>;',
        '', 'idata-extpos')
    assert result.returncode != 0
    assert 'must not be combined with external data properties' in \
        result.stderr


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
def test_fit_image_data_cipher_invalid(ubman):
    """Sharing encrypted data without compatible cipher settings fails"""
    # Only the sharing node is encrypted
    its_text = BASE_ITS.replace('entry = <0x80000>;',
                                'entry = <0x80000>;' + CIPHER)
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-oneside')
    assert result.returncode != 0
    assert 'only one of them is encrypted' in result.stderr

    # Both encrypted but without any IV source, so each would get a
    # random IV and the ciphertexts would differ
    its_text = BASE_ITS.replace('hash-1 {', CIPHER_NO_IV + 'hash-1 {')
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-noiv')
    assert result.returncode != 0
    assert 'must use the same iv-name-hint or iv' in result.stderr

    # Different explicit IVs would produce different ciphertexts
    its_text = BASE_ITS.replace('entry = <0x40000>;',
                                'entry = <0x40000>;' + CIPHER_IV)
    its_text = its_text.replace('entry = <0x80000>;',
                                'entry = <0x80000>;' + CIPHER_IV_ALT)
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-diff-iv')
    assert result.returncode != 0
    assert 'must use the same iv-name-hint or iv' in result.stderr

    # Different iv-name-hints as well, since a hint drives the
    # encryption whenever it is present
    its_text = BASE_ITS.replace('entry = <0x40000>;',
                                'entry = <0x40000>;' + CIPHER)
    its_text = its_text.replace('entry = <0x80000>;',
                                'entry = <0x80000>;' +
                                CIPHER.replace('kiv', 'kiv2'))
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-diff-hint')
    assert result.returncode != 0
    assert 'must use the same iv-name-hint' in result.stderr


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
def test_fit_image_data_cipher(ubman):
    """Nodes with identical cipher settings share the same ciphertext"""
    keydir = fit_util.make_fname(ubman, 'idata-keys')
    os.makedirs(keydir, exist_ok=True)
    with open(os.path.join(keydir, 'kern.bin'), 'wb') as outf:
        outf.write(os.urandom(32))
    with open(os.path.join(keydir, 'kiv.bin'), 'wb') as outf:
        outf.write(os.urandom(16))

    its_text = BASE_ITS.replace('hash-1 {', CIPHER + 'hash-1 {')
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-cipher',
                                        '-k', keydir)
    assert result.returncode == 0, result.stderr

    ciphertext = fdt_bytes(itb, '/images/kernel-1', 'data')
    assert ciphertext == fdt_bytes(itb, '/images/kernel-alt', 'data')
    assert ciphertext != kdata
    assert fdtget(itb, '/images/kernel-1', 'data-size-unciphered') == \
        fdtget(itb, '/images/kernel-alt', 'data-size-unciphered') == \
        str(len(kdata))

    # The shared ciphertext can also be made external
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-cipher-ext',
                                        '-k', keydir, '-E')
    assert result.returncode == 0, result.stderr
    assert fdtget(itb, '/images/kernel-1', 'data-offset') == \
        fdtget(itb, '/images/kernel-alt', 'data-offset')

    # A shared explicit iv works as well: it is honored by the
    # encryption, retained in the FIT and yields identical ciphertext
    its_text = BASE_ITS.replace('hash-1 {', CIPHER_IV + 'hash-1 {')
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-cipher-iv',
                                        '-k', keydir)
    assert result.returncode == 0, result.stderr
    ciphertext = fdt_bytes(itb, '/images/kernel-1', 'data')
    assert ciphertext == fdt_bytes(itb, '/images/kernel-alt', 'data')
    assert ciphertext != kdata
    authored_iv = bytes.fromhex('112233445566778899aabbccddeeff00')
    assert fdt_bytes(itb, '/images/kernel-1/cipher', 'iv') == authored_iv
    assert fdt_bytes(itb, '/images/kernel-alt/cipher', 'iv') == authored_iv

    result, itb, kdata = build_fit_text(ubman, its_text,
                                        'idata-cipher-iv-ext', '-k', keydir,
                                        '-E')
    assert result.returncode == 0, result.stderr
    assert fdtget(itb, '/images/kernel-1', 'data-offset') == \
        fdtget(itb, '/images/kernel-alt', 'data-offset')


@pytest.mark.boardspec('sandbox')
@pytest.mark.requiredtool('dtc')
def test_fit_image_data_overlap(ubman):
    """Sharers' load regions are covered by the configuration overlap check"""
    its_text = BASE_ITS.replace(
        'kernel = "kernel-alt";',
        'kernel = "kernel-alt";\n            loadables = "filler";')
    result, itb, kdata = build_fit_text(ubman, its_text, 'idata-overlap',
                                        extra_images=OVERLAP_SHARER)
    assert result.returncode != 0
    assert 'overlapping load regions' in result.stderr
