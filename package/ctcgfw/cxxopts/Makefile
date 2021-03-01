#
# Copyright (C) 2021 ImmortalWrt
# (https://immortalwrt.org)
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=cxxopts
PKG_VERSION:=2.2.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/jarro2783/cxxopts/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=984aa3c8917d649b14d7f6277104ce38dd142ce378a9198ec926f03302399681

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DCXXOPTS_ENABLE_INSTALL=ON \
	-DCXXOPTS_BUILD_EXAMPLES=OFF \
	-DCXXOPTS_BUILD_TESTS=OFF

define Package/cxxopts
  SECTION:=lib
  CATEGORY:=Libraries
  URL:=https://github.com/jarro2783/cxxopts
  TITLE:=Lightweight C++ command line option parser
  DEPENDS:=+libc
endef

define Package/cxxopts/description
  This is a lightweight C++ option parser library, supporting the standard GNU style syntax for options.
endef

$(eval $(call BuildPackage,cxxopts))
