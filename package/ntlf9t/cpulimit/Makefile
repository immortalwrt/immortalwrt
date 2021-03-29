include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit
PKG_VERSION:=0.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/cpulimit
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=cpulimit
  URL:=https://github.com/opsengine/cpulimit.git
endef

define Package/cpulimit/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpulimit $(1)/usr/bin/
endef


$(eval $(call BuildPackage,cpulimit))
