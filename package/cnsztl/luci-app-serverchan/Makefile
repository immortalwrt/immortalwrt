# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-serverchan
PKG_VERSION:=1.0
PKG_RELEASE:=40

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-serverchan
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for serverchan on or off
	DEPENDS:=+iputils-arping +curl
endef

define Package/luci-app-serverchan/description
	LuCI support for serverchan
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/luci-app-serverchan/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	$(INSTALL_DIR) $(1)/
	cp -pR ./root/* $(1)/
endef

define Package/luci-app-serverchan/postinst
#!/bin/sh
	chmod 755 /etc/init.d/serverchan >/dev/null 2>&1
	chmod 755 /usr/bin/serverchan/serverchan >/dev/null 2>&1
	/etc/init.d/serverchan enable >/dev/null 2>&1
exit 0
endef

define Package/luci-app-serverchan/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
     /etc/init.d/serverchan disable
     /etc/init.d/serverchan stop
fi
exit 0
endef

$(eval $(call BuildPackage,luci-app-serverchan))
