include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=open-app-filter
PKG_VERSION:=5.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/destan19/OpenAppFilter.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2021-03-04
PKG_SOURCE_VERSION:=4483aab7d158031adc99b8669d5e7cf4f1023c14
PKG_MIRROR_HASH:=07ba70f41ae62704b8698703f2f3a0b06d4317d4292297514a7ffa13ccecebfc

include $(INCLUDE_DIR)/package.mk

RSTRIP:=:

define KernelPackage/oaf
	SECTION:=Kernel
	CATEGORY:=Kernel modules
	SUBMENU:=Netfilter Extensions
	TITLE:=open app filter kernel module
	FILES:=$(PKG_BUILD_DIR)/oaf.ko
	DEPENDS:=+kmod-ipt-conntrack
	AUTOLOAD:=$(call AutoLoad,0,oaf)
endef

define KernelPackage/oaf/description
	open appfilter kernel module
endef

define Package/appfilter
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=+libubox-lua +kmod-oaf
	TITLE:=App filter userspace module
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Configure
	$(call Build/Configure/Default)
	$(CP) $(PKG_BUILD_DIR)/oaf/src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define Package/appfilter/conffiles
/etc/config/appfilter
endef

define Package/appfilter/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/appfilter.sh $(1)/usr/bin/appfilter.sh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/gen_class.sh $(1)/usr/bin/gen_class.sh
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/appfilter.lua $(1)/usr/bin/appfilter
	$(INSTALL_DIR) $(1)/etc/appfilter
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/open-app-filter/files/feature.cfg $(1)/etc/appfilter/feature.cfg
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(PKG_BUILD_DIR)/open-app-filter/files/appfilter.config $(1)/etc/config/appfilter
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/open-app-filter/files/appfilter.init $(1)/etc/init.d/appfilter
endef

$(eval $(call KernelPackage,oaf))
$(eval $(call BuildPackage,appfilter))
