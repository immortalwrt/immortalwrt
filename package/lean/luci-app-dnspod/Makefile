include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dnspod
PKG_VERSION=1.0
PKG_RELEASE:=1
LUCI_TITLE:=Auto DNS for dnspod
LUCI_DEPENDS:=+curl +libcurl
LUCI_PKGARCH:=all
LUCI_DESCRIPTION=auto get pub ip,dnspod ddns,must include curl with ssl

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
