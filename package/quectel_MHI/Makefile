#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pcie_mhi
PKG_VERSION:=2011-07-29-1026
PKG_RELEASE:=1


PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/pcie_mhi
  CATEGORY:=PCIEMHI
  DEPENDS:=
  TITLE:=pcie_mhi
  FILES:= $(PKG_BUILD_DIR)/pcie_mhi.ko
  AUTOLOAD:=$(call AutoLoad,90,pcie_mhi)
endef

define KernelPackage/pcie_mhi/Description
driver for pcie_mhi 5g modem
endef

define Build/Prepare
	$(CP) src/* $(PKG_BUILD_DIR)
	$(call Build/Prepare/Default)
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,pcie_mhi))


