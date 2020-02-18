#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=jpcre2
PKG_VERSION:=10.31.03

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jpcre2/jpcre2.git
PKG_SOURCE_VERSION:=709ec0e866c3ae3ac61e5683b7f9b3b6feaac3f6
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_HASH:=22deb97515e48adf80383ca691229b83d13d8850f6e8b98710426b24955ae5de

HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/host-build.mk

HOSTCC :=g++
HOST_CFLAGS +=-g -O2

define Host/Configure
	(cd $(HOST_BUILD_DIR); \
	CC="$(HOSTCC)" \
	CFLAGS="$(HOST_CFLAGS)" \
	./configure --prefix=$(STAGING_DIR_HOST) \
	);
	$(call Host/Configure/Default)
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
endef

define Host/Clean
	$(MAKE) -C $(HOST_BUILD_DIR) uninstall
	rm -rf $(HOST_BUILD_DIR)
endef

$(eval $(call HostBuild))
