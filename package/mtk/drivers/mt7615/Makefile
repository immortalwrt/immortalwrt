include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=mt7615
PKG_RELEASE:=1
PKG_BUILD_DEPENDS:=base-files
PKG_FILE_DEPENDS:=
PKG_LICENSE:=GPL-2.0

PKG_MAINTAINER:=Hua Shao <nossiac@163.com>
PKG_URLS:=http://nossiac.com/download/mtk-wifi-ko \
	http://119.23.148.191:8888/mtk-wifi-ko

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_KO:=$(PKG_NAME)-for-$(CONFIG_TARGET_SUBTARGET)-linux-$(LINUX_VERSION).ko

define Package/mt7615
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MTK MT7615 WiFi AP driver
  AUTOLOAD:=$(call AutoProbe,mt7615)
  DEPENDS:=+wireless-tools
endef

define Package/mt7615/config-todo
	if PACKAGE_mt7615
	source "$(SOURCE)/mt7615-config.in"
	endif
endef

define Package/mt7615/config-notyet
	config MT7615_USING_LUA_SCRIPT
	bool "use lua script to init/config driver"
	depends on PACKAGE_mt7615
	default y
endef

define Build/Compile
	for PKG_URL in $(PKG_URLS); do \
		wget $$$${PKG_URL}/$(PKG_KO) \
			-O $(PKG_BUILD_DIR)/$(PKG_KO); \
		if [ "$$$$?" != "0" ]; then continue; else break; fi \
	done
endef

define Package/mt7615/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	-$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_KO) \
		$(1)/lib/modules/$(LINUX_VERSION)/$(PKG_NAME).ko
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,mt7615))
