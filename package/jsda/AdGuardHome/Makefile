#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=AdGuardHome
PKG_VERSION:=v0.99.2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=AdGuardHome
	DEPENDS:=
	URL:=https://github.com/AdguardTeam/AdGuardHome/releases
endef

define Package/$(PKG_NAME)/description
	AdGuardHome
endef

ifeq ($(ARCH),x86_64)
	PKG_ARCH_AdGuardHome:=linux_amd64
	ADG_CONF_NAME:=big_space
endif
ifeq ($(ARCH),i386)
	PKG_ARCH_AdGuardHome:=linux_386
	ADG_CONF_NAME:=big_space
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_AdGuardHome:=linux_mipsle
	ADG_CONF_NAME:=small_flash
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_AdGuardHome:=linux_mips
	ADG_CONF_NAME:=small_flash
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_AdGuardHome:=linux_arm
	ADG_CONF_NAME:=big_space
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_AdGuardHome:=linux_arm64
	ADG_CONF_NAME:=big_space
endif

define Build/Prepare
	[ ! -e "$(PKG_BUILD_DIR)/AdGuardHome_$(PKG_ARCH_AdGuardHome).tar.gz" ] && curl --location https://github.com/AdguardTeam/AdGuardHome/releases/download/$(PKG_VERSION)/AdGuardHome_$(PKG_ARCH_AdGuardHome).tar.gz -o $(PKG_BUILD_DIR)/AdGuardHome_$(PKG_ARCH_AdGuardHome).tar.gz
	$(RM) $(PKG_BUILD_DIR)/AdGuardHome/AdGuardHome
	tar -xzf $(PKG_BUILD_DIR)/AdGuardHome_$(PKG_ARCH_AdGuardHome).tar.gz -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	chmod 0755 $(PKG_BUILD_DIR)/AdGuardHome/AdGuardHome
	upx --lzma $(PKG_BUILD_DIR)/AdGuardHome/AdGuardHome
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
ifeq ($(BOARD),bcm53xx)
	$(INSTALL_BIN) ./files/bin/adg_bcm53xx $(1)/usr/bin/AdGuardHome
else
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/AdGuardHome/AdGuardHome $(1)/usr/bin/AdGuardHome
endif
	$(INSTALL_DIR) $(1)/etc/AdGuardHome
	$(INSTALL_CONF) ./files/conf/$(ADG_CONF_NAME).yaml $(1)/etc/AdGuardHome/AdGuardHome.yaml
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
