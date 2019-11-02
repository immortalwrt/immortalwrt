#
# Copyright (C) 2018 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smartdns
PKG_VERSION:=2019.11.02
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
    SECTION:=net
    CATEGORY:=Network
    TITLE:=smartdns
endef

define Package/$(PKG_NAME)/description
  smartdns
endef

define Build/Compile
endef

ifeq ($(ARCH),i386)
	PKG_ARCH_SMARTDNS:=x86
endif
ifeq ($(ARCH),x86_64)
	PKG_ARCH_SMARTDNS:=x86_64
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_SMARTDNS:=mips
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_SMARTDNS:=mipsel
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_SMARTDNS:=arm
endif
ifeq ($(ARCH),arm64)
	PKG_ARCH_SMARTDNS:=arm64
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_SMARTDNS:=arm64
endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/usr/sbin/$(PKG_ARCH_SMARTDNS) $(1)/usr/sbin/smartdns
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/smartdns $(1)/etc/config/smartdns
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/smartdns $(1)/etc/init.d/smartdns
	$(INSTALL_DIR) $(1)/etc/smartdns
	$(INSTALL_CONF) ./files/etc/smartdns/address.conf $(1)/etc/smartdns/address.conf
	$(INSTALL_CONF) ./files/etc/smartdns/blacklist-ip.conf $(1)/etc/smartdns/blacklist-ip.conf
	$(INSTALL_CONF) ./files/etc/smartdns/custom.conf $(1)/etc/smartdns/custom.conf
	$(INSTALL_CONF) ./files/etc/smartdns/smartdns.conf $(1)/etc/smartdns/smartdns.conf
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
