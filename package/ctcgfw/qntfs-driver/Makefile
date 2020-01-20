#
# Copyright (C) 2005-2019 lintel.huang@gmail.com
#
# This is non-free software, licensed under author,more code from Apple/Tuxera
#
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fs-qntfs
PKG_RELEASE:=4
PKG_MAINTAINER:=lintel <lintel.huang@gmail.com>

USE_SRC=$(shell ls ./src/ 2>/dev/null >/dev/null && echo 1)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fs-qntfs
  SUBMENU:=Filesystems
  TITLE:=PandoraBox Quick-NTFS filesystem
  FILES:=$(PKG_BUILD_DIR)/qntfs.ko
  DEPENDS:=@(TARGET_ar71xx||TARGET_ath79||TARGET_ipq40xx||TARGET_ipq806x||TARGET_ramips) \
	+kmod-nls-cp936 +kmod-nls-utf8 +kmod-nls-cp437 +kmod-nls-iso8859-1
  AUTOLOAD:=$(call AutoLoad,30,qntfs,1)
endef

define KernelPackage/fs-qntfs/description
  a Kernel module drvier base on NTFS-3G support NTFS
endef

EXTRA_KCONFIG:= \
	CONFIG_QNTFS_FS=m \
	CONFIG_QNTFS_SYMLINKS=y

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	$(EXTRA_KCONFIG)

ifneq ($(USE_SRC),)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules

	@echo ----------Build:$(CONFIG_TARGET_BOARD)-$(CONFIG_TARGET_SUBTARGET)----------
	mkdir -p ./files/$(CONFIG_TARGET_BOARD)-$(CONFIG_TARGET_SUBTARGET)
	$(CP) $(PKG_BUILD_DIR)/qntfs.ko  ./files/$(CONFIG_TARGET_BOARD)-$(CONFIG_TARGET_SUBTARGET)/
endef

else

define Build/Prepare
	@echo ----------Build:$(CONFIG_TARGET_BOARD)-$(CONFIG_TARGET_SUBTARGET)----------
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) ./files/$(CONFIG_TARGET_BOARD)-$(CONFIG_TARGET_SUBTARGET)/qntfs.ko $(PKG_BUILD_DIR)/
endef

define Build/Compile
	true
endef

endif

$(eval $(call KernelPackage,fs-qntfs))
