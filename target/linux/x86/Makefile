# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2006-2011 OpenWrt.org

include $(TOPDIR)/rules.mk

ARCH:=i386
BOARD:=x86
BOARDNAME:=x86
FEATURES:=squashfs ext4 vdi vmdk vhdx pcmcia targz fpu boot-part rootfs-part
SUBTARGETS:=64 generic legacy geode

KERNEL_PATCHVER:=4.19
KERNEL_TESTING_PATCHVER:=4.19

KERNELNAME:=bzImage

include $(INCLUDE_DIR)/target.mk

DEFAULT_PACKAGES += partx-utils mkf2fs fdisk e2fsprogs kmod-button-hotplug kmod-usb-hid \
		    kmod-r8168 kmod-fs-f2fs alsa-utils \
		    kmod-ac97 kmod-sound-hda-core kmod-sound-hda-codec-realtek kmod-sound-hda-codec-via \
		    kmod-sound-via82xx kmod-sound-hda-intel kmod-sound-hda-codec-hdmi kmod-sound-i8x0 \
		    autocore-x86 automount

$(eval $(call BuildTarget))

$(eval $(call $(if $(CONFIG_ISO_IMAGES),SetupHostCommand,Ignore),mkisofs, \
	Please install mkisofs. , \
	mkisofs -v 2>&1 , \
	genisoimage -v 2>&1 | grep genisoimage, \
	xorrisofs -v 2>&1 | grep xorriso \
))
