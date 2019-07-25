#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Open App Filter Module
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-appfilter
PKG_VERSION:=1.0
PKG_RELEASE:=1

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for oaf
	PKGARCH:=all
	DEPENDS:=+oaf +appfilter
endef

include $(TOPDIR)/feeds/luci/luci.mk
# call BuildPackage - OpenWrt buildroot signature
