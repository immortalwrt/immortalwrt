#
# Copyright (C) 2014-2018 OpenWrt-dist
# Copyright (C) 2019 [CTCGFW] Project OpenWRT
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=filebrowser
PKG_VERSION:=snapshot
PKG_RELEASE:=6ce44f7

PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=[CTCGFW] Project OpenWRT

ifeq ($(ARCH),x86_64)
	PKG_ARCH_filebrowser:=linux_amd64
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_filebrowser:=linux_386
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_filebrowser:=linux_mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_filebrowser:=linux_mips
endif
ifeq ($(ARCH),arm)
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_filebrowser:=linux_arm5
else ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_filebrowser:=linux_arm5
else
	PKG_ARCH_filebrowser:=linux_arm7
endif
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_filebrowser:=linux_arm64
endif

PKG_SOURCE:=filebrowser_$(PKG_ARCH_filebrowser).tar.gz
PKG_SOURCE_URL:=https://package.project-openwrt.eu.org/utils/filebrowser/$(PKG_RELEASE)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=OpenWrt Support for FileBrowser.
	DEPENDS:=@(aarch64||arm||i386||mips||mipsel||x86_64)
	URL:=https://github.com/filebrowser/filebrowser
endef

define Package/$(PKG_NAME)/description
OpenWrt Support for FileBrowser.
endef

define Build/Prepare
	tar -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	chmod 0755 $(PKG_BUILD_DIR)/filebrowser
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/filebrowser $(1)/usr/bin/filebrowser
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
