include $(TOPDIR)/rules.mk

PKG_NAME:=njit8021xclient
PKG_BASE_VERSION:=2.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/bitdust/njit8021xclient.git
PKG_SOURCE_DATE:=2018-11-24
PKG_SOURCE_VERSION:=dd28c17f24275bbbf4c44504b832c0f1e6b9ae40
PKG_MIRROR_HASH:=66e4f20ceeceaed457657b60ec0b390aab0d7ff380b0abe749738b8f4aa23556

PKG_VERSION:=$(PKG_BASE_VERSION)-$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/njit8021xclient
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Campus Network
  TITLE:=NJIT 802.1X client program
  DEPENDS:=+libopenssl +libpcap
endef

define Package/njit8021xclient/description
  802.1X client from Nanjing Institude of Technology, compatable with
  H3C iNode 802.1X client. Support H3C/iNode's private authentication
  protocol V7.10.
endef

define Package/njit8021xclient/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/njit-client $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,njit8021xclient))
