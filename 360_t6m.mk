define Device/360_t6m
  $(Device/nand)
  DEVICE_VENDOR := 360
  DEVICE_MODEL := T6M
  DEVICE_DTS := mt7621_qihoo_360t6m
  SOC := mt7621
  KERNEL_SIZE := 4096k
  IMAGE_SIZE := 124416k
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_LOADADDR := 0x80001000
  KERNEL := kernel-bin | append-dtb | lzma | uImage lzma
  DEVICE_PACKAGES := kmod-mt76-connac kmod-mt76-core kmod-mt7915-firmware kmod-mt7915e kmod-tun kmod-nft-tproxy wpad-openssl luci luci-ssl dropbear
  IMAGES += factory.bin
  IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-ubi | check-size
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += 360_t6m
