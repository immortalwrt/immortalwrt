include $(TOPDIR)/rules.mk

PKG_NAME:=rtl88x2bu
PKG_VERSION:=5.6.1
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/cilynx/rtl88x2bu.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-07-21
PKG_SOURCE_VERSION:=a2abcaf523dfab2afeda8cc2cdb5e3c9510fc5f3
PKG_MIRROR_HASH:=ba79fb9a823a38c5acf22999210645ba881da0b5498ee765383dc5eaa8234e62

PKG_MAINTAINER:=[CTCGFW] Project OpenWrt
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl88x2bu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek 8812BU/8822BU support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl88x2bu.ko
  AUTOLOAD:=$(call AutoProbe,rtl88x2bu)
  PROVIDES:=kmod-rtl88x2bu
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/backport.h

EXTRA_CFLAGS=-DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_RADIO_WORK -DCONFIG_CONCURRENT_MODE 

# CONFIG_IOCTL_CFG80211: If kernel version is after 2.6.35, we recommend user to use the new nl80211 API.
# RTW_USE_CFG80211_STA_EVENT:It is used for driver to indicate new cfg80211 STA event, 
#		which is required by wpa_supplicant_8. Linux kernel supports this feature after kernel 3.2.
# CONFIG_RADIO_WORK:It is used for driver to fit ‘radio work’ mechanism of wpa_supplicant_8_L & wpa_supplicant_8_M.
# CONFIG_CONCURRENT_MODE:Realtek’s Linux Wi-Fi driver can support Station+AP and Station+P2P single channel concurrent mode.

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		USER_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CONFIG_RTL8822BU=m \
		USER_MODULE_NAME=rtl88x2bu \
		modules
endef

$(eval $(call KernelPackage,rtl88x2bu))
