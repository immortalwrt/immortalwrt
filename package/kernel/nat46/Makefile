include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=nat46

PKG_SOURCE_URL:=https://github.com/ayourtch/nat46.git
PKG_SOURCE_DATE:=2025-04-23
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=04923c51039e8ca270c6f1dde3f04f3b36958089
PKG_MIRROR_HASH:=35d7987eed7f05e5f7d1d2e111a8c9f5d019ccf11eb839dfe0bd2e2c46b6199a

PKG_MAINTAINER:=Hans Dedecker <dedeckeh@gmail.com>
PKG_LICENSE:=GPL-2.0

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/nat46
  DEPENDS:=@IPV6 +kmod-nf-conntrack6
  TITLE:=Stateless NAT46 translation kernel module
  SECTION:=kernel
  SUBMENU:=Network Support
ifneq ($(CONFIG_PACKAGE_kmod-nat46),)
  FILES:=$(PKG_BUILD_DIR)/nat46/modules/nat46.ko
  AUTOLOAD:=$(call AutoLoad,33,nat46)
endif
endef

ifneq ($(CONFIG_PACKAGE_kmod-nat46),)

define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/nat46
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/nat46/modules/*.h  $(STAGING_DIR)/usr/include/nat46/
endef

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)/nat46/modules" \
		MODFLAGS="-DMODULE -mlong-calls" \
		EXTRA_CFLAGS="-DNAT46_VERSION=\\\"$(PKG_SOURCE_VERSION)\\\"" \
		modules
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/nat46/modules/Module.symvers $(PKG_BUILD_DIR)/Module.symvers
endef

else

define Build/Compile
	:
endef

define Build/Install
	:
endef

endif

$(eval $(call KernelPackage,nat46))
