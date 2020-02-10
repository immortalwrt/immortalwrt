#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=UnblockNeteaseMusic-Go
PKG_VERSION:=snapshot
PKG_RELEASE:=r35-07453d1

ifeq ($(ARCH),x86_64)
	PKG_ARCH_UnblockNeteaseMusic:=linux_amd64
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_UnblockNeteaseMusic:=linux_386
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_UnblockNeteaseMusic:=linux_mipsle
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_UnblockNeteaseMusic:=linux_mips
endif
ifeq ($(ARCH),arm)
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_UnblockNeteaseMusic:=linux_arm5
else ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_UnblockNeteaseMusic:=linux_arm5
else
	PKG_ARCH_UnblockNeteaseMusic:=linux_arm7
endif
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_UnblockNeteaseMusic:=linux_arm64
endif

PKG_SOURCE:=UnblockNeteaseMusic_$(PKG_ARCH_UnblockNeteaseMusic).tar.gz
PKG_SOURCE_URL:=https://package.project-openwrt.eu.org/utils/UnblockNeteaseMusic-Go/$(PKG_RELEASE)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Revive unavailable songs for Netease Cloud Music (Golang)
	URL:=https://github.com/cnsilvan/UnblockNeteaseMusic
endef

define Build/Prepare
	tar -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	chmod 0755 $(PKG_BUILD_DIR)/UnblockNeteaseMusic
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/UnblockNeteaseMusic $(1)/usr/bin/UnblockNeteaseMusic
	$(INSTALL_DIR) $(1)/usr/share/UnblockNeteaseMusic-Go
	$(INSTALL_CONF) ./cert/server.crt $(1)/usr/share/UnblockNeteaseMusic-Go/server.crt
	$(INSTALL_CONF) ./cert/server.key $(1)/usr/share/UnblockNeteaseMusic-Go/server.key
	
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
