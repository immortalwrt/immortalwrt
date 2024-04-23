include $(TOPDIR)/rules.mk

PKG_NAME:=mhz
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/wtarreau/mhz.git
PKG_SOURCE_DATE:=2023-06-17
PKG_SOURCE_VERSION:=11aac2399780a1f7ea9f007b14af0464797d5cf1
PKG_MIRROR_HASH:=3accbc6ff26b029d257686e1a7c7f9a40ef5d914a031d31f4d46a807edbad7c3

PKG_MAINTAINER:=Robert Marko <robimarko@gmail.com>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/mhz
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CPU frequency measurement utility
  URL:=https://github.com/wtarreau/mhz
endef

define Package/mhz/description
  Tool to mathematically calculate the current CPU frequency.
endef

define Package/mhz/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mhz $(1)/usr/sbin/mhz
endef

$(eval $(call BuildPackage,mhz))
