include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-smstool
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-smstool
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=SMS Tool for 3G Modem
	PKGARCH:=all
endef

define Package/luci-app-smstool/description
	This package contains sms tool for 3G Modem
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-smstool/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view
	$(INSTALL_DATA) ./files/usr/lib/lua/luci/controller/sms.lua $(1)/usr/lib/lua/luci/controller/sms.lua
	$(INSTALL_DATA) ./files//usr/lib/lua/luci/view/sms.htm $(1)/usr/lib/lua/luci/view/sms.htm
endef

$(eval $(call BuildPackage,luci-app-smstool))

