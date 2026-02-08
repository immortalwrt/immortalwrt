define Device/generic
  DEVICE_VENDOR := Generic
  DEVICE_MODEL := x86/64
  DEVICE_PACKAGES += \
	kmod-amazon-ena kmod-amd-xgbe kmod-bnx2 kmod-e1000 \
	kmod-dwmac-intel kmod-forcedeth kmod-fs-vfat kmod-tg3 \
	kmod-drm-i915
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
