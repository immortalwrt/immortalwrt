include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fast-classifier
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/fast-classifier
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  TITLE:=Kernel driver for FAST Classifier
  FILES:=$(PKG_BUILD_DIR)/fast-classifier.ko
  KCONFIG:= \
	CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y \
	CONFIG_NF_CONNTRACK_MARK=y
  DEPENDS:=+kmod-ipt-conntrack +kmod-shortcut-fe
  PROVIDES:=fast-classifier
endef

define KernelPackage/fast-classifier/description
  FAST Classifier talks to SFE to make decisions about offloading connections
endef

define Package/fast-classifier-example
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Example user space program for fast-classifier
  DEPENDS:=+kmod-fast-classifier +libnl
endef

define Package/fast-classifier-example/description
  Example user space program that communicates with fast
  classifier kernel module.
endef

SFE_MAKE_OPTS:=SFE_SUPPORT_IPV6=y
EXTRA_CFLAGS+=-I$(STAGING_DIR)/usr/include/shortcut-fe

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" $(strip $(SFE_MAKE_OPTS)) \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		CONFIG_FAST_CLASSIFIER=m \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		modules

ifneq ($(CONFIG_PACKAGE_fast-classifier-example),)
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/userspace_fast_classifier \
		-I $(PKG_BUILD_DIR) \
		-I$(STAGING_DIR)/usr/include/libnl \
		-I$(STAGING_DIR)/usr/include/libnl3 \
		-lnl-genl-3 -lnl-3 \
		$(PKG_BUILD_DIR)/nl_classifier_test.c
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/fast-classifier.h $(1)/usr/include/
endef

define Package/fast-classifier-example/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/userspace_fast_classifier $(1)/sbin/
endef

$(eval $(call KernelPackage,fast-classifier))
$(eval $(call BuildPackage,fast-classifier-example))
