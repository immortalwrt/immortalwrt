#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ngrokc
PKG_VERSION:=4.18
PKG_RELEASE:=20190217

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=Secure tunnels to localhost.
	DEPENDS:=+libc +libpthread +libopenssl +libstdcpp
	URL:=https://github.com/dosgo/ngrok-c.git
endef

define Package/$(PKG_NAME)/description
Secure tunnels to localhost.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
ifeq ($(ARCH),mipsel)
	$(INSTALL_BIN) ./files/7620n/ngrokc $(1)/usr/bin/
endif
ifeq ($(ARCH),mips)
	$(INSTALL_BIN) ./files/ar71xx/ngrokc $(1)/usr/bin/
endif
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ./files/x86/ngrokc $(1)/usr/bin/
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ./files/x86_64/ngrokc $(1)/usr/bin/
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ./files/arm/ngrokc $(1)/usr/bin/
endif
ifeq ($(ARCH),aarch64)
	$(INSTALL_BIN) ./files/armv7/ngrokc $(1)/usr/bin/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
