include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-athena-led
PKG_DEPENDS:=
PKG_VERSION:=0.0.7
PKG_RELEASE:=20241029

LUCI_TITLE:=LuCI Support for athena-led
LUCI_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

# 包定义;指示我们的包将如何以及在哪里出现在整体配置菜单中 （'make menuconfig'）
define Package/$(PKG_NAME)
  TITLE:=$(PKG_NAME)
endef

# Package description; a more verbose description on what our package does
define Package/$(PKG_NAME)/description
  LuCI support for athenaLed
endef


define Package/$(PKG_NAME)/install
  $(INSTALL_DIR) $(1)/usr/lib/lua/luci
  cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
  $(INSTALL_DIR) $(1)/
  cp -pR ./root/* $(1)/
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
	chmod +x /usr/sbin/athena-led
	chmod +x /etc/init.d/athena_led
exit 0
endef

include $(TOPDIR)/feeds/luci/luci.mk

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,$(PKG_NAME)))


