include $(TOPDIR)/rules.mk

PKG_NAME:=ngrokc
PKG_VERSION:=1.54
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/dosgo/ngrok-c.git
PKG_SOURCE_DATE:=2019-11-02
PKG_SOURCE_VERSION:=6388b9d8e4d5fef018ee64dfd55795f10cc6aaad
PKG_MIRROR_HASH:=a0f22073807e0ae7869cecce398e848800e5961e00d76a6185f0c1004609f093

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ngrokc
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=Secure tunnels to localhost (C++ port)
  URL:=https://github.com/dosgo/ngrok-c
  DEPENDS:=+libc +libpthread +libopenssl +libstdcpp
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) $(PKG_BUILD_DIR)/Makefile.openssl $(PKG_BUILD_DIR)/Makefile
endef

define Package/ngrokc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ngrokc $(1)/usr/bin
endef

$(eval $(call BuildPackage,ngrokc))
