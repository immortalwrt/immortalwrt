include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=mt7612
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

define Package/mt76x2
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MTK MT76x2 WiFi AP driver
  DEPENDS:=+wireless-tools
  HIDDEN:=1
endef

define Package/mt7602
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MTK MT7602 WiFi AP driver
  AUTOLOAD:=$(call AutoProbe,mt7602)
  DEPENDS:=+wireless-tools +mt76x2
endef

define Package/mt7612
  CATEGORY:=MTK Properties
  SUBMENU:=Drivers
  TITLE:=MTK MT7612 WiFi AP driver
  AUTOLOAD:=$(call AutoProbe,mt7612)
  DEPENDS:=+wireless-tools +mt76x2
endef

define Package/mt7612/config-notyet
	config MT7612_USING_LUA_SCRIPT
		bool "use lua script to init/config driver"
		depends on PACKAGE_mt7612
		default y
endef


define Build/Compile
	for PKG_URL in $(PKG_URLS); do \
		wget $$$${PKG_URL}/$(PKG_KO) \
			-O $(PKG_BUILD_DIR)/$(PKG_KO); \
		if [ "$$$$?" != "0" ]; then continue; else break; fi \
	done
endef


define Package/mt76x2/install
	if [ "$$(CONFIG_PACKAGE_mt7602)" != "" ] || [ "$$(CONFIG_PACKAGE_mt7612)" != "" ]; then \
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION); \
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_KO) \
		$(1)/lib/modules/$(LINUX_VERSION)/mt7612.ko; \
	fi

	if [ "$$(CONFIG_PACKAGE_mt7602)" != "" ]; then \
	$(INSTALL_DIR) $(1)/etc/wireless/mt7602; \
	$(CP) ./files/etc/wireless/mt7602/mt7602.dat $(1)/etc/wireless/mt7602/mt7602.dat; \
	$(INSTALL_DIR) $(1)/lib/wifi; \
	$(CP) ./files/lib/wifi/mt7602.lua $(1)/lib/wifi/mt7602.lua; \
	fi

	if [ "$$(CONFIG_PACKAGE_mt7612)" != "" ]; then \
	$(INSTALL_DIR) $(1)/etc/wireless/mt7612; \
	$(CP) ./files/etc/wireless/mt7612/mt7612.dat $(1)/etc/wireless/mt7612/mt7612.dat; \
	$(INSTALL_DIR) $(1)/lib/wifi; \
	$(CP) ./files/lib/wifi/mt7612.lua $(1)/lib/wifi/mt7612.lua; \
	fi
endef

$(eval $(call BuildPackage,mt76x2))
$(eval $(call BuildPackage,mt7602))
$(eval $(call BuildPackage,mt7612))
