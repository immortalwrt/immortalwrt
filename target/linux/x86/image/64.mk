define Device/generic
  DEVICE_TITLE := Generic x86/64
  DEVICE_PACKAGES += kmod-amazon-ena kmod-bnx2 kmod-e1000 \
	kmod-forcedeth kmod-amd-xgbe
  GRUB2_VARIANT := generic
endef
TARGET_DEVICES += generic
