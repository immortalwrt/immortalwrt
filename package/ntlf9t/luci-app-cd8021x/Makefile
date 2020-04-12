include $(TOPDIR)/rules.mk

LUCI_TITLE:=a wired IEEE 802.1x client for Luci
LUCI_DESCRIPTION:=This package contains LuCI configuration pages for wired IEEE 802.1x authentication.
LUCI_DEPENDS:=+wpad
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-cd8021x
PKG_VERSION:=1.2.2
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=max0y <askmaxwork@gmail.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
