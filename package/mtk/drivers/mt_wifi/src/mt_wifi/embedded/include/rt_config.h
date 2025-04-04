/*
 * Copyright (c) [2020], MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/
/*
 ***************************************************************************
 ***************************************************************************

	Module Name:
	rt_config.h

	Abstract:
	Central header file to maintain all include files for all NDIS
	miniport driver routines.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Paul Lin    08-01-2002    created

*/
#ifndef	__RT_CONFIG_H__
#define	__RT_CONFIG_H__


#include "rtmp_comm.h"

#include "rtmp_def.h"
#include "rtmp_chip.h"
#include "rtmp_timer.h"


#ifdef AGS_SUPPORT
#include "ags.h"
#endif /* AGS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BAND_STEERING
#include "band_steering_def.h"
#endif /* BAND_STEERING */
/* #ifdef VOW_SUPPORT */
/* VOW support */
#include "ap_vow.h"
/* #endif */ /* VOW_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WAPP_SUPPORT
#include "wapp/wapp_cmm_type.h"
#endif



#ifdef VERIFICATION_MODE
#include "verification/veri_ctl.h"
#endif

#ifdef MT_MAC
#ifdef CUT_THROUGH
#include "token.h"
#endif /* CUT_THROUGH */
#endif /* MT_MAC */

#include "mlme.h"
/*#include "rtmp_cmd.h" */
#ifdef WIDI_SUPPORT
#include "l2sd_ta.h"
#endif /* WIDI_SUPPORT */

#include "fp_qm.h"
#include "qm.h"
#include "rtmp.h"
#include "security/sec.h"
#include "chlist.h"
#include "spectrum.h"
#ifdef CONFIG_AP_SUPPORT
#include "ap.h"
#include "ap_autoChSel.h"
#endif /* CONFIG_AP_SUPPORT */
#include "rt_os_util.h"

#include "mgmt/mgmt_entrytb.h"

#include "eeprom.h"
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
#include "mcu/mcu.h"
#endif

#ifdef RTMP_EFUSE_SUPPORT
#include "efuse.h"
#endif /* RTMP_EFUSE_SUPPORT */

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#include "rt_os_net.h"


#ifdef UAPSD_SUPPORT
#include "uapsd.h"
#endif /* UAPSD_SUPPORT */

#include "tx_power.h"
#include "txpwr/txpwr.h"
#include "phystate/phystate.h"

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
#include "ap_mbss.h"
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
#include "ap_wds.h"
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
#include "ap_apcli.h"
#include "sta.h"
#endif /* APCLI_SUPPORT */

#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */

#include "ap_ids.h"
#include "ap_cfg.h"

#ifdef CLIENT_WDS
#include "client_wds.h"
#endif /* CLIENT_WDS */

#ifdef ROUTING_TAB_SUPPORT
#include "routing_tab.h"
#endif /* ROUTING_TAB_SUPPORT */

#ifdef A4_CONN
#include "a4_conn.h"
#endif

#ifdef MWDS
#include "mwds.h"
#endif /* MWDS */

#ifdef WH_EVENT_NOTIFIER
#include "event_notifier.h"
#endif /* WH_EVENT_NOTIFIER */

#endif /* CONFIG_AP_SUPPORT */

#ifdef MAT_SUPPORT
#include "mat.h"
#endif /* MAT_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#include "sta.h"
#ifdef WSC_STA_SUPPORT
#define STA_WSC_INCLUDED
#endif /* WSC_STA_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
#include "igmp_snoop.h"
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef CONFIG_MAP_SUPPORT
#include "map.h"
#endif
#ifdef CONFIG_ATE
#include "ate_agent.h"
#include "ate.h"
#include "mt_testmode.h"
#include "testmode_common.h"
#ifdef LINUX
#include <linux/utsname.h>
#endif
#if defined(CONFIG_WLAN_SERVICE)
#include "agent.h"
#endif /* CONFIG_WLAN_SERVICE */
#endif /* CONFIG_ATE */

#ifdef WCX_SUPPORT
#include <mach/mt_boot.h>
#include "meta_agent.h"
#endif /* WCX_SUPPORT */

#ifdef CONFIG_QA
/* #include "qa_agent.h" */
#include "testmode_ioctl.h"
#include "LoopBack.h"
#endif /* CONFIG_QA */

#ifdef CONFIG_QA
#ifndef CONFIG_ATE
#error "For supporting QA GUI, please set HAS_ATE=y and HAS_QA_SUPPORT=y."
#endif /* CONFIG_ATE */
#endif /* CONFIG_QA */


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
#include "ap_repeater.h"
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef DOT11K_RRM_SUPPORT
#include "rrm.h"
#endif /* DOT11K_RRM_SUPPORT */

#ifdef MBO_SUPPORT
#include "mbo.h"
#endif /* MBO_SUPPORT */

#ifdef OCE_SUPPORT
#include "oce.h"
#endif /* OCE_SUPPORT */

#ifdef DOT11Z_TDLS_SUPPORT
#include "tdls.h"
#include "tdls_uapsd.h"
#endif /* DOT11Z_TDLS_SUPPORT */

#if defined(AP_WSC_INCLUDED) || defined(STA_WSC_INCLUDED)
#define WSC_INCLUDED
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef WDS_SUPPORT
#define RALINK_PASSPHRASE	"Ralink"
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_WPA_SUPPLICANT_SUPPORT
#ifndef APCLI_SUPPORT
#error "Build Apcli for being controlled by NetworkManager or wext, please set HAS_APCLI_SUPPORT=y and HAS_APCLI_WPA_SUPPLICANT=y"
#endif /* APCLI_SUPPORT */
#define WPA_SUPPLICANT_SUPPORT
#endif /* APCLI_WPA_SUPPLICANT_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
#ifndef WPA_SUPPLICANT_SUPPORT
#error "Build for being controlled by NetworkManager or wext, please set HAS_WPA_SUPPLICANT=y and HAS_NATIVE_WPA_SUPPLICANT_SUPPORT=y"
#endif /* WPA_SUPPLICANT_SUPPORT */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

#endif /* CONFIG_STA_SUPPORT */

#ifdef WSC_INCLUDED
#include "security/crypt_biginteger.h"
#include "security/crypt_dh.h"
#include "wsc_tlv.h"

#endif /* WSC_INCLUDED */


#ifdef IKANOS_VX_1X0
#include "vr_ikans.h"
#endif /* IKANOS_VX_1X0 */

#ifdef DOT11R_FT_SUPPORT
#include	"ft.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#include "rrm.h"
#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
#include "security/pmf.h"
#endif /* DOT11W_PMF_SUPPORT */



#ifdef P2P_SUPPORT
/*#include "p2p_inf.h" */
#include "p2p.h"
#include "p2pcli.h"
#endif /* P2P_SUPPORT */

#ifdef WFD_SUPPORT
#include "wfd.h"
#endif /* WFD_SUPPORT */

#ifdef DOT11_N_SUPPORT
#include "ht.h"
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
#include "vht.h"
#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
#include "he.h"
#include "bss_color.h"
#endif /* DOT11_HE_AX */

#ifdef CONFIG_STA_SUPPORT
#include "sta_cfg.h"
#endif /* CONFIG_STA_SUPPORT */

#ifdef WORKQUEUE_BH
#include <linux/workqueue.h>
#endif /* WORKQUEUE_BH / */

#ifdef MT_MAC
#ifdef CFG_SUPPORT_MU_MIMO
/* TODO: NeedModify-Jeffrey, shall we integrate the data structure definition in ap_mumimo.h and sta_mumimo.c? */
#ifdef CONFIG_AP_SUPPORT
#include "ap_mumimo.h"
#endif /* CONFIG_AP_SUPPORT */
#endif

#ifdef CFG_SUPPORT_MU_MIMO_RA
#ifdef CONFIG_AP_SUPPORT
#include "ap_mura.h"
#endif /* CONFIG_AP_SUPPORT */
#endif

#ifdef CFG_SUPPORT_FALCON_MURU
#ifdef CONFIG_AP_SUPPORT
#include "ap_muru.h"
#endif /* CONFIG_AP_SUPPORT */
#endif

#ifdef CFG_SUPPORT_FALCON_TXCMD_DBG
#include "he_cfg.h"
#endif /* CFG_SUPPORT_FALCON_TXCMD_DBG */

#endif /* MT_MAC */

#ifdef BAND_STEERING
#include "band_steering.h"
#endif /* BAND_STEERING */

#ifdef WAPP_SUPPORT
#include "wapp/wapp.h"
#endif /* WAPP_SUPPORT */

#ifdef TXBF_SUPPORT
#ifdef MT_MAC
#include "txbf/mt_txbf.h"
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
#include "icap.h"
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */

#ifdef SMART_ANTENNA
#include "smartant.h"
#endif /* SMART_ANTENNA */

#ifdef MT_MAC
#include "mt_io.h"
#include "protocol/tmr.h"
#endif

#include "sniffer/sniffer.h"

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
#include "cfg80211extr.h"
#include "cfg80211_cmm.h"
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

#ifdef CONFIG_DOT11U_INTERWORKING
#include "dot11u_interworking.h"
#include "gas.h"
#endif

#if defined(CONFIG_DOT11V_WNM) || defined(CONFIG_PROXY_ARP)
#include "wnm.h"
#endif

#if defined(CONFIG_HOTSPOT) || defined(CONFIG_PROXY_ARP)
#include "hotspot.h"
#endif

#ifdef LINUX
#ifdef CONFIG_TRACE_SUPPORT
#include "os/trace.h"
#endif
#endif

#include "tm.h"
#include "hw_ctrl.h"
#include "hdev_ctrl.h"
#ifdef DOT11_HE_AX
#ifdef WIFI_TWT_SUPPORT
#include "twt_ctrl.h"
#include "protocol/twt.h"
#include "protocol/twt_core.h"
#endif /* WIFI_TWT_SUPPORT */
#endif /* DOT11_HE_AX */

#include "protocol/protection.h"

#ifdef SINGLE_SKU_V2
#include "txpwr/single_sku.h"
#endif

#include "bcn.h"

#ifdef BACKGROUND_SCAN_SUPPORT
#include "bgnd_scan.h"
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
#include "scs.h"
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#ifdef DYNAMIC_WMM_SUPPORT
#include "dynwmm.h"
#endif /* DYNAMIC_WMM_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
#include "cmm_tcprack.h"
#endif

#include "wlan_config/config_export.h"
#include "mgmt/be_export.h"

#if defined(BB_SOC) && defined(BB_RA_HWNAT_WIFI)
#include <linux/foe_hook.h>
#endif

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include "phy/rlm_cal_cache.h"
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#define MCAST_WCID_TO_REMOVE 0 /* Pat: */

#if defined(RED_SUPPORT)
#include "ra_ac_q_mgmt.h"
#endif /* RED_SUPPORT */
#ifdef DABS_QOS
#include "dabs_qos.h"
#endif
#include "fq_qm.h"
#include "cmm_rvr_dbg.h"
#include "fsm/fsm_sync.h"
#include "fsm/fsm_cntl.h"
#include "fsm/fsm_auth.h"
#include "fsm/fsm_assoc.h"
#include "multi_hif.h"
#include "misc_app.h"

#ifdef WTBL_TDD_SUPPORT
#include "mgmt/wtbl_tdd.h"
#endif /* WTBL_TDD_SUPPORT */

#ifdef RANDOM_PKT_GEN
#include "mac/mac_mt/dmac/dma_sch.h"
#endif /* RANDOM_PKT_GEN */

#include "log_time.h"

#ifdef CFG_SUPPORT_FALCON_SR
#ifdef CONFIG_AP_SUPPORT
#include "sr_cmd.h"
#endif /* CONFIG_AP_SUPPORT */
#endif /* CFG_SUPPORT_FALCON_SR */

#ifdef CFG_SUPPORT_FALCON_PP
#ifdef CONFIG_AP_SUPPORT
#include "pp_cmd.h"
#endif /* CONFIG_AP_SUPPORT */
#endif /* CFG_SUPPORT_FALCON_PP */

#ifdef WIFI_DIAG
#include "os/diag.h"
#endif

#include "capi.h"

#ifdef WIFI_MODULE_DVT
#include "mdvt.h"
#endif


#ifdef CONFIG_6G_SUPPORT
#include "ap_bss_mnger.h"
#ifdef CONFIG_6G_AFC_SUPPORT
#include "afc.h"
#endif /* CONFIG_6G_AFC_SUPPORT */
#endif /* CONFIG_6G_SUPPORT */
#ifdef QOS_R1
#include "qos.h"
#endif
#ifdef CFG_SUPPORT_CSI
#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#endif

#endif	/* __RT_CONFIG_H__ */
