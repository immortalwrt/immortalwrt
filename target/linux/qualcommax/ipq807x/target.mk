SUBTARGET:=ipq807x
BOARDNAME:=Qualcomm Atheros IPQ807x
DEFAULT_PACKAGES += ath11k-firmware-ipq8074 nss-firmware-ipq807x kmod-qca-nss-crypto

define Target/Description
	Build firmware images for Qualcomm Atheros IPQ807x based boards.
endef
