include $(TOPDIR)/rules.mk

PKG_NAME:=rtl88x2bu
PKG_RELEASE=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_SOURCE_URL:=https://github.com/cilynx/rtl88x2BU_WiFi_linux_v5.3.1_27678.20180430_COEX20180427-5959.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2019-09-23
PKG_SOURCE_VERSION:=eece0638c546cd303da518c505cbe93513595030
PKG_MIRROR_HASH:=0bff49dfab9c3a287a83802d631b2151ebecdc52da1f2407f763c75e64548cac

PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl88x2bu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek 8812BU/8822BU support
  DEPENDS:=@USB_SUPPORT +kmod-cfg80211 +kmod-mac80211 +kmod-usb-core +wireless-tools +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/88x2bu.ko
  AUTOLOAD:=$(call AutoProbe,88x2bu)
endef

EXTRA_CFLAGS=-DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_RADIO_WORK -DCONFIG_CONCURRENT_MODE 

# CONFIG_IOCTL_CFG80211: If kernel version is after 2.6.35, we recommend user to use the new nl80211 API.
# RTW_USE_CFG80211_STA_EVENT:It is used for driver to indicate new cfg80211 STA event, 
#		which is required by wpa_supplicant_8. Linux kernel supports this feature after kernel 3.2.
# CONFIG_RADIO_WORK:It is used for driver to fit ‘radio work’ mechanism of wpa_supplicant_8_L & wpa_supplicant_8_M.
# CONFIG_CONCURRENT_MODE:Realtek’s Linux Wi-Fi driver can support Station+AP and Station+P2P single channel concurrent mode.

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		USER_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		CONFIG_RTL8822BU=m \
		modules
endef

$(eval $(call KernelPackage,rtl88x2bu))
