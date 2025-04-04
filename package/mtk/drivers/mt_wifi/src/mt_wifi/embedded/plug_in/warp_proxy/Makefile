ccflags-y += $(MODULE_FLAGS)

MODULE_NAME=mtk_warp_proxy

CHIP_DIR=chips

OBJS:= warp_proxy.o
#OBJS += $(CHIP_DIR)/warp_wifi_mt7615.o
ifeq ($(CONFIG_CHIP_MT7915),y)
OBJS += $(CHIP_DIR)/warp_wifi_mt7915.o
endif
ifeq ($(CONFIG_CHIP_MT7986),y)
OBJS += $(CHIP_DIR)/warp_wifi_mt7986.o
endif
ifeq ($(CONFIG_CHIP_MT7916),y)
OBJS += $(CHIP_DIR)/warp_wifi_mt7916.o
endif
ifeq ($(CONFIG_CHIP_MT7981),y)
OBJS += $(CHIP_DIR)/warp_wifi_mt7981.o
endif
#######################################
# For integrating to kernel source
######################################
obj-m += $(MODULE_NAME).o


$(MODULE_NAME)-objs += $(OBJS)

install:

clean:
	rm -rf *.o *.ko *.modules .*.cmd .*.o *.order $(MODULE_NAME).mod.c
