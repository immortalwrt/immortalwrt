#
# Copyright (C) 2021 ImmortalWrt
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinymembench
PKG_VERSION:=0.4
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ssvb/tinymembench.git
PKG_SOURCE_VERSION:=a2cf6d7e382e3aea1eb39173174d9fa28cad15f3
PKG_MIRROR_HASH:=b5b69e3d0c19e5a55bfebe386e29196c751612e30bd5aefc2ee1df11c18e0300

include $(INCLUDE_DIR)/package.mk

define Package/tinymembench
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=https://github.com/ssvb/tinymembench
  TITLE:=Simple benchmark for memory
endef

define Package/tinymembench/description
  Simple benchmark for memory throughput and latency.
endef

define Package/tinymembench/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinymembench $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tinymembench))
