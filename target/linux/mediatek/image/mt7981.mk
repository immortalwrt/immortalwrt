KERNEL_LOADADDR := 0x48080000

MT7981_USB_PKGS := automount blkid blockdev fdisk \
    kmod-nls-cp437 kmod-nls-iso8859-1 kmod-usb2 kmod-usb3 \
    luci-app-usb-printer luci-i18n-usb-printer-zh-cn \
    kmod-usb-net-rndis usbutils luci-app-usbmodem

define Device/mt7981-spim-nor-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nor-rfb
  DEVICE_DTS := mt7981-spim-nor-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-nor-rfb
endef
TARGET_DEVICES += mt7981-spim-nor-rfb

define Device/mt7981-spim-nand-2500wan-gmac2
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nand-2500wan-gmac2
  DEVICE_DTS := mt7981-spim-nand-2500wan-gmac2
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-2500wan-gmac2-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-2500wan-gmac2

define Device/mt7981-spim-nand-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nand-rfb
  DEVICE_DTS := mt7981-spim-nand-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-spim-snand-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-rfb

define Device/mt7981-spim-nand-gsw
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-spim-nand-gsw
  DEVICE_DTS := mt7981-spim-nand-gsw
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-rfb,ubi
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-spim-nand-gsw

define Device/mt7981-emmc-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-emmc-rfb
  DEVICE_DTS := mt7981-emmc-rfb
  SUPPORTED_DEVICES := mediatek,mt7981-emmc-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-emmc-rfb

define Device/mt7981-sd-rfb
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-sd-rfb
  DEVICE_DTS := mt7981-sd-rfb
  SUPPORTED_DEVICES := mediatek,mt7981-sd-rfb
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-sd-rfb

define Device/mt7981-snfi-nand-2500wan-p5
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-snfi-nand-2500wan-p5
  DEVICE_DTS := mt7981-snfi-nand-2500wan-p5
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-snfi-snand-pcie-2500wan-p5-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-snfi-nand-2500wan-p5

define Device/mt7981-fpga-spim-nor
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-spim-nor
  DEVICE_DTS := mt7981-fpga-spim-nor
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-nor
endef
TARGET_DEVICES += mt7981-fpga-spim-nor

define Device/mt7981-fpga-snfi-nand
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-snfi-nand
  DEVICE_DTS := mt7981-fpga-snfi-nand
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-snfi-snand
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-snfi-nand

define Device/mt7981-fpga-spim-nand
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-spim-nand
  DEVICE_DTS := mt7981-fpga-spim-nand
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-spim-snand
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-spim-nand

define Device/mt7981-fpga-emmc
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-emmc
  DEVICE_DTS := mt7981-fpga-emmc
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-emmc
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-emmc

define Device/mt7981-fpga-sd
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := mt7981-fpga-sd
  DEVICE_DTS := mt7981-fpga-sd
  SUPPORTED_DEVICES := mediatek,mt7981-fpga-sd
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs e2fsprogs blkid blockdev losetup kmod-fs-ext4 \
		     kmod-mmc kmod-fs-f2fs kmod-fs-vfat kmod-nls-cp437 \
		     kmod-nls-iso8859-1
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-fpga-sd

define Device/mt7981-360-t7
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := 360 T7
  DEVICE_DTS := mt7981-360-t7
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := 360,t7
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 36864k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-360-t7

define Device/mt7981-360-t7-108M
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := 360 T7 (with 108M ubi)
  DEVICE_DTS := mt7981-360-t7-108M
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := 360,t7
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 110592k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-360-t7-108M

define Device/abt_asr3000
  DEVICE_VENDOR := ABT
  DEVICE_MODEL := ASR3000
  DEVICE_DTS := mt7981-abt-asr3000
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := abt,asr3000
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114816k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += abt_asr3000

define Device/cetron_ct3003
  DEVICE_VENDOR := CETRON
  DEVICE_MODEL := CT3003
  DEVICE_DTS := mt7981-cetron-ct3003
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := cetron,ct3003
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114816k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += cetron_ct3003

define Device/mt7981-clt-r30b1
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := CLT R30B1
  DEVICE_DTS := mt7981-clt-r30b1
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := clt,30rb1
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-clt-r30b1

define Device/mt7981-clt-r30b1-112M
  DEVICE_VENDOR := MediaTek
  DEVICE_MODEL := CLT R30B1 (with 112M ubi)
  DEVICE_DTS := mt7981-clt-r30b1-112M
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := clt,r30b1
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += mt7981-clt-r30b1-112M

define Device/xiaomi_mi-router-wr30u-stock
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router WR30U (stock layout)
  DEVICE_DTS := mt7981-xiaomi-mi-router-wr30u-stock
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 34816k
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-wr30u-stock

define Device/xiaomi_mi-router-wr30u-112m
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router WR30U (112M UBI layout)
  DEVICE_DTS := mt7981-xiaomi-mi-router-wr30u-112m
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.ubi
  IMAGE/factory.ubi := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-wr30u-112m

define Device/xiaomi_mi-router-ax3000t-stock
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router AX3000T (stock layout)
  DEVICE_DTS := mt7981-xiaomi-mi-router-ax3000t-stock
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 34816k
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-ax3000t-stock

define Device/xiaomi_mi-router-ax3000t
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi Router AX3000T
  DEVICE_DTS := mt7981-xiaomi-mi-router-ax3000t
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.ubi := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += xiaomi_mi-router-ax3000t

define Device/glinet_gl-mt3000
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT3000
  DEVICE_DTS := mt7981-gl-mt3000
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := glinet,mt3000-snand
  DEVICE_PACKAGES := kmod-hwmon-pwmfan
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES := sysupgrade.tar
  IMAGE/sysupgrade.tar := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-mt3000

define Device/glinet_gl-x3000
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-X3000
  DEVICE_DTS := mt7981-gl-x3000
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := glinet,x3000-emmc
  DEVICE_PACKAGES := kmod-hwmon-pwmfan mkf2fs kmod-mmc kmod-fs-f2fs gdisk
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-x3000

define Device/glinet_gl-xe3000
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-XE3000
  DEVICE_DTS := mt7981-gl-xe3000
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := glinet,xe3000-emmc
  DEVICE_PACKAGES := kmod-hwmon-pwmfan mkf2fs kmod-mmc kmod-fs-f2fs gdisk
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-xe3000

define Device/glinet_gl-mt2500
  DEVICE_VENDOR := GL.iNet
  DEVICE_MODEL := GL-MT2500
  DEVICE_DTS := mt7981-gl-mt2500
  SUPPORTED_DEVICES := glinet,mt2500-emmc
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := mkf2fs kmod-mmc kmod-fs-f2fs gdisk
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += glinet_gl-mt2500

define Device/jcg_q30
  DEVICE_VENDOR := JCG
  DEVICE_MODEL := Q30
  DEVICE_DTS := mt7981-jcg-q30
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := jcg,q30
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114816k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += jcg_q30

define Device/livinet_zr-3020
  DEVICE_VENDOR := Livinet
  DEVICE_MODEL := ZR-3020
  DEVICE_DTS := mt7981-zr-3020
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := livinet,zr-3020 mediatek,mt7981-spim-snand-gsw-rfb
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += livinet_zr-3020

define Device/cmcc_a10
  DEVICE_VENDOR := CMCC
  DEVICE_MODEL := A10
  DEVICE_DTS := mt7981-cmcc-a10
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := cmcc,a10
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += cmcc_a10

define Device/cmcc_rax3000m
  DEVICE_VENDOR := CMCC
  DEVICE_MODEL := RAX3000M NAND
  DEVICE_DTS := mt7981-cmcc-rax3000m
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  DEVICE_PACKAGES := $(MT7981_USB_PKGS) luci-app-ksmbd luci-i18n-ksmbd-zh-cn ksmbd-utils
  SUPPORTED_DEVICES := cmcc,rax3000m
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 116736k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += cmcc_rax3000m

define Device/cmcc_rax3000m-emmc
  DEVICE_VENDOR := CMCC
  DEVICE_MODEL := RAX3000M eMMC
  DEVICE_DTS := mt7981-cmcc-rax3000m-emmc
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := cmcc,rax3000m-emmc
  DEVICE_PACKAGES := $(MT7981_USB_PKGS) f2fsck losetup mkf2fs kmod-fs-f2fs kmod-mmc \
	luci-app-ksmbd luci-i18n-ksmbd-zh-cn ksmbd-utils 
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += cmcc_rax3000m-emmc

define Device/h3c_nx30pro
  DEVICE_VENDOR := H3C
  DEVICE_MODEL := NX30PRO
  DEVICE_DTS := mt7981-h3c-nx30pro
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := h3c,nx30pro
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 65536k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += h3c_nx30pro

define Device/konka_komi-a31
  DEVICE_VENDOR := KONKA
  DEVICE_MODEL := KOMI A31
  DEVICE_DTS := mt7981-konka-komi-a31
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := konka,komi-a31
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114688k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += konka_komi-a31


define Device/nokia_ea0326gmp
  DEVICE_VENDOR := Nokia
  DEVICE_MODEL := EA0326GMP
  DEVICE_DTS := mt7981-nokia-ea0326gmp
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := nokia,ea0326gmp
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 112640k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += nokia_ea0326gmp


define Device/imou_lc-hx3001
  DEVICE_VENDOR := Imou
  DEVICE_MODEL := LC-HX3001
  DEVICE_DTS := mt7981-imou_lc-hx3001
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := imou,lc-hx3001
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 114816k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += imou_lc-hx3001

define Device/nradio_wt9103
  DEVICE_VENDOR := NRADIO
  DEVICE_MODEL := WT9103
  DEVICE_DTS := mt7981-nradio-wt9103
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := nradio,wt9103
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 131072k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += nradio_wt9103

define Device/nradio_wt9103_512m
  DEVICE_VENDOR := NRADIO
  DEVICE_MODEL := WT9103 (with 512M Flash)
  DEVICE_DTS := mt7981-nradio-wt9103-512m
  DEVICE_DTS_DIR := $(DTS_DIR)/mediatek
  SUPPORTED_DEVICES := nradio,wt9103
  UBINIZE_OPTS := -E 5
  BLOCKSIZE := 128k
  PAGESIZE := 2048
  IMAGE_SIZE := 131072k
  KERNEL_IN_UBI := 1
  IMAGES += factory.bin
  IMAGE/factory.bin := append-ubi | check-size $$$$(IMAGE_SIZE)
  IMAGE/sysupgrade.bin := sysupgrade-tar | append-metadata
endef
TARGET_DEVICES += nradio_wt9103_512m
