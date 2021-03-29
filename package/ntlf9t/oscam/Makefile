#
# Copyright (C) 2016-2017 Tomasz Maciej Nowak <tomek_n@o2.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=oscam
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/nx111/oscam.git
PKG_SOURCE_DATE:=2020-12-12
PKG_SOURCE_VERSION:=aafda4bca3c347698ef1dc32f7ebeff76378d55a
PKG_MIRROR_HASH:=d2cd4508ef80a3cbd8ef950e5446f2f754b339f179c4f774ef8660b39fd5c4c1

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILE:=COPYING
PKG_MAINTAINER:=Tomasz Maciej Nowak <tomek_n@o2.pl>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/oscam/config
	source "$(SOURCE)/Config.in"
endef

define Package/oscam
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Open Source Conditional Access Modul
  URL:=http://www.streamboard.tv/oscam
  DEPENDS:=+OSCAM_USE_LIBCRYPTO:libopenssl +OSCAM_USE_LIBUSB:libusb-1.0 +OSCAM_USE_PCSC:pcscd
endef

define Package/list-smargo
  SECTION:=net
  CATEGORY:=Network
  TITLE:=List Smargo readers
  URL:=http://www.streamboard.tv/oscam
  DEPENDS:=oscam @OSCAM_CARDREADER_SMARGO
endef

CONFIGURE_CMD:=./config.sh
CONFIGURE_ARGS+= \
	--restore \
	$(if $(CONFIG_OSCAM_WEBIF),--enable,--disable) WEBIF \
	$(if $(CONFIG_OSCAM_WEBIF_LIVELOG),--enable,--disable) WEBIF_LIVELOG \
	$(if $(CONFIG_OSCAM_WEBIF_JQUERY),--enable,--disable) WEBIF_JQUERY \
	$(if $(CONFIG_OSCAM_TOUCH),--enable,--disable) TOUCH \
	$(if $(CONFIG_OSCAM_WITH_SSL),--enable,--disable) WITH_SSL \
	$(if $(CONFIG_OSCAM_HAVE_DVBAPI),--enable,--disable) HAVE_DVBAPI \
	$(if $(CONFIG_OSCAM_READ_SDT_CHARSETS),--enable,--disable) READ_SDT_CHARSETS \
	$(if $(CONFIG_OSCAM_IRDETO_GUESSING),--enable,--disable) IRDETO_GUESSING \
	$(if $(CONFIG_OSCAM_CS_ANTICASC),--enable,--disable) CS_ANTICASC \
	$(if $(CONFIG_OSCAM_WITH_DEBUG),--enable,--disable) WITH_DEBUG \
	$(if $(CONFIG_OSCAM_MODULE_MONITOR),--enable,--disable) MODULE_MONITOR \
	$(if $(CONFIG_OSCAM_WITH_LB),--enable,--disable) WITH_LB \
	$(if $(CONFIG_OSCAM_CS_CACHEEX),--enable,--disable) CS_CACHEEX \
	$(if $(CONFIG_OSCAM_CW_CYCLE_CHECK),--enable,--disable) CW_CYCLE_CHECK \
	$(if $(CONFIG_OSCAM_LCDSUPPORT),--enable,--disable) LCDSUPPORT \
	$(if $(CONFIG_OSCAM_LEDSUPPORT),--enable,--disable) LEDSUPPORT \
	$(if $(CONFIG_OSCAM_CLOCKFIX),--enable,--disable) CLOCKFIX \
	$(if $(CONFIG_OSCAM_IPV6SUPPORT),--enable,--disable) IPV6SUPPORT \
	$(if $(CONFIG_OSCAM_MODULE_CAMD33),--enable,--disable) MODULE_CAMD33 \
	$(if $(CONFIG_OSCAM_MODULE_CAMD35),--enable,--disable) MODULE_CAMD35 \
	$(if $(CONFIG_OSCAM_MODULE_CAMD35_TCP),--enable,--disable) MODULE_CAMD35_TCP \
	$(if $(CONFIG_OSCAM_MODULE_NEWCAMD),--enable,--disable) MODULE_NEWCAMD \
	$(if $(CONFIG_OSCAM_MODULE_CCCAM),--enable,--disable) MODULE_CCCAM \
	$(if $(CONFIG_OSCAM_MODULE_CCCSHARE),--enable,--disable) MODULE_CCCSHARE \
	$(if $(CONFIG_OSCAM_MODULE_GBOX),--enable,--disable) MODULE_GBOX \
	$(if $(CONFIG_OSCAM_MODULE_RADEGAST),--enable,--disable) MODULE_RADEGAST \
	$(if $(CONFIG_OSCAM_MODULE_SCAM),--enable,--disable) MODULE_SCAM \
	$(if $(CONFIG_OSCAM_MODULE_SERIAL),--enable,--disable) MODULE_SERIAL \
	$(if $(CONFIG_OSCAM_MODULE_CONSTCW),--enable,--disable) MODULE_CONSTCW \
	$(if $(CONFIG_OSCAM_MODULE_PANDORA),--enable,--disable) MODULE_PANDORA \
	$(if $(CONFIG_OSCAM_MODULE_GHTTP),--enable,--disable) MODULE_GHTTP \
	$(if $(CONFIG_OSCAM_READER_NAGRA),--enable,--disable) READER_NAGRA \
	$(if $(CONFIG_OSCAM_READER_IRDETO),--enable,--disable) READER_IRDETO \
	$(if $(CONFIG_OSCAM_READER_CONAX),--enable,--disable) READER_CONAX \
	$(if $(CONFIG_OSCAM_READER_CRYPTOWORKS),--enable,--disable) READER_CRYPTOWORKS \
	$(if $(CONFIG_OSCAM_READER_SECA),--enable,--disable) READER_SECA \
	$(if $(CONFIG_OSCAM_READER_VIACCESS),--enable,--disable) READER_VIACCESS \
	$(if $(CONFIG_OSCAM_READER_VIDEOGUARD),--enable,--disable) READER_VIDEOGUARD \
	$(if $(CONFIG_OSCAM_READER_DRE),--enable,--disable) READER_DRE \
	$(if $(CONFIG_OSCAM_READER_TONGFANG),--enable,--disable) READER_TONGFANG \
	$(if $(CONFIG_OSCAM_READER_BULCRYPT),--enable,--disable) READER_BULCRYPT \
	$(if $(CONFIG_OSCAM_READER_GRIFFIN),--enable,--disable) READER_GRIFFIN \
	$(if $(CONFIG_OSCAM_READER_DGCRYPT),--enable,--disable) READER_DGCRYPT \
	$(if $(CONFIG_OSCAM_CARDREADER_PHOENIX),--enable,--disable) CARDREADER_PHOENIX \
	$(if $(CONFIG_OSCAM_CARDREADER_SC8IN1),--enable,--disable) CARDREADER_SC8IN1 \
	$(if $(CONFIG_OSCAM_CARDREADER_MP35),--enable,--disable) CARDREADER_MP35 \
	$(if $(CONFIG_OSCAM_CARDREADER_SMARGO),--enable,--disable) CARDREADER_SMARGO \
	$(if $(CONFIG_OSCAM_CARDREADER_STINGER),--enable,--disable) CARDREADER_STINGER \
	$(if $(CONFIG_OSCAM_CARDREADER_DRECAS),--enable,--disable) CARDREADER_DRECAS \
	--disable CARDREADER_INTERNAL_AZBOX \
	--disable CARDREADER_INTERNAL_COOLAPI \
	--disable CARDREADER_INTERNAL_COOLAPI2 \
	--disable CARDREADER_INTERNAL_SCI \
	--disable CARDREADER_DB2COM \
	--disable CARDREADER_STAPI \
	--disable CARDREADER_STAPI5

ifeq ($(CONFIG_OSCAM_USE_LIBUSB),y)
	MAKE_FLAGS += USE_LIBUSB=1
endif

ifeq ($(CONFIG_OSCAM_USE_PCSC),y)
	MAKE_FLAGS += USE_PCSC=1
endif

ifeq ($(CONFIG_OSCAM_USE_LIBCRYPTO),y)
	MAKE_FLAGS += USE_LIBCRYPTO=1
endif

ifeq ($(CONFIG_OSCAM_WITH_SSL),y)
	MAKE_FLAGS += USE_SSL=1
endif

MAKE_FLAGS += \
	CONF_DIR=/etc/oscam \
	OSCAM_BIN=Distribution/oscam \
	LIST_SMARGO_BIN=Distribution/list-smargo

define Package/oscam/conffiles
/etc/oscam/
endef

define Package/oscam/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Distribution/oscam $(1)/usr/bin/oscam

	$(INSTALL_DIR) $(1)/etc/oscam
	# $(CP) $(PKG_BUILD_DIR)/Distribution/doc/example/* $(1)/etc/oscam/
	$(INSTALL_CONF) ./files/oscam.conf $(1)/etc/oscam/oscam.conf
	$(INSTALL_CONF) ./files/oscam.user $(1)/etc/oscam/oscam.user
	$(INSTALL_CONF) ./files/oscam.dvbapi $(1)/etc/oscam/oscam.dvbapi
	$(INSTALL_CONF) ./files/oscam.server $(1)/etc/oscam/oscam.server

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/oscam.init $(1)/etc/init.d/oscam

	$(INSTALL_DIR) $(1)/usr/share/oscam
	$(INSTALL_BIN) ./files/oscam-watchdog.sh $(1)/usr/share/oscam/oscam-watchdog.sh

endef

define Package/list-smargo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/Distribution/list-smargo $(1)/usr/bin/list-smargo
endef

$(eval $(call BuildPackage,oscam))
$(eval $(call BuildPackage,list-smargo))
