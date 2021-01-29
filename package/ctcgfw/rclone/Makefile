####
 #  File: /Makefile
 #  Project: rclone
 #  File Created: Friday, 11th October 2019 4:50:49 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2019 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=rclone
PKG_VERSION:=1.53.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/rclone/rclone/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=acb53ccef1c1e638e53ca24933510d9be3666145372e1163470aa38414af8d48

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_RCLONE_COMPRESS_GOPROXY \
	CONFIG_RCLONE_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/rclone/rclone
GO_PKG_EXCLUDES:=test
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	github.com/rclone/rclone/fs.Version=v$(PKG_VERSION) \
	main.Version=v$(PKG_VERSION) \
	main.BuildUser=openwrt \
	main.BuildHost=openwrt

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/rclone
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=rsync for cloud storage.
	URL:=https://rclone.org
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/rclone/description
	Rclone ("rsync for cloud storage") is a command line program to sync root/usr/bin and
	directories to and from different cloud storage providers.
	Cloud storage providers:
		1Fichier, Alibaba Cloud (Aliyun) Object Storage System (OSS), Amazon Drive, Amazon S3,
		Backblaze B2, Box, Ceph, C14, DigitalOcean Spaces, Dreamhost, Dropbox, FTP,
		Google Cloud Storage, Google Drive, Google Photos, HTTP, Hubic, Jottacloud,
		IBM COS S3, Koofr, Memset Memstore, Mega, Microsoft Azure Blob Storage,
		Microsoft OneDrive, Minio, Nextcloud, OVH, OpenDrive, Openstack Swift,
		Oracle Cloud Storage, ownCloud, pCloud, premiumize.me, put.io, QingStor,
		Rackspace Cloud root/usr/bin, rsync.net, Scaleway, SFTP, Wasabi, WebDAV,
		Yandex Disk, The local root/usr/binystem.
endef

define Package/rclone/config
config RCLONE_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config RCLONE_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default n
endef

ifeq ($(CONFIG_RCLONE_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_RCLONE_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/rclone
endif
endef

$(eval $(call GoBinPackage,rclone))
$(eval $(call BuildPackage,rclone))
