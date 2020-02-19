include $(TOPDIR)/rules.mk

PKG_NAME:=tcping
PKG_VERSION:=0.2
PKG_RELEASE=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/coolsnowwolf/tcping
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=d890cc1bd8e3951390ceeff1ccb092a5d802850c
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MAINTAINER:=coolsnowwolf <coolsnowwolf@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/tcping
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libnet-1.2.x
  TITLE:=Ping look-alike that uses TCP SYN packets to get around firewalls and ICMP blackholes
endef

define Package/tcping/description
Ping look-alike that uses TCP SYN packets to get around firewalls and ICMP blackholes
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/lib/libnet-1.2.x/include -L$(STAGING_DIR)/usr/lib/libnet-1.2.x/lib" \
		INSTALL_PROG=":"
endef

define Package/tcping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcpping $(1)/usr/bin/tcping
endef

$(eval $(call BuildPackage,tcping))
