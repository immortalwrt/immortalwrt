include $(TOPDIR)/rules.mk

PKG_NAME:=ipq-wifi-qdm530
PKG_RELEASE:=1
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk

define Package/ipq-wifi-qdm530
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=ath11k board data for the Unbranded QDM530 5G CPE
  DEPENDS:=@TARGET_qualcommax_ipq50xx +ath11k-firmware-qcn9074
endef

define Package/ipq-wifi-qdm530/description
Board data files (BDF) for the Unbranded QDM530 5G CPE, ODM board
QDM530-GL_V2.1 (Qualcomm reference design AP-MP03.6-C1): the IPQ5018
internal 2.4 GHz radio and the QCN9074 5 GHz radio on PCIe.

Extracted from the vendor firmware, whose wifi_fw UBI volume is mounted at
/lib/firmware/IPQ5018/WIFI_FW. The vendor image ships many board variants;
these two are the ones its cnss driver actually loads, per its boot log:

  BDF IPQ5018/bdwlan.b24        -> IPQ5018-board.bin
  Downloading BDF: qcn9000/bdwlan.ba0 -> QCN9074-board.bin

Note there is deliberately no per-unit calibration data here. The board's
0:ART partition is blank (reads back all 0xff), so the vendor's own
/etc/init.d/ftm copies 0xff into its caldata files and the firmware runs on
the BDF defaults alone. There is therefore nothing per-device to preserve,
and nothing for an nvmem-cells caldata lookup to read.

These are raw BDFs, not board-2.bin containers, so they are installed under
the board.bin name that ath11k falls back to when board-2.bin is absent or
has no matching variant entry. Before submitting upstream to
firmware_qca-wireless, wrap them into a real board-2.bin with
ath11k-bdencoder from qca-swiss-army-knife, keyed to the identifiers ath11k
requests (bus, qmi-chip-id, qmi-board-id and the variant named by
qcom,ath11k-calibration-variant in ipq5018-qdm530.dts).
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/ipq-wifi-qdm530/install
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/IPQ5018/hw1.0
	$(INSTALL_DATA) ./files/IPQ5018-board.bin \
		$(1)/lib/firmware/ath11k/IPQ5018/hw1.0/board.bin
	$(INSTALL_DIR) $(1)/lib/firmware/ath11k/QCN9074/hw1.0
	$(INSTALL_DATA) ./files/QCN9074-board.bin \
		$(1)/lib/firmware/ath11k/QCN9074/hw1.0/board.bin
endef

# Remove the stock ath11k-firmware QCN9074 board-2.bin so ath11k falls back to
# the vendor board.bin installed above. The stock board-2.bin's generic
# qmi-board-id=255 entry gives the self-managed QCN9074 firmware a regdomain
# with NO 5 GHz rules (every 5 GHz channel comes up "disabled" and hostapd
# fails with "Not supported"); the vendor board.bin carries the UNII-3
# regdomain (channels 149-161) the 5 GHz radio actually needs. Runs at image
# build time (IPKG_INSTROOT set); DEPENDS on ath11k-firmware-qcn9074 ensures it
# is installed (and its board-2.bin present) before this postinst runs.
define Package/ipq-wifi-qdm530/postinst
#!/bin/sh
rm -f "$${IPKG_INSTROOT}/lib/firmware/ath11k/QCN9074/hw1.0/board-2.bin"
exit 0
endef

$(eval $(call BuildPackage,ipq-wifi-qdm530))
