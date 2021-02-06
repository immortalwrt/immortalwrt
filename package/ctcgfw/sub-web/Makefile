#
# Copyright (C) 2021 ImmortalWrt
# (https://project-openwrt.eu.org)
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=sub-web
PKG_VERSION:=1.0.0
PKG_RELEASE:=6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/CareyWang/sub-web.git
PKG_SOURCE_DATE:=2021-01-07
PKG_SOURCE_VERSION:=26d278800cc42025f8ceecff64b26f47b9fc2d14
PKG_MIRROR_HASH:=a83f49aeecfa086ad8da3503b1ff95f8505845588839ed569d22fb0ced417b9f

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_DEPENDS:=node/host node-yarn/host

include $(INCLUDE_DIR)/package.mk

define Package/sub-web
  SECTION:=net
  CATEGORY:=Network
  URL:=https://github.com/CareyWang/sub-web
  TITLE:=A WebUI for generating proxy subscription url
  DEPENDS:=+subconverter
  PKGARCH:=all
endef

define Package/sub-web/description
  Based on vue-cli and subconverter, for generating proxy subscription
  url automatically.
endef

define Build/Compile
	( \
		cd $(PKG_BUILD_DIR); \
		yarn install; \
		yarn build; \
	)
endef

define Package/sub-web/install
	$(INSTALL_DIR) $(1)/www
	$(CP) $(PKG_BUILD_DIR)/dist $(1)/www/sub-web
endef

$(eval $(call BuildPackage,sub-web))
