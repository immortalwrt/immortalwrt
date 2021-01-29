####
 #  File: /Makefile
 #  Project: ootoc
 #  File Created: Saturday, 7th March 2020 5:44:11 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2020 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=ootoc
PKG_VERSION:=2.2.3
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ElonH/ootoc.git
PKG_SOURCE_DATE:=2020-03-21
PKG_SOURCE_VERSION:=8e7495e49018a100043b2e013b65b589900ddabe
PKG_MIRROR_HASH:=9fe7b360241a070cfe623cd59330b7305a59d329056bbf04666f1840e3ec8988

PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DOOTOC_TEST:BOOL=OFF

define Package/ootoc
	SECTION:=base
	CATEGORY:=Base system
	URL:=https://github.com/msgpack/ootoc
	TITLE:=opkg over tar over curl
	DEPENDS:=+libcurl +libyaml-cpp +libtar +spdlog
endef

define Package/ootoc/description
	opkg over tar over curl
endef

define Package/ootoc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ootocCLI $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_CONF) ./files/ootoc.conf $(1)/etc/config/ootoc
	$(INSTALL_BIN) ./files/ootoc.init $(1)/etc/init.d/ootoc
endef

$(eval $(call BuildPackage,ootoc))