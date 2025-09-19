# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=brcmfmac-firmware-4366c0-pcie-k3
PKG_VERSION:=1
PKG_RELEASE:=4

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/brcmfmac-firmware-4366c0-pcie-k3
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=Broadcom 4366c0 FullMac PCIe firmware for Phicomm K3
  CONFLICTS:=brcmfmac-firmware-4366c0-pcie
  DEPENDS:=@TARGET_bcm53xx
endef

define Build/Compile
endef

define Package/brcmfmac-firmware-4366c0-pcie-k3/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) ./files/brcmfmac4366c-pcie.bin $(1)/lib/firmware/brcm/brcmfmac4366c-pcie.bin
endef

$(eval $(call BuildPackage,brcmfmac-firmware-4366c0-pcie-k3))
