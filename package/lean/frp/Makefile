#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=frp
PKG_VERSION:=0.31.2
PKG_RELEASE:=1

ifeq ($(ARCH),mipsel)
	FRPC_ARCH:=mipsle
endif
ifeq ($(ARCH),mips)
	FRPC_ARCH:=mips
endif
ifeq ($(ARCH),i386)
	FRPC_ARCH:=386
endif
ifeq ($(ARCH),x86_64)
	FRPC_ARCH:=amd64
endif
ifeq ($(ARCH),arm)
	FRPC_ARCH:=arm
endif
ifeq ($(ARCH),aarch64)
	FRPC_ARCH:=arm64
endif

PKG_LICENSE:=Apache-2.0

PKG_SOURCE_URL:=https://github.com/fatedier/frp/releases/download/v$(PKG_VERSION)
PKG_SOURCE:=frp_$(PKG_VERSION)_linux_$(FRPC_ARCH).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/frp_$(PKG_VERSION)_linux_$(FRPC_ARCH)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/default
	SECTION:=net
	CATEGORY:=Network
	TITLE:=FRP
	URL:=https://github.com/fatedier/frp/releases
endef

define Package/$(PKG_NAME)s
	$(call Package/$(PKG_NAME)/default)
	TITLE+= (Server)
	VARIANT:=s
endef

define Package/$(PKG_NAME)c
	$(call Package/$(PKG_NAME)/default)
	TITLE+= (Client)
	VARIANT:=c
endef

define Package/$(PKG_NAME)/description
frp is a fast reverse proxy to help you expose a local server behind a NAT or firewall to the internet
endef

define Package/$(PKG_NAME)s/description
	$(call Package/$(PKG_NAME)/description)
endef

define Package/$(PKG_NAME)c/description
	$(call Package/$(PKG_NAME)/description)
endef

define Build/Prepare
	$(PKG_UNPACK)
endef

define Build/Configure
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(PKG_BUILD_DIR)/frps
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(PKG_BUILD_DIR)/frpc
endef

define Build/Compile
endef

define Package/$(PKG_NAME)s/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/frps $(1)/usr/bin/
endef

define Package/$(PKG_NAME)c/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/frpc $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)s))
$(eval $(call BuildPackage,$(PKG_NAME)c))
