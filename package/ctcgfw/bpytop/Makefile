#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bpytop
PKG_VERSION:=1.0.60
PKG_RELEASE:=2

PYPI_NAME:=bpytop
PKG_HASH:=48cf23725a873f77f562a89b18ac91dedfd39cf35586582a7668580da7b12826

PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE

include $(TOPDIR)/feeds/packages/lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/python/python3-package.mk

define Package/bpytop
  SECTION:=admin
  CATEGORY:=Administration
  TITLE:=Python port of bashtop
  URL:=https://github.com/aristocratos/bpytop
  DEPENDS:=+python3-psutil +python3-setuptools
endef

define Package/bpytop/description
  Resource monitor that shows usage and stats for
  processor, memory, disks, network and processes.
endef

define Package/bpytop/install
	$(call Py3Package/$(1)/install,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_BUILD_DIR)/bpytop-themes $(1)/usr/share/bpytop
endef

$(eval $(call Py3Package,bpytop))
$(eval $(call BuildPackage,bpytop))
#$(eval $(call BuildPackage,bpytop-src))
