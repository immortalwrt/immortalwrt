#
# Copyright (C) 2019
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=datconf
PKG_REVISION:=6bb733f7
PKG_SOURCE:=$(PKG_NAME)-$(PKG_REVISION).tar.bz2
PKG_RELEASE:=1
PKG_USE_NINJA:=0

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

PKG_MAINTAINER:=Weijie Gao <weijie.gao@mediatek.com>

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/libkvcutil
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C library for Key-value based config files
endef

define Package/kvcedit
  SECTION:=util
  CATEGORY:=Utilities
  DEPENDS:=+libkvcutil
  TITLE:=Utility for editing key-value based config files
endef

define Package/datconf
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  DEPENDS:=+kvcedit
  TITLE:=Utility for editing dat files used by MediaTek Wi-Fi drivers
endef

define Package/datconf-lua
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  DEPENDS:=+datconf +liblua
  TITLE:=Lua plugin for datconf
endef

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib

CMAKE_OPTIONS = \
        -DLUAPATH=/usr/lib/lua


define Package/libkvcutil/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/kvcutil/libkvcutil.so* $(1)/lib/
endef

define Package/kvcedit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/kvcutil/kvcedit $(1)/usr/bin/
endef

define Package/datconf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/datconf/datconf $(1)/usr/bin/
endef

define Package/datconf-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(CP) $(PKG_BUILD_DIR)/datconf/lua/datconf.so $(1)/usr/lib/lua/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/kvcutil/libkvcutil.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/datconf/libdatconf.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/kvcutil/libkvcutil.so* $(1)/usr/lib
	-$(CP) $(PKG_BUILD_DIR)/datconf/libdatconf.a $(1)/usr/lib
endef

$(eval $(call BuildPackage,libkvcutil))
$(eval $(call BuildPackage,kvcedit))
$(eval $(call BuildPackage,datconf))
$(eval $(call BuildPackage,datconf-lua))
