# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=arm-trusted-firmware-rockchip-vendor
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/rockchip-linux/rkbin.git
PKG_SOURCE_DATE:=2021-06-01
PKG_SOURCE_VERSION:=7d631e0d5b2d373b54d4533580d08fb9bd2eaad4
PKG_MIRROR_HASH:=3a4794d1d00890401b032a573fca3121935ea036468a67228f203d68b007d916

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

MAKE_PATH:=$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/arm-trusted-firmware-rockchip-vendor
    SECTION:=boot
    CATEGORY:=Boot Loaders
    TITLE:=ARM Trusted Firmware for Rockchip
    DEPENDS:=@TARGET_rockchip_armv8
endef

define Package/arm-trusted-firmware-rk3328
    $(Package/arm-trusted-firmware-rockchip-vendor)
ifneq ($(CONFIG_PACKAGE_arm-trusted-firmware-rk3328),)
    SOC:=rk3328
    ATF:=rk33/rk322xh_bl31_v1.46.elf
    DDR:=rk33/rk3328_ddr_333MHz_v1.17.bin
    LOADER:=rk33/rk322xh_miniloader_v2.50.bin
endif
endef

define Package/arm-trusted-firmware-rk3399
    $(Package/arm-trusted-firmware-rockchip-vendor)
ifneq ($(CONFIG_PACKAGE_arm-trusted-firmware-rk3399),)
    SOC:=rk3399
    ATF:=rk33/rk3399_bl31_v1.35.elf
    DDR:=rk33/rk3399_ddr_800MHz_v1.25.bin
    LOADER:=rk33/rk3399_miniloader_v1.26.bin
endif
endef

define Build/Configure
	$(SED) 's,$$$$(PKG_BUILD_DIR),$(PKG_BUILD_DIR),g' $(PKG_BUILD_DIR)/trust.ini
	$(call Build/Configure/Default)
endef

define Build/Compile
	mkimage -n $$(SOC) -T rksd -d $(PKG_BUILD_DIR)/bin/$$(DDR) $(PKG_BUILD_DIR)/idbloader.bin
	cat $(PKG_BUILD_DIR)/bin/$$(LOADER) >> $(PKG_BUILD_DIR)/idbloader.bin
	$(PKG_BUILD_DIR)/tools/trust_merger --replace bl31.elf $(PKG_BUILD_DIR)/bin/$$(ATF) $(PKG_BUILD_DIR)/trust.ini
endef

define Build/InstallDev
	$(INSTALL_DIR) -p $(STAGING_DIR_IMAGE)
	$(CP) $(PKG_BUILD_DIR)/bin/$$(ATF) $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/tools/loaderimage $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/idbloader.bin $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/trust.bin $(STAGING_DIR_IMAGE)/
endef

define Package/arm-trusted-firmware-rk3328/install
endef

define Package/arm-trusted-firmware-rk3399/install
endef

$(eval $(call BuildPackage,arm-trusted-firmware-rk3328))
$(eval $(call BuildPackage,arm-trusted-firmware-rk3399))
