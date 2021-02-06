#
# Copyright (C) 2021 ImmortalWrt
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=subweb
PKG_VERSION:=20200702
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lzdnico/subweb.git
PKG_MIRROR_HASH:=61855d9412056f2f0eb588840bb35b9b47dea97db887e47c15c52fe897f8b7bd
PKG_SOURCE_VERSION:=9acc47d2e9cda1b16d7fe184a16f528848467810
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=(WebUI) Utility to convert between various subscription format.
	URL:=https://github.com/lzdnico/subweb
	DEPENDS:=+python3 +python3-flask +python3-flask-restful +python3-requests +subconverter
endef

define Package/$(PKG_NAME)/description
	(WebUI) Utility to convert between various subscription format.
endef

define Build/Configure
	$(RM) $(PKG_BUILD_DIR)/config/{subconverter,README.md,README-cn.md}
	$(RM) -r $(PKG_BUILD_DIR)/api/__pycache__
	$(SED) 's/root/etc/g;s/ -ef//g' $(PKG_BUILD_DIR)/subweb.sh
	$(SED) 's/10010/25500/g' $(PKG_BUILD_DIR)/api/aff.py
endef

define Build/Compile
endef

define Build/Install
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/subweb
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/subweb.sh $(1)/etc/subweb/
	$(CP) $(PKG_BUILD_DIR)/{api,config,static,templates,api.py} $(1)/etc/subweb/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/subweb.init $(1)/etc/init.d/subweb
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
