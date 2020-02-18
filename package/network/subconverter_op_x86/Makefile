include $(TOPDIR)/rules.mk

PKG_NAME:=subconverter
PKG_VERSION:=0.3.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/subconverter
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Utility to convert between various subscription format.
  PKGARCH:=x86_64
  DEPENDS:=+libpthread +libstdcpp \
		+libevent2 +libyaml-cpp +libcurl +libopenssl
endef

define Package/subconverter/description
	Utility to convert between various subscription format.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/subconverter/install
	$(INSTALL_DIR) $(1)/usr/sbin/subconverter
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN) ./bin/subconverter $(1)/usr/sbin/subconverter/subconverter
	$(CP) ./bin/* $(1)/usr/sbin/subconverter
	$(INSTALL_BIN) ./files/subconverter.init $(1)/etc/init.d/subconverter
endef

$(eval $(call BuildPackage,subconverter))

