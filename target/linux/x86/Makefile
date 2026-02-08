# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=i386
BOARD:=x86
BOARDNAME:=x86
FEATURES:=squashfs ext4 vdi vmdk vhdx pcmcia targz fpu boot-part rootfs-part
SUBTARGETS:=64 generic legacy geode

KERNEL_PATCHVER:=6.12

KERNELNAME:=bzImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += fdisk mkf2fs partx-utils kmod-button-hotplug kmod-e1000e kmod-i40e \
		    kmod-igb kmod-igbvf kmod-igc kmod-ixgbe kmod-ixgbevf kmod-pcnet32 \
		    kmod-tulip kmod-vmxnet3 kmod-r8101 kmod-r8125 kmod-r8126 kmod-r8168 \
		    kmod-8139cp kmod-8139too kmod-fs-f2fs kmod-usb-hid kmod-usb-net \
		    kmod-usb-net-asix kmod-usb-net-asix-ax88179 kmod-usb-net-rtl8150 \
		    kmod-usb-net-rtl8152-vendor grub2-bios-setup i915-firmware-dmc \
		    automount

$(eval $(call BuildTarget))

$(eval $(call $(if $(CONFIG_ISO_IMAGES),SetupHostCommand,Ignore),mkisofs, \
	Please install mkisofs. , \
	mkisofs -v 2>&1 , \
	genisoimage -v 2>&1 | grep genisoimage, \
	xorrisofs -v 2>&1 | grep xorriso \
))
