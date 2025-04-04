ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
WIFI_DRV_PATH := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/..
else
ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
WIFI_DRV_PATH = drivers/net/wireless/mediatek
else
WIFI_DRV_PATH = drivers/net/wireless
endif
endif

EXTRA_CFLAGS = -I$(WIFI_DRV_PATH)/mt_wifi/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/embedded/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/ate/include \
				-I$(WIFI_DRV_PATH)/mt_wifi/include/mcu \
				-I$(WIFI_DRV_PATH)/mt_wifi/embedded/dbg_txcmd/include \
				-I$(WIFI_DRV_PATH)/warp_driver/warp \
				-I$(WIFI_DRV_PATH)/warp_driver/warp/mcu

ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
GLOBAL_DEFINES = include/uapi/linux/wapp
EXTRA_CFLAGS +=	-I$(GLOBAL_DEFINES)
endif

# CONFIG_MT7915_MT7916_COEXIST_COMPATIBLE only enable in CPE project
ifneq ($(CONFIG_CPE_SUPPORT),y)
ifeq ($(CONFIG_MT7915_MT7916_COEXIST_COMPATIBLE),y)
CONFIG_MT7915_MT7916_COEXIST_COMPATIBLE=n
endif
endif

ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ifeq ($(CONFIG_CHIP_MT7663E),y)
DRV_NAME = mt7663e
else
DRV_NAME = mt_wifi
endif
else
DRV_NAME = mt_wifi
endif

SRC_DIR = ../mt_wifi

ifneq ($(CONFIG_GCC_VERSION), )
ifeq ($(shell expr $(CONFIG_GCC_VERSION) \>= 8.4.0), 1)
# this option only have after 8.x.x
# workaround of bin compare on preflight will get the .rodata.str1.8 and it's abs path of __FILE__
# will false the compare due to build server folder is not the same
ccflags-y += -fmacro-prefix-map=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))=
endif
endif

SRC_EMBEDDED_DIR = ../mt_wifi/embedded
SERV_DIR = ../wlan_service
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
EXTRA_CFLAGS += -DCONFIG_SUPPORT_OPENWRT

ifneq ($(CONFIG_FIRST_IF_NONE),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD=$(CONFIG_RT_FIRST_CARD)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_IF_RF_OFFSET=$(CONFIG_RT_FIRST_IF_RF_OFFSET)
ifeq ($(CONFIG_FIRST_IF_EEPROM_PROM),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"prom\""
else
ifeq ($(CONFIG_FIRST_IF_EEPROM_EFUSE),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"efuse\""
else
ifeq ($(CONFIG_FIRST_IF_EEPROM_FLASH),y)
EXTRA_CFLAGS += -DCONFIG_RT_FIRST_CARD_EEPROM="\"flash\""
endif
endif
endif
endif	#CONFIG_FIRST_IF_NONE
ifneq ($(CONFIG_SECOND_IF_NONE),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD=$(CONFIG_RT_SECOND_CARD)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_IF_RF_OFFSET=$(CONFIG_RT_SECOND_IF_RF_OFFSET)
ifeq ($(CONFIG_SECOND_IF_EEPROM_PROM),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD_EEPROM="\"prom\""
else
ifeq ($(CONFIG_SECOND_IF_EEPROM_EFUSE),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD_EEPROM="\"efuse\""
else
ifeq ($(CONFIG_SECOND_IF_EEPROM_FLASH),y)
EXTRA_CFLAGS += -DCONFIG_RT_SECOND_CARD_EEPROM="\"flash\""
endif
endif
endif
endif	#CONFIG_SECOND_IF_NONE
ifneq ($(CONFIG_THIRD_IF_NONE),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_IF_RF_OFFSET=$(CONFIG_RT_THIRD_IF_RF_OFFSET)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD=$(CONFIG_RT_THIRD_CARD)
ifeq ($(CONFIG_THIRD_IF_EEPROM_PROM),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD_EEPROM="\"prom\""
else
ifeq ($(CONFIG_THIRD_IF_EEPROM_EFUSE),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD_EEPROM="\"efuse\""
else
ifeq ($(CONFIG_THIRD_IF_EEPROM_FLASH),y)
EXTRA_CFLAGS += -DCONFIG_RT_THIRD_CARD_EEPROM="\"flash\""
endif
endif
endif
endif	#CONFIG_THIRD_IF_NONE
endif	#CONFIG_SUPPORT_OPENWRT

ifeq ($(CONFIG_FIRST_IF_EPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_EPAELNA
else
ifeq ($(CONFIG_FIRST_IF_IPAILNA),y)
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_IPAILNA
else
ifeq ($(CONFIG_FIRST_IF_IPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_FIRST_IF_IPAELNA
endif
endif
endif
ifeq ($(CONFIG_SECOND_IF_EPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_SECOND_IF_EPAELNA
else
ifeq ($(CONFIG_SECOND_IF_IPAILNA),y)
EXTRA_CFLAGS += -DCONFIG_SECOND_IF_IPAILNA
else
ifeq ($(CONFIG_SECOND_IF_IPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_SECOND_IF_IPAELNA
endif
endif
endif
ifeq ($(CONFIG_THIRD_IF_EPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_THIRD_IF_EPAELNA
else
ifeq ($(CONFIG_THIRD_IF_IPAILNA),y)
EXTRA_CFLAGS += -DCONFIG_THIRD_IF_IPAILNA
else
ifeq ($(CONFIG_THIRD_IF_IPAELNA),y)
EXTRA_CFLAGS += -DCONFIG_THIRD_IF_IPAELNA
endif
endif
endif
# support built-in/ko for Openwrt/Linux SDK
obj-$(CONFIG_MT_AP_SUPPORT) += $(DRV_NAME).o

ifeq ($(CONFIG_MTK_EMI_7622),y)
EXTRA_CFLAGS += -DCONFIG_ARCH_MT7622
endif

ifeq ($(CONFIG_CPE_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_CPE_SUPPORT
ifeq ($(CONFIG_PLATFORM), mt6890)
EXTRA_CFLAGS += -DMT6890
else
ifeq ($(CONFIG_PLATFORM), mt6990)
EXTRA_CFLAGS += -DMT6990
endif
endif
endif

EXTRA_CFLAGS += -DWIFI_SYS_FW_V1
EXTRA_CFLAGS += -DWIFI_SYS_FW_V2
########################################################
# 1.5K Beacon
#EXTRA_CFLAGS += -DBCN_V2_SUPPORT
########################################################
# Common files
#		$(SRC_EMBEDDED_DIR)/common/cmm_mumimo.o\
########################################################
cmm_objs := $(SRC_EMBEDDED_DIR)/common/action.o\
		$(SRC_EMBEDDED_DIR)/common/ra_ac_q_mgmt.o\
		$(SRC_EMBEDDED_DIR)/common/ba_action.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/radio_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/omac_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/wmm_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/wtbl_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/hdev_basic.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_ops_v1.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_ops_v2.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_cmd.o\
		$(SRC_EMBEDDED_DIR)/hw_ctrl/hw_ctrl_basic.o\
		$(SRC_DIR)/hw_ctrl/hw_init.o\
		$(SRC_EMBEDDED_DIR)/common/bcn.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_aes.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_cfg.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_cmd.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_data.o\
		$(SRC_EMBEDDED_DIR)/common/capi.o\
		$(SRC_EMBEDDED_DIR)/common/qm.o\
		$(SRC_EMBEDDED_DIR)/common/fp_qm.o\
		$(SRC_EMBEDDED_DIR)/common/fq_qm.o\
		$(SRC_EMBEDDED_DIR)/common/fp_fair_qm.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_info.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_profile.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_radar.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_sync.o\
		$(SRC_EMBEDDED_DIR)/common/wifi_sys_info.o\
		$(SRC_EMBEDDED_DIR)/common/wifi_sys_notify.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_basic.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_phy.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_ht.o\
		$(SRC_EMBEDDED_DIR)/wlan_config/config_vht.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_basic.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_phy.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_ht.o\
		$(SRC_EMBEDDED_DIR)/mgmt/be_vht.o\
		$(SRC_EMBEDDED_DIR)/fsm/fsm_sync.o\
		$(SRC_EMBEDDED_DIR)/fsm/fsm_assoc.o\
		$(SRC_EMBEDDED_DIR)/fsm/fsm_cntl.o\
		$(SRC_EMBEDDED_DIR)/fsm/fsm_auth.o\
                $(SRC_EMBEDDED_DIR)/mgmt/bss_ops.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_sanity.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_sec.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_tkip.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_wep.o\
		$(SRC_EMBEDDED_DIR)/security/cmm_wpa.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_aes.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_arc4.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_hmac.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_md5.o\
		$(SRC_EMBEDDED_DIR)/security/crypt_sha2.o\
		$(SRC_EMBEDDED_DIR)/common/eeprom.o\
		$(SRC_EMBEDDED_DIR)/common/mlme.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_ht.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_dev.o\
		$(SRC_EMBEDDED_DIR)/common/rtmp_init.o\
		$(SRC_EMBEDDED_DIR)/common/rtmp_init_inf.o\
		$(SRC_EMBEDDED_DIR)/common/spectrum.o\
		$(SRC_EMBEDDED_DIR)/common/rtmp_timer.o\
		$(SRC_EMBEDDED_DIR)/common/rt_channel.o\
		$(SRC_EMBEDDED_DIR)/common/op_class.o\
		$(SRC_EMBEDDED_DIR)/common/rt_os_util.o\
		$(SRC_EMBEDDED_DIR)/common/scan.o\
                 $(SRC_EMBEDDED_DIR)/common/scan_util.o\
		$(SRC_EMBEDDED_DIR)/common/sys_log.o\
		$(SRC_EMBEDDED_DIR)/common/txpower.o\
		$(SRC_EMBEDDED_DIR)/common/icap.o\
		$(SRC_EMBEDDED_DIR)/common/vendor.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_info_element.o\
                $(SRC_EMBEDDED_DIR)/common/log_time.o\
		$(SRC_DIR)/chips/rtmp_chip.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_hw.o\
		$(SRC_EMBEDDED_DIR)/mgmt/mgmt_entrytb.o\
		$(SRC_EMBEDDED_DIR)/common/wdev.o\
		$(SRC_EMBEDDED_DIR)/common/multi_hif.o\
		$(SRC_DIR)/os/linux/rt_profile.o\
		$(SRC_DIR)/os/linux/tm.o\
		$(SRC_DIR)/txpwr/txpwr.o\
		$(SRC_DIR)/txpwr/single_sku.o\
                $(SRC_DIR)/phystate/phystate.o\
		$(SRC_EMBEDDED_DIR)/common/afc.o\
		$(SRC_EMBEDDED_DIR)/common/dabs_qos.o\
		$(SRC_EMBEDDED_DIR)/common/kernel_rps_adjust.o\
		$(SRC_EMBEDDED_DIR)/common/cmm_rvr_dbg.o\
		$(SRC_DIR)/protocol/protection.o\
		$(SRC_EMBEDDED_DIR)/common/misc_app.o\
		$(SRC_EMBEDDED_DIR)/common/fd.o

EXTRA_CFLAGS += -DMULTI_PROFILE
cmm_objs += $(SRC_EMBEDDED_DIR)/common/multi_profile.o

ifeq ($(CONFIG_DEFAULT_5G_PROFILE),y)
EXTRA_CFLAGS += -DDEFAULT_5G_PROFILE
endif

#EXTRA_CFLAGS += -DANDLINK_FEATURE_SUPPORT -DANDLINK_HOSTNAME_IP -DTXRX_STAT_SUPPORT
#EXTRA_CFLAGS += -DANDLINK_V4_0

#ifeq ($(CONFIG_TR181_SUPPORT),y)
EXTRA_CFLAGS += -DTR181_SUPPORT
EXTRA_CFLAGS += -DSTAT_ENHANCE_SUPPORT
#endif

ifeq ($(CONFIG_ZERO_LOSS_CSA_SUPPORT),y)
EXTRA_CFLAGS += -DZERO_LOSS_CSA_SUPPORT -DOFFCHANNEL_ZERO_LOSS -DNF_SUPPORT_V2
endif

#ifeq ($(CONFIG_GNAX_ONLY_SUPPORT),y)
#EXTRA_CFLAGS += -DGN_MIXMODE_SUPPORT
#endif
#
ifeq ($(CONFIG_WIFI_TWT_SUPPORT),y)
EXTRA_CFLAGS += -DWIFI_TWT_SUPPORT
cmm_objs +=$(SRC_DIR)/protocol/twt_core.o
cmm_objs +=$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/twt_ctrl.o
endif

EXTRA_CFLAGS += -DSTA_LP_PHASE_2_SUPPORT

ifeq ($(CONFIG_DOT11_HE_AX),y)
cmm_objs +=$(SRC_EMBEDDED_DIR)/hw_ctrl/hdev/bcolor_ctrl.o
endif

ifeq ($(CONFIG_CFG_SUPPORT_FALCON_SR),y)
EXTRA_CFLAGS += -DCFG_SUPPORT_FALCON_SR
cmm_objs +=$(SRC_EMBEDDED_DIR)/common/sr_cmd.o
endif

ifeq ($(CONFIG_CFG_SUPPORT_FALCON_PP),y)
EXTRA_CFLAGS += -DCFG_SUPPORT_FALCON_PP
cmm_objs +=$(SRC_EMBEDDED_DIR)/common/pp_cmd.o
endif

ifeq ($(CONFIG_RPS_EFFICIENCY),y)
EXTRA_CFLAGS += -DDYNAMIC_STEERING_LOADING
cmm_objs +=$(SRC_EMBEDDED_DIR)/common/dyn_steering_ld.o
endif

ifeq ($(CONFIG_PROFILING_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_PROFILING_SUPPORT
cmm_objs +=$(SRC_EMBEDDED_DIR)/profiling/profiling.o
endif

ifeq ($(CONFIG_MLME_MULTI_QUEUE_SUPPORT),y)
EXTRA_CFLAGS += -DMLME_MULTI_QUEUE_SUPPORT
endif

ifeq ($(CONFIG_MASK_PARTIAL_MACADDR),y)
EXTRA_CFLAGS += -DMASK_PARTIAL_MACADDR
endif

########################################################
# Coex related files
########################################################
ifeq ($(CONFIG_COEX_SUPPORT),y)
   EXTRA_CFLAGS += -DCOEX_SUPPORT

    cmm_objs += $(SRC_EMBEDDED_DIR)/mcu/bt_coex.o
endif

# For wifi coex in CPE project
ifeq ($(CONFIG_WIFI_MD_COEX_SUPPORT),y)
	EXTRA_CFLAGS += -DWIFI_MD_COEX_SUPPORT

	cmm_objs += $(SRC_EMBEDDED_DIR)/mcu/wifi_md_coex.o
	cmm_objs += $(SRC_EMBEDDED_DIR)/common/safe_chn.o
endif

ifeq ($(CONFIG_COEX_DIRECT_PATH_SUPPORT),y)
	EXTRA_CFLAGS += -DCOEX_DIRECT_PATH
endif
########################################################
# Rate adaptation related files
########################################################
rate_objs := $(SRC_DIR)/rate_ctrl/ra_ctrl_mt.o\
		$(SRC_DIR)/rate_ctrl/ra_ctrl_mt_drv.o\
		$(SRC_DIR)/rate_ctrl/ra_wrapper_embedded.o\
		$(SRC_DIR)/rate_ctrl/ra_cfg.o

#ifeq ($(CONFIG_NEW_RATE_ADAPT_SUPPORT),y)
#    EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
#    rate_objs += $(SRC_DIR)/rate_ctrl/alg_grp_mt.o
#    rate_objs += $(SRC_DIR)/rate_ctrl/ra_table.o
#endif

ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
endif

########################################################
# ASIC related files
########################################################
asic_objs := $(SRC_DIR)/phy/phy.o

########################################################
# Spec feature related files
########################################################
spec_objs += $(SRC_EMBEDDED_DIR)/common/ps.o

ifeq ($(CONFIG_UAPSD),y)
    EXTRA_CFLAGS += -DUAPSD_SUPPORT -DUAPSD_DEBUG
    spec_objs += $(SRC_EMBEDDED_DIR)/common/uapsd.o
endif

ifeq ($(CONFIG_WTBL_TDD_SUPPORT),y)
    EXTRA_CFLAGS += -DWTBL_TDD_SUPPORT
    spec_objs += $(SRC_EMBEDDED_DIR)/mgmt/wtbl_tdd.o
    spec_objs += $(SRC_EMBEDDED_DIR)/mgmt/wtbl_tdd_sm.o
endif

ifeq ($(CONFIG_SW_CONNECT_SUPPORT),y)
    #increase Max connect STA with FixRate S/W Meethods
    EXTRA_CFLAGS += -DSW_CONNECT_SUPPORT
endif

ifeq ($(CONFIG_MT_MAC),y)
	spec_objs += $(SRC_EMBEDDED_DIR)/common/mt_ps.o
	spec_objs += $(SRC_EMBEDDED_DIR)/common/mt_io.o
	spec_objs += $(SRC_EMBEDDED_DIR)/common/txs.o
endif

# WSC
ifeq ($(CONFIG_WSC_INCLUDED),y)
    EXTRA_CFLAGS += -DWSC_INCLUDED -DWSC_SINGLE_TRIGGER

    ifneq ($(CONFIG_USER_GOAHEAD_HTTPD),)
        EXTRA_CFLAGS += -DSDK_GOAHEAD_HTTPD
    endif

    ifneq ($(CONFIG_USER_LIGHTY),)
        EXTRA_CFLAGS += -DSDK_USER_LIGHTY
    endif

    ifneq ($(CONFIG_MT_AP_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_AP_SUPPORT
	EXTRA_CFLAGS += -DWSC_STA_SUPPORT
    endif

    ifneq ($(CONFIG_MT_STA_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_STA_SUPPORT
    endif

    spec_objs += $(SRC_EMBEDDED_DIR)/common/wsc.o\
                    $(SRC_EMBEDDED_DIR)/common/wsc_tlv.o\
                    $(SRC_EMBEDDED_DIR)/security/crypt_dh.o\
                    $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o\
                    $(SRC_EMBEDDED_DIR)/common/wsc_ufd.o

    ifeq ($(CONFIG_WSC_V2_SUPPORT),y)
        EXTRA_CFLAGS += -DWSC_V2_SUPPORT
        spec_objs += $(SRC_EMBEDDED_DIR)/common/wsc_v2.o
    endif
endif

# VHT
ifeq ($(CONFIG_DOT11_VHT_AC),y)
    EXTRA_CFLAGS += -DDOT11_VHT_AC
    spec_objs += $(SRC_EMBEDDED_DIR)/mgmt/mgmt_vht.o
	# WFA VHT R2 PF
	ifeq ($(CONFIG_WFA_VHT_R2_PF),y)
		EXTRA_CFLAGS += -DWFA_VHT_R2_PF
	endif
endif

# HE
ifeq ($(CONFIG_DOT11_HE_AX),y)
    EXTRA_CFLAGS += -DDOT11_HE_AX
    spec_objs += $(SRC_EMBEDDED_DIR)/mgmt/mgmt_he.o\
	$(SRC_EMBEDDED_DIR)/mgmt/be_he.o\
	$(SRC_EMBEDDED_DIR)/wlan_config/config_he.o\
	$(SRC_EMBEDDED_DIR)/mgmt/bss_color.o
endif

# DPP
#CONFIG_DPP_FEATURE=n
#ifeq ($(CONFIG_DPP_FEATURE),y)
#EXTRA_CFLAGS += -DCHANNEL_SWITCH_MONITOR_CONFIG
#EXTRA_CFLAGS += -DDPP_SUPPORT
#EXTRA_CFLAGS += -DDPP_R2_SUPPORT
#endif

# WAPI
ifeq ($(CONFIG_WAPI_SUPPORT),y)
    EXTRA_CFLAGS += -DWAPI_SUPPORT
    ifeq ($(CONFIG_RALINK_RT3052),y)
        EXTRA_CFLAGS += -DSOFT_ENCRYPT
    endif

  spec_objs += $(SRC_EMBEDDED_DIR)/security/wapi.o\
                $(SRC_EMBEDDED_DIR)/security/wapi_sms4.o\
		$(SRC_EMBEDDED_DIR)/security/wapi_crypt.o
endif

# WPA3
ifeq ($(CONFIG_WPA3_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11_SAE_SUPPORT -DDOT11_SUITEB_SUPPORT -DDOT11W_PMF_SUPPORT
    EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN -DDOT11_SAE_PWD_ID_SUPPORT

  spec_objs += $(SRC_EMBEDDED_DIR)/security/sae.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/bn_lib.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/ecc.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o
endif

# OWE
ifeq ($(CONFIG_OWE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_OWE_SUPPORT -DDOT11_SAE_OPENSSL_BN -DDOT11W_PMF_SUPPORT

  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/bn_lib.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/ecc.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/owe.o
  spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o
endif

# ACM
ifeq ($(CONFIG_WMM_ACM_SUPPORT),y)
    EXTRA_CFLAGS += -DWMM_ACM_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/acm_edca.o\
            $(SRC_EMBEDDED_DIR)/common/acm_comm.o\
            $(SRC_EMBEDDED_DIR)/common/acm_iocl.o
endif

#PMF
ifeq ($(CONFIG_MT_MAC),y)
ifeq ($(CONFIG_DOT11W_PMF_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11W_PMF_SUPPORT -DBCN_PROTECTION_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o
endif
endif

# LLTD
ifeq ($(CONFIG_LLTD_SUPPORT),y)
    EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

#RRM   #need to fix STA mode code
ifeq ($(CONFIG_DOT11K_RRM_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPPLE_11K_IOT
    spec_objs += $(SRC_EMBEDDED_DIR)/common/rrm_tlv.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm_sanity.o
endif

# KVRH
ifeq ($(CONFIG_KVRH_SUPPORT),y)
    EXTRA_CFLAGS += -DQUERY_KVRH_SUPPORT
endif

# THERMAL
ifeq ($(CONFIG_THERMAL_PROTECT_SUPPORT),y)
    EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
endif

# TPC
ifeq ($(CONFIG_TPC_SUPPORT),y)
    EXTRA_CFLAGS += -DTPC_SUPPORT
endif

# FTM
ifeq ($(CONFIG_FTM_SUPPORT),y)
    EXTRA_CFLAGS += -DFTM_SUPPORT -DFTM_INITIATOR
    EXTRA_CFLAGS += -DTMR_SUPPORT
    EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/ftm.o
    spec_objs += $(SRC_DIR)/protocol/tmr.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/rrm_tlv.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm.o \
				 $(SRC_EMBEDDED_DIR)/common/rrm_sanity.o
endif

ifeq ($(CONFIG_PASSPOINT_R2),y)

ifeq ($(CONFIG_CFG80211_SUPPORT),y)
	EXTRA_CFLAGS += -DCONFIG_DOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DCONFIG_HOTSPOT_R2 -DCONFIG_HOTSPOT_R3 -DHOSTAPD_11V_BTM_SUPPORT
else
	EXTRA_CFLAGS += -DCONFIG_DOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DCONFIG_HOTSPOT_R2 -DCONFIG_HOTSPOT_R3 -DWAPP_SUPPORT
endif
	spec_objs += $(SRC_EMBEDDED_DIR)/common/wnm.o\
				$(SRC_EMBEDDED_DIR)/common/gas.o\
				$(SRC_EMBEDDED_DIR)/common/hotspot.o
ifneq ($(CONFIG_CFG80211_SUPPORT),y)
       spec_objs +=  $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif

else
ifeq ($(CONFIG_CFG80211_SUPPORT),y)
	EXTRA_CFLAGS += -DCONFIG_PROXY_ARP

	spec_objs += $(SRC_EMBEDDED_DIR)/common/wnm.o\
		     $(SRC_EMBEDDED_DIR)/common/hotspot.o
endif
endif

ifneq ($(CONFIG_CFG80211_SUPPORT),y)
ifeq ($(CONFIG_WNM_SUPPORT),y)
        EXTRA_CFLAGS += -DCONFIG_DOT11V_WNM

        spec_objs += $(SRC_EMBEDDED_DIR)/common/wnm.o
endif

ifeq ($(CONFIG_INTERWORKING),y)
        EXTRA_CFLAGS += -DCONFIG_DOT11U_INTERWORKING
        spec_objs += $(SRC_EMBEDDED_DIR)/common/gas.o
endif
endif

ifeq ($(CONFIG_DSCP_PRI_SUPPORT),y)
	EXTRA_CFLAGS += -DDSCP_PRI_SUPPORT
endif

# MBO
ifeq ($(CONFIG_MBO_SUPPORT),y)
    EXTRA_CFLAGS += -DMBO_SUPPORT -DWAPP_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/mbo.o
    spec_objs +=  $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif

ifeq ($(CONFIG_MAP_SUPPORT),y)
    EXTRA_CFLAGS += -DA4_CONN -DROUTING_TAB_SUPPORT -DWAPP_SUPPORT -DCONFIG_MAP_SUPPORT
    EXTRA_CFLAGS += -DWPS_UNCONFIG_FEATURE_SUPPORT
    EXTRA_CFLAGS += -DMAP_BL_SUPPORT
ifeq ($(CONFIG_MAP_R2_VER_SUPPORT),y)
    EXTRA_CFLAGS += -DMAP_R2 -DMAP_TS_TRAFFIC_SUPPORT
endif

ifeq ($(CONFIG_MAP_R2_6E_SUPPORT), y)
    EXTRA_CFLAGS += -DMAP_6E_SUPPORT
endif

ifeq ($(CONFIG_MAP_R3_6E_SUPPORT), y)
    EXTRA_CFLAGS += -DMAP_R3_6E_SUPPORT
endif

ifeq ($(CONFIG_MAP_R3_VER_SUPPORT),y)
    EXTRA_CFLAGS += -DCHANNEL_SWITCH_MONITOR_CONFIG
    EXTRA_CFLAGS += -DDPP_SUPPORT
    EXTRA_CFLAGS += -DDPP_R2_SUPPORT
    EXTRA_CFLAGS += -DMAP_R3_WF6
    EXTRA_CFLAGS += -DMAP_R3
endif
    spec_objs += $(SRC_EMBEDDED_DIR)/common/map.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/a4_conn.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/routing_tab.o
    spec_objs +=  $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif

ifeq ($(CONFIG_MAP_R4_VER_SUPPORT),y)
    EXTRA_CFLAGS += -DMAP_R4
endif

ifeq ($(CONFIG_QOS_R1_SUPPORT), y)
ifneq ($(CONFIG_SOC_MT7621), y)
    EXTRA_CFLAGS += -DQOS_R1
    EXTRA_CFLAGS += -DQOS_R2
    EXTRA_CFLAGS += -DMSCS_PROPRIETARY
    EXTRA_CFLAGS += -DDABS_QOS
    spec_objs += $(SRC_EMBEDDED_DIR)/common/qos.o
endif
endif

ifeq ($(CONFIG_BS_SUPPORT),y)
    EXTRA_CFLAGS += -DWAPP_SUPPORT
    spec_objs +=  $(SRC_EMBEDDED_DIR)/wapp/wapp.o
endif



#ifeq ($(CONFIG_WAPP),y)
#    EXTRA_CFLAGS += -DWAPP_SUPPORT
#    spec_objs +=  $(SRC_EMBEDDED_DIR)/wapp/wapp.o
#endif

ifeq ($(CONFIG_TRACE_TCP_PKT),y)
       EXTRA_CFLAGS += -DTRACELOG_TCP_PKT
endif

ifeq ($(CONFIG_BACKGROUND_SCAN_SUPPORT),y)
    EXTRA_CFLAGS += -DBACKGROUND_SCAN_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/bgnd_scan.o
endif

ifeq ($(CONFIG_MT_DFS_SUPPORT),y)
	EXTRA_CFLAGS += -DMT_DFS_SUPPORT

	spec_objs += $(SRC_EMBEDDED_DIR)/common/cmm_rdm_mt.o
endif

ifeq ($(CONFIG_WIFI_GPIO_CTRL),y)
	EXTRA_CFLAGS += -DWIFI_GPIO_CTRL
endif
## FT  #need to fix sta mode code
ifeq ($(CONFIG_DOT11R_FT_SUPPORT),y)
EXTRA_CFLAGS += -DDOT11R_FT_SUPPORT
dot11_ft_objs += $(SRC_EMBEDDED_DIR)/common/ft.o\
                    $(SRC_EMBEDDED_DIR)/common/ft_tlv.o\
                    $(SRC_EMBEDDED_DIR)/common/ft_iocl.o\
                    $(SRC_EMBEDDED_DIR)/common/ft_rc.o\
                    $(SRC_EMBEDDED_DIR)/ap/ap_ftkd.o\
		    #$(SRC_EMBEDDED_DIR)/sta/ft_action.o
## Preserve R1KH during Radio ON/OFF
ifeq ($(CONFIG_MBO_SUPPORT),y)
EXTRA_CFLAGS += -DFT_R1KH_KEEP
endif
endif

# RR
ifeq ($(CONFIG_RT2860V2_80211K_RR),y)
dot11k_rr_objs += $(SRC_EMBEDDED_DIR)/common/rrm_tlv.o\
                    $(SRC_EMBEDDED_DIR)/common/rrm_sanity.o\
                    $(SRC_EMBEDDED_DIR)/common/rrm.o
endif

#EAP
ifeq ($(CONFIG_WIFI_EAP_FEATURE),y)
	EXTRA_CFLAGS += -DWIFI_EAP_FEATURE
endif

#SNIFFER
ifeq ($(CONFIG_SNIFFER_SUPPORT),y)
    EXTRA_CFLAGS += -DSNIFFER_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/sniffer_radiotap.o\
                 $(SRC_EMBEDDED_DIR)/common/sniffer_prism.o
endif

ifeq ($(CONFIG_SNIFFER_RADIOTAP_SUPPORT),y)
    EXTRA_CFLAGS += -DSNIFFER_RADIOTAP_SUPPORT

    spec_objs += $(SRC_EMBEDDED_DIR)/common/sniffer_radiotap.o
endif

ifeq ($(CONFIG_WF_RESET_SUPPORT),y)
    EXTRA_CFLAGS += -DWF_RESET_SUPPORT
endif


# VLAN_SUPPORT
ifeq ($(CONFIG_VLAN_SUPPORT),y)
        EXTRA_CFLAGS += -DVLAN_SUPPORT
endif

#EAP-TXRX-Statistics
ifeq ($(CONFIG_TXRX_STAT_SUPPORT),y)
EXTRA_CFLAGS += -DTXRX_STAT_SUPPORT
endif

#EAP-Antenna Control
ifeq ($(CONFIG_ANTENNA_CONTROL_SUPPORT),y)
	EXTRA_CFLAGS += -DANTENNA_CONTROL_SUPPORT
endif

#EAP-Mgmt TxPower Control
ifeq ($(CONFIG_MGMT_TXPWR_CTRL),y)
EXTRA_CFLAGS += -DMGMT_TXPWR_CTRL
endif

#EAP-RA Phy rate support
ifeq ($(CONFIG_RA_PHY_RATE_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_RA_PHY_RATE_SUPPORT
endif

#MBSS DTIM SUPPORT
ifeq ($(CONFIG_MBSS_DTIM_SUPPORT),y)
EXTRA_CFLAGS += -DMBSS_DTIM_SUPPORT
endif

#EAP-AMPDU Config retry & agglimitSupport
ifeq ($(CONFIG_AMPDU_CONF_SUPPORT),y)
EXTRA_CFLAGS += -DAMPDU_CONF_SUPPORT
endif

#EAP-ACK CTS Timeout cck & ofdm Support
ifeq ($(CONFIG_ACK_CTS_TIMEOUT_SUPPORT),y)
EXTRA_CFLAGS += -DACK_CTS_TIMEOUT_SUPPORT
endif

#EAP-Highpriority Rate Specific
ifeq ($(CONFIG_HIGHPRI_RATE_SPECIFIC), y)
EXTRA_CFLAGS += -DHIGHPRI_RATE_SPECIFIC
endif

#HOSTAPD_PMKID_IN_DRIVER_SUPPORT, depends on CFG80211 module flag: CONFIG_NL80211_TESTMODE
#EXTRA_CFLAGS += -DHOSTAPD_PMKID_IN_DRIVER_SUPPORT

#HOSTAPD_MAP_SUPPORT
#EXTRA_CFLAGS += -DHOSTAPD_MAP_SUPPORT

#HOSTAPD_MAPR3_SUPPORT
#EXTRA_CFLAGS += -DHOSTAPD_MAPR3_SUPPORT

#ENTERPRISE_AP
ifeq ($(CONFIG_ENTERPRISE_AP_SUPPORT),y)
	EXTRA_CFLAGS += -DEAP_STATS_SUPPORT

	ifeq ($(CONFIG_RADIUS_MAC_AUTH_SUPPORT),y)
		EXTRA_CFLAGS += -DRADIUS_MAC_AUTH_SUPPORT
	endif

	ifeq ($(CONFIG_DYNAMIC_VLAN_SUPPORT),y)
		EXTRA_CFLAGS += -DDYNAMIC_VLAN_SUPPORT
	endif

        ifeq ($(CONFIG_MBSS_AS_WDS_AP_SUPPORT),y)
                 EXTRA_CFLAGS += -DMBSS_AS_WDS_AP_SUPPORT
                 EXTRA_CFLAGS += -DCLIENT_WDS
                 ap_objs += $(SRC_EMBEDDED_DIR)/common/client_wds.o
        endif

        ifeq ($(CONFIG_WDS_STA_SUPPORT),y)
		EXTRA_CFLAGS += -DAPCLI_AS_WDS_STA_SUPPORT
	endif
endif

#EAP/IAP FEATURE IW CMD WIFI_IAP_STA_DUMP_FEATURE
ifeq ($(CONFIG_WIFI_IAP_STA_DUMP_FEATURE_SUPPORT),y)
    EXTRA_CFLAGS += -DWIFI_IAP_STA_DUMP_FEATURE -DTXRX_STAT_SUPPORT -DEAP_STATS_SUPPORT -DWIFI_IAP_BCN_STAT_FEATURE
endif

#EAP/IAP FEATURE IW CMD WIFI_IAP_POWER_SAVE_FEATURE
ifeq ($(CONFIG_WIFI_IAP_POWER_SAVE_FEATURE_SUPPORT),y)
    EXTRA_CFLAGS += -DWIFI_IAP_POWER_SAVE_FEATURE -DAPCLI_SUPPORT -DLEGACY_PSMODE -DFAST_PSMODE -DUAPSD_SUPPORT -DWIFI_TWT_SUPPORT
endif


#EAP/IAP FEATURE IW CMD DWIFI_IAP_IW_SCAN_FEATURE
ifeq ($(CONFIG_WIFI_IAP_IW_SCAN_FEATURE_SUPPORT),y)
    EXTRA_CFLAGS += -DWIFI_IAP_IW_SCAN_FEATURE
endif

#EAP/IAP FEATURE IW CMD WIFI_IAP_IW_SET_CHANNEL_FEATURE
ifeq ($(CONFIG_WIFI_IAP_IW_SET_CHANNEL_FEATURE_SUPPORT),y)
    EXTRA_CFLAGS += -DWIFI_IAP_IW_SET_CHANNEL_FEATURE
endif

#CFG
ifeq ($(CONFIG_CFG80211_SUPPORT),y)
    EXTRA_CFLAGS += -DRT_CFG80211_SUPPORT -DEXT_BUILD_CHANNEL_LIST

    ifeq ($(CONFIG_APCLI_STA_SUPPORT),y)
         EXTRA_CFLAGS += -DAPCLI_CFG80211_SUPPORT
       EXTRA_CFLAGS += -DSUPP_SAE_SUPPORT  -DSUPP_OWE_SUPPORT
       EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN -DDOT11_SAE_PWD_ID_SUPPORT

       spec_objs += $(SRC_EMBEDDED_DIR)/security/sae.o
       spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_bignum.o
       spec_objs += $(SRC_EMBEDDED_DIR)/security/crypt_biginteger.o
       spec_objs += $(SRC_EMBEDDED_DIR)/security/bn_lib.o
       spec_objs += $(SRC_EMBEDDED_DIR)/security/ecc.o
         spec_objs += $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_apcli.o
    endif
    ifeq ($(CONFIG_HOSTAPD_WPA3_SUPPORT),y)
        EXTRA_CFLAGS += -DHOSTAPD_WPA3_SUPPORT -DHOSTAPD_OWE_SUPPORT -DHOSTAPD_SAE_SUPPORT
        EXTRA_CFLAGS += -DHOSTAPD_SUITEB_SUPPORT -DDOT11W_PMF_SUPPORT
	EXTRA_CFLAGS += -DHOSTAPD_PMKID_IN_DRIVER_SUPPORT


        spec_objs += $(SRC_EMBEDDED_DIR)/security/pmf.o
    endif

    ifeq ($(CONFIG_CUSTOMISED_HOSTAPD_SUPPORT),y)
	EXTRA_CFLAGS += -DDISABLE_HOSTAPD_BEACON -DDISABLE_HOSTAPD_PROBE_RESP -DHOSTAPD_AUTO_CH_SUPPORT -DSCAN_RADAR_COEX_SUPPORT -DFRAG_ATTACK_SUPPORT
	EXTRA_CFLAGS += -DIPI_SCAN_WITH_PD_OFF
	EXTRA_CFLAGS += -DNF_SUPPORT_V2
    endif

    ifeq ($(CONFIG_HOSTAPD_WPA3R3_SUPPORT),y)
	EXTRA_CFLAGS += -DHOSTAPD_WPA3R3_SUPPORT
    endif

    ifeq ($(CONFIG_DOT11R_FT_SUPPORT),y)
	EXTRA_CFLAGS += -DHOSTAPD_11R_SUPPORT
    endif

    ifeq ($(CONFIG_PASSPOINT_R2),y)
		EXTRA_CFLAGS += -DHOSTAPD_HS_R2_SUPPORT -DHOSTAPD_HS_R3_SUPPORT
    endif

    ifeq ($(CONFIG_IWCOMMAND_CFG80211_SUPPORT),y)
		EXTRA_CFLAGS += -DIWCOMMAND_CFG80211_SUPPORT
    endif

    ifeq ($(CONFIG_VLAN_GTK_SUPPORT),y)
		EXTRA_CFLAGS += -DCONFIG_VLAN_GTK_SUPPORT
    endif
    spec_objs += $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_scan.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_rx.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_tx.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_inf.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_ap.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211drv.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211_util.o\
                $(SRC_EMBEDDED_DIR)/../os/linux/cfg80211/cfg80211.o
endif
ifeq ($(CONFIG_DBDC_ONE_BAND_SUPPORT),y)
	EXTRA_CFLAGS += -DDBDC_ONE_BAND_SUPPORT
endif

#Automation
ifeq ($(CONFIG_AUTOMATION),y)
EXTRA_CFLAGS += -DAUTOMATION
chip_objs += $(SRC_EMBEDDED_DIR)/common/automation.o
endif

ifeq ($(CONFIG_RALINK_TIMER_WDG),y)
    EXTRA_CFLAGS += -DSDK_TIMER_WDG
endif

ifeq ($(CONFIG_OFFCHANNEL_SCAN_FEATURE),y)
    EXTRA_CFLAGS += -DOFFCHANNEL_SCAN_FEATURE
endif
ifeq ($(CONFIG_MTK_WD_KICKER),y)
    ifeq ($(CONFIG_MTK_WD_KICKER_WCN),y)
        EXTRA_CFLAGS += -DSDK_TIMER_WDG
    endif
endif

ifeq ($(CONFIG_MTK_VENDOR_FEATURE10_SUPPORT),y)
    EXTRA_CFLAGS += -DFOLLOW_HIDDEN_SSID_FEATURE
    EXTRA_CFLAGS += -DCONVERTER_MODE_SWITCH_SUPPORT
    EXTRA_CFLAGS += -DMCAST_VENDOR10_CUSTOM_FEATURE
    EXTRA_CFLAGS += -DSPECIAL_11B_OBW_FEATURE
    ifeq ($(CONFIG_MT_DFS_SUPPORT),y)
        EXTRA_CFLAGS += -DDFS_VENDOR10_CUSTOM_FEATURE
    endif
    ifeq ($(CONFIG_MCAST_RATE_SPECIFIC),y)
        EXTRA_CFLAGS += -DMCAST_BCAST_RATE_SET_SUPPORT
    endif
    ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
        EXTRA_CFLAGS += -DIGMP_TVM_SUPPORT
    endif
    ifeq ($(CONFIG_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DVENDOR10_CUSTOM_RSSI_FEATURE
	EXTRA_CFLAGS += -DBW_VENDOR10_CUSTOM_FEATURE
    endif
endif

#
# Common Feature related files
#
func_objs :=

ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
    EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT

    func_objs += $(SRC_EMBEDDED_DIR)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_SINGLE_SKU),y)
    EXTRA_CFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(CONFIG_RT2860V2_AP_VIDEO_TURBINE),y)
    EXTRA_CFLAGS += -DVIDEO_TURBINE_SUPPORT

    func_objs += $(SRC_EMBEDDED_DIR)/common/cmm_video.o
endif

ifeq ($(CONFIG_SMART_CARRIER_SENSE_SUPPORT),y)
    EXTRA_CFLAGS += -DSMART_CARRIER_SENSE_SUPPORT

        spec_objs += $(SRC_EMBEDDED_DIR)/common/scs.o
endif

ifeq ($(CONFIG_SCS_FW_OFFLOAD),y)
    EXTRA_CFLAGS += -DSCS_FW_OFFLOAD
endif

ifeq ($(CONFIG_DYNAMIC_WMM_SUPPORT),y)
    EXTRA_CFLAGS += -DDYNAMIC_WMM_SUPPORT

        spec_objs += $(SRC_EMBEDDED_DIR)/common/dynwmm.o
endif

ifeq ($(CONFIG_WIFI_CSI_CN_INFO_SUPPORT),y)
    EXTRA_CFLAGS += -DWIFI_CSI_CN_INFO_SUPPORT
endif

# CTXD_MEM_CPY
ifeq ($(CONFIG_CTXD_MEM_CPY_SUPPORT),y)
    EXTRA_CFLAGS += -DCTXD_MEM_CPY -DCONFIG_TX_DELAY
endif

# CTXD_SCATTER_AND_GATHER
ifeq ($(CONFIG_CTXD_SCATTER_AND_GATHER_SUPPORT),y)
    EXTRA_CFLAGS += -DCTXD_SCATTER_AND_GATHER -DCONFIG_TX_DELAY
endif

########################################################
# AP feature related files
########################################################
ap_objs := $(SRC_EMBEDDED_DIR)/ap/ap.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_mlme.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_sanity.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_wpa.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_sec.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_data.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_autoChSel.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_qload.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_cfg.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_vow.o\
            $(SRC_DIR)/os/linux/ap_ioctl.o\
            $(SRC_EMBEDDED_DIR)/fsm/ap_mgmt_cntl.o\
            $(SRC_EMBEDDED_DIR)/fsm/ap_mgmt_sync.o\
            $(SRC_EMBEDDED_DIR)/fsm/ap_mgmt_auth.o\
            $(SRC_EMBEDDED_DIR)/fsm/ap_mgmt_assoc.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_bss_mnger.o

ifeq ($(CONFIG_MBSS_SUPPORT),y)
    EXTRA_CFLAGS += -DMBSS_SUPPORT

    ifeq ($(CONFIG_NEW_MBSSID_MODE),y)
        EXTRA_CFLAGS += -DNEW_MBSSID_MODE
        ifeq ($(CONFIG_ENHANCE_NEW_MBSSID_MODE),y)
            EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
        endif
    endif

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_mbss.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_mbss_inf.o
endif


ifeq ($(CONFIG_WDS_SUPPORT),y)
    EXTRA_CFLAGS += -DWDS_SUPPORT

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_wds.o\
            $(SRC_EMBEDDED_DIR)/ap/ap_wds_inf.o\
            $(SRC_EMBEDDED_DIR)/common/client_wds.o
endif

ifeq ($(CONFIG_APCLI_SUPPORT),y)
# TO BE REMOVED (still needed for apcli/sta struct merge) =====================================================================
    EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT -DMULTI_APCLI_SUPPORT -DAPCLI_AUTO_CONNECT_SUPPORT  -DAPCLI_AUTO_BW_TMP
    #EXTRA_CFLAGS += -DAPCLI_CERT_SUPPORT #-DAPCLI_CONNECTION_TRIAL
# TO BE REMOVED END ============================================================================================================
    ifeq ($(CONFIG_MBSS_AS_WDS_AP_SUPPORT),y)
    	EXTRA_CFLAGS += -DCONFIG_APSTA_MIXED_SUPPORT -DMAT_SUPPORT -DCONFIG_APCLI_INF_NAME
    else
    EXTRA_CFLAGS += -DCONFIG_APSTA_MIXED_SUPPORT -DETH_CONVERT_SUPPORT -DMAT_SUPPORT -DCONFIG_APCLI_INF_NAME
    endif
    EXTRA_CFLAGS += -DDHCP_UC_SUPPORT
    EXTRA_CFLAGS += -DBT_APCLI_SUPPORT
    EXTRA_CFLAGS += -DCONFIG_STA_SUPPORT #-DFAST_EAPOL_WAR
    ap_objs += \
		$(SRC_EMBEDDED_DIR)/sta/sanity.o\
		$(SRC_EMBEDDED_DIR)/sta/sta_data.o\
	    $(SRC_EMBEDDED_DIR)/sta/sta_sec.o\
		$(SRC_EMBEDDED_DIR)/sta/wpa.o\
		$(SRC_EMBEDDED_DIR)/sta/sta_cfg.o\
		$(SRC_EMBEDDED_DIR)/sta/sta.o\
		$(SRC_DIR)/os/linux/sta_ioctl.o\
		$(SRC_EMBEDDED_DIR)/fsm/sta_mgmt_cntl.o\
		$(SRC_EMBEDDED_DIR)/fsm/sta_mgmt_auth.o\
		$(SRC_EMBEDDED_DIR)/fsm/sta_mgmt_sync.o\
        $(SRC_EMBEDDED_DIR)/common/cmm_mat.o\
        $(SRC_EMBEDDED_DIR)/common/cmm_mat_iparp.o\
        $(SRC_EMBEDDED_DIR)/common/cmm_mat_pppoe.o\
        $(SRC_EMBEDDED_DIR)/common/cmm_mat_ipv6.o\
	$(SRC_EMBEDDED_DIR)/fsm/sta_mgmt_assoc.o

    ifeq ($(CONFIG_MAC_REPEATER_SUPPORT),y)
        EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT

        ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_repeater.o
    endif
endif


ifeq ($(CONFIG_IDS_SUPPORT),y)
    EXTRA_CFLAGS += -DIDS_SUPPORT

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_ids.o
endif

ifeq ($(CONFIG_NINTENDO_AP),y)
    EXTRA_CFLAGS += -DNINTENDO_AP

    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_nintendo.o
endif

#ifeq ($(CONFIG_COC_SUPPORT),y)
#   EXTRA_CFLAGS += -DGREENAP_SUPPORT -DCOC_SUPPORT
#
#    ap_objs += $(SRC_DIR)/../hw_ctrl/greenap.o
#endif

ifeq ($(CONFIG_GREENAP_SUPPORT),y)
   EXTRA_CFLAGS += -DGREENAP_SUPPORT

    ap_objs += $(SRC_DIR)/hw_ctrl/greenap.o
endif

ifeq ($(CONFIG_PCIE_ASPM_DYM_CTRL_SUPPORT),y)
   EXTRA_CFLAGS += -DPCIE_ASPM_DYM_CTRL_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_ATE -DCONFIG_QA -DCONFIG_RT2880_ATE_CMD_NEW -DCONFIG_HW_HAL_OFFLOAD  #-DCSD_VERIFICATION -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(CONFIG_WLAN_SERVICE),y)
    EXTRA_CFLAGS += -DCONFIG_WLAN_SERVICE
endif

ifeq ($(CONFIG_ICAP_SUPPORT),y)
		EXTRA_CFLAGS += -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(CONFIG_SPECTRUM_SUPPORT),y)
		EXTRA_CFLAGS += -DWIFI_SPECTRUM_SUPPORT
endif

ifeq ($(CONFIG_PHY_ICS_SUPPORT),y)
		EXTRA_CFLAGS += -DPHY_ICS_SUPPORT
endif

ifeq ($(CONFIG_RTMP_FLASH_SUPPORT),y)
    EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(CONFIG_PRE_CAL_TRX_SET1_SUPPORT),y)
    EXTRA_CFLAGS += -DPRE_CAL_TRX_SET1_SUPPORT
endif

ifeq ($(CONFIG_RLM_CAL_CACHE_SUPPORT),y)
    EXTRA_CFLAGS += -DRLM_CAL_CACHE_SUPPORT
endif

ifeq ($(CONFIG_PRE_CAL_TRX_SET2_SUPPORT),y)
    EXTRA_CFLAGS += -DPRE_CAL_TRX_SET2_SUPPORT
endif

ifeq ($(CONFIG_CAL_BIN_FILE_SUPPORT),y)
    EXTRA_CFLAGS += -DCAL_BIN_FILE_SUPPORT
endif

ifeq ($(CONFIG_LINK_TEST_SUPPORT),y)
    EXTRA_CFLAGS += -DLINK_TEST_SUPPORT
endif

ifeq ($(CONFIG_RTMP_TEMPERATURE_TX_ALC),y)
    EXTRA_CFLAGS += -DRTMP_TEMPERATURE_TX_ALC
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DTXBF_SUPPORT -DVHT_TXBF_SUPPORT -DHE_TXBF_SUPPORT
    ap_objs += $(SRC_DIR)/txbf/cmm_txbf_mt.o\
                    $(SRC_DIR)/txbf/cmm_txbf_cal_mt.o\
                    $(SRC_DIR)/txbf/txbf_wrapper_embedded.o
endif

ifeq ($(CONFIG_MUMIMO_SUPPORT),y)
    EXTRA_CFLAGS += -DCFG_SUPPORT_MU_MIMO
    ap_objs += $(SRC_EMBEDDED_DIR)/common/cmm_mumimo.o
    ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_mumimo.o
ifneq ($(CONFIG_MUMIMO_PLATFORM_PERFORMANCE_LEVEL),)
    EXTRA_CFLAGS += -DCFG_MUMIMO_PLATFORM_PERFORMANCE_LEVEL=$(CONFIG_MUMIMO_PLATFORM_PERFORMANCE_LEVEL)
endif
endif

ifeq ($(CONFIG_MU_RA_SUPPORT),y)
   EXTRA_CFLAGS += -DCFG_SUPPORT_MU_MIMO_RA
   ap_objs += $(SRC_EMBEDDED_DIR)/ap/ap_mura.o
endif

ifeq ($(CONFIG_CFG_SUPPORT_FALCON_TXCMD_DBG),y)
   EXTRA_CFLAGS += -DCFG_SUPPORT_FALCON_TXCMD_DBG
    ap_objs += $(SRC_EMBEDDED_DIR)/common/he_cfg.o
endif

ifeq ($(CONFIG_CFG_SUPPORT_FALCON_MURU),y)
EXTRA_CFLAGS += -DCFG_SUPPORT_FALCON_MURU
spec_objs += $(SRC_EMBEDDED_DIR)/ap/ap_muru.o
endif

ifeq ($(CONFIG_CON_WPS_SUPPORT), y)
    EXTRA_CFLAGS += -DCON_WPS -DCON_WPS_AP_SAME_UUID
ifeq ($(CONFIG_FIRST_IF_MT7626),y)
    EXTRA_CFLAGS += -DMULTI_INF_SUPPORT
    ap_objs += $(SRC_EMBEDDED_DIR)/../os/linux/multi_main_dev.o
endif
endif

ifeq ($(CONFIG_RT2860V2_AP_CARRIER),y)
    EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT

    ap_objs += $(SRC_EMBEDDED_DIR)/common/cmm_cs.o
endif

ifeq ($(CONFIG_MCAST_RATE_SPECIFIC),y)
    EXTRA_CFLAGS += -DMCAST_RATE_SPECIFIC
endif

ifeq ($(CONFIG_AIRPLAY_SUPPORT),y)
    EXTRA_CFLAGS += -DAIRPLAY_SUPPORT
endif

ifeq ($(CONFIG_BAND_STEERING),y)
    EXTRA_CFLAGS += -DBAND_STEERING
    ap_objs +=  $(SRC_EMBEDDED_DIR)/ap/ap_band_steering.o
endif

ifeq ($(CONFIG_AIR_MONITOR),y)
    EXTRA_CFLAGS += -DAIR_MONITOR
endif

ifeq ($(CONFIG_EASY_SETUP_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/easy_setup/ez_cmm.o
endif

ifeq ($(CONFIG_MWDS),y)
    ap_objs += $(SRC_EMBEDDED_DIR)/common/mwds.o
    ap_objs += $(SRC_EMBEDDED_DIR)/common/a4_conn.o
    ap_objs += $(SRC_EMBEDDED_DIR)/common/routing_tab.o
endif

ifeq ($(CONFIG_EVENT_NOTIFIER_SUPPORT),y)
    ap_objs += $(SRC_EMBEDDED_DIR)/common/event_notifier.o
endif

ifeq ($(CONFIG_VOW_SUPPORT),y)
    EXTRA_CFLAGS += -DVOW_SUPPORT
endif

ifeq ($(CONFIG_RED_SUPPORT),y)
    EXTRA_CFLAGS += -DRED_SUPPORT
endif

ifeq ($(CONFIG_FQ_SCH_SUPPORT),y)
    EXTRA_CFLAGS += -DFQ_SCH_SUPPORT
endif

ifeq ($(CONFIG_CUSTOMER_VENDOR_IE_SUPPORT),y)
    EXTRA_CFLAGS += -DCUSTOMER_VENDOR_IE_SUPPORT
endif

#EXTRA_CFLAGS += -DACS_CTCC_SUPPORT
#EXTRA_CFLAGS += -DCHANNEL_SWITCH_MONITOR_CONFIG
#CTCC FEATURE ACL_BLK_CONUT
#EXTRA_CFLAGS += -DACL_BLK_COUNT_SUPPORT
#CTCC FEATURE RATE_PRIOR_SUPPORT
#EXTRA_CFLAGS += -DRATE_PRIOR_SUPPORT

ifeq ($(CONFIG_WIFI_DIAG), y)
	EXTRA_CFLAGS += -DWIFI_DIAG
	ap_objs += $(SRC_DIR)/os/linux/diag.o
endif

ifeq ($(CONFIG_CONN_FAIL_EVENT), y)
	EXTRA_CFLAGS += -DCONN_FAIL_EVENT
endif

########################################################
# Linux system related files
########################################################
os_objs := $(SRC_DIR)/os/linux/rt_proc.o\
            $(SRC_DIR)/os/linux/rt_linux.o\
            $(SRC_DIR)/os/linux/rt_profile.o\
            $(SRC_DIR)/os/linux/rt_main_dev.o

ifeq ($(CONFIG_WLAN_HOOK),y)
	EXTRA_CFLAGS +=-DRTMP_WLAN_HOOK_SUPPORT
	os_objs += $(SRC_DIR)/os/linux/rt_txrx_hook.o
endif

ifeq ($(CONFIG_MULTI_INF_SUPPORT),y)
    EXTRA_CFLAGS += -DMULTI_INF_SUPPORT
    os_objs += $(SRC_DIR)/os/linux/multi_main_dev.o
endif

ifeq ($(CONFIG_WIFI_WORK_QUEUE_BH),y)
    EXTRA_CFLAGS += -DWORKQUEUE_BH
endif

ifeq ($(CONFIG_KTHREAD),y)
    EXTRA_CFLAGS += -DKTHREAD_SUPPORT
endif

ifeq ($(CONFIG_LINUX_NET_TXQ_SUPPORT),y)
EXTRA_CFLAGS += -DLINUX_NET_TXQ_SUPPORT
endif

ifeq ($(CONFIG_CONNINFRA_APSOC),y)
EXTRA_CFLAGS += -DCONFIG_CONNINFRA_SUPPORT
endif

########################################################
# chip related files
########################################################
ifeq ($(CONFIG_SECOND_IF_MT7637E),y)
EXTRA_CFLAGS += -DMT7637 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT -DDMA_SCH_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTAR_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
# -DCUT_THROUGH
#EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
#EXTRA_CFLAGS += -DNEED_ROM_PATCH
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_smac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7637.o\
		$(SRC_DIR)/chips/mt7637_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_mac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif

ifeq ($(CONFIG_DBDC_MODE),y)
EXTRA_CFLAGS += -DDBDC_MODE
endif

ifeq ($(CONFIG_CHIP_MT7615E),y)
EXTRA_CFLAGS += -DMT7615 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DTXPWRMANUAL
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DTX_POWER_CONTROL_SUPPORT
EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DRATE_ADAPTION
EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TX_DELAY
EXTRA_CFLAGS += -DRELEASE_EXCLUDE
EXTRA_CFLAGS += -DPRE_CAL_TRX_SET1_SUPPORT
EXTRA_CFLAGS += -DRLM_CAL_CACHE_SUPPORT
EXTRA_CFLAGS += -DPRE_CAL_TRX_SET2_SUPPORT
EXTRA_CFLAGS += -DOUI_CHECK_SUPPORT
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif
EXTRA_CFLAGS += -Wframe-larger-than=4096
ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
ifeq ($(CONFIG_SMP),y)
EXTRA_CFLAGS += -DSW_RPS_RX_OFFLOAD_SUPPORT
endif
endif

ifeq ($(CONFIG_VLAN_SUPPORT),y)
	EXTRA_CFLAGS += -DVLAN_SUPPORT
endif
#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
#	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

chip_objs += $(SRC_DIR)/chips/mt7615.o\
		$(SRC_DIR)/chips/mt7615_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_cmm_mac.o\
		$(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_EMBEDDED_DIR)/common/token.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif

ifeq ($(CONFIG_CHIP_MT7663E),y)
EXTRA_CFLAGS += -DMT7663 -DMT_BBP -DMT_RF -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
#EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
#EXTRA_CFLAGS += -DOUI_CHECK_SUPPORT
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
WFLAGS += -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DCONFIG_TX_DELAY
#7663 E1 Workaround
EXTRA_CFLAGS += -DTESTMODE_TX_CONTROL
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER
ifeq ($(CONFIG_AUTOMATION),y)
EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT
EXTRA_CFLAGS += -DCSO_TEST_SUPPORT
endif
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif
CONFIG_TCP_RACK_SUPPORT=y

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_VLAN_SUPPORT),y)
	EXTRA_CFLAGS += -DVLAN_SUPPORT
endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
	chip_objs += $(SRC_EMBEDDED_DIR)/common/cmm_tcprack.o
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

ifeq ($(CONFIG_MT7663_RFB_MANUAL_CAL),y)
    EXTRA_CFLAGS += -DMT7663_RFB_MANUAL_CAL
    #EXTRA_CFLAGS += -DMT7663_RFB_19
    chip_objs += $(SRC_DIR)/chips/mt7663_manual_cal.o
endif

ifeq ($(CONFIG_FTM_SUPPORT),y)
    EXTRA_CFLAGS += -DTMR_GEN2
endif

chip_objs += $(SRC_DIR)/chips/mt7663.o\
		$(SRC_DIR)/chips/mt7663_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_cmm_mac.o\
		$(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_EMBEDDED_DIR)/common/token.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif

ifeq ($(CONFIG_CHIP_MT7663U),y)
EXTRA_CFLAGS += -DMT7663 -DMT_BBP -DMT_RF -DRTMP_USB_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_USB -DRTMP_TIMER_TASK_SUPPORT
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
#EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DDBDC_MODE
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
WFLAGS += -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

chip_objs += $(SRC_DIR)/chips/mt7663.o\
		$(SRC_DIR)/chips/mt7663_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_cmm_mac.o\
		$(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_usb.o\
                $(SRC_EMBEDDED_DIR)/common/hif_usb.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_usb_io.o\
                $(SRC_EMBEDDED_DIR)/common/mtusb_io.o\
                $(SRC_EMBEDDED_DIR)/common/rtusb_dev_id.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_usb.o\
                $(SRC_DIR)/os/linux/rt_usb_util.o\
                $(SRC_DIR)/os/linux/usb_main_dev.o
endif


ifeq ($(CONFIG_FIRST_IF_MT7626),y)
ifneq ($(CONFIG_SUPPORT_OPENWRT),y)
# Turn on for MSP auto test due to use FPGA S4.
CONFIG_7626_USE_PCI=y
endif
EXTRA_CFLAGS += -DMT7626 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
#PCI is for debug purpose
ifeq ($(CONFIG_7626_USE_PCI),y)
EXTRA_CFLAGS += -DRTMP_PCI_SUPPORT
else
#RBUS is for mt7626
EXTRA_CFLAGS += -DRTMP_RBUS_SUPPORT
CONFIG_PROPRIETARY_DRIVER=y
EXTRA_CFLAGS += -DWIFI_RAM_EMI_SUPPORT
endif

EXTRA_CFLAGS += -DRELEASE_EXCLUDE

# Rx detect for multi client
EXTRA_CFLAGS += -DRX_COUNT_DETECT

#merge A-Die Efuse to Host
EXTRA_CFLAGS += -DEEPROM_RETRIEVE_SUPPORT

EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMT7626_REDUCE_TX_OVERHEAD
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT

EXTRA_CFLAGS += -DTESTMODE_TX_CONTROL
ifeq ($(CONFIG_AUTOMATION),y)
EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT
EXTRA_CFLAGS += -DCSO_TEST_SUPPORT
endif
#WFLAGS += -DMSP_UTILITY

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

CONFIG_PRE_CAL_SUPPORT=n
ifeq ($(CONFIG_PRE_CAL_SUPPORT),y)
EXTRA_CFLAGS += -DPRE_CAL_MT7626_SUPPORT
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_VLAN_SUPPORT),y)
	EXTRA_CFLAGS += -DVLAN_SUPPORT
endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
    ifeq ($(CONFIG_WSC_INCLUDED),y)
        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

#workaround for openwrt, depden on system flags
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif
chip_objs += $(SRC_DIR)/chips/mt7626.o\
		$(SRC_DIR)/chips/mt7626_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_cmm_mac.o\
		$(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_EMBEDDED_DIR)/common/token.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o

ifeq ($(CONFIG_7626_USE_PCI),y)
chip_objs += $(SRC_DIR)/os/linux/pci_main_dev.o
endif

endif



ifeq ($(CONFIG_FIRST_IF_MT7622),y)
EXTRA_CFLAGS += -DMT7622 -DMT_BBP -DMT_RF -DRTMP_RBUS_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
EXTRA_CFLAGS += -DTXPWRMANUAL
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DETSI_RX_BLOCKER_SUPPORT
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
#EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DCONFIG_CSO_SUPPORT
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
#EXTRA_CFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DRATE_ADAPTION
EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DCONFIG_TX_DELAY
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DPRE_CAL_MT7622_SUPPORT
CONFIG_PROPRIETARY_DRIVER=y

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
EXTRA_CFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

#EXTRA_CFLAGS += -Werror
ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif
ifeq ($(CONFIG_VHT_TXBF_2G_EPIGRAM_IE_SUPPORT),y)
        EXTRA_CFLAGS += -DVHT_TXBF_2G_EPIGRAM_IE
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
#        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

chip_objs += $(SRC_DIR)/chips/mt7622.o\
		$(SRC_DIR)/chips/mt7622_dbg.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_dmac.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
                $(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
				$(SRC_DIR)/mac/mt_cmm_mac.o\
                $(SRC_DIR)/mac/mt_dmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
                $(SRC_DIR)/mcu/mt_cmd.o\
                $(SRC_DIR)/mcu/fwdl_mt.o\
                $(SRC_DIR)/mcu/fw_cmd.o\
                $(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/token.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o
endif

ifeq ($(CONFIG_CHIP_AXE),y)
EXTRA_CFLAGS += -DAXE -DMT_BBP -DMT_RF -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
#EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
#EXTRA_CFLAGS += -DNEED_ROM_PATCH
#EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
#EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DAXE_ASIC_WORKAROUND
#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
	EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
	EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
	EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
	EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
	EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_dmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

chip_objs += $(SRC_DIR)/chips/axe.o\
		$(SRC_DIR)/chips/axe_dbg.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fmac.o\
		$(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
		$(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/mac/mt_fmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
		$(SRC_DIR)/mcu/mt_cmd.o\
		$(SRC_DIR)/mcu/fwdl_mt.o\
		$(SRC_DIR)/mcu/fw_cmd.o\
		$(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
		$(SRC_EMBEDDED_DIR)/common/token.o\
		$(SRC_DIR)/protocol/tmr.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif

ifeq ($(CONFIG_CHIP_MT7915),y)
EXTRA_CFLAGS += -DMT7915 -DMT_BBP -DMT_RF -DRTMP_PCI_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC -DMT_FMAC
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
#EXTRA_CFLAGS += -DCONFIG_FPGA_MODE
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DFW_LOG_DUMP
EXTRA_CFLAGS += -DDOT11V_MBSSID_SUPPORT
EXTRA_CFLAGS += -DA_BAND_SUPPORT
#EXTRA_CFLAGS += -DMT7915_E1_WORKAROUND
EXTRA_CFLAGS += -DRELEASE_EXCLUDE
ccflags-y += -Werror
#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DTX_POWER_CONTROL_SUPPORT
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

CONFIG_PROPRIETARY_DRIVER=y

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER -DPROPRIETARY_DRIVER_SUPPORT
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
EXTRA_CFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
        EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
        EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
        EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_fmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DHE_TXBF_SUPPORT
endif

# Rx detect for multi client
EXTRA_CFLAGS += -DRX_COUNT_DETECT
EXTRA_CFLAGS += -DKERNEL_RPS_ADJUST
ifeq ($(CONFIG_SOC_MT7621),y)
EXTRA_CFLAGS += -DRX_RPS_SUPPORT
endif
EXTRA_CFLAGS += -DPKTLOSS_CHK
EXTRA_CFLAGS += -DVERIFICATION_MODE
EXTRA_CFLAGS += -DWIFI_MODULE_DVT
EXTRA_CFLAGS += -DDABS_QOS

CONFIG_PRE_CAL_SUPPORT=y
ifeq ($(CONFIG_PRE_CAL_SUPPORT),y)
EXTRA_CFLAGS += -DPRE_CAL_MT7915_SUPPORT
endif

chip_objs += $(SRC_DIR)/chips/mt7915.o\
                $(SRC_DIR)/chips/mt7915_dbg.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fmac.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
                $(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
                $(SRC_DIR)/mac/mt_cmm_mac.o\
                $(SRC_DIR)/mac/mt_fmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
                $(SRC_DIR)/mcu/mt_cmd.o\
                $(SRC_DIR)/mcu/fwdl_mt.o\
                $(SRC_DIR)/mcu/fw_cmd.o\
                $(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/token.o\
                $(SRC_EMBEDDED_DIR)/common/dbg_ctrl.o\
                $(SRC_EMBEDDED_DIR)/common/verification_mode.o\
                $(SRC_EMBEDDED_DIR)/common/mdvt.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/os/linux/pci_main_dev.o
endif

ifeq ($(CONFIG_CHIP_MT7986),y)
EXTRA_CFLAGS += -DMT7986 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC -DMT_FMAC -DDATA_TXPWR_CTRL
EXTRA_CFLAGS += -DWARP_512_SUPPORT
EXTRA_CFLAGS += -DICAP_FW_SUPPORT
EXTRA_CFLAGS += -DDFS_ADJ_BW_ZERO_WAIT
EXTRA_CFLAGS += -DDELAY_TCP_ACK_V2
EXTRA_CFLAGS += -DPS_STA_FLUSH_SUPPORT
ifeq ($(CONFIG_WIFI_TWT_SUPPORT),y)
ifeq ($(CONFIG_WIFI_SKU_TYPE),AX4200)
EXTRA_CFLAGS += -DDYNAMIC_TWT_NODE_SUPPORT
endif
endif
ifeq ($(CONFIG_WIFI_SKU_TYPE),AX7800)
EXTRA_CFLAGS += -DSKU_AX7800
EXTRA_CFLAGS += -DZWDFS_AX7800
endif
EXTRA_CFLAGS += -DCONFIG_3_WIRE_SUPPORT
EXTRA_CFLAGS += -DMTK_FE_RESET_RECOVER
# Turn on PCI for FPGA stage.
# CONFIG_7986_USE_PCI=n

ifeq ($(CONFIG_WIFI_FW_BIN_LOAD),y)
#let MT7986 fw bin load default only by read boot strap
EXTRA_CFLAGS += -DCONFIG_MT7986_FW_BIN_LOAD
endif

ifeq ($(CONFIG_BACKGROUND_SCAN_SUPPORT),y)
EXTRA_CFLAGS += -DIPI_SCAN_SUPPORT
endif

#EXTRA_CFLAGS += -DSNR_NOISE_SUPPORT

#Add for default debug driver INT done issue
EXTRA_CFLAGS += -DCONFIG_TP_DBG

ifeq ($(CONFIG_7986_USE_PCI),y)
EXTRA_CFLAGS += -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DCONFIG_FPGA_MODE -DMT7986_FPGA
else
EXTRA_CFLAGS += -DRTMP_RBUS_SUPPORT
EXTRA_CFLAGS += -DMULTI_INTR_SUPPORT
EXTRA_CFLAGS += -DWIFI_RAM_EMI_SUPPORT
endif

#HWITS for WTBL MISS INT
#EXTRA_CFLAGS += -DCONFIG_WTBL_MISS_DVT

EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#FPGA efuse TBD, now the in-band cmd EXT_CMD_ID_EFUSE_ACCESS will lead F/W hang on FPGA(D)
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#MT7986 Tx Latency
#EXTRA_CFLAGS += -DEAP_STATS_SUPPORT
#EXTRA_CFLAGS += -DEAP_ENHANCE_STATS_SUPPORT
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DSINGLE_SKU_V2
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
#extra check for WED Rx Scatter
EXTRA_CFLAGS += -DRXD_WED_SCATTER_SUPPORT
#WO mark DW2 bit[8] as scatter
EXTRA_CFLAGS += -DWO_SCATTER_INDICATE
#non scatter cases still len error, lookup & correct again
EXTRA_CFLAGS += -DSINGLE_DMAD_FIX

EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DFW_LOG_DUMP
EXTRA_CFLAGS += -DDOT11V_MBSSID_SUPPORT
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DSWACI_MECHANISM
#EXTRA_CFLAGS += -DMT7915_E1_WORKAROUND
EXTRA_CFLAGS += -DRELEASE_EXCLUDE
#EXTRA_CFLAGS += -DCONFIG_ICS_FRAME_HANDLE
#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DTX_POWER_CONTROL_SUPPORT
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

CONFIG_PROPRIETARY_DRIVER=y

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER -DPROPRIETARY_DRIVER_SUPPORT
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
EXTRA_CFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
        EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
        EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_CFG80211_SUPPORT),y)
    EXTRA_CFLAGS += -Wframe-larger-than=1024
    WFLAGS += -Werror
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
        EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_fmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DHE_TXBF_SUPPORT
endif

# Rx detect for multi client
EXTRA_CFLAGS += -DRX_COUNT_DETECT
ifeq ($(CONFIG_SOC_MT7621),y)
EXTRA_CFLAGS += -DKERNEL_RPS_ADJUST
EXTRA_CFLAGS += -DRX_RPS_SUPPORT
endif
EXTRA_CFLAGS += -DVERIFICATION_MODE
EXTRA_CFLAGS += -DWIFI_MODULE_DVT

CONFIG_PRE_CAL_SUPPORT=y
ifeq ($(CONFIG_PRE_CAL_SUPPORT),y)
EXTRA_CFLAGS += -DPRE_CAL_MT7986_SUPPORT
endif

ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
EXTRA_CFLAGS += -DIGMP_SNOOPING_NON_OFFLOAD -DIGMP_SNOOPING_DENY_LIST
endif

chip_objs += $(SRC_DIR)/chips/mt7986.o\
                $(SRC_DIR)/chips/mt7986_dbg.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fmac.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
                $(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
                $(SRC_DIR)/mac/mt_cmm_mac.o\
                $(SRC_DIR)/mac/mt_fmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
                $(SRC_DIR)/mcu/mt_cmd.o\
                $(SRC_DIR)/mcu/fwdl_mt.o\
                $(SRC_DIR)/mcu/fw_cmd.o\
                $(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/token.o\
                $(SRC_EMBEDDED_DIR)/common/dbg_ctrl.o\
                $(SRC_EMBEDDED_DIR)/common/verification_mode.o\
                $(SRC_EMBEDDED_DIR)/common/mdvt.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/hw_ctrl/cmm_fw_uni_cmd.o\
                $(SRC_DIR)/hw_ctrl/cmm_fw_uni_event.o\

ifeq ($(CONFIG_7986_USE_PCI),y)
chip_objs += $(SRC_DIR)/os/linux/pci_main_dev.o
endif
endif

ifeq ($(CONFIG_CHIP_MT7916),y)
EXTRA_CFLAGS += -DMT7916 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC -DMT_FMAC
EXTRA_CFLAGS += -DICAP_FW_SUPPORT
EXTRA_CFLAGS += -DDELAY_TCP_ACK_V2
ifneq ($(CONFIG_MT7915_MT7916_COEXIST_COMPATIBLE),y)
EXTRA_CFLAGS += -DIXIA_C50_MODE
endif
EXTRA_CFLAGS += -DDFS_MT7916_DEDICATED_ZW
EXTRA_CFLAGS += -DDFS_ADJ_BW_ZERO_WAIT
# Turn on PCI for FPGA stage.
CONFIG_7916_USE_PCI=y
EXTRA_CFLAGS += -DRTMP_PCI_SUPPORT
# EXTRA_CFLAGS += -DCONFIG_FPGA_MODE -DMT7916_FPGA
ifeq ($(CONFIG_WIFI_FW_BIN_LOAD),y)
#let MT7916 fw bin load default only by dts file setting
EXTRA_CFLAGS += -DCONFIG_MT7916_FW_BIN_LOAD
endif
EXTRA_CFLAGS += -DCONFIG_3_WIRE_SUPPORT
#HWITS for WTBL MISS INT
#EXTRA_CFLAGS += -DCONFIG_WTBL_MISS_DVT

EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#FPGA efuse TBD, now the in-band cmd EXT_CMD_ID_EFUSE_ACCESS will lead F/W hang on FPGA(D)
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#MT7916 Tx Latency
#EXTRA_CFLAGS += -DEAP_STATS_SUPPORT
#EXTRA_CFLAGS += -DEAP_ENHANCE_STATS_SUPPORT
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DSINGLE_SKU_V2
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DFW_LOG_DUMP
EXTRA_CFLAGS += -DDOT11V_MBSSID_SUPPORT
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DSWACI_MECHANISM
#EXTRA_CFLAGS += -DMT7915_E1_WORKAROUND
EXTRA_CFLAGS += -DRELEASE_EXCLUDE

#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DTX_POWER_CONTROL_SUPPORT
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

CONFIG_PROPRIETARY_DRIVER=y

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER -DPROPRIETARY_DRIVER_SUPPORT
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
EXTRA_CFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
        EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
        EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
        EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_fmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DHE_TXBF_SUPPORT
endif

# Rx detect for multi client
EXTRA_CFLAGS += -DRX_COUNT_DETECT
ifeq ($(CONFIG_SOC_MT7621),y)
EXTRA_CFLAGS += -DKERNEL_RPS_ADJUST
EXTRA_CFLAGS += -DRX_RPS_SUPPORT
endif
EXTRA_CFLAGS += -DVERIFICATION_MODE
EXTRA_CFLAGS += -DWIFI_MODULE_DVT

CONFIG_PRE_CAL_SUPPORT=y
ifeq ($(CONFIG_PRE_CAL_SUPPORT),y)
EXTRA_CFLAGS += -DPRE_CAL_MT7916_SUPPORT
endif

ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
EXTRA_CFLAGS += -DIGMP_SNOOPING_NON_OFFLOAD
endif

chip_objs += $(SRC_DIR)/chips/mt7916.o\
                $(SRC_DIR)/chips/mt7916_dbg.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fmac.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
                $(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
                $(SRC_DIR)/mac/mt_cmm_mac.o\
                $(SRC_DIR)/mac/mt_fmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
                $(SRC_DIR)/mcu/mt_cmd.o\
                $(SRC_DIR)/mcu/fwdl_mt.o\
                $(SRC_DIR)/mcu/fw_cmd.o\
                $(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/token.o\
                $(SRC_EMBEDDED_DIR)/common/dbg_ctrl.o\
                $(SRC_EMBEDDED_DIR)/common/verification_mode.o\
                $(SRC_EMBEDDED_DIR)/common/mdvt.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\

ifeq ($(CONFIG_7916_USE_PCI),y)
chip_objs += $(SRC_DIR)/os/linux/pci_main_dev.o
endif
endif

ifeq ($(CONFIG_PER_PKT_CTRL_FOR_CTMR),y)
EXTRA_CFLAGS += -DPER_PKT_CTRL_FOR_CTMR
endif

ifeq ($(CONFIG_CHIP_MT7981),y)
EXTRA_CFLAGS += -DMT7981 -DMT_BBP -DMT_RF -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI
EXTRA_CFLAGS += -DMT_DMAC -DMT_FMAC -DDATA_TXPWR_CTRL
EXTRA_CFLAGS += -DICAP_FW_SUPPORT
EXTRA_CFLAGS += -DPS_STA_FLUSH_SUPPORT
EXTRA_CFLAGS += -DDFS_MT7981_DEDICATED_ZW
EXTRA_CFLAGS += -DDFS_ADJ_BW_ZERO_WAIT
ifeq ($(CONFIG_WIFI_FW_BIN_LOAD),y)
EXTRA_CFLAGS += -DCONFIG_MT7981_FW_BIN_LOAD
endif
ifeq ($(CONFIG_WIFI_SKB_USES_SLAB),y)
EXTRA_CFLAGS += -DMT7981_WIFI_SKB_USES_SLAB
endif
EXTRA_CFLAGS += -DMTK_FE_RESET_RECOVER
ifeq ($(CONFIG_WIFI_TWT_SUPPORT),y)
EXTRA_CFLAGS += -DDYNAMIC_TWT_NODE_SUPPORT
endif
EXTRA_CFLAGS += -DCONFIG_3_WIRE_SUPPORT
#Add for default debug driver INT done issue
EXTRA_CFLAGS += -DCONFIG_TP_DBG

# Turn on PCI for FPGA stage.
CONFIG_7981_USE_PCI=n
ifeq ($(CONFIG_7981_USE_PCI),y)
EXTRA_CFLAGS += -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DCONFIG_FPGA_MODE -DMT7981_FPGA
else
EXTRA_CFLAGS += -DRTMP_RBUS_SUPPORT
EXTRA_CFLAGS += -DMULTI_INTR_SUPPORT
EXTRA_CFLAGS += -DWIFI_RAM_EMI_SUPPORT
endif

ifeq ($(CONFIG_BACKGROUND_SCAN_SUPPORT),y)
EXTRA_CFLAGS += -DIPI_SCAN_SUPPORT
endif

#HWITS for WTBL MISS INT
#EXTRA_CFLAGS += -DCONFIG_WTBL_MISS_DVT

EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DRTMP_EFUSE_SUPPORT
#FPGA efuse TBD, now the in-band cmd EXT_CMD_ID_EFUSE_ACCESS will lead F/W hang on FPGA(D)
EXTRA_CFLAGS += -DCAL_FREE_IC_SUPPORT
EXTRA_CFLAGS += -DBCN_OFFLOAD_SUPPORT
EXTRA_CFLAGS += -DERR_RECOVERY
#MT7981 Tx Latency
#EXTRA_CFLAGS += -DEAP_STATS_SUPPORT
#EXTRA_CFLAGS += -DEAP_ENHANCE_STATS_SUPPORT
#EXTRA_CFLAGS += -DNEW_SET_RX_STREAM
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT -DCUT_THROUGH
EXTRA_CFLAGS += -DUNIFY_FW_CMD
#EXTRA_CFLAGS += -DTMR_VERIFY
EXTRA_CFLAGS += -DNEED_ROM_PATCH
EXTRA_CFLAGS += -DMAC_INIT_OFFLOAD
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DCONFIG_FWOWN_SUPPORT
EXTRA_CFLAGS += -DNEWSEC
EXTRA_CFLAGS += -DSINGLE_SKU_V2
#EXTRA_CFLAGS += -DTHERMAL_PROTECT_SUPPORT
EXTRA_CFLAGS += -DCONFIG_HW_HAL_OFFLOAD
EXTRA_CFLAGS += -DRX_SCATTER
EXTRA_CFLAGS += -DPKT_BUDGET_CTRL_SUPPORT
EXTRA_CFLAGS += -DHW_TX_AMSDU_SUPPORT
EXTRA_CFLAGS += -DDBG_AMSDU
EXTRA_CFLAGS += -DFW_LOG_DUMP
EXTRA_CFLAGS += -DDOT11V_MBSSID_SUPPORT
EXTRA_CFLAGS += -DA_BAND_SUPPORT
EXTRA_CFLAGS += -DSWACI_MECHANISM
#EXTRA_CFLAGS += -DMT7915_E1_WORKAROUND
EXTRA_CFLAGS += -DRELEASE_EXCLUDE

#WFLAGS += -DCONFIG_CSO_SUPPORT
#WFLAGS += -DMSP_UTILITY
EXTRA_CFLAGS += -DTX_POWER_CONTROL_SUPPORT
ifeq ($(CONFIG_G_BAND_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DG_BAND_256QAM
ifeq ($(CONFIG_BRCM_256QAM_SUPPORT),y)
EXTRA_CFLAGS += -DBRCM_256QAM
endif
endif

CONFIG_PROPRIETARY_DRIVER=y

ifeq ($(CONFIG_PROPRIETARY_DRIVER),y)
EXTRA_CFLAGS += -DCONFIG_PROPRIETARY_DRIVER -DPROPRIETARY_DRIVER_SUPPORT
CONFIG_PLATFORM_DRV_SUPPORT=n
else
CONFIG_PLATFORM_DRV_SUPPORT=y
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPROT),y)
EXTRA_CFLAGS += -DPLATFORM_DRIVER_SUPPORT
endif

#EXTRA_CFLAGS += -Wframe-larger-than=4096
#WFLAGS += -Werror

ifeq ($(CONFIG_HDR_TRANS_TX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_TX_SUPPORT -DCUT_THROUGH_FULL_OFFLOAD
endif
ifeq ($(CONFIG_HDR_TRANS_RX_SUPPORT),y)
        EXTRA_CFLAGS += -DHDR_TRANS_RX_SUPPORT
endif

#ifeq ($(CONFIG_RATE_ADAPT_AGBS_SUPPORT),y)
        EXTRA_CFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
        EXTRA_CFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
#endif

ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
        EXTRA_CFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(CONFIG_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
#    ifeq ($(CONFIG_WSC_INCLUDED),y)
#        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
#    endif

    func_objs += $(SRC_EMBEDDED_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_RADIUS_ACCOUNTING_SUPPORT),y)
        EXTRA_CFLAGS += -DRADIUS_ACCOUNTING_SUPPORT
endif

ifeq ($(CONFIG_ATE_SUPPORT),y)
chip_objs += $(SRC_DIR)/ate/mt_mac/mt_testmode_fmac.o
endif

ifeq ($(CONFIG_FDB_SUPPORT),y)
    EXTRA_CFLAGS += -DMT_FDB
    chip_objs += $(SRC_DIR)/mcu/mt_fdb.o
endif

ifeq ($(CONFIG_TXBF_SUPPORT),y)
    EXTRA_CFLAGS += -DHE_TXBF_SUPPORT
endif

# Rx detect for multi client
EXTRA_CFLAGS += -DRX_COUNT_DETECT
ifeq ($(CONFIG_SOC_MT7621),y)
EXTRA_CFLAGS += -DKERNEL_RPS_ADJUST
EXTRA_CFLAGS += -DRX_RPS_SUPPORT
endif
EXTRA_CFLAGS += -DVERIFICATION_MODE
EXTRA_CFLAGS += -DWIFI_MODULE_DVT

CONFIG_PRE_CAL_SUPPORT=y
ifeq ($(CONFIG_PRE_CAL_SUPPORT),y)
EXTRA_CFLAGS += -DPRE_CAL_MT7981_SUPPORT
endif

ifeq ($(CONFIG_IGMP_SNOOP_SUPPORT),y)
EXTRA_CFLAGS += -DIGMP_SNOOPING_NON_OFFLOAD -DIGMP_SNOOPING_DENY_LIST
endif

chip_objs += $(SRC_DIR)/chips/mt7981.o\
                $(SRC_DIR)/chips/mt7981_dbg.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fmac.o\
                $(SRC_DIR)/hw_ctrl/cmm_asic_mt_fw.o\
                $(SRC_DIR)/hw_ctrl/cmm_chip_mt.o\
                $(SRC_DIR)/mac/mt_cmm_mac.o\
                $(SRC_DIR)/mac/mt_fmac.o\
                $(SRC_EMBEDDED_DIR)/mcu/mcu.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_core.o\
                $(SRC_EMBEDDED_DIR)/mcu/andes_mt.o \
                $(SRC_DIR)/mcu/mt_cmd.o\
                $(SRC_DIR)/mcu/fwdl_mt.o\
                $(SRC_DIR)/mcu/fw_cmd.o\
                $(SRC_DIR)/mcu/fwdl.o\
                $(SRC_DIR)/phy/mt_rf.o\
                $(SRC_DIR)/phy/rf.o\
                $(SRC_DIR)/phy/mt_phy.o\
                $(SRC_DIR)/phy/rlm_cal_cache.o\
                $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o\
                $(SRC_EMBEDDED_DIR)/common/hif_pci.o\
                $(SRC_EMBEDDED_DIR)/common/ee_prom.o\
                $(SRC_EMBEDDED_DIR)/common/ee_efuse.o\
                $(SRC_EMBEDDED_DIR)/common/token.o\
                $(SRC_EMBEDDED_DIR)/common/dbg_ctrl.o\
                $(SRC_EMBEDDED_DIR)/common/verification_mode.o\
                $(SRC_EMBEDDED_DIR)/common/mdvt.o\
                $(SRC_DIR)/protocol/tmr.o\
                $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                $(SRC_DIR)/hw_ctrl/cmm_fw_uni_cmd.o\
                $(SRC_DIR)/hw_ctrl/cmm_fw_uni_event.o\

ifeq ($(CONFIG_7981_USE_PCI),y)
chip_objs += $(SRC_DIR)/os/linux/pci_main_dev.o
endif
endif

ifeq ($(CONFIG_RTMP_FLASH_SUPPORT)$(CONFIG_MTD),yy)
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ifeq ($(CONFIG_MTD_SPI_NOR),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
else
ifeq ($(CONFIG_MTD_NAND),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif
endif
else
ifeq ($(CONFIG_DEFAULTS_KERNEL_4_4),y)
ifeq ($(CONFIG_MTD_SPI_NOR),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
else
ifeq ($(CONFIG_MTD_NAND),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif
endif
endif
endif
endif

ifeq ($(CONFIG_DEFAULTS_MEDIATEK_MT7622),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MTD
endif

ifeq ($(CONFIG_PLATFORM_DRV_SUPPORT),y)
chip_objs += $(SRC_DIR)/os/linux/wbsys_main_dev.o
else
chip_objs += $(SRC_DIR)/os/linux/rbus_prop_dev.o
endif

ifeq ($(CONFIG_FW_LOG_DUMP_SUPPORT),y)
EXTRA_CFLAGS += -DFW_LOG_DUMP
cmm_objs += $(SRC_EMBEDDED_DIR)/common/dbg_ctrl.o
endif

########################################################
# Wlan service related files
########################################################
ifeq ($(CONFIG_WLAN_SERVICE),y)
EXTRA_CFLAGS += -I$(WIFI_DRV_PATH)/wlan_service/include \
		-I$(WIFI_DRV_PATH)/wlan_service/service/include \
		-I$(WIFI_DRV_PATH)/wlan_service/glue/hal/include \
		-I$(WIFI_DRV_PATH)/wlan_service/glue/osal/include

serv_objs := $(SERV_DIR)/agent/agent.o\
		$(SERV_DIR)/service/service_test.o\
		$(SERV_DIR)/service/test_engine.o\
		$(SERV_DIR)/glue/hal/jedi/test_dmac.o\
		$(SERV_DIR)/glue/hal/jedi/operation_jedi.o\
		$(SERV_DIR)/glue/osal/jedi/net_adaption_jedi.o\
		$(SERV_DIR)/glue/osal/jedi/sys_adaption_jedi.o
endif

#
# Root
#

$(DRV_NAME)-objs += $(ap_objs) $(cmm_objs) $(asic_objs) $(chip_objs) $(rate_objs)\
                    $(spec_objs) $(func_objs) $(os_objs) $(dot11_ft_objs) $(serv_objs)

$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/eeprom.o\
					$(SRC_EMBEDDED_DIR)/common/ee_flash.o\
					$(SRC_EMBEDDED_DIR)/common/ee_efuse.o

$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/cmm_mac_pci.o
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/hif_pci.o

$(DRV_NAME)-objs += $(SRC_DIR)/os/linux/rt_pci_rbus.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_util.o\
                    #$(SRC_DIR)/os/linux/rbus_main_dev.o

ifeq ($(CONFIG_ATE_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_DIR)/ate/ate_agent.o\
                    $(SRC_DIR)/ate/testmode_ioctl.o\
		    $(SRC_DIR)/ate/mt_mac/mt_testmode.o\
 		    $(SRC_DIR)/ate/LoopBack.o
endif


ifeq ($(CONFIG_TCP_RACK_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_EMBEDDED_DIR)/common/cmm_tcprack.o
endif
#CONFIG_WIFI_SYSDVT=n
ifeq ($(CONFIG_WIFI_SYSDVT), y)
EXTRA_CFLAGS += -DVERIFICATION_MODE
DVT_DIR=$(WIFI_DRV_PATH)/mt_wifi/embedded/dvt
DVT_OBJ_PATH=$(SRC_EMBEDDED_DIR)/dvt
include $(DVT_DIR)/dvt.mk
$(DRV_NAME)-objs += $(obj_dvt)
EXTRA_CFLAGS += -DCONFIG_WIFI_SYSDVT -DTRAFFIC_NOTIFY
EXTRA_CFLAGS += -I$(DVT_DIR)/include
endif

CONFIG_WIFI_DBG_TXCMD=y
ifeq ($(CONFIG_WIFI_DBG_TXCMD), y)
DBG_TXCMD_DIR=$(WIFI_DRV_PATH)/mt_wifi/embedded/dbg_txcmd
DBG_TXCMD_OBJ_PATH=$(SRC_EMBEDDED_DIR)/dbg_txcmd
include $(DBG_TXCMD_DIR)/dbg_txcmd.mk
$(DRV_NAME)-objs += $(obj_dbg_txcmd)
EXTRA_CFLAGS += -DCONFIG_WIFI_DBG_TXCMD
EXTRA_CFLAGS += -I$(DBG_TXCMD_DIR)/include
endif

# OCE
ifeq ($(CONFIG_OCE_SUPPORT),y)
EXTRA_CFLAGS += -DOCE_SUPPORT -DOCE_FILS_SUPPORT
func_objs += $(SRC_EMBEDDED_DIR)/common/oce_fils.o
func_objs += $(SRC_EMBEDDED_DIR)/common/oce.o
endif

ifeq ($(CONFIG_6G_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_6G_SUPPORT
ifeq ($(CONFIG_BSSMGR_CROSS_MODULE_SUPPORT),y)
EXTRA_CFLAGS += -DBSSMGR_CROSS_MODULE_SUPPORT
endif
endif

ifeq ($(CONFIG_MT7976_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_MT7976_SUPPORT
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
              -Wstrict-prototypes -Wno-trigraphs -Wall

EXTRA_CFLAGS += -DOLDSEC #-Werror
#-DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS

EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DSCAN_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DDOT11N_DRAFT3 -DSTATS_COUNT_SUPPORT -DIAPP_SUPPORT -DDOT1X_SUPPORT
#EXTRA_CFLAGS += -DRALINK_ATE -DRALINK_QA -DCONFIG_RT2880_ATE_CMD_NEW

#provide busy time statistics for every TBTT */
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_STATS

# provide busy time alarm mechanism
# use the function to avoid to locate in some noise environments
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_ALARM

ifeq ($(CONFIG_RT2860V2_AUTO_CH_SELECT_ENCANCE),y)
EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_RT2860V2_SNMP),y)
EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_AP_32B_DESC),y)
EXTRA_CFLAGS += -DDESC_32B_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_HW_ANTENNA_DIVERSITY),y)
EXTRA_CFLAGS += -DHW_ANTENNA_DIVERSITY_SUPPORT
endif

ifeq ($(CONFIG_RT2860V2_EXT_CHANNEL_LIST),y)
EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_MEMORY_SHRINK),y)
EXTRA_CFLAGS += -DMEMORY_SHRINK
endif

ifeq ($(CONFIG_MEMORY_SHRINK_AGGRESS),y)
EXTRA_CFLAGS += -DMEMORY_SHRINK_AGGRESS
endif

ifeq ($(CONFIG_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
else
#EXTRA_CFLAGS += -DDBG
endif
EXTRA_CFLAGS += -DDBG

#ifeq ($(CONFIG_DBG_ENHANCE),y)
EXTRA_CFLAGS += -DDBG_ENHANCE
#endif

ifeq ($(CONFIG_RTMP_INTERNAL_TX_ALC),y)
EXTRA_CFLAGS += -DRTMP_INTERNAL_TX_ALC
endif

ifeq ($(CONFIG_RALINK_MT7621),y)
EXTRA_CFLAGS += -DRACTRL_LIMIT_MAX_PHY_RATE=1300
EXTRA_CFLAGS += -DCONFIG_TP_DBG
endif

ifeq ($(CONFIG_FAST_NAT_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_FAST_NAT_SUPPORT
endif
ifeq ($(CONFIG_WLAN_HOOK),y)
ifeq ($(CONFIG_WHNAT_SUPPORT),y)
EXTRA_CFLAGS += -DWHNAT_SUPPORT
EXTRA_CFLAGS += -DWFDMA_WED_COMPATIBLE
else
ifeq ($(CONFIG_WHNAT_SUPPORT),m)
EXTRA_CFLAGS += -DWHNAT_SUPPORT
EXTRA_CFLAGS += -DWFDMA_WED_COMPATIBLE
endif
endif
endif

ifeq ($(CONFIG_EASY_SETUP_SUPPORT),y)
EXTRA_CFLAGS += -DWH_EZ_SETUP
EXTRA_CFLAGS += -DNEW_CONNECTION_ALGO
EXTRA_CFLAGS += -DEZ_NETWORK_MERGE_SUPPORT
EXTRA_CFLAGS += -DEZ_DUAL_BAND_SUPPORT
EXTRA_CFLAGS += -DEZ_ROAM_SUPPORT
EXTRA_CFLAGS += -DDUAL_CHIP
EXTRA_CFLAGS += -DEZ_API_SUPPORT
EXTRA_CFLAGS += -DEZ_PUSH_BW_SUPPORT
endif

ifeq ($(CONFIG_MWDS),y)
EXTRA_CFLAGS += -DMWDS -DA4_CONN -DROUTING_TAB_SUPPORT
endif

ifeq ($(CONFIG_EVENT_NOTIFIER_SUPPORT),y)
EXTRA_CFLAGS += -DWH_EVENT_NOTIFIER
endif

ifneq ($(CONFIG_CFG80211_SUPPORT),y)
ifeq ($(CONFIG_WIFI_FWD_SUPPORT),y)
EXTRA_CFLAGS += -DCONFIG_WIFI_PKT_FWD
else
EXTRA_CFLAGS += -DMTFWD
endif
endif

ifeq ($(CONFIG_ROAMING_ENHANCE_SUPPORT),y)
EXTRA_CFLAGS += -DROAMING_ENHANCE_SUPPORT
endif

EXTRA_CFLAGS += -DIP_ASSEMBLY
EXTRA_CFLAGS += -DHTC_DECRYPT_IOT
#EXTRA_CFLAGS += -DANTENNA_DIVERSITY_SUPPORT
#EXTRA_CFLAGS += -DFIXED_HE_GI_SUPPORT
EXTRA_CFLAGS += -Inet/nat
#EXTRA_CFLAGS += -DDBG_STARVATION
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
ifeq ($(CONFIG_CHIP_MT7663E),y)
	ifeq ($(CONFIG_MT_WIFI),m)
		EXTRA_CFLAGS += -DMT_WIFI_MODULE
	endif
else ifeq ($(CONFIG_CHIP_MT7915),y)
	ifeq ($(CONFIG_MT_WIFI),m)
		EXTRA_CFLAGS += -DMT_WIFI_MODULE
	endif
else
	EXTRA_CFLAGS += -DMT_WIFI_MODULE
endif
else
ifeq ($(CONFIG_MT_WIFI),m)
EXTRA_CFLAGS += -DMT_WIFI_MODULE
endif
endif

ifeq ($(CONFIG_MT7915_MT7916_COEXIST_COMPATIBLE),y)
EXTRA_CFLAGS += -DMT7915_MT7916_COEXIST_COMPATIBLE
endif

MODULE_FLAGS=$(EXTRA_CFLAGS)
export MODULE_FLAGS
obj-m+=$(SRC_EMBEDDED_DIR)/plug_in/
