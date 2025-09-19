include $(TOPDIR)/rules.mk

PKG_NAME:=photonicat-firmware
PKG_RELEASE:=1

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/photonicat-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=$(1) firmware for Photonicat
  DEPENDS:=@TARGET_rockchip_armv8 $(2)
  CONFLICTS:=$(3)
endef

define Build/Compile
endef

Package/ath10k-board-qca9377-photonicat = $(call Package/photonicat-firmware-default,ath10k qca9377 board sdio,,ath10k-board-qca9377-sdio)
define Package/ath10k-board-qca9377-photonicat/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_DATA) \
		$(CURDIR)/files/board.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/board.bin
endef
$(eval $(call BuildPackage,ath10k-board-qca9377-photonicat))

Package/ath10k-firmware-qca9377-photonicat = $(call Package/photonicat-firmware-default,ath10k qca9377 sdio,+ath10k-board-qca9377-photonicat,ath10k-firmware-qca9377-sdio)
define Package/ath10k-firmware-qca9377-photonicat/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9377/hw1.0
	$(INSTALL_DATA) \
		$(CURDIR)/files/firmware-sdio-5.bin \
		$(1)/lib/firmware/ath10k/QCA9377/hw1.0/firmware-sdio-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9377-photonicat))
