# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2017 Linaro Limited

obj-$(CONFIG_LIBAVB) += avb_chain_partition_descriptor.o avb_cmdline.o
obj-$(CONFIG_LIBAVB) += avb_crypto.o avb_footer.o avb_hashtree_descriptor.o
obj-$(CONFIG_LIBAVB) += avb_property_descriptor.o avb_mldsa.o
obj-$(CONFIG_LIBAVB) += avb_slot_verify.o avb_util.o avb_version.o
obj-$(CONFIG_LIBAVB) += avb_descriptor.o avb_hash_descriptor.o
obj-$(CONFIG_LIBAVB) += avb_kernel_cmdline_descriptor.o avb_rsa.o
obj-$(CONFIG_LIBAVB) += avb_sysdeps_posix.o avb_vbmeta_image.o
obj-$(CONFIG_LIBAVB) += crypto/sha_impl.o crypto/mldsa_impl.o

# avb_sha.h includes the crypto back end's avb_crypto_ops_impl.h
ccflags-y = -DAVB_COMPILATION -I$(srctree)/$(src)/crypto
