#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8821cu
PKG_VERSION:=5.8.1
PKG_RELEASE:=1
PKG_LICENSE:=GPLv3

PKG_SOURCE_URL:=https://github.com/brektrou/rtl8821CU.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-03-02
PKG_SOURCE_VERSION:=7756a7ab6b4dfd105d36d8433a72802ac0620d53
PKG_MIRROR_HASH:=4e9ac6bcedc0d3fc7d4b44be2076d6b9838d404ce0b8a8158b12287e9c0f53b6

PKG_MAINTAINER:=[CTCGFW] Project OpenWrt
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8821cu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek rtl8821cu Support
  DEPENDS:=@USB_SUPPORT +kmod-cfg80211 +kmod-mac80211 +kmod-usb-core +wireless-tools
  FILES:=$(PKG_BUILD_DIR)/rtl8821cu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8821cu)
  PROVIDES:=kmod-rtl8821cu
endef

define KernelPackage/rtl8821cu/description
 Kernel module for rtl8821cu wireless module.
endef

EXTRA_KCONFIG:=\
	CONFIG_RTL8821CU=m \
	USER_MODULE_NAME=rtl8821cu

ifeq ($(ARCH),aarch64)
	EXTRA_KCONFIG += CONFIG_MP_VHT_HW_TX_MODE=n
endif

EXTRA_CFLAGS:= \
	-DRTW_USE_CFG80211_STA_EVENT \
	-DCONFIG_RADIO_WORK \
	-DCONFIG_IOCTL_CFG80211 \
	-DCONFIG_CONCURRENT_MODE \

ifeq ($(ARCH),x86_64)
	EXTRA_CFLAGS += -mhard-float
endif

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	USER_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,rtl8821cu))
