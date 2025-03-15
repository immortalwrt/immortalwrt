KERNEL_LOADADDR := 0x48080000

MT7986_USB_PKGS := automount blkid blockdev fdisk \
    kmod-nls-cp437 kmod-nls-iso8859-1 kmod-usb2 kmod-usb3 \
    luci-app-usb-printer luci-i18n-usb-printer-zh-cn \
    kmod-usb-net-rndis usbutils

MT7986_WWAN_PKGS := wwan uqmi modemmanager \
    luci-proto-modemmanager luci-proto-qmi \
    kmod-usb-net-cdc-ether kmod-usb-net-cdc-mbim kmod-usb-net-cdc-ncm \
    kmod-usb-serial-option kmod-usb-serial-qualcomm

define Device/mt7986a-ax6000-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-spim-nor-rfb
  DEVICE_DTS := mt7986a-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-nor-rfb
endef
TARGET_DEVICES += mt7986a-ax6000-spim-nor-rfb

define Device/mt7986a-ax6000-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax6000-spim-nand-rfb

define Device/mt7986a-ax8400-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax8400-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax8400-spim-nand-rfb

define Device/mt7986a-ax6000-snfi-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-snfi-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-snfi-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-snfi-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax6000-snfi-nand-rfb

define Device/mt7986a-ax6000-emmc-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-emmc-rfb
  DEVICE_DTS := mt7986a-emmc-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-emmc-rfb
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax6000-emmc-rfb

define Device/mt7986a-ax6000-2500wan-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-2500wan-spim-nor-rfb
  DEVICE_DTS := mt7986a-2500wan-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-2500wan-nor-rfb
endef
TARGET_DEVICES += mt7986a-ax6000-2500wan-spim-nor-rfb

define Device/mt7986a-ax6000-2500wan-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-2500wan-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-2500wan-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-2500wan-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax6000-2500wan-spim-nand-rfb

define Device/mt7986a-ax6000-2500wan-gsw-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-2500wan-gsw-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-2500wan-gsw-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-2500wan-gsw-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax6000-2500wan-gsw-spim-nand-rfb

define Device/mt7986a-ax6000-2500wan-sd-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax6000-2500wan-sd-rfb
  DEVICE_DTS := mt7986a-2500wan-sd-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-2500wan-sd-rfb
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax6000-2500wan-sd-rfb

define Device/mt7986a-ax8400-2500wan-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax8400-2500wan-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-2500wan-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-2500wan-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax8400-2500wan-spim-nand-rfb

define Device/mt7986a-ax7800-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax7800-spim-nor-rfb
  DEVICE_DTS := mt7986a-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-nor-rfb
endef
TARGET_DEVICES += mt7986a-ax7800-spim-nor-rfb

define Device/mt7986a-ax7800-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax7800-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax7800-spim-nand-rfb

define Device/mt7986a-ax7800-2500wan-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax7800-2500wan-spim-nor-rfb
  DEVICE_DTS := mt7986a-2500wan-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-2500wan-nor-rfb
endef
TARGET_DEVICES += mt7986a-ax7800-2500wan-spim-nor-rfb

define Device/mt7986a-ax7800-2500wan-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986a-ax7800-2500wan-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986a-2500wan-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986a-2500wan-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986a-ax7800-2500wan-spim-nand-rfb

define Device/mt7986b-ax6000-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-spim-nor-rfb
  DEVICE_DTS := mt7986b-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-nor-rfb
endef
TARGET_DEVICES += mt7986b-ax6000-spim-nor-rfb

define Device/mt7986b-ax6000-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986b-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-spim-nand-rfb

define Device/mt7986b-ax6000-snfi-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-snfi-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986b-snfi-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-snfi-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-snfi-nand-rfb

define Device/mt7986b-ax6000-emmc-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-emmc-rfb
  DEVICE_DTS := mt7986b-emmc-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-emmc-rfb
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-emmc-rfb

define Device/mt7986b-ax6000-2500wan-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-2500wan-spim-nor-rfb
  DEVICE_DTS := mt7986b-2500wan-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-nor-rfb
endef
TARGET_DEVICES += mt7986b-ax6000-2500wan-spim-nor-rfb

define Device/mt7986b-ax6000-2500wan-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-2500wan-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986b-2500wan-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-2500wan-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-2500wan-spim-nand-rfb

define Device/mt7986b-ax6000-2500wan-gsw-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-2500wan-gsw-spim-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986b-2500wan-gsw-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-2500wan-gsw-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-2500wan-gsw-spim-nand-rfb

define Device/mt7986b-ax6000-2500wan-snfi-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-2500wan-snfi-nand-rfb (SPI-NAND,UBI)
  DEVICE_DTS := mt7986b-2500wan-snfi-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-2500wan-snfi-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-2500wan-snfi-nand-rfb

define Device/mt7986b-ax6000-2500wan-sd-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7986b-ax6000-2500wan-sd-rfb
  DEVICE_DTS := mt7986b-2500wan-sd-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986b-2500wan-sd-rfb
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7986b-ax6000-2500wan-sd-rfb

define Device/mediatek_mt7986-fpga
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7986 FPGA
  DEVICE_DTS := mt7986-fpga
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  IMAGE/sysupgrade.bin := append-kernel | pad-to 256k | \
       append-rootfs | pad-rootfs | append-metadata
endef
TARGET_DEVICES += mediatek_mt7986-fpga

define Device/mediatek_mt7986-fpga-ubi
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := MTK7986 FPGA (UBI)
  DEVICE_DTS := mt7986-fpga-ubi
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7986-fpga,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mediatek_mt7986-fpga-ubi

define Device/xiaomi_redmi-router-ax6000
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Redmi Router AX6000
  DEVICE_DTS := mt7986a-xiaomi-redmi-router-ax6000
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-leds-ws2812b
  SUPPORTED_DEVICES := xiaomi,redmi-router-ax6000
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 112640k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_redmi-router-ax6000

define Device/xiaomi_redmi-router-ax6000-stock
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Redmi Router AX6000 (stock layout)
  DEVICE_DTS := mt7986a-xiaomi-redmi-router-ax6000-stock
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := kmod-leds-ws2812b
  SUPPORTED_DEVICES := xiaomi,redmi-router-ax6000-stock
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_redmi-router-ax6000-stock

define Device/ruijie-rg-x60-pro-uboot
  DEVICE_VENDOR := Ruijie
  DEVICE_MODEL := Ruijie RG-X60 Pro(uboot layout)
  DEVICE_DTS := mt7986a-ruijie-rg-x60-pro-uboot
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := ruijie,rg-x60-pro-uboot
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += ruijie-rg-x60-pro-uboot

define Device/ruijie-rg-x60-pro
  DEVICE_VENDOR := Ruijie
  DEVICE_MODEL := Ruijie RG-X60 Pro
  DEVICE_DTS := mt7986a-ruijie-rg-x60-pro
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := ruijie,rg-x60-pro
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += ruijie-rg-x60-pro

define Device/BPI-R3MINI-NAND
  DEVICE_VENDOR := Banana Pi
  DEVICE_MODEL := Banana Pi R3MINI
  DEVICE_TITLE := MTK7986a BPI R3MINI NAND
  DEVICE_DTS := mt7986a-bananapi-bpi-r3mini-nand
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := $(MT7986_USB_PKGS) $(MT7986_WWAN_PKGS) \
        kmod-phy-air-en8811h bpir3_mini-properties pciutils
  SUPPORTED_DEVICES := bananapi,bpi-r3mini
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += BPI-R3MINI-NAND

define Device/BPI-R3MINI-NAND-110M
  DEVICE_VENDOR := Banana Pi
  DEVICE_MODEL := Banana Pi R3MINI
  DEVICE_TITLE := MTK7986a BPI R3MINI NAND 110M Layout
  DEVICE_DTS := mt7986a-bananapi-bpi-r3mini-nand-110m
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := $(MT7986_USB_PKGS) $(MT7986_WWAN_PKGS) \
        kmod-phy-air-en8811h bpir3_mini-properties pciutils
  SUPPORTED_DEVICES := bananapi,bpi-r3mini
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 110592k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += BPI-R3MINI-NAND-110M

define Device/BPI-R3MINI-EMMC
  DEVICE_VENDOR := Banana Pi
  DEVICE_MODEL := Banana Pi R3MINI
  DEVICE_TITLE := MTK7986a BPI R3MINI EMMC
  DEVICE_DTS := mt7986a-bananapi-bpi-r3mini-emmc
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := $(MT7986_USB_PKGS) $(MT7986_WWAN_PKGS) \
        kmod-phy-air-en8811h bpir3_mini-properties pciutils \
        f2fsck losetup mkf2fs f2fs-tools kmod-fs-f2fs kmod-mmc \
	luci-app-ksmbd luci-i18n-ksmbd-zh-cn ksmbd-utils 
  SUPPORTED_DEVICES := bananapi,bpi-r3mini-emmc
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += BPI-R3MINI-EMMC

define Device/jdcloud_re-cp-03
  DEVICE_VENDOR := JDCloud
  DEVICE_MODEL := RE-CP-03
  DEVICE_DTS := mt7986a-jdcloud-re-cp-03
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := jdcloud,re-cp-03
  DEVICE_PACKAGES := blkid blockdev fdisk f2fsck mkf2fs losetup \
	kmod-mmc kmod-nls-cp437 kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += jdcloud_re-cp-03

define Device/netcore_n60
  DEVICE_VENDOR := Netcore
  DEVICE_MODEL := N60
  DEVICE_DTS := mt7986a-netcore-n60
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := netcore,n60
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += netcore_n60

define Device/glinet_gl-mt6000
   DEVICE_VENDOR := GL.iNet
   DEVICE_MODEL := GL-MT6000
   DEVICE_DTS := mt7986a-glinet-gl-mt6000
   DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
   DEVICE_PACKAGES := $(MT7986_USB_PKGS) f2fsck losetup mkf2fs kmod-fs-f2fs kmod-mmc
   IMAGES += factory.bin
   IMAGE/factory.bin := append-kernel | pad-to 32M | append-rootfs
   IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-mt6000

define Device/tplink_tl-common
    DEVICE_VENDOR := TP-Link
    DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
    DEVICE_PACKAGES := $(MT7986_USB_PKGS)
    UBINIZE_OPTS := -E 5
    BLOCKSIZE := 128k
    PAGESIZE := 2048
    KERNEL_IN_UBI := 1
    IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef

define Device/tplink_tl-xdr6086
    DEVICE_MODEL := TL-XDR6086
    DEVICE_DTS := mt7986a-tl-xdr6086
    $(call Device/tplink_tl-common)
endef
TARGET_DEVICES += tplink_tl-xdr6086

define Device/tplink_tl-xdr6088
    DEVICE_MODEL := TL-XDR6088
    DEVICE_DTS := mt7986a-tl-xdr6088
    $(call Device/tplink_tl-common)
endef
TARGET_DEVICES += tplink_tl-xdr6088
