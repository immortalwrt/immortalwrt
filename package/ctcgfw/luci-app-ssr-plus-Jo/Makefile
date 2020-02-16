include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus-Jo
PKG_VERSION:=1.46
PKG_RELEASE:=8
PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_V2ray \
        CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Trojan \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ipt2socks \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_haproxy \
        CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_udpspeeder \
        CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_udp2raw-tunnel \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_privoxy \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs-server\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-client\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-server\
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks New Version"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server
	bool "Include Shadowsocks Server"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include ShadowsocksR Socks and Tunnel"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks
	bool "Include Shadowsocks Socks and Tunnel"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_ipt2socks
	bool "Include ipt2socks"
	default y	

config PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy
	bool "Include dnscrypt-proxy-full"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder
	bool "Include dnsforwarder"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS
	bool "Include chinadns"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_haproxy
	bool "Include haproxy"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_privoxy
	bool "Include privoxy http local"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs
	bool "Include simple-obfsl"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs-server
	bool "Include simple-obfs-server"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_udpspeeder
	bool "Include udpspeeder"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_udp2raw-tunnel
	bool "Include udp2raw-tunnel"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-client
	bool "Include GoQuiet-client"
	default y
	
config PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-server
	bool "Include GoQuiet-server"
	default y

config PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin
	bool "Include v2ray-plugin"
	default y
endef

define Package/luci-app-ssr-plus-Jo
 	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SS/SSR/V2Ray LuCI interface
	PKGARCH:=all
	DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +bash +pdnsd-alt +curl +wget +unzip \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
            +PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Server:shadowsocks-libev-ss-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:shadowsocksr-libev-ssr-local \
            +PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks_Socks:shadowsocks-libev-ss-local \
	    +PACKAGE_$(PKG_NAME)_INCLUDE_ipt2socks:ipt2socks \
            +PACKAGE_$(PKG_NAME)_INCLUDE_dnscrypt_proxy:dnscrypt-proxy-full \
            +PACKAGE_$(PKG_NAME)_INCLUDE_dnsforwarder:dnsforwarder \
            +PACKAGE_$(PKG_NAME)_INCLUDE_ChinaDNS:openwrt_chinadns \
            +PACKAGE_$(PKG_NAME)_INCLUDE_haproxy:haproxy \
            +PACKAGE_$(PKG_NAME)_INCLUDE_privoxy:privoxy \
            +PACKAGE_$(PKG_NAME)_INCLUDE_udpspeeder:speederv2-tunnel \
            +PACKAGE_$(PKG_NAME)_INCLUDE_udp2raw-tunnel:udp2raw-tunnel \
            +PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs:simple-obfs \
            +PACKAGE_$(PKG_NAME)_INCLUDE_simple-obfs-server:simple-obfs-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-client:gq-client \
	    +PACKAGE_$(PKG_NAME)_INCLUDE_GoQuiet-server:gq-server \
            +PACKAGE_$(PKG_NAME)_INCLUDE_v2ray-plugin:v2ray-plugin
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-ssr-plus-Jo/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	cp -pR ./po/zh-cn/ssr-plus.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/ssr-plus.zh-cn.lmo
endef

define Package/luci-app-ssr-plus-Jo/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
	chmod 755 /etc/init.d/shadowsocksr >/dev/null 2>&1
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-ssr-plus-Jo/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/shadowsocksr disable
     /etc/init.d/shadowsocksr stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-ssr-plus-Jo))
