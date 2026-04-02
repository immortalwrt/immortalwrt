#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fortify-headers
PKG_VERSION:=3.0.1
PKG_RELEASE=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jvoisin/$(PKG_NAME).git
PKG_SOURCE_DATE:=2025-01-05
PKG_SOURCE_VERSION:=eec2b5e06bdb01d20f1a1125074a379e0b154d80
PKG_MIRROR_HASH:=f2ce18b031aecbf43d18142941ece7c90691a8b4e96fbb1d5af94f24e5572dad

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Compile
	true
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX="" DESTDIR="$(TOOLCHAIN_DIR)" install
endef

$(eval $(call HostBuild))
