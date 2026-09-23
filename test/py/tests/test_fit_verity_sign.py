# SPDX-License-Identifier: GPL-2.0
# Copyright 2026 Daniel Golle <daniel@makrotopia.org>

"""Verify that the dm-verity roothash is covered by the FIT configuration
signature.

A dm-verity protected filesystem image is not hashed by U-Boot; its integrity
is delegated to the kernel, which trusts the roothash taken from the FIT
``dm-verity`` subnode. That roothash must therefore be part of the signed
region of the configuration, otherwise an attacker can replace both the
filesystem and the roothash while keeping the configuration signature valid.

This test signs a configuration referencing a filesystem image that carries a
``dm-verity`` subnode, then flips one byte of the roothash and of the salt and
checks that verification rejects the image. A control tampering a byte that is
known to be signed confirms that the check is able to detect a broken region.

The FIT pairs a signed configuration with a filesystem image carrying a
``dm-verity`` subnode:

.. code-block:: devicetree

    images {
        rootfs-1 {
            data = /incbin/("rootfs.bin");
            type = "filesystem";
            compression = "none";
            hash-1 {
                algo = "sha256";
            };
            dm-verity {
                algo = "sha256";
                data-block-size = <4096>;
                hash-block-size = <4096>;
                num-data-blocks = <16>;
                hash-start-block = <16>;
            };
        };
    };

    configurations {
        conf-1 {
            kernel = "kernel-1";
            loadables = "rootfs-1";
            signature-1 {
                algo = "sha256,rsa2048";
                key-name-hint = "dev";
                sign-images = "kernel", "loadables";
            };
        };
    };

mkimage builds the dm-verity hash tree when assembling the image and records
the resulting roothash and salt in the ``dm-verity`` subnode; fit_check_sign
must reject an image where either was modified after signing.
"""

import os
import pytest
import utils

# 16 blocks of 4096 bytes, matching num-data-blocks/data-block-size below.
ROOTFS_SIZE = 16 * 4096

ITS = '''
/dts-v1/;
/ {
    description = "verity roothash signing coverage test";
    #address-cells = <1>;

    images {
        kernel-1 {
            description = "kernel";
            data = /incbin/("kernel.bin");
            type = "kernel";
            arch = "arm64";
            os = "linux";
            compression = "none";
            load = <0x40000000>;
            entry = <0x40000000>;
            hash-1 { algo = "sha256"; };
        };
        rootfs-1 {
            description = "rootfs";
            data = /incbin/("rootfs.bin");
            type = "filesystem";
            arch = "arm64";
            compression = "none";
            hash-1 { algo = "sha256"; };
            dm-verity {
                algo = "sha256";
                data-block-size = <4096>;
                hash-block-size = <4096>;
                num-data-blocks = <16>;
                hash-start-block = <16>;
            };
        };
    };

    configurations {
        default = "conf-1";
        conf-1 {
            description = "signed config";
            kernel = "kernel-1";
            loadables = "rootfs-1";
            signature-1 {
                algo = "sha256,rsa2048";
                key-name-hint = "dev";
                sign-images = "kernel", "loadables";
            };
        };
    };
};
'''

VERITY_NODE = '/images/rootfs-1/dm-verity'
ROOTFS_HASH_NODE = '/images/rootfs-1/hash-1'


def flip_prop_byte(ubman, fit, node, prop):
    """Flip the first byte of a byte-array property in a FIT, in place.

    The property is rewritten with the same length so that no node is
    relaid out and the signed regions keep their offsets.
    """
    val = utils.run_and_log(ubman, 'fdtget -t bx %s %s %s' % (fit, node, prop))
    bytelist = val.split()
    bytelist[0] = '%x' % (int(bytelist[0], 16) ^ 0xff)
    utils.run_and_log(ubman, 'fdtput -t bx %s %s %s %s' %
                      (fit, node, prop, ' '.join(bytelist)))


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('fit_signature')
@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
@pytest.mark.requiredtool('fdtput')
@pytest.mark.requiredtool('openssl')
@pytest.mark.requiredtool('veritysetup')
def test_fit_verity_roothash_signed(ubman):
    """The dm-verity roothash must be inside the signed configuration region."""
    tmpdir = os.path.join(ubman.config.result_dir, 'verity-sign') + '/'
    if not os.path.exists(tmpdir):
        os.makedirs(tmpdir)
    mkimage = ubman.config.build_dir + '/tools/mkimage'
    fit_check_sign = ubman.config.build_dir + '/tools/fit_check_sign'
    dtc_args = '-I dts -O dtb -i %s' % tmpdir
    its = tmpdir + 'verity.its'
    fit = tmpdir + 'verity.itb'
    dtb = tmpdir + 'control.dtb'

    # Signing key and empty control dtb to receive the public key.
    utils.run_and_log(ubman, 'openssl genpkey -algorithm RSA -out %sdev.key '
                      '-pkeyopt rsa_keygen_bits:2048 '
                      '-pkeyopt rsa_keygen_pubexp:65537' % tmpdir)
    utils.run_and_log(ubman, 'openssl req -batch -new -x509 -key %sdev.key '
                      '-out %sdev.crt' % (tmpdir, tmpdir))
    with open(tmpdir + 'control.dts', 'w') as f:
        f.write('/dts-v1/; / { model = "verity-test"; };\n')
    utils.run_and_log(ubman, 'dtc -O dtb -o %s %scontrol.dts' % (dtb, tmpdir))

    # Payloads. The rootfs must be a whole number of data blocks so mkimage can
    # build the dm-verity hash tree and compute the roothash.
    with open(tmpdir + 'rootfs.bin', 'wb') as f:
        f.write(b'R' * ROOTFS_SIZE)
    with open(tmpdir + 'kernel.bin', 'wb') as f:
        f.write(b'KERNEL')

    with open(its, 'w') as f:
        f.write(ITS)

    # Build and sign. -E keeps the (large) rootfs external, as on a real device.
    utils.run_and_log(ubman, [mkimage, '-D', dtc_args, '-E', '-f', its,
                              '-k', tmpdir, '-K', dtb, '-r', fit])

    # Baseline: the freshly signed image must verify.
    utils.run_and_log(ubman, [fit_check_sign, '-f', fit, '-k', dtb])

    # Control: tampering a byte that is signed (the filesystem image hash value)
    # must be detected. This proves the check can fail.
    control = tmpdir + 'control.itb'
    utils.run_and_log(ubman, 'cp %s %s' % (fit, control))
    flip_prop_byte(ubman, control, ROOTFS_HASH_NODE, 'value')
    utils.run_and_log_expect_exception(
        ubman, [fit_check_sign, '-f', control, '-k', dtb],
        1, 'Failed to verify required signature')

    # Roothash: tampering the dm-verity digest must be rejected. If the digest
    # is outside the signed region this check passes and boot is compromised.
    tampered = tmpdir + 'tamper-digest.itb'
    utils.run_and_log(ubman, 'cp %s %s' % (fit, tampered))
    flip_prop_byte(ubman, tampered, VERITY_NODE, 'digest')
    utils.run_and_log_expect_exception(
        ubman, [fit_check_sign, '-f', tampered, '-k', dtb],
        1, 'Failed to verify required signature')

    # Salt: likewise, the salt feeds the dm-verity target and must be signed.
    tampered = tmpdir + 'tamper-salt.itb'
    utils.run_and_log(ubman, 'cp %s %s' % (fit, tampered))
    flip_prop_byte(ubman, tampered, VERITY_NODE, 'salt')
    utils.run_and_log_expect_exception(
        ubman, [fit_check_sign, '-f', tampered, '-k', dtb],
        1, 'Failed to verify required signature')
