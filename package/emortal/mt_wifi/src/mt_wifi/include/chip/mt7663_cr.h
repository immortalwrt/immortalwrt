#ifndef __MT7663_CR_H__
#define __MT7663_CR_H__
/*
	Please don't include this header outside of per chip scope
*/

#define MT_HIF_BASE			0x4000

/*
	CR CONN_HIF_ON_LPCTL
*/
#define MT_CONN_HIF_ON_LPCTL	(MT_HIF_HOST_CSR_BASE + 0x0000)
#define MT_HOST_SET_OWN	(1<<0)
#define MT_HOST_CLR_OWN	(1<<1)

/*
	CR CONN_HIF_RST
*/
#define MT_CONN_HIF_RST			(MT_HIF_BASE + 0x0100)
#define CONN_HIF_LOGIC_RST_N	(1 << 4)
#define DMASHDL_ALL_RST_N		(1 << 5)
/*
	CR CONN_HIF_BUSY_STATUS
*/
#define MT_CONN_HIF_BUSY_STATUS	(MT_HIF_BASE + 0x0138)
#define CONN_HIF_BUSY			(1 << 31)

/*
	CR MCU2HOST_SW_INT_STA/ENA
*/
#define MT_MCU2HOST_SW_INT_STA	(MT_HIF_BASE + 0x01f0)
#define MT_MCU2HOST_SW_INT_ENA	(MT_HIF_BASE + 0x01f4)

/*
	CR SUBSYS2HOST_INT_STA/ENA
*/
#define MT_SUBSYS2HOST_INT_STA	(MT_HIF_BASE + 0x01F8)
#define MT_SUBSYS2HOST_INT_ENA	(MT_HIF_BASE + 0x01FC)
#define CONN_HIF_ON_HOST_INT	(1<<8)

/*
	CR CONN_HIF_ON_IRQ_STAT/ENA
*/
#define MT_HIF_HOST_CSR_BASE	(0x7000)
#define MT_CONN_HIF_ON_IRQ_STAT	(MT_HIF_HOST_CSR_BASE + 0x0004)
#define MT_CONN_HIF_ON_IRQ_ENA	(MT_HIF_HOST_CSR_BASE + 0x0008)
#define HOST_OWN_INT			(1<<0)

#define MT7663_MCU_INT_EVENT     (MT_HIF_BASE + 0x0108)
#define MT_INT_SOURCE_CSR	(MT_HIF_BASE + 0x0200)
#define MT_INT_MASK_CSR	(MT_HIF_BASE + 0x0204)

/*
	remap CR
*/
#define HIF_ADDR_REMAP_ADDR	0x7010
#define HIF_ADDR_REMAP_MASK	0xFFFF0000
#define HIF_ADDR_REMAP_SHFT	16
#define HIF_ADDR_REMAP_BASE_ADDR	0xF0000
#define REMAP_OFFSET_MASK (0xffff)
#define GET_REMAP_OFFSET(p) (((p) & REMAP_OFFSET_MASK))
#define REMAP_BASE_MASK (0xffff << 16)
#define GET_REMAP_BASE(p) (((p) & REMAP_BASE_MASK) >> 16)

/*
	WPDMA_GLO_CFG
*/
#define MT_WPDMA_GLO_CFG	    (MT_HIF_BASE + 0x0208)

#define MULTI_DMA_EN_DISABLE			0
#define MULTI_DMA_EN_FEATURE_1			1
#define MULTI_DMA_EN_FEATURE_2			2
#define MULTI_DMA_EN_FEATURE_2_PREFETCH		3

/*
	PCIe master interrupt switch
*/
#define MT_PCIE_MAC_BASE_ADDR   (0xF0000)
#define MT_PCIE_MAC_INT_ENABLE_ADDR   (MT_PCIE_MAC_BASE_ADDR + 0x188)
#define MT_PCIE_MAC_INT_STATUS_ADDR   (MT_PCIE_MAC_BASE_ADDR + 0x18C)

#ifdef RT_BIG_ENDIAN
typedef	union _WPDMA_GLO_CFG_STRUC	{
	struct {
		UINT32 rx_2b_offset:1;
		UINT32 clk_gate_dis:1;
		UINT32 byte_swap:1;
		UINT32 omit_tx_info:1;
		UINT32 omit_rx_info:1;
		UINT32 pdma_addr_ext_en:1;
		UINT32 force_tx_eof:1;
		UINT32 rsv0:3;
		UINT32 trx_pfet_arb_mi_depth:3;
		UINT32 trx_dfet_arb_mi_depth:3;
		UINT32 trx_pfet_dfet_mi_depth:3;
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
	} MT7663_field;
	UINT32 word;
} WPDMA_GLO_CFG_STRUC;
#else
typedef	union _WPDMA_GLO_CFG_STRUC	 {
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
		UINT32 trx_pfet_dfet_mi_depth:3;
		UINT32 trx_dfet_arb_mi_depth:3;
		UINT32 trx_pfet_arb_mi_depth:3;
		UINT32 rsv0:3;
		UINT32 force_tx_eof:1;
		UINT32 pdma_addr_ext_en:1;
		UINT32 omit_rx_info:1;
		UINT32 omit_tx_info:1;
		UINT32 byte_swap:1;
		UINT32 clk_gate_dis:1;
		UINT32 rx_2b_offset:1;
	} MT7663_field;
	UINT32 word;
} WPDMA_GLO_CFG_STRUC;
#endif /* RT_BIG_ENDIAN */

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

#define CONN_HIF_PDMA_CSR_BASE		0x4000

#define CONN_HIF_PDMA_TX_RING0_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x300)
#define CONN_HIF_PDMA_TX_RING1_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x310)
#define CONN_HIF_PDMA_TX_RING2_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x320)
#define CONN_HIF_PDMA_TX_RING3_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x330)
#define CONN_HIF_PDMA_TX_RING4_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x340)
#define CONN_HIF_PDMA_TX_RING5_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x350)
#ifdef RANDOM_PKT_GEN
#define CONN_HIF_PDMA_TX_RING6_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x360)
#define CONN_HIF_PDMA_TX_RING7_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x370)
#define CONN_HIF_PDMA_TX_RING8_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x380)
#define CONN_HIF_PDMA_TX_RING9_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x390)
#define CONN_HIF_PDMA_TX_RING10_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x3a0)
#define CONN_HIF_PDMA_TX_RING11_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x3b0)
#define CONN_HIF_PDMA_TX_RING12_BASE		(CONN_HIF_PDMA_CSR_BASE + 0x3c0)
#endif
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

#define MT_INT_SUBSYS_INT_STS				(1<<28)
#define MT_INT_MCU2HOST_SW_INT_STS			(1<<29)

#define MT_INT_TX_DONE (MT_INT_T0_DONE | MT_INT_T1_DONE | MT_INT_T2_DONE | MT_INT_T3_DONE |\
			MT_INT_T4_DONE | MT_INT_T5_DONE | MT_INT_T6_DONE | MT_INT_T7_DONE |\
			MT_INT_T8_DONE | MT_INT_T9_DONE | MT_INT_T10_DONE | MT_INT_T11_DONE |\
			MT_INT_T12_DONE | MT_INT_T13_DONE | MT_INT_T14_DONE | MT_INT_T15_DONE)

#define MT_TxCoherent		MT_INT_TX_COHE
#define MT_RxCoherent		MT_INT_RX_COHE
#define MT_CoherentInt		(MT_INT_RX_COHE | MT_INT_TX_COHE)

#define MT_INT_RX			(MT_INT_R0_DONE | MT_INT_R1_DONE)
#define MT_INT_RX_DATA		(MT_INT_R0_DONE)
#define MT_INT_RX_CMD		(MT_INT_R1_DONE)

#define MT_INT_CMD			(MT_INT_T5_DONE)



#endif /* __MT7663_CR_H__ */
