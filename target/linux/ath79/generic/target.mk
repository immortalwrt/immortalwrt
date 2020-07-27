BOARDNAME:=Generic
FEATURES += minor squashfs

DEFAULT_PACKAGES += wpad-basic-openssl

define Target/Description
	Build firmware images for generic Atheros AR71xx/AR913x/AR934x based boards.
endef
