define KernelPackage/phy-qcom-ipq806x-usb
  TITLE:=Qualcomm IPQ806x DWC3 USB PHY driver
  DEPENDS:=@TARGET_ipq806x
  KCONFIG:= CONFIG_PHY_QCOM_IPQ806X_USB
  FILES:= \
    $(LINUX_DIR)/drivers/phy/qualcomm/phy-qcom-ipq806x-usb.ko
  AUTOLOAD:=$(call AutoLoad,45,phy-qcom-ipq806x-usb,1)
  $(call AddDepends/usb)
endef

define KernelPackage/phy-qcom-ipq806x-usb/description
 This driver provides support for the integrated DesignWare
 USB3 IP Core within the QCOM SoCs.
endef

$(eval $(call KernelPackage,phy-qcom-ipq806x-usb))


define KernelPackage/hwmon-emc2305
  TITLE:=SMSC EMC2301/2/3/5 fan controller
  KCONFIG:=CONFIG_SENSORS_EMC2305
  FILES:=$(LINUX_DIR)/drivers/hwmon/emc2305.ko
  AUTOLOAD:=$(call AutoProbe,emc2305)
  $(call AddDepends/hwmon,+kmod-i2c-core +kmod-thermal +kmod-regmap-i2c)
endef

define KernelPackage/hwmon-emc2305/description
 Kernel module for SMSC EMC2301/EMC2302/EMC2303/EMC2305 fan controllers
endef

$(eval $(call KernelPackage,hwmon-emc2305))

