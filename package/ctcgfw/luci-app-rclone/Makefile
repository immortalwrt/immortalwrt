####
 #  File: /Makefile
 #  Project: luci-app-rclone
 #  File Created: Wednesday, 9th October 2019 1:39:36 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2019 [ElonH]
####

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Rclone
LUCI_DEPENDS:=+rclone +rclone-webui-react
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-rclone
PKG_VERSION:=1.0.7
PKG_RELEASE:=2
PKG_LICENSE:=GPLv3.0+
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
