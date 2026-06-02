#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/target.mk

PKG_NAME:=fortify-headers
PKG_VERSION:=3.0.2
PKG_RELEASE=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jvoisin/$(PKG_NAME).git
PKG_SOURCE_DATE:=2026-05-10
PKG_SOURCE_VERSION:=e7c86620bb0c0f8b868d3e4c8dcdebeeffb99631
PKG_MIRROR_HASH:=a4aab14c56eb00239cbd61ac65b3e778cf29ec82f06116ba343a50552882f587

include $(INCLUDE_DIR)/toolchain-build.mk

define Host/Compile
	true
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX="" DESTDIR="$(TOOLCHAIN_DIR)" install
endef

$(eval $(call HostBuild))
