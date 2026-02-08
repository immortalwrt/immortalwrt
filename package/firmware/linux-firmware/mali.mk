Package/panthor-firmware = $(call Package/firmware-default,Gen10 Arm Mali GPUs firmware)
define Package/panthor-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/arm/mali/arch10.8
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/arm/mali/arch10.8/mali_csffw.bin \
		$(1)/lib/firmware/arm/mali/arch10.8/
endef
$(eval $(call BuildPackage,panthor-firmware))
