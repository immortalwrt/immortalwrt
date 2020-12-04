# Copyright (C) 2018-2019 Lienol
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-autoipsetadder
PKG_VERSION:=1.0
PKG_RELEASE:=7

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-autoipsetadder
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for autoipsetadder
	PKGARCH:=all
	DEPENDS:= +httping +curl +coreutils-stdbuf
endef

define Package/luci-app-autoipsetadder/description
	LuCI support for autoipsetadder
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-autoipsetadder/conffiles
/etc/config/autoipsetadder
endef

define Package/luci-app-autoipsetadder/install
    $(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
endef

define Package/luci-app-autoipsetadder/postinst
#!/bin/sh
	/etc/init.d/autoipsetadder enable >/dev/null 2>&1
	/etc/init.d/autoipsetadder start
	rm -f /tmp/luci-indexcache
	rm -f /tmp/luci-modulecache/*
exit 0
endef

define Package/luci-app-autoipsetadder/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/autoipsetadder disable
     /etc/init.d/autoipsetadder stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-autoipsetadder))
