include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk


KERNEL_LOADADDR := 0x02000000
KERNEL_ENTRYADDR := 0x02000000

define Device/Default
  PROFILES := Default
  KERNEL := kernel-bin | append-dtb | uImage none
  IMAGES := sysupgrade.bin
  DEVICE_DTS_DIR = $$(DTS_DIR)/hisilicon
  IMAGE/sysupgrade.bin := append-kernel | pad-to 16M | append-rootfs | append-metadata
endef

define Device/huawei_ec6108v9c
  DEVICE_VENDOR := Huawei
  DEVICE_MODEL := EC6108V9C
  DEVICE_DTS := hi3798mv100-ec6108v9c
  KERNEL_NAME := zImage
  
  IMAGES := sysupgrade.bin kernel.bin rootfs.squashfs
  ARTIFACTS := partition.xml bootargs.bin

  IMAGE/kernel.bin := append-kernel
  IMAGE/rootfs.squashfs := append-rootfs
  # sysupgrade.bin                        (from Default)

  ARTIFACT/bootargs.bin := mkbootargs-bin
  ARTIFACT/partition.xml := gen-partition-xml-from-template
endef
TARGET_DEVICES += huawei_ec6108v9c

define Build/gen-partition-xml-from-template
	sed -e 's|@@BOOTARGS_FILE@@|$(DEVICE_IMG_PREFIX)-bootargs.bin|g' \
	    -e 's|@@KERNEL_FILE@@|$(DEVICE_IMG_PREFIX)-squashfs-kernel.bin|g' \
	    -e 's|@@ROOTFS_FILE@@|$(DEVICE_IMG_PREFIX)-squashfs-rootfs.squashfs|g' \
	    $(CURDIR)/partition.xml.template > $@
endef
define Build/mkbootargs-bin
	mkbootargs \
		-s 64 \
		-r $(CURDIR)/bootargs.txt \
		-o $@
endef
$(eval $(call BuildImage))