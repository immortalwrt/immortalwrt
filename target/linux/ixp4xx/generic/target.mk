BOARDNAME:=Generic

DEFAULT_PACKAGES+= wpad-basic-openssl

define Target/Description
	Build firmware images for ixp4xx based boards that boot from internal flash
	(e.g : Linksys NSLU2, ...)
endef

