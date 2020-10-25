include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=oaf
PKG_VERSION:=3.0
PKG_RELEASE:=2

PKG_SOURCE_URL:=https://github.com/destan19/OpenAppFilter.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-10-20
PKG_SOURCE_VERSION:=b235da54c45ac4cd16ec96e1ec832d3851a247da

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
