ifneq ($(CONFIG_USE_APK),)
  DEFAULT_PACKAGES += apk-openssl
else
  DEFAULT_PACKAGES += opkg
endif
