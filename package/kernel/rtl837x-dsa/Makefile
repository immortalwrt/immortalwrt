# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=dsa-rtl837x
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/airjinkela/rtl837x-dsa-driver.git
PKG_SOURCE_DATE:=2026-10-01
PKG_SOURCE_VERSION:=7f3b64c7f3a37db1866fd80ba815d89974d995fd
PKG_MIRROR_HASH:=f85646325990ad5a1142b89e99edfa68c6225c4acb2dbc172dee1bdb79bd94e8

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=StarField Xu <air_jinkela@163.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/dsa-rtl837x
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL837x switch DSA support
  KCONFIG:= \
	CONFIG_NET_DSA_TAG_RTL8_4=y \
	CONFIG_NET_DSA_TAG_MXL_862XX_8021Q=y
  FILES:=$(PKG_BUILD_DIR)/src/rtl8372n_dsa.ko
  DEPENDS:=@TARGET_mediatek_filogic
  AUTOLOAD:=$(call AutoLoad,42,rtl8372n_dsa,1)
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_RTL8372N_DSA=m \
		CONFIG_RTL837X_PHY_PATCH=y \
		CONFIG_RTL8372N_DSA_DEBUG=n \
		modules
endef

$(eval $(call KernelPackage,dsa-rtl837x))
