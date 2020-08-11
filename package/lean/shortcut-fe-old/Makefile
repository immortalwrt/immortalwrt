#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=shortcut-fe-old
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/shortcut-fe-old
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@LINUX_4_9 @IPV6
  TITLE:=Kernel driver for SFE
  FILES:=$(PKG_BUILD_DIR)/shortcut-fe.ko $(PKG_BUILD_DIR)/shortcut-fe-ipv6.ko
  KCONFIG:=CONFIG_NF_CONNTRACK_EVENTS=y CONFIG_SHORTCUT_FE=y
  AUTOLOAD:=$(call AutoProbe,shortcut-fe shortcut-fe-ipv6)
endef

define KernelPackage/shortcut-fe-old/Description
Shortcut is an in-Linux-kernel IP packet forwarding engine.
endef

define KernelPackage/shortcut-fe-old-cm
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=+kmod-ipt-conntrack +kmod-shortcut-fe-old
  TITLE:=Kernel driver for SFE
  FILES:=$(PKG_BUILD_DIR)/shortcut-fe-cm.ko
  KCONFIG:=CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y CONFIG_NF_CONNTRACK_MARK=y
  AUTOLOAD:=$(call AutoProbe,shortcut-fe-cm)
endef

define KernelPackage/shortcut-fe-old-cm/Description
Simple connection manager for the Shortcut forwarding engine.
endef

define KernelPackage/fast-classifier-old
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=+kmod-ipt-conntrack +kmod-shortcut-fe-old
  TITLE:=Kernel driver for FAST Classifier
  FILES:=$(PKG_BUILD_DIR)/fast-classifier.ko
  KCONFIG:=CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y CONFIG_NF_CONNTRACK_MARK=y
  AUTOLOAD:=$(call AutoLoad,z,fast-classifier)
  PROVIDES:=$(PKG_NAME)
endef

define KernelPackage/fast-classifier-old/description
FAST Classifier connection manager for Shortcut forwarding engine. 
It talks to SFE to make decisions about offloading connections.
endef

define Package/fast-classifier-old-example
  TITLE:=Example user space program for fast-classifier
  DEPENDS:=+libnl +kmod-fast-classifier-old
endef

define Package/fast-classifier-old-example/description
Example user space program that communicates with fast
classifier kernel module
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)"

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
	$(if $(CONFIG_PACKAGE_fast-classifier-old-example),$(Build/Compile/fast-classifier-old-example))
endef

define Build/Compile/fast-classifier-old-example
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/userspace_fast_classifier \
		-I $(PKG_BUILD_DIR) \
		-I$(STAGING_DIR)/usr/include/libnl \
		-I$(STAGING_DIR)/usr/include/libnl3 \
		-lnl-genl-3 -lnl-3 \
		$(PKG_BUILD_DIR)/nl_classifier_test.c
endef

ifneq ($(CONFIG_PACKAGE_kmod-shortcut-fe-old)$(CONFIG_PACKAGE_kmod-shortcut-fe-old-cm)$(CONFIG_PACKAGE_kmod-fast-classifier-old),)
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/shortcut-fe
	$(CP) -rf $(PKG_BUILD_DIR)/sfe.h $(1)/usr/include/shortcut-fe
ifneq ($(CONFIG_PACKAGE_kmod-fast-classifier),)
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/fast-classifier.h $(1)/usr/include/
endif
endef
endif

define Package/fast-classifier-old-example/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/userspace_fast_classifier $(1)/sbin/
endef

$(eval $(call KernelPackage,shortcut-fe-old))
$(eval $(call KernelPackage,shortcut-fe-old-cm))
$(eval $(call KernelPackage,fast-classifier-old))
#$(eval $(call BuildPackage,fast-classifier-old-example))
