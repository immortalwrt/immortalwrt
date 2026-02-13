# SPDX-License-Identifier: GPL-2.0-only

ifeq ($(SUBTARGET),msm8916)

define Device/msm8916
	SOC := msm8916
	CMDLINE := "earlycon console=tty0 console=ttyMSM0,115200 root=/dev/mmcblk0p14 rw rootwait"
endef

define Device/openstick-ufi001c
  $(Device/msm8916)
  DEVICE_VENDOR := Zhihe
  DEVICE_MODEL := OpenStick UFI001C
  DEVICE_DTS := msm8916-thwc-ufi001c
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick_tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-ufi001c-firmware qcom-msm8916-openstick-ufi001c-wcnss-firmware qcom-msm8916-wcnss-openstick-ufi001c-nv
endef
TARGET_DEVICES += openstick-ufi001c

define Device/openstick-ufi001b
  $(Device/msm8916)
  DEVICE_VENDOR := Zhihe
  DEVICE_MODEL := OpenStick UFI001B
  DEVICE_DTS := msm8916-thwc-ufi001b
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick_tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-ufi001b-firmware qcom-msm8916-openstick-ufi001b-wcnss-firmware qcom-msm8916-wcnss-openstick-ufi001b-nv
endef
TARGET_DEVICES += openstick-ufi001b

define Device/openstick-ufi103s
  $(Device/openstick-ufi001c)
  DEVICE_MODEL := OpenStick UFI103S
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-ufi103s-firmware qcom-msm8916-openstick-ufi103s-wcnss-firmware qcom-msm8916-wcnss-openstick-ufi103s-nv
endef
TARGET_DEVICES += openstick-ufi103s

define Device/openstick-qrzl903
  $(Device/openstick-ufi001c)
  DEVICE_MODEL := OpenStick QRZL903
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-qrzl903-firmware qcom-msm8916-openstick-qrzl903-wcnss-firmware qcom-msm8916-wcnss-openstick-qrzl903-nv
endef
TARGET_DEVICES += openstick-qrzl903

define Device/openstick-w001
  $(Device/openstick-ufi001c)
  DEVICE_MODEL := OpenStick W001
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-w001-firmware qcom-msm8916-openstick-w001-wcnss-firmware qcom-msm8916-wcnss-openstick-w001-nv
endef
TARGET_DEVICES += openstick-w001

define Device/openstick-ufi003
  $(Device/openstick-ufi001c)
  DEVICE_MODEL := OpenStick UFI003
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-ufi003-firmware qcom-msm8916-openstick-ufi003-wcnss-firmware qcom-msm8916-wcnss-openstick-ufi003-nv
endef
TARGET_DEVICES += openstick-ufi003

define Device/openstick-uz801
  $(Device/msm8916)
  DEVICE_VENDOR := YiMing
  DEVICE_MODEL := OpenStick UZ801
  DEVICE_DTS := msm8916-yiming-uz801v3
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-uz801-firmware qcom-msm8916-openstick-uz801-wcnss-firmware qcom-msm8916-wcnss-openstick-uz801-nv
endef
TARGET_DEVICES += openstick-uz801

define Device/openstick-mf32
  $(Device/msm8916)
  DEVICE_VENDOR := XinXun
  DEVICE_MODEL := OpenStick MF32
  DEVICE_DTS := msm8916-ufi-mf32
    DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-mf32-firmware qcom-msm8916-openstick-mf32-wcnss-firmware qcom-msm8916-wcnss-openstick-mf32-nv
endef
TARGET_DEVICES += openstick-mf32

define Device/openstick-mf601
  $(Device/msm8916)
  DEVICE_VENDOR := BenTeng
  DEVICE_MODEL := OpenStick MF601
  DEVICE_DTS := msm8916-ufi-mf601
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-mf601-firmware qcom-msm8916-openstick-mf601-wcnss-firmware qcom-msm8916-wcnss-openstick-mf601-nv
endef
TARGET_DEVICES += openstick-mf601

define Device/openstick-wf2
  $(Device/msm8916)
  DEVICE_VENDOR := XinXun
  DEVICE_MODEL := OpenStick WF2
  DEVICE_DTS := msm8916-xinxun-wf2
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-wf2-firmware qcom-msm8916-openstick-wf2-wcnss-firmware qcom-msm8916-wcnss-openstick-wf2-nv kmod-gpio-button-hotplug kmod-button-hotplug kmod-input-gpio-keys kmod-input-gpio-keys-polled
endef
TARGET_DEVICES += openstick-wf2

define Device/openstick-jz02v10
  $(Device/msm8916)
  DEVICE_VENDOR := Zhihe
  DEVICE_MODEL := OpenStick JZ02V10
  DEVICE_DTS := msm8916-thwc-jz02v10
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-jz02v10-firmware qcom-msm8916-openstick-jz02v10-wcnss-firmware qcom-msm8916-wcnss-openstick-jz02v10-nv
endef
TARGET_DEVICES += openstick-jz02v10

define Device/openstick-sp970v11
  $(Device/msm8916)
  DEVICE_VENDOR := GeXing
  DEVICE_MODEL := OpenStick SP970V11
  DEVICE_DTS := msm8916-gexing-sp970
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-sp970v11-firmware qcom-msm8916-openstick-sp970v11-wcnss-firmware qcom-msm8916-wcnss-openstick-sp970v11-nv
endef
TARGET_DEVICES += openstick-sp970v11

define Device/openstick-sp970v10
  $(Device/msm8916)
  DEVICE_VENDOR := GeXing
  DEVICE_MODEL := OpenStick SP970V10
  DEVICE_DTS := msm8916-gexing-sp970
  DEVICE_DTS_DIR := ../dts
  DEVICE_PACKAGES := openstick-tweaks wpad-basic-wolfssl qcom-msm8916-modem-openstick-sp970v10-firmware qcom-msm8916-openstick-sp970v10-wcnss-firmware qcom-msm8916-wcnss-openstick-sp970v10-nv
endef
TARGET_DEVICES += openstick-sp970v10

endif
