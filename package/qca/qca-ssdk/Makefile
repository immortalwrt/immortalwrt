
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-ssdk
PKG_SOURCE_PROTO:=git
PKG_BRANCH:=master
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_VERSION:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
endif
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_VERSION)

LOCAL_VARIANT=$(patsubst qca-ssdk-%,%,$(patsubst qca-ssdk-%,%,$(BUILD_VARIANT)))
include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-ssdk/default-nohnat
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Devices
  TITLE:=Kernel driver for QCA SSDK
  FILES:=$(PKG_BUILD_DIR)/build/bin/qca-ssdk.ko
  AUTOLOAD:=$(call AutoLoad,30,qca-ssdk)
  PROVIDES:=qca-ssdk
endef

define KernelPackage/qca-ssdk-nohnat
$(call KernelPackage/qca-ssdk/default-nohnat)
  DEPENDS:=@!TARGET_ipq40xx||TARGET_ipq806x||TARGET_ar71xx||TARGET_ath79 +kmod-nf-conntrack +kmod-ppp
  VARIANT:=nohnat
endef

define KernelPackage/qca-ssdk-nohnat/Description
This package contains a qca-ssdk driver for QCA chipset
endef

define KernelPackage/qca-ssdk-hnat
$(call KernelPackage/qca-ssdk/default-nohnat)
  DEPENDS:=@TARGET_ipq806x||TARGET_ar71xx||TARGET_ath79 +kmod-ipt-extra +kmod-ipt-filter +kmod-ppp
  TITLE+= (hnat)
  VARIANT:=hnat
endef

define KernelPackage/qca-ssdk-hnat/Description
This package contains a qca-ssdk-hnat driver for QCA chipset
endef

GCC_VERSION=$(shell echo "$(CONFIG_GCC_VERSION)" | sed 's/[^0-9.]*\([0-9.]*\).*/\1/')
ifeq ($(GCC_VERSION), 4.8)
GCC_VERSION=4.8.3
endif

ifdef CONFIG_TOOLCHAIN_BIN_PATH
TOOLCHAIN_BIN_PATH=$(CONFIG_TOOLCHAIN_BIN_PATH)
else
TOOLCHAIN_BIN_PATH=$(TOOLCHAIN_DIR)/bin
endif

ifdef CONFIG_TARGET_NAME
QCASSDK_CONFIG_OPTS+= \
		      TARGET_NAME=$(CONFIG_TARGET_NAME)
endif

QCASSDK_CONFIG_OPTS+= TOOL_PATH=$(TOOLCHAIN_BIN_PATH) \
                SYS_PATH=$(LINUX_DIR) \
                TOOLPREFIX=$(TARGET_CROSS) \
                KVER=$(LINUX_VERSION) \
                ARCH=$(LINUX_KARCH) \
		TARGET_SUFFIX=$(CONFIG_TARGET_SUFFIX) \
		GCC_VERSION=$(GCC_VERSION) \
		CFLAGS=-I$(STAGING_DIR)/usr/include

ifeq ($(LOCAL_VARIANT),hnat)
    QCASSDK_CONFIG_OPTS+= HNAT_FEATURE=enable
endif

ifeq ($(CONFIG_TARGET_ath79),)
    QCASSDK_CONFIG_OPTS+= ATH79_FIX=enable
endif

ifeq ($(CONFIG_TARGET_ipq806x_QSDK_Standard),)
    QCASSDK_CONFIG_OPTS+= HK_CHIP=enable
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/api
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/ref
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/fal
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/sal
	$(INSTALL_DIR) $(1)/usr/include/qca-ssdk/init
	$(CP) -rf $(PKG_BUILD_DIR)/include/api/sw_ioctl.h $(1)/usr/include/qca-ssdk/api
	if [ -f $(PKG_BUILD_DIR)/include/ref/ref_vsi.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/ref/ref_vsi.h $(1)/usr/include/qca-ssdk/ref/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/ref/ref_fdb.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/ref/ref_fdb.h $(1)/usr/include/qca-ssdk/ref/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/ref/ref_port_ctrl.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/ref/ref_port_ctrl.h $(1)/usr/include/qca-ssdk/ref/; \
	fi
	if [ -f $(PKG_BUILD_DIR)/include/init/ssdk_init.h ]; then \
	$(CP) -rf $(PKG_BUILD_DIR)/include/init/ssdk_init.h $(1)/usr/include/qca-ssdk/init/; \
	fi
	$(CP) -rf $(PKG_BUILD_DIR)/include/fal $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/common $(1)/usr/include/qca-ssdk
	$(CP) -rf $(PKG_BUILD_DIR)/include/sal/os $(1)/usr/include/qca-ssdk/sal
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(strip $(QCASSDK_CONFIG_OPTS))
endef

define KernelPackage/qca-ssdk-nohnat/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qca-ssdk $(1)/etc/init.d/qca-ssdk
endef

define KernelPackage/qca-ssdk-hnat/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/qca-ssdk $(1)/etc/init.d/qca-ssdk
endef

$(eval $(call KernelPackage,qca-ssdk-nohnat))
$(eval $(call KernelPackage,qca-ssdk-hnat))
