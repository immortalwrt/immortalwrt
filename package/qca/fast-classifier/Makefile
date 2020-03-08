include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fast-classifier
PKG_SOURCE_PROTO:=git
PKG_BRANCH:=master
PKG_RELEASE:=1

LOCAL_SRC:=$(TOPDIR)/package/qca/shortcut-fe/src/
include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_REV:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
  PKG_VERSION:=g$(PKG_REV)
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)/Default
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@IPV6 +kmod-ipt-conntrack +kmod-shortcut-fe
  TITLE:=Kernel driver for FAST Classifier
  FILES:=$(PKG_BUILD_DIR)/fast-classifier/fast-classifier.ko
  KCONFIG:=CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y
  PROVIDES:=$(PKG_NAME)
endef

define KernelPackage/$(PKG_NAME)
  $(call KernelPackage/$(PKG_NAME)/Default)
endef

define KernelPackage/$(PKG_NAME)-noload
  $(call KernelPackage/$(PKG_NAME)/Default)
endef

define KernelPackage/$(PKG_NAME)/Default/description
FAST Classifier talks to SFE to make decisions about offloading connections
endef

define KernelPackage/$(PKG_NAME)/description
$(call KernelPackage/$(PKG_NAME)/Default/description)
endef

define KernelPackage/$(PKG_NAME)-noload/description
$(call KernelPackage/$(PKG_NAME)/Default/description)

This package does not load $(PKG_NAME) at boot by default
endef

SFE_MAKE_OPTS:=SFE_SUPPORT_IPV6=y

define Build/Compile/kmod
	$(MAKE) -C "$(LINUX_DIR)" $(strip $(SFE_MAKE_OPTS)) \
		CONFIG_FAST_CLASSIFIER=m \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/fast-classifier" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS) -Wno-implicit-function-declaration" \
		modules
endef

define Build/Compile
	$(Build/Compile/kmod)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/fast-classifier/fast-classifier.h $(1)/usr/include/
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
$(eval $(call KernelPackage,$(PKG_NAME)-noload))
