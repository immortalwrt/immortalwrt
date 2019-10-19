include $(TOPDIR)/rules.mk

PKG_NAME:=clash
PKG_VERSION:=0.16.0
PKG_MAINTAINER:=frainzy1477

ifeq ($(ARCH),mipsel)
	PKG_ARCH:=mipsle
	PKG_SOURCE:=clash-linux-$(PKG_ARCH)-softfloat-v$(PKG_VERSION).gz
endif
ifeq ($(ARCH),mips)
	PKG_ARCH:=mips
	PKG_SOURCE:=clash-linux-$(PKG_ARCH)-softfloat-v$(PKG_VERSION).gz
endif
ifeq ($(ARCH),x86_64)
	PKG_ARCH:=amd64
	PKG_SOURCE:=clash-linux-$(PKG_ARCH)-v$(PKG_VERSION).gz
endif
ifeq ($(ARCH),arm)
	PKG_ARCH:=armv7
	PKG_SOURCE:=clash-linux-$(PKG_ARCH)-v$(PKG_VERSION).gz
endif
ifeq ($(ARCH),aarch64)
	PKG_ARCH:=armv8
	PKG_SOURCE:=clash-linux-$(PKG_ARCH)-v$(PKG_VERSION).gz
endif

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
