#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tcpping
PKG_VERSION:=0.2
PKG_RELEASE=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/coolsnowwolf/tcping
PKG_SOURCE_DATE:=2020-09-19
PKG_SOURCE_VERSION:=d890cc1bd8e3951390ceeff1ccb092a5d802850c
PKG_MIRROR_HASH:=018554a80e8a9d8fedd39821f35aa02c7c763fd42213761fba9b21c2533e1ab8

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tcpping
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libnet-1.2.x +libpcap
  TITLE:=Ping look-alike that uses TCP SYN packets to get around firewalls and ICMP blackholes
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/lib/libnet-1.2.x/include -L$(STAGING_DIR)/usr/lib/libnet-1.2.x/lib" \
		INSTALL_PROG=":"
endef

define Package/tcpping/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcpping $(1)/usr/bin/tcpping
endef

$(eval $(call BuildPackage,tcpping))
