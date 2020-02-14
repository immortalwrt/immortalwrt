include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ssr-plus
PKG_RELEASE:=140
PKG_VERSION:=1

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks
	bool "Include Shadowsocks New Version"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_Simple_obfs
        bool "Include Shadowsocks Simple-obfs Plugin"
        default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin
        bool "Include Shadowsocks V2ray Plugin"
        default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_V2ray
	bool "Include V2ray"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y if x86_64

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks
	bool "Include Socks Sever"
	default y if x86_64
endef

LUCI_TITLE:=SS/SSR/V2Ray/Trojan LuCI interface
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+shadowsocksr-libev-alt +ipset +ip-full +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +pdnsd-alt +wget +lua \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Shadowsocks:shadowsocks-libev-ss-redir \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Simple_obfs:simple-obfs \
	+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray_plugin:v2ray-plugin \
	+PACKAGE_$(PKG_NAME)_INCLUDE_V2ray:v2ray \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-server \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Socks:srelay

define Package/luci-app-ssr-plus/conffiles
/etc/ssr_ip
/etc/dnsmasq.ssr/gfw_list.conf
endef

include $(TOPDIR)/feeds/luci/luci.mk

define Package/luci-app-ssr-plus/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-ssr-plus ) && rm -f /etc/uci-defaults/luci-ssr-plus
	rm -f /tmp/luci-indexcache
	/etc/init.d/shadowsocksr enable >/dev/null 2>&1
fi
exit 0
endef

define Package/luci-app-ssr-plus/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	/etc/init.d/shadowsocksr disable
	/etc/init.d/shadowsocksr stop
fi
exit 0
endef

# call BuildPackage - OpenWrt buildroot signature
