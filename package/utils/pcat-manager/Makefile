include $(TOPDIR)/rules.mk

PKG_NAME:=pcat-manager
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/photonicat/rockchip_rk3568_pcat_manager.git
PKG_SOURCE_DATE:=2023-11-13
PKG_SOURCE_VERSION:=0149b4780866648d53a2a8633ecf7bc23f1937e5
PKG_MIRROR_HASH:=a02802e79da03d553e907ea168dc6004afb76fa316cd8441d3a3c7b18e9c77bf

PKG_LICENSE:=GPL-3.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_FLAGS:=nonshared
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk
include $(INCLUDE_DIR)/meson.mk

define Package/pcat-manager
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=PCat System Manager
  DEPENDS:=@TARGET_rockchip_armv8 +glib2 +libgpiod +libjson-c +libusb-1.0 @BROKEN
  URL:=https://photonicat.com
endef

define Package/pcat-manager/description
  PCat Manager is a system service for power management
  and modem controlling.
endef

define Package/pcat-manager/conffiles
/etc/pcat-manager-userdata.conf
endef

define Package/pcat-manager/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pcat-manager $(1)/usr/sbin/pcat-manager

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/conf/pcat-manager.conf.sample $(1)/etc/pcat-manager.conf

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pcat-manager.init $(1)/etc/init.d/pcat-manager
endef

$(eval $(call BuildPackage,pcat-manager))
