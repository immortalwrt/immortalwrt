#
# Copyright (C) 2017 McMCC
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dvb-firmware
PKG_VERSION:=1.0
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/dvb-firmware
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=dvb-firmware megapack
endef

define Package/dvb-firmware/description
  dvb-firmware is specialised software that is both specific to some hardware
  and also integral for its proper functioning.
endef

define Build/Compile
endef

define Package/dvb-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(CP) $(CURDIR)/files/* $(1)/lib/firmware/
endef

$(eval $(call BuildPackage,dvb-firmware))
