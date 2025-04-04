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
/***************************************************************************
 ***************************************************************************

*/
#ifndef ___TOKEN_H__
#define ___TOKEN_H__

#include "rtmp_type.h"
#include "rtmp_os.h"
#include "rtmp_timer.h"

typedef enum _CT_MSDU_INFO_FLAG {
	CT_INFO_APPLY_TXD = BIT(0),
	HIF_PKT_FLAGS_COPY_HOST_TXD_ALL = BIT(1),
	CT_INFO_MGN_FRAME = BIT(2),
	CT_INFO_NONE_CIPHER_FRAME = BIT(3),
	CT_INFO_HSR2_TX = BIT(4),
	CT_INFO_PTK_NO_ACK = BIT(5),
	CT_INFO_PKT_FR_HOST = BIT(7),
	CT_INFO_MCAST_CLONE = BIT(8),
	CT_INFO_RTS_ENABLE = BIT(9),
} CT_MSDU_INFO_FLAG;


typedef struct GNU_PACKED _CR4_TXP_MSDU_INFO {
#define MAX_BUF_NUM_PER_PKT 6
	UINT16 type_and_flags;
	UINT16 msdu_token;
	UINT8 bss_index;
	/* #256STA - Low Byte, if not rept/wds(dmac) entry, leave to 0xff. 2015-June3 discussion. */
	/* #256STA - Low Byte, if not rept/wds/sta/apcli(fmac) entry, leave to 0xff. 2020/4/23 discussion. */
	UINT8 rept_wds_wcid;
	UINT8 reserved; /* #256STA - High Byte and Version */
	UINT8 buf_num;
	UINT32 buf_ptr[MAX_BUF_NUM_PER_PKT];
	UINT16 buf_len[MAX_BUF_NUM_PER_PKT];
} CR4_TXP_MSDU_INFO;


#define CUT_THROUGH_TYPE_TX 1
#ifdef MEMORY_OPTIMIZATION
#define DEFAUT_PKT_TX_TOKEN_ID_MAX 2047
#else
#define DEFAUT_PKT_TX_TOKEN_ID_MAX 8191 /* token ID in range of 0~4095 */
#endif
#ifdef WHNAT_SUPPORT
#define DEFAUT_WHNAT_PKT_TX_TOKEN_ID_MAX 1023 /* token ID in range of 0~2047 for SW path */
#endif

enum {
	TOKEN_NONE,
	TOKEN_TX_DATA,
	TOKEN_TX_MGT,
};


struct token_tx_pkt_entry {
	PNDIS_PACKET pkt_buf;
	NDIS_PHYSICAL_ADDRESS pkt_phy_addr;
	size_t pkt_len;
	UINT16 wcid;
	UINT8 Type;
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_PROXY_ARP)
	BOOLEAN Reprocessed;
#endif /* CONFIG_HOTSPOT_R2 */
} ____cacheline_aligned;

#define TX_FREE_NOTIFY_DEEP_STAT_SIZE 12

struct tx_free_notify_deep_stat {
	UINT16 boundary;
	UINT32 cnt;
};

/*
    queue operation behavior:
    1. If id_head != id_tail
	has free token
    2. if id_head == id_tail
	empty and no free token
*/
struct token_tx_pkt_queue {
	BOOLEAN token_inited;
	UINT8 band_idx;
	NDIS_SPIN_LOCK enq_lock;
	NDIS_SPIN_LOCK deq_lock;
	INT16 id_head; /* Index for first use-able token in free_id[] */
	INT16 id_tail; /* Index for first free_id[] to store recycled token */
	UINT32 pkt_tkid_end;
	UINT32 pkt_tkid_start;
	UINT32 pkt_tkid_invalid;
	UINT32 pkt_tkid_cnt;
	UINT32 pkt_tkid_aray;
	UINT16 *free_id;
	struct token_tx_pkt_entry *pkt_token;
	UINT32 low_water_mark;
	UINT32 high_water_mark;
	ULONG token_state;
	UINT32 token_full_cnt;
	atomic_t free_token_cnt;
	UINT32 total_enq_cnt;
	UINT32 total_deq_cnt;
	UINT32 total_back_cnt;
	UINT32 high_water_mark_per_band[2];
	UINT32 token_full_cnt_per_band[2];
	atomic_t used_token_per_band[2];
	struct tx_free_notify_deep_stat deep_stat[TX_FREE_NOTIFY_DEEP_STAT_SIZE];
};

struct token_rx_pkt_entry {
	PNDIS_PACKET pkt_buf;
	RTMP_DMABUF dma_buf;
}  ____cacheline_aligned;

struct token_rx_pkt_queue {
	struct token_rx_pkt_entry *pkt_token;
	atomic_t cur_free_idx;
	UINT32 pkt_tkid_cnt;
};

enum {
	NO_ENOUGH_FREE_TX_TOKEN = (1 << 0),
	NO_ENOUGH_FREE_TX_RING = (1 << 1),
};

enum {
	TX_TOKEN_LOW,
	TX_TOKEN_HIGH,
};

enum {
	TX_TOKEN_HIGH_TO_HIGH,
	TX_TOKEN_HIGH_TO_LOW,
	TX_TOKEN_LOW_TO_LOW,
	TX_TOKEN_LOW_TO_HIGH,
	TX_TOKEN_UNKNOW_CHANGE,
};


enum {
	TOKEN_LOWMARK,
	TOKEN_FREE_HIGHMARK,
	TOKEN_ENQ_HIGHMARK,
};

enum {
	TOKEN_WATERMARK,
	TOKEN_BOUNDARY,
	TOKEN_DEBUG,
};

enum {
	TOKEN_CNT_RECORD = (1 << 0),
};

typedef struct _PKT_TOKEN_CB {
	UINT8 que_nums;
	struct token_tx_pkt_queue que[2];
	struct token_rx_pkt_queue rx_que;
	VOID *pAd;
	UINT8 dbg;
} PKT_TOKEN_CB;

PNDIS_PACKET token_tx_deq(struct _RTMP_ADAPTER *pAd, struct token_tx_pkt_queue *que,
									UINT16 token, UINT8 *type);
UINT16 token_tx_enq(
	struct _RTMP_ADAPTER *pAd,
	struct token_tx_pkt_queue *que,
	PNDIS_PACKET pkt,
	UCHAR type,
	UINT16 wcid,
	NDIS_PHYSICAL_ADDRESS pkt_phy_addr,
	size_t pkt_len);

BOOLEAN token_tx_get_state(struct token_tx_pkt_queue *que);
INT token_tx_set_state(struct token_tx_pkt_queue *que, BOOLEAN state);
VOID token_tx_inc_full_cnt(struct token_tx_pkt_queue *que);
struct token_tx_pkt_queue *token_tx_get_queue_by_band(struct _PKT_TOKEN_CB *cb, UINT32 band_idx);
struct token_tx_pkt_queue *token_tx_get_queue_by_token_id(PKT_TOKEN_CB *cb, UINT32 token_id);
UINT32 cut_through_check_token_state(struct token_tx_pkt_queue *que);
UINT32 token_tx_get_free_cnt(struct token_tx_pkt_queue *que);
UINT32 token_tx_get_lwmark(struct token_tx_pkt_queue *que);
UINT32 token_tx_get_hwmark(struct token_tx_pkt_queue *que);
VOID token_tx_inc_full_cnt_per_band(struct token_tx_pkt_queue *que, UINT32 band_idx);
UINT32 token_tx_get_used_cnt_per_band(struct token_tx_pkt_queue *que, UINT32 band_idx);
UINT32 token_tx_get_hwmark_per_band(struct token_tx_pkt_queue *que, UINT32 band_idx);
VOID token_tx_set_lwmark(struct token_tx_pkt_queue *que, UINT32 value);
VOID token_tx_set_hwmark(struct token_tx_pkt_queue *que, UINT32 value);
VOID token_tx_record_free_notify(struct token_tx_pkt_queue *que, UINT32 token_cnt);
INT token_tx_setting(struct _RTMP_ADAPTER *pAd, UINT8 q_idx, INT32 option, INT32 sub_option, INT32 value);
UINT32 token_rx_dmad_init(struct token_rx_pkt_queue *que, PNDIS_PACKET pkt,
								ULONG alloc_size, PVOID alloc_va, NDIS_PHYSICAL_ADDRESS alloc_pa);
INT token_rx_dmad_lookup(struct token_rx_pkt_queue *que, UINT32 token_id, PNDIS_PACKET *pkt,
							PVOID *alloc_va, NDIS_PHYSICAL_ADDRESS *alloc_pa);
INT token_rx_dmad_lookup_pa(struct token_rx_pkt_queue *que, UINT32 *token_id, PNDIS_PACKET *pkt,
							PVOID *alloc_va, NDIS_PHYSICAL_ADDRESS alloc_pa);
VOID token_rx_dmad_pool_dump(struct token_rx_pkt_queue *que);

INT token_rx_dmad_update(struct token_rx_pkt_queue *que, UINT32 token_id, PNDIS_PACKET pkt,
								ULONG alloc_size, PVOID alloc_va, NDIS_PHYSICAL_ADDRESS alloc_pa);
INT token_deinit(PKT_TOKEN_CB **ppktTokenCb);
INT token_init(VOID **ppktTokenCb, VOID *pAd);
#endif

