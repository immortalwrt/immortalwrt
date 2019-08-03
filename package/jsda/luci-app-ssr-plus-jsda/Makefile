include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus-jsda
PKG_VERSION:=1
PKG_RELEASE:=140

PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Simple_Ofbs \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_V2Ray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_haproxy

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks New Version"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server
	bool "Include Shadowsocks Server"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks
	bool "Include Shadowsocks Socks and Tunnel"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Simple_Obfs
	bool "Include Shadowsocks Simple Obfs Plugin"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_V2Ray
	bool "Include Shadowsocks V2Ray Plugin"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS
	bool "ChinaDNS"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy
	bool "dnscrypt-proxy-full"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder
	bool "dnsforwarder"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_haproxy
	bool "haproxy"
	default n
endef

define Package/luci-app-ssr-plus-jsda
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +wget +unzip \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
	+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server:shadowsocks-libev-ss-server \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks:shadowsocks-libev-ss-local \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Simple_Obfs:simple-obfs \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_V2Ray:v2ray-plugin \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS:openwrt_chinadns \
	+PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy:dnscrypt-proxy-full \
	+PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder:openwrt-dnsforwarder \
	+PACKAGE_$(PKG_NAME)_INCLUDE_haproxy:haproxy
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-ssr-plus-jsda/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	cp -pR ./po/zh-cn/ssr-plus.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/luci-app-ssr-plus-jsda/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-ssr-plus-jsda/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/shadowsocksr disable
     /etc/init.d/shadowsocksr stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-ssr-plus-jsda))
