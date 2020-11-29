#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bpytop
PKG_VERSION:=1.0.52
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/aristocratos/bpytop/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=15ef1bd094d945b9145343566c2596bb34922870e6be56c3cc028b33aff056fa
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=Python port of bashtop
  URL:=https://github.com/aristocratos/bpytop
  DEPENDS:=+python3-distutils +python3-logging +python3-openssl \
    +python3-psutil +python3-urllib
endef

define Package/$(PKG_NAME)/description
  Resource monitor that shows usage and stats for
  processor, memory, disks, network and processes.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bpytop.py $(1)/usr/bin/bpytop
	$(INSTALL_DIR) $(1)/usr/share/bpytop
	$(CP) -pr $(PKG_BUILD_DIR)/themes $(1)/usr/share/bpytop
	chmod 755 $(1)/usr/bin/bpytop
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
