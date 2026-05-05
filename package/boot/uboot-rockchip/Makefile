#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=2026.01
PKG_RELEASE:=1
PKG_HASH:=b60d5865cefdbc75da8da4156c56c458e00de75a49b80c1a2e58a96e30ad0d54

PKG_MAINTAINER:=Sarah Maedel <openwrt@tbspace.de>

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


# RK3399 boards

define U-Boot/rk3399/Default
  BUILD_SUBTARGET:=armv8
  DEPENDS:=+PACKAGE_u-boot-$(1):trusted-firmware-a-rk3399
  ATF:=$(RK3399_ATF)
endef

define U-Boot/doornet2-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=DoorNet2
  BUILD_DEVICES:= \
    embedfire_doornet2
endef

define U-Boot/nanopi-r4s-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4S
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4s \
    friendlyarm_nanopi-r4s-enterprise
endef

define U-Boot/nanopi-r4se-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=NanoPi R4SE
  BUILD_DEVICES:= \
    friendlyarm_nanopi-r4se
endef

define U-Boot/rock-4c-plus-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=ROCK 4C+
  BUILD_DEVICES:= \
    radxa_rock-4c-plus
endef

define U-Boot/rock-4se-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=ROCK 4SE
  BUILD_DEVICES:= \
    radxa_rock-4se
endef

define U-Boot/rock-pi-4-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=ROCK Pi 4
  BUILD_DEVICES:= \
    radxa_rock-pi-4a
endef

define U-Boot/rockpro64-rk3399
  $(U-Boot/rk3399/Default)
  NAME:=RockPro64
  BUILD_DEVICES:= \
    pine64_rockpro64
endef


UBOOT_TARGETS := \
  doornet2-rk3399 \
  nanopi-r4se-rk3399 \


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
		--disable VIDEO \
	)

UBOOT_MAKE_FLAGS += \
  PATH=$(STAGING_DIR_HOST)/bin:$(PATH) \
  BL31=$(STAGING_DIR_IMAGE)/$(ATF) \
  $(if $(TPL),ROCKCHIP_TPL=$(STAGING_DIR_IMAGE)/$(TPL))

define Build/Configure
	$(call Build/Configure/U-Boot)

ifneq ($(filter %rk3576,$(BUILD_VARIANT)),)
	$(TARGET_CC) -nostdlib -ffreestanding -Os -S -o $(PKG_BUILD_DIR)/rk3576-boost.S $(PKG_BUILD_DIR)/rk3576-boost.c
	$(TARGET_CROSS)as -o $(PKG_BUILD_DIR)/rk3576-boost.o $(PKG_BUILD_DIR)/rk3576-boost.S
	$(TARGET_CROSS)objcopy -O binary -j .text $(PKG_BUILD_DIR)/rk3576-boost.o $(PKG_BUILD_DIR)/rk3576-boost.bin
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR_IMAGE)
ifneq ($(TPL),)
	$(CP) $(PKG_BUILD_DIR)/u-boot-rockchip.bin $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-u-boot-rockchip.bin
else
	$(STAGING_DIR_IMAGE)/loaderimage --pack --uboot $(PKG_BUILD_DIR)/u-boot-dtb.bin $(PKG_BUILD_DIR)/uboot.img 0x800000
	$(CP) $(PKG_BUILD_DIR)/uboot.img $(STAGING_DIR_IMAGE)/$(BUILD_VARIANT)-uboot.img
endif
endef

define Package/u-boot/install/default
endef

$(eval $(call BuildPackage/U-Boot))
