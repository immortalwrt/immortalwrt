#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2024.07
PKG_RELEASE:=1
PKG_HASH:=f591da9ab90ef3d6b3d173766d0ddff90c4ed7330680897486117df390d83c8f

PKG_MAINTAINER:=Tobias Maedel <openwrt@tbspace.de>

UBOOT_USE_BINMAN:=1
UBOOT_USE_INTREE_DTC:=1

include $(INCLUDE_DIR)/u-boot.mk
include $(INCLUDE_DIR)/package.mk
include ../arm-trusted-firmware-rockchip/atf-version.mk

define U-Boot/Default
  BUILD_TARGET:=rockchip
  UENV:=default
  HIDDEN:=1
endef


# RK3328 boards

define U-Boot/Default/rk3328
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3328
  ATF:=$(RK3328_ATF)
endef

define U-Boot/nanopi-r2c-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=NanoPi R2C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2c
endef

define U-Boot/nanopi-r2c-plus-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=NanoPi R2C Plus
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2c-plus
endef

define U-Boot/nanopi-r2s-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=NanoPi R2S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2s
endef

define U-Boot/orangepi-r1-plus-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=Orange Pi R1 Plus
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus
endef

define U-Boot/orangepi-r1-plus-lts-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=Orange Pi R1 Plus LTS
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus-lts
endef

define U-Boot/roc-cc-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=ROC-RK3328-CC
  BUILD_DEVICES:= \
    firefly_roc-rk3328-cc
endef

define U-Boot/rock-pi-e-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=Rock Pi E
  BUILD_DEVICES:= \
    radxa_rock-pi-e
endef

define U-Boot/rock64-rk3328
  $(U-Boot/Default/rk3328)
  NAME:=Rock64
  BUILD_DEVICES:= \
    pine64_rock64
endef


# RK3399 boards

define U-Boot/Default/rk3399
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3399
  ATF:=$(RK3399_ATF)
endef

define U-Boot/guangmiao-g4c-rk3399
  $(U-Boot/Default/rk3399)
  NAME:=GuangMiao G4C
  BUILD_DEVICES:= \
    huake_guangmiao-g4c
endef

define U-Boot/nanopc-t4-rk3399
  $(U-Boot/Default/rk3399)
  NAME:=NanoPC T4
  BUILD_DEVICES:= \
    friendlyarm_nanopc-t4
endef

define U-Boot/nanopi-r4s-rk3399
  $(U-Boot/Default/rk3399)
  NAME:=NanoPi R4S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4s \
    friendlyarm_nanopi-r4s-enterprise
endef

define U-Boot/nanopi-r4se-rk3399
  $(U-Boot/Default/rk3399)
  NAME:=NanoPi R4SE
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4se
endef

define U-Boot/rock-pi-4-rk3399
  $(U-Boot/Default/rk3399)
  NAME:=Rock Pi 4
  BUILD_DEVICES:= \
    radxa_rock-pi-4a
endef

define U-Boot/rockpro64-rk3399
  $(U-Boot/Default/rk3399)
  NAME:=RockPro64
  BUILD_DEVICES:= \
    pine64_rockpro64
endef


# RK3566 boards

define U-Boot/Default/rk3566
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3566
  ATF:=$(RK3566_ATF)
  TPL:=$(RK3566_TPL)
endef

define U-Boot/radxa-cm3-io-rk3566
  $(U-Boot/Default/rk3566)
  NAME:=CM3 IO
  BUILD_DEVICES:= \
    radxa_cm3-io
endef


# RK3568 boards

define U-Boot/Default/rk3568
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3568
  ATF:=$(RK3568_ATF)
  TPL:=$(RK3568_TPL)
endef

define U-Boot/bpi-r2-pro-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=Bananapi-R2 Pro
  BUILD_DEVICES:= \
    sinovoip_bpi-r2-pro
endef

define U-Boot/fastrhino-r66s-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=FastRhino R66S
  BUILD_DEVICES:= \
    lunzn_fastrhino-r66s
endef

define U-Boot/fastrhino-r68s-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=FastRhino R68S
  BUILD_DEVICES:= \
    lunzn_fastrhino-r68s
endef

define U-Boot/mrkaio-m68s-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=Mrkaio M68S
  BUILD_DEVICES:= \
    ezpro_mrkaio-m68s
endef

define U-Boot/nanopi-r5c-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=NanoPi R5C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r5c
endef

define U-Boot/nanopi-r5s-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=NanoPi R5S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r5s
endef

define U-Boot/photonicat-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=Photonicat
  BUILD_DEVICES:= \
    ariaboard_photonicat
endef

define U-Boot/radxa-e25-rk3568
  $(U-Boot/Default/rk3568)
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3568-e25
  TPL:=$(RK3568_TPL_E25)
  NAME:=E25
  BUILD_DEVICES:= \
    radxa_e25
endef

define U-Boot/roc-pc-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=Station P2
  BUILD_DEVICES:= \
    firefly_roc-rk3568-pc
endef

define U-Boot/rock-3a-rk3568
  $(U-Boot/Default/rk3568)
  NAME:=Rock 3A
  BUILD_DEVICES:= \
    radxa_rock-3a
endef


# RK3588 boards

define U-Boot/Default/rk3588
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3588
  ATF:=$(RK3588_ATF)
  TPL:=$(RK3588_TPL)
endef

define U-Boot/nanopc-t6-rk3588
  $(U-Boot/Default/rk3588)
  NAME:=NanoPC T6
  BUILD_DEVICES:= \
    friendlyarm_nanopc-t6
endef

define U-Boot/orangepi-5-plus-rk3588
  $(U-Boot/Default/rk3588)
  NAME:=OrangePi 5 Plus
  BUILD_DEVICES:= \
    xunlong_orangepi-5-plus
endef

define U-Boot/rock5b-rk3588
  $(U-Boot/Default/rk3588)
  NAME:=Rock 5B
  BUILD_DEVICES:= \
    radxa_rock-5b
endef

define U-Boot/sige7-rk3588
  $(U-Boot/Default/rk3588)
  NAME:=Sige7
  BUILD_DEVICES:= \
    armsom_sige7
endef


# RK3588S boards

define U-Boot/nanopi-r6c-rk3588s
  $(U-Boot/Default/rk3588)
  NAME:=NanoPi R6C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r6c
endef

define U-Boot/nanopi-r6s-rk3588s
  $(U-Boot/Default/rk3588)
  NAME:=NanoPi R6S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r6s
endef

define U-Boot/orangepi-5-rk3588s
  $(U-Boot/Default/rk3588)
  NAME:=OrangePi 5
  BUILD_DEVICES:= \
    xunlong_orangepi-5
endef

define U-Boot/rock5a-rk3588s
  $(U-Boot/Default/rk3588)
  NAME:=Rock 5A
  BUILD_DEVICES:= \
    radxa_rock-5a
endef

UBOOT_TARGETS := \
  guangmiao-g4c-rk3399 \
  nanopc-t4-rk3399 \
  nanopi-r4s-rk3399 \
  nanopi-r4se-rk3399 \
  rock-pi-4-rk3399 \
  rockpro64-rk3399 \
  nanopi-r2c-rk3328 \
  nanopi-r2c-plus-rk3328 \
  nanopi-r2s-rk3328 \
  orangepi-r1-plus-rk3328 \
  orangepi-r1-plus-lts-rk3328 \
  roc-cc-rk3328 \
  rock-pi-e-rk3328 \
  rock64-rk3328 \
  radxa-cm3-io-rk3566 \
  bpi-r2-pro-rk3568 \
  fastrhino-r66s-rk3568 \
  fastrhino-r68s-rk3568 \
  mrkaio-m68s-rk3568 \
  nanopi-r5c-rk3568 \
  nanopi-r5s-rk3568 \
  photonicat-rk3568 \
  radxa-e25-rk3568 \
  roc-pc-rk3568 \
  rock-3a-rk3568 \
  nanopc-t6-rk3588 \
  orangepi-5-plus-rk3588 \
  rock5b-rk3588 \
  sige7-rk3588 \
  nanopi-r6c-rk3588s \
  nanopi-r6s-rk3588s \
  orangepi-5-rk3588s \
  rock5a-rk3588s

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_CUSTOMIZE_CONFIG := \
	--disable TOOLS_MKEFICAPSULE \
	--set-str MKIMAGE_DTC_PATH $(PKG_BUILD_DIR)/scripts/dtc/dtc \
	$(if $(TPL),, \
		--disable CMD_DFU \
		--disable CMD_EFIDEBUG \
		--disable CMD_NVEDIT_EFI \
		--disable DFU_MMC \
		--disable EFI_CAPSULE_ON_DISK \
		--disable EFI_CAPSULE_FIRMWARE_RAW \
		--disable SPL_FIT_SIGNATURE \
		--disable USB_FUNCTION_FASTBOOT \
	)

UBOOT_MAKE_FLAGS += \
  BL31=$(STAGING_DIR_IMAGE)/$(ATF) \
  $(if $(TPL),ROCKCHIP_TPL=$(STAGING_DIR_IMAGE)/$(TPL))

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifneq ($(TPL),)
	$(CP) $(PKG_BUILD_DIR)/u-boot-rockchip.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot-rockchip.bin
else
	$(STAGING_DIR_IMAGE)/loaderimage --pack --uboot $(PKG_BUILD_DIR)/u-boot-dtb.bin $(PKG_BUILD_DIR)/uboot.img 0x200000
	$(CP) $(PKG_BUILD_DIR)/uboot.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.img
endif
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
