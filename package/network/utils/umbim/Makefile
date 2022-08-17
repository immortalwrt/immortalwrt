include $(TOPDIR)/rules.mk

PKG_NAME:=umbim
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(PROJECT_GIT)/project/umbim.git
PKG_SOURCE_DATE:=2022-08-13
PKG_SOURCE_VERSION:=146bc77c98ace3d1cc672986669650d2e1da71f3
PKG_MIRROR_HASH:=3cf04858ff5c3f529904f9789b094aa9645ad41c91f553e6fc3fcd3cb341d359
PKG_MAINTAINER:=John Crispin <john@phrozen.org>

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/umbim
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=WWAN
  DEPENDS:=+libubox +kmod-usb-net +kmod-usb-net-cdc-mbim +wwan
  TITLE:=Control utility for mobile broadband modems
endef

define Package/umbim/description
  umbim is a command line tool for controlling mobile broadband modems using
  the MBIM-protocol.
endef

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include -ffunction-sections -fdata-sections

# Disable GCC 9 warning about pointers to members of a packed struct
GCCV_GE = $(filter $(1),$(shell echo -e "$(1)\n$(GCCV)" | sort -V | head -n1))
TARGET_CFLAGS += $(if $(call GCCV_GE,9),-Wno-address-of-packed-member)

TARGET_LDFLAGS += -Wl,--gc-sections

define Package/umbim/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/umbim $(1)/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,umbim))
