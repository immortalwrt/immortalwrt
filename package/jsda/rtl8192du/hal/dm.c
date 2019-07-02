/******************************************************************************
 *
 * Copyright(c) 2007 - 2013 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#include <rtl8192d_hal.h>

#define u1Byte		u8
#define pu1Byte		u8*

#define u2Byte		u16
#define pu2Byte		u16*

#define u4Byte		u32
#define pu4Byte		u32*

#define u8Byte		u64
#define pu8Byte		u64*

#define s1Byte		s8
#define ps1Byte		s8*

#define s2Byte		s16
#define ps2Byte		s16*

#define s4Byte		s32
#define ps4Byte		s32*

#define s8Byte		s64
#define ps8Byte		s64*

typedef enum _ODM_RF_RADIO_PATH {
    ODM_RF_PATH_A = 0,   //Radio Path A
    ODM_RF_PATH_B = 1,   //Radio Path B
    ODM_RF_PATH_C = 2,   //Radio Path C
    ODM_RF_PATH_D = 3,   //Radio Path D
    ODM_RF_PATH_AB,
    ODM_RF_PATH_AC,
    ODM_RF_PATH_AD,
    ODM_RF_PATH_BC,
    ODM_RF_PATH_BD,
    ODM_RF_PATH_CD,
    ODM_RF_PATH_ABC,
    ODM_RF_PATH_ACD,
    ODM_RF_PATH_BCD,
    ODM_RF_PATH_ABCD,
  //  ODM_RF_PATH_MAX,    //Max RF number 90 support
} ODM_RF_RADIO_PATH_E, *PODM_RF_RADIO_PATH_E;

#define	ODM_AP			0x01	//BIT0
#define	ODM_ADSL		0x02	//BIT1
#define	ODM_CE			0x04	//BIT2
#define	ODM_WIN			0x08	//BIT3

#define	DM_ODM_SUPPORT_TYPE			ODM_CE

typedef enum tag_ODM_Support_Interface_Definition
{
	ODM_ITRF_PCIE	=	0x1,
	ODM_ITRF_USB	=	0x2,
	ODM_ITRF_SDIO	=	0x4,
	ODM_ITRF_ALL	=	0x7,
}ODM_INTERFACE_E;

typedef enum tag_ODM_Support_IC_Type_Definition
{
	ODM_RTL8192S	=	BIT0,
	ODM_RTL8192C	=	BIT1,
	ODM_RTL8192D	=	BIT2,
	ODM_RTL8723A	=	BIT3,
	ODM_RTL8188E	=	BIT4,
	ODM_RTL8812	=	BIT5,
	ODM_RTL8821	=	BIT6,
	ODM_RTL8192E	=	BIT7,
	ODM_RTL8723B	=	BIT8,
	ODM_RTL8813A	=	BIT9,
	ODM_RTL8881A	=	BIT10
}ODM_IC_TYPE_E;

#define ODM_IC_11N_SERIES		(ODM_RTL8192S|ODM_RTL8192C|ODM_RTL8192D|ODM_RTL8723A|ODM_RTL8188E|ODM_RTL8192E|ODM_RTL8723B)
#define ODM_IC_11AC_SERIES		(ODM_RTL8812|ODM_RTL8821|ODM_RTL8813A|ODM_RTL8881A)

u1Byte
ODM_Read1Byte(
	PDM_ODM_T		pDM_Odm,
	u4Byte			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	return	RTL_R8(RegAddr);
#elif(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return	PlatformEFIORead1Byte(Adapter, RegAddr);
#endif

}


u2Byte
ODM_Read2Byte(
	PDM_ODM_T		pDM_Odm,
	u4Byte			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	return	RTL_R16(RegAddr);
#elif(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return	PlatformEFIORead2Byte(Adapter, RegAddr);
#endif

}


u4Byte
ODM_Read4Byte(
	PDM_ODM_T		pDM_Odm,
	u4Byte			RegAddr
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	return	RTL_R32(RegAddr);
#elif(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return	PlatformEFIORead4Byte(Adapter, RegAddr);
#endif

}


VOID
ODM_Write1Byte(
	PDM_ODM_T		pDM_Odm,
	u4Byte			RegAddr,
	u1Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	RTL_W8(RegAddr, Data);
#elif(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformEFIOWrite1Byte(Adapter, RegAddr, Data);
#endif

}


VOID
ODM_Write2Byte(
	PDM_ODM_T		pDM_Odm,
	u4Byte			RegAddr,
	u2Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	RTL_W16(RegAddr, Data);
#elif(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformEFIOWrite2Byte(Adapter, RegAddr, Data);
#endif

}


VOID
ODM_Write4Byte(
	PDM_ODM_T		pDM_Odm,
	u4Byte			RegAddr,
	u4Byte			Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	RTL_W32(RegAddr, Data);
#elif(DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformEFIOWrite4Byte(Adapter, RegAddr, Data);
#endif

}


VOID
ODM_SetMACReg(
	PDM_ODM_T	pDM_Odm,
	u4Byte		RegAddr,
	u4Byte		BitMask,
	u4Byte		Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	PHY_SetBBReg(pDM_Odm->priv, RegAddr, BitMask, Data);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif
}


u4Byte
ODM_GetMACReg(
	PDM_ODM_T	pDM_Odm,
	u4Byte		RegAddr,
	u4Byte		BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return PHY_QueryMacReg(pDM_Odm->priv, RegAddr, BitMask);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryMacReg(Adapter, RegAddr, BitMask);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_CE))
	return PHY_QueryBBReg(pDM_Odm->Adapter, RegAddr, BitMask);
#endif
}


VOID
ODM_SetBBReg(
	PDM_ODM_T	pDM_Odm,
	u4Byte		RegAddr,
	u4Byte		BitMask,
	u4Byte		Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	PHY_SetBBReg(pDM_Odm->priv, RegAddr, BitMask, Data);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif
}


u4Byte
ODM_GetBBReg(
	PDM_ODM_T	pDM_Odm,
	u4Byte		RegAddr,
	u4Byte		BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return PHY_QueryBBReg(pDM_Odm->priv, RegAddr, BitMask);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
#endif
}


VOID
ODM_SetRFReg(
	PDM_ODM_T			pDM_Odm,
	ODM_RF_RADIO_PATH_E	eRFPath,
	u4Byte				RegAddr,
	u4Byte				BitMask,
	u4Byte				Data
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	PHY_SetRFReg(pDM_Odm->priv, eRFPath, RegAddr, BitMask, Data);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data);
#endif
}


u4Byte
ODM_GetRFReg(
	PDM_ODM_T			pDM_Odm,
	ODM_RF_RADIO_PATH_E	eRFPath,
	u4Byte				RegAddr,
	u4Byte				BitMask
	)
{
#if(DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return PHY_QueryRFReg(pDM_Odm->priv, eRFPath, RegAddr, BitMask, 1);
#elif(DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_WIN))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask);
#endif
}


#include "odm_RegDefine11N.h"
#include "odm_RegDefine11AC.h"
#include "odm_debug.h"

VOID
ODM_InitDebugSetting(
	PDM_ODM_T		pDM_Odm
	)
{
pDM_Odm->DebugLevel				=	ODM_DBG_LOUD;

pDM_Odm->DebugComponents			=
\
#if ODM_DBG
//BB Functions
//									ODM_COMP_DIG					|
//									ODM_COMP_RA_MASK				|
//									ODM_COMP_DYNAMIC_TXPWR		|
//									ODM_COMP_FA_CNT				|
//									ODM_COMP_RSSI_MONITOR			|
//									ODM_COMP_CCK_PD				|
//									ODM_COMP_ANT_DIV				|
//									ODM_COMP_PWR_SAVE				|
//									ODM_COMP_PWR_TRAIN			|
//									ODM_COMP_RATE_ADAPTIVE		|
//									ODM_COMP_PATH_DIV				|
//									ODM_COMP_DYNAMIC_PRICCA		|
//									ODM_COMP_RXHP					|
//									ODM_COMP_MP					|
//									ODM_COMP_DYNAMIC_ATC		|

//MAC Functions
//									ODM_COMP_EDCA_TURBO			|
//									ODM_COMP_EARLY_MODE			|
//RF Functions
//									ODM_COMP_TX_PWR_TRACK		|
//									ODM_COMP_RX_GAIN_TRACK		|
//									ODM_COMP_CALIBRATION			|
//Common
//									ODM_COMP_COMMON				|
//									ODM_COMP_INIT					|
//									ODM_COMP_PSD					|
#endif
									0;
}

const char *odm_comp_str[] = {
	/* BIT0 */"ODM_COMP_DIG",
	/* BIT1 */"ODM_COMP_RA_MASK",
	/* BIT2 */"ODM_COMP_DYNAMIC_TXPWR",
	/* BIT3 */"ODM_COMP_FA_CNT",
	/* BIT4 */"ODM_COMP_RSSI_MONITOR",
	/* BIT5 */"ODM_COMP_CCK_PD",
	/* BIT6 */"ODM_COMP_ANT_DIV",
	/* BIT7 */"ODM_COMP_PWR_SAVE",
	/* BIT8 */"ODM_COMP_PWR_TRAIN",
	/* BIT9 */"ODM_COMP_RATE_ADAPTIVE",
	/* BIT10 */"ODM_COMP_PATH_DIV",
	/* BIT11 */"ODM_COMP_PSD",
	/* BIT12 */"ODM_COMP_DYNAMIC_PRICCA",
	/* BIT13 */"ODM_COMP_RXHP",
	/* BIT14 */"ODM_COMP_MP",
	/* BIT15 */"ODM_COMP_DYNAMIC_ATC",
	/* BIT16 */"ODM_COMP_EDCA_TURBO",
	/* BIT17 */"ODM_COMP_EARLY_MODE",
	/* BIT18 */NULL,
	/* BIT19 */NULL,
	/* BIT20 */NULL,
	/* BIT21 */NULL,
	/* BIT22 */NULL,
	/* BIT23 */NULL,
	/* BIT24 */"ODM_COMP_TX_PWR_TRACK",
	/* BIT25 */"ODM_COMP_RX_GAIN_TRACK",
	/* BIT26 */"ODM_COMP_CALIBRATION",
	/* BIT27 */NULL,
	/* BIT28 */NULL,
	/* BIT29 */NULL,
	/* BIT30 */"ODM_COMP_COMMON",
	/* BIT31 */"ODM_COMP_INIT",
};

#define RTW_ODM_COMP_MAX 32

const char *odm_dbg_level_str[] = {
	NULL,
	"ODM_DBG_OFF",
	"ODM_DBG_SERIOUS",
	"ODM_DBG_WARNING",
	"ODM_DBG_LOUD",
	"ODM_DBG_TRACE",
};

#define RTW_ODM_DBG_LEVEL_NUM 6

void rtw_odm_dbg_comp_msg(void *sel, _adapter *adapter)
{
	DM_ODM_T *odm = adapter_to_odm(adapter);
	int cnt = 0;
	u64 dbg_comp;
	int i;

	dbg_comp = odm->DebugComponents;
	DBG_871X_SEL_NL(sel, "odm.DebugComponents = 0x%016llx \n", dbg_comp);
	for (i=0;i<RTW_ODM_COMP_MAX;i++) {
		if (odm_comp_str[i])
		DBG_871X_SEL_NL(sel, "%cBIT%-2d %s\n",
			(BIT0 << i) & dbg_comp ? '+' : ' ', i, odm_comp_str[i]);
	}
}

inline void rtw_odm_dbg_comp_set(_adapter *adapter, u64 comps)
{
	DM_ODM_T *odm = adapter_to_odm(adapter);
	odm->DebugComponents = comps;
}

void rtw_odm_dbg_level_msg(void *sel, _adapter *adapter)
{
	DM_ODM_T *odm = adapter_to_odm(adapter);
	int cnt = 0;
	u32 dbg_level;
	int i;

	dbg_level = odm->DebugLevel;
	DBG_871X_SEL_NL(sel, "odm.DebugLevel = %u\n", dbg_level);
	for (i=0;i<RTW_ODM_DBG_LEVEL_NUM;i++) {
		if (odm_dbg_level_str[i])
			DBG_871X_SEL_NL(sel, "%u %s\n", i, odm_dbg_level_str[i]);
	}
}

inline void rtw_odm_dbg_level_set(_adapter *adapter, u32 level)
{
	DM_ODM_T *odm = adapter_to_odm(adapter);
	odm->DebugLevel= level;
}

const char *dm_ability_str[] = {
	/* BIT0 */"DYNAMIC_FUNC_DIG",
	/* BIT1 */"DYNAMIC_FUNC_HP",
	/* BIT2 */"DYNAMIC_FUNC_SS",
	/* BIT3 */"DYNAMIC_FUNC_BT",
	/* BIT4 */"DYNAMIC_FUNC_ANT_DIV",
#ifdef CONFIG_ODM_ADAPTIVITY
	/* BIT5 */"DYNAMIC_FUNC_ADAPTIVITY",
#endif
};

#ifdef CONFIG_ODM_ADAPTIVITY
#define RTW_DM_ABILITY_MAX 6
#else
#define RTW_DM_ABILITY_MAX 5
#endif

void rtw_dm_check_rxfifo_full(_adapter *adapter)
{
	struct dvobj_priv *psdpriv = adapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	//check RX fifo counter
	rtw_write8(adapter, REG_RXERR_RPT+3, rtw_read8(adapter, REG_RXERR_RPT+3)|0xa0);
	pdbgpriv->dbg_rx_fifo_last_overflow = pdbgpriv->dbg_rx_fifo_curr_overflow;
	pdbgpriv->dbg_rx_fifo_curr_overflow = rtw_read16(adapter, REG_RXERR_RPT);
	pdbgpriv->dbg_rx_fifo_diff_overflow = pdbgpriv->dbg_rx_fifo_curr_overflow-pdbgpriv->dbg_rx_fifo_last_overflow;
}

void rtw_dm_ability_msg(void *sel, _adapter *adapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(adapter);
	int cnt = 0;
	u8 ability = 0;
	int i;

	rtw_hal_get_hwreg(adapter, HW_VAR_DM_FLAG, (u8*)&ability);
	DBG_871X_SEL_NL(sel, "dm.DMFlag = 0x%02x\n", ability);
	for (i=0;i<RTW_DM_ABILITY_MAX;i++) {
		if (dm_ability_str[i])
		DBG_871X_SEL_NL(sel, "%cBIT%-2d %s\n",
			(BIT0 << i) & ability ? '+' : ' ', i, dm_ability_str[i]);
	}
}

inline void rtw_dm_ability_set(_adapter *adapter, u8 ability)
{
	rtw_hal_set_hwreg(adapter, HW_VAR_DM_FLAG, (u8*)&ability);
}

bool rtw_adapter_linked(_adapter *adapter)
{
	bool linked = _FALSE;
	struct mlme_priv	*mlmepriv = &adapter->mlmepriv;

	if(	(check_fwstate(mlmepriv, WIFI_AP_STATE) == _TRUE) ||
		(check_fwstate(mlmepriv, WIFI_ADHOC_STATE|WIFI_ADHOC_MASTER_STATE) == _TRUE))
	{
		if(adapter->stapriv.asoc_sta_count > 2)
			linked = _TRUE;
	}
	else{//Station mode
		if(check_fwstate(mlmepriv, _FW_LINKED)== _TRUE)
			linked = _TRUE;
	}

	return linked;
}

bool dm_linked(_adapter *adapter)
{
	bool linked;

	if ((linked = rtw_adapter_linked(adapter)))
		goto exit;

#ifdef CONFIG_CONCURRENT_MODE
	if ((adapter =  adapter->pbuddy_adapter) == NULL)
		goto exit;
	linked = rtw_adapter_linked(adapter);
#endif

exit:
	return linked;
}

/* v4 branch doesn't have common traffic_stat in dvobj */
u64 dev_tx_uncast_bytes(_adapter *adapter)
{
	u64	tx_bytes = 0;

	tx_bytes += adapter->xmitpriv.tx_bytes;

#ifdef CONFIG_CONCURRENT_MODE
	if ((adapter = adapter->pbuddy_adapter) == NULL)
		goto exit;
	tx_bytes += adapter->xmitpriv.tx_bytes;
#endif

exit:
	return tx_bytes;
}

u64 dev_rx_uncast_bytes(_adapter *adapter)
{
	u64	rx_bytes = 0;

	rx_bytes += adapter->recvpriv.rx_bytes;

#ifdef CONFIG_CONCURRENT_MODE
	if ((adapter = adapter->pbuddy_adapter) == NULL)
		goto exit;
	rx_bytes += adapter->recvpriv.rx_bytes;
#endif

exit:
	return rx_bytes;
}

void rtw_odm_adaptivity_parm_msg(void *sel, _adapter *adapter)
{
#ifdef CONFIG_ODM_ADAPTIVITY
	DM_ODM_T *odm = adapter_to_odm(adapter);

	DBG_871X_SEL_NL(sel, "%10s %16s %8s %10s %11s %14s\n"
		, "TH_L2H_ini", "TH_EDCCA_HL_diff", "IGI_Base", "ForceEDCCA", "AdapEn_RSSI", "IGI_LowerBound");
	DBG_871X_SEL_NL(sel, "0x%-8x %-16d 0x%-6x %-10d %-11u %-14u\n"
		, (u8)odm->TH_L2H_ini
		, odm->TH_EDCCA_HL_diff
		, odm->IGI_Base
		, odm->ForceEDCCA
		, odm->AdapEn_RSSI
		, odm->IGI_LowerBound
	);
#endif /* CONFIG_ODM_ADAPTIVITY */
}

void rtw_odm_adaptivity_parm_set(_adapter *pAdapter, s8 TH_L2H_ini, s8 TH_EDCCA_HL_diff,
	s8 IGI_Base, bool ForceEDCCA, u8 AdapEn_RSSI, u8 IGI_LowerBound)
{
#ifdef CONFIG_ODM_ADAPTIVITY
	DM_ODM_T *odm = adapter_to_odm(pAdapter);

	odm->TH_L2H_ini = TH_L2H_ini;
	odm->TH_EDCCA_HL_diff = TH_EDCCA_HL_diff;
	odm->IGI_Base = IGI_Base;
	odm->ForceEDCCA = ForceEDCCA;
	odm->AdapEn_RSSI = AdapEn_RSSI;
	odm->IGI_LowerBound = IGI_LowerBound;

#endif /* CONFIG_ODM_ADAPTIVITY */
}

#ifdef CONFIG_ODM_ADAPTIVITY

VOID
ODM_Write_DIG(
	PDM_ODM_T		pDM_Odm,
	u1Byte			CurrentIGI
	)
{
	_adapter *adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(adapter);
	struct dm_priv *dmpriv = &pHalData->dmpriv;
	DIG_T *pDigTable = &dmpriv->DM_DigTable;

	if(pDigTable->CurIGValue != CurrentIGI) {
		pDigTable->CurIGValue = CurrentIGI;
		rtw_warn_on(!pDM_Odm->write_dig);
		if (pDM_Odm->write_dig)
			pDM_Odm->write_dig(adapter);
	}
}

VOID
Phydm_SearchPwdBLowerBound(
	PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	u4Byte			value32 = 0;
	u1Byte			cnt, IGI = 0x50;		/*IGI = 0x50 for cal EDCCA lower bound*/
	u1Byte			txEdcca1 = 0, txEdcca0 = 0;
	BOOLEAN			bAdjust = _TRUE;
	s1Byte			TH_L2H_dmc, TH_H2L_dmc, IGI_target = 0x32;
	s1Byte			Diff;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pDM_Odm->Adapter);

	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT3 | BIT2 | BIT1, 0x1);			/*set TXmod to standby mode to remove outside noise affect*/
	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT22 | BIT21 | BIT20, 0x1);		/*set RXmod to standby mode to remove outside noise affect*/
	if (pHalData->rf_type !=  RF_1T1R ) {
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT3 | BIT2 | BIT1, 0x1);		/*set TXmod to standby mode to remove outside noise affect*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT22 | BIT21 | BIT20, 0x1);	/*set RXmod to standby mode to remove outside noise affect*/
	}
	ODM_Write_DIG(pDM_Odm, 0x7e);

	Diff = IGI_target - (s1Byte)IGI;
	TH_L2H_dmc = pDM_Odm->TH_L2H_ini + Diff;
	if (TH_L2H_dmc > 10)
		TH_L2H_dmc = 10;
	TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;

	ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte0, (u1Byte)TH_L2H_dmc);
	ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte2, (u1Byte)TH_H2L_dmc);

	rtw_mdelay_os(5);

	while (bAdjust) {
		for (cnt = 0; cnt < 20; cnt++) {
			if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
				value32 = ODM_GetBBReg(pDM_Odm, ODM_REG_RPT_11N, bMaskDWord);

			if (value32 & BIT30 && (pDM_Odm->SupportICType & (ODM_RTL8723A | ODM_RTL8723B | ODM_RTL8188E)))
				txEdcca1 = txEdcca1 + 1;
			else if (value32 & BIT29)
				txEdcca1 = txEdcca1 + 1;
			else
				txEdcca0 = txEdcca0 + 1;
		}

		if (txEdcca1 > 1) {
			IGI = IGI - 1;
			TH_L2H_dmc = TH_L2H_dmc + 1;
			if (TH_L2H_dmc > 10)
				TH_L2H_dmc = 10;
			TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;

			ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte0, (u1Byte)TH_L2H_dmc);
			ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte2, (u1Byte)TH_H2L_dmc);
			if (TH_L2H_dmc == 10) {
				bAdjust = _FALSE;
				pDM_Odm->H2L_lb = TH_H2L_dmc;
				pDM_Odm->L2H_lb = TH_L2H_dmc;
				pDM_Odm->Adaptivity_IGI_upper = IGI;
			}

			txEdcca1 = 0;
			txEdcca0 = 0;

		} else {
			bAdjust = _FALSE;
			pDM_Odm->H2L_lb = TH_H2L_dmc;
			pDM_Odm->L2H_lb = TH_L2H_dmc;
			pDM_Odm->Adaptivity_IGI_upper = IGI;
			txEdcca1 = 0;
			txEdcca0 = 0;
		}
	}

	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT3 | BIT2 | BIT1, 0x2);			/*set TXmod to standby mode to remove outside noise affect*/
	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT22 | BIT21 | BIT20, 0x3);		/*set RXmod to standby mode to remove outside noise affect*/
	if (pHalData->rf_type !=  RF_1T1R ) {
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT3 | BIT2 | BIT1, 0x2);		/*set TXmod to standby mode to remove outside noise affect*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT22 | BIT21 | BIT20, 0x3);	/*set RXmod to standby mode to remove outside noise affect*/
	}
	ODM_Write_DIG(pDM_Odm, 0x20);

	ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte0, (u1Byte)0x7f);
	ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte2, (u1Byte)0x7f);
}
#endif /* CONFIG_ODM_ADAPTIVITY */

VOID
odm_AdaptivityInit(
PDM_ODM_T pDM_Odm
)
{
#ifdef CONFIG_ODM_ADAPTIVITY

	pDM_Odm->TH_L2H_ini = 0xef; // -17

	pDM_Odm->TH_EDCCA_HL_diff = 7;
	pDM_Odm->IGI_Base = 0x32;
	pDM_Odm->IGI_target = 0x1c;
	pDM_Odm->ForceEDCCA = 0;
	pDM_Odm->AdapEn_RSSI = 20;

	pDM_Odm->NHM_disable = _FALSE;
	pDM_Odm->TxHangFlg = _TRUE;
	pDM_Odm->txEdcca0 = 0;
	pDM_Odm->txEdcca1 = 0;
	pDM_Odm->H2L_lb= 0;
	pDM_Odm->L2H_lb= 0;
	pDM_Odm->Adaptivity_IGI_upper = 0;
	pDM_Odm->adaptivity_flag= _TRUE;
	//Reg524[11]=0 is easily to transmit packets during adaptivity test

	ODM_SetMACReg(pDM_Odm, REG_TX_PTCL_CTRL, BIT15, 0);	/*don't ignore EDCCA	 reg520[15]=0*/
	ODM_SetMACReg(pDM_Odm, REG_RD_CTRL, BIT11, 1);			/*reg524[11]=1	*/

	if (pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		ODM_SetBBReg(pDM_Odm, ODM_REG_DBG_RPT_11N, bMaskDWord, 0x208);

	Phydm_SearchPwdBLowerBound(pDM_Odm);


#endif /* CONFIG_ODM_ADAPTIVITY */
}

VOID
odm_Adaptivity(
	PDM_ODM_T		pDM_Odm
)
{
#ifdef CONFIG_ODM_ADAPTIVITY
	s1Byte TH_L2H_dmc, TH_H2L_dmc;
	s1Byte Diff, IGI_target;
	u32 value32;
	BOOLEAN EDCCA_State = _TRUE;

	_adapter *pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	struct dm_priv *dmpriv = &pHalData->dmpriv;
	DIG_T *pDigTable = &dmpriv->DM_DigTable;
	u8 IGI = pDigTable->CurIGValue;
	u8 RSSI_Min = pDigTable->Rssi_val_min;
	HT_CHANNEL_WIDTH BandWidth = rtw_get_oper_bw(pAdapter);

	if (!(dmpriv->DMFlag & DYNAMIC_FUNC_ADAPTIVITY))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("Go to odm_DynamicEDCCA() \n"));
		// Add by Neil Chen to enable edcca to MP Platform
		return;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_Adaptivity() =====> \n"));

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("ForceEDCCA=%d, IGI_Base=0x%x, TH_L2H_ini = %d, TH_EDCCA_HL_diff = %d, AdapEn_RSSI = %d\n",
		pDM_Odm->ForceEDCCA, pDM_Odm->IGI_Base, pDM_Odm->TH_L2H_ini, pDM_Odm->TH_EDCCA_HL_diff, pDM_Odm->AdapEn_RSSI));

	IGI_target = pDM_Odm->IGI_Base;

	pDM_Odm->IGI_target = (u1Byte) IGI_target;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("BandWidth=%s, IGI_target=0x%x, EDCCA_State=%d\n",
		(BandWidth==HT_CHANNEL_WIDTH_40)?"40M":"20M", IGI_target, EDCCA_State));

	if(EDCCA_State == _TRUE)
	{
		Diff = IGI_target -(s1Byte)IGI;
		TH_L2H_dmc = pDM_Odm->TH_L2H_ini + Diff;
		if(TH_L2H_dmc > 10)	TH_L2H_dmc = 10;
		TH_H2L_dmc = TH_L2H_dmc - pDM_Odm->TH_EDCCA_HL_diff;

		//replace lower bound to prevent EDCCA always equal 1
			if(TH_H2L_dmc < pDM_Odm->H2L_lb)
				TH_H2L_dmc = pDM_Odm->H2L_lb;
			if(TH_L2H_dmc < pDM_Odm->L2H_lb)
				TH_L2H_dmc = pDM_Odm->L2H_lb;
	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("IGI=0x%x, TH_L2H_dmc = %d, TH_H2L_dmc = %d\n",
		IGI, TH_L2H_dmc, TH_H2L_dmc));

		ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte0, (u1Byte)TH_L2H_dmc);
		ODM_SetBBReg(pDM_Odm,rOFDM0_ECCAThreshold, bMaskByte2, (u1Byte)TH_H2L_dmc);


#endif /* CONFIG_ODM_ADAPTIVITY */
}

void rtw_odm_init(_adapter *adapter)
{
	PDM_ODM_T odm = adapter_to_odm(adapter);

	odm->Adapter = adapter;
	rtw_warn_on(!odm->Adapter);

	switch (adapter->chip_type) {
	case RTL8188C_8192C:
		odm->SupportICType = ODM_RTL8192C;
		break;
	case RTL8192D:
		odm->SupportICType = ODM_RTL8192D;
		break;
	default:
		odm->SupportICType = 0;
	};
	rtw_warn_on(!odm->SupportICType);

	switch (adapter->interface_type) {
	case RTW_USB:
		odm->SupportInterface = ODM_ITRF_USB;
		break;
	case RTW_PCIE:
		odm->SupportInterface = ODM_ITRF_PCIE;
		break;
	default:
		odm->SupportInterface = 0;
	};
	rtw_warn_on(!odm->SupportInterface);

	ODM_InitDebugSetting(odm);
}
