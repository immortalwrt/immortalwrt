# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 Weijie Gao <hackpascal@gmail.com>

include $(TOPDIR)/rules.mk

ARCH:=loongarch64
BOARD:=loongarch64
BOARDNAME:=Loongson LoongArch
FEATURES:=audio display ext4 pcie boot-part rootfs-part rtc usb targz
SUBTARGETS:=generic

KERNEL_PATCHVER:=6.6

KERNELNAME:=vmlinuz.efi dtbs

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += \
	partx-utils blkid fdisk automount grub2-efi-loongarch64 \
	kmod-e1000e kmod-i40e kmod-igb kmod-igbvf kmod-igc kmod-ixgbe \
	kmod-ixgbevf kmod-r8125 kmod-r8126 kmod-r8127 kmod-r8168 kmod-yt6801 \
	kmod-usb-net-asix kmod-usb-net-asix-ax88179 kmod-usb-net-rtl8150 \
	kmod-usb-net-rtl8152-vendor

$(eval $(call BuildTarget))
