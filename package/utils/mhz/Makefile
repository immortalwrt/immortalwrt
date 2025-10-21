include $(TOPDIR)/rules.mk

PKG_NAME:=mhz
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/wtarreau/mhz.git
PKG_SOURCE_DATE:=2025-07-26
PKG_SOURCE_VERSION:=512f63021f6f0cec8e5e13be744dba0b4704cfae
PKG_MIRROR_HASH:=1827af2ed7a3c6f4499f6aacafcb668c3efcb1274135dfa50fe5ffa3eca4e140

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
