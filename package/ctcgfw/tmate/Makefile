#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tmate
PKG_VERSION:=2.4.0
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tmate-io/tmate.git
PKG_SOURCE_DATE:=2020-04-25
PKG_SOURCE_VERSION:=cbec43f56dfb48c2fb6e00faa2cb85443d4b7d8f
PKG_MIRROR_HASH:=8622869c8acfa147275d4658fbc2c9afdc9dde45d5d07fc4a4cb51ec740a8ac7

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_DEPENDS:=ncurses
PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tmate
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=SSH
  TITLE:=Instant Terminal Sharing
  URL:=https://tmate.io
  DEPENDS:=+libpthread +libevent2 +libssh +libmsgpack-c +libncurses
endef

CONFIGURE_ARGS+= \
	--oldincludedir=$(STAGING_DIR)

CONFIGURE_VARS+= \
	LIBSSH_CFLAGS="-I$(STAGING_DIR)/usr/include" \
	LIBSSH_LIBS="-lssh"

define Package/tmate/description
	Tmate is a fork of tmux. It provides an instant pairing solution.
endef

define Package/tmate/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tmate $(1)/usr/bin/tmate
endef

$(eval $(call BuildPackage,tmate))
