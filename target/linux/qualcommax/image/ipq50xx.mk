DEVICE_VARS += BOOT_SCRIPT

define Build/mstc-header
	$(eval version=$(word 1,$(1)))
	$(eval hdrlen=$(if $(word 2,$(1)),$(word 2,$(1)),0x400))
	gzip -c $@ | tail -c8 > $@.crclen
	( \
		printf "CMOC"; \
		tail -c+5 $@.crclen; head -c4 $@.crclen; \
		printf '$(call toupper,$(LINUX_KARCH)) $(VERSION_DIST) Linux-$(LINUX_VERSION)' | \
			dd bs=64 count=1 conv=sync 2>/dev/null; \
		printf "$(version)" | \
			dd bs=64 count=1 conv=sync 2>/dev/null; \
		dd if=/dev/zero bs=$$(($(hdrlen) - 0x8c)) count=1 2>/dev/null; \
		cat $@; \
	) > $@.new
	mv $@.new $@
	rm -f $@.crclen
endef

define Device/cmcc_pz-l8
	$(call Device/FitImageLzma)
	$(call Device/UbiFit)
	DEVICE_VENDOR := CMCC
	DEVICE_MODEL := PZ-L8
	DEVICE_DTS_CONFIG := config@mp02.1
	SOC := ipq5018
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 59392k
	NAND_SIZE := 128m
endef
TARGET_DEVICES += cmcc_pz-l8

define Device/elecom_wrc-x3000gs2
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := ELECOM
	DEVICE_MODEL := WRC-X3000GS2
	KERNEL_IN_UBI := 1
	IMAGE_SIZE := 52480k
	NAND_SIZE := 128m
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.3
	DEVICE_PACKAGES := ipq-wifi-elecom_wrc-x3000gs2 ath11k-firmware-qcn6122
	IMAGES += factory.bin
	IMAGE/factory.bin := append-ubi | qsdk-ipq-factory-nand | mstc-header 4.04(XZF.0)b90 | elecom-product-header WRC-X3000GS2
endef
TARGET_DEVICES += elecom_wrc-x3000gs2

define Device/glinet_gl-b3000
	$(call Device/FitImage)
	DEVICE_VENDOR := GL.iNet
	DEVICE_MODEL := GL-B3000
	SUPPORTED_DEVICES += b3000
	BOOT_SCRIPT:= glinet_gl-b3000.bootscript
	KERNEL_IN_UBI := 1
	NAND_SIZE := 128m
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.5-c1
	DEVICE_PACKAGES := ipq-wifi-glinet_gl-b3000 ath11k-firmware-qcn6122 dumpimage
	IMAGES := factory.img sysupgrade.bin
	IMAGE/factory.img := append-ubi | gl-qsdk-factory | append-metadata
endef
TARGET_DEVICES += glinet_gl-b3000

define Device/iodata_wn-dax3000gr
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := I-O DATA
	DEVICE_MODEL := WN-DAX3000GR
	KERNEL_IN_UBI := 1
	IMAGE_SIZE := 52480k
	NAND_SIZE := 128m
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.3
	DEVICE_PACKAGES := ipq-wifi-iodata_wn-dax3000gr ath11k-firmware-qcn6122
	IMAGES += factory.bin
	IMAGE/factory.bin := append-ubi | qsdk-ipq-factory-nand | mstc-header 4.04(XZH.1)b90 0x480
endef
TARGET_DEVICES += iodata_wn-dax3000gr

define Device/linksys_ipq50xx_mx_base
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := Linksys
	KERNEL_SIZE := 8192k
	IMAGE_SIZE := 83968k
	NAND_SIZE := 256m
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	IMAGES += factory.bin
	IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | linksys-image type=$$$$(DEVICE_MODEL)
endef

define Device/linksys_mr5500
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := MR5500
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := ipq-wifi-linksys_mr5500 ath11k-firmware-qcn9074 kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += linksys_mr5500

define Device/linksys_mx2000
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := MX2000
	DEVICE_DTS_CONFIG := config@mp03.5-c1
	DEVICE_PACKAGES := ipq-wifi-linksys_mx2000 ath11k-firmware-qcn6122
endef
TARGET_DEVICES += linksys_mx2000

define Device/linksys_mx5500
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := MX5500
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := ipq-wifi-linksys_mx5500 ath11k-firmware-qcn9074
endef
TARGET_DEVICES += linksys_mx5500

define Device/linksys_spnmx56
	$(call Device/linksys_ipq50xx_mx_base)
	DEVICE_MODEL := SPNMX56
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := ipq-wifi-linksys_spnmx56 ath11k-firmware-qcn9074
endef
TARGET_DEVICES += linksys_spnmx56

define Device/xiaomi_ax6000
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := AX6000
	KERNEL_SIZE := 36864k
	NAND_SIZE := 128m
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax6000 ath11k-firmware-qcn9074 ath10k-firmware-qca9887 kmod-ath10k-smallbuffers
ifeq ($(IB),)
ifneq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),)
	ARTIFACTS := initramfs-factory.ubi
	ARTIFACT/initramfs-factory.ubi := append-image-stage initramfs-uImage.itb | ubinize-kernel
endif
endif
endef
TARGET_DEVICES += xiaomi_ax6000

define Device/yuncore_ax830
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Yuncore
	DEVICE_MODEL := AX830
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.5-c1
	DEVICE_PACKAGES := ipq-wifi-yuncore_ax830 ath11k-firmware-qcn6122
endef
TARGET_DEVICES += yuncore_ax830

define Device/yuncore_ax850
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Yuncore
	DEVICE_MODEL := AX850
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.1
	DEVICE_PACKAGES := ipq-wifi-yuncore_ax850 ath11k-firmware-qcn9074
endef
TARGET_DEVICES += yuncore_ax850

define Device/jdcloud_re-cs-03
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := RE-CS-03
	KERNEL_SIZE := 6144k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq5018
	DEVICE_DTS_CONFIG := config@mp03.5-c2
	DEVICE_PACKAGES := ipq-wifi-jdcloud_re-cs-03 ath11k-firmware-qcn6122
	IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-rootfs | append-metadata
endef
TARGET_DEVICES += jdcloud_re-cs-03
