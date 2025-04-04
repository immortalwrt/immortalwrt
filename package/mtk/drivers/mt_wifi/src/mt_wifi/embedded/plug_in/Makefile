export MODULE_FLAGS

KBUILD_EXTRA_SYMBOLS+= $(RT28xx_DIR)/os/linux/Module.symvers


ifdef CONFIG_WHNAT_SUPPORT
ifeq ($(CONFIG_WARP_V2), y)
obj-$(CONFIG_WHNAT_SUPPORT)+= warp_proxy/
else
obj-$(CONFIG_WHNAT_SUPPORT)+= whnat/
endif
endif

clean:
	find . -name "*.o" | xargs rm -rf
	find . -name "*.ko" | xargs rm -rf
	find . -name "modules.order" | xargs rm -rf
	find . -name "Module.symvers" | xargs rm -rf
install:
