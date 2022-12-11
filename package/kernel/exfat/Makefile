#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=exfat
PKG_VERSION:=6.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/namjaejeon/linux-exfat-oot/tar.gz/$(PKG_VERSION)?
PKG_HASH:=08513c4ad7bceda6570435f54e76fc91369cbf404968838bdd3f566495f20f36
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/linux-exfat-oot-$(PKG_VERSION)

PKG_MAINTAINER:=
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-exfat
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Filesystems
  TITLE:=exFAT kernel module
  URL:=https://github.com/namjaejeon/linux-exfat-oot
  FILES:=$(PKG_BUILD_DIR)/exfat.ko
  AUTOLOAD:=$(call AutoProbe,exfat)
  DEPENDS:= +kmod-nls-base
endef

define KernelPackage/exfat/description
  This package provides the kernel module for exfat.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(PKG_EXTRA_KCONFIG) \
	CONFIG_EXFAT_FS=m \
	modules
endef

$(eval $(call KernelPackage,fs-exfat))
