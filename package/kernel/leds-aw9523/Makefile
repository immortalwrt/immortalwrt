#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=leds-aw9523
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-only

include $(INCLUDE_DIR)/package.mk

define KernelPackage/leds-aw9523
  SUBMENU:=LED modules
  TITLE:=Awinic AW9523B constant-current LED support
  DEPENDS:=+kmod-i2c-core +kmod-regmap-i2c
  FILES:=$(PKG_BUILD_DIR)/leds-aw9523.ko
  AUTOLOAD:=$(call AutoProbe,leds-aw9523,1)
endef

define KernelPackage/leds-aw9523/description
  Driver for the Awinic AW9523/AW9523B 16-channel I2C GPIO/LED expander
  operating its outputs in constant-current (LED) mode. Each output is exposed
  as an LED class device: describe them as child nodes of the aw9523 i2c node
  (compatible "awinic,aw9523-led") with reg = DIM channel (0..15) plus the usual
  color/function/linux,default-trigger properties.
endef

define Build/Compile
	$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" modules
endef

$(eval $(call KernelPackage,leds-aw9523))
