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

define Device/armsom_sige3
  $(Device/rk3568)
  DEVICE_VENDOR := ArmSoM
  DEVICE_MODEL := Sige3
  DEVICE_DTS := rk3568-armsom-sige3
  DEVICE_PACKAGES := kmod-brcmfmac kmod-r8125 wpad-openssl \
	brcmfmac-firmware-43752-sdio brcmfmac-nvram-43752-sdio
endef
TARGET_DEVICES += armsom_sige3

define Device/firefly_roc-rk3328-cc
  $(Device/rk3328)
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := ROC-RK3328-CC
  DEVICE_DTS := rk3328-roc-cc
  UBOOT_DEVICE_NAME := roc-cc-rk3328
endef
TARGET_DEVICES += firefly_roc-rk3328-cc

define Device/firefly_roc-rk3568-pc
  $(Device/rk3568)
  DEVICE_VENDOR := Firefly
  DEVICE_MODEL := Station P2
  DEVICE_ALT0_VENDOR := Firefly
  DEVICE_ALT0_MODEL := ROC-RK3568-PC
  DEVICE_DTS := rk3568-roc-pc
  SUPPORTED_DEVICES := firefly,rk3568-roc-pc
  UBOOT_DEVICE_NAME := roc-pc-rk3568
  DEVICE_PACKAGES := kmod-ata-ahci-dwc kmod-brcmfmac wpad-openssl \
	brcmfmac-firmware-43752-sdio brcmfmac-nvram-43752-sdio
endef
TARGET_DEVICES += firefly_roc-rk3568-pc


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

define Device/friendlyarm_nanopi-r5c
  $(Device/rk3568)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R5C
  DEVICE_PACKAGES := kmod-r8125 kmod-rtw88-8822ce rtl8822ce-firmware wpad-openssl
endef
TARGET_DEVICES += friendlyarm_nanopi-r5c

define Device/friendlyarm_nanopi-r5s
  $(Device/rk3568)
  DEVICE_VENDOR := FriendlyARM
  DEVICE_MODEL := NanoPi R5S
  DEVICE_PACKAGES := kmod-r8125
endef
TARGET_DEVICES += friendlyarm_nanopi-r5s

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

define Device/radxa_cm3-io
  $(Device/rk3566)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := CM3 IO
  DEVICE_DTS := rk3566-radxa-cm3-io
  UBOOT_DEVICE_NAME := radxa-cm3-io-rk3566
endef
TARGET_DEVICES += radxa_cm3-io

define Device/radxa_e20c
  $(Device/rk3528)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E20C
  DEVICE_DTS := rk3528-radxa-e20c
  UBOOT_DEVICE_NAME := radxa-e20c-rk3528
  DEVICE_PACKAGES := kmod-r8169
endef
TARGET_DEVICES += radxa_e20c

define Device/radxa_e25
  $(Device/rk3568)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E25
  DEVICE_DTS := rk3568-radxa-e25
  UBOOT_DEVICE_NAME := radxa-e25-rk3568
  BOOT_SCRIPT := radxa-e25
  DEVICE_PACKAGES := kmod-r8125 kmod-ata-ahci-dwc
endef
TARGET_DEVICES += radxa_e25

define Device/radxa_e52c
  $(Device/rk3582)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := E52C
  DEVICE_DTS := rk3582-radxa-e52c
  UBOOT_DEVICE_NAME := generic-rk3588
  DEVICE_PACKAGES := blkdiscard kmod-r8125
  DEVICE_COMPAT_VERSION := 1.1
  DEVICE_COMPAT_MESSAGE := Network interface names have been changed
endef
TARGET_DEVICES += radxa_e52c

define Device/radxa_rock-2a
  $(Device/rk3528)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 2A
  UBOOT_DEVICE_NAME := rock-2-rk3528
endef
TARGET_DEVICES += radxa_rock-2a

define Device/radxa_rock-2f
  $(Device/rk3528)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 2F
  UBOOT_DEVICE_NAME := rock-2-rk3528
endef
TARGET_DEVICES += radxa_rock-2f

define Device/radxa_rock-3a
  $(Device/rk3568)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 3A
  SUPPORTED_DEVICES := radxa,rock3a
  DEVICE_PACKAGES := kmod-usb-net-cdc-ncm kmod-usb-net-rndis
endef
TARGET_DEVICES += radxa_rock-3a

define Device/radxa_rock-3b
  $(Device/rk3568)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 3B
  DEVICE_PACKAGES := kmod-usb-net-cdc-ncm kmod-usb-net-rndis
endef
TARGET_DEVICES += radxa_rock-3b

define Device/radxa_rock-3c
  $(Device/rk3566)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 3C
  DEVICE_PACKAGES := kmod-usb-net-cdc-ncm kmod-usb-net-rndis
endef
TARGET_DEVICES += radxa_rock-3c

define Device/radxa_rock-4c-plus
  $(Device/rk3399)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 4C+
endef
TARGET_DEVICES += radxa_rock-4c-plus

define Device/radxa_rock-4d
  $(Device/rk3576)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 4D
endef
TARGET_DEVICES += radxa_rock-4d

define Device/radxa_rock-4se
  $(Device/rk3399)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ROCK 4SE
endef
TARGET_DEVICES += radxa_rock-4se

define Device/radxa_zero-3e
  $(Device/rk3566)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ZERO 3E
  DEVICE_DTS := rk3566-radxa-zero-3e
  UBOOT_DEVICE_NAME := radxa-zero-3-rk3566
  DEVICE_PACKAGES := kmod-usb-net-cdc-ncm kmod-usb-net-rndis
endef
TARGET_DEVICES += radxa_zero-3e

define Device/radxa_zero-3w
  $(Device/rk3566)
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := ZERO 3W
  DEVICE_DTS := rk3566-radxa-zero-3w
  UBOOT_DEVICE_NAME := radxa-zero-3-rk3566
  DEVICE_PACKAGES := kmod-usb-net-cdc-ncm kmod-usb-net-rndis
endef
TARGET_DEVICES += radxa_zero-3w

define Device/sinovoip_bpi-r2-pro
  $(Device/rk3568)
  DEVICE_VENDOR := Bananapi
  DEVICE_MODEL := BPi-R2 Pro
  SUPPORTED_DEVICES := sinovoip,rk3568-bpi-r2pro
  DEVICE_PACKAGES := kmod-ata-ahci-dwc
endef
TARGET_DEVICES += sinovoip_bpi-r2-pro

define Device/widora_mangopi-m28c
  $(Device/rk3528)
  DEVICE_VENDOR := Widora
  DEVICE_MODEL := MangoPi M28C
  DEVICE_PACKAGES := kmod-aic8800-sdio wpad-openssl kmod-hwmon-pwmfan
endef
TARGET_DEVICES += widora_mangopi-m28c

define Device/widora_mangopi-m28k
  $(Device/rk3528)
  DEVICE_VENDOR := Widora
  DEVICE_MODEL := MangoPi M28K
  DEVICE_PACKAGES := kmod-aic8800-sdio wpad-openssl kmod-r8169
endef
TARGET_DEVICES += widora_mangopi-m28k

define Device/xunlong_orangepi-5
  $(Device/rk3588s)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi 5
endef
TARGET_DEVICES += xunlong_orangepi-5

define Device/xunlong_orangepi-5-plus
  $(Device/rk3588)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi 5 Plus
  DEVICE_PACKAGES := kmod-r8125
endef
TARGET_DEVICES += xunlong_orangepi-5-plus

define Device/xunlong_orangepi-r1-plus
  $(Device/rk3328)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1 Plus
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += xunlong_orangepi-r1-plus

define Device/xunlong_orangepi-r1-plus-lts
  $(Device/rk3328)
  DEVICE_VENDOR := Xunlong
  DEVICE_MODEL := Orange Pi R1 Plus LTS
  DEVICE_PACKAGES := kmod-usb-net-rtl8152
endef
TARGET_DEVICES += xunlong_orangepi-r1-plus-lts
