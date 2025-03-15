#
# MTK-factory read and write
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtkhqos_util
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

define Package/mtkhqos_util
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=mtk hqos utility
  VERSION:=$(PKG_RELEASE)-$(REVISION)
  SUBMENU:=Applications
endef

define Package/mtkhqos_util/description
  mtk hqos util
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default
endef

Build/Compile = $(Build/Compile/Default)

define Package/mtkhqos_util/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_BIN) ./files/mtkhqos $(1)/sbin/
	$(INSTALL_DATA) ./files/mtkhqos.config $(1)/etc/config/mtkhqos
	$(INSTALL_BIN) ./files/mtkhqos.init $(1)/etc/init.d/mtkhqos
endef

$(eval $(call BuildPackage,mtkhqos_util))
