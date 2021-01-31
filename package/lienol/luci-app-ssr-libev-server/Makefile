# Copyright (C) 2018-2020 Lienol <lawlienol@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for SSR Libev Server
LUCI_DEPENDS:=+libsodium +shadowsocksr-libev-ssr-server
LUCI_PKGARCH:=all
PKG_VERSION:=1
PKG_RELEASE:=8-20200326

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
