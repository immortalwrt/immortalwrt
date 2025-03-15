#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mii_mgr
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/mii_mgr
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=mii_mgr/mii_mgr_cl45
  SUBMENU:=Applications
endef

define Package/mii_mgr/description
  An mdio r/w phy regs program.
endef

define Package/mii_mgr/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mii_mgr $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mii_mgr $(1)/usr/sbin/mii_mgr_cl45
endef


$(eval $(call BuildPackage,mii_mgr))

