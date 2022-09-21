include $(TOPDIR)/rules.mk

PKG_NAME:=mhz
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/wtarreau/mhz.git
PKG_SOURCE_DATE:=2022-08-18
PKG_SOURCE_VERSION:=6ec38cbb1371d20078a7a5059dd9faa5b281b2d9
PKG_MIRROR_HASH:=37559cc8b5e08d23c09878d63cf81ea8d123be45408f3e76e1dc042766a746a7

PKG_MAINTAINER:=Robert Marko <robimarko@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/mhz
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CPU frequency measurement utility
  URL:=https://github.com/wtarreau/mhz
endef

define Package/mhz/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mhz $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,mhz))
