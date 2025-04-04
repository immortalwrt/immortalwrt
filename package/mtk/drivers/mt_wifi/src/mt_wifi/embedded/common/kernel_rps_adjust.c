#include "rt_config.h"
#include "qm.h"
#ifdef KERNEL_RPS_ADJUST
#include "kernel_rps_adjust.h"

/* extern int (*nf_hnat_hook_toggle)(int enable);*/

char proc_rps_cfg[MAX_PROC_RPS_FILE][8];
static const char proc_rps_default_cfg[MAX_PROC_RPS_FILE][8] = {
	"5", "3", "3", "4", "1000", "60", "1"};
static const char *proc_rps_path[MAX_PROC_RPS_FILE] = {
"/sys/class/net/eth0/queues/rx-0/rps_cpus",
"/sys/class/net/ra0/queues/rx-0/rps_cpus",
"/sys/class/net/rax0/queues/rx-0/rps_cpus",
"/proc/irq/31/smp_affinity",
"/proc/sys/net/core/netdev_max_backlog",
"/sys/module/rcupdate/parameters/rcu_cpu_stall_timeout",
"/sys/kernel/debug/hnat/hook_toggle",
"/sys/kernel/debug/hnat/all_entry"
};

static RTMP_OS_FD proc_srcf[MAX_PROC_RPS_FILE] = {NULL};
static RTMP_OS_FS_INFO proc_osFSInfo;
struct net_device *proc_net_dev[MAX_NET_DEV] = {0};
static const char *proc_net_dev_name[MAX_NET_DEV] = {"eth0", "ra0", "rax0"};
static UINT32 tx_queue_cfg[MAX_NET_DEV] = {
DEFAULT_NET_DEV_TX_QLEN, DEFAULT_NET_DEV_TX_QLEN, DEFAULT_NET_DEV_TX_QLEN};
static UINT32 proc_rps_mode_cnt[MAX_PROC_RPS_MODE] = {0};
static BOOLEAN proc_rps_apply[MAX_PROC_RPS_FILE] = {FALSE};
static UINT32 stop_ixia_mode_counter = 0;
static BOOLEAN stop_ixia_mode = FALSE;
static VOID rps_mode_config_setting(struct _RTMP_ADAPTER *pAd, UINT32 proc_rps_mode,
	UINT32 *the_apply_mask, UINT8 *the_band_idx);
/* static void detect_cca_abort(RTMP_ADAPTER *pAd); */

static void parse_hnat_entry(RTMP_ADAPTER *pAd)
{
	UCHAR buf[512];
	int ret, i;

	RtmpOSFSInfoChange(&proc_osFSInfo, TRUE);
	RtmpOSFileSeek(proc_srcf[HNAT_ALL_ENTRY_FILE], 0);
	for (i = 0; i < 4; i++) {
		ret = RtmpOSFileRead(proc_srcf[HNAT_ALL_ENTRY_FILE], buf, 512);
		if (ret <= 0)
			break;
	}
	pAd->ixia_mode_ctl.num_entry = i;
	RtmpOSFileSeek(proc_srcf[HNAT_ALL_ENTRY_FILE], 0);
	RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
}

static BOOLEAN apply_proc_rps_setting(struct _RTMP_ADAPTER *pAd, UINT32 apply_mask, UINT8 band_idx_apply)
{
	pAd->ixia_mode_ctl.rps_mask |= apply_mask;
	pAd->ixia_mode_ctl.band = band_idx_apply;

	if (!(pAd->ixia_mode_ctl.rps_mask & APPLY_NEED_BH_APPLY_FLAG))
		pAd->ixia_mode_ctl.rps_mask |= APPLY_NEED_BH_APPLY_FLAG;

	return TRUE;
}

BOOLEAN apply_proc_rps_setting_bh(struct _RTMP_ADAPTER *pAd)
{
	UINT32 tx_len = 0, rx_len = 0, dbg_lvl = 0;
	INT32 idx, ret = 0;
	BOOLEAN bRet = TRUE;
	UINT32 apply_mask;
	UINT8 band_idx_apply;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;

	apply_mask = pAd->ixia_mode_ctl.rps_mask;
	band_idx_apply = pAd->ixia_mode_ctl.band;

	if (band_idx_apply < DBDC_BAND_NUM) {
		tx_len = pAd->mcli_ctl[band_idx_apply].pkt_avg_len;
		rx_len = pAd->mcli_ctl[band_idx_apply].pkt_rx_avg_len;
	} else {
		bRet = FALSE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : band_idx_apply is %d error.\n", __func__, band_idx_apply));
		return bRet;
	}

	if ((pAd->mcli_ctl[DBDC_BAND0].debug_on & MCLI_DEBUG_RPS_CFG_MODE)
#ifdef DBDC_MODE
	|| (pAd->CommonCfg.dbdc_mode &&
		(pAd->mcli_ctl[DBDC_BAND1].debug_on & MCLI_DEBUG_RPS_CFG_MODE))
#endif
		)
		dbg_lvl = DBG_LVL_OFF;
	else
		dbg_lvl = DBG_LVL_WARN;

	RtmpOSFSInfoChange(&proc_osFSInfo, TRUE);

	if ((proc_srcf[HNAT_HOOK_TOGGLE_FILE] == NULL) && (APPLY_HOOK_TOGGLE_FLAG & apply_mask)) {
		proc_srcf[HNAT_HOOK_TOGGLE_FILE] =
			RtmpOSFileOpen((char *)proc_rps_path[HNAT_HOOK_TOGGLE_FILE],
				O_RDWR|O_TRUNC, 0);

		if (IS_FILE_OPEN_ERR(proc_srcf[HNAT_HOOK_TOGGLE_FILE])) {
			proc_srcf[HNAT_HOOK_TOGGLE_FILE] = NULL;
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s open proc file[%s] OK\n", __func__,
				proc_rps_path[HNAT_HOOK_TOGGLE_FILE]));
		}

	}

	if ((proc_srcf[HNAT_ALL_ENTRY_FILE] == NULL) && (APPLY_ALL_ENTRY_FLAG & apply_mask)) {
		proc_srcf[HNAT_ALL_ENTRY_FILE] =
			RtmpOSFileOpen((char *)proc_rps_path[HNAT_ALL_ENTRY_FILE],
				O_RDWR|O_TRUNC, 0);

		if (IS_FILE_OPEN_ERR(proc_srcf[HNAT_ALL_ENTRY_FILE])) {
			proc_srcf[HNAT_ALL_ENTRY_FILE] = NULL;
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s open proc file[%s] OK\n", __func__,
				proc_rps_path[HNAT_ALL_ENTRY_FILE]));
		}
	}


	if (apply_mask & APPLY_RPS_RESET_FLAG) {
		for (idx = 0; idx < MAX_APPLY_PROC_RPS_FILE; idx++) {
			if (idx == HNAT_ALL_ENTRY_FILE)
				continue;
			if (proc_rps_apply[idx]) {
			/*
				if (idx == HNAT_HOOK_TOGGLE_FILE)
				{
					if(nf_hnat_hook_toggle !=NULL) {
						nf_hnat_hook_toggle(1);
						proc_rps_apply[idx] = FALSE;
					}
					continue;
				}
			*/
				if ((proc_srcf[idx]) && (!IS_FILE_OPEN_ERR(proc_srcf[idx]))) {
					RtmpOSFileSeek(proc_srcf[idx], 0);
					ret = RtmpOSFileWrite(proc_srcf[idx], (char *)proc_rps_default_cfg[idx],
								strlen(proc_rps_default_cfg[idx]));
					RtmpOSFileSeek(proc_srcf[idx], 0);
					if (ret > 0)
						proc_rps_apply[idx] = FALSE;
					else
						bRet = FALSE;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
						("%s():pros_rps %d reset to %s (ret:%d) CPU%u\n",
						__func__, idx, proc_rps_default_cfg[idx],
						ret, smp_processor_id()));
				}
			}
			if (bRet == FALSE) {
				RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
				return bRet;
			}
		}

		for (idx = 0; idx < MAX_NET_DEV; idx++)
			if (proc_net_dev[idx])
			proc_net_dev[idx]->tx_queue_len = DEFAULT_NET_DEV_TX_QLEN;

		for (idx = DBDC_BAND0; idx < DBDC_BAND_NUM; idx++) {
			pAd->mcli_ctl[idx].rps_adjust = FALSE;
			pAd->mcli_ctl[idx].rps_state_flag &= ~(MCLI_RPS_APPLY_FLAG | MCLI_RPS_RESET_FLAG);
			pAd->mcli_ctl[idx].proc_rps_mode = NORMAL_RPS_MODE;
		}

		pAd->ixia_mode_ctl.rps_mask = 0;
		pAd->ixia_mode_ctl.mode_entered = FALSE;

		RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
		return bRet;
	}

	if (apply_mask) {
		UINT32 band_net_dev = (band_idx_apply == DBDC_BAND0) ? BAND0_NET_DEV : BAND1_NET_DEV;

		for (idx = 0; idx < MAX_APPLY_PROC_RPS_FILE; idx++) {
			if ((idx == HNAT_ALL_ENTRY_FILE) && (proc_srcf[idx])) {
				if ((1 << idx) & apply_mask)
					parse_hnat_entry(pAd);
				continue;
			}
			if ((proc_rps_apply[idx] == FALSE) && ((1 << idx) & apply_mask)) {
			/*
				if (idx == HNAT_HOOK_TOGGLE_FILE) {
					if(nf_hnat_hook_toggle !=NULL) {
					       nf_hnat_hook_toggle(0);
					       proc_rps_apply[idx] = TRUE;
					}
					continue;
				}
			*/
				if ((proc_srcf[idx]) && (!IS_FILE_OPEN_ERR(proc_srcf[idx]))) {
					RtmpOSFileSeek(proc_srcf[idx], 0);
					ret = RtmpOSFileWrite(proc_srcf[idx],
						proc_rps_cfg[idx],
						strlen(proc_rps_cfg[idx]));
					if (ret > 0)
						proc_rps_apply[idx] = TRUE;
					else
						bRet = FALSE;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
					("band%u:new rps[%d] mask:%x,cfg=%s,ixia_sta:%u,max_tx_process:%u,mgmt_que:%u,data_que:%u,tx:%u,rx:%u,entry:%u,(ret:%d)\n",
					band_idx_apply, idx, apply_mask, proc_rps_cfg[idx],
					pAd->ixia_mode_ctl.sta_nums, qm->max_tx_process_cnt, qm->max_mgmt_que_num,
					qm->max_data_que_num, tx_len, rx_len, pAd->ixia_mode_ctl.num_entry,
					ret));

					RtmpOSFileSeek(proc_srcf[idx], 0);
				}
			}
			if (bRet == FALSE)
				break;
		}

		proc_net_dev[LAN_NET_DEV]->tx_queue_len = tx_queue_cfg[LAN_NET_DEV];
		if (proc_net_dev[band_net_dev])
		proc_net_dev[band_net_dev]->tx_queue_len = tx_queue_cfg[band_net_dev];
	}

	if (bRet) {
		pAd->mcli_ctl[band_idx_apply].rps_adjust = TRUE;
		pAd->mcli_ctl[band_idx_apply].rps_state_flag |= MCLI_RPS_APPLY_FLAG;
		pAd->ixia_mode_ctl.rps_mask &= ~APPLY_NEED_BH_APPLY_FLAG;
	} else
		pAd->mcli_ctl[band_idx_apply].proc_rps_mode = NORMAL_RPS_MODE;

	RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
	return bRet;
}

static BOOLEAN periodic_detect_ixia_mode(RTMP_ADAPTER *pAd)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	INT i;
	CHAR MaxRssi  = -127, MinRssi  = -127, myAvgRssi = -127, deltaRSSI = 0;
	INT maclowbyteMin = 0, maclowbyteMax = 0;
	UCHAR tempAddr[MAC_ADDR_LEN], pollcnt = 0,
		pollcnt_per_band[DBDC_BAND_NUM] = {0}, band_idx = 0;
	INT maclowbyteSum = 0, temsum = 0, tempMax = 0;
	UINT16 onlinestacnt = 0;
	BOOLEAN iMacflag = FALSE, iRssiflag = FALSE;
	UINT32 dbg_lvl[DBDC_BAND_NUM] = {0}, origRxPath = pAd->Antenna.field.RxPath;
	struct multi_cli_ctl *mcli = NULL;
	struct txop_ctl *txopctl = NULL;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	enum PHY_CAP phy_caps = chip_cap->phy_caps;

	if (!pAd->CommonCfg.dbdc_mode)
		return FALSE;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->mcli_ctl[band_idx].debug_on & MCLI_DEBUG_IXIA_MODE)
			dbg_lvl[band_idx] = DBG_LVL_OFF;
	else
			dbg_lvl[band_idx] = DBG_LVL_INFO;
	}

	if (pAd->MacTab.Size == 0)
		goto IXIA_STATE_RESET;

	if (stop_ixia_mode)
		return FALSE;

	if ((pAd->ixia_mode_ctl.mode_entered) && (pAd->MacTab.Size >= 5) &&
		((pAd->ixia_mode_ctl.sta_nums == pAd->MacTab.Size)
		|| (pAd->MacTab.Size >= 40))) {
		return TRUE;
	}

	/* while DBDC mode is active and standard BW160 is not supported,
	 * we shrink RxPath here and restore it back after calling RTMPMaxRssi
	 * and RTMPMinRssi which use RxPath to determine Rx stream number
	 * rather than dbdc_band0_rx_path/dbdc_band1_rx_path
	 */
	if (!IS_PHY_CAPS(phy_caps, fPHY_CAP_BW160C_STD))
		pAd->Antenna.field.RxPath = 2;

	NdisZeroMemory(tempAddr, MAC_ADDR_LEN);
	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pEntry = &pAd->MacTab.Content[i];
		if (!(IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)))
			continue;

		if (pEntry->MaxHTPhyMode.field.MODE == MODE_HE)
			goto IXIA_STATE_RESET;

		if ((maclowbyteMax == 0) && (maclowbyteMin == 0)) {
			COPY_MAC_ADDR(tempAddr, pEntry->Addr);
			maclowbyteMin = (INT)pEntry->Addr[5];
			maclowbyteMax = (INT)pEntry->Addr[5];
		}

		if ((pAd->CommonCfg.dbdc_mode) && WMODE_CAP_5G(pEntry->wdev->PhyMode)) {
			pollcnt_per_band[DBDC_BAND1]++;
			band_idx = DBDC_BAND1;
		} else {
			pollcnt_per_band[DBDC_BAND0]++;
			band_idx = DBDC_BAND0;
		}

		if (NdisEqualMemory(tempAddr, pEntry->Addr, (MAC_ADDR_LEN - 1))) {
			if (maclowbyteMin > (INT)pEntry->Addr[5])
				maclowbyteMin = (INT)pEntry->Addr[5];
			if (maclowbyteMax < (INT)pEntry->Addr[5])
				maclowbyteMax = (INT)pEntry->Addr[5];
			maclowbyteSum += (INT)pEntry->Addr[5];
		} else if (NdisEqualMemory(tempAddr, pEntry->Addr, (MAC_ADDR_LEN - 3))
				&& NdisEqualMemory(&tempAddr[4], &pEntry->Addr[4], 2)) {
				/*00:41:dd:01:00:00*/
				/*00:41:dd:02:00:00*/
				/*00:41:dd:03:00:00*/
				/*00:41:dd:04:00:00*/
				/*		......	*/
				/*00:41:dd:0f:00:00*/
				/*00:41:dd:10:00:00*/
				/*00:41:dd:11:00:00*/
			if (maclowbyteMin > (INT)pEntry->Addr[3])
				maclowbyteMin = (INT)pEntry->Addr[3];
			if (maclowbyteMax < (INT)pEntry->Addr[3])
				maclowbyteMax = (INT)pEntry->Addr[3];
			maclowbyteSum += (INT)pEntry->Addr[3];
		} else {
			maclowbyteMin = 0;
			maclowbyteMax = 0;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, dbg_lvl[band_idx],
				("%s:WCID%d, DiffMACDetect %x:%x:%x:%x:%x:%x.\n",
				__func__, i, PRINT_MAC(pEntry->Addr)));
			break;
		}
		myAvgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);
		if ((MaxRssi == -127) && (MinRssi == -127)) {
			MaxRssi = myAvgRssi;
			MinRssi = myAvgRssi;
		} else {
			MaxRssi = RTMPMaxRssi(pAd, MaxRssi, myAvgRssi, 0);
			MinRssi = RTMPMinRssi(pAd, MinRssi, myAvgRssi, 0, 0);
		}
		pollcnt += 1;
	}
	pAd->Antenna.field.RxPath = origRxPath;

	deltaRSSI = MaxRssi - MinRssi;
	/*Arithmetic Sequence Property: Sn = n*(a1 + an)/2, an = a1 + (n -1)*d*/
	if (pollcnt > onlinestacnt)
		onlinestacnt = pollcnt;
	temsum = ((INT)onlinestacnt)*(maclowbyteMax + maclowbyteMin) / 2;
	tempMax = ((INT)onlinestacnt - 1) + maclowbyteMin;
	/*Veriwave MAC Address increase by 1.*/
	if ((temsum != 0) && (maclowbyteSum == temsum) &&
		(maclowbyteMax == tempMax) && (pollcnt >= 5))
	/*Arithmetic Sequence and diff is 1.*/
		iMacflag = TRUE;

	if ((deltaRSSI < 10) && (MinRssi >= -65))
		iRssiflag = TRUE;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, dbg_lvl[band_idx],
				("%s:band%u iMacflag=%u, iRssiflag=%u [delta=%d, max=%d min=%d]\n", __func__,
				band_idx, iMacflag, iRssiflag, deltaRSSI, MaxRssi, MinRssi));
	}

	/* if (iMacflag && iRssiflag) { */
	if (iMacflag) {
		band_idx = DBDC_BAND_NUM;
		if (pAd->CommonCfg.dbdc_mode) {
		if ((pollcnt_per_band[DBDC_BAND0] > 0) && (pollcnt_per_band[DBDC_BAND1] == 0))
			band_idx = DBDC_BAND0;

		if ((pollcnt_per_band[DBDC_BAND1] > 0) && (pollcnt_per_band[DBDC_BAND0] == 0))
			band_idx = DBDC_BAND1;
		} else
			band_idx = DBDC_BAND0;

		if (band_idx >= DBDC_BAND_NUM)
			return FALSE;

		pAd->ixia_mode_ctl.sta_nums = pollcnt_per_band[band_idx];

		mcli = &(pAd->mcli_ctl[band_idx]);
		txopctl = &(pAd->txop_ctl[band_idx]);

		if (mcli->kernel_rps_adjust_enable == FALSE)
			mcli->rps_state_flag &= ~MCLI_IXIA_STA_DETECT_FLAG;
		else {
			if (!(mcli->rps_state_flag & MCLI_IXIA_STA_DETECT_FLAG)) {
				UINT32 apply_mask = 0, proc_rps_mode = 0;
				UINT8 band_idx_apply = band_idx;

				strncpy(proc_rps_cfg[HNAT_HOOK_TOGGLE_FILE], "0", 2);
				strncpy(proc_rps_cfg[RCU_STALL_TIMEOUT_FILE], "60", 3);
				proc_rps_mode = (band_idx == DBDC_BAND0) ? BAND0_DETECT_MODE : BAND1_DETECT_MODE;
				rps_mode_config_setting(pAd, proc_rps_mode, &apply_mask, &band_idx_apply);
				apply_mask |= (APPLY_HOOK_TOGGLE_FLAG | APPLY_RCU_TIMEOUT_FLAG);
				apply_mask |= (APPLY_RCU_TIMEOUT_FLAG);
				if (apply_proc_rps_setting(pAd, apply_mask, band_idx_apply))
					pAd->mcli_ctl[band_idx_apply].proc_rps_mode = proc_rps_mode;

#ifdef RX_RPS_SUPPORT
				change_rx_tasklet_method(pAd, TRUE);
				if (band_idx == DBDC_BAND0) {
					if (NR_CPUS == 4) {
					change_rx_qm_cpumap(pAd, 0xd);
					chip_cap->RxSwRpsNum = 3;
					chip_cap->RxSwRpsCpuMap[0] = 0;
					chip_cap->RxSwRpsCpuMap[1] = 3;
					chip_cap->RxSwRpsCpuMap[2] = 2;
					chip_cap->RxSwRpsCpuMap[3] = NR_CPUS;
				} else
						change_rx_qm_cpumap(pAd, 0x3);
				} else
					change_rx_qm_cpumap(pAd, 0xd);
#endif
				if (chip_cap->multi_token_ques_per_band) {
					pAd->ixia_mode_ctl.max_data_que_num_backup = qm->max_data_que_num;
					pAd->ixia_mode_ctl.max_mgmt_que_num_backup = qm->max_mgmt_que_num;
					qm->max_mgmt_que_num = 512;
					qm->max_data_que_num = 6144;
				}

				enable_tx_burst(pAd, txopctl->cur_wdev, AC_BE,
						PRIO_MULTI_CLIENT, TXOP_80);
				mcli->cur_txop = TXOP_80;
				txopctl->multi_cli_txop_running = TRUE;
				pAd->ixia_mode_ctl.tx_tasklet_sch = TRUE;
				pAd->ixia_mode_ctl.rx_tasklet_sch = TRUE;
			}
			mcli->rps_state_flag &= ~MCLI_RPS_RESET_FLAG;
				mcli->rps_state_flag |= MCLI_IXIA_STA_DETECT_FLAG;
		}
	} else {
IXIA_STATE_RESET:
		for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
			mcli = &(pAd->mcli_ctl[band_idx]);
			txopctl = &(pAd->txop_ctl[band_idx]);

			if (mcli->rps_state_flag &
				MCLI_IXIA_STA_DETECT_FLAG) {
				if (txopctl->multi_cli_txop_running == TRUE) {
					disable_tx_burst(pAd, txopctl->cur_wdev, AC_BE,
						PRIO_MULTI_CLIENT, 0);
					mcli->cur_txop = 0;
					txopctl->multi_cli_txop_running = FALSE;
				}
				mcli->rps_state_flag |= MCLI_RPS_RESET_FLAG;
				mcli->rps_state_flag &= ~MCLI_IXIA_STA_DETECT_FLAG;
			}
		}
		pAd->ixia_mode_ctl.sta_nums = 0;
		if (stop_ixia_mode == TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: release stop_ixia_mode\n",__func__));
			stop_ixia_mode = FALSE;
			stop_ixia_mode_counter = 0;
		}
		return FALSE;
	}

	if (iMacflag && !iRssiflag) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, dbg_lvl[band_idx],
			("%s: ixia_mode: mac addr match, but rssi not match [delta=%d, max=%d min=%d]\n",
			__func__, deltaRSSI, MaxRssi, MinRssi));
	}
	/* return (iMacflag && iRssiflag); */
	return iMacflag;

}

static VOID rps_mode_config_setting(struct _RTMP_ADAPTER *pAd, UINT32 proc_rps_mode,
	UINT32 *the_apply_mask, UINT8 *the_band_idx)
{
	UINT32 apply_mask = 0, band_idx = DBDC_BAND0;
	BOOLEAN rps_force_cfg = FALSE;
	struct multi_cli_ctl *mcli = NULL;

	switch (proc_rps_mode) {
	case BAND0_DETECT_MODE:
		band_idx = DBDC_BAND0;
		break;
	case BAND1_DETECT_MODE:
		band_idx = DBDC_BAND1;
		break;
	case BAND0_DL_RPS_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_ETH0_RPS_FLAG|
			APPLY_BAND0_RPS_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);
		strncpy(proc_rps_cfg[BAND0_RPS_FILE], "c", 2);
		break;
	case BAND0_UDP_DL_RPS_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_ETH0_RPS_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);
		break;
	case BAND0_UDP_DL_TXFREE_IRQ_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_ETH0_RPS_FLAG|
				APPLY_IRQ_FLAG;
		if (pAd->ixia_mode_ctl.sta_nums >= 20) {
			strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "2", 2);
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "d", 2);
		} else {
			strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "1", 2);
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "c", 2);
		}
		break;
	case BAND0_1_PAIR_DL_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_ETH0_RPS_FLAG;
		proc_rps_apply[ETH0_RPS_FILE] = FALSE;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "8", 2);
		break;
	case BAND0_1_PAIR_UDP_DL_TX_FREE_IRQ_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_ETH0_RPS_FLAG |
				APPLY_IRQ_FLAG;
		proc_rps_apply[TXFREE_IRQ_FILE] = FALSE;
		proc_rps_apply[ETH0_RPS_FILE] = FALSE;
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "1", 2);
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "8", 2);
		break;
	case BAND0_1_PAIR_UL_MODE:
		band_idx = DBDC_BAND0;
		proc_rps_apply[ETH0_RPS_FILE] = FALSE;
		proc_rps_apply[BAND0_RPS_FILE] = FALSE;
		apply_mask = APPLY_ETH0_RPS_FLAG|APPLY_BAND0_RPS_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "8", 2);
		strncpy(proc_rps_cfg[BAND0_RPS_FILE], "0", 2);
		break;
	case BAND0_1_PAIR_UL_UDP_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_BAND0_RPS_FLAG;
		proc_rps_apply[BAND0_RPS_FILE] = FALSE;
		strncpy(proc_rps_cfg[BAND0_RPS_FILE], "8", 2);
		break;
	case BAND0_UL_RPS_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_ETH0_RPS_FLAG|APPLY_BAND0_RPS_FLAG;
		if (pAd->ixia_mode_ctl.sta_nums > 20)
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "f", 2);
		else
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);
		strncpy(proc_rps_cfg[BAND0_RPS_FILE], "b", 2);
		break;
	case BAND1_1_PAIR_UL_MODE:
		band_idx = DBDC_BAND1;
		proc_rps_apply[ETH0_RPS_FILE] = FALSE;
		proc_rps_apply[BAND1_RPS_FILE] = FALSE;
		proc_rps_apply[TXFREE_IRQ_FILE] = FALSE;
		apply_mask = APPLY_ETH0_RPS_FLAG|
			APPLY_BAND1_RPS_FLAG|
			APPLY_IRQ_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "4", 2);
		strncpy(proc_rps_cfg[BAND1_RPS_FILE], "4", 2);
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "1", 2);
		break;
	case BAND1_1_PAIR_L_PKT_UL_MODE:
		band_idx = DBDC_BAND1;
		proc_rps_apply[ETH0_RPS_FILE] = FALSE;
		proc_rps_apply[BAND1_RPS_FILE] = FALSE;
		apply_mask = APPLY_ETH0_RPS_FLAG|APPLY_BAND1_RPS_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "0", 2);
		strncpy(proc_rps_cfg[BAND1_RPS_FILE], "0", 2);
		break;
	case BAND1_1_PAIR_DL_MODE:
	case BAND1_1_PAIR_L_PKT_DL_MODE:
		band_idx = DBDC_BAND1;
		proc_rps_apply[ETH0_RPS_FILE] = FALSE;
		proc_rps_apply[TXFREE_IRQ_FILE] = FALSE;
		apply_mask = APPLY_ETH0_RPS_FLAG|APPLY_IRQ_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "4", 2);
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "1", 2);
		break;
	case BAND1_DL_RPS_MODE:
		band_idx = DBDC_BAND1;
		apply_mask = APPLY_ETH0_RPS_FLAG;
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);
		break;
	case BAND1_UDP_DL_RPS_MODE:
		band_idx = DBDC_BAND1;
		apply_mask = APPLY_ETH0_RPS_FLAG;
		if (pAd->ixia_mode_ctl.sta_nums > 20)
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "d", 2);
		else
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);
		break;
	case BAND1_UDP_DL_RPS_TXFREE_IRQ_MODE:
		band_idx = DBDC_BAND1;
		apply_mask = APPLY_ETH0_RPS_FLAG | APPLY_IRQ_FLAG;
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "2", 2);
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "d", 2);
		break;
	case BAND1_UL_RPS_MODE:
		band_idx = DBDC_BAND1;
		apply_mask = APPLY_ETH0_RPS_FLAG|APPLY_BAND1_RPS_FLAG;
		if (pAd->ixia_mode_ctl.sta_nums > 20)
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "7", 2);
		else
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);

		strncpy(proc_rps_cfg[BAND1_RPS_FILE], "0", 2);
		break;
	case BAND1_UL_TXFREE_IRQ_MODE:
		band_idx = DBDC_BAND1;
		apply_mask = APPLY_IRQ_FLAG|
			APPLY_BAND1_RPS_FLAG|
			APPLY_ETH0_RPS_FLAG;
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "2", 2);
		strncpy(proc_rps_cfg[ETH0_RPS_FILE], "9", 2);
		strncpy(proc_rps_cfg[BAND1_RPS_FILE], "5", 2);
		break;
	case BAND1_UDP_UL_RPS_MODE:
		band_idx = DBDC_BAND1;
		apply_mask = APPLY_BAND1_RPS_FLAG;
		if (pAd->ixia_mode_ctl.sta_nums >= 16)
			strncpy(proc_rps_cfg[BAND1_RPS_FILE], "7", 2);
		else
			strncpy(proc_rps_cfg[BAND1_RPS_FILE], "3", 2);
		break;
	case BAND1_UDP_UL_RPS_TXFREE_IRQ_MODE:
		band_idx = DBDC_BAND1;
		apply_mask =
			APPLY_BAND1_RPS_FLAG|
			APPLY_IRQ_FLAG;
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "2", 2);
		if (pAd->ixia_mode_ctl.sta_nums >= 16)
			strncpy(proc_rps_cfg[BAND1_RPS_FILE], "7", 2);
		else
			strncpy(proc_rps_cfg[BAND1_RPS_FILE], "3", 2);
		break;
	case BAND0_UL_RPS_TXFREE_IRQ_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_IRQ_FLAG|APPLY_ETH0_RPS_FLAG|
				APPLY_BAND0_RPS_FLAG;
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "1", 2);
		if (pAd->ixia_mode_ctl.sta_nums > 20) {
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "e", 2);
			strncpy(proc_rps_cfg[BAND0_RPS_FILE], "d", 2);
		} else {
			strncpy(proc_rps_cfg[ETH0_RPS_FILE], "e", 2);
			strncpy(proc_rps_cfg[BAND0_RPS_FILE], "c", 2);
		}
		break;
	case BAND0_UDP_UL_RPS_TXFREE_IRQ_MODE:
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_IRQ_FLAG|APPLY_BAND0_RPS_FLAG;
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE], "2", 2);
		strncpy(proc_rps_cfg[BAND0_RPS_FILE], "0", 2);
		break;
	case RESET_RPS_MODE:
		proc_rps_mode = NORMAL_RPS_MODE;
		band_idx = DBDC_BAND0;
		apply_mask = APPLY_RPS_RESET_FLAG;
		break;
	default:
		break;
	}

	if ((proc_rps_mode != NORMAL_RPS_MODE) &&
		(proc_rps_mode != RESET_RPS_MODE) &&
		(!(apply_mask & APPLY_QLEN_FLAG))) {
		apply_mask |= APPLY_QLEN_FLAG;
		strncpy(proc_rps_cfg[BACKLOG_QUEUE_FILE], "8000", 5);
		tx_queue_cfg[LAN_NET_DEV] = 7000;
		tx_queue_cfg[BAND0_NET_DEV] = 7000;
		tx_queue_cfg[BAND1_NET_DEV] = 7000;
	}

	if (pAd->mcli_ctl[DBDC_BAND0].force_rps_cfg == TRUE) {
		band_idx = DBDC_BAND0;
		rps_force_cfg = TRUE;
	}
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
	if (pAd->mcli_ctl[DBDC_BAND1].force_rps_cfg == TRUE) {
		band_idx = DBDC_BAND1;
		rps_force_cfg = TRUE;
	}
	}

#endif
	if (rps_force_cfg == TRUE) {
		apply_mask = APPLY_ETH0_RPS_FLAG|
			APPLY_BAND0_RPS_FLAG|
			APPLY_BAND1_RPS_FLAG|
			APPLY_IRQ_FLAG|
			APPLY_QLEN_FLAG;
		mcli = &pAd->mcli_ctl[band_idx];
		strncpy(proc_rps_cfg[ETH0_RPS_FILE],
			mcli->force_proc_rps[ETH0_RPS_FILE], 2);
		strncpy(proc_rps_cfg[BAND0_RPS_FILE],
			mcli->force_proc_rps[BAND0_RPS_FILE], 2);
		strncpy(proc_rps_cfg[BAND1_RPS_FILE],
			mcli->force_proc_rps[BAND1_RPS_FILE], 2);
		strncpy(proc_rps_cfg[TXFREE_IRQ_FILE],
			mcli->force_proc_rps[TXFREE_IRQ_FILE], 2);
		strncpy(proc_rps_cfg[BACKLOG_QUEUE_FILE], "6000", 5);
		tx_queue_cfg[LAN_NET_DEV] = 6000;
		tx_queue_cfg[BAND0_NET_DEV] = 6000;
		tx_queue_cfg[BAND1_NET_DEV] = 6000;
	}
	*(the_apply_mask) = apply_mask;
	*(the_band_idx) = band_idx;
}

static BOOLEAN dynamic_1_pair_proc_rps_adjust(struct _RTMP_ADAPTER *pAd)
{
	UINT8 band_idx = DBDC_BAND0, band_idx_apply = DBDC_BAND_NUM;
	static UINT8 last_band_idx_apply = DBDC_BAND_NUM;
	UINT32 dbg_lvl[DBDC_BAND_NUM] = {0}, proc_rps_mode = 0, apply_mask = 0;
	UINT32 level = TXOP_80,
		client_nums[DBDC_BAND_NUM] = {0},
		rx_client_nums[DBDC_BAND_NUM] = {0},
		last_client_num[DBDC_BAND_NUM] = {0},
		last_rx_client_num[DBDC_BAND_NUM] = {0};
	UINT32 peak_tx_clients[DBDC_BAND_NUM] = {0},
		peak_tx_pkts[DBDC_BAND_NUM] = {0},
		peak_rx_clients[DBDC_BAND_NUM] = {0},
		peak_rx_pkts[DBDC_BAND_NUM] = {0};
	BOOLEAN uplink[DBDC_BAND_NUM] = {FALSE},
		downlink[DBDC_BAND_NUM] = {FALSE};
	BOOLEAN ret = TRUE, rx_qm_on = FALSE;
	struct multi_cli_ctl *mcli = NULL;
	struct txop_ctl *txopctl = NULL;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	static INT32 reset_count = 1;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->mcli_ctl[band_idx].debug_on & MCLI_DEBUG_SINGLE_PAIR)
			dbg_lvl[band_idx] = DBG_LVL_OFF;
	else
			dbg_lvl[band_idx] = DBG_LVL_INFO;
	}

	if ((pAd->MacTab.Size == 0) || (pAd->MacTab.Size > 1)) {
		if (last_band_idx_apply < DBDC_BAND_NUM)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl[last_band_idx_apply],
				("%s:band%u goto RESET\n", __func__, last_band_idx_apply));
		goto CHECK_RESET;
	}

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		mcli = &(pAd->mcli_ctl[band_idx]);
		txopctl = &(pAd->txop_ctl[band_idx]);

		uplink[band_idx] = downlink[band_idx] = FALSE;
		peak_tx_clients[band_idx] = mcli->peak_tx_clients;
		peak_rx_clients[band_idx] = mcli->peak_rx_clients;
		peak_tx_pkts[band_idx] = mcli->peak_tx_pkts;
		peak_rx_pkts[band_idx] = mcli->peak_rx_pkts;
		client_nums[band_idx] = txopctl->multi_client_nums;
		rx_client_nums[band_idx] = txopctl->multi_rx_client_nums;
		last_client_num[band_idx] = txopctl->last_client_num;
		last_rx_client_num[band_idx] = txopctl->last_rx_client_num;
		txopctl->last_client_num = client_nums[band_idx];
		txopctl->last_rx_client_num = rx_client_nums[band_idx];
		mcli->peak_tx_clients = 0;
		mcli->peak_rx_clients = 0;
		mcli->peak_tx_pkts = 0;
		mcli->peak_rx_pkts = 0;


		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl[band_idx],
		("%s(): band%d peak_tx:%u,peak_rx:%u,peak_tx_pkts:%u, peak_rx_pkts:%u, pkt_len:%u, pkt_rx_len:%u\n",
			__func__, band_idx,
			peak_tx_clients[band_idx],
			peak_rx_clients[band_idx],
			peak_tx_pkts[band_idx], peak_rx_pkts[band_idx],
			mcli->pkt_avg_len,
			mcli->pkt_rx_avg_len));

		if ((peak_rx_clients[band_idx] == 1) && (mcli->rx_sta_nums == 1)
			&& (mcli->pkt_avg_len < 64)) {
			uplink[band_idx] = TRUE;
			band_idx_apply = band_idx;
		}
		if ((peak_tx_clients[band_idx] == 1) && (mcli->sta_nums == 1)
			&& (peak_rx_clients[band_idx] == 0)) {
			downlink[band_idx] = TRUE;
			band_idx_apply = band_idx;
		}
	}

CHECK_RESET:
	if (band_idx_apply == DBDC_BAND_NUM) {

		band_idx = last_band_idx_apply;

		if (last_band_idx_apply == DBDC_BAND_NUM)
			return FALSE;

		mcli = &(pAd->mcli_ctl[band_idx]);
		proc_rps_mode = mcli->proc_rps_mode;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl[band_idx], ("%s():band%d proc_rps_mode:%u\n",
			__func__, band_idx, proc_rps_mode));

		if ((proc_rps_mode > MIN_1_PAIR_RPS_MODE) &&
			(proc_rps_mode < MAX_1_PAIR_RPS_MODE)) {
			if (reset_count > 10) {
				reset_count = 1;
				rx_qm_on = FALSE;
				level = TXOP_BB;
				proc_rps_mode = RESET_RPS_MODE;
				ret = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl[band_idx],
				("%s(): band%d peak_tx:%u,peak_rx:%u,peak_tx_pkts:%u, peak_rx_pkts:%u, pkt_len:%u, pkt_rx_len:%u,sta:%u,rx_sta:%u\n",
				__func__, band_idx, peak_tx_clients[band_idx], peak_rx_clients[band_idx],
				peak_tx_pkts[band_idx], peak_rx_pkts[band_idx],
				mcli->pkt_avg_len, mcli->pkt_rx_avg_len,
				mcli->sta_nums, mcli->rx_sta_nums));
				goto APPLY_RPS;
			} else
				reset_count++;

			pAd->ixia_mode_ctl.mode_entered = TRUE;
			return TRUE;
		}

		return FALSE;
	}

	band_idx = band_idx_apply;

	if ((uplink[band_idx]) && (downlink[band_idx]))
		return FALSE;

	if (pAd->mcli_ctl[band_idx].kernel_rps_adjust_enable == FALSE)
		return FALSE;

	if (pAd->ixia_mode_ctl.num_entry == 0) {
		apply_proc_rps_setting(pAd, APPLY_ALL_ENTRY_FLAG, band_idx);
		return TRUE;
	}

	if (pAd->ixia_mode_ctl.num_entry > 1)
		return FALSE;

	mcli = &(pAd->mcli_ctl[band_idx]);

	if (uplink[band_idx]) {
		reset_count = 1;
		proc_rps_mode = mcli->proc_rps_mode;
		if (band_idx == DBDC_BAND0) {
			if ((proc_rps_mode != BAND0_1_PAIR_UL_MODE) &&
				(proc_rps_mode != BAND0_1_PAIR_UL_UDP_MODE) &&
				(mcli->pkt_rx_avg_len < 1280)) {
				rx_qm_on = TRUE;
				if (mcli->pkt_avg_len < 32)
					proc_rps_mode = BAND0_1_PAIR_UL_UDP_MODE;
				else
					proc_rps_mode = BAND0_1_PAIR_UL_MODE;

				level = TXOP_80;
				pAd->ixia_mode_ctl.mode_entered = TRUE;
				ret = TRUE;
				last_band_idx_apply = band_idx_apply;
				goto APPLY_RPS;
			}
			if ((proc_rps_mode == BAND0_1_PAIR_UL_MODE) ||
				(proc_rps_mode == BAND0_1_PAIR_UL_UDP_MODE)) {
				return TRUE;
			}
		}
		if (band_idx == DBDC_BAND1) {
			if ((proc_rps_mode != BAND1_1_PAIR_UL_MODE) &&
			(proc_rps_mode != BAND1_1_PAIR_L_PKT_UL_MODE) &&
			(mcli->pkt_rx_avg_len < 1280)) {
				proc_rps_mode = BAND1_1_PAIR_UL_MODE;
				pAd->ixia_mode_ctl.mode_entered = TRUE;
				rx_qm_on = TRUE;
				level = TXOP_80;
				ret = TRUE;
				last_band_idx_apply = band_idx_apply;
				goto APPLY_RPS;
			}
			/*
			if (proc_rps_mode == BAND1_1_PAIR_UL_MODE) {
				if (mcli->pkt_rx_avg_len >= (1024 >> 1)) {
					proc_rps_mode = BAND1_1_PAIR_L_PKT_UL_MODE;
					rx_qm_on = TRUE;
					level = TXOP_80;
					ret = TRUE;
					last_band_idx_apply = band_idx_apply;
					goto APPLY_RPS;
				}
				return TRUE;
			}
			*/
			if ((proc_rps_mode == BAND1_1_PAIR_UL_MODE) ||
				(proc_rps_mode == BAND1_1_PAIR_L_PKT_UL_MODE))
				return TRUE;
		}
		proc_rps_mode = 0;
	}

	if (downlink[band_idx]) {
		reset_count = 1;
		proc_rps_mode = mcli->proc_rps_mode;
		if (band_idx == DBDC_BAND0) {
			if ((proc_rps_mode != BAND0_1_PAIR_DL_MODE) &&
				(proc_rps_mode != BAND0_1_PAIR_UDP_DL_TX_FREE_IRQ_MODE) &&
				(mcli->pkt_avg_len < 1280)) {
				if (mcli->pkt_rx_avg_len > 16)
				proc_rps_mode = BAND0_1_PAIR_DL_MODE;
				else
					proc_rps_mode = BAND0_1_PAIR_UDP_DL_TX_FREE_IRQ_MODE;
				rx_qm_on = FALSE;
				level = TXOP_80;
				ret = TRUE;
				last_band_idx_apply = band_idx_apply;
				goto APPLY_RPS;
			}
			if ((proc_rps_mode == BAND0_1_PAIR_DL_MODE) ||
				(proc_rps_mode == BAND0_1_PAIR_UDP_DL_TX_FREE_IRQ_MODE))
				return TRUE;
		}
		if (band_idx == DBDC_BAND1) {
			if ((proc_rps_mode != BAND1_1_PAIR_DL_MODE) &&
				(proc_rps_mode != BAND1_1_PAIR_L_PKT_DL_MODE) &&
				(mcli->pkt_avg_len < 1280)) {
				proc_rps_mode = BAND1_1_PAIR_DL_MODE;
				rx_qm_on = FALSE;
				if (mcli->pkt_rx_avg_len > 32)
					level = TXOP_60;
				else
				level = TXOP_80;
				ret = TRUE;
				last_band_idx_apply = band_idx_apply;
				goto APPLY_RPS;
			}
			if (proc_rps_mode == BAND1_1_PAIR_DL_MODE) {
				if (mcli->pkt_avg_len >= 512) {
					proc_rps_mode =
						BAND1_1_PAIR_L_PKT_DL_MODE;
					rx_qm_on = FALSE;
					if (mcli->pkt_rx_avg_len > 32)
						level = 0x48;
					else
						level = TXOP_80;
					ret = TRUE;
					last_band_idx_apply = band_idx_apply;
					goto APPLY_RPS;
				}
				return TRUE;
			}

			if ((proc_rps_mode == BAND1_1_PAIR_DL_MODE) ||
				(proc_rps_mode == BAND1_1_PAIR_L_PKT_DL_MODE))
				return TRUE;
		}
	}

	return FALSE;

APPLY_RPS:
	if (proc_rps_mode == RESET_RPS_MODE) {
		band_idx_apply = last_band_idx_apply;
		last_band_idx_apply = DBDC_BAND_NUM;
	} else
		strncpy(proc_rps_cfg[RCU_STALL_TIMEOUT_FILE], "60", 3);

#ifdef RX_RPS_SUPPORT
	change_rx_tasklet_method(pAd, rx_qm_on);
	change_rx_qm_cpumap(pAd, (rx_qm_on ? 1 : 0));
#endif
	rps_mode_config_setting(pAd, proc_rps_mode, &apply_mask, &band_idx_apply);

	if (proc_rps_mode != RESET_RPS_MODE) {
		mcli = &pAd->mcli_ctl[band_idx_apply];
		txopctl = &pAd->txop_ctl[band_idx_apply];
		apply_mask |= APPLY_RCU_TIMEOUT_FLAG;

		if (mcli->cur_txop != level) {
			enable_tx_burst(pAd, txopctl->cur_wdev,
				AC_BE, PRIO_MULTI_CLIENT, level);
			mcli->cur_txop = level;
		}
		txopctl->multi_cli_txop_running = TRUE;

		if (mcli->force_tx_process_cnt > 0)
			qm->max_tx_process_cnt = mcli->force_tx_process_cnt;
		pAd->ixia_mode_ctl.mode_entered = TRUE;
		pAd->ixia_mode_ctl.sta_nums = 1;

		if (chip_cap->multi_token_ques_per_band) {
			pAd->ixia_mode_ctl.max_data_que_num_backup = qm->max_data_que_num;
			pAd->ixia_mode_ctl.max_mgmt_que_num_backup = qm->max_mgmt_que_num;
			qm->max_mgmt_que_num = 512;
			qm->max_data_que_num = 6144;
		}
/*
		strncpy(proc_rps_cfg[HNAT_HOOK_TOGGLE_FILE], "0", 1);
		apply_mask |= APPLY_HOOK_TOGGLE_FLAG;
*/
	}

	if (apply_proc_rps_setting(pAd, apply_mask, band_idx_apply))
		pAd->mcli_ctl[band_idx_apply].proc_rps_mode = proc_rps_mode;

	return ret;
}

static VOID dynamic_mcli_proc_rps_adjust(struct _RTMP_ADAPTER *pAd,
	UINT32 *the_proc_rps_mode,
	UINT8 *the_band_idx_apply)
{
	UINT8 band_idx = DBDC_BAND0, band_idx_apply = DBDC_BAND0;
	UINT32 last_rx_num = 0, last_tx_num = 0, cur_tx_num = 0,
		cur_rx_num = 0, proc_rps_mode = 0, level = TXOP_80;
	struct multi_cli_ctl *mcli = NULL;
	struct txop_ctl *txopctl = NULL;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	UINT32 dbg_lvl[DBDC_BAND_NUM] = {0};

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		if (pAd->mcli_ctl[band_idx].debug_on & MCLI_DEBUG_MULTICLIENT)
			dbg_lvl[band_idx] = DBG_LVL_OFF;
		else
			dbg_lvl[band_idx] = DBG_LVL_INFO;
	}

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		txopctl = &(pAd->txop_ctl[band_idx]);

		last_tx_num = txopctl->last_client_num;
		last_rx_num = txopctl->last_rx_client_num;
		cur_tx_num = txopctl->multi_client_nums;
		cur_rx_num = txopctl->multi_rx_client_nums;
		txopctl->last_client_num = cur_tx_num;
		txopctl->last_rx_client_num = cur_rx_num;

		mcli = &(pAd->mcli_ctl[band_idx]);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl[band_idx],
			("%s():band%u cur_tx:%u,cur_rx:%u,bidir:%u,pkt_tx_len:%u,pkt_rx_len:%u(tx:%lu,rx:%lu)\n", __func__,
				band_idx, cur_tx_num, cur_rx_num,
				mcli->is_bidir,
				mcli->pkt_avg_len,
				mcli->pkt_rx_avg_len,
				mcli->tot_tx_pkts,
				mcli->tot_rx_pkts));

		if ((cur_tx_num == 0) && (cur_rx_num == 0)) {
			if ((last_tx_num == 0) && (last_rx_num == 0)) {
				mcli->silent_period_cnt++;
				goto LOOP_END;
			}
		}
		mcli->silent_period_cnt = 0;
		if (MTK_REV_ET(pAd, MT7915, MT7915E1) ||
			MTK_REV_ET(pAd, MT7915, MT7915E2) ||
			IS_MT7986(pAd) || IS_MT7916(pAd) || IS_MT7981(pAd)) {
			UINT32 wmm_idx;
			EDCA_PARM *edcaparam = NULL;

			if ((mcli->is_bidir == TRUE) && (mcli->adjust_backoff == FALSE)) {
				wmm_idx = HcGetWmmIdx(pAd, txopctl->cur_wdev);
				edcaparam = HcGetEdca(pAd, txopctl->cur_wdev);
				if (edcaparam)
					os_move_mem(&mcli->edcaparam_backup, edcaparam, sizeof(EDCA_PARM));

				HW_SET_PART_WMM_PARAM(pAd, txopctl->cur_wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMAX, 3);
				HW_SET_PART_WMM_PARAM(pAd, txopctl->cur_wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMIN, 2);
				mcli->adjust_backoff = TRUE;
				goto LOOP_END;
			}
		}
		if ((cur_tx_num >= 5) && (cur_rx_num >= 5) && (mcli->pkt_avg_len > 80) &&
			(mcli->pkt_rx_avg_len > 80)) {
				/* bi-direction case , no change */
				if (mcli->rps_adjust == TRUE)
					mcli->rps_state_flag |= MCLI_RPS_RESET_FLAG;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s bi-dir [tx:%d,rx:%d]\n", __func__, band_idx, mcli->rps_state_flag));

			goto LOOP_END;
		}

		if (band_idx == DBDC_BAND1) {
			if ((cur_tx_num >= 3) &&
				(mcli->pkt_rx_avg_len <= 64)) {
				/* TCP/UDP short packet DL case */
				if (mcli->pkt_rx_avg_len > 16) {
					proc_rps_mode = BAND1_DL_RPS_MODE;
					qm->max_tx_process_cnt = 4096;
					level = TXOP_BB;
				} else {
					if (pAd->ixia_mode_ctl.sta_nums >= 20)
						proc_rps_mode = BAND1_UDP_DL_RPS_TXFREE_IRQ_MODE;
					else
						proc_rps_mode = BAND1_UDP_DL_RPS_MODE;
					level = TXOP_BB;
				}

				band_idx_apply = DBDC_BAND1;

				if (mcli->cur_txop != level) {
					enable_tx_burst(pAd,
						txopctl->cur_wdev,
						AC_BE,
						PRIO_MULTI_CLIENT, level);
					mcli->cur_txop = level;
					txopctl->multi_cli_txop_running = TRUE;
				}
			} else if ((cur_rx_num >= 3) && (mcli->pkt_avg_len <= 64)
				) {
				/* TCP/UDP short packet UL case */
				if (mcli->pkt_avg_len > 32) {
					if (pAd->ixia_mode_ctl.sta_nums >= 20) {
						proc_rps_mode = BAND1_UL_RPS_MODE;
						if (mcli->pkt_rx_avg_len < ((128+256)>>1))
							qm->max_tx_process_cnt = 2048;
						else
							qm->max_tx_process_cnt = 8192;
					} else {
						if (mcli->pkt_rx_avg_len < ((128+256)>>1))
							qm->max_tx_process_cnt = 2048;
						else
							qm->max_tx_process_cnt = 8192;
						proc_rps_mode = BAND1_UL_RPS_MODE;
					}
				} else
					proc_rps_mode =
						BAND1_UDP_UL_RPS_TXFREE_IRQ_MODE;
				band_idx_apply = DBDC_BAND1;
			}
		} else {
			if ((cur_rx_num >= 3) &&
				(mcli->pkt_avg_len <= 64)) {
				/* TCP/UDP short packet UL case */
				band_idx_apply = DBDC_BAND0;
				if (mcli->pkt_avg_len > 32) {
					if (pAd->ixia_mode_ctl.sta_nums > 20)
						proc_rps_mode = BAND0_UL_RPS_MODE;
					else
						proc_rps_mode = BAND0_UL_RPS_TXFREE_IRQ_MODE;
				} else {
					proc_rps_mode =
						BAND0_UDP_UL_RPS_TXFREE_IRQ_MODE;
				}
				break;
			} else if ((cur_tx_num >= 3) &&
					(mcli->pkt_rx_avg_len <= 64)) {
					band_idx_apply = DBDC_BAND0;
					if (mcli->pkt_rx_avg_len > 32) {
						proc_rps_mode = BAND0_DL_RPS_MODE;

						if (pAd->ixia_mode_ctl.sta_nums < 20)
							qm->max_tx_process_cnt = 2048;
						break;
					} else {
						if (pAd->ixia_mode_ctl.sta_nums <= 20) {
							proc_rps_mode =
								BAND0_UDP_DL_RPS_MODE;
							if (mcli->pkt_avg_len < 128)
								qm->max_tx_process_cnt = 2048;
							else
								qm->max_tx_process_cnt = 6144;
						} else {
							proc_rps_mode = BAND0_UDP_DL_TXFREE_IRQ_MODE;
							if (mcli->pkt_avg_len < 128)
								qm->max_tx_process_cnt = 2048;
							else
								qm->max_tx_process_cnt = 6144;
						}
					}

					level = TXOP_80;
					if (mcli->pkt_avg_len > 512)
						level = TXOP_BB;
					if (mcli->cur_txop != level) {
						enable_tx_burst(pAd,
						txopctl->cur_wdev,
						AC_BE,
						PRIO_MULTI_CLIENT, level);
						mcli->cur_txop = level;
						txopctl->multi_cli_txop_running = TRUE;
					}
					break;
			}
		}
LOOP_END:
		if (mcli->silent_period_cnt >= 480)
			mcli->silent_period_cnt = 0;
	}

	if (band_idx_apply < DBDC_BAND_NUM) {
		mcli = &pAd->mcli_ctl[band_idx_apply];
		if (mcli->force_tx_process_cnt > 0)
			qm->max_tx_process_cnt = mcli->force_tx_process_cnt;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl[band_idx_apply],
			("%s():band%u mode:%u\n", __func__,
			band_idx_apply, proc_rps_mode));
	}

	*(the_band_idx_apply) = band_idx_apply;
	*(the_proc_rps_mode) = proc_rps_mode;

	return;
}

void dynamic_proc_rps_adjust(struct _RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;
	UINT32 rps_state_flag = 0;
	UINT8 band_idx = DBDC_BAND0, band_idx_apply = DBDC_BAND0;
	INT32 i = 0;
	UINT32 proc_rps_mode = 0, apply_mask = 0, dbg_lvl = 0, silent_thd = 30;
	BOOLEAN is_ixia_mode = FALSE, rps_apply = FALSE, rps_reset = FALSE;
	static INT32 reset_count = 1;
	struct fp_qm *qm = (struct fp_qm *)pAd->qm;
	struct multi_cli_ctl *mcli = NULL;
	struct txop_ctl *txopctl = NULL;
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);

	proc_rps_mode = 0;

	if (dynamic_1_pair_proc_rps_adjust(pAd))
		return;

	if ((pAd->mcli_ctl[DBDC_BAND0].debug_on & MCLI_DEBUG_RPS_CFG_MODE)
#ifdef DBDC_MODE
	||
	((pAd->CommonCfg.dbdc_mode) &&
		(pAd->mcli_ctl[DBDC_BAND1].debug_on & MCLI_DEBUG_RPS_CFG_MODE))
#endif
		)
		dbg_lvl = DBG_LVL_OFF;
	else
		dbg_lvl = DBG_LVL_WARN;

	is_ixia_mode = periodic_detect_ixia_mode(pAd);
	if (is_ixia_mode == TRUE) {
		pAd->ixia_mode_ctl.isIxiaOn = TRUE;
		goto IS_IXIA_MODE;
	} else
		pAd->ixia_mode_ctl.isIxiaOn = FALSE;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		rps_state_flag |= pAd->mcli_ctl[band_idx].rps_state_flag;
		rps_apply = rps_apply || pAd->mcli_ctl[band_idx].rps_adjust;
	}

IXIA_MODE_RESET:
	if ((rps_state_flag & MCLI_RPS_RESET_FLAG) && rps_apply) {
		if (reset_count > 10) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, dbg_lvl,
				("leave ixia_mode goto MCLI_RPS_RESET\n"));
			reset_count = 1;
			rps_reset = TRUE;
			goto MCLI_RPS_RESET;
		} else {
			reset_count++;
			return;
		}
	} else
		return;

IS_IXIA_MODE:
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		mcli = &(pAd->mcli_ctl[band_idx]);
		if (((mcli->tot_tx_pkts < 8000) && (mcli->tot_rx_pkts < 8000))
			&& (mcli->proc_rps_mode > MIN_RPS_MODE))
			stop_ixia_mode_counter = 0;
	}

	stop_ixia_mode_counter++;
	if (stop_ixia_mode_counter >= 300) {

		stop_ixia_mode_counter = 0;

		stop_ixia_mode = TRUE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("reset ixia to normal\n"));
		rps_state_flag |= MCLI_RPS_RESET_FLAG;
		rps_apply = TRUE;
		reset_count = 11;
		goto IXIA_MODE_RESET;

	}
	reset_count = 1;
	pAd->ixia_mode_ctl.mode_entered = TRUE;
	/*
	detect_cca_abort(pAd);
	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		mcli = &(pAd->mcli_ctl[band_idx]);
		if (mcli->cca_abort_cnt > 100) {
			mcli->is_bidir = TRUE;
		}
		else
			mcli->is_bidir = FALSE;
	}
	*/
	dynamic_mcli_proc_rps_adjust(pAd, &proc_rps_mode, &band_idx_apply);
	rps_state_flag = pAd->mcli_ctl[DBDC_BAND0].rps_state_flag;
#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode)
		rps_state_flag |= pAd->mcli_ctl[DBDC_BAND1].rps_state_flag;
#endif
MCLI_RPS_RESET:
	if (pAd->MacTab.Size >= 64)
		silent_thd = 60*5;
	else
		silent_thd = 30;

	if (((pAd->mcli_ctl[DBDC_BAND0].silent_period_cnt >= silent_thd)
#ifdef DBDC_MODE
		&& ((pAd->CommonCfg.dbdc_mode) && (pAd->mcli_ctl[DBDC_BAND1].silent_period_cnt >= silent_thd))

#endif
		)
		|| (!(rps_state_flag & MCLI_IXIA_STA_DETECT_FLAG))
		|| (rps_state_flag & MCLI_RPS_RESET_FLAG) || rps_reset) {
#ifdef DBDC_MODE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
			("%s(): band0 silent:%u, band1 silent:%u st_fg:%x reset:%u\n", __func__,
				pAd->mcli_ctl[DBDC_BAND0].silent_period_cnt,
				pAd->mcli_ctl[DBDC_BAND1].silent_period_cnt,
				rps_state_flag,
				rps_reset));
#else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
			("%s(): band0 silent:%u, st_fg:%x reset:%u\n", __func__,
				pAd->mcli_ctl[DBDC_BAND0].silent_period_cnt,
				rps_state_flag,
				rps_reset));
#endif
				rps_reset = TRUE;
				proc_rps_mode = RESET_RPS_MODE;

		for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
			mcli = &(pAd->mcli_ctl[band_idx]);
			txopctl = &(pAd->txop_ctl[band_idx]);

			if ((mcli->silent_period_cnt < silent_thd) &&
				(rps_state_flag & MCLI_IXIA_STA_DETECT_FLAG))
					continue;

			wdev = txopctl->cur_wdev;

			if ((mcli->c2s_only == TRUE) && (wdev)) {
				asic_rts_on_off(wdev, TRUE);
				mcli->c2s_only = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, dbg_lvl,
					("%s(): enable band%d RTS\n",  __func__, band_idx));
			}

			if (mcli->cur_agglimit) {
				asic_set_agglimit(pAd, band_idx, WMM_AC_BE,
					txopctl->cur_wdev, 0);
				mcli->cur_agglimit = 0;
			}

			if ((!(rps_state_flag & MCLI_IXIA_STA_DETECT_FLAG)) &&
				(txopctl->multi_cli_txop_running == TRUE)) {
				disable_tx_burst(pAd, txopctl->cur_wdev, AC_BE,
					PRIO_MULTI_CLIENT, 0);
				mcli->cur_txop = 0;
				txopctl->multi_cli_txop_running = FALSE;
			}

			if (mcli->adjust_backoff) {
				UINT32 wmm_idx;
				mcli->adjust_backoff = FALSE;
				wmm_idx = HcGetWmmIdx(pAd, txopctl->cur_wdev);
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMAX,
					mcli->edcaparam_backup.Cwmax[WMM_PARAM_AC_1]);
				HW_SET_PART_WMM_PARAM(pAd, wdev, wmm_idx, WMM_AC_BE, WMM_PARAM_CWMIN,
					mcli->edcaparam_backup.Cwmin[WMM_PARAM_AC_1]);

			}

			if (pAd->ixia_mode_ctl.max_data_que_num_backup > 0) {
				qm->max_data_que_num = pAd->ixia_mode_ctl.max_data_que_num_backup;
				qm->max_mgmt_que_num = pAd->ixia_mode_ctl.max_mgmt_que_num_backup;
				chip_cap->multi_token_ques_per_band = TRUE;
				pAd->ixia_mode_ctl.max_mgmt_que_num_backup = 0;
				pAd->ixia_mode_ctl.max_data_que_num_backup = 0;
			}
		}

#ifdef RX_RPS_SUPPORT
		if (!(rps_state_flag & MCLI_IXIA_STA_DETECT_FLAG))
			change_rx_tasklet_method(pAd, FALSE);
#endif
		pAd->mcli_ctl[DBDC_BAND0].silent_period_cnt = 0;
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			pAd->mcli_ctl[DBDC_BAND1].silent_period_cnt = 0;
#endif
		qm->max_tx_process_cnt = 8192;
		pAd->ixia_mode_ctl.tx_tasklet_sch = FALSE;
		pAd->ixia_mode_ctl.rx_tasklet_sch = FALSE;
		rps_state_flag = 0;
	}

	for (i = 1; i < MAX_PROC_RPS_MODE; i++) {
		if ((i == proc_rps_mode) && (rps_reset == FALSE))
			proc_rps_mode_cnt[i]++;
		else
			proc_rps_mode_cnt[i] = 0;
	}

	if ((proc_rps_mode_cnt[proc_rps_mode] < 1) && (rps_reset == FALSE))
		return;

	if ((proc_rps_mode > MIN_RPS_MODE) && (rps_reset == FALSE) &&
		(band_idx_apply < DBDC_BAND_NUM)) {
		if (pAd->mcli_ctl[band_idx_apply].proc_rps_mode > MIN_RPS_MODE)
			return;
	}

	rps_mode_config_setting(pAd, proc_rps_mode, &apply_mask, &band_idx_apply);

	if (apply_proc_rps_setting(pAd, apply_mask, band_idx_apply)) {
		if (rps_reset) {
			for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++)
				pAd->mcli_ctl[band_idx].proc_rps_mode = 0;
		} else
		pAd->mcli_ctl[band_idx_apply].proc_rps_mode = proc_rps_mode;
}
}

void detect_1_pair_peak(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pEntry, UCHAR BandIdx)
{
	UINT32 peak_hi_thd, peak_lo_thd, dbg_lvl;

	if (pAd->mcli_ctl[BandIdx].debug_on & MCLI_DEBUG_SINGLE_PAIR)
		dbg_lvl = DBG_LVL_OFF;
	else
		dbg_lvl = DBG_LVL_INFO;

	if (WMODE_CAP_5G(pEntry->wdev->PhyMode)) {
		peak_hi_thd = 150000;
		peak_lo_thd = 19000;
	} else {
		peak_hi_thd = 100000;
		peak_lo_thd = 9000;
	}

	if ((pEntry->avg_rx_pkts < peak_hi_thd) && (pEntry->avg_rx_pkts > peak_lo_thd))
		pAd->mcli_ctl[BandIdx].peak_rx_clients++;
	if ((pEntry->avg_tx_pkts < peak_hi_thd) && (pEntry->avg_tx_pkts > peak_lo_thd))
		pAd->mcli_ctl[BandIdx].peak_tx_clients++;
	if (pAd->mcli_ctl[BandIdx].peak_tx_pkts <  pEntry->avg_tx_pkts)
		pAd->mcli_ctl[BandIdx].peak_tx_pkts = pEntry->avg_tx_pkts;
	if (pAd->mcli_ctl[BandIdx].peak_rx_pkts <  pEntry->avg_rx_pkts)
		pAd->mcli_ctl[BandIdx].peak_rx_pkts = pEntry->avg_rx_pkts;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, dbg_lvl,
		("%s():WCID:%d MODE:%x\n", __func__, pEntry->wcid, pEntry->MaxHTPhyMode.field.MODE));

	if (pEntry->MaxHTPhyMode.field.MODE == MODE_HE) {
		pAd->mcli_ctl[BandIdx].peak_rx_clients = 0;
		pAd->mcli_ctl[BandIdx].peak_tx_clients = 0;
		return;
	}
}
/*
void detect_cca_abort(RTMP_ADAPTER *pAd)
{
	struct multi_cli_ctl *mcli = NULL;
	UINT8 band_idx;
	UINT32 val;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {
		MAC_IO_READ32(pAd->hdev_ctrl, 0x820ED4A0+band_idx*0x10000, &val);
		val &= 0x0FFFF;
		mcli = &(pAd->mcli_ctl[band_idx]);
		mcli->cca_abort_cnt = val - mcli->last_cca_abort_val;
	}

}
*/
void proc_rps_file_open(RTMP_ADAPTER *pAd)
{
	BOOLEAN ret = TRUE;
	INT32 idx, ntry;

	if (!IS_MT7915(pAd) && !IS_MT7986(pAd) && !IS_MT7916(pAd) && !IS_MT7981(pAd)) {
		for (idx = 0; idx < DBDC_BAND_NUM; idx++) {
			pAd->mcli_ctl[idx].tx_cnt_from_red = FALSE;
			pAd->mcli_ctl[idx].kernel_rps_adjust_enable = FALSE;
		}
		return;
	}

	os_zero_mem(proc_rps_cfg, MAX_PROC_RPS_FILE*sizeof(char)*8);
	os_zero_mem(proc_rps_apply, MAX_PROC_RPS_FILE*sizeof(BOOLEAN));
	os_zero_mem(proc_rps_mode_cnt, MAX_PROC_RPS_MODE*sizeof(UINT32));
	os_zero_mem(&proc_osFSInfo, sizeof(RTMP_OS_FS_INFO));

	RtmpOSFSInfoChange(&proc_osFSInfo, TRUE);

	for (idx = 0; idx < MAX_KERNEL_PROC_RPS_FILE; idx++) {
		ntry = 5;
		if (proc_rps_path[idx] == NULL)
			continue;
RETRY_OPEN_RPS_FILE:
		proc_srcf[idx] = RtmpOSFileOpen((char *)proc_rps_path[idx],
			O_RDWR|O_TRUNC, 0);
		if (IS_FILE_OPEN_ERR(proc_srcf[idx])) {
			proc_srcf[idx] = NULL;
			ntry--;
			if (ntry > 0)
				goto RETRY_OPEN_RPS_FILE;
			else {
				ret = FALSE;
				goto EXIT;
			}
		}

		if (ntry > 0)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s open proc file[%s] OK\n", __func__,
				proc_rps_path[idx]));
	}

	for (idx = 0; idx < MAX_NET_DEV; idx++) {
		if (proc_net_dev_name[idx] == NULL)
			continue;
		proc_net_dev[idx] = dev_get_by_name(&init_net, proc_net_dev_name[idx]);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s get %s net_dev=%p\n", __func__,
			proc_net_dev_name[idx],
			proc_net_dev[idx]));
		tx_queue_cfg[idx] = DEFAULT_NET_DEV_TX_QLEN;
	}

EXIT:
	for (idx = 0; idx < DBDC_BAND_NUM; idx++) {
		pAd->mcli_ctl[idx].tx_cnt_from_red = ret;
		pAd->mcli_ctl[idx].kernel_rps_adjust_enable = ret;
	}

	if (ret == TRUE) {
		/*
		if (MTK_REV_ET(pAd, MT7915, MT7915E1) ||
			MTK_REV_ET(pAd, MT7915, MT7915E2)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,("[%d]Set CCA DBG\n", __func__));
			MAC_IO_WRITE32(pAd->hdev_ctrl, 0x820ED4B0, 0xFEFDFCA1);
			MAC_IO_WRITE32(pAd->hdev_ctrl, 0x820FD4B0, 0xFEFDFCA1);
		}
		*/
		RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
		return;
	}

	for (idx = 0; idx < MAX_KERNEL_PROC_RPS_FILE; idx++) {
		if (IS_FILE_OPEN_ERR(proc_srcf[idx]) ||
			(proc_srcf[idx] == NULL))
			continue;
		RtmpOSFileSeek(proc_srcf[idx], 0);
		RtmpOSFileClose(proc_srcf[idx]);
		proc_srcf[idx] = NULL;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s close proc file[%s] OK\n", __func__,  proc_rps_path[idx]));
	}

	RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
}

void proc_rps_file_close(RTMP_ADAPTER *pAd)
{
	INT32 idx;

	if (!IS_MT7915(pAd) && !IS_MT7986(pAd) && !IS_MT7916(pAd) && !IS_MT7981(pAd))
		return;

	for (idx = 0; idx < DBDC_BAND_NUM; idx++) {
		pAd->mcli_ctl[idx].tx_cnt_from_red = FALSE;
		pAd->mcli_ctl[idx].kernel_rps_adjust_enable = FALSE;
	}

	for (idx = 0; idx < MAX_NET_DEV; idx++) {
		if (proc_net_dev_name[idx] == NULL)
			continue;
		if (proc_net_dev[idx])
			RtmpOSNetDeviceRefPut(proc_net_dev[idx]);
	}

	RtmpOSFSInfoChange(&proc_osFSInfo, TRUE);

	for (idx = 0; idx < MAX_PROC_RPS_FILE; idx++) {
		if (!IS_FILE_OPEN_ERR(proc_srcf[idx])) {
			RtmpOSFileSeek(proc_srcf[idx], 0);
			RtmpOSFileClose(proc_srcf[idx]);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s close proc file[%s] OK\n", __func__,  proc_rps_path[idx]));
			proc_srcf[idx] = NULL;
		}
	}

	RtmpOSFSInfoChange(&proc_osFSInfo, FALSE);
	/*
	if (MTK_REV_ET(pAd, MT7915, MT7915E1) ||
		MTK_REV_ET(pAd, MT7915, MT7915E2)) {

		MAC_IO_WRITE32(pAd->hdev_ctrl, 0x820ED4B0, 0x0);
		MAC_IO_WRITE32(pAd->hdev_ctrl, 0x820FD4B0, 0x0);
	}
	*/

}
#endif /* KERNEL_RPS_ADJUST */
