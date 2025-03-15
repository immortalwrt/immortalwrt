#
# Copyright (C) 2006-2011 Xmlad.com
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wrtbwmon
PKG_VERSION:=1.2.1
PKG_RELEASE:=3

include $(INCLUDE_DIR)/package.mk

define Package/wrtbwmon
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+iptables +@BUSYBOX_CONFIG_IP
	TITLE:=A Traffic Usage Monitor
	SUBMENU:=Tools
	PKGARCH:=all
endef

define Package/wrtbwmon/description
An CERNET client daemon,
Most usually used in China collages.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/wrtbwmon/install
	$(INSTALL_DIR) $(1)/usr/sbin $(1)/usr/share/wrtbwmon
	$(INSTALL_BIN) ./net/usr/sbin/* $(1)/usr/sbin/
	$(INSTALL_BIN) ./net/usr/share/wrtbwmon/* $(1)/usr/share/wrtbwmon/

	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./net/etc/init.d/wrtbwmon $(1)/etc/init.d/wrtbwmon
	$(INSTALL_BIN) ./net/etc/config/wrtbwmon $(1)/etc/config/wrtbwmon
	$(INSTALL_BIN) ./net/etc/hotplug.d/iface/99-wrtbwmon $(1)/etc/hotplug.d/iface/99-wrtbwmon
endef

$(eval $(call BuildPackage,wrtbwmon))
