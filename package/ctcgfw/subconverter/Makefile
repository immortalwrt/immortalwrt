#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=subconverter
PKG_VERSION:=0.6.3-2abbd90
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tindy2013/subconverter.git
PKG_SOURCE_VERSION:=2abbd901e4211a771704aaed336cfb1a80b26cf0
PKG_MIRROR_HASH:=de349763b748e80b076c1ebed87c5df80e3aa0fcead472bde768d205be5084b1
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=duktape jpcre2 rapidjson

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Utility to convert between various subscription format.
	URL:=https://github.com/tindy2013/subconverter
	DEPENDS:=+libpthread +libstdcpp +libevent2 +libyaml-cpp +libpcre2 +libcurl +libopenssl +zlib
endef

define Package/$(PKG_NAME)/description
	Utility to convert between various subscription format.
endef

TARGET_CXXFLAGS := $(FPIC) $(filter-out -O%,$(TARGET_CXXFLAGS)) -O3
TARGET_CXXFLAGS += -Wall -Wextra -ffunction-sections -fdata-sections -flto
TARGET_LDFLAGS += -Wl,--gc-sections -flto

CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/subconverter $(1)/usr/bin/subconverter
	$(INSTALL_DIR) $(1)/etc/subconverter
	$(CP) $(PKG_BUILD_DIR)/base/* $(1)/etc/subconverter/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/subconverter.init $(1)/etc/init.d/subconverter
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
