#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=antileech
PKG_VERSION:=2.3.2
PKG_RELEASE:=2

PKG_FIXUP:=autoreconf
PKG_BUILD_DEPENDS:=libgd
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/antileech
  SUBMENU:=P2P
  SECTION:=net
  CATEGORY:=Network
  TITLE:=AntiLeech for amule
  URL:=http://www.amule.org/
  DEPENDS:=+libc +amule +libwxbase
endef

define Build/Configure
	$(SED) "s:^AC_CHECK_PROG.WXCONFIG,.*::g" $(PKG_BUILD_DIR)/configure.ac
	$(SED) "s:wx-config --cppflags:$(STAGING_DIR)/usr/bin/wx-config --cppflags:g" $(PKG_BUILD_DIR)/configure.ac

	cd $(PKG_BUILD_DIR); $(SHELL) ./autogen.sh
	$(call Build/Configure/Default)
endef

MAKE_FLAGS += HOSTCC="$(HOSTCC)"

define Package/antileech/install
	$(INSTALL_DIR) $(1)/usr/share/amule
	$(CP) $(PKG_INSTALL_DIR)/usr/share/amule/libantiLeech.so* $(1)/usr/share/amule/
endef

$(eval $(call BuildPackage,antileech))
