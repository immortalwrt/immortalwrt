# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=mt7601u-ap
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Anthony96922/mt7601u-ap.git
PKG_SOURCE_DATE:=2022-12-28
PKG_SOURCE_VERSION:=bb9bfcfd8327bd0973c68417d41cafc41ca2ed09
PKG_MIRROR_HASH:=0b34fc401eb027d46cb07b4579dd4212e93b7eb45c93591806582166611267b3

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7601u-ap
  SUBMENU:=Wireless Drivers
  TITLE:=AP driver for MT7601U dongles
  DEPENDS:=+kmod-usb-core +wireless-tools @USB_SUPPORT +@DRIVER_WEXT_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/os/linux/mt7601Uap.ko
  AUTOLOAD:=$(call AutoProbe,mt7601Uap)
  CONFLICTS:=kmod-mt7601u
endef

define KernelPackage/mt7601u-ap/description
  AP driver for MT7601U chipset based adapters
endef

define Build/Prepare
	$(Build/Prepare/Default)
	$(CP) $(PKG_BUILD_DIR)/os/linux/Makefile.6 $(PKG_BUILD_DIR)/os/linux/Makefile
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/os/linux" \
		RT28xx_DIR="$(PKG_BUILD_DIR)" \
		RT28xx_MODE=AP \
		CHIPSET=7601U \
		MODULE=7601U \
		OSABL=NO \
		modules
endef

define KernelPackage/mt7601u-ap/install
	$(INSTALL_DIR) $(1)/etc/wifi/RT2870AP
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/RT2870AP.txt $(1)/etc/wifi/RT2870AP/
endef

$(eval $(call KernelPackage,mt7601u-ap))
