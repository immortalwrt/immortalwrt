
#ifndef __HIF_H__
#define __HIF_H__


#ifdef MT_MAC
#ifdef RTMP_MAC_PCI
#include "hif/mt_hif_pci.h"
#include "mac/mac_mt/mt_mac_pci.h"
#endif /* RTMP_MAC_PCI */


#endif /* MT_MAC */

#include "tx_power.h"

enum {
	HIF_RX_IDX0,
	HIF_RX_IDX1,
};

typedef enum _RTMP_INF_TYPE_ {
	RTMP_DEV_INF_UNKNOWN = 0,
	RTMP_DEV_INF_PCI = 1,
	RTMP_DEV_INF_USB = 2,
	RTMP_DEV_INF_RBUS = 4,
	RTMP_DEV_INF_PCIE = 5,
	RTMP_DEV_INF_SDIO = 6,
} RTMP_INF_TYPE;

#define IS_SDIO_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_SDIO)
#define IS_USB_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_USB)
#define IS_PCIE_INF(_pAd)		((_pAd)->infType == RTMP_DEV_INF_PCIE)
#define IS_PCI_INF(_pAd)		(((_pAd)->infType == RTMP_DEV_INF_PCI) || IS_PCIE_INF(_pAd))
#define IS_PCI_ONLY_INF(_pAd)	((_pAd)->infType == RTMP_DEV_INF_PCI)
#define IS_RBUS_INF(_pAd) ((_pAd)->infType == RTMP_DEV_INF_RBUS)


/*generic hif operation*/
struct hif_ops {
	/*generic hif txrx struct handler*/
	UINT32 (*get_resource_type)(void *hif, UINT8 resource_idx);
	BOOLEAN (*free_txd)(struct _RTMP_ADAPTER *ad, UINT8 resource_idx);
	VOID (*free_rx_buf)(void *hdev_ctrl, UCHAR resource_idx);
	NDIS_STATUS (*init_txrx_mem)(void *hdev_ctrl);
	VOID (*reset_txrx_mem)(void *hdev_ctrl);
	BOOLEAN (*poll_txrx_empty)(void *hdev_ctrl, UINT8 pcie_port_or_all);
	UCHAR* (*get_tx_buf) (void *hdev_ctrl, struct _TX_BLK *tx_blk, UCHAR resource_idx, UCHAR frame_type);
	UINT32 (*get_tx_resource_free_num)(void *hdev_ctrl, UINT8 resource_idx);
	UINT32 (*get_resource_idx)(void *hdev_ctrl, UINT8 band_idx, enum PACKET_TYPE pkt_type, UINT8 que_idx);
	NDIS_STATUS (*init_task_group)(void *hdev_ctrl);
	NDIS_STATUS (*reset_task_group)(void *hdev_ctrl);
	NDIS_STATUS (*register_irq)(void *hdev_ctrl);
	NDIS_STATUS (*free_irq)(void *hdev_ctrl);
	/*generic dma operate*/
	VOID (*dma_reset)(void *hdev_ctrl);
	VOID (*dma_enable)(void *hdev_ctrl);
	VOID (*dma_disable)(void *hdev_ctrl);
	/*generic mcu handler*/
	VOID (*mcu_init)(void *hdev_ctrl);
	VOID (*mcu_exit)(void *hdev_ctrl);
	VOID (*mcu_fw_init) (void *hdev_ctrl);
	VOID (*mcu_fw_exit) (void *hdev_ctrl);
	VOID (*mcu_unlink_ackq) (struct cmd_msg *msg);
	struct cmd_msg* (*mcu_alloc_msg)(struct _RTMP_ADAPTER *ad, unsigned int length);
	INT32 (*kick_out_cmd_msg)(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
	INT32 (*kick_out_fwdl_msg)(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
	VOID (*kickout_data_tx)(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk, UCHAR resource_idx);
	NDIS_STATUS (*kickout_nullframe_tx)(struct _RTMP_ADAPTER *ad, UCHAR que_idx, UCHAR *data, UINT len);
	VOID (*rx_event_process)(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
	INT (*sys_init) (void *hdev_ctrl);
	INT (*cmd_thread) (ULONG context);
#ifdef CONFIG_STA_SUPPORT
	VOID (*ps_poll_enq)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*sta_wakeup)(struct _RTMP_ADAPTER *ad, BOOLEAN bFromTx, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*sta_sleep_auto_wakeup)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
#endif
	UINT8 (*get_tx_res_num)(void *hif_ctrl);
	UINT8 (*get_rx_res_num)(void *hif_ctrl);
};

UINT32 hif_get_resource_type(void *hdev_ctrl, UINT8 resouce_idx);
BOOLEAN hif_free_txd(struct _RTMP_ADAPTER *ad, UINT8 resource_idx);
UCHAR *hif_get_tx_buf(void *hdev_ctrl, struct _TX_BLK *tx_blk, UCHAR resource_idx, UCHAR frame_type);
UINT32 hif_get_tx_resource_free_num(void *hdev_ctrl, UINT8 resource_idx);
VOID hif_free_rx_buf(void *hdev_ctrl, UCHAR resource_idx);
VOID hif_reset_txrx_mem(void *hdev_ctrl);
NDIS_STATUS hif_init_txrx_mem(void *hdev_ctrl);
VOID hif_dma_reset(void *hdev_ctrl);
VOID hif_dma_enable(void *hdev_ctrl);
VOID hif_dma_disable(void *hdev_ctrl);
BOOLEAN hif_poll_txrx_empty(void *hdev_ctrl, UINT8 pcie_port_or_all);
UINT32 hif_get_resource_idx(void *hdev_ctrl, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UCHAR q_idx);
NDIS_STATUS hif_init_task_group(void *hdev_ctrl);
NDIS_STATUS hif_reset_task_group(void *hdev_ctrl);
NDIS_STATUS hif_register_irq(void *hdev_ctrl);
NDIS_STATUS hif_free_irq(void *hdev_ctrl);

/*MCU related*/
VOID hif_mcu_init(void *hdev_ctrl);
VOID hif_mcu_exit(void *hdev_ctrl);
VOID hif_mcu_fw_init (struct _RTMP_ADAPTER *ad);
VOID hif_mcu_fw_exit (struct _RTMP_ADAPTER *ad);
VOID hif_mcu_unlink_ackq(struct cmd_msg *msg);
struct cmd_msg *hif_mcu_alloc_msg(struct _RTMP_ADAPTER *ad, unsigned int length, BOOLEAN bOldCmdFmt);
VOID hif_rx_event_process(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
INT32 hif_kick_out_fwdl_msg(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
INT32 hif_kick_out_cmd_msg(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
VOID hif_kickout_data_tx(struct _RTMP_ADAPTER *ad, struct _TX_BLK *tx_blk, UCHAR resource_idx);
NDIS_STATUS hif_kickout_nullframe_tx(struct _RTMP_ADAPTER *ad, UCHAR que_idx, UCHAR *data, UINT len);

/*PS*/
#ifdef CONFIG_STA_SUPPORT
VOID hif_ps_poll_enq(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID hif_sta_wakeup(struct _RTMP_ADAPTER *ad, BOOLEAN bFromTx, struct _STA_ADMIN_CONFIG *pStaCfg);
VOID hif_sta_sleep_auto_wakeup(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
#endif

/*core alloc/free*/
NDIS_STATUS hif_sys_init(void *hdev_ctrl);
INT hif_cmd_thread(ULONG Context);
UINT8 hif_get_tx_res_num(VOID *hdev_ctrl);
UINT8 hif_get_rx_res_num(VOID *hdev_ctrl);



#endif /* __HIF_H__ */

