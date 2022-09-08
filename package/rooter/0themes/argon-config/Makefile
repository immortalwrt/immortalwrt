#Owned by DairyMan@Whirlpool
#
#Copyright GNU act.
include $(TOPDIR)/rules.mk

PKG_NAME:=argon-config
PKG_VERSION:=1.000
PKG_RELEASE:=1

PKG_MAINTAINER:=Created by DM/makefile by Cobia@whirlpool
include $(INCLUDE_DIR)/package.mk

define Package/argon-config
  SECTION:=utils
  CATEGORY:=ROOter
  SUBMENU:=Themes
  TITLE:=Install scripts for Argon Configuration
  PKGARCH:=all
endef

define Package/argon-config/description
  Helper scripts to install scripts for Argon Configuration
endef


define Build/Compile
endef

define Package/argon-config/install
	$(CP) ./files/* $(1)/


endef

$(eval $(call BuildPackage,argon-config))
