#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webdav-go
PKG_VERSION:=3.2.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/hacdias/webdav.git
PKG_SOURCE_DATE:=2020-12-18
PKG_SOURCE_VERSION:=fca4e548391e8198d0a651a3f22af1ceee966d5c
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=510999ec95faef0f8d9d6e5d5f7813b10cc2432c1c859e342a7deabe8de3af2b

PKG_LICENSE:=MIT
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0
GO_PKG:=github.com/hacdias/webdav
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=File Transfer
	TITLE:=A powerful WebDav server written in Golang.
	URL:=https://github.com/hacdias/webdav
	DEPENDS:=$(GO_ARCH_DEPENDS) 
endef

define Package/$(PKG_NAME)/description
webdav command line interface is really easy to use so you can easily create a WebDAV server for your own user.
By default, it runs on a random free port and supports JSON, YAML and TOML configuration.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/webdav $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
