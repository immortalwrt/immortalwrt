#ifndef __WIFI_MD_COEX_H__
#define __WIFI_MD_COEX_H__

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT

#define APCCCI_DRIVER_FW 0x0100
#define FW_DRIVER_APCCCI 0x0200
#define REGISTER_WIFI_MD_DTB 0x0300
#define UNREGISTER_WIFI_MD_DTB 0x0400
#define QUERY_FW_STATUS 0x0500
#define COEX_DEBUG_LEVEL_MSG 0x0600

#define COEX_WIFI_2G 0x1
#define COEX_WIFI_5G 0x2
#define COEX_WIFI_DBDC 0x3
#define COEX_WIFI_5G_HIGH 0x4
#define COEX_WIFI_5G_LOW 0x5
#define COEX_WIFI_DBDC_5G_HIGH 0x6
#define COEX_WIFI_DBDC_5G_LOW 0x7

#ifdef COEX_DIRECT_PATH
#define ADDR_CONN_INFRA_SYSRAM 0x7c001198
#define VAL_CONN_INFRA_SYSRAM 0x18051800
#endif /* COEX_DIRECT_PATH */

#define QUERY_IDC_INFO_INTVL      1000   /* unit: msec */
#define WIFI_CH_MASK_IDX_NUM	4
#define WIFI_RAM_BAND_NUM	2
#define MAX_WIFI_FREQ_NUM 4

typedef struct _COEX_IDC_INFO {
	/* IDC Enable / Disable */
	BOOLEAN idcEnable;
	/* MD operfreq: LTE/NR */
	UINT8 lte_oper_band;
	UINT16 lte_dl_freq;
	UINT16 lte_ul_freq;
	UINT8 nr_oper_band;
	UINT32 nr_dl_freq;
	UINT32 nr_ul_freq;
	/* MD Unsafe channel (driver/ fw) */
	UINT32 u4SafeChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	UINT32 u4TdmChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	UINT32 u4PwrChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	UINT32 u4FdmChannelBitmask[WIFI_CH_MASK_IDX_NUM];
	/* IDC Solution (Free run / Power backoff / TDM / Unsafe FDM) */
	UINT8 u4WiFiFinalSoluation[WIFI_RAM_BAND_NUM];
	UINT8	u4MDFinalSoluation[WIFI_RAM_BAND_NUM];
	/* MD Connected mode / Idle mode */
	BOOLEAN lteTxExist;
	BOOLEAN nrTxExist;
	/* TDM flag */
	BOOLEAN isTdmForLte[WIFI_RAM_BAND_NUM];
	BOOLEAN isTdmForNr[WIFI_RAM_BAND_NUM];
	UINT16 lteCcBmp[WIFI_RAM_BAND_NUM];
	UINT16 nrCcBmp[WIFI_RAM_BAND_NUM];
	/* Pwr flag */
	UINT16  u2LteScellBmp;
	UINT16  u2NrScellBmp;
	/* MD / Wi-Fi TDM time */
	UINT16 tdm_lte_window;
	UINT16 tdm_lte_conn_window;
	UINT16 tdm_nr_window;
	UINT16 tdm_conn_window;
	/* 3-Wire Arbitration */
	BOOLEAN fg3WireCommon[WIFI_RAM_BAND_NUM];
	UINT_32 u43WirePin;
	UINT_32 u43WireReq;
} COEX_IDC_INFO, *P_COEX_IDC_INFO;

/* Export API function */
#ifdef COEX_DIRECT_PATH
VOID mtf_set_conn_infra_sysram(struct _RTMP_ADAPTER *pAd);
VOID mtf_get_wm_pc_status(struct _RTMP_ADAPTER *pAd);
#endif/* COEX_DIRECT_PATH */

int send_wifi_info_to_wifi_coex(struct _RTMP_ADAPTER *pAd, BOOL status);
VOID Coex_IDC_Info_Handle(RTMP_ADAPTER *pAd, COEX_IDC_INFO *pEventIdcInfo);
INT Set_Idc_State(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Idc_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
VOID send_debug_level_to_wifi_md_coex(UCHAR debug_level);
VOID init_wifi_md_coex_hw_config(struct _RTMP_ADAPTER *pAd);
VOID init_wifi_md_coex(struct _RTMP_ADAPTER *pAd);
VOID deinit_wifi_md_coex(struct _RTMP_ADAPTER *pAd);
extern int register_wifi_md_coex_notifier(struct notifier_block *nb);
extern int unregister_wifi_md_coex_notifier(struct notifier_block *nb);
extern int call_wifi_md_coex_notifier(unsigned long event, void *msg);

#endif /* WIFI_MD_COEX_SUPPORT */
#endif /* __WIFI_MD_COEX_H__ */

