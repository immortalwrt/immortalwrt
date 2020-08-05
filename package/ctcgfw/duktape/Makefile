#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=duktape
PKG_VERSION:=2.99.99-65f1e02
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/svaarala/duktape.git
PKG_SOURCE_VERSION:=65f1e02e580796929e92e91ba4776d4b0131beb5
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=python2

PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/duktape
	SECTION:=utils
	CATEGORY:=Utilities
	URL:=https://github.com/svaarala/duktape
	TITLE:=Duktape is an embeddable Javascript engine, with a focus on portability and compact footprint.
endef

define Package/libduktape
	$(call Package/duktape)
	SECTION:=lib
	CATEGORY:=Libraries
	TITLE+= (Libraries)
	VARIANT:=lib
endef

define Package/duktape/description
	Duktape is easy to integrate into a C/C++ project: add duktape.c,
	duktape.h, and duk_config.h to your build, and use the Duktape API
	to call ECMAScript functions from C code and vice versa.
endef

define Package/libduktape/description
	$(call Package/duktape/description)
endef

define Build/Compile
( \
  cd $(PKG_BUILD_DIR); \
  mkdir dist; \
  $(STAGING_DIR_HOSTPKG)/bin/pip2 install PyYAML; \
  $(STAGING_DIR_HOSTPKG)/bin/python2 util/dist.py; \
  cd dist/source/src; \
  $(TARGET_CC) -c -O3 -o duktape.o duktape.c; \
  $(TARGET_CC) -c -O3 -o duk_module_node.o -I. ../extras/module-node/duk_module_node.c; \
  $(TARGET_AR) cr libduktape.a duktape.o; \
  $(TARGET_AR) cr libduktape_module.a duk_module_node.o; \
  env $(call Build/Compile/Default) \
)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist/source/src/*.a $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist/source/src/duk*.h $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist/source/extras/module-node/duk_module_node.h $(1)/usr/include/
endef

define Package/duktape/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/build/duk $(1)/usr/bin/duk
endef

define Package/libduktape/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/dist/source/src/*.a $(1)/usr/lib/
endef

$(eval $(call BuildPackage,duktape))
$(eval $(call BuildPackage,libduktape))
