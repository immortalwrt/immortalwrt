#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=conninfra
PKG_VERSION:=bbf588
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)
PKG_MAINTAINER:=Kun-Ze Syue<kun-ze.syue@mediatek.com>

PKG_KCONFIG:= \
	MTK_CONNINFRA_APSOC \
	MTK_CONNINFRA_APSOC_MT7986 \
	MTK_CONNINFRA_APSOC_MT7981 \
	CONNINFRA_EMI_SUPPORT \
	FPGA_EARLY_PORTING \
	CONNINFRA_AUTO_UP

PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
define KernelPackage/conninfra
	CATEGORY:=MTK Properties
	TITLE:= Conninfra driver
	FILES:=$(PKG_BUILD_DIR)/conninfra.ko
	AUTOLOAD:=$(call AutoLoad,10,conninfra,1)
	SUBMENU:=Drivers
	MENU:=1
endef

define KernelPackage/conninfra/description
	Support for connectivity conninfra driver.
endef

define KernelPackage/conninfra/config
if PACKAGE_kmod-conninfra
	config MTK_CONNINFRA_APSOC
	bool "Conninfra APSOC Only"
	default n

if MTK_CONNINFRA_APSOC
choice
	prompt "Choose APSOC Chip"

	config MTK_CONNINFRA_APSOC_MT7986
	bool "MT7986"

	config MTK_CONNINFRA_APSOC_MT7981
	bool "MT7981"
endchoice

	config CONNINFRA_EMI_SUPPORT
	bool "EMI Support"
	default n

	config FPGA_EARLY_PORTING
	bool "Only for FPGA Stage"
	default n

	config CONNINFRA_AUTO_UP
	bool "Conninfra Up by Self"
	default n
endif
endif
endef

# Specify what needs to be done to prepare for building the package.

# Specify where and how to install the program.
#define Package/kmod-conninfra/install
#  true
#endef

# Transfer local kernel config to compile option
KCONFIG_FLAGS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=$(CONFIG_$(c))))

$(info $$KCONFIG_FLAGS is [${KCONFIG_FLAGS}])

# Transfer local kernel config to compile option
EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(KCONFIG_FLAGS)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(KCONFIG_FLAGS)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=0, $(patsubst %=n,%,$(filter %=n,$(KCONFIG_FLAGS)))) \

EXTRA_CFLAGS+=-DEEPROM_NAME=${CONFIG_first_card_EEPROM_name}

$(info EXTRA_CFLAGS=${EXTRA_CFLAGS})

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	SUBDIRS="$(PKG_BUILD_DIR)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(KCONFIG_FLAGS)

define Build/Compile
	 $(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,conninfra))
