#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ap6256-firmware
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/paralin/ap6256-firmware.git
PKG_SOURCE_VERSION:=7074a2e21dd804e229eab1c031bc00246e9173e0
PKG_MIRROR_HASH:=fa376d4372b1c5111fa21021d3f5ecdafa16f4271c06c0ebcdafd2a4442ad6e0

include $(INCLUDE_DIR)/package.mk

define Package/ap6256-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Broadcom BCM43456 FullMac SDIO firmware
  URL:=https://github.com/armbian/firmware
endef

define Build/Compile
	true
endef

define Package/ap6256-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/BCM4345C5.hcd \
		$(1)/lib/firmware/brcm/BCM4345C5.hcd
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/fw_bcm43456c5_ag.bin \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcmfmac43456-sdio.clm_blob \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.clm_blob
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/nvram_ap6256.txt \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.txt
endef

$(eval $(call BuildPackage,ap6256-firmware))
