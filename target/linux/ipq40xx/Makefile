include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=ipq40xx
BOARDNAME:=Qualcomm Atheros IPQ40XX
FEATURES:=squashfs fpu ramdisk nand
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon-vfpv4
MAINTAINER:=John Crispin <john@phrozen.org>

KERNEL_PATCHVER:=5.4

KERNELNAME:=zImage Image dtbs

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-usb-dwc3-qcom \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig \
	kmod-ath10k wpad-openssl \
	kmod-usb3 kmod-usb-dwc3 ath10k-firmware-qca4019 \
	automount autosamba luci-app-ipsec-vpnd v2ray shadowsocks-libev-ss-redir shadowsocksr-libev-server \
	luci-app-unblockmusic luci-app-cpufreq luci-app-zerotier ca-certificates \
	htop fdisk e2fsprogs ethtool

$(eval $(call BuildTarget))
