# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2026 xmb505
#

include $(TOPDIR)/rules.mk

PKG_NAME:=zte-mf253s
PKG_VERSION:=1.0.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/xmb505/zte-mf253s/archive/refs/tags/
PKG_SOURCE_URL_FILE:=v$(PKG_VERSION).tar.gz
PKG_HASH:=1b3cdd3b56dce8bf8099f25690cb69f9e163e8d3de492bf35891c44d9db98701

PKG_LICENSE:=GPL-2.0
PKG_MAINTAINER:=xmb505 <retromilia@foxmail.com>
PKG_BUILD_PARALLEL:=1

PKG_EXTMOD_SUBDIRS:=drivers/zte_ecm drivers/zte_atfix

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

TAR_OPTIONS+= --strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1) $(TAR_OPTIONS)

define KernelPackage/zte-ecm
  SUBMENU:=USB Support
  TITLE:=ZTE ZX297510 (MF253S) ECM data interface driver
  DEPENDS:=+kmod-usb-net +kmod-usb-net-cdc-ether +kmod-zte-atfix
  FILES:=$(PKG_BUILD_DIR)/drivers/zte_ecm/zte_ecm.ko
  AUTOLOAD:=$(call AutoProbe,zte_ecm)
endef

define KernelPackage/zte-ecm/description
  usbnet driver for the ECM data interface (if1) of the ZTE MF253S /
  ME3760V2 (ZX297510) modem.  IPv6 frames are dropped in the data path;
  the IPv4 settings are learned from the +ZGIPDNS URC (handled by
  kmod-zte-atfix), there is no DHCP on the interface.
endef

define KernelPackage/zte-atfix
  SUBMENU:=USB Support
  TITLE:=ZTE ZX297510 (MF253S) AT fixup and link management
  DEPENDS:=+kmod-usb-serial
  FILES:=$(PKG_BUILD_DIR)/drivers/zte_atfix/zte_atfix.ko
  AUTOLOAD:=$(call AutoProbe,zte_atfix)
endef

define KernelPackage/zte-atfix/description
  usb-serial driver for the AT interfaces (if0/2/3) of the ZTE MF253S /
  ME3760V2 (ZX297510) modem: AT command fixups, ZTE Icera spoofing so
  stock ModemManager drives the ECM data port, private-port dial
  translation (AT+CGACT / AT+ZGACT), +ZGIPDNS IP cache and the keepalive
  polling the firmware needs.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/drivers/zte_ecm" \
		modules
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/drivers/zte_atfix" \
		modules
endef

$(eval $(call KernelPackage,zte-ecm))
$(eval $(call KernelPackage,zte-atfix))
