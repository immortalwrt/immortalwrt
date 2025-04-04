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
	ate_agent.h
*/

#ifndef __ATE_AGENT_H__
#define __ATE_AGENT_H__

struct _RTMP_ADAPTER;

INT32 SetTxStop(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetRxStop(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#ifdef DBG
VOID ATE_QA_Statistics(struct _RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI, RXINFO_STRUC *pRxInfo, PHEADER_802_11 pHeader);
#ifdef CONFIG_QA
INT32 RtmpDoAte(struct _RTMP_ADAPTER *pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, RTMP_STRING *wrq_name);
#endif
#ifdef WCX_SUPPORT
INT32 do_meta_cmd(int ioctl_cmd, PRTMP_ADAPTER	pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, RTMP_STRING *wrq_name);
#endif
INT32 SetEERead(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetEEWrite(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetBBPRead(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetBBPWrite(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetRFWrite(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#endif /* DBG */
VOID rt_ee_read_all(struct _RTMP_ADAPTER *pAd, UINT16 *Data);
VOID rt_ee_write_all(struct _RTMP_ADAPTER pAd, UINT16 *Data);
VOID rt_ee_write_bulk(struct _RTMP_ADAPTER pAd, UINT16 *Data, UINT16 offset, UINT16 length);
INT32 SetATEQid(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEDeqCnt(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxUser(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSDump(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSPhyMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSRate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSPath(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSPayloadLen(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSPktCnt(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSPwr(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSNss(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSPktBw(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEMPSStart(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATELOGEnable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATELOGDump(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxSEnable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxFilter(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxStream(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxStream(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEMACTRx(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATELOGDisable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEDa(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATESa(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEBssid(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEInitChan(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetADCDump(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPower0(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPower1(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPower2(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPower3(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEForceTxPower(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPowerEvaluation(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxAntenna(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxAntenna(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 Default_Set_ATE_TX_FREQ_OFFSET_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxFreqOffset(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 GetATETxFreqOffset(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 Default_Set_ATE_TX_BW_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxLength(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxCount(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxMcs(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxNss(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxLdpc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxStbc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxGi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATERxFer(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEReadRF(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATELoadE2p(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#ifdef RTMP_EFUSE_SUPPORT
INT32 SetATELoadE2pFromBuf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#endif /* RTMP_EFUSE_SUPPORT */
INT32 SetATEReadE2p(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEAutoAlc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxGi(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_max_pe(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_ru_info(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_ru_rx_aid(RTMP_ADAPTER *ad, RTMP_STRING *Arg);
INT32 set_ate_retry(RTMP_ADAPTER *ad, RTMP_STRING *Arg);
INT32 set_ate_tx_policy(RTMP_ADAPTER *ad, RTMP_STRING *Arg);
INT32 SetATETempSensor(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIpg(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEPayload(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEFixedPayload(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEAssocProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
INT32 SetATETxBfDutInitProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxBfGdInitProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPacketWithBf(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxBfChanProfileUpdate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxBfProfileRead(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETXBFProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxSoundingProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIBfGdCal(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIBfInstCal(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIBfProfileUpdate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEEBfProfileConfig(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIBfPhaseComp(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEIBfPhaseVerify(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxBfLnaGain(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxBfPhaseE2pUpdate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEConTxETxBfGdProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEConTxETxBfInitProc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATESpeIdx(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEEBfTx(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEEBFCE(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEEBFCEInfo(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEEBFCEHelp(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 MT_ATEComposePkt(struct _RTMP_ADAPTER *pAd, UCHAR *buf, UINT32 band_idx, UINT16 sta_idx);
#endif /* TXBF_SUPPORT && MT_MAC */
INT32 SetATETtr(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEShow(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 ShowATERUInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 ShowATETxDoneInfo(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 SetATEHelp(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 ATESampleRssi(struct _RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI);
VOID  ATEPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3);
INT32 SetATE(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEChannel(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_duty_cycle(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_pkt_tx_time(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_control_band_idx(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_show_rx_stat(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 set_ate_rx_stat_reset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#ifdef DOT11_VHT_AC
INT32 set_ate_channel_ext(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT32 set_ate_start_tx_ext(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* MT7615 */
INT32 SetATETxBw(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 MT_ATEInit(struct _RTMP_ADAPTER *pAd);
INT32 MT_ATEExit(struct _RTMP_ADAPTER *pAd);
PNDIS_PACKET ATEPayloadInit(struct _RTMP_ADAPTER *pAd, UINT32 TxIdx);
INT32 ATEPayloadAlloc(struct _RTMP_ADAPTER *pAd, UINT32 Index);
VOID EEReadAll(struct _RTMP_ADAPTER *pAd, UINT16 *Data, UINT32 size);
VOID rtmp_ate_init(struct _RTMP_ADAPTER *pAd);
VOID RTMPCfgTssiGainFromEEPROM(struct _RTMP_ADAPTER *pAd);
#ifdef SINGLE_SKU_V2
INT32 SetATESingleSKUEn(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
#endif /* SINGLE_SKU_V2 */
INT32 SetATEBFBackoffMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETempCompEn(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEPowerPercentEn(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEPowerPercentCtrl(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATEBFBackoffEn(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETSSIEn(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);
INT32 SetATETxPowerCtrlEn(struct _RTMP_ADAPTER *pAd, RTMP_STRING *Arg);

#endif
