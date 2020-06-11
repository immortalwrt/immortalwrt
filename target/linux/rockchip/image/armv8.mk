# 
# Copyright (C) 2020 Tobias Maedel
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
define Device/radxa_rock-pi-e
  DEVICE_VENDOR := Radxa
  DEVICE_MODEL := Rock Pi E
  SOC := rk3328
  UBOOT_DEVICE_NAME := rock-pi-e-rk3328
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | sdcard-img | gzip | append-metadata
  DEVICE_PACKAGES += kmod-phy-realtek
endef
TARGET_DEVICES += radxa_rock-pi-e

define Device/pine64_rockpro64
  DEVICE_VENDOR := Pine64
  DEVICE_MODEL := RockPro64
  SOC := rk3399
  UBOOT_DEVICE_NAME := rockpro64-rk3399
  IMAGE/sysupgrade.img.gz := boot-common | boot-script | sdcard-img | gzip | append-metadata
endef
TARGET_DEVICES += pine64_rockpro64
