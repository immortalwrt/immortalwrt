#
# Copyright (C) 2017 openwrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
# Copyright (C) 2018 openwrt-brook-tproxy
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Brook LuCI interface
LUCI_DESCRIPTION:=LuCI Support for Brook.
LUCI_DEPENDS:=+ipset +ip +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +brook
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-brook
PKG_VERSION:=1.0.2
PKG_RELEASE:=2

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=WouldChar

define Package/$(PKG_NAME)/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Removing rc.d symlink for brook"
	/etc/init.d/brook disable
	/etc/init.d/brook stop
	echo "Removing firewall rule for brook"
	uci -q batch <<-EOF >/dev/null
		delete firewall.brook
		commit firewall
EOF
	sed -i '/conf-dir/d' /etc/dnsmasq.conf
	/etc/init.d/dnsmasq restart

fi
exit 0
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh

if [ -z "$${IPKG_INSTROOT}" ]; then
	uci -q batch <<-EOF >/dev/null
		delete firewall.brook
		set firewall.brook=include
		set firewall.brook.type=script
		set firewall.brook.path=/var/etc/brook.include
		set firewall.brook.reload=1
		commit firewall
EOF
	( . /etc/uci-defaults/luci-brook ) && rm -f /etc/uci-defaults/luci-brook
	chmod 755 /etc/init.d/brook >/dev/null 2>&1
	/etc/init.d/brook enable >/dev/null 2>&1
fi
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
