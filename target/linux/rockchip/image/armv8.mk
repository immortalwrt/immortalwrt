# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2020 Sarah Maedel

define Device/embedfire_doornet2
  DEVICE_VENDOR := EmbedFire
  DEVICE_MODEL := DoorNet2
  SOC := rk3399
  BOOT_FLOW := pine64-bin
  DEVICE_PACKAGES := kmod-r8168 -urngd
endef
TARGET_DEVICES += embedfire_doornet2
