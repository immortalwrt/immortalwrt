#
# Copyright (C) 2018 OpenWrt.org
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smartdns
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
    SECTION:=net
    CATEGORY:=Network
    TITLE:=smartdns
    URL:=
    DEPENDS:=+libopenssl
endef

define Package/$(PKG_NAME)/description
  smartdns
endef

define Build/Compile
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ./files/usr/sbin/x86 $(1)/usr/sbin/smartdns
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ./files/usr/sbin/x86_64 $(1)/usr/sbin/smartdns
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ./files/usr/sbin/arm $(1)/usr/sbin/smartdns
endif
ifeq ($(ARCH),aarch64)
	$(INSTALL_BIN) ./files/usr/sbin/aarch64 $(1)/usr/sbin/smartdns
endif
ifeq ($(ARCH),mips)
	$(INSTALL_BIN) ./files/usr/sbin/mips $(1)/usr/sbin/smartdns
endif
ifeq ($(ARCH),mipsel)
	$(INSTALL_BIN) ./files/usr/sbin/mipsel $(1)/usr/sbin/smartdns
endif
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
