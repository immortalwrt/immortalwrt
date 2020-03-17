include $(TOPDIR)/rules.mk

LUCI_TITLE:=Docker Manager interface for LuCI
LUCI_DEPENDS:=+luci-lib-docker +docker-ce +e2fsprogs +fdisk +ttyd
PKG_NAME:=luci-app-dockerman
PKG_VERSION:=v0.4.7
PKG_RELEASE:=leanmod
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-dockerman>
PKG_LICENSE:=AGPL-3.0

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
