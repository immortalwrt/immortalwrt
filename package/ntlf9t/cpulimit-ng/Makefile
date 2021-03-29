include $(TOPDIR)/rules.mk

PKG_NAME:=cpulimit-ng
PKG_VERSION:=1.2
PKG_RELEASE:=$(AUTORELEASE)

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/cpulimit-ng
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CPU Usage Limiter for Linux
endef

define Pacakge/cpulimit-ng/description
  cpulimit-ng is a simple program which attempts to limit the cpu
  usage of a process (expressed in percentage, not in cpu time).
endef

define Package/cpulimit-ng/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cpulimit-ng $(1)/bin/
endef

$(eval $(call BuildPackage,cpulimit-ng))
