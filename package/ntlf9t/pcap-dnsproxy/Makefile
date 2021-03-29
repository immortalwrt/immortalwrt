#
# Copyright (C) 2015-2018 wongsyrone
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pcap-dnsproxy
PKG_BASE_VERSION:=0.4.9.13
PKG_RELEASE:=$(AUTORELESE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/1715173329/Pcap_DNSProxy.git
PKG_SOURCE_DATE:=2019-05-05
PKG_SOURCE_VERSION:=bc9f540d9febc5f61d24d583dbdbcc858dd817e3
PKG_MIRROR_HASH:=416cea47ef5662c32111c56eeaaaaec4b338e6eddfe98fca0a0d854e00653dc3

PKG_VERSION:=$(PKG_BASE_VERSION)-$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Chengr28 <chengr28@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_PCAP_DNSPROXY_LIBSODIUM \
	CONFIG_PCAP_DNSPROXY_LIBPCAP \
	CONFIG_PCAP_DNSPROXY_TLS \
	CONFIG_PCAP_DNSPROXY_LISTENPORT

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

TARGET_CXXFLAGS += -Wall -Wextra
TARGET_CXXFLAGS += $(FPIC)

# redef CFLAGS and CXXFLAGS, replace -O* with -O3
TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS)) -O3
TARGET_CXXFLAGS := $(filter-out -O%,$(TARGET_CXXFLAGS)) -O3

# CXX standard
TARGET_CXXFLAGS += -std=c++14

CMAKE_OPTIONS += \
	-DPLATFORM_OPENWRT=ON \
	-DENABLE_LIBSODIUM=$(if $(CONFIG_PCAP_DNSPROXY_LIBSODIUM),ON,OFF) \
	-DENABLE_PCAP=$(if $(CONFIG_PCAP_DNSPROXY_LIBPCAP),ON,OFF) \
	-DENABLE_TLS=$(if $(CONFIG_PCAP_DNSPROXY_TLS),ON,OFF)

define Package/pcap-dnsproxy/config
	config PCAP_DNSPROXY_LIBPCAP
		bool "Build with libpcap support.(Strongly recommended)"
		default y
		help
		  LibPcap is for packet capture, Pcap_DNSProxy takes advantage
		  of it to detect poisoned DNS reply.

		  We strongly recommend to keep it as-is.

	config PCAP_DNSPROXY_LIBSODIUM
		bool "Build with libsodium support.(Recommended)"
		default y
		help
		  LibSodium is for DNSCurve/DNSCrypt protocol support.

		  We recommend to keep it as-is unless you do NOT
		  need this protocol anymore.

	config PCAP_DNSPROXY_TLS
		bool "Build with TLS support.(Recommended)"
		default y
		help
		  We recommend to keep it as-is.

	config PCAP_DNSPROXY_LISTENPORT
		int "Listen Port, should NOT be 53"
		default 1053
		help
		  You can customize the listen port of Pcap_DNSProxy.
		  Note that you should NOT set the value to 53, which
		  conflicts with DNSMasq in OpenWrt.
endef

define Package/pcap-dnsproxy
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A local DNS server based on LibPcap
  URL:=https://github.com/chengr28/Pcap_DNSProxy
  DEPENDS:=+libpthread +libstdcpp +libevent2-core \
    +PCAP_DNSPROXY_LIBPCAP:libpcap \
    +PCAP_DNSPROXY_LIBSODIUM:libsodium \
    +PCAP_DNSPROXY_TLS:libopenssl
endef

define Package/pcap-dnsproxy/conffiles
/etc/pcap-dnsproxy/Config.conf
/etc/pcap-dnsproxy/Hosts.conf
/etc/pcap-dnsproxy/IPFilter.conf
/etc/pcap-dnsproxy/Routing.txt
/etc/pcap-dnsproxy/WhiteList.txt
endef

define Package/pcap-dnsproxy/postinst
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Be sure to set configuration file(s) before rebooting your router."
	/etc/init.d/pcap-dnsproxy enable
fi
exit 0
endef

define Package/pcap-dnsproxy/prerm
#!/bin/sh
# check if we are on real system
if [ -z "$${IPKG_INSTROOT}" ]; then
	echo "Stopping service and removing rc.d symlink for pcap-dnsproxy"
	/etc/init.d/pcap-dnsproxy stop
	/etc/init.d/pcap-dnsproxy disable
fi
exit 0
endef

define Package/pcap-dnsproxy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/Pcap_DNSProxy $(1)/usr/sbin/Pcap_DNSProxy
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/pcap-dnsproxy.config $(1)/etc/config/pcap-dnsproxy
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/pcap-dnsproxy.procd.init $(1)/etc/init.d/pcap-dnsproxy
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/pcap-dnsproxy.hotplug $(1)/etc/hotplug.d/iface/99-pcap-dnsproxy
	$(INSTALL_DIR) $(1)/etc/pcap-dnsproxy
	$(SED) 's,^\xEF\xBB\xBF,,g' $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/*
	$(SED) 's,\x0D,,g' $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/*
	$(SED) 's,Listen Port = 53,Listen Port = $(CONFIG_PCAP_DNSPROXY_LISTENPORT),g' $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/Config.ini
	$(SED) 's,Log Maximum Size = 8MB,Log Maximum Size = 50KB,g' $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/Config.ini
	$(SED) 's,Operation Mode = Private,Operation Mode = Server,g' $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/Config.ini
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/Config.ini $(1)/etc/pcap-dnsproxy/Config.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/Hosts.ini $(1)/etc/pcap-dnsproxy/Hosts.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/IPFilter.ini $(1)/etc/pcap-dnsproxy/IPFilter.conf
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/Source/Auxiliary/ExampleConfig/Routing.txt $(1)/etc/pcap-dnsproxy/Routing.txt
	$(INSTALL_CONF) ./files/configs/WhiteList.txt $(1)/etc/pcap-dnsproxy/WhiteList.txt
endef

$(eval $(call BuildPackage,pcap-dnsproxy))
