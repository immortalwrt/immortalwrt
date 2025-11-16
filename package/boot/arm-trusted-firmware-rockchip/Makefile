# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2023 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rkbin
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rockchip-linux/rkbin.git
PKG_SOURCE_DATE:=2025-06-13
PKG_SOURCE_VERSION:=74213af1e952c4683d2e35952507133b61394862
PKG_MIRROR_HASH:=4b801b1301ae297f660340617b5f398b23a3f0b43bc7f0ef42c21f0f43eb8990

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/trusted-firmware-a.mk
include $(INCLUDE_DIR)/package.mk
include ./atf-version.mk

define Trusted-Firmware-A/Default
  NAME:=Rockchip $(1) SoCs
  BUILD_TARGET:=rockchip
endef

define Trusted-Firmware-A/rk3308
  BUILD_SUBTARGET:=armv8
  ATF:=rk33/$(RK3308_ATF)
  TPL:=rk33/$(RK3308_TPL)
endef

define Trusted-Firmware-A/rk3308-tpl-rock-pi-s
  NAME:=Radxa ROCK Pi S board
  BUILD_SUBTARGET:=armv8
  TPL:=rk33/$(RK3308_TPL_ROCK_PI_S)
endef

define Trusted-Firmware-A/rk3328
  BUILD_SUBTARGET:=armv8
  ATF:=rk33/$(RK3328_ATF)
  TPL:=rk33/$(RK3328_TPL)
  LOADER:=rk33/$(RK3328_LOADER)
endef

define Trusted-Firmware-A/rk3399
  BUILD_SUBTARGET:=armv8
  ATF:=rk33/$(RK3399_ATF)
  TPL:=rk33/$(RK3399_TPL)
  LOADER:=rk33/$(RK3399_LOADER)
endef

define Trusted-Firmware-A/rk3528
  BUILD_SUBTARGET:=armv8
  ATF:=rk35/$(RK3528_ATF)
  TPL:=rk35/$(RK3528_TPL)
endef

define Trusted-Firmware-A/rk3566
  BUILD_SUBTARGET:=armv8
  ATF:=rk35/$(RK3566_ATF)
  TPL:=rk35/$(RK3566_TPL)
endef

define Trusted-Firmware-A/rk3568
  BUILD_SUBTARGET:=armv8
  ATF:=rk35/$(RK3568_ATF)
  TPL:=rk35/$(RK3568_TPL)
endef

define Trusted-Firmware-A/rk3568-tpl-e25
  NAME:=Radxa E25 board
  BUILD_SUBTARGET:=armv8
  TPL:=rk35/$(RK3568_TPL_E25)
endef

define Trusted-Firmware-A/rk3576
  BUILD_SUBTARGET:=armv8
  ATF:=rk35/$(RK3576_ATF)
  TPL:=rk35/$(RK3576_TPL)
endef

define Trusted-Firmware-A/rk3588
  BUILD_SUBTARGET:=armv8
  ATF:=rk35/$(RK3588_ATF)
  TPL:=rk35/$(RK3588_TPL)
endef

TFA_TARGETS:= \
	rk3308 \
	rk3308-tpl-rock-pi-s \
	rk3328 \
	rk3399 \
	rk3528 \
	rk3566 \
	rk3568 \
	rk3568-tpl-e25 \
	rk3576 \
	rk3588

ifeq ($(BUILD_VARIANT),rk3308-tpl-rock-pi-s)
  define Download/rk3308-tpl-rock-pi-s
    FILE:=$(notdir $(TPL))
    URL_FILE:=$(TPL)
    URL:=https://github.com/radxa/rkbin/raw/2b54df9d062ef91a9fffbc85472b070c9220c4cf/bin/
    HASH:=45af030ed2cb322cc5a91c32350130fc1f1ea9508794fa4b5d309eadf70e3d04
  endef

  define Build/Prepare
	$(eval $(call Download,rk3308-tpl-rock-pi-s))
	$(call Build/Prepare/Default)

	$(CP) $(DL_DIR)/$(notdir $(TPL)) $(PKG_BUILD_DIR)/bin/$(TPL)
  endef
endif

ifeq ($(BUILD_VARIANT),rk3568-tpl-e25)
  define Download/rk3568-tpl-e25
    FILE:=$(notdir $(TPL))
    URL_FILE:=$(TPL)
    URL:=https://github.com/radxa/rkbin/raw/2e77c53ab0279585b09ecdaa54fe3e2bf80f9475/bin/
    HASH:=1bb9f92a6515a70b91c0f8bd3aa4dc31432afc4317b9408f82c43ca63cb10ab6
  endef

  define Build/Prepare
	$(eval $(call Download,rk3568-tpl-e25))
	$(call Build/Prepare/Default)

	$(CP) $(DL_DIR)/$(notdir $(TPL)) $(PKG_BUILD_DIR)/bin/$(TPL)
  endef
endif

define Build/Compile
	# This comment is the workaround for "extraneous 'endif'" error
ifneq ($(LOADER),)
	( \
		pushd $(PKG_BUILD_DIR) ; \
		$(SED) 's,$$$$(PKG_BUILD_DIR),$(PKG_BUILD_DIR),g' trust.ini ; \
		$(SED) 's,$$$$(VARIANT),$(BUILD_VARIANT),g' trust.ini ; \
		./tools/mkimage -n $(BUILD_VARIANT) -T rksd -d bin/$(TPL) \
			$(BUILD_VARIANT)-idbloader.bin ; \
		cat bin/$(LOADER) >> $(BUILD_VARIANT)-idbloader.bin ; \
		./tools/trust_merger --replace bl31.elf bin/$(ATF) trust.ini ; \
		popd ; \
	)
endif
endef

define Package/trusted-firmware-a/install
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)

ifneq ($(ATF),)
	$(CP) $(PKG_BUILD_DIR)/bin/$(ATF) $(STAGING_DIR_IMAGE)/
endif
ifneq ($(LOADER),)
	$(CP) $(PKG_BUILD_DIR)/tools/loaderimage $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/$(BUILD_VARIANT)-idbloader.bin $(STAGING_DIR_IMAGE)/
	$(CP) $(PKG_BUILD_DIR)/$(BUILD_VARIANT)-trust.bin $(STAGING_DIR_IMAGE)/
else
	$(CP) $(PKG_BUILD_DIR)/bin/$(TPL) $(STAGING_DIR_IMAGE)/
endif
endef

$(eval $(call BuildPackage/Trusted-Firmware-A))
