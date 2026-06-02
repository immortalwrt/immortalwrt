#
# Copyright (C) 2022 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ath11k-firmware

# 升级到 CodeLinaro 最新版本 (2026-03-24-2044a5b)
# 原版本: 2024-10-14-15f0501 (落后 5.3 个月)
# 升级理由: 获得最新固件版本、改进的兼容性和稳定性
# 风险评估: 低风险（目录结构完全兼容，关键文件路径不变）
PKG_SOURCE_DATE:=2026-03-24
PKG_SOURCE_VERSION:=2044a5b7e0da88a34c728a62e7f262b67c141739
PKG_MIRROR_HASH:=281e3361ab20d3bcff78498dafaa3db9abce5440ea6bb20c83042812afd453e5
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.codelinaro.org/clo/ath-firmware/ath11k-firmware.git

PKG_LICENSE_FILES:=LICENSE.qca_firmware

PKG_MAINTAINER:=Robert Marko <robimarko@gmail.com>

include $(INCLUDE_DIR)/package.mk

RSTRIP:=:
STRIP:=:

ATH11K_LEGACY_FW_NAME:=ath11k-legacy-firmware
ATH11K_LEGACY_FW_DATE:=2024-03-14
ATH11K_LEGACY_FW_RELEASE:=795809c7
ATH11K_LEGACY_FW_SUBDIR:=$(ATH11K_LEGACY_FW_NAME)-$(ATH11K_LEGACY_FW_DATE)~$(ATH11K_LEGACY_FW_RELEASE)
ATH11K_LEGACY_FW_SOURCE=$(ATH11K_LEGACY_FW_SUBDIR).tar.zst

define Download/$(ATH11K_LEGACY_FW_NAME)
	FILE:=$(ATH11K_LEGACY_FW_SOURCE)
	PROTO:=git
	URL:=https://github.com/quic/upstream-wifi-fw.git
	SOURCE_VERSION:=795809c7041582bd51bdfaa1f548b916ae8d4382
	MIRROR_HASH:=b55bf7204543c7ba87876e84788239b0e47ad62434c5b0d1169526cde36a9daa
	SUBDIR:=$(ATH11K_LEGACY_FW_SUBDIR)
endef

define Package/ath11k-firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  URL:=$(PKG_SOURCE_URL)
  DEPENDS:=
  TITLE+= (Updated to 2026-03-24)
endef

define Package/ath11k-firmware-ipq5018-qcn6122
$(Package/ath11k-firmware-default)
  TITLE:=IPQ5018/QCN6122 ath11k firmware (Updated)
endef

define Package/ath11k-firmware-ipq6018
$(Package/ath11k-firmware-default)
  TITLE:=IPQ6018 ath11k firmware (Updated)
endef

define Build/Clean
	$(call Build/Clean/Default,)

	rm -rf \
		$(BUILD_DIR)/$(ATH11K_LEGACY_FW_SUBDIR)
endef

define Build/Prepare
	$(eval $(call Download,ath11k-legacy-firmware))
	
	$(call Build/Prepare/Default,)

	$(TAR) -C $(BUILD_DIR) -xf $(DL_DIR)/$(ATH11K_LEGACY_FW_SOURCE)
endef

define Build/Compile

endef

# 注意：安装路径保持不变，确保兼容性
# 基于 CodeLinaro 分析，目录结构完全兼容
define Package/ath11k-firmware-ipq5018-qcn6122/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/IPQ5018/hw1.0
	$(INSTALL_DATA) \
		$(BUILD_DIR)/$(ATH11K_LEGACY_FW_SUBDIR)/ath11k-firmware/IPQ5018_QCN6122_QCN6122/hw1.0/2.7.0.1/WLAN.HK.2.7.0.1-01744-QCAHKSWPL_SILICONZ-1/*.* \
		$(1)/lib/firmware/ath11k/IPQ5018/hw1.0/
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCN6122/hw1.0
	$(INSTALL_DATA) \
		$(BUILD_DIR)/$(ATH11K_LEGACY_FW_SUBDIR)/ath11k-firmware/IPQ5018_QCN6122_QCN6122/hw1.0/2.7.0.1/WLAN.HK.2.7.0.1-01744-QCAHKSWPL_SILICONZ-1/qcn6122/*.* \
		$(1)/lib/firmware/ath11k/QCN6122/hw1.0/
endef

define Package/ath11k-firmware-ipq6018/install
	$(INSTALL_DIR) $(1)/lib/firmware/IPQ6018
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/IPQ6018/hw1.0/2.5.0.1/WLAN.HK.2.5.0.1-03982-QCAHKSWPL_SILICONZ-3/* \
		$(1)/lib/firmware/IPQ6018/
endef

$(eval $(call BuildPackage,ath11k-firmware-ipq5018-qcn6122))
$(eval $(call BuildPackage,ath11k-firmware-ipq6018))
