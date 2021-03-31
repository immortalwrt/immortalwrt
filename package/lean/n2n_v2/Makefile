#
# Copyright (C) 2020 - ntop.org and contributors
#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=n2n
PKG_VERSION:=2.8
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/ntop/n2n/tar.gz/$(PKG_VERSION)?
PKG_HASH:=ad9ab95a79dcf6b412c36489f50de54bb54417b5cea854eb54722b6cb8f0fd04

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=Emanuele Faranda <faranda@ntop.org>

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/n2n/template
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=N2N Peer-to-peer VPN
  URL:=http://www.ntop.org/n2n
endef

define Package/n2n-edge
  $(call Package/n2n/template)
  TITLE+= client (edge node)
  DEPENDS+=+kmod-tun +resolveip +libopenssl +libcap +libzstd
endef

define Package/n2n-supernode
  $(call Package/n2n/template)
  TITLE+= server (supernode)
  DEPENDS+=+libcap
endef

define Package/n2n-edge/description
  The client node for the N2N infrastructure
endef

define Package/n2n-supernode/description
  The supernode for the N2N infrastructure
endef

define Build/Configure
	( cd $(PKG_BUILD_DIR); ./autogen.sh )
	$(call Build/Configure/Default)
endef

define Package/n2n-edge/conffiles
/etc/config/n2n-edge.conf
endef

define Package/n2n-supernode/conffiles
/etc/config/n2n-supernode.conf
endef

define Package/n2n-edge/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/edge $(1)/usr/bin/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/n2n_v2.config $(1)/etc/config/n2n_v2
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/n2n_v2.init $(1)/etc/init.d/n2n_v2
endef

define Package/n2n-supernode/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/supernode $(1)/usr/bin/
endef

$(eval $(call BuildPackage,n2n-edge))
$(eval $(call BuildPackage,n2n-supernode))
