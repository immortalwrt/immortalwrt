#ifndef __MT7622_CR_H__
#define __MT7622_CR_H__
/*
	Please don't include this header outside of per chip scope
*/

#define MT_HIF_BASE			0x4000

#define MT_HOST_SET_OWN	(1<<0)
#define MT_HOST_CLR_OWN	(1<<1)

/*
	CR HIF_SYS_SW_RST
	this CR can't be accessd directly, needs remap.
*/
#define HIF_REG_BASE			0x50000000
#define MT_HIF_SYS_SW_RST	    (HIF_REG_BASE + 0x4034)
#define HIF_DATA_PATH_RESET_N	(1 << 1)

/*
	WPDMA_GLO_CFG
*/
#define MT_WPDMA_GLO_CFG	    (MT_HIF_BASE + 0x0208)

#define MULTI_DMA_EN_DISABLE			0
#define MULTI_DMA_EN_FEATURE_1			1
#define MULTI_DMA_EN_FEATURE_2			2
#define MULTI_DMA_EN_FEATURE_2_PREFETCH		3

#ifdef RT_BIG_ENDIAN
	typedef union _WPDMA_GLO_CFG_STRUC	{
		struct {
			UINT32 rx_2b_offset:1;
			UINT32 clk_gate_dis:1;
			UINT32 byte_swap:1;
			UINT32 omit_tx_info:1;
			UINT32 omit_rx_info:1;
			UINT32 first_token_only:1;
			UINT32 force_tx_eof:1;
			UINT32 sw_rst:1;
			UINT32 rsv0:2;
			UINT32 mi_depth_rd_8_6:3;
			UINT32 mi_depth_rd_5_3:3;
			UINT32 mi_depth_rd_2_0:3;
			UINT32 fifo_little_endian:1;
			UINT32 multi_dma_en:2;
			UINT32 fw_ring_bp_tx_sch:1;
			UINT32 Desc32BEn:1;
			UINT32 BigEndian:1;
			UINT32 EnTXWriteBackDDONE:1;
			UINT32 WPDMABurstSIZE:2;
			UINT32 RxDMABusy:1;
			UINT32 EnableRxDMA:1;
			UINT32 TxDMABusy:1;
			UINT32 EnableTxDMA:1;
		} MT7622_field;
		UINT32 word;
	} WPDMA_GLO_CFG_STRUC;
#else
	typedef union _WPDMA_GLO_CFG_STRUC	{
		struct {
			UINT32 EnableTxDMA:1;
			UINT32 TxDMABusy:1;
			UINT32 EnableRxDMA:1;
			UINT32 RxDMABusy:1;
			UINT32 WPDMABurstSIZE:2;
			UINT32 EnTXWriteBackDDONE:1;
			UINT32 BigEndian:1;
			UINT32 Desc32BEn:1;
			UINT32 fw_ring_bp_tx_sch:1;
			UINT32 multi_dma_en:2;
			UINT32 fifo_little_endian:1;
			UINT32 mi_depth_rd_2_0:3;
			UINT32 mi_depth_rd_5_3:3;
			UINT32 mi_depth_rd_8_6:3;
			UINT32 rsv0:2;
			UINT32 sw_rst:1;
			UINT32 force_tx_eof:1;
			UINT32 first_token_only:1;
			UINT32 omit_rx_info:1;
			UINT32 omit_tx_info:1;
			UINT32 byte_swap:1;
			UINT32 clk_gate_dis:1;
			UINT32 rx_2b_offset:1;
		} MT7622_field;
		UINT32 word;
	} WPDMA_GLO_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */

#define MT_MCU_CMD_CSR	(MT_HIF_BASE + 0x0234)
#define MT_MCU_INT_EVENT         (MT_HIF_BASE + 0x01f8)
#define MT_INT_SOURCE_CSR	(MT_HIF_BASE + 0x0200)
#define MT_INT_MASK_CSR	(MT_HIF_BASE + 0x0204)

#define MT_CFG_LPCR_HOST	(MT_HIF_BASE + 0x01f0)

/*
	Delay interrupt configuration CR
	[0:7]=RX_MAX_PTIME (unit=20us), [8:14]=RX_MAX_PINT, [15]=RX_DLY_INT_EN
*/
#define MT_DELAY_INT_CFG	(MT_HIF_BASE + 0x0210)
#define RX_DLY_INT_CFG  (0x811c)

/*
	WPDMA_RST_PTR
*/
#define WPDMA_RST_PTR		(MT_HIF_BASE + 0x020c)

/*
	remap CR
*/
#define MCU_PCIE_REMAP_1       (MCU_CFG_BASE + 0x500)
#define REMAP_1_OFFSET_MASK (0x3ffff)
#define GET_REMAP_1_OFFSET(p) (((p) & REMAP_1_OFFSET_MASK))
#define REMAP_1_BASE_MASK		(0x3fff << 18)
#define GET_REMAP_1_BASE(p) (((p) & REMAP_1_BASE_MASK) >> 18)
#define MCU_PCIE_REMAP_2		(MCU_CFG_BASE + 0x504)
#define REMAP_2_OFFSET_MASK (0x7ffff)
#define GET_REMAP_2_OFFSET(p) (((p) & REMAP_2_OFFSET_MASK))
#define REMAP_2_BASE_MASK (0x1fff << 19)
#define GET_REMAP_2_BASE(p) (((p) & REMAP_2_BASE_MASK) >> 19)
#define MT_PCI_REMAP_ADDR_1            0x40000
#define MT_PCI_REMAP_ADDR_2            0x80000

#define CONN_HIF_PDMA_CSR_BASE		0x4000

#define CONN_HIF_PDMA_TX_RING0_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x300)
#define CONN_HIF_PDMA_TX_RING1_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x310)
#define CONN_HIF_PDMA_TX_RING2_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x320)
#define CONN_HIF_PDMA_TX_RING3_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x330)
#define CONN_HIF_PDMA_TX_RING4_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x340)
#define CONN_HIF_PDMA_TX_RING5_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x350)
#define CONN_HIF_PDMA_TX_RING15_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x3f0)

#define CONN_HIF_PDMA_RX_RING0_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x400)
#define CONN_HIF_PDMA_RX_RING1_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x410)

/* interrupt bit-shift definition */
#define MT_INT_R0_DONE		(1<<0)
#define MT_INT_R1_DONE		(1<<1)

#define MT_INT_T0_DONE		(1<<4)
#define MT_INT_T1_DONE		(1<<5)
#define MT_INT_T2_DONE		(1<<6)
#define MT_INT_T3_DONE		(1<<7)
#define MT_INT_T4_DONE		(1<<8)
#define MT_INT_T5_DONE		(1<<9)
#define MT_INT_T6_DONE		(1<<10)
#define MT_INT_T7_DONE		(1<<11)
#define MT_INT_T8_DONE		(1<<12)
#define MT_INT_T9_DONE		(1<<13)
#define MT_INT_T10_DONE		(1<<14)
#define MT_INT_T11_DONE		(1<<15)
#define MT_INT_T12_DONE		(1<<16)
#define MT_INT_T13_DONE		(1<<17)
#define MT_INT_T14_DONE		(1<<18)
#define MT_INT_T15_DONE		(1<<19)


#define MT_INT_RX_COHE		(1<<20)
#define MT_INT_TX_COHE		(1<<21)

#define MT_INT_RX_DLY		(1<<22)
#define MT_INT_TX_DLY		(1<<23)

#define WF_MAC_INT_0		(1<<24)
#define WF_MAC_INT_1		(1<<25)
#define WF_MAC_INT_2		(1<<26)
#define WF_MAC_INT_3		(1<<27)
#define WF_MAC_INT_4		(1<<28)
#define WF_MAC_INT_5		(1<<29)

#define MT_INT_MCU_CMD		(1<<30)
#define MT_FW_CLR_OWN_INT		(1<<31)

#define MT_INT_TX_DONE (MT_INT_T0_DONE | MT_INT_T1_DONE | MT_INT_T2_DONE | MT_INT_T3_DONE |\
			MT_INT_T4_DONE | MT_INT_T5_DONE | MT_INT_T6_DONE | MT_INT_T7_DONE |\
			MT_INT_T8_DONE | MT_INT_T9_DONE | MT_INT_T10_DONE | MT_INT_T11_DONE |\
			MT_INT_T12_DONE | MT_INT_T13_DONE | MT_INT_T14_DONE | MT_INT_T15_DONE)

#define MT_TxCoherent		MT_INT_TX_COHE
#define MT_RxCoherent		MT_INT_RX_COHE
#define MT_CoherentInt		(MT_INT_RX_COHE | MT_INT_TX_COHE)

#define MT_MacInt		(WF_MAC_INT_0 | WF_MAC_INT_1 | \
						 WF_MAC_INT_2 | WF_MAC_INT_3 | \
						 WF_MAC_INT_4 | WF_MAC_INT_5)

#define MT_INT_RX			(MT_INT_R0_DONE | MT_INT_R1_DONE)
#define MT_INT_RX_DATA		(MT_INT_R0_DONE)
#define MT_INT_RX_CMD		(MT_INT_R1_DONE)

#define MT_INT_CMD			(MT_INT_T5_DONE)

#endif /* __MT7622_CR_H__ */
