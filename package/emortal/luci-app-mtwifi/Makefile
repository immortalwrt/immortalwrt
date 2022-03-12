include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-mtwifi
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Hua Shao <nossiac@163.com>

LUCI_TITLE:=LuCI support for mt wifi driver
LUCI_PKGARCH:=all
PKG_VERSION:=1
PKG_RELEASE:=8

define Package/luci-app-mtwifi
endef

define Package/l1profile
  DEPENDS:=@TARGET_mediatek
  TITLE:=l1profile init script
endef

define Package/l1profile/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/lib/wifi/
	$(INSTALL_DIR) $(1)/etc/wireless/mt7615/
	$(INSTALL_DIR) $(1)/etc_ro/Wireless/RT2860AP/
	$(INSTALL_BIN) ./files/l1profile.dat $(1)/etc/wireless/
	$(INSTALL_BIN) ./files/mt7615.1.dat $(1)/etc/wireless/mt7615/
	$(INSTALL_BIN) ./files/mt7615.2.dat $(1)/etc/wireless/mt7615/
	$(INSTALL_BIN) ./files/mt7615.lua $(1)/lib/wifi
	$(INSTALL_BIN) ./files/firmware.sh $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/apcli.sh $(1)/etc/init.d/
endef


include $(TOPDIR)/feeds/luci/luci.mk
$(eval $(call BuildPackage,luci-app-mtwifi))
$(eval $(call BuildPackage,l1profile))
# call BuildPackage OpenWrt buildroot signature
