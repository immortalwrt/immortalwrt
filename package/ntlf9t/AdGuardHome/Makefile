#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=AdGuardHome
PKG_VERSION:=snapshot
PKG_RELEASE:=r1882-8086186

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
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_AdGuardHome:=linux_arm5
else ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_AdGuardHome:=linux_arm5
else
	PKG_ARCH_AdGuardHome:=linux_arm7
endif
	ADG_CONF_NAME:=big_space
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_AdGuardHome:=linux_arm64
	ADG_CONF_NAME:=big_space
endif

PKG_SOURCE:=AdGuardHome_$(PKG_ARCH_AdGuardHome).tar.gz
PKG_SOURCE_URL:=https://package.project-openwrt.eu.org/network/services/AdGuardHome/$(PKG_RELEASE)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Network-wide ads & trackers blocking DNS server
	URL:=https://github.com/AdguardTeam/AdGuardHome
endef

define Build/Prepare
	tar -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	chmod 0755 $(PKG_BUILD_DIR)/AdGuardHome/AdGuardHome
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/AdGuardHome/AdGuardHome $(1)/usr/bin/AdGuardHome
	$(INSTALL_DIR) $(1)/etc/AdGuardHome
	$(INSTALL_CONF) ./conf/$(ADG_CONF_NAME).yaml $(1)/etc/AdGuardHome/AdGuardHome.yaml
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
