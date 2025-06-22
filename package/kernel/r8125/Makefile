include $(TOPDIR)/rules.mk

PKG_NAME:=r8125
PKG_VERSION:=9.016.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8125/releases/download/$(PKG_VERSION)
PKG_HASH:=cd1955dd07d2f5a6faaa210ffc4e8af992421295a32ab6ddcfa759bed9eba922

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8125
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8125 PCI 2.5 Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8125.ko
  AUTOLOAD:=$(call AutoProbe,r8125,1)
  PROVIDES:=kmod-r8169 kmod-r8125-rss
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		CONFIG_ASPM=n \
		ENABLE_MULTIPLE_TX_QUEUE=y \
		ENABLE_RSS_SUPPORT=y \
		modules
endef

$(eval $(call KernelPackage,r8125))
