include $(TOPDIR)/rules.mk

PKG_NAME:=rtl88x2bu
PKG_VERSION:=5.6.1
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/cilynx/rtl88x2bu.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-06-05
PKG_SOURCE_VERSION:=64aa087c5ead11960425125c955dcfe98233202e
PKG_MIRROR_HASH:=3532897af3cd3b327c825708d1c4db750c171647ddeaf21fb003f575f6b1ec82

PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/autoconf.h \
	-include backport/backport.h

NOSTDINC_FLAGS+=-DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_CONCURRENT_MODE -DBUILD_OPENWRT

define KernelPackage/rtl88x2bu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek 8812BU/8822BU support
  DEPENDS:=+@USB_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT +kmod-mac80211 +kmod-usb-core
  FILES:=$(PKG_BUILD_DIR)/rtl88x2bu.ko
  AUTOLOAD:=$(call AutoProbe,rtl88x2bu)
  PROVIDES:=kmod-rtl88x2bu
endef

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
		USER_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CONFIG_RTL8822BU=m \
		USER_MODULE_NAME=rtl88x2bu \
		modules
endef

$(eval $(call KernelPackage,rtl88x2bu))
