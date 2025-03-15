#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=miniupnp-mtk-adjust
PKG_VERSION:=2.3.7
PKG_RELEASE:=1

PKG_MAINTAINER:=
PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:miniupnp_project:miniupnpd-mtk-adjust

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/version.mk

TAR_OPTIONS+= --strip-components 1
TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)

define Package/miniupnpd-mtk-adjust/Default
  SECTION:=MTK Properties
  CATEGORY:=MTK Properties
  DEPENDS:= \
	+libcap \
	+libmnl \
	+libuuid
  PROVIDES:=miniupnpd-mtk-adjust
  TITLE:=Lightweight UPnP IGD & PCP/NAT-PMP daemon
  SUBMENU:=Applications
  URL:=https://miniupnp.tuxfamily.org/
endef

define Package/miniupnpd-mtk-adjust-iptables
  $(call Package/miniupnpd-mtk-adjust/Default)
  DEPENDS+= \
	+IPV6:ip6tables \
	+IPV6:libip6tc \
	+iptables \
	+libip4tc \
	+libnetfilter-conntrack
  TITLE+= (iptables)
  VARIANT:=iptables
endef

define Package/miniupnpd-mtk-adjust-nftables
  $(call Package/miniupnpd-mtk-adjust/Default)
  DEPENDS+= \
	+libnftnl
  TITLE+= (nftables)
  VARIANT:=nftables
  DEFAULT_VARIANT:=1
  CONFLICTS:=miniupnpd-mtk-adjust-iptables
endef

define Package/miniupnpd-mtk-adjust/conffiles/Default
/etc/config/upnpd
endef

Package/miniupnpd-mtk-adjust-iptables/conffiles = $(Package/miniupnpd-mtk-adjust/conffiles/Default)
Package/miniupnpd-mtk-adjust-nftables/conffiles = $(Package/miniupnpd-mtk-adjust/conffiles/Default)

define Build/Prepare
	$(call Build/Prepare/Default)
	echo "$(VERSION_NUMBER)" | tr '() ' '_' >$(PKG_BUILD_DIR)/os.openwrt
endef

CONFIGURE_ARGS = \
	$(if $(CONFIG_IPV6),--ipv6) \
	--igd2 \
	--leasefile \
	--portinuse \
	--firewall=$(BUILD_VARIANT) \
	--disable-fork \
	--regex

TARGET_CFLAGS += $(FPIC)
TARGET_LDFLAGS += -Wl,--gc-sections,--as-needed

ifeq ($(BUILD_VARIANT),iptables)
	TARGET_CFLAGS += -flto
endif

define Package/miniupnpd-mtk-adjust/install/Default
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/miniupnpd $(1)/usr/sbin/miniupnpd
	$(INSTALL_BIN) ./files/miniupnpd.init $(1)/etc/init.d/miniupnpd
	$(INSTALL_CONF) ./files/upnpd.config $(1)/etc/config/upnpd
	$(INSTALL_DATA) ./files/miniupnpd.hotplug $(1)/etc/hotplug.d/iface/50-miniupnpd
endef

define Package/miniupnpd-mtk-adjust-iptables/install
	$(call Package/miniupnpd-mtk-adjust/install/Default,$1)
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DIR) $(1)/usr/share/miniupnpd
	$(INSTALL_BIN) ./files/miniupnpd.defaults.iptables $(1)/etc/uci-defaults/99-miniupnpd
	$(INSTALL_DATA) ./files/firewall3.include $(1)/usr/share/miniupnpd/firewall.include
endef

define Package/miniupnpd-mtk-adjust-nftables/install
	$(call Package/miniupnpd-mtk-adjust/install/Default,$1)
	$(INSTALL_DIR) $(1)/usr/share/nftables.d
	$(CP) ./files/nftables.d/* $(1)/usr/share/nftables.d/
endef

$(eval $(call BuildPackage,miniupnpd-mtk-adjust-iptables))
$(eval $(call BuildPackage,miniupnpd-mtk-adjust-nftables))
