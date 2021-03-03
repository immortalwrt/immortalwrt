#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=shadowsocksr-libev
PKG_VERSION:=2.5.6
PKG_RELEASE:=6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/shadowsocksrr/shadowsocksr-libev
PKG_SOURCE_DATE:=2018-03-07
PKG_SOURCE_VERSION:=d63ff863800a5645aca4309d5dd5962bd1e95543
PKG_MIRROR_HASH:=34308ed827a5dd4f4e35619914102d55b00604faa44fda051d1d25fb4a319325

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE

PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/shadowsocksr-libev/template
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=shadowsocksr-libev
	URL:=https://github.com/shadowsocksrr/shadowsocksr-libev
	DEPENDS:=+libev +libsodium +libopenssl +libpthread +libpcre +libudns +zlib
endef

define Package/shadowsocksr-libev-ssr-check
	$(call Package/shadowsocksr-libev/template,$(1))
	TITLE+= ssr-check
endef

define Package/shadowsocksr-libev-ssr-local
	$(call Package/shadowsocksr-libev/template,$(1))
	TITLE+= ssr-local
endef

define Package/shadowsocksr-libev-ssr-nat
	$(call Package/shadowsocksr-libev/template,$(1))
	TITLE+= ssr-nat
endef

define Package/shadowsocksr-libev-ssr-redir
	$(call Package/shadowsocksr-libev/template,$(1))
	TITLE+= ssr-redir
endef

define Package/shadowsocksr-libev-ssr-server
	$(call Package/shadowsocksr-libev/template,$(1))
	TITLE+= ssr-server
endef

CONFIGURE_ARGS += \
	--disable-documentation \
	--disable-ssp \
	--disable-assert \
	--enable-system-shared-lib

TARGET_CFLAGS += -flto
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

define Package/shadowsocksr-libev/install/template
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ss-$(2) $(1)/usr/bin/ssr-$(2)
endef

define Package/shadowsocksr-libev-ssr-check/install
	$(call Package/shadowsocksr-libev/install/template,$(1),check)
endef

define Package/shadowsocksr-libev-ssr-local/install
	$(call Package/shadowsocksr-libev/install/template,$(1),local)
	$(LN) ssr-local $(1)/usr/bin/ssr-tunnel
endef

define Package/shadowsocksr-libev-ssr-nat/install
	$(call Package/shadowsocksr-libev/install/template,$(1),nat)
endef

define Package/shadowsocksr-libev-ssr-redir/install
	$(call Package/shadowsocksr-libev/install/template,$(1),redir)
endef

define Package/shadowsocksr-libev-ssr-server/install
	$(call Package/shadowsocksr-libev/install/template,$(1),server)
endef

$(eval $(call BuildPackage,shadowsocksr-libev-ssr-check))
$(eval $(call BuildPackage,shadowsocksr-libev-ssr-local))
$(eval $(call BuildPackage,shadowsocksr-libev-ssr-nat))
$(eval $(call BuildPackage,shadowsocksr-libev-ssr-redir))
$(eval $(call BuildPackage,shadowsocksr-libev-ssr-server))
