include $(TOPDIR)/rules.mk

PKG_NAME:=mkbootargs
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/host-build.mk


define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) -o $(HOST_BUILD_DIR)/mkbootargs src/crc32.c src/mkbootargs.c
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/mkbootargs $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))