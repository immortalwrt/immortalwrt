include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-vssr
PKG_VERSION:=1.22
PKG_RELEASE:=20210227

PKG_CONFIG_DEPENDS:= \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Trojan \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Xray \
	CONFIG_PACKAGE_$(PKG_NAME)_INCLUDE_Xray_plugin

LUCI_TITLE:=A New SS/SSR/Xray/Trojan LuCI interface
LUCI_PKGARCH:=all
LUCI_DEPENDS:= \
	+bash +coreutils +coreutils-base64 +dnsmasq-full +ip-full +ipset +iptables-mod-tproxy \
	+lua +lua-maxminddb +luasocket +pdnsd-alt +shadowsocksr-libev-ssr-check +shadowsocksr-libev-ssr-redir \
	+shadowsocks-libev-ss-local +shadowsocksr-libev-ssr-local +shadowsocks-libev-ss-redir +simple-obfs +wget-ssl \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun:kcptun-client \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server:shadowsocksr-libev-ssr-server \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:trojan \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Trojan:ipt2socks \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Xray:xray-core \
	+PACKAGE_$(PKG_NAME)_INCLUDE_Xray_plugin:xray-plugin

define Package/$(PKG_NAME)/config

config PACKAGE_$(PKG_NAME)_INCLUDE_Kcptun
	bool "Include Kcptun"
	default n

config PACKAGE_$(PKG_NAME)_INCLUDE_ShadowsocksR_Server
	bool "Include ShadowsocksR Server"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Trojan
	bool "Include Trojan"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Xray
	bool "Include Xray"
	default y if i386||x86_64||arm||aarch64

config PACKAGE_$(PKG_NAME)_INCLUDE_Xray_plugin
	bool "Include Shadowsocks Xray Plugin"
	default y if i386||x86_64||arm||aarch64
endef

define Package/$(PKG_NAME)/conffiles
/etc/vssr/
/etc/config/vssr
endef

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
