#
# This is free software, licensed under the GNU General Public License v2.
#


include $(TOPDIR)/rules.mk

PKG_NAME:=mtwifi-cfg
PKG_LICENSE:=GPL-2.0+
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/mtwifi-cfg
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  SUBMENU:=Applications
  TITLE:=mtwifi configure scripts for mt798x
  DEPENDS:=+iwinfo +wifi-dats +lua-cjson +datconf-lua  
endef

define Build/Compile
endef

define Package/mtwifi-cfg/install
	$(INSTALL_DIR) $(1)/sbin $(1)/usr/lib/lua
	$(INSTALL_DIR) $(1)/lib/netifd/wireless/ $(1)/lib/wifi/

	$(INSTALL_DIR) $(1)/etc/hotplug.d/net/

	$(INSTALL_BIN) ./files/netifd/mtwifi.sh $(1)/lib/netifd/wireless/
	$(INSTALL_BIN) ./files/mtwifi.sh $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/mtwifi-cfg/mtwifi_cfg $(1)/sbin/
	$(INSTALL_BIN) ./files/mtwifi-cfg/inspect.lua $(1)/usr/lib/lua/
	$(INSTALL_BIN) ./files/mtwifi-cfg/mtwifi_defs.lua $(1)/usr/lib/lua/
	$(INSTALL_BIN) ./files/mtwifi-cfg/mtwifi_utils.lua $(1)/usr/lib/lua/
	$(INSTALL_BIN) ./files/l1util/l1util $(1)/sbin/
	$(LN) l1util $(1)/sbin/l1dat
	$(INSTALL_BIN) ./files/l1util/l1dat_parser.lua $(1)/usr/lib/lua/
	$(INSTALL_DATA) ./files/hotplug/10-mtwifi-detect $(1)/etc/hotplug.d/net/
endef

$(eval $(call BuildPackage,mtwifi-cfg))
