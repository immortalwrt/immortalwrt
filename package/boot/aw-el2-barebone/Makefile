# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=aw-el2-barebone
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/Icenowy/aw-el2-barebone
PKG_SOURCE_DATE:=2020-03-30
PKG_SOURCE_VERSION:=b40bcb75cfa0acfaea84a5234fb225240b32b749
PKG_MIRROR_HASH:=aeb70287e0b35736f806ed20bad84d6a6cae977f70cad63ce8139cdfcc181108

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define Package/aw-el2-barebone
  SECTION:=boot
  CATEGORY:=Boot Loaders
  TITLE:=Allwinner SoCs' 64-bit EL2 barebone
  DEPENDS:=@TARGET_sunxi_cortexa53
endef

MAKE_VARS = \
	CROSS_COMPILE="$(TARGET_CROSS)"

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/el2-bb.bin $(STAGING_DIR_IMAGE)/hyp.bin
endef

define Package/aw-el2-barebone/install
endef

$(eval $(call BuildPackage,aw-el2-barebone))
