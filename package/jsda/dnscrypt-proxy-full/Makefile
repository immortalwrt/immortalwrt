#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dnscrypt-proxy-full
PKG_VERSION:=2.0.36
PKG_RELEASE:=1

ifeq ($(ARCH),i386)
	PKG_ARCH_DNSCRYPT:=linux_i386
	PKG_BUILD_DNSCRYPT:=linux-i386
endif
ifeq ($(ARCH),x86_64)
	PKG_ARCH_DNSCRYPT:=linux_x86_64
	PKG_BUILD_DNSCRYPT:=linux-x86_64
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_DNSCRYPT:=linux_mips
	PKG_BUILD_DNSCRYPT:=linux-mips
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_DNSCRYPT:=linux_mipsle
	PKG_BUILD_DNSCRYPT:=linux-mipsle
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_DNSCRYPT:=linux_arm
	PKG_BUILD_DNSCRYPT:=linux-arm
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_DNSCRYPT:=linux_arm64
	PKG_BUILD_DNSCRYPT:=linux-arm64
endif

PKG_SOURCE:=dnscrypt-proxy-$(PKG_ARCH_DNSCRYPT)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/DNSCrypt/dnscrypt-proxy/releases/download/$(PKG_VERSION)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A flexible DNS proxy, with support for modern encrypted DNS protocols such as DNSCrypt v2 and DNS-over-HTTPS.
	DEPENDS:=+ca-bundle
	URL:=https://github.com/jedisct1/dnscrypt-proxy/releases
endef

define Package/$(PKG_NAME)/description
  A flexible DNS proxy, with support for modern encrypted DNS protocols such as DNSCrypt v2 and DNS-over-HTTPS.
endef

define Build/Prepare
	tar -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	chmod 0755 $(PKG_BUILD_DIR)/$(PKG_BUILD_DNSCRYPT)/dnscrypt-proxy
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_BUILD_DNSCRYPT)/dnscrypt-proxy $(1)/usr/bin/dnscrypt-proxy
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
