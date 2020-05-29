include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=oaf
PKG_VERSION:=3.0
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/destan19/OpenAppFilter.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-05-26
PKG_SOURCE_VERSION:=bba08ecedaf97bd4bbb991e32b89eb81ad017da6

include $(INCLUDE_DIR)/package.mk

RSTRIP:=:

define KernelPackage/oaf
  SECTION:=Kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Netfilter Extensions
  TITLE:=open app filter kernel module
  FILES:=$(PKG_BUILD_DIR)/oaf.ko
  DEPENDS:=+kmod-ipt-conntrack
  KCONFIG:=
endef

define KernelPackage/oaf/description
  open appfilter kernel module
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	tar xJf $(DL_DIR)/$(PKG_SOURCE) -C $(TMP_DIR)
	$(CP) $(TMP_DIR)/$(PKG_NAME)-$(PKG_VERSION)/oaf/src/* $(PKG_BUILD_DIR)
	rm -rf $(TMP_DIR)/$(PKG_NAME)-$(PKG_VERSION)
endef

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,oaf))
