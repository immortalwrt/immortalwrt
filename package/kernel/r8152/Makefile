#Download realtek r8152 linux driver from official site [https://www.realtek.com/en/component/zoo/category/network-interface-controllers-10-100-1000m-gigabit-ethernet-usb-3-0-software]
#Unpack source file
#Replace orginal Makefile with this file
#Put this source to 'package' folder of OpenWRT/LEDE SDK
#Build(make menuconfig, make defconfig, make)

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8152
PKG_VERSION:=2.22.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@IMMORTALWRT
PKG_HASH:=7ed3094264e7e660b4f2f4c772252028c12788a0bef26be1553870716821b834
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-v$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/usb-net-rtl8152-vendor
  TITLE:=Kernel module for USB-to-Ethernet Realtek convertors
  SUBMENU:=USB Support
  DEPENDS:=+kmod-usb-core +kmod-libphy
  CONFLICTS:=kmod-usb-net-rtl8152
  FILES:=$(PKG_BUILD_DIR)/r8152.ko
  AUTOLOAD:=$(call AutoProbe,r8152,1)
endef

define KernelPackage/usb-net-rtl8152-vendor/description
  Kernel module for Realtek RTL8152/RTL8153 Based USB Ethernet Adapters
endef

define Build/Compile
	+$(KERNEL_MAKE) M=$(PKG_BUILD_DIR) modules
endef

$(eval $(call KernelPackage,usb-net-rtl8152-vendor))
