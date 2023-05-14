include $(TOPDIR)/rules.mk

PKG_NAME:=h69k-fancontroller
PKG_VERSION:=1.0
PKG_RELEASE:=2
#PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/h69k-fancontroller
  SECTION:=utils
  CATEGORY:=H69K
  TITLE:=Make h69k fan quiet
  DEPENDS:=
#  CONFLICTS:=kmod-hwmon-pwmfan
endef

define Package/h69k-fancontroller/description
  Make the fan of Hinlink-H69K runs at 50% speed.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/h69k-fancontroller/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/fancontroller $(1)/etc/init.d/fancontroller
endef

define Package/fancontroller/postinst
	#!/bin/sh
	ln -sf /etc/init.d/fancontroller /etc/rc.d/S99fancontroller
	/etc/init.d/fancontroller enable
	/etc/init.d/fancontroller start
	exit 0
endef

$(eval $(call BuildPackage,h69k-fancontroller))
