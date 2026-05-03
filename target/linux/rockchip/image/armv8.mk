# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 Sarah Maedel

define Device/rk3308
  SOC := rk3308
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3328
  SOC := rk3328
  KERNEL_LOADADDR := 0x03200000
  BOOT_FLOW := pine64-bin
endef

define Device/rk3399
  SOC := rk3399
  KERNEL_LOADADDR := 0x03200000
  BOOT_FLOW := pine64-bin
endef

define Device/rk3528
  SOC := rk3528
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3566
  SOC := rk3566
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3568
  SOC := rk3568
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3576
  SOC := rk3576
  KERNEL_LOADADDR := 0x43000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3582
  SOC := rk3582
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3588
  SOC := rk3588
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/rk3588s
  SOC := rk3588s
  KERNEL_LOADADDR := 0x03000000
  BOOT_FLOW := pine64-img
endef

define Device/embedfire_doornet2
  DEVICE_VENDOR := EmbedFire
  DEVICE_MODEL := DoorNet2
  SOC := rk3399
  BOOT_FLOW := pine64-bin
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += embedfire_doornet2



define Device/firefly_roc-rk3328-cc
  $(Device/rk3328)
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := ROC-RK3328-CC
  DEVICE_DTS := rk3328-roc-cc
  UBOOT_DEVICE_NAME := roc-cc-rk3328
endef
TARGET_DEVICES += firefly_roc-rk3328-cc



define Device/friendlyarm_nanopi-r2c
  $(Device/rk3328)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2C
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2c

define Device/friendlyarm_nanopi-r2c-plus
  $(Device/rk3328)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2C Plus
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2c-plus

define Device/friendlyarm_nanopi-r2s
  $(Device/rk3328)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R2S
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += friendlyarm_nanopi-r2s

define Device/friendlyarm_nanopi-r3s
  $(Device/rk3566)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R3S
  DEVICE_PACKAGES := kmod-r8169
endef
TARGET_DEVICES += friendlyarm_nanopi-r3s

define Device/friendlyarm_nanopi-r4s
  $(Device/rk3399)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4S
  DEVICE_PACKAGES := kmod-r8169
endef
TARGET_DEVICES += friendlyarm_nanopi-r4s

define Device/friendlyarm_nanopi-r4se
  $(Device/rk3399)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4SE
  DEVICE_PACKAGES := kmod-r8169
endef
TARGET_DEVICES += friendlyarm_nanopi-r4se

define Device/friendlyarm_nanopi-r4s-enterprise
  $(Device/rk3399)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R4S Enterprise Edition
  UBOOT_DEVICE_NAME := nanopi-r4s-rk3399
  DEVICE_PACKAGES := kmod-r8169
endef
TARGET_DEVICES += friendlyarm_nanopi-r4s-enterprise

define Device/friendlyarm_nanopi-r6c
  $(Device/rk3588s)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R6C
  DEVICE_PACKAGES := kmod-r8125
endef
TARGET_DEVICES += friendlyarm_nanopi-r6c

define Device/friendlyarm_nanopi-r6s
  $(Device/rk3588s)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R6S
  DEVICE_PACKAGES := kmod-r8125
endef
TARGET_DEVICES += friendlyarm_nanopi-r6s

define Device/friendlyarm_nanopi-r76s
  $(Device/rk3576)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R76S
  UBOOT_DEVICE_NAME := generic-rk3576
  DEVICE_PACKAGES := kmod-r8125 kmod-rtw88-8822cs wpad-openssl
endef
TARGET_DEVICES += friendlyarm_nanopi-r76s

define Device/pine64_rock64
  $(Device/rk3328)
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := Rock64
endef
TARGET_DEVICES += pine64_rock64

define Device/pine64_rockpro64
  $(Device/rk3399)
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := RockPro64
  SUPPORTED_DEVICES += pine64,rockpro64-v2.1
endef
TARGET_DEVICES += pine64_rockpro64
