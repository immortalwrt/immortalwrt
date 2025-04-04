#include "rt_config.h"

#include "os/bb_soc.h"
/* #include "rtmp_timer.h" */
/* #include "rt_config.h" */
#ifdef WSC_AP_SUPPORT
extern int wscTimerRunning;
extern int wscStatus;
extern int statusprobe;
extern unsigned short wsc_done;
#endif

VOID BBU_PCIE_Init(void)
{
	pcieReset();
	pcieRegInitConfig();
}

VOID BBUPrepareMAC(IN RTMP_ADAPTER *pAd, PUCHAR macaddr)
{
	UCHAR FourByteOffset = 0;
	UCHAR NWlanExt = 0;

	FourByteOffset = macaddr[5] % 4;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\r\nFourByteOffset is %d", FourByteOffset));
	NWlanExt = pAd->ApCfg.BssidNum;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\r\nNWlanExt is %d", NWlanExt));

	switch (NWlanExt) {
	case 1:
		break;

	case 2:
		switch (FourByteOffset) {
		case 1:
		case 3:
			macaddr[5]--;
			break;

		case 0:
		case 2:
			break;
		}

		break;

	case 3:
	case 4:
		switch (FourByteOffset) {
		case 0:
			break;

		case 1:
			macaddr[5]--;
			break;

		case 2:
			macaddr[5] -= 2;
			break;

		case 3:
			macaddr[5] -= 3;
			break;
		}

		break;

	default:
		break;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("current MAC="MACSTR"\n",
			 MAC2STR(macaddr)));
	/*generate bssid from cpe mac address end, merge from linos, 20100208*/
}

#if defined(BB_SOC) && defined(TCSUPPORT_WLAN_SW_RPS)
extern int (*ecnt_set_wifi_rps_hook)(int RxOn, int WLanCPU, int TxOn, int LanCPU);
extern int (*get_WifitolanRps_hook)(void);

int ecnt_wifi_rx_rps(struct sk_buff *skb)
{
	RTMP_ADAPTER *pAd = NULL;
	RX_BLK *pRxBlk = NULL;

	if (skb) {
		pAd = (RTMP_ADAPTER *) skb->pAd;
		pRxBlk = (RX_BLK *) skb->rxBlk;
		if ((pAd != NULL) && (pRxBlk != NULL))
			asic_rx_pkt_process(pAd, HIF_RX_IDX0, pRxBlk, skb);
	}
	return 0;
}

int rx_detect_flag;
UINT rx_pkt_oneSec;
UINT pre_rx_pkt_num;
UINT32 rx_reach_max_cnt;

void ecnt_rx_detection(RTMP_ADAPTER *pAd)
{
	UINT rxrps = 0;
	UINT rx_pkt_speed = 0;

	rx_pkt_oneSec = pAd->RxTotalByteCnt - pre_rx_pkt_num;
	pre_rx_pkt_num = pAd->RxTotalByteCnt;

	if (get_WifitolanRps_hook && rx_detect_flag) {
		rx_pkt_speed = rx_pkt_oneSec*8/(1024*1024);
		if ((rx_pkt_speed >= pAd->rxThreshold) && (get_WifitolanRps_hook() == 0)) {
			rx_reach_max_cnt++;
			if (rx_reach_max_cnt >= pAd->rxPassThresholdCnt) {
				if (ecnt_set_wifi_rps_hook)
					ecnt_set_wifi_rps_hook(1, 2, 0, 0);
				printk("1. Open rx sw rps\n");
			}
		} else if ((rx_pkt_speed < pAd->rxThreshold) && (get_WifitolanRps_hook() == 1)) {
			if (rx_reach_max_cnt > 0)
				rx_reach_max_cnt--;
			if (rx_reach_max_cnt == 0) {
				if (ecnt_set_wifi_rps_hook)
					ecnt_set_wifi_rps_hook(0, 0, 0, 0);
				printk("1. Close rx sw rps\n");
				}
			}
		}

}
#endif

