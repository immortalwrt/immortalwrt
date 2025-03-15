#
# Copyright (C) 2016 MediaTek
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bpir3_mini-properties
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/bpir3_mini-properties
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=PHYs firmware & fan control for BPI-R3-MINI 
endef

define Package/bpir3_mini-properties/description
  This package install PHYs firmware & fan control for BPI-R3-MINI.
endef

define Package/bpir3_mini-properties/config
endef

define Build/Compile
endef

define Package/bpir3_mini-properties/install
	$(INSTALL_DIR) $(1)/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/lib/firmware/
	$(INSTALL_BIN) ./files/bpir3_fanspeed $(1)/sbin/
	$(INSTALL_BIN) ./files/fanspeed.init $(1)/etc/init.d/fanspeed
	$(INSTALL_BIN) ./files/ltecalling.init $(1)/etc/init.d/ltecalling
	$(INSTALL_DATA) ./files/EthMD32.dm.bin $(1)/lib/firmware/
	$(INSTALL_DATA) ./files/EthMD32.DSP.bin $(1)/lib/firmware/
endef

$(eval $(call BuildPackage,bpir3_mini-properties))
