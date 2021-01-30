#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=adbyby
PKG_VERSION:=2.7
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/adbyby
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Powerful adblock module to block ad.
	DEPENDS:=@(i386||x86_64||arm||aarch64||mipsel||mips)
	URL:=http://www.adbyby.com/
endef

define Package/adbyby/description
	Adbyby is a powerful adblock module to block ad,just like adblock.
endef

ifeq ($(ARCH),i386)
	ADBYBY_DIR:=x86
endif

ifeq ($(ARCH),x86_64)
	ADBYBY_DIR:=amd64
endif

ifeq ($(ARCH),arm)
	ADBYBY_DIR:=armv7
	ifeq ($(CONFIG_HAS_FPU),)
		ADBYBY_DIR:=arm
	endif
endif

ifeq ($(ARCH),aarch64)
	ADBYBY_DIR:=armv7
endif

ifeq ($(ARCH),mips)
	ADBYBY_DIR:=mips
endif

ifeq ($(ARCH),mipsel)
	ADBYBY_DIR:=mipsle
endif

define Build/Compile
	true
endef

define Package/adbyby/install
	$(INSTALL_DIR) $(1)/usr/share/adbyby
	$(INSTALL_BIN) ./files/adbyby.sh $(1)/usr/share/adbyby/
	$(INSTALL_CONF) ./files/adhook.ini $(1)/usr/share/adbyby/
	$(INSTALL_CONF) ./files/user.action $(1)/usr/share/adbyby/

	$(INSTALL_DIR) $(1)/usr/share/adbyby/data
	$(INSTALL_DATA) ./files/data/* $(1)/usr/share/adbyby/data/

	$(INSTALL_DIR) $(1)/usr/share/adbyby/doc
	$(INSTALL_DATA) ./files/doc/* $(1)/usr/share/adbyby/doc/

	$(INSTALL_BIN) ./files/$(ADBYBY_DIR)/adbyby $(1)/usr/share/adbyby/adbyby
endef

$(eval $(call BuildPackage,adbyby))
