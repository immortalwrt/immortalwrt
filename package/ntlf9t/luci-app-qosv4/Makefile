#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for QoSv4.
LUCI_DESCRIPTION:=An agent script that makes qosv4 configuration simple.
LUCI_DEPENDS:=+tc +iptables-mod-conntrack-extra +iptables-mod-conntrack-extra +iptables-mod-filter +iptables-mod-imq +iptables-mod-ipopt +iptables-mod-nat-extra +iptables-mod-imq +kmod-sched
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-qosv4
PKG_VERSION:=1.1f
PKG_RELEASE:=2

PKG_MAINTAINER:=qq 3341249

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
