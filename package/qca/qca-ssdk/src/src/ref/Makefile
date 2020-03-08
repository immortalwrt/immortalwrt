LOC_DIR=src/ref
LIB=REF

include $(PRJ_PATH)/make/config.mk

SRC_LIST_TMP=$(wildcard *.c)
SRC_SW_LIST=ref_misc.c ref_uci.c

ifeq (FALSE, $(SWCONFIG))
	SRC_LIST=$(filter-out $(SRC_SW_LIST), $(SRC_LIST_TMP))
else
	SRC_LIST=$(SRC_LIST_TMP)
endif


include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj

