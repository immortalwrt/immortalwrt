# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2026 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=yt6801
PKG_VERSION:=1.0.31
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-dkms_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=@DEBIAN/pool/main/y/yt6801-dkms
PKG_HASH:=b8a558c845e7e2605dd1954913896208edb6ab56b0967068610cfa1a59727710
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-dkms-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0-or-later
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/yt6801
  SUBMENU:=Network Devices
  TITLE:=Motorcomm YT6801 ethernet controller driver
  DEPENDS:=@PCI_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/src/yt6801.ko
  AUTOLOAD:=$(call AutoProbe,yt6801,1)
endef

define KernelPackage/yt6801/description
  The YT6801 is a high-performance Ethernet controller designed by
  Motor-comm Corporation, offering advanced networking features and
  reliable connectivity for various applications.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		FXGMAC_EPHY_LOOPBACK_DETECT_ENABLED=y \
		modules
endef

$(eval $(call KernelPackage,yt6801))
