include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for koolproxyR
LUCI_DESCRIPTION:=This package contains LuCI configuration pages for koolproxy.
LUCI_DEPENDS:=+koolproxy +wget
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-koolproxyR
PKG_VERSION:=3.8.4
PKG_RELEASE:=6-20200413

PKG_MAINTAINER:=panda-mute <wxuzju@gmail.com>
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

