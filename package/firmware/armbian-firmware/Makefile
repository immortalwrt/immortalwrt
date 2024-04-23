include $(TOPDIR)/rules.mk

PKG_NAME:=armbian-firmware
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2023-07-11
PKG_SOURCE_URL:=https://github.com/armbian/firmware.git
PKG_SOURCE_VERSION:=a5e591c919a280d05f3bfcdc5892ea5788d020f6
PKG_MIRROR_HASH:=7fe84fe5ad118b22083546fef10fcc81dcabbe880b9cb601f681839047f780d7

include $(INCLUDE_DIR)/package.mk

RSTRIP:=:
STRIP:=:

define Package/armbian-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=https://github.com/armbian/firmware
  TITLE:=$(1)
  DEPENDS:=$(2)
endef

define Build/Compile
endef

Package/brcmfmac-firmware-4356-sdio = $(call Package/armbian-firmware-default,Broadcom BCM4356 FullMac SDIO firmware)
define Package/brcmfmac-firmware-4356-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/BCM4356A2.hcd \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4356-sdio.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4356-sdio.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-4356-sdio))

Package/brcmfmac-firmware-43456-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43456 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43456-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43456-sdio))

Package/brcmfmac-firmware-43752-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43752 FullMac SDIO firmware)
define Package/brcmfmac-firmware-43752-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/BCM4362A2.hcd \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-sdio.bin \
		$(1)/lib/firmware/brcm/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-sdio.clm_blob \
		$(1)/lib/firmware/brcm/
endef
$(eval $(call BuildPackage,brcmfmac-firmware-43752-sdio))

Package/brcmfmac-nvram-4356-sdio = $(call Package/armbian-firmware-default,Broadcom BCM4356 NVRAM firmware)
define Package/brcmfmac-nvram-4356-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac4356-sdio.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac4356-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac4356-sdio.friendlyarm,nanopc-t4.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-4356-sdio))

Package/brcmfmac-nvram-43456-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43456 NVRAM firmware)
define Package/brcmfmac-nvram-43456-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43456-sdio.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac43456-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac43456-sdio.radxa,rockpi4a.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-43456-sdio))

Package/brcmfmac-nvram-43752-sdio = $(call Package/armbian-firmware-default,Broadcom BCM43752 NVRAM firmware)
define Package/brcmfmac-nvram-43752-sdio/install
	$(INSTALL_DIR) $(1)/lib/firmware/brcm
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/brcm/brcmfmac43752-sdio.txt \
		$(1)/lib/firmware/brcm/
	$(LN) \
		brcmfmac43752-sdio.txt \
		$(1)/lib/firmware/brcm/brcmfmac43752-sdio.firefly,rk3568-roc-pc.txt
endef
$(eval $(call BuildPackage,brcmfmac-nvram-43752-sdio))
