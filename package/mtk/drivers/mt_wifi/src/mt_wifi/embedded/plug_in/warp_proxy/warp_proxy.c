#include <linux/init.h>
#include <linux/module.h>
#include "warp_proxy.h"
#include <rtmp_def.h>
#include <warp.h>

#define DRIVER_DESC "Proxy agent between wifi client card & warp "
#define proxy_print(fmt, args...) pr_info(fmt, ## args)

extern void mt7615_chip_specific_get(struct wifi_hw *hw);
extern void mt7915_chip_specific_get(struct wifi_hw *hw);
extern void mt7986_chip_specific_get(struct wifi_hw *hw);
extern void mt7916_chip_specific_get(struct wifi_hw *hw);
extern void mt7981_chip_specific_get(struct wifi_hw *hw);
extern u32 mt7615_warp_register_client(struct wifi_hw *hw);
extern u32 mt7915_warp_register_client(struct wifi_hw *hw);
extern u32 mt7986_warp_register_client(struct wifi_hw *hw);
extern u32 mt7916_warp_register_client(struct wifi_hw *hw);
extern u32 mt7981_warp_register_client(struct wifi_hw *hw);



/*
*
*/
static inline u32
client_get_chip_id(void *priv_data)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;

	return ad->ChipID;
}
/*
*
*/
#ifdef WF_RESET_SUPPORT
static inline bool
client_get_wf_reset_in_progress(void *priv_data)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;

	return ad->wf_reset_in_progress;
}
#endif /* WF_RESET_SUPPORT */
/*
*
*/
static inline u32
client_get_wed_id(void *priv_data)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;

	return ad->CommonCfg.wed_idx;
}

/*
*
*/
static inline u32
client_get_bus_type(void *priv_data)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;

	switch (ad->infType) {
	case RTMP_DEV_INF_PCI:
	case RTMP_DEV_INF_PCIE:
		return BUS_TYPE_PCIE;
	case RTMP_DEV_INF_RBUS:
		return BUS_TYPE_AXI;
	default:
		return BUS_TYPE_PCIE;
	}
}

/*
*
*/
static inline void *
client_get_hif_dev(void *priv_data)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;

	return ((struct os_cookie *)ad->OS_Cookie)->pci_dev;
}

/*
*
*/
static inline u32
client_get_axi_slot_id(void *vad)
{
#ifdef MT7986
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) vad;

	return 0; /* previousl ad->wfsys_id; */
#endif
	proxy_print("%s(): wrong BUS_TYPE\n", __func__);
	return 0;
}

/*
*
*/
static inline u32
client_whnat_en_get(void *priv_data)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) priv_data;

	return ad->CommonCfg.whnat_en;
}

/*
*
*/
static void
client_whnat_en_set(void *priv_data, u32 en)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) priv_data;

	ad->CommonCfg.whnat_en = en;
}

static int
client_io_read( void *hw, u32 addr, u32 *value)
{
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *) hw;

	return warp_proxy_read(ctrl->priv, addr, value);
}

static int
client_io_write(void *hw, u32 addr, u32 value)
{
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *) hw;

	return warp_proxy_write(ctrl->priv, addr, value);
}

/*
*
*/
void client_config_atc(void *priv_data, bool enable)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) priv_data;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (enable) {
		ops->hif_io_read32 = NULL;
		ops->hif_io_write32 = NULL;
	} else {
		ops->hif_io_read32 = client_io_read;
		ops->hif_io_write32 = client_io_write;
	}
}

/*
*
*/
void client_swap_irq(void *priv_data, u32 irq)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) priv_data;
	struct net_device *dev = ad->net_dev;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;

	dev->irq = irq;
	hif_chip->irq = irq;
}

/*
*
*/
void client_txinfo_wrapper(u8 *tx_info, struct wlan_tx_info *info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;

	info->pkt = txblk->pPacket;
	info->bssidx = txblk->wdev->bss_info_argument.ucBssIndex;
	info->ringidx = txblk->dbdc_band;

#if defined(APCLI_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	if (txblk->pMacEntry && (IS_ENTRY_PEER_AP(txblk->pMacEntry))) {
			info->wcid = txblk->pMacEntry->wcid;
	}
	else
#endif
#ifdef MAC_REPEATER_SUPPORT
	if (txblk->pMacEntry && IS_ENTRY_REPEATER(txblk->pMacEntry))
		info->wcid = txblk->pMacEntry->wcid;
	else
#endif
#ifdef A4_CONN
	if (txblk->pMacEntry && IS_ENTRY_A4(txblk->pMacEntry))
		info->wcid = txblk->pMacEntry->wcid;
	else
#endif /* A4_CONN */
	{
		info->wcid = 0x3ff;
	}
}

/*
*
*/
void client_txinfo_set_drop(u8 *tx_info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;

	txblk->DropPkt = true;
}

/*
*
*/
bool client_hw_tx_allow(u8 *tx_info)
{
	struct _TX_BLK *txblk = (struct _TX_BLK *)tx_info;
	struct wifi_dev *wdev = txblk->wdev;

	if (!wdev)
		return false;

	if (wlan_operate_get_frag_thld(wdev) != DEFAULT_FRAG_THLD)
		return false;

	return true;
}

static void
dump_raw(char *str, u8 *va, u32 size)
{
	u8 *pt;
	char buf[512] = "";
	u32 len = 0;
	int x;
	int ret = 0;

	pt = va;
	proxy_print("%s: %p, len = %d\n", str, va, size);

	for (x = 0; x < size; x++) {
		if (x % 16 == 0) {
			ret = snprintf(buf + len, sizeof(buf) - len, "\n0x%04x : ", x);
			if (os_snprintf_error(sizeof(buf) - len, ret)) {
				proxy_print("%s: snprintf error!\n", __func__);
				return;
			}
			len = strlen(buf);
		}

		ret = snprintf(buf + len, sizeof(buf) - len, "%02x ", ((u8)pt[x]));
		if (os_snprintf_error(sizeof(buf) - len, ret)) {
			proxy_print("%s: snprintf error!\n", __func__);
			return;
		}
		len = strlen(buf);
	}

	proxy_print("%s\n", buf);
}

/*
*
*/
void client_tx_ring_info_dump(void *priv_data, u8 ring_id, u32 idx)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) priv_data;
	struct _PCI_HIF_T *pci_cfg = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct hif_pci_tx_ring *tx_ring = pci_get_tx_ring_by_ridx(pci_cfg, ring_id);
	RTMP_DMACB *cb = &tx_ring->Cell[idx];

	proxy_print("AllocPA\t: %pad\n", &cb->AllocPa);
	proxy_print("AllocVa\t: %p\n", cb->AllocVa);
	proxy_print("Size\t: %lu\n", cb->AllocSize);
	proxy_print("PktPtr\t: %p\n", cb->pNdisPacket);
	dump_raw("WED_TX_RING", cb->AllocVa, cb->AllocSize);
}

/*
*
*/
void client_warp_ver_notify(void *priv_data, u8 ver, u8 sub_ver, u8 branch, int hw_cap)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (ver >= 0x2) {
		if (hw_cap & 0x2)
			cap->tkn_info.feature |= TOKEN_RX;

		if ((hw_cap & 0x8) || (hw_cap & 0x10))
			cap->asic_caps |= fASIC_CAP_BA_OFFLOAD;

		/*Even though WED can support V4, 7915 and 7615 WA still support V1 format*/
		if (sub_ver > 0 && !(IS_MT7915(ad)) && !(IS_MT7615(ad)))
			cap->asic_caps |= fASIC_CAP_TX_FREE_NOTIFY_V4;
		}

	ad->CommonCfg.wed_version = ((ver << WED_REV_ID_FLD_MAJOR_OFFSET) & WED_REV_ID_FLD_MAJOR_MASK)+
								((sub_ver << WED_REV_ID_FLD_MINOR_OFFSET) & WED_REV_ID_FLD_MINOR_MASK) +
								((branch << WED_REV_ID_FLD_BRANCH_OFFSET) & WED_REV_ID_FLD_BRANCH_MASK);
}

/*
*
*/
u32 client_token_rx_dmad_init(void *priv_data, void *pkt,
				unsigned long alloc_size, void *alloc_va,
				dma_addr_t alloc_pa)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(ad->hdev_ctrl);

	return token_rx_dmad_init(&cb->rx_que, pkt, alloc_size, alloc_va, alloc_pa);
}

/*
*
*/
int client_token_rx_dmad_lookup(void *priv_data, u32 tkn_rx_id,
				void **pkt,
				void **alloc_va, dma_addr_t *alloc_pa)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;
	PKT_TOKEN_CB *cb = hc_get_ct_cb(ad->hdev_ctrl);

	token_rx_dmad_lookup(&cb->rx_que, tkn_rx_id, pkt,
			     alloc_va, alloc_pa);

	return 0;
}

/*
*
*/
void client_rxinfo_wrapper(u8 *rx_info, struct wlan_rx_info *wlan_info)
{
	struct rxdmad_info *info = (struct rxdmad_info *)rx_info;

	wlan_info->pkt = info->pkt;
	wlan_info->ppe_entry = info->ppe_entry;
	wlan_info->csrn = info->csrn;
}

/*
*
*/
void client_do_wifi_reset(void *priv_data)
{
#ifdef WF_RESET_SUPPORT
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;
	RTMP_CHIP_OP *chip_op = hc_get_chip_ops(ad->hdev_ctrl);

	ad->wf_reset_wo_count++;

	if (chip_op->do_wifi_reset)
		chip_op->do_wifi_reset(ad);
#endif /* WF_RESET_SUPPORT */
}

#ifdef TR181_SUPPORT
#ifdef STAT_ENHANCE_SUPPORT
UINT8  tid_to_ac_mapping[NUM_OF_TID] = {
	WMM_AC_BE, WMM_AC_BK, WMM_AC_BK, WMM_AC_BE,
	WMM_AC_VI, WMM_AC_VI, WMM_AC_VO, WMM_AC_VO
};
#endif
#endif

/*
*
*/
void client_update_wo_rxcnt(void *priv_data, void *wo_rxcnt)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)priv_data;
	struct wo_cmd_rxcnt_t *r_cnt = (struct wo_cmd_rxcnt_t *)wo_rxcnt;
#ifdef TR181_SUPPORT
#ifdef STAT_ENHANCE_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
#endif
#endif

	if (!ad) {
		proxy_print("%s(): invalid ad, ignored!\n", __func__);
		return;
	}
	if ((r_cnt->wlan_idx < MAX_LEN_OF_MAC_TABLE) && (r_cnt->tid < NUM_OF_TID)) {
		ad->wo_rxcnt[r_cnt->tid][r_cnt->wlan_idx].rx_pkt_cnt += r_cnt->rx_pkt_cnt;
		ad->wo_rxcnt[r_cnt->tid][r_cnt->wlan_idx].rx_byte_cnt += r_cnt->rx_byte_cnt;
		ad->wo_rxcnt[r_cnt->tid][r_cnt->wlan_idx].rx_err_cnt += r_cnt->rx_err_cnt;
		ad->wo_rxcnt[r_cnt->tid][r_cnt->wlan_idx].rx_drop_cnt += r_cnt->rx_drop_cnt;
	} else {
		if (r_cnt->wlan_idx >= MAX_LEN_OF_MAC_TABLE) {
			proxy_print("%s(): invalid wcid=%d, ignored!\n", __func__, r_cnt->wlan_idx);
			return;
		}
		if (r_cnt->tid >= NUM_OF_TID) {
			proxy_print("%s(): invalid tid=%d, ignored!\n", __func__, r_cnt->tid);
			return;
		}
	}
#ifdef TR181_SUPPORT
#ifdef STAT_ENHANCE_SUPPORT
	pEntry = &ad->MacTab.Content[r_cnt->wlan_idx];

	if (!(IS_VALID_ENTRY(pEntry)) || (pEntry->Sst != SST_ASSOC) ||
		(pEntry->wcid != r_cnt->wlan_idx))
		return;

	pEntry->RxPackets.QuadPart += r_cnt->rx_pkt_cnt;
	pEntry->RxBytes += r_cnt->rx_byte_cnt;
	if (pEntry->pMbss) {
		pEntry->pMbss->RxCount += r_cnt->rx_pkt_cnt;
		pEntry->pMbss->ReceivedByteCount += r_cnt->rx_byte_cnt;
		pEntry->pMbss->RxDropCount += r_cnt->rx_drop_cnt;
		pEntry->pMbss->RxErrorCount += r_cnt->rx_err_cnt;
		pEntry->pMbss->RxCountPerAC[tid_to_ac_mapping[r_cnt->tid]] += r_cnt->rx_pkt_cnt;
		pEntry->pMbss->ReceivedByteCountPerAC[tid_to_ac_mapping[r_cnt->tid]] += r_cnt->rx_byte_cnt;
		pEntry->pMbss->RxErrorCountPerAC[tid_to_ac_mapping[r_cnt->tid]] += r_cnt->rx_err_cnt;
		pEntry->pMbss->RxDropCountPerAC[tid_to_ac_mapping[r_cnt->tid]] += r_cnt->rx_drop_cnt;
	}
#endif
#endif
}

/*
*
*/
static void
client_hif_specific_get(struct wifi_hw *hw, void *vad)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)vad;
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_dev *pci_dev = hw->hif_dev;

	hw->hif_type = client_get_bus_type(vad);
	hw->base_addr = (unsigned long)pci_hif->CSRBaseAddress;
	hw->base_phy_addr = pci_resource_start(pci_dev, 0);
	hw->wpdma_base = hw->base_phy_addr | hw->dma_offset;
	proxy_print("%s(): hw->base_phy_addr 0x%lx!!\n",
			__func__, hw->base_phy_addr);

	return;
}

static void
client_chip_specific_get(struct wifi_hw *hw)
{
	switch (hw->chip_id) {
#ifdef MT7986
	case 0x7986:
		mt7986_chip_specific_get(hw);
		break;
#endif
#ifdef MT7916
	case 0x7906:
		mt7916_chip_specific_get(hw);
		proxy_print("%s(): 0x7916 chip id 0x%4x!!\n",
			__func__, hw->chip_id);
		break;
#endif
#ifdef MT7981
	case 0x7981:
		mt7981_chip_specific_get(hw);
		proxy_print("%s(): 0x7981 chip id 0x%4x!!\n",
			__func__, hw->chip_id);
		break;
#endif


	default:
		proxy_print("%s(): wrong chip id 0x%4x!!\n",
			__func__, hw->chip_id);
		return;
	}
}

/*
*
*/
static void
client_cap_get(struct wifi_hw *hw, void *vad)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)vad;
	struct _RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct _PCI_HIF_T *pci_hif = hc_get_hif_ctrl(ad->hdev_ctrl);
	struct pci_hif_chip *hif_chip = pci_hif->main_hif_chip;
	const struct hif_pci_tx_ring_desc *tx_ring_layout =
				hif_chip->ring_layout.tx_ring_layout;
	const struct hif_pci_rx_ring_desc *rx_ring_layout =
				hif_chip->ring_layout.rx_ring_layout;
	u8 i;
	u16 ring_size = 0;

	/*basic information*/
	hw->priv = vad;
	hw->chip_id = client_get_chip_id(vad);
	hw->hif_dev = client_get_hif_dev(vad);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	hw->msi_enable = hif_chip->is_msi;
#endif

	/* tx ring num */
	hw->tx_ring_num = 0;
	for (i = 0; i < hif_chip->tx_res_num; i++) {
		struct hif_pci_tx_ring_desc ring_desc = tx_ring_layout[i];

		if (ring_desc.ring_attr == HIF_TX_DATA)
			hw->tx_ring_num++;
	}

	/* rx ring num */
	hw->rx_ring_num = 0;
	hw->rx_data_ring_size = 0;

	for (i = 0; i < hif_chip->rx_res_num; i++) {
		struct hif_pci_rx_ring_desc ring_desc = rx_ring_layout[i];

		if (ring_desc.ring_attr == HIF_RX_DATA) {
			hw->rx_ring_num++;
#ifdef MEMORY_SHRINK
			if (chip_cap->rx_sw_ring_size > ring_size)
				ring_size = chip_cap->rx_sw_ring_size;
#else
			if (ring_desc.ring_size > ring_size)
				ring_size = ring_desc.ring_size;
#endif
		}
	}

	hw->rx_data_ring_size = ring_size;

	/* token */
	hw->tx_token_nums = hc_get_chip_tx_token_nums(ad->hdev_ctrl);
	hw->sw_tx_token_nums = hc_get_chip_sw_tx_token_nums(ad->hdev_ctrl);
	hw->hw_rx_token_num = chip_cap->tkn_info.token_rx_cnt - hw->rx_ring_num * hw->rx_data_ring_size;

	/* max rxd size */
	hw->max_rxd_size = hc_get_chip_mac_rxd_size(ad->hdev_ctrl);

	/* dma address */
	hw->p_int_mask = &hif_chip->int_enable_mask;

	/*original wbsys irq*/
	hw->irq = hif_chip->irq;

	/* get hif specific profile */
	client_hif_specific_get(hw, vad);

	/* get chip specific profile */
	client_chip_specific_get(hw);
}

void fw_cmd_rsp_handler(char *msg, u16 msg_len, void *user_data) {

	int cmd_id = *(int *)user_data;
	struct wo_cmd_rxstat_para *rxstat_p;

	if (cmd_id == WO_CMD_WED_RX_STAT &&
		msg_len == sizeof(struct wo_cmd_rxstat_para)) {
		UINT8 prof_lvl = 0;
		ULONG prof_total = 0;
		rxstat_p = (struct wo_cmd_rxstat_para *)msg;

		proxy_print("%s(), ack:%d, rev:%d, err:%d, drop:%d\n", __func__,
			rxstat_p->rx_ack_cnt, rxstat_p->rx_rev_cnt,
			rxstat_p->rx_err_cnt, rxstat_p->rx_drop_cnt);

		for (prof_lvl = 0 ; prof_lvl < MAX_WO_PROF_LVL ; prof_lvl++) {
			if (rxstat_p->prof_record[prof_lvl*2] > 0)
				prof_total += rxstat_p->prof_record[prof_lvl*2+1];
		}
		proxy_print("[Profiling Statistic Report]\n");
		for (prof_lvl = 0 ; prof_lvl < MAX_WO_PROF_LVL ; prof_lvl++) {
			if (rxstat_p->prof_record[prof_lvl*2] > 0 && prof_total > 0)
				proxy_print("\t < %uus = %u packets (%d%%)",
					rxstat_p->prof_record[prof_lvl*2],
					rxstat_p->prof_record[prof_lvl*2+1],
					(u32)(rxstat_p->prof_record[prof_lvl*2+1]*100/prof_total));
			else {
				proxy_print("\t N/A");
				break;
			}
		}
	}
}

static INT fw_cmd_notify_traffic_handler(struct notify_entry *ne, INT cmd_id,
		VOID *data)
{
	struct fw_cmd_notify_info *info = (struct fw_cmd_notify_info *)data;
	u8 wed_idx;
	struct warp_msg_cmd cmd = {0};

	if (info == NULL) {
		proxy_print("%s(): fw_cmd_notify null\n", __func__);
		return -1;
	}
	/*
	proxy_print("%s(): cmd_id=0x%x\n", __func__, cmd_id);
	*/
#ifdef WF_RESET_SUPPORT
	/* do not send cmd to wo if wf reset is in progress */
	if (client_get_wf_reset_in_progress((void *)info->ad))
		return 0;
#endif


	wed_idx = warp_get_wed_idx(info->ad);

	if (wed_idx == 0xff)
		return -1;

	cmd.param.cmd_id = cmd_id;
	cmd.param.to_id = MODULE_ID_WO;
	cmd.param.wait_type = WARP_MSG_WAIT_TYPE_RSP_STATUS;
	cmd.param.timeout = 3000;
	cmd.param.rsp_hdlr = fw_cmd_rsp_handler;
	cmd.param.user_data = (void *)&cmd_id;
	cmd.msg = info->msg;
	cmd.msg_len = info->msg_len;

	return warp_msg_send_cmd(wed_idx, &cmd);
}

struct notify_entry fw_cmd_ne = {
	.notify_call = fw_cmd_notify_traffic_handler,
	.priority = FW_CMD_NOTIFY_PRIORITY_PROXY,
	.priv = NULL,
};

/*
*
*/
static INT
client_register(struct wifi_hw *hw)
{
	switch (hw->chip_id) {
#ifdef MT7986
	case 0x7986:
		return mt7986_warp_register_client(hw);
#endif
#ifdef MT7916
	case 0x7906:
		return mt7916_warp_register_client(hw);
#endif
#ifdef MT7981
	case 0x7981:
		return mt7981_warp_register_client(hw);
#endif

	default:
		proxy_print("%s(): wrong chip id!!\n", __func__);
		return -1;
	}
}

/*
*
*/
static void
client_driver_probe(void *ad)
{
	struct wifi_hw *hw;
	u32 bus_type = client_get_bus_type(ad);
	u32 chip_id = client_get_chip_id(ad);
	u32 slot_id = bus_type == BUS_TYPE_AXI ?
		client_get_axi_slot_id(ad) : 0;
	u8 wed_idx = client_get_wed_id(ad);

	if (!client_whnat_en_get(ad)) {
		proxy_print("%s(): chip not enable wifi hardware nat feature!\n", __func__);
		goto err;
	}

	hw = warp_alloc_client(chip_id, bus_type, wed_idx, slot_id, client_get_hif_dev(ad));
	if (!hw) {
		proxy_print("%s(): chip allocate warp fail!\n", __func__);
		goto err;
	}

	client_cap_get(hw, ad);
	if (client_register(hw) < 0) {
		/*probe client fail, disable wrap in client card*/
		client_whnat_en_set(ad, false);
		goto err;
	}
	register_fw_cmd_notifier(ad, &fw_cmd_ne);
err:
	return;
}

/*
*
*/
static void
client_driver_remove(void *ad)
{
	unregister_fw_cmd_notifier(ad, &fw_cmd_ne);
	warp_client_remove(ad);
}


/*
*
*/
static void
client_dma_handler(void *ad, int *wifi_dma)
{
	unsigned int warp_dma;

	switch (*wifi_dma) {
	case DMA_TX_RX:
		warp_dma = WARP_DMA_TXRX;
		break;

	case DMA_TX:
		warp_dma = WARP_DMA_TX;
		break;

	case DMA_RX:
		warp_dma = WARP_DMA_RX;
		break;

	default:
		warp_dma = WARP_DMA_DISABLE;
		break;
	}

	warp_dma_handler(ad, warp_dma);
}

/*
*
*/
static void
client_ser_handler(void *ad, void *ser_ctrl)
{
#ifdef ERR_RECOVERY
	ERR_RECOVERY_CTRL_T *ctrl = (ERR_RECOVERY_CTRL_T *)ser_ctrl;

	if (ctrl)
		warp_ser_handler(ad, ctrl->errRecovStage);
#endif /*ERR_RECOVERY*/
}

/*
*
*/
static u32
client_handle(u16 hook, void *ad, void *priv)
{

	if (!client_whnat_en_get(ad))
		return 0;

	switch (hook) {
	case WLAN_HOOK_HIF_INIT:
		warp_ring_init(ad);
		break;

	case WLAN_HOOK_HIF_EXIT:
		warp_ring_exit(ad);
		break;

	case WLAN_HOOK_TX:
		warp_wlan_tx(ad, priv);
		break;

	case WLAN_HOOK_RX:
		warp_wlan_rx(ad, priv);
		break;
	case WLAN_HOOK_SYS_UP:
		client_driver_probe(ad);
		break;

	case WLAN_HOOK_SYS_DOWN:
		client_driver_remove(ad);
		break;

	case WLAN_HOOK_ISR:
		warp_isr_handler(ad);
		break;

	case WLAN_HOOK_DMA_SET:
		client_dma_handler(ad, priv);
		break;

	case WLAN_HOOK_SER:
		client_ser_handler(ad, priv);
		break;

	case WLAN_HOOK_SUSPEND:
		warp_suspend_handler(ad);
		break;

	case WLAN_HOOK_RESUME:
		warp_resume_handler(ad);
		break;

#ifdef WF_RESET_SUPPORT
	case WLAN_HOOK_HB_CHK:
		warp_hb_chk_handler(ad, priv);
		break;
#endif
#ifdef WARP_512_SUPPORT
	case WLAN_HOOK_WARP_512_SUPPORT:
		warp_512_support_handler(ad, (u8 *)priv);
		break;
#endif

	default:
		proxy_print("%s(): can't find whnat handle (hook: %u)\n", __func__, hook);
		break;
	}

	return 0;
}

static struct mt_wlan_hook_ops  warp_ops = {
	.name = "WARP",
	.hooks = (1 << WLAN_HOOK_HIF_INIT) |
	(1 << WLAN_HOOK_HIF_EXIT) |
	(1 << WLAN_HOOK_DMA_SET) |
	(1 << WLAN_HOOK_SYS_UP) |
	(1 << WLAN_HOOK_SYS_DOWN) |
	(1 << WLAN_HOOK_ISR) |
	(1 << WLAN_HOOK_TX) |
	(1 << WLAN_HOOK_RX) |
	(1 << WLAN_HOOK_SER) |
	(1 << WLAN_HOOK_SUSPEND) |
	(1 << WLAN_HOOK_RESUME) |
	(1 << WLAN_HOOK_HB_CHK) |
	(1 << WLAN_HOOK_WARP_512_SUPPORT),
	.fun = client_handle,
	.priority = WLAN_HOOK_PRI_WOE
};

static int __init warp_proxy_init(void)
{
	/*register hook function*/
	mt_wlan_hook_register(&warp_ops);
	return 0;
}

static void __exit warp_proxy_exit(void)
{
	mt_wlan_hook_unregister(&warp_ops);
}

module_init(warp_proxy_init);
module_exit(warp_proxy_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(DRIVER_DESC);

