include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=photonicat-pm
PKG_RELEASE:=1

PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define KernelPackage/photonicat-pm
  SUBMENU:=Other modules
  TITLE:=Photonicat Power Manager
  FILES:=$(PKG_BUILD_DIR)/photonicat-pm.ko
  AUTOLOAD:=$(call AutoLoad,60,photonicat-pm,1)
  DEPENDS:= @TARGET_rockchip_armv8 +kmod-hwmon-core +kmod-thermal 
  KCONFIG:= 
endef

define KernelPackage/photonicat-pm/description
  Power manager for photonicat board.
  Unless you have the platform, you will want to say 'N'.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,photonicat-pm))
