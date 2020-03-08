LOC_DIR=src/adpt/cppe
LIB=ADPT

include $(PRJ_PATH)/make/config.mk

SRC_LIST=

ifeq (TRUE, $(IN_PORTCONTROL))
  SRC_LIST += adpt_cppe_portctrl.c
endif

ifeq (, $(findstring HPPE, $(SUPPORT_CHIP)))
  SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
