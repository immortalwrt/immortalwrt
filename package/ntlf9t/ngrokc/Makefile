include $(TOPDIR)/rules.mk

PKG_NAME:=ngrokc
PKG_VERSION:=1.54
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/dosgo/ngrok-c.git
PKG_SOURCE_VERSION:=5ecd4e03d04ac538051fe8f1b0deaca500dc7185
PKG_MIRROR_HASH:=6bb27003b9357e9edd07b3b869e150ad506b253f3469fbbbae1fcece630afeb2

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/ngrokc
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=Secure tunnels to localhost(C++ port)
	URL:=https://github.com/dosgo/ngrok-c
	DEPENDS:=+libc +libpthread +libopenssl +libstdcpp
endef

define Build/Prepare
	$(call Build/Prepare/Default,)
	$(CP) $(PKG_BUILD_DIR)/Makefile.openssl $(PKG_BUILD_DIR)/Makefile
endef

define Package/ngrokc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ngrokc $(1)/usr/bin
endef

$(eval $(call BuildPackage,ngrokc))
