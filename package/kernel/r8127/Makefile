include $(TOPDIR)/rules.mk

PKG_NAME:=r8127
PKG_VERSION:=11.014.00
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/openwrt/rtl8127/releases/download/$(PKG_VERSION)
PKG_HASH:=f496bc16c32d2e8f9482c57d006604c70d9e8d55b4f1f999b88c602de9104094

PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPLv2
PKG_MAINTAINER:=Alvaro Fernandez Rojas <noltari@gmail.com>

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8127
  SUBMENU:=Network Devices
  TITLE:=Realtek RTL8127 PCI 10 Gigabit Ethernet driver
  DEPENDS:=@PCI_SUPPORT +kmod-libphy
  FILES:=$(PKG_BUILD_DIR)/src/r8127.ko
  AUTOLOAD:=$(call AutoProbe,r8127,1)
  PROVIDES:=kmod-r8169 kmod-r8127-rss
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)/src" \
		ENABLE_MULTIPLE_TX_QUEUE=y \
		ENABLE_RSS_SUPPORT=y \
		modules
endef

$(eval $(call KernelPackage,r8127))
