#
# Copyright (c) 2015 Justin Liu
# Author: Justin Liu <rssnsj@gmail.com>
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipset-lists
PKG_VERSION:=20181104
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/ipset-lists
  CATEGORY:=Network
  TITLE:=Service for IPSET address tables
  MAINTAINER:=Justin Liu <rssnsj@gmail.com>
  DEPENDS:=+ipset
endef

define Package/ipset-lists/conffiles
/etc/gfwlist/china-banned
endef

define Build/Compile
	true
endef

define Package/ipset-lists/install
	$(CP) files/* $(1)/
endef

define Package/ipset-lists/postinst
#!/bin/sh
if [ -e /etc/openwrt_release ]; then
	/etc/init.d/ipset.sh restart
	/etc/init.d/ipset.sh enable || :
fi
endef

$(eval $(call BuildPackage,ipset-lists))
