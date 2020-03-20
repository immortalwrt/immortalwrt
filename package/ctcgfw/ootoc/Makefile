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
PKG_VERSION:=2.0.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ElonH/ootoc.git
PKG_MIRROR_HASH:=565aa99c09c70c964c16ef57b731932e9376067aa133256cad200854ea64b9d0
PKG_SOURCE_VERSION:=c100c430f9222fd3ff9af2ee26bacbb998367e76
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DOOTOC_TEST:BOOL=OFF

define Package/ootoc
	SECTION:=base
	CATEGORY:=Base system
	URL:=https://github.com/msgpack/ootoc
	TITLE:=opkg over tar over curl
	DEPENDS:=+libcurl +libyaml-cpp +libtar +spdlog
	MENU:=1
endef

define Package/ootoc/description
	opkg over tar over curl
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
endef

define Package/ootoc/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ootocCLI $(1)/usr/bin/
	$(INSTALL_BIN) ./files/ootoc.init $(1)/etc/init.d/ootoc
	$(INSTALL_DATA) ./files/ootoc.conf $(1)/etc/config/ootoc
endef

$(eval $(call BuildPackage,ootoc))
