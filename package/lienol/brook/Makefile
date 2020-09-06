#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk
ifeq ($(ARCH),i386)
	PKG_ARCH_BROOK:=386
endif
ifeq ($(ARCH),x86_64)
	PKG_ARCH_BROOK:=amd64
endif
ifeq ($(ARCH),arm)
	PKG_ARCH_BROOK:=arm7
endif
ifeq ($(BOARD),bcm53xx)
	PKG_ARCH_BROOK:=arm6
ifeq ($(word 2,$(subst +,$(space),$(call qstrip,$(CONFIG_CPU_TYPE)))),)
	PKG_ARCH_BROOK:=arm5
endif
endif
ifeq ($(BOARD),kirkwood)
	PKG_ARCH_BROOK:=arm5
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH_BROOK:=arm64
endif
ifeq ($(ARCH),mips)
	PKG_ARCH_BROOK:=mips_softfloat
endif
ifeq ($(ARCH),mipsel)
	PKG_ARCH_BROOK:=mipsle_softfloat
endif
ifeq ($(ARCH),mips64)
	PKG_ARCH_BROOK:=mips64
endif
ifeq ($(ARCH),mips64el)
	PKG_ARCH_BROOK:=mips64le
endif

PKG_NAME:=brook
PKG_VERSION:=20200909
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=brook_linux_$(PKG_ARCH_BROOK)
PKG_SOURCE_URL:=https://github.com/txthinking/brook/releases/download/v$(PKG_VERSION)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Brook is a cross-platform proxy software
	DEPENDS:=@!(arc||armeb||powerpc)
	URL:=https://github.com/txthinking/brook
endef

define Package/$(PKG_NAME)/description
  Brook is a cross-platform proxy software
endef

define Build/Prepare
	$(CP) $(DL_DIR)/$(PKG_SOURCE) $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_SOURCE) $(1)/usr/bin/brook
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
