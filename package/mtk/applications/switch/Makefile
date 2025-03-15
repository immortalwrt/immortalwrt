#
# hua.shao@mediatek.com
#
# MTK Property Software.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=switch
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/switch
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  DEPENDS:=+libnl-tiny
  TITLE:=Command to config switch
  SUBMENU:=Applications
endef

define Package/switch/description
  An program to config switch.
endef

TARGET_CPPFLAGS := \
	-D_GNU_SOURCE \
	-I$(LINUX_DIR)/user_headers/include \
	-I$(STAGING_DIR)/usr/include/libnl-tiny \
	-I$(PKG_BUILD_DIR) \
	$(TARGET_CPPFLAGS) \

define Build/Compile
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LIBS="$(TARGET_LDFLAGS) -lnl-tiny -lm"
endef

define Package/switch/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/lib/network
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/switch $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/switch_ctl.sh $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/switch_ctl.init $(1)/etc/init.d/switch_ctl
endef

$(eval $(call BuildPackage,switch))
