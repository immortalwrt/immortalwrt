#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

ifeq ($(ARCH),x86_64)
	PKG_ARCH_BROOK:=
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_BROOK:=_linux_mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_BROOK:=_linux_mips
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_BROOK:=_linux_386
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_BROOK:=_linux_arm7
endif
ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_BROOK:=_linux_arm6
endif
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_BROOK:=_linux_arm5
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_BROOK:=_linux_arm64
endif

PKG_NAME:=brook
PKG_VERSION:=20200201
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=brook$(PKG_ARCH_BROOK)
PKG_SOURCE_URL:=https://github.com/txthinking/brook/releases/download/v$(PKG_VERSION)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Brook is a cross-platform proxy software
	DEPENDS:=
	URL:=https://github.com/txthinking/brook
endef

define Package/$(PKG_NAME)/description
  Brook is a cross-platform proxy software
endef

define Build/Prepare
	cp $(DL_DIR)/$(PKG_SOURCE) $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brook$(PKG_ARCH_BROOK) $(1)/usr/bin/brook
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
