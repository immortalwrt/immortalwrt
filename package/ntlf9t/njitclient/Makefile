include $(TOPDIR)/rules.mk

PKG_NAME:=njit8021xclient
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/bitdust/njit8021xclient.git
PKG_SOURCE_VERSION:=dd28c17f24275bbbf4c44504b832c0f1e6b9ae40
PKG_MIRROR_HASH:=689c01fe559a1ad00446bdb92890ddfcfbe0f53e5afd9ae54d5649e7a43ff961

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/njit8021xclient
	SECTION:=net
	CATEGORY:=Network
	TITLE:=NJIT 802.1X client program
	DEPENDS:=+libopenssl +libpcap
	SUBMENU:=Campus Network
endef

define Package/njit8021xclient/description
 802.1X client from Nanjing Institude of Technology,
 compatable with H3C iNode 802.1X client. Support
 H3C/iNode's private authentication protocol V7.10.
endef

define Build/Compile  
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -static" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LIBS="$(STAGING_DIR)/usr/lib/libpcap.a"
endef

define Package/njit8021xclient/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/client $(1)/usr/sbin/njit-client
endef

$(eval $(call BuildPackage,njit8021xclient))
