#
# Copyright (C) 2017 openwrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
# Copyright (C) 2018 openwrt-brook-tproxy
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-brook
PKG_VERSION:=1.0.2
PKG_RELEASE:=2

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=WouldChar

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/Default
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Brook LuCI interface
	URL:=https://github.com/WouldChar/openwrt-brook-tproxy
	DEPENDS:=$(1)
	PKGARCH:=all
endef

Package/$(PKG_NAME) = $(call Package/$(PKG_NAME)/Default,+ipset +ip +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +brook)

define Package/$(PKG_NAME)/description
	LuCI Support for Brook.
endef

define Build/Configure
endef

define Build/Compile
endef

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

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/brook.lua $(1)/usr/lib/lua/luci/controller/brook.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/brook
	$(INSTALL_DATA) ./luasrc/model/cbi/brook/*.lua $(1)/usr/lib/lua/luci/model/cbi/brook/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/brook
	$(INSTALL_DATA) ./luasrc/view/brook/*.htm $(1)/usr/lib/lua/luci/view/brook/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-brook $(1)/etc/uci-defaults/luci-brook
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./root/etc/config/brook $(1)/etc/config/brook
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/brook $(1)/etc/init.d/brook
	$(INSTALL_DIR) $(1)/etc/dnsmasq.brook
	$(INSTALL_DATA) ./root/etc/dnsmasq.brook/gfw_list.conf $(1)/etc/dnsmasq.brook/gfw_list.conf
	$(INSTALL_DATA) ./root/etc/dnsmasq.brook/custom_list.conf $(1)/etc/dnsmasq.brook/custom_list.conf
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./root/etc/china_ip.txt $(1)/etc/china_ip.txt
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./root/usr/bin/brook-gfw $(1)/usr/bin/brook-gfw
	$(INSTALL_BIN) ./root/usr/bin/brook-ad $(1)/usr/bin/brook-ad
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/brook.po $(1)/usr/lib/lua/luci/i18n/brook.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
