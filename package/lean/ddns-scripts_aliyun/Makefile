include $(TOPDIR)/rules.mk

PKG_NAME:=ddns-scripts_aliyun
PKG_VERSION:=1.0.3
PKG_RELEASE:=1

PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Sense <sensec@gmail.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=IP Addresses and Names
	TITLE:=DDNS extension for AliYun.com
	PKGARCH:=all
	DEPENDS:=ddns-scripts +wget-ssl +openssl-util
endef

define Package/$(PKG_NAME)/description
	Dynamic DNS Client scripts extension for AliYun.com
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/ddns $(1)/usr/share/ddns/default
	$(INSTALL_BIN) $(CURDIR)/files/update_aliyun_com.sh $(1)/usr/lib/ddns
	$(INSTALL_DATA) $(CURDIR)/files/aliyun.com.json $(1)/usr/share/ddns/default
endef

define Package/$(PKG_NAME)/prerm
	#!/bin/sh
	# if NOT run buildroot then stop service
	[ -z "$${IPKG_INSTROOT}" ] && /etc/init.d/ddns stop >/dev/null 2>&1
	exit 0 # suppress errors
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
