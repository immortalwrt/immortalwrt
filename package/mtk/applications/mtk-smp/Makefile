#
# This is free software, licensed under the GNU General Public License v2.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtk-smp
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/mtk-smp
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  TITLE:=Setup mtk smp settings
  SUBMENU:=Applications
endef

define Build/Compile
endef

define Package/mtk-smp/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/smp.sh $(1)/sbin/
	$(INSTALL_BIN) ./files/mtk_smp.init $(1)/etc/init.d/mtk_smp
endef

$(eval $(call BuildPackage,mtk-smp))
