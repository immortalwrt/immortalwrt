LOC_DIR=src/hsl/hppe
LIB=HSL

include $(PRJ_PATH)/make/config.mk

SRC_LIST=hppe_init.c hppe_reg_access.c hppe_ip.c hppe_flow.c
SRC_LIST+=hppe_qm.c hppe_qos.c
SRC_LIST+=hppe_vsi.c hppe_fdb.c hppe_stp.c hppe_portvlan.c
SRC_LIST+=hppe_sec.c
SRC_LIST+=hppe_bm.c
SRC_LIST+=hppe_mib.c
SRC_LIST+=hppe_acl.c hppe_servcode.c hppe_ctrlpkt.c
SRC_LIST+=hppe_rss.c
SRC_LIST+=hppe_pppoe.c
SRC_LIST+=hppe_portctrl.c
SRC_LIST+=hppe_mirror.c
SRC_LIST+=hppe_trunk.c
SRC_LIST+=hppe_acl.c
SRC_LIST+=hppe_policer.c
SRC_LIST+=hppe_shaper.c
SRC_LIST+=hppe_xgmacmib.c
SRC_LIST+=hppe_xgportctrl.c
SRC_LIST+=hppe_global.c
SRC_LIST+=hppe_uniphy.c
ifeq (, $(findstring HPPE, $(SUPPORT_CHIP)))
  SRC_LIST=
endif

include $(PRJ_PATH)/make/components.mk
include $(PRJ_PATH)/make/defs.mk
include $(PRJ_PATH)/make/target.mk

all: dep obj
