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
PKG_VERSION:=0.0.1
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ElonH/ootoc.git
PKG_MIRROR_HASH:=1618751ed55f5b0b54ceddca43b92f55c8ff8d60bec22616d7a2d2303f7d2328
PKG_SOURCE_VERSION:=8fb0c485eb03c6e97b5be002b731cbabe65ea536
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=httplib/host

PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DOOTOC_TEST:BOOL=OFF -DOOTOC_BUILDIN_LIBTAR:BOOL=OFF -DOOTOC_BUILDIN_HTTPLIB:BOOL=OFF

define Package/ootoc
	SECTION:=base
	CATEGORY:=Base system
	URL:=https://github.com/msgpack/ootoc
	TITLE:=opkg over tar over curl
	DEPENDS:=+libootoc
	MENU:=1
endef

define Package/libootoc
	URL:=https://github.com/msgpack/ootoc
	TITLE:=opkg over tar over curl
	DEPENDS:=+libcurl +libyaml-cpp +libtar
	SECTION:=lib
	CATEGORY:=Libraries
	VARIANT:=lib
endef

define Package/ootoc/description
	opkg over tar over curl
endef

define Package/libootocc/description
	$(call Package/ootoc/description)
endef

define Package/ootoc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ootocCLI $(1)/usr/bin/
endef

$(eval $(call BuildPackage,ootoc))
$(eval $(call BuildPackage,libootoc))
