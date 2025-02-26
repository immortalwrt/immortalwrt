define Device/8devices_mango-dvk
	$(call Device/FitImageLzma)
	DEVICE_VENDOR := 8devices
	DEVICE_MODEL := Mango-DVK
	IMAGE_SIZE := 27776k
	BLOCKSIZE := 64k
	SOC := ipq6010
	SUPPORTED_DEVICES += 8devices,mango
	IMAGE/sysupgrade.bin := append-kernel | pad-to 64k | append-rootfs | pad-rootfs | check-size | append-metadata
	DEVICE_PACKAGES := ipq-wifi-8devices_mango
endef
TARGET_DEVICES += 8devices_mango-dvk

define Device/cambiumnetworks_xe3-4
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Cambium Networks
	DEVICE_MODEL := XE3-4
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@cp01-c3-xv3-4
	SOC := ipq6010
	DEVICE_PACKAGES := ipq-wifi-cambiumnetworks_xe34 ath11k-firmware-qcn9074 kmod-ath11k-pci
endef
TARGET_DEVICES += cambiumnetworks_xe3-4

define Device/linksys_mr7350
	$(call Device/FitImage)
	DEVICE_VENDOR := Linksys
	DEVICE_MODEL := MR7350
	SOC := ipq6000
	NAND_SIZE := 256m
	KERNEL_SIZE := 8192k
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	IMAGE_SIZE := 75776k
	IMAGE/factory.bin := append-kernel | pad-to $$$$(KERNEL_SIZE) | append-ubi | linksys-image type=MR7350
	DEVICE_PACKAGES := ipq-wifi-linksys_mr7350 kmod-leds-pca963x kmod-usb-ledtrig-usbport
endef
TARGET_DEVICES += linksys_mr7350

define Device/netgear_wax214
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Netgear
	DEVICE_MODEL := WAX214
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@cp03-c1
	SOC := ipq6010
	DEVICE_PACKAGES := ipq-wifi-netgear_wax214
endef
TARGET_DEVICES += netgear_wax214

define Device/qihoo_360v6
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Qihoo
	DEVICE_MODEL := 360V6
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-qihoo_360v6
endef
TARGET_DEVICES += qihoo_360v6

define Device/tplink_eap610-outdoor
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := TP-Link
	DEVICE_MODEL := EAP610-Outdoor
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_PACKAGES := ipq-wifi-tplink_eap610-outdoor
	IMAGES += web-ui-factory.bin
	IMAGE/web-ui-factory.bin := append-ubi | tplink-image-2022
	TPLINK_SUPPORT_STRING := SupportList: \
		EAP610-Outdoor(TP-Link|UN|AX1800-D):1.0 \
		EAP610-Outdoor(TP-Link|JP|AX1800-D):1.0 \
		EAP610-Outdoor(TP-Link|CA|AX1800-D):1.0
endef
TARGET_DEVICES += tplink_eap610-outdoor

define Device/yuncore_fap650
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Yuncore
	DEVICE_MODEL := FAP650
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	DEVICE_DTS_CONFIG := config@cp03-c1
	SOC := ipq6000
	DEVICE_PACKAGES := ipq-wifi-yuncore_fap650
endef
TARGET_DEVICES += yuncore_fap650

define Device/cmiot_ax18
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := CMIOT
	DEVICE_MODEL := AX18
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-cmiot_ax18
endef
TARGET_DEVICES += cmiot_ax18

define Device/glinet_gl-ax1800
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := GL-iNet
	DEVICE_MODEL := GL-AX1800
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-glinet_gl-ax1800
endef
TARGET_DEVICES += glinet_gl-ax1800

define Device/glinet_gl-axt1800
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := GL-iNet
	DEVICE_MODEL := GL-AXT1800
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-glinet_gl-axt1800
endef
TARGET_DEVICES += glinet_gl-axt1800

define Device/redmi_ax5
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Redmi
	DEVICE_MODEL := AX5
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-redmi_ax5
endef
TARGET_DEVICES += redmi_ax5

define Device/xiaomi_ax1800
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := Xiaomi
	DEVICE_MODEL := AX1800
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-xiaomi_ax1800
endef
TARGET_DEVICES += xiaomi_ax1800

define Device/zn_m2
	$(call Device/FitImage)
	$(call Device/UbiFit)
	DEVICE_VENDOR := ZN
	DEVICE_MODEL := M2
	BLOCKSIZE := 128k
	PAGESIZE := 2048
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-zn_m2
endef
TARGET_DEVICES += zn_m2

define Device/redmi_ax5-jdcloud
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := Redmi
	DEVICE_MODEL := AX5 JDCloud
	BLOCKSIZE := 128k
	KERNEL_SIZE := 6144k
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c1
	DEVICE_PACKAGES := ipq-wifi-redmi_ax5-jdcloud
	IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-rootfs | append-metadata
endef
TARGET_DEVICES += redmi_ax5-jdcloud

define Device/link_nn6000-v1
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := Link
	DEVICE_MODEL := NN6000 v1
	BLOCKSIZE := 128k
	KERNEL_SIZE := 6144k
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c2
	DEVICE_PACKAGES := ipq-wifi-link_nn6000
	IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-rootfs | append-metadata
endef
TARGET_DEVICES += link_nn6000-v1

define Device/link_nn6000-v2
	$(Device/link_nn6000-v1)
	DEVICE_MODEL := NN6000 v2
endef
TARGET_DEVICES += link_nn6000-v2

define Device/jdcloud_re-ss-01
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := RE-SS-01
	BLOCKSIZE := 128k
	KERNEL_SIZE := 6144k
	SOC := ipq6000
	DEVICE_DTS_CONFIG := config@cp03-c2
	DEVICE_PACKAGES := ipq-wifi-jdcloud_re-ss-01
	IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-rootfs | append-metadata
endef
TARGET_DEVICES += jdcloud_re-ss-01

define Device/jdcloud_re-cs-02
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := RE-CS-02
	BLOCKSIZE := 128k
	KERNEL_SIZE := 6144k
	SOC := ipq6010
	DEVICE_DTS_CONFIG := config@cp03-c3
	DEVICE_PACKAGES := ipq-wifi-jdcloud_re-cs-02 ath11k-firmware-qcn9074 kmod-ath11k-pci luci-app-athena-led luci-i18n-athena-led-zh-cn
	IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-rootfs | append-metadata
endef
TARGET_DEVICES += jdcloud_re-cs-02

define Device/jdcloud_re-cs-07
	$(call Device/FitImage)
	$(call Device/EmmcImage)
	DEVICE_VENDOR := JDCloud
	DEVICE_MODEL := RE-CS-07
	BLOCKSIZE := 128k
	KERNEL_SIZE := 6144k
	SOC := ipq6010
	DEVICE_DTS_CONFIG := config@cp03-c4
	DEVICE_PACKAGES := -ath11k-firmware-ipq6018 -ath11k-firmware-qcn9074 -kmod-ath11k -kmod-ath11k-ahb -kmod-ath11k-pci -hostapd-common -wpad-openssl
	IMAGE/factory.bin := append-kernel | pad-to $$(KERNEL_SIZE) | append-rootfs | append-metadata
endef
TARGET_DEVICES += jdcloud_re-cs-07
