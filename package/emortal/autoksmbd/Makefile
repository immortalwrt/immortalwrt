#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autoksmbd
PKG_VERSION:=1
PKG_RELEASE:=12

include $(INCLUDE_DIR)/package.mk

define Package/autoksmbd
  TITLE:=ksmbd autoconfig hotplug script.
  MAINTAINER:=Lean
  DEPENDS:=+luci-app-ksmbd +wsdd2
  PKGARCH:=all
endef

define Package/autoksmbd/description
  A hotplug script to config Ksmbd share automatically.
endef

define Build/Compile
endef

define Package/autoksmbd/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/block
	$(INSTALL_BIN) ./files/20-ksmbd $(1)/etc/hotplug.d/block/
endef

$(eval $(call BuildPackage,autoksmbd))
