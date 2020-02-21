include $(TOPDIR)/rules.mk

PKG_NAME:=clash
PKG_VERSION:=0.17.2.3
PKG_MAINTAINER:=frainzy1477

ifeq ($(ARCH),mipsel)
	PKG_ARCH:=mipsle-softfloat
endif
ifeq ($(ARCH),mips)
	PKG_ARCH:=mips-softfloat
endif
ifeq ($(ARCH),i386)
	PKG_ARCH:=386
endif
ifeq ($(ARCH),x86_64)
	PKG_ARCH:=amd64
endif
ifeq ($(ARCH),arm)
	PKG_ARCH:=armv7
	ifneq ($(BOARD),bcm53xx)
		PKG_ARCH:=armv5
	endif
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH:=armv8
endif

PKG_SOURCE:=clash-linux-$(PKG_ARCH)-v$(PKG_VERSION).gz
PKG_SOURCE_URL:=https://github.com/frainzy1477/clash_dev/releases/download/v$(PKG_VERSION)/
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_HASH:=skip

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=clash is a cross-platform proxy software
	DEPENDS:=
	URL:=https://github.com/frainzy1477/clash_dev/releases
endef

define Package/$(PKG_NAME)/description
	clash is a cross-platform proxy software
endef

define Build/Prepare
	gzip -dc "$(DL_DIR)/$(PKG_SOURCE)" > $(PKG_BUILD_DIR)/clash-linux-$(PKG_ARCH)
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/clash
	$(INSTALL_DIR) $(1)/usr/share/clash

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/clash-linux-$(PKG_ARCH) $(1)/etc/clash/clash
	$(INSTALL_BIN) ./file/core_version $(1)/usr/share/clash/
endef


$(eval $(call BuildPackage,$(PKG_NAME)))
