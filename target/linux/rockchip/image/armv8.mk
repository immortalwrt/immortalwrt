# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 Sarah Maedel

define Device/embedfire_doornet2
  DEVICE_VENDOR := EmbedFire
  DEVICE_MODEL := DoorNet2
  SOC := rk3399
  BOOT_FLOW := pine64-bin
  UBOOT_DEVICE_NAME := doornet2-rk3399
  DEVICE_PACKAGES := kmod-r8168 -urngd
  KERNEL_LOADADDR := 0x800800
  KERNEL_ENTRY_POINT := 0x800800
endef
TARGET_DEVICES += embedfire_doornet2
