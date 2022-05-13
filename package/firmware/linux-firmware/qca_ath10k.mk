Package/ath10k-firmware-qca4019 = $(call Package/firmware-default,ath10k qca4019 firmware)
define Package/ath10k-firmware-qca4019/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA4019/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA4019/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA4019/hw1.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA4019/hw1.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca4019))

Package/ath10k-firmware-qca9887 = $(call Package/firmware-default,ath10k qca9887 firmware)
define Package/ath10k-firmware-qca9887/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9887/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9887/hw1.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/firmware-5.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9887/hw1.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA9887/hw1.0/board.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9887))

Package/ath10k-firmware-qca9888 = $(call Package/firmware-default,ath10k qca9888 firmware)
define Package/ath10k-firmware-qca9888/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9888/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9888/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9888/hw2.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA9888/hw2.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9888))

Package/ath10k-firmware-qca988x = $(call Package/firmware-default,ath10k qca988x firmware)
define Package/ath10k-firmware-qca988x/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA988X/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA988X/hw2.0/board.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA988X/hw2.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA988X/hw2.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca988x))

Package/ath10k-firmware-qca6174 = $(call Package/firmware-default,ath10k qca6174 firmware)
define Package/ath10k-firmware-qca6174/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA6174/hw2.1
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw2.1/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw2.1/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw2.1/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw2.1/firmware-5.bin
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA6174/hw3.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw3.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw3.0/
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA6174/hw3.0/firmware-6.bin \
		$(1)/lib/firmware/ath10k/QCA6174/hw3.0/firmware-6.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca6174))

Package/ath10k-firmware-qca99x0 = $(call Package/firmware-default,ath10k qca99x0 firmware)
define Package/ath10k-firmware-qca99x0/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA99X0/hw2.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA99X0/hw2.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA99X0/hw2.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA99X0/hw2.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca99x0))

Package/ath10k-firmware-qca9984 = $(call Package/firmware-default,ath10k qca9984 firmware)
define Package/ath10k-firmware-qca9984/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath10k/QCA9984/hw1.0
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9984/hw1.0/board-2.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/board-2.bin
	$(INSTALL_DATA) \
		$(PKG_BUILD_DIR)/ath10k/QCA9984/hw1.0/firmware-5.bin \
		$(1)/lib/firmware/ath10k/QCA9984/hw1.0/firmware-5.bin
endef
$(eval $(call BuildPackage,ath10k-firmware-qca9984))

