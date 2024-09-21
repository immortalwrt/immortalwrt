#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2023.07.02
PKG_RELEASE:=1
PKG_HASH:=6b6a48581c14abb0f95bd87c1af4d740922406d7b801002a9f94727fdde021d5

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

define U-Boot/rk3328/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3328
  ATF:=$(RK3328_ATF)
endef

define U-Boot/nanopi-r2c-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=NanoPi R2C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2c
endef

define U-Boot/nanopi-r2c-plus-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=NanoPi R2C Plus
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2c-plus
endef

define U-Boot/nanopi-r2s-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=NanoPi R2S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r2s
endef

define U-Boot/orangepi-r1-plus-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=Orange Pi R1 Plus
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus
endef

define U-Boot/orangepi-r1-plus-lts-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=Orange Pi R1 Plus LTS
  BUILD_DEVICES:= \
    xunlong_orangepi-r1-plus-lts
endef

define U-Boot/roc-cc-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=ROC-RK3328-CC
  BUILD_DEVICES:= \
    firefly_roc-rk3328-cc
endef

define U-Boot/rock-pi-e-rk3328
  $(U-Boot/rk3328/Default)
  NAME:=Rock Pi E
  BUILD_DEVICES:= \
    radxa_rock-pi-e
endef

# RK3399 boards

define U-Boot/rk3399/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3399
  ATF:=$(RK3399_ATF)
endef

define U-Boot/guangmiao-g4c-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=GuangMiao G4C
  BUILD_DEVICES:= \
    huake_guangmiao-g4c
endef

define U-Boot/nanopc-t4-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPC T4
  BUILD_DEVICES:= \
    friendlyarm_nanopc-t4
endef

define U-Boot/nanopi-r4s-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4s
endef

define U-Boot/nanopi-r4se-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4SE
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4se
endef

define U-Boot/nanopi-r4s-enterprise-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4S Enterprise Edition
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4s-enterprise
endef

define U-Boot/rock-pi-4-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=Rock Pi 4
  BUILD_DEVICES:= \
    radxa_rock-pi-4a
endef

define U-Boot/rockpro64-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=RockPro64
  BUILD_DEVICES:= \
    pine64_rockpro64
endef


# RK3568 boards

define U-Boot/rk3568/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3568
  ATF:=$(RK3568_ATF)
  TPL:=$(RK3568_TPL)
endef

define U-Boot/fastrhino-r66s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=FastRhino R66S
  BUILD_DEVICES:= \
    lunzn_fastrhino-r66s
endef

define U-Boot/fastrhino-r68s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=FastRhino R68S
  BUILD_DEVICES:= \
    lunzn_fastrhino-r68s
endef

define U-Boot/lyt-t68m-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=LYT T68M
  BUILD_DEVICES:= \
    lyt_t68m
endef

define U-Boot/mrkaio-m68s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Mrkaio M68S
  BUILD_DEVICES:= \
    ezpro_mrkaio-m68s
endef

define U-Boot/nanopi-r5c-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=NanoPi R5C
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r5c
endef

define U-Boot/nanopi-r5s-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=NanoPi R5S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r5s
endef

define U-Boot/photonicat-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Ariaboard Photonicat
  BUILD_DEVICES:= \
    ariaboard_photonicat
endef

define U-Boot/roc-pc-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Station P2
  BUILD_DEVICES:= \
    firefly_roc-rk3568-pc
endef

define U-Boot/sige3-rk3568
  $(U-Boot/rk3568/Default)
  NAME:=Sige3
  BUILD_DEVICES:= \
    armsom_sige3
endef

UBOOT_TARGETS := \
  guangmiao-g4c-rk3399 \
  nanopc-t4-rk3399 \
  nanopi-r4s-rk3399 \
  nanopi-r4se-rk3399 \
  nanopi-r4s-enterprise-rk3399 \
  rock-pi-4-rk3399 \
  rockpro64-rk3399 \
  nanopi-r2c-rk3328 \
  nanopi-r2c-plus-rk3328 \
  nanopi-r2s-rk3328 \
  orangepi-r1-plus-rk3328 \
  orangepi-r1-plus-lts-rk3328 \
  roc-cc-rk3328 \
  rock-pi-e-rk3328 \
  fastrhino-r66s-rk3568 \
  fastrhino-r68s-rk3568 \
  lyt-t68m-rk3568 \
  mrkaio-m68s-rk3568 \
  nanopi-r5c-rk3568 \
  nanopi-r5s-rk3568 \
  photonicat-rk3568 \
  roc-pc-rk3568 \
  sige3-rk3568

UBOOT_CONFIGURE_VARS += USE_PRIVATE_LIBGCC=yes

UBOOT_CUSTOMIZE_CONFIG := \
	--disable SPL_FIT_SIGNATURE \
	--disable TOOLS_MKEFICAPSULE \
	--set-str MKIMAGE_DTC_PATH $(PKG_BUILD_DIR)/scripts/dtc/dtc

UBOOT_MAKE_FLAGS += \
  PATH=$(STAGING_DIR_HOST)/bin:$(PATH) \
  BL31=$(STAGING_DIR_IMAGE)/$(ATF) \
  $(if $(TPL),ROCKCHIP_TPL=$(STAGING_DIR_IMAGE)/$(TPL))

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifneq ($(TPL),)
	$(CP) $(PKG_BUILD_DIR)/idbloader.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-idbloader.img
	$(CP) $(PKG_BUILD_DIR)/u-boot.itb $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot.itb
else
	$(STAGING_DIR_IMAGE)/loaderimage --pack --uboot $(PKG_BUILD_DIR)/u-boot-dtb.bin $(PKG_BUILD_DIR)/uboot.img 0x200000
	$(CP) $(PKG_BUILD_DIR)/uboot.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.img
endif
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
