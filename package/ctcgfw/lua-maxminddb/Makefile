#
# Copyright (C) 2020 jerryk <jerrykuku@qq.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lua-maxminddb
PKG_VERSION:=0.2
PKG_RELEASE:=1
PKG_MAINTAINER:=fabled
PKG_LICENSE:=MIT

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MIRROR_HASH:=8aff0958546e4082d48854fdeb32f4fd1ef6a9f78ef066a1886f4d9569898d0e
PKG_SOURCE_URL:=https://github.com/fabled/lua-maxminddb.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=93da9f4e6c814c3a23044dd2cdd22d4a6b4f665b
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/lua-maxminddb
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua-Maxminddb
  URL:=https://github.com/jerrykuku/lua-maxminddb
  DEPENDS:=+lua +libmaxminddb
endef

define Package/lua-maxminddb/description
  libmaxminddb bindings for lua
endef

TARGET_CFLAGS += $(FPIC)
# add make variable overrides here
MAKE_FLAGS +=

define Build/Configure
endef


define Package/lua-maxminddb/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/maxminddb.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,lua-maxminddb))