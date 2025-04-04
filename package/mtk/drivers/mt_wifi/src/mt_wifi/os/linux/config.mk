################################################################
# Common Feature Selection
################################################################

# Support ATE and QA function
HAS_ATE=y
HAS_QA_SUPPORT=y

# Support wlan service function
HAS_WLAN_SERVICE=y

# Support WSC function
HAS_WSC=y
HAS_WSC_LED=n
HAS_WSC_NFC=n
HAS_IWSC_SUPPORT=n

HAS_TXBF_SUPPORT=y
HAS_VHT_TXBF_SUPPORT=y
HAS_HE_TXBF_SUPPORT=y

HAS_UAPSD_SUPPORT=y

HAS_CSO_SUPPORT=n

# Support Air Monitor
HAS_AIR_MONITOR=n
HAS_LED_CONTROL_SUPPORT=n

#Support TCP_RACK function
HAS_TCP_RACK_SUPPORT=n

HAS_THERMAL_PROTECT_SUPPORT=y

#Support Internal-Capture function
HAS_ICAP_SUPPORT=y

#Support Wifi-Spectrum function
HAS_WIFI_SPECTRUM_SUPPORT=y

#Support PHY In Chip Sniffer function
HAS_PHY_ICS_SUPPORT=y

# Support LLTD function
HAS_LLTD=y

#Support features of Single SKU.
HAS_SINGLE_SKU_V2_SUPPORT=y

HAS_RLM_CAL_CACHE=n

HAS_CAL_FREE_IC_SUPPORT=n


#Support Carrier-Sense function
HAS_CS_SUPPORT=n




#Support for Net-SNMP
HAS_SNMP_SUPPORT=n

#Support TSSI Antenna Variation
HAS_TSSI_ANTENNA_VARIATION=n

#Support for dot11r FT
HAS_DOT11R_FT_SUPPORT=n

#Support for dot11k RRM
HAS_DOT11K_RRM_SUPPORT=n


HAS_FW_LOG_DUMP_SUPPORT=y

HAS_KTHREAD_SUPPORT=n

HAS_MEM_ALLOC_INFO_SUPPORT=n

# Support for Multiple Cards
HAS_MC_SUPPORT=n

#Support for Bridge Fast Path & Bridge Fast Path function open to other module
HAS_BGFP_SUPPORT=n
HAS_BGFP_OPEN_SUPPORT=n

#Support GreenAP function
HAS_GREENAP_SUPPORT=n

#Support PCIE ASPM dynamic control
HAS_PCIE_ASPM_DYM_CTRL_SUPPORT=n

#Support 802.11ax TWT
HAS_WIFI_TWT_SUPPORT=y

HAS_FW_DUMP_SUPPORT=n

HAS_DELAY_INT=n

HAS_TRACE_SUPPORT=n

HAS_FW_DEBUG_SUPPORT=y

HAS_KEEP_ALIVE_OFFLOAD=y

#Wifi MTD related access
HAS_WIFI_MTD=n

HAS_CALIBRATION_COLLECTION_SUPPORT=y

HAS_WIFI_REGION32_HIDDEN_SSID_SUPPORT=n

# Falcon MURU
HAS_FALCON_MURU_SUPPORT=y

# MU-MIMO
HAS_MU_MIMO_SUPPORT=y

# Falcon SR
HAS_FALCON_SR_SUPPORT=y

HAS_FLASH_SUPPORT=n

# Falcon TXCMD
HAS_FALCON_TXCMD_SUPPORT_DBG=y

#------ IOT Feature Support ------#
# This workaround can fix IOT issue with some specific USB host controllers.
# On those host controllers, a USB packet is unexpectedly divided into 2 smaller
# packets.
HAS_USB_IOT_WORKAROUND2=n

#------ CFG80211 Feature Support (Linux Only) ------#
#Please make sure insmod the cfg80211.ko before our driver
HAS_CFG80211_SUPPORT=n
#smooth the scan signal for cfg80211 based driver
HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT=n
#control two STA using wpa_supplicant
HAS_CFG80211_MULTI_STA_SUPPORT=n
#Cfg80211-based P2P Support
HAS_CFG80211_P2P_SUPPORT=n
#Cfg80211-based P2P Mode Selection (must one be chosen)
HAS_CFG80211_P2P_CONCURRENT_DEVICE=n
HAS_CFG80211_P2P_SINGLE_DEVICE=n
HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE=n
HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT=n
#Cfg80211-based TDLS support
HAS_CFG80211_TDLS_SUPPORT=n
#For android wifi priv-lib (cfg80211_based wpa_supplicant cmd expansion)
HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT=n
#Support RFKILL hardware block/unblock LINUX-only function
HAS_RFKILL_HW_SUPPORT=n

HAS_VERIFICATION_MODE=y

HAS_WIFI_SYSTEM_DVT=y

HAS_AUTOMATION=n
#DBG_TXCMD
HAS_WIFI_DBG_TXCMD=y

# Support AP-Client function
HAS_BEACON_MISS_ON_PRIMARY_CHANNEL=n

################################################################
# Mandatory feature definitions
################################################################
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

WFLAGS := -I$(RT28xx_DIR)/include -I$(RT28xx_EMBEDDED_DIR)/include
WFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -Wno-date-time
WFLAGS += -DLINUX -DENHANCED_STAT_DISPLAY
WFLAGS += -DCONFIG_ANDES_SUPPORT -DRTMP_EFUSE_SUPPORT
WFLAGS += -DSYSTEM_LOG_SUPPORT -DRT28xx_MODE=$(RT28xx_MODE) -DCHIPSET=$(MODULE)
WFLAGS += -DIP_ASSEMBLY -DRTMP_WLAN_HOOK_SUPPORT -DMCS_LUT_SUPPORT
WFLAGS += -DDBDC_MODE -DMULTI_PROFILE
WFLAGS += -DDBG
WFLAGS += -DMT_WIFI_MODULE -DCONFIG_PROPRIETARY_DRIVER # Need to think about 3st platform
#WFLAGS += -DDBG_DIAGNOSE
#WFLAGS += -Wframe-larger-than=4096

# For linux kernel > 3.5 to select SLAB or PAGE memory alloc method
#WFLAGS += -DCONFIG_WIFI_PAGE_ALLOC_SKB
#WFLAGS += -DCONFIG_WIFI_SLAB_ALLOC_SKB


### Common HW/SW definitions ###
HAS_DOT11_N_SUPPORT=y

HAS_DOT11W_PMF_SUPPORT=y

#Support statistics count
HAS_STATS_COUNT=y

HAS_ERR_RECOVERY=y

HAS_HW_HAL_OFFLOAD=y

#Wifi System FW cmd support version
HAS_WIFI_SYS_FW_V1=y
HAS_WIFI_SYS_FW_V2=y

################################################################
# AP Feature Selection and definitions
################################################################
ifneq ($(findstring AP,$(RT28xx_MODE)),)
include $(RT28xx_OS_DIR)/linux/config_ap.mk
endif

HAS_WIFI_EAP_FEATURE=y

HAS_WIFI_GPIO_CTRL=y

################################################################
# STA Feature Selection and definitions
################################################################
ifneq ($(findstring STA,$(RT28xx_MODE)),)
include $(RT28xx_OS_DIR)/linux/config_sta.mk
endif


################################################################
# Chipset definitions
################################################################
include $(RT28xx_OS_DIR)/linux/config_chipset.mk


################################################################
# Common Feature Compiler Flag
################################################################

ifeq ($(HAS_ATE),y)
WFLAGS += -DCONFIG_ATE
WFLAGS += -I$(RT28xx_DIR)/ate/include
ifeq ($(HAS_QA_SUPPORT),y)
WFLAGS += -DCONFIG_QA
endif
endif

ifeq ($(HAS_WLAN_SERVICE),y)
WFLAGS += -DCONFIG_WLAN_SERVICE
WFLAGS += -I$(RT28xx_DIR)/../wlan_service/include \
		-I$(RT28xx_DIR)/../wlan_service/service/include \
		-I$(RT28xx_DIR)/../wlan_service/glue/hal/include \
		-I$(RT28xx_DIR)/../wlan_service/glue/osal/include
endif

ifeq ($(HAS_TXBF_SUPPORT),y)
WFLAGS += -DTXBF_SUPPORT -DVHT_TXBF_SUPPORT
endif

ifeq ($(HAS_HE_TXBF_SUPPORT),y)
WFLAGS += -DHE_TXBF_SUPPORT
endif

ifeq ($(HAS_UAPSD_SUPPORT),y)
WFLAGS += -DUAPSD_SUPPORT
endif

ifeq ($(HAS_BEACON_MISS_ON_PRIMARY_CHANNEL),y)
WFLAGS += -DBEACON_MISS_ON_PRIMARY_CHANNEL
endif

ifeq ($(HAS_DOT11_N_SUPPORT),y)
WFLAGS += -DDOT11_N_SUPPORT -DDOT11N_DRAFT3
endif

ifeq ($(HAS_DOT11_VHT_SUPPORT),y)
WFLAGS += -DDOT11_VHT_AC -DG_BAND_256QAM
endif

ifeq ($(HAS_DOT11_HE_SUPPORT),y)
WFLAGS += -DDOT11_HE_AX
endif

#ifeq ($(HAS_WPA_SUPPLICANT),n)
#ifeq ($(HAS_P2P_SUPPORT),n)
ifeq ($(HAS_DOT11W_PMF_SUPPORT),y)
WFLAGS += -DDOT11W_PMF_SUPPORT -DBCN_PROTECTION_SUPPORT
endif

ifeq ($(HAS_STATS_COUNT),y)
WFLAGS += -DSTATS_COUNT_SUPPORT
endif

ifeq ($(HAS_NEW_RATE_ADAPT_SUPPORT),y)
WFLAGS += -DNEW_RATE_ADAPT_SUPPORT
endif

ifeq ($(HAS_RATE_ADAPT_AGBS_SUPPORT),y)
WFLAGS += -DRATE_ADAPT_AGBS_SUPPORT
WFLAGS += -DRACTRL_FW_OFFLOAD_SUPPORT
endif

ifeq ($(HAS_CSO_SUPPORT), y)
WFLAGS += -DCONFIG_CSO_SUPPORT
endif

ifeq ($(HAS_LED_CONTROL_SUPPORT),y)
WFLAGS += -DLED_CONTROL_SUPPORT
endif

ifeq ($(HAS_ERR_RECOVERY),y)
WFLAGS += -DERR_RECOVERY
endif

ifeq ($(HAS_TCP_RACK_SUPPORT),y)
WFLAGS += -DREDUCE_TCP_ACK_SUPPORT
endif

ifeq ($(HAS_THERMAL_PROTECT_SUPPORT),y)
WFLAGS += -DTHERMAL_PROTECT_SUPPORT
endif

ifeq ($(HAS_ICAP_SUPPORT),y)
WFLAGS += -DINTERNAL_CAPTURE_SUPPORT
endif

ifeq ($(HAS_WIFI_SPECTRUM_SUPPORT),y)
WFLAGS += -DWIFI_SPECTRUM_SUPPORT
endif

ifeq ($(HAS_PHY_ICS_SUPPORT),y)
WFLAGS += -DPHY_ICS_SUPPORT
endif

ifeq ($(HAS_HW_HAL_OFFLOAD),y)
WFLAGS += -DCONFIG_HW_HAL_OFFLOAD
endif

ifeq ($(HAS_LLTD),y)
WFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(HAS_SINGLE_SKU_V2_SUPPORT),y)
WFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(HAS_RLM_CAL_CACHE),y)
WFLAGS += -DRLM_CAL_CACHE_SUPPORT
endif

ifeq ($(HAS_CAL_FREE_IC_SUPPORT),y)
WFLAGS += -DCAL_FREE_IC_SUPPORT
endif


ifeq ($(HAS_WIFI_SYS_FW_V1),y)
WFLAGS += -DWIFI_SYS_FW_V1
endif

ifeq ($(HAS_WIFI_SYS_FW_V2),y)
WFLAGS += -DWIFI_SYS_FW_V2
endif

ifeq ($(HAS_CS_SUPPORT),y)
WFLAGS += -DCARRIER_DETECTION_SUPPORT
endif




ifeq ($(HAS_SNMP_SUPPORT),y)
WFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(HAS_TSSI_ANTENNA_VARIATION),y)
WFLAGS += -DTSSI_ANTENNA_VARIATION
endif

ifeq ($(HAS_DOT11R_FT_SUPPORT),y)
WFLAGS += -DDOT11R_FT_SUPPORT
endif

ifeq ($(HAS_DOT11K_RRM_SUPPORT),y)
WFLAGS += -DDOT11K_RRM_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPPLE_11K_IOT
endif


ifeq ($(HAS_FW_LOG_DUMP_SUPPORT),y)
WFLAGS += -DFW_LOG_DUMP
endif

ifeq ($(HAS_KTHREAD_SUPPORT),y)
WFLAGS += -DKTHREAD_SUPPORT
endif

ifeq ($(HAS_MEM_ALLOC_INFO_SUPPORT),y)
WFLAGS += -DMEM_ALLOC_INFO_SUPPORT
endif

ifeq ($(HAS_MC_SUPPORT),y)
WFLAGS += -DMULTIPLE_CARD_SUPPORT
endif

ifeq ($(HAS_BGFP_SUPPORT),y)
WFLAGS += -DBG_FT_SUPPORT
endif

ifeq ($(HAS_BGFP_OPEN_SUPPORT),y)
WFLAGS += -DBG_FT_OPEN_SUPPORT
endif

ifeq ($(HAS_FW_DUMP_SUPPORT),y)
WFLAGS += -DFW_DUMP_SUPPORT
endif

ifeq ($(HAS_USB_IOT_WORKAROUND2),y)
WFLAGS += -DUSB_IOT_WORKAROUND2
endif

ifeq ($(HAS_DELAY_INT),y)
WFLAGS += -DCONFIG_DELAY_INT
endif

ifeq ($(HAS_TRACE_SUPPORT),y)
WFLAGS += -DCONFIG_TRACE_SUPPORT
endif

ifeq ($(HAS_FW_DEBUG_SUPPORT),y)
WFLAGS += -DCONFIG_FW_DEBUG
endif

ifeq ($(HAS_WIFI_EAP_FEATURE),y)
WFLAGS += -DWIFI_EAP_FEATURE
endif

ifeq ($(HAS_KEEP_ALIVE_OFFLOAD),y)
WFLAGS += -DCONFIG_KEEP_ALIVE_OFFLOAD
endif

ifeq ($(HAS_WIFI_MTD),y)
WFLAGS += -DCONFIG_WIFI_MTD
endif

ifeq ($(HAS_CALIBRATION_COLLECTION_SUPPORT),y)
WFLAGS += -DCONFIG_CALIBRATION_COLLECTION
endif

ifeq ($(HAS_WIFI_REGION32_HIDDEN_SSID_SUPPORT),y)
WFLAGS += -DWIFI_REGION32_HIDDEN_SSID_SUPPORT
endif

ifeq ($(HAS_FLASH_SUPPORT),y)
WFLAGS += -DRTMP_FLASH_SUPPORT
endif

ifeq ($(HAS_WIFI_SYSTEM_DVT),y)
WFLAGS += -I$(RT28xx_EMBEDDED_DIR)/dvt/include
WFLAGS += -DCONFIG_WIFI_SYSDVT -DTRAFFIC_NOTIFY
endif

ifeq ($(HAS_WIFI_DBG_TXCMD),y)
WFLAGS += -I$(RT28xx_EMBEDDED_DIR)/dbg_txcmd/include
WFLAGS += -DCONFIG_WIFI_DBG_TXCMD
endif

ifeq ($(HAS_FALCON_TXCMD_SUPPORT_DBG),y)
WFLAGS += -DCFG_SUPPORT_FALCON_TXCMD_DBG
endif

ifeq ($(HAS_FALCON_MURU_SUPPORT),y)
WFLAGS += -DCFG_SUPPORT_FALCON_MURU
endif

ifeq ($(HAS_MU_MIMO_SUPPORT),y)
WFLAGS += -DCFG_SUPPORT_MU_MIMO
endif

ifeq ($(HAS_FALCON_SR_SUPPORT),y)
WFLAGS += -DCFG_SUPPORT_FALCON_SR
endif


ifeq ($(HAS_AUTOMATION),y)
WFLAGS += -DAUTOMATION
endif
################################################################
# 6. CFG80211 Feature Flag (Linux Only)
################################################################
ifeq ($(HAS_CFG80211_SUPPORT),y)
	WFLAGS += -DRT_CFG80211_SUPPORT -DWPA_SUPPLICANT_SUPPORT
	#WFLAGS += -DEXT_BUILD_CHANNEL_LIST
	ifeq ($(HAS_RFKILL_HW_SUPPORT),y)
		WFLAGS += -DRFKILL_HW_SUPPORT
	endif

	ifeq ($(HAS_CFG80211_SCAN_SIGNAL_AVG_SUPPORT),y)
		WFLAGS += -DCFG80211_SCAN_SIGNAL_AVG
	endif

	ifeq ($(HAS_CFG80211_MULTI_STA_SUPPORT),y)
		WFLAGS += -DCFG80211_MULTI_STA -DCONFIG_AP_SUPPORT -DAPCLI_SUPPORT
		WFLAGS += -DMBSS_SUPPORT -DAP_SCAN_SUPPORT -DMULTI_WMM_SUPPORT
	endif

	ifeq ($(HAS_CFG80211_P2P_SUPPORT),y)
		WFLAGS += -DRT_CFG80211_P2P_SUPPORT -DUAPSD_SUPPORT -DMBSS_SUPPORT -DAP_SCAN_SUPPORT
		WFLAGS += -DCONFIG_AP_SUPPORT -DAPCLI_SUPPORT
		ifeq ($(HAS_CFG80211_P2P_SINGLE_DEVICE),y)
			WFLAGS += -DRT_CFG80211_P2P_SINGLE_DEVICE
		else
			ifeq ($(HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE),y)
				WFLAGS += -DRT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
			else
				WFLAGS += -DRT_CFG80211_P2P_CONCURRENT_DEVICE
				ifeq ($(HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT),y)
					WFLAGS +=  -DMULTI_WMM_SUPPORT -DCONFIG_MULTI_CHANNEL -DRT_CFG80211_P2P_MULTI_CHAN_SUPPORT
				endif #HAS_CFG80211_P2P_MULTI_CHAN_SUPPORT
			endif #HAS_CFG80211_P2P_STATIC_CONCURRENT_DEVICE
		endif #HAS_CFG80211_P2P_SINGLE_DEVICE
	endif #HAS_CFG80211_P2P_SUPPORT

	ifeq ($(HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT),y)
		WFLAGS += -DRT_RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
	endif #HAS_CFG80211_ANDROID_PRIV_LIB_SUPPORT

	ifeq ($(HAS_CFG80211_TDLS_SUPPORT),y)
		WFLAGS += -DCFG_TDLS_SUPPORT -DUAPSD_SUPPORT
	endif
endif #HAS_CFG80211_SUPPORT

################################################################
# Platform definitions
################################################################
include $(RT28xx_OS_DIR)/linux/config_platform.mk
