include $(TOPDIR)/rules.mk

PKG_NAME:=udpspeeder
PKG_VERSION:=20190408
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wangyu-/UDPspeeder.git
PKG_SOURCE_VERSION:=ecc90928d33741dbe726b547f2d8322540c26ea0
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/udpspeeder
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A tunnel which improves your network quality on a high-latency lossy link
	URL:=https://github.com/wangyu-/UDPspeeder
endef

define Package/udpspeeder/description
A tunnel which improves your network quality on a high-latency lossy link
by using forward error correction, for all traffics (TCP/UDP/ICMP).
endef

define Package/udpspeeder/conffiles
/etc/config/udpspeeder
endef

define Build/Configure
	$(SED) 's/cc_cross=.*/cc_cross=$(TARGET_CXX)/g' $(PKG_BUILD_DIR)/makefile
	$(SED) 's/\\".*shell git rev-parse HEAD.*\\"/\\"$(PKG_SOURCE_VERSION)\\"/g' $(PKG_BUILD_DIR)/makefile
endef

MAKE_FLAGS += cross2

define Package/udpspeeder/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/speederv2_cross $(1)/usr/bin/udpspeeder
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/udpspeeder.init $(1)/etc/init.d/udpspeeder
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) files/udpspeeder.config $(1)/etc/config/udpspeeder
endef

$(eval $(call BuildPackage,udpspeeder))
