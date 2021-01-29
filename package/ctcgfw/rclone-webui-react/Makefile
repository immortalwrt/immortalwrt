####
 #  File: /Makefile
 #  Project: rclone-webui-react
 #  File Created: Thursday, 10th October 2019 5:57:39 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #	Copyright (C) 2019 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone-webui-react
PKG_VERSION:=2.0.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://github.com/rclone/rclone-webui-react/releases/download/v$(PKG_VERSION)/currentbuild.zip?
PKG_HASH:=38ca0a73071b70d25ecc5995f0da912edd448669715c1b177a44fafa08460990

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

include $(INCLUDE_DIR)/package.mk

UNZIP_CMD:=unzip -q -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)

define Package/rclone-webui-react
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Cloud Manager
	TITLE:=A reactjs based web UI for rclone
	URL:=https://github.com/rclone/rclone-webui-react
	DEPENDS:=+rclone
	PKGARCH:=all
endef

define Package/rclone-webui-react/description
	A full fledged UI for the rclone cloud sync tool.
endef


define Build/Compile
	true
endef

define Package/rclone-webui-react/install
	$(INSTALL_DIR) $(1)/www/rclone-webui-react
	$(CP) $(PKG_BUILD_DIR)/build/* $(1)/www/rclone-webui-react
endef

$(eval $(call BuildPackage,rclone-webui-react))
