# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2020 Lean <coolsnowwolf@gmail.com>
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=autocore
PKG_FLAGS:=nonshared
PKG_RELEASE:=$(COMMITCOUNT)

PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_bcm27xx \
	CONFIG_TARGET_bcm53xx

include $(INCLUDE_DIR)/package.mk

define Package/autocore-arm
  TITLE:=Arm auto core loadbalance script
  MAINTAINER:=CN_SZTL
  DEPENDS:=@(arm||aarch64) \
    +TARGET_bcm27xx:bcm27xx-userland \
    +TARGET_bcm53xx:nvram
  VARIANT:=arm
endef

define Package/autocore-x86
  TITLE:=x86/x64 auto core loadbalance script
  MAINTAINER:=Lean
  DEPENDS:=@TARGET_x86 +lm-sensors +ethtool
  VARIANT:=x86
endef

define Build/Compile
endef

define Package/autocore/install/Default
	$(INSTALL_DIR) $(1)/etc
	$(CP) ./files/generic/index.htm $(1)/etc/

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/generic/090-cover-index_htm $(1)/etc/uci-defaults/

	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/generic/cpuinfo $(1)/sbin/
	$(INSTALL_BIN) ./files/generic/ethinfo $(1)/sbin/
endef

define Package/autocore-arm/install
	$(call Package/autocore/install/Default,$(1))

ifneq (, $(findstring $(BOARD), ipq40xx ipq806x))
	$(INSTALL_BIN) ./files/arm/tempinfo $(1)/sbin/
endif
endef

define Package/autocore-x86/install
	$(call Package/autocore/install/Default,$(1))

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/x86/autocore $(1)/etc/init.d/
endef

$(eval $(call BuildPackage,autocore-arm))
$(eval $(call BuildPackage,autocore-x86))
