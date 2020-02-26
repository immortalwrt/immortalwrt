#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=subconverter
PKG_VERSION:=0.4.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tindy2013/subconverter.git
PKG_SOURCE_VERSION:=844c68df9aaf68685922f0e444a36931de9604a2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=jpcre2 rapidjson

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Utility to convert between various subscription format.
	URL:=https://github.com/tindy2013/subconverter
	DEPENDS:=+libpthread +libstdcpp +libevent2 +libyaml-cpp +libpcre2 +libcurl +libopenssl +zlib
endef

define Package/$(PKG_NAME)/description
	Utility to convert between various subscription format.
endef

define Build/Configure
	mkdir -p $(PKG_BUILD_DIR)/obj
endef

define Build/Compile
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/interfaces.cpp -o $(PKG_BUILD_DIR)/obj/interfaces.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/logger.cpp -o $(PKG_BUILD_DIR)/obj/logger.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/main.cpp -o $(PKG_BUILD_DIR)/obj/main.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/misc.cpp -o $(PKG_BUILD_DIR)/obj/misc.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/multithread.cpp -o $(PKG_BUILD_DIR)/obj/multithread.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/nodemanip.cpp -o $(PKG_BUILD_DIR)/obj/nodemanip.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/rapidjson_extra.cpp -o $(PKG_BUILD_DIR)/obj/rapidjson_extra.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/speedtestutil.cpp -o $(PKG_BUILD_DIR)/obj/speedtestutil.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/subexport.cpp -o $(PKG_BUILD_DIR)/obj/subexport.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/webget.cpp -o $(PKG_BUILD_DIR)/obj/webget.o
	$(TARGET_CROSS)c++ -std=c++17 -Wall -fexceptions -c $(PKG_BUILD_DIR)/src/webserver_libevent.cpp -o $(PKG_BUILD_DIR)/obj/webserver_libevent.o
	$(TARGET_CROSS)c++ -o $(PKG_BUILD_DIR)/subconverter $(PKG_BUILD_DIR)/obj/*.o -lpcre2-8 -levent -lpthread -lyaml-cpp -lcurl -lssl -lcrypto -lz -O3 -s
endef

define Build/Install
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/subconverter $(1)/usr/bin/subconverter
	$(INSTALL_DIR) $(1)/etc/subconverter
	$(CP) $(PKG_BUILD_DIR)/base/* $(1)/etc/subconverter/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/subconverter.init $(1)/etc/init.d/subconverter
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	ln -sf /usr/bin/subconverter /etc/subconverter/subconverter
fi
exit 0
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
