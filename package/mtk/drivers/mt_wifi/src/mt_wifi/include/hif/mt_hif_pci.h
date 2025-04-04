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
	mt_hif_pci.h
*/

#ifndef __MT_HIF_PCI_H__
#define __MT_HIF_PCI_H__

#include "mac/mac.h"
#include "rtmp_dmacb.h"
#include "hif_base.h"
#include <linux/skbuff.h>

#ifdef ERR_RECOVERY
#define MCU_INT_PDMA0_STOP_DONE	        BIT(0)
#define MCU_INT_PDMA0_INIT_DONE	        BIT(1)
#define MCU_INT_SER_TRIGGER_FROM_HOST   BIT(2)
#define MCU_INT_PDMA0_RECOVERY_DONE	    BIT(3)
#endif /* ERR_RECOVERY .*/

#ifdef ERR_RECOVERY
#define ERROR_DETECT_STOP_PDMA_WITH_FW_RELOAD BIT(1)
#define ERROR_DETECT_STOP_PDMA BIT(2)
#define ERROR_DETECT_RESET_DONE BIT(3)
#define ERROR_DETECT_RECOVERY_DONE BIT(4)
#define ERROR_DETECT_N9_NORMAL_STATE  BIT(5)
#define CP_LMAC_HANG_WORKAROUND_STEP1 BIT(8)
#define CP_LMAC_HANG_WORKAROUND_STEP2 BIT(9)
#define ERROR_DETECT_LMAC_ERROR BIT(24)
#define ERROR_DETECT_PSE_ERROR BIT(25)
#define ERROR_DETECT_PLE_ERROR BIT(26)
#define ERROR_DETECT_PDMA_ERROR BIT(27)
#define ERROR_DETECT_PCIE_ERROR BIT(28)
#endif

#define MT_MCU_CMD_CLEAR_FW_OWN BIT(0)
#ifdef ERR_RECOVERY
#define ERROR_DETECT_STOP_PDMA_WITH_FW_RELOAD BIT(1)
#define ERROR_DETECT_STOP_PDMA BIT(2)
#define ERROR_DETECT_RESET_DONE BIT(3)
#define ERROR_DETECT_RECOVERY_DONE BIT(4)
#define ERROR_DETECT_N9_NORMAL_STATE  BIT(5)
#define CP_LMAC_HANG_WORKAROUND_STEP1 BIT(8)
#define CP_LMAC_HANG_WORKAROUND_STEP2 BIT(9)
#define ERROR_DETECT_LMAC_ERROR BIT(24)
#define ERROR_DETECT_PSE_ERROR BIT(25)
#define ERROR_DETECT_PLE_ERROR BIT(26)
#define ERROR_DETECT_PDMA_ERROR BIT(27)
#define ERROR_DETECT_PCIE_ERROR BIT(28)

#if defined(P18) || defined(MT7663) || defined(AXE) || defined(MT7626) || \
	defined(MT7915) || defined(MT7986) || defined(MT7916) || defined(MT7981)
#define MT7663_ERROR_DETECT_MASK \
			(ERROR_DETECT_STOP_PDMA \
			| ERROR_DETECT_RESET_DONE \
			| ERROR_DETECT_RECOVERY_DONE \
			| ERROR_DETECT_N9_NORMAL_STATE)
#endif


#define ERROR_RECOVERY_PDMA0_STOP_NOTIFY BIT(0)
#define ERROR_RECOVERY_PDMA0_INIT_DONE_NOTIFY BIT(1)
#endif /* ERR_RECOVERY */


#define MT_INT_RX_DLY_BIT		(1<<22)
#define MT_FW_CLEAR_OWN_BIT		(1<<31) /* 7615 and 7622 only, equal MT_FW_CLR_OWN_INT, used by mt_mac_fw_own_func */
#define MT_McuCommand		(1<<30) /* 7615 and 7622, equal MT_INT_MCU_CMD, CONNAC use MT_INT_MCU2HOST_SW_INT_STS, used by mt_mac_recovery_func */
#define MT_INT_SUBSYS_INT_STS_BIT				(1<<28) /* CONNAC only, equal MT_INT_SUBSYS_INT_STS, change in AXE, used by mt_subsys_int_func */
#define MT_INT_MCU2HOST_SW_INT_STS_BIT			(1<<29) /* CONNAC only, equal MT_INT_MCU2HOST_SW_INT_STS, change in AXE, used by mt_mac_recovery_func */

#define MT_WPDMA_MEM_RNG_ERR   (0x4000 + 0x0224) /* 7615 and 7622 SER flow */
#ifdef CONFIG_WIFI_MSI_SUPPORT
#define MSI_IRQ0	0
#define MSI_IRQ1	1
#define MSI_IRQ2	2
#define MSI_IRQ3	3
#define MSI_IRQ4	4
#define MSI_IRQ5	5
#define MSI_IRQ6	6
#define MSI_IRQ7	7
#endif

/* =================================================================================
	PCI/RBUS TX / RX Frame Descriptors format

	Memory Layout

	1. Tx Descriptor
			TxD (12 bytes) + TXINFO (4 bytes)
	2. Packet Buffer
			TXWI + 802.11
	 31                                                                                                                             0
	+--------------------------------------------------------------------------+
	|                                   SDP0[31:0]                                                                               |
	+-+--+---------------------+-+--+-----------------------------------------+
	|D |L0|       SDL0[13:0]              |B|L1|                    SDL1[13:0]                                    |
	+-+--+---------------------+-+--+-----------------------------------------+
	|                                   SDP1[31:0]                                                                               |
	+--------------------------------------------------------------------------+
	|                                        TXINFO                                                                                |
	+--------------------------------------------------------------------------+
================================================================================= */
/*
	TX descriptor format for Tx Data/Mgmt Rings
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _TXD_STRUC {
	/* Word 0 */
	UINT32		SDPtr0;

	/* Word 1 */
	UINT32		DMADONE:1;
	UINT32		LastSec0:1;
	UINT32		SDLen0:14;
	UINT32		Burst:1;
	UINT32		LastSec1:1;
	UINT32		SDLen1:14;
	/* Word 2 */
	UINT32		SDPtr1;
} TXD_STRUC;
#else
typedef	struct GNU_PACKED _TXD_STRUC {
	/* Word	0 */
	UINT32		SDPtr0;

	/* Word	1 */
	UINT32		SDLen1:14;
	UINT32		LastSec1:1;
	UINT32		Burst:1;
	UINT32		SDLen0:14;
	UINT32		LastSec0:1;
	UINT32		DMADONE:1;
	/*Word2 */
	UINT32		SDPtr1;
} TXD_STRUC;
#endif /* RT_BIG_ENDIAN */

/*
	Rx descriptor format for Rx Rings
*/

#define RX_TOKEN_ID_MASK (0xffff << 16)
#define RX_TOKEN_ID_SHIFT 16
#define TO_HOST_SHIFT 8

#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXD_STRUC {
	/* Word 0 */
	UINT32		SDP0;

	/* Word 1 */
	UINT32		DDONE:1;
	UINT32		LS0:1;
	UINT32		SDL0:14;
	UINT32		BURST:1;
	UINT32		LS1:1;
	UINT32		SDL1:14;

	/* Word 2 */
	UINT32		SDP1;
} RXD_STRUC;
#else
typedef	struct GNU_PACKED _RXD_STRUC {
	/* Word	0 */
	UINT32		SDP0;

	/* Word	1 */
	UINT32		SDL1:14;
	UINT32		LS1:1;
	UINT32		BURST:1;
	UINT32		SDL0:14;
	UINT32		LS0:1;
	UINT32		DDONE:1;

	/* Word	2 */
	UINT32		SDP1;
} RXD_STRUC;
#endif /* RT_BIG_ENDIAN */

#ifdef RXD_WED_SCATTER_SUPPORT
#define MAX_RECORD 32
typedef	struct _RXD_RECORD {
	/* Word 0 */
	UINT32		DW0;
	/* Word 1 */
	UINT32		DW1;
	/* Word 2 */
	UINT32		DW2;
	/* Word 3 */
	UINT32		DW3;
} RXD_RECORD;

struct rx_data {
	RXD_RECORD RxD_dump;
	UCHAR buffer[20];
};

struct rx_scatter_data {
	UINT scatter_cnt;
	UINT32 gather_size;
	UINT32 rx_bytes_cnt;
	UINT32 build_skb_len;
};
#endif /* RXD_WED_SCATTER_SUPPORT */

struct rxdmad_info {
	PNDIS_PACKET pkt;
	UINT16 ppe_entry;
	UINT8 csrn;
};

/*
	Rx descriptor debug format for Rx Rings
*/

#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXD_DEBUG_STRUC {
	/* Word 0 */
	UINT32		SDP0;

	/* Word 1 */
	UINT32		DDONE:1;
	UINT32		LS0:1;
	UINT32		SDL0:14;
	UINT32		BURST:1;
	UINT32		LS1:1;
	UINT32		QID:2;
	UINT32		DIDX:12;

	/* Word 2 */
	UINT32		SW_INFO:20;
	UINT32		CIDX:12;
} RXD_DEBUG_STRUC;
#else
typedef	struct GNU_PACKED _RXD_DEBUG_STRUC {
	/* Word	0 */
	UINT32		SDP0;

	/* Word	1 */
	UINT32		DIDX:12;
	UINT32		QID:2;
	UINT32		LS1:1;
	UINT32		BURST:1;
	UINT32		SDL0:14;
	UINT32		LS0:1;
	UINT32		DDONE:1;

	/* Word	2 */
	UINT32		CIDX:12;
	UINT32		SW_INFO:20;
} RXD_DEBUG_STRUC;
#endif /* RT_BIG_ENDIAN */

/* RXD DW1 */
#define RXDMAD_TO_HOST (1 << 8)
#define RXDMAD_RING_INFO (1 << 9)
#define RXDMAD_TO_HOST_A (1 << 12)
#define RXDMAD_RXD_ERR (1 << 13)
#define RXDMAD_RXD_DROP (1 << 14)
#define RXDMAD_M_DONE (1 << 15)
/* RXD DW2 */
/* White List of WO pre checked */
/* WO will toggle the bit[12] for scatter announce from WO */
#define RXDMAD_WO_INDICATE_SCATTER (1 << 8)

/* RXD DW3 */
#define RXDMAD_CS_STATUS_MASK (0x0f << 0)
#define RXDMAD_CS_STATUS_SHIFT 0
#define RXDMAD_CS_TYPE_MASK (0xf << 4)
#define RXDMAD_CS_TYPE_SHIFT 4
#define RXDMAD_C (1 << 8)
#define RXDMAD_F (1 << 9)
#define RXDMAD_UN (1 << 10)
#define RXDMAD_CSRN_MASK (0x1f << 11)
#define RXDMAD_CSRN_SHIFT 11
#define RXDMAD_PPE_ENTRY_MASK (0x7fff << 16)
#define RXDMAD_PPE_ENTRY_SHIFT 16
#define RXDMAD_PPE_VLD (1 << 31)

#define INC_INDEX(_idx, _RingSize)    \
	{                                          \
		(_idx) = (_idx+1) % (_RingSize);       \
	}

enum {
	TX_RING_LOW,
	TX_RING_HIGH,
};

enum {
	GET_PKT_DDONE,
	GET_PKT_IO,
	GET_PKT_METHOD_NUMS
};

enum buf_alloc_type {
	DYNAMIC_PAGE_ALLOC,
	DYNAMIC_SLAB_ALLOC,
	PRE_SLAB_ALLOC,
	DYNAMIC_PAGE_ALLOC_DEBUG,
	PKT_ALLOC_TYPE_NUMS
};

/**
 * avg_tp: average T.P
 * dly_number: delay number in unit of interrupt event
 * dly_time: delay time in unit of 20us
 */
struct dly_ctl_cfg {
	UINT32 avg_tp;
	UINT16 dly_number;
	UINT16 dly_time;
};

struct hif_pci_ring_bh_group {
	UINT32 int_mask; /* HW interrupt bitmask */
	UINT32 ring_num; /* number of ring that's served in the bh group */
	UINT8 ring_idx[32]; /* ring index (inside struct pci_hif_chip) of the rings in the bh group */
};

struct hif_pci_tx_ring_desc {
	UINT32 hw_desc_base;
	UINT32 hw_int_mask;
	UINT16 ring_size;
	enum resource_attr ring_attr;
	UINT8 band_idx;
	char *const ring_info;
};

#define MAX_DDONE_CHECK_TIMES 10

struct hif_pci_rx_ring_desc {
	UINT32 hw_desc_base;
	UINT32 hw_int_mask;
	UINT16 ring_size;
	enum resource_attr ring_attr;
	UINT32 event_type;
	BOOLEAN delay_int_en;
	struct dly_ctl_cfg *dl_dly_ctl_tbl;
	UINT32 dl_dly_ctl_tbl_size;
	struct dly_ctl_cfg *ul_dly_ctl_tbl;
	UINT32 ul_dly_ctl_tbl_size;
	UINT16 max_rx_process_cnt;
	UINT16 max_sw_read_idx_inc;
	UINT8 buf_type;
	UINT8 band_idx;
	char *const ring_info;
};

struct hif_pci_ring_layout {
	const struct hif_pci_tx_ring_desc *tx_ring_layout;
	const struct hif_pci_rx_ring_desc *rx_ring_layout;
};

struct hif_pci_tx_ring {
	enum resource_attr ring_attr;
	UINT32 hw_didx_addr;
	UINT32 TxDmaIdx;
	RTMP_DMACB *Cell;
	UINT32 TxSwFreeIdx;
	ULONG tx_ring_state;
	UINT32 tx_ring_low_water_mark;
	UINT32 tx_ring_high_water_mark;
	UINT32 tx_ring_full_cnt;
	UINT32 hw_cidx_addr;
	UINT32 TxCpuIdx;
	UINT32 hw_desc_base;
	UINT32 hw_cnt_addr;
	NDIS_SPIN_LOCK tx_lock;
	NDIS_SPIN_LOCK tx_done_lock;
	RTMP_DMABUF desc_ring;
	RTMP_DMABUF buf_space;
	UINT32 hw_int_mask;
	UINT8 resource_idx;
	UINT8 band_idx;
	UINT16 ring_size;
#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	UINT8 cur_txd_cnt;
#endif
} ____cacheline_aligned;

struct hif_pci_rx_ring {
	enum resource_attr ring_attr;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 3, 0)
	struct page_frag_cache rx_page;
#endif
	UINT16 max_rx_process_cnt;
	UINT32 hw_didx_addr;
	UINT32 RxDmaIdx;
	UINT32 RxSwReadIdx;
	UINT16 ring_size;
	RTMP_DMACB *Cell;
	UINT8 resource_idx;
	UINT8 buf_type;
	UINT8 buf_flags;
	UINT8 get_pkt_method;
	UINT16 free_buf_head;
	UINT16 free_buf_tail;
	UINT16 free_buf_size;
	RTMP_DMABUF *free_buf;
	UINT16 cur_free_buf_len;
	UINT16 RxBufferSize;
	UINT16 sw_read_idx_inc;
	UINT32 RxCpuIdx;
	UINT8 band_idx;
	UINT32 hw_cidx_addr;
	UINT16 free_buf_64k_head;
	UINT16 free_buf_64k_tail;
	UINT16 free_buf_64k_size;
	RTMP_DMABUF *free_buf_64k;
	UINT32 hw_desc_base;
	UINT32 hw_cnt_addr;
	NDIS_SPIN_LOCK ring_lock;
	RTMP_DMABUF desc_ring;
	UINT32 hw_int_mask;
	UINT8 buf_debug;
	UINT16 max_sw_read_idx_inc;
	BOOLEAN delay_int_en;
	struct dly_ctl_cfg *dl_dly_ctl_tbl;
	UINT32 dl_dly_ctl_tbl_size;
	struct dly_ctl_cfg *ul_dly_ctl_tbl;
	UINT32 ul_dly_ctl_tbl_size;
	UINT32 event_type;
} ____cacheline_aligned;

enum {
	FREE_BUF_1k,
	FREE_BUF_64k,
};

typedef struct _PCI_HIF_T {
	UINT8 tx_res_num;
	UINT8 rx_res_num;
	struct hif_pci_rx_ring **rx_ring;
	struct hif_pci_tx_ring **tx_ring;
	BOOLEAN (*dma_done_handle[RING_ATTR_NUM])(
							struct _RTMP_ADAPTER *pAd,
							UINT8 resource_idx);
	/* map swq to SW TxRing resource_idx */
	UINT8 swq_to_tx_ring[DBDC_BAND_NUM][PACKET_TYPE_NUM][WMM_QUE_NUM];
	PNDIS_PACKET (*get_pkt_from_rx_resource[PKT_ALLOC_TYPE_NUMS][GET_PKT_METHOD_NUMS])(
												struct _RTMP_ADAPTER *pAd,
												BOOLEAN *pbReschedule,
												UINT32 *pRxPending,
												UCHAR resource_idx);
	/* PCI MMIO Base Address, all access will use */
	PUINT8	CSRBaseAddress;

#ifdef CONFIG_ANDES_SUPPORT
	/* remain these rings as pointers for easier back-ward compatible.*/
	struct hif_pci_tx_ring *ctrl_ring;
	struct hif_pci_tx_ring *fwdl_ring;
#endif /* CONFIG_ANDES_SUPPORT */

	/* flag that indicate if the PICE power status in configuration space.. */
	BOOLEAN bPCIclkOff;
#ifdef CUT_THROUGH
	VOID *PktTokenCb;
#endif /* CUT_THROUGH */
	UINT16 host_msdu_id_rpt_idx;
	/* lock for IO remap HW */
	NDIS_SPIN_LOCK io_remap_lock;
	struct pci_hif_chip *main_hif_chip;
	struct pci_hif_chip *slave_hif_chip;
	UINT8 pci_hif_chip_num;
	struct pci_hif_chip **pci_hif_chip;
	VOID *net_dev; /* for suspend, resume, get pAd */
} PCI_HIF_T, *PPCI_HIF_T;

struct pci_task_group {
	RTMP_NET_TASK_STRUCT tx_dma_done_task;
	RTMP_NET_TASK_STRUCT rx_data_done_task;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	RTMP_NET_TASK_STRUCT rx_data_done_task_msi_band0;
	RTMP_NET_TASK_STRUCT rx_data_done_task_msi_band1;
#endif
	RTMP_NET_TASK_STRUCT rx_event_done_task;
	RTMP_NET_TASK_STRUCT rx_dly_done_task;
#ifdef ERR_RECOVERY
	RTMP_NET_TASK_STRUCT mac_error_recovey_task;
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	RTMP_NET_TASK_STRUCT mac_fw_own_task;
#endif
	RTMP_NET_TASK_STRUCT subsys_int_task;
	RTMP_NET_TASK_STRUCT sw_int_task;
#ifdef WF_RESET_SUPPORT
	RTMP_NET_TASK_STRUCT wf_reset_task;
#endif
	struct net_device napi_dev;
	struct napi_struct rx_data_done_napi_task;
	VOID *priv;
};

struct pci_schedule_task_ops {
	INT(*schedule_tx_dma_done)(struct pci_task_group *group);
	INT(*schedule_rx_data_done)(struct pci_task_group *group);
#ifdef CONFIG_WIFI_MSI_SUPPORT
	INT(*schedule_rx_data_done_msi)(struct pci_task_group *group, UINT8 resource_idx);
#endif
	INT(*schedule_rx_event_done)(struct pci_task_group *group);
	INT(*schedule_rx_dly_done)(struct pci_task_group *group);
#ifdef ERR_RECOVERY
	INT(*schedule_mac_recovery)(struct pci_task_group *group);
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	INT(*schedule_mac_fw_own)(struct pci_task_group *group);
#endif
	INT(*schedule_subsys_int)(struct pci_task_group *group);
	INT(*schedule_sw_int)(struct pci_task_group *group);
#ifdef WF_RESET_SUPPORT
	INT(*schedule_wf_reset)(struct pci_task_group *group);
#endif
};

struct pci_hif_chip_cfg {
	VOID *device;
	UINT32 device_id;
	UINT32 irq;
#ifdef MULTI_INTR_SUPPORT
	UINT32 multi_intr_2nd;
	UINT32 multi_intr_3rd;
	UINT32 multi_intr_4th;
#endif
	ULONG csr_addr;
#ifdef INTERFACE_SPEED_DETECT
	UINT32 IfaceSpeed;
#endif
	BOOLEAN msi_en;
};

struct pci_hif_chip {
	UINT32 int_enable_mask ____cacheline_aligned;
	UINT32 int_ena_reg_addr;
	UINT32 intDisableMask;
	UINT32 IntPending;
	UINT8 tx_res_num;
	UINT8 rx_res_num;
	struct hif_pci_rx_ring *RxRing;
	struct hif_pci_tx_ring *TxRing;
	struct hif_pci_ring_bh_group tx_bh_group;
	struct hif_pci_ring_bh_group rx_bh_group;
	struct hif_pci_ring_bh_group rx_data_bh_group;
	struct hif_pci_ring_bh_group rx_event_bh_group;
	/* PCI MMIO Base Address, all access will use */
	PUINT8	CSRBaseAddress;
	NDIS_SPIN_LOCK LockInterrupt;
	struct hif_pci_ring_layout ring_layout;
#ifdef CONFIG_WIFI_MSI_SUPPORT
	BOOLEAN is_msi;
	BOOLEAN is_main;
	UINT32 first_irq;
	VOID (*msi_isr_message3)(struct pci_hif_chip *hif_chip);
	VOID (*msi_isr_message4)(struct pci_hif_chip *hif_chip);
	VOID (*msi_isr_message2_pcie1)(struct pci_hif_chip *hif_chip);
#endif
	VOID *pdev; /* pointer to struct device for mem alloc */
	struct _PCI_HIF_T *hif;

	/* irq */
	UINT32 irq;
#ifdef MULTI_INTR_SUPPORT
	BOOLEAN is_multi_intr;
	UINT32 multi_irq_2nd;
	UINT32 multi_irq_3rd;
	UINT32 multi_irq_4th;
#endif

	VOID (*isr)(struct pci_hif_chip *hif_chip);
#ifdef MULTI_INTR_SUPPORT
	VOID (*multi_isr)(struct pci_hif_chip *hif_chip);
	VOID (*multi_isr_2nd)(struct pci_hif_chip *hif_chip);
	VOID (*multi_isr_3rd)(struct pci_hif_chip *hif_chip);
	VOID (*multi_isr_4th)(struct pci_hif_chip *hif_chip);
#endif

#if defined(MT7986) || defined(MT7916) || defined(MT7981)
#ifdef RTMP_PCI_SUPPORT
	VOID (*isr_handler)(struct pci_hif_chip *hif_chip);
#endif /* RTMP_PCI_SUPPORT */
	struct pci_hif_chip_cfg cfg;
#endif /* MT7986 || MT7916 || MT7981*/

	/* task management */
	struct pci_task_group task_group;
	struct pci_schedule_task_ops *schedule_task_ops;
	UINT32 tx_dma_1st_buffer_size;
#if defined(CTXD_SCATTER_AND_GATHER) || defined(CTXD_MEM_CPY)
	UINT8 max_ctxd_agg_num;
#endif
#ifdef CTXD_MEM_CPY
	UINT8 ctxd_size_unit;
	UINT8 ct_partial_payload_offset;
#endif
};

USHORT mtd_pci_write_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber);

#ifdef CTXD_MEM_CPY
USHORT mtd_pci_write_tx_resource_for_ctxd(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber);

VOID mtd_pci_write_last_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	UCHAR resource_idx);
#endif

#ifdef CTXD_SCATTER_AND_GATHER
VOID mtd_pci_write_last_tx_resource_last_sec(
	struct _RTMP_ADAPTER *pAd,
	UCHAR resource_idx);
#endif

USHORT mt_pci_write_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	BOOLEAN bIsLast,
	USHORT *FreeNumber);

VOID mt_pci_write_final_tx_resource(
	struct _RTMP_ADAPTER *pAd,
	struct _TX_BLK *pTxBlk,
	USHORT totalMPDUSize,
	USHORT FirstTxIdx);

UINT32 pci_dynamic_dly_int_init(
	void *hdev_ctrl
	);

UINT32 pci_dynamic_dly_int_adjust(
	void *hdev_ctrl,
	UINT32 tx_tp_mpbs,
	UINT32 rx_tp_mbps
	);

VOID mt_int_disable(struct _RTMP_ADAPTER *pAd, struct pci_hif_chip *hif_chip, unsigned int mode);
VOID mt_int_enable(struct _RTMP_ADAPTER *pAd, struct pci_hif_chip *hif_chip, unsigned int mode);

VOID pci_core_ops_register(void *hdev_ctrl);
VOID pci_core_ops_unregister(void *hdev_ctrl);

static inline struct hif_pci_rx_ring* pci_get_rx_ring_by_ridx(struct _PCI_HIF_T * pci_hif, UINT8 resource_idx)
{
	return pci_hif->rx_ring[resource_idx];
}

static inline struct hif_pci_tx_ring* pci_get_tx_ring_by_ridx(struct _PCI_HIF_T * pci_hif, UINT8 resource_idx)
{
	return pci_hif->tx_ring[resource_idx];
}

#ifdef CONFIG_WIFI_MSI_SUPPORT
VOID pci_handle_msi_irq(int irq, void *hif_chip);
#endif

VOID pci_handle_irq(void *hif_chip);
#ifdef MULTI_INTR_SUPPORT
VOID pci_handle_multi_irq(void *hif_chip);
VOID pci_handle_multi_irq_2nd(void *hif_chip);
VOID pci_handle_multi_irq_3rd(void *hif_chip);
VOID pci_handle_multi_irq_4th(void *hif_chip);
#endif

NDIS_STATUS pci_hif_chip_init(VOID **hif_chip, struct pci_hif_chip_cfg *cfg);
VOID pci_hif_chip_exit(struct pci_hif_chip *hif_chip);
BOOLEAN pci_rx_event_dma_done_handle(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
VOID pci_rx_all(struct _PCI_HIF_T *pci_hif);

#endif /* __MT_HIF_PCI_H__ */

