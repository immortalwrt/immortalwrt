#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gq-client
PKG_VERSION:=1.2.2
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/temple
  SECTION:=net
  CATEGORY:=Network
  TITLE:=GoQuiet
  DEPENDS:=+libpthread
  URL:=https://github.com/cbeuw/GoQuiet
endef

define Package/$(PKG_NAME)
    $(call Package/$(PKG_NAME)/temple)
endef

define Package/gq-server
    $(call Package/$(PKG_NAME)/temple)
endef

define Package/$(PKG_NAME)/description
GoQuiet-client
endef

define Package/gq-server/description
GoQuiet-server
endef

ifeq ($(ARCH),x86_64)
  GOQUIET_NAME:=gq-client-linux-amd64-$(PKG_VERSION)
  GOQUIETSERVER_NAME:=gq-server-linux-amd64-$(PKG_VERSION)
endif
ifeq ($(ARCH),i386)
  GOQUIET_NAME:=gq-client-linux-386-$(PKG_VERSION)
  GOQUIETSERVER_NAME:=gq-server-linux-386-$(PKG_VERSION)
endif
ifeq ($(ARCH),mipsel)
  GOQUIET_NAME:=gq-client-linux-mips64-$(PKG_VERSION)
endif
ifeq ($(ARCH),mips)
  GOQUIET_NAME:=gq-client-linux-mips-$(PKG_VERSION)
endif
ifeq ($(ARCH),arm)
  GOQUIET_NAME:=gq-client-linux-arm-$(PKG_VERSION)
  GOQUIETSERVER_NAME:=gq-server-linux-arm-$(PKG_VERSION)
endif
ifeq ($(ARCH),aarch64)
  GOQUIET_NAME:=gq-client-linux-arm64-$(PKG_VERSION)
  GOQUIETSERVER_NAME:=gq-server-linux-arm64-$(PKG_VERSION)
endif

define Build/Prepare
	[ ! -f $(PKG_BUILD_DIR)/gq-client ] && wget -O $(PKG_BUILD_DIR)/gq-client https://github.com/cbeuw/GoQuiet/releases/download/v$(PKG_VERSION)/$(GOQUIET_NAME)
	[ ! -f $(PKG_BUILD_DIR)/gq-server ] && wget -O $(PKG_BUILD_DIR)/gq-server https://github.com/cbeuw/GoQuiet/releases/download/v$(PKG_VERSION)/$(GOQUIETSERVER_NAME)
endef

define Build/Configure
	chmod +x $(PKG_BUILD_DIR)/gq-client && upx --ultra-brute $(PKG_BUILD_DIR)/gq-client
	chmod +x $(PKG_BUILD_DIR)/gq-server && upx --ultra-brute $(PKG_BUILD_DIR)/gq-server
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gq-client $(1)/usr/bin/
endef

define Package/gq-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gq-server $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,gq-server))
