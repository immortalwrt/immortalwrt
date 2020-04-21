include $(TOPDIR)/rules.mk

PKG_NAME:=njit8021xclient
PKG_VERSION:=2.0

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/njit8021xclient
	SECTION:=net
	CATEGORY:=Network
	TITLE:=NJIT 802.1X client program
	DEPENDS:=+libpcap +libopenssl
	SUBMENU:=Campus Network
endef

define Package/njit8021xclient/description
 802.1X client from Nanjing Institude of Technology,
 compatable with H3C iNode 802.1X client.
 Support H3C/iNode's private authentication protocol V7.10
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile  
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -static" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LIBS="$(STAGING_DIR)/usr/lib/libpcap.a"
	$(CP) $(PKG_BUILD_DIR)/client $(PKG_BUILD_DIR)/njit-client
endef

define Package/njit8021xclient/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/njit-client $(1)/bin/
endef

$(eval $(call BuildPackage,njit8021xclient))
