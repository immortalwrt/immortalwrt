#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ndpi-netfilter
PKG_VERSION:=3.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/betolj/ndpi-netfilter
PKG_MIRROR_HASH:=1a26ea73375a99831be6949faa9bc768e9f4f9e3594286729ec9e61dd6402b07
PKG_SOURCE_VERSION:=4875069df027199698ba0b7c53541ae2a4e3ba00

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/iptables-mod-ndpi
  SUBMENU:=Firewall
  SECTION:=net
  CATEGORY:=Network
  TITLE:=nDPI successor of OpenDPI
  URL:=https://github.com/ntop/nDPI
  DEPENDS:=+iptables +iptables-mod-conntrack-extra +kmod-ipt-ndpi
  MAINTAINER:=Thomas Heil <heil@terminal-consulting.de>
endef

define Package/iptables-mod-ndpi/description
  nDPI is a ntop-maintained superset of the popular OpenDPI library
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(TAR) -zxf $(PKG_BUILD_DIR)/nDPI.tar.gz -C $(PKG_BUILD_DIR)
endef

MAKE_FLAGS +=\
	KERNEL_DIR="$(LINUX_DIR)" \
	ARCH="$(LINUX_KARCH)" \
	NDPI_PATH=$(PKG_BUILD_DIR)/nDPI \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	all

define Package/iptables-mod-ndpi/install
	$(INSTALL_DIR) $(1)/usr/lib/iptables
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipt/libxt_ndpi.so $(1)/usr/lib/iptables
endef

define KernelPackage/ipt-ndpi
  SUBMENU:=Netfilter Extensions
  TITLE:=nDPI netfilter module
  DEPENDS:=+kmod-nf-conntrack +kmod-nf-conntrack-netlink +kmod-ipt-compat-xtables
  FILES:= \
	$(PKG_BUILD_DIR)/src/xt_ndpi.ko
  AUTOLOAD:=$(call AutoProbe,xt_ndpi)
endef

$(eval $(call BuildPackage,iptables-mod-ndpi))
$(eval $(call KernelPackage,ipt-ndpi))
