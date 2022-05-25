BOARDNAME:=MIPS 74K
CPU_TYPE:=74kc

DEFAULT_PACKAGES += wpad-basic-openssl

define Target/Description
	Build firmware for Broadcom BCM47xx and BCM53xx devices with
	MIPS 74K CPU.
endef
