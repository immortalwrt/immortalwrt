include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=hisilicon
BOARDNAME:=HiSilicon
FEATURES:=ext4 usb rtc neon vfpv4
CPU_TYPE:=cortex-a7
SUBTARGETS:=hi3798mv100

KERNEL_PATCHVER:=6.12

define Target/Description
	Build firmware images for HiSilicon Hi3798MV100 (EC6108V9C) TV Box.
endef

include $(INCLUDE_DIR)/target.mk

KERNELNAME:=zImage dtbs


DEFAULT_PACKAGES += \
	fdt-utils uboot-envtools \
	kmod-gpio-button-hotplug \
	kmod-usb-storage kmod-fs-ext4 \
	kmod-usb2 kmod-usb3 \
	kmod-hisi-femac \
	kmod-nft-core kmod-nft-ipv4 kmod-nft-nat nftables \
	kmod-fs-f2fs f2fs-tools

$(eval $(call BuildTarget))