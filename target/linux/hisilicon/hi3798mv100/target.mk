BOARDNAME:=HiSilicon Hi3798MV100
# Hi3798MV100  Cortex-A7
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4

FEATURES:=ext4 squashfs usb fpu neon rtc
DEFAULT_PACKAGES += kmod-usb-storage kmod-fs-ext4 kmod-fs-f2fs f2fs-tools