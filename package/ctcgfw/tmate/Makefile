#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=tmate
PKG_VERSION:=2.4.0-cbec43f
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tmate-io/tmate.git
PKG_SOURCE_VERSION:=cbec43f56dfb48c2fb6e00faa2cb85443d4b7d8f
PKG_MIRROR_HASH:=a553810a740bed3770ff94237a982c0dbddaf2efec61c61f3660f6674e8320b5
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=ISC
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=msgpack-c ncurses

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=SSH
  TITLE:=Instant Terminal Sharing
  URL:=https://tmate.io
  DEPENDS:=+libpthread +libevent2 +libssh +libmsgpack-c +libncurses
endef

CONFIGURE_ARGS+=--oldincludedir=$(STAGING_DIR)

CONFIGURE_VARS+=LIBSSH_CFLAGS="-I$(STAGING_DIR)/usr/include " LIBSSH_LIBS="-lssh"

define Package/$(PKG_NAME)/description
	Tmate is a fork of tmux. It provides an instant pairing solution.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tmate $(1)/usr/bin/tmate
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
