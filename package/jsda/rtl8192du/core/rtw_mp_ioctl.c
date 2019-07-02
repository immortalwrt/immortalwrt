/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
#define _RTW_MP_IOCTL_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <mlme_osdep.h>

//#include <rtw_mp.h>
#include <rtw_mp_ioctl.h>


//****************  oid_rtl_seg_81_85   section start ****************
NDIS_STATUS oid_rt_wireless_mode_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->information_buf_len < sizeof(u8))
		return NDIS_STATUS_INVALID_LENGTH;

	if (poid_par_priv->type_of_oid == SET_OID) {
		Adapter->registrypriv.wireless_mode = *(u8*)poid_par_priv->information_buf;
	} else if (poid_par_priv->type_of_oid == QUERY_OID) {
		*(u8*)poid_par_priv->information_buf = Adapter->registrypriv.wireless_mode;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
		 RT_TRACE(_module_mp_, _drv_info_, ("-query Wireless Mode=%d\n", Adapter->registrypriv.wireless_mode));
	} else {
		status = NDIS_STATUS_NOT_ACCEPTED;
	}

_func_exit_;

	return status;
}
//****************  oid_rtl_seg_81_87_80   section start ****************
NDIS_STATUS oid_rt_pro_write_bb_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	struct bb_reg_param *pbbreg;
	u16 offset;
	u32 value;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_write_bb_reg_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(struct bb_reg_param))
		return NDIS_STATUS_INVALID_LENGTH;

	pbbreg = (struct bb_reg_param *)(poid_par_priv->information_buf);

	offset = (u16)(pbbreg->offset) & 0xFFF; //0ffset :0x800~0xfff
	if (offset < BB_REG_BASE_ADDR) offset |= BB_REG_BASE_ADDR;

	value = pbbreg->value;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("oid_rt_pro_write_bb_reg_hdl: offset=0x%03X value=0x%08X\n",
		  offset, value));

	_irqlevel_changed_(&oldirql, LOWER);
	write_bbreg(Adapter, offset, 0xFFFFFFFF, value);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_read_bb_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	struct bb_reg_param *pbbreg;
	u16 offset;
	u32 value;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_read_bb_reg_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(struct bb_reg_param))
		return NDIS_STATUS_INVALID_LENGTH;

	pbbreg = (struct bb_reg_param *)(poid_par_priv->information_buf);

	offset = (u16)(pbbreg->offset) & 0xFFF; //0ffset :0x800~0xfff
	if (offset < BB_REG_BASE_ADDR) offset |= BB_REG_BASE_ADDR;

	_irqlevel_changed_(&oldirql, LOWER);
	value = read_bbreg(Adapter, offset, 0xFFFFFFFF);
	_irqlevel_changed_(&oldirql, RAISE);

	pbbreg->value = value;
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("-oid_rt_pro_read_bb_reg_hdl: offset=0x%03X value:0x%08X\n",
		  offset, value));
_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_write_rf_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	struct rf_reg_param *pbbreg;
	u8 path;
	u8 offset;
	u32 value;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_write_rf_reg_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(struct rf_reg_param))
		return NDIS_STATUS_INVALID_LENGTH;

	pbbreg = (struct rf_reg_param *)(poid_par_priv->information_buf);

	if (pbbreg->path >= MAX_RF_PATH_NUMS)
		return NDIS_STATUS_NOT_ACCEPTED;
	if (pbbreg->offset > 0xFF)
		return NDIS_STATUS_NOT_ACCEPTED;
	if (pbbreg->value > 0xFFFFF)
		return NDIS_STATUS_NOT_ACCEPTED;

	path = (u8)pbbreg->path;
	offset = (u8)pbbreg->offset;
	value = pbbreg->value;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("oid_rt_pro_write_rf_reg_hdl: path=%d offset=0x%02X value=0x%05X\n",
		  path, offset, value));

	_irqlevel_changed_(&oldirql, LOWER);
	write_rfreg(Adapter, path, offset, value);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_read_rf_reg_hdl(struct oid_par_priv *poid_par_priv)
{
	struct rf_reg_param *pbbreg;
	u8 path;
	u8 offset;
	u32 value;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_read_rf_reg_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(struct rf_reg_param))
		return NDIS_STATUS_INVALID_LENGTH;

	pbbreg = (struct rf_reg_param *)(poid_par_priv->information_buf);

	if (pbbreg->path >= MAX_RF_PATH_NUMS)
		return NDIS_STATUS_NOT_ACCEPTED;
	if (pbbreg->offset > 0xFF)
		return NDIS_STATUS_NOT_ACCEPTED;

	path = (u8)pbbreg->path;
	offset = (u8)pbbreg->offset;

	_irqlevel_changed_(&oldirql, LOWER);
	value = read_rfreg(Adapter, path, offset);
	_irqlevel_changed_(&oldirql, RAISE);

	pbbreg->value = value;

	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("-oid_rt_pro_read_rf_reg_hdl: path=%d offset=0x%02X value=0x%05X\n",
		  path, offset, value));

_func_exit_;

	return status;
}
//****************  oid_rtl_seg_81_87_00   section end****************
//------------------------------------------------------------------------------

//****************  oid_rtl_seg_81_80_00   section start ****************
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_data_rate_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		ratevalue;//4
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("+oid_rt_pro_set_data_rate_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len != sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	ratevalue = *((u32*)poid_par_priv->information_buf);//4
	RT_TRACE(_module_mp_, _drv_notice_,
		 ("oid_rt_pro_set_data_rate_hdl: data rate idx=%d\n", ratevalue));
	if (ratevalue >= MPT_RATE_LAST)
		return NDIS_STATUS_INVALID_DATA;

	Adapter->mppriv.rateidx = ratevalue;

	_irqlevel_changed_(&oldirql, LOWER);
	SetDataRate(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_start_test_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		mode;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_start_test_hdl\n"));

	if (Adapter->registrypriv.mp_mode == 0)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);

	//IQCalibrateBcut(Adapter);

	mode = *((u32*)poid_par_priv->information_buf);
	Adapter->mppriv.mode = mode;// 1 for loopback

	if (mp_start_test(Adapter) == _FAIL) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		goto exit;
	}

exit:
	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_mp_, _drv_notice_, ("-oid_rt_pro_start_test_hdl: mp_mode=%d\n", Adapter->mppriv.mode));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_stop_test_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+Set OID_RT_PRO_STOP_TEST\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);
	mp_stop_test(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_mp_, _drv_notice_, ("-Set OID_RT_PRO_STOP_TEST\n"));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_channel_direct_call_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		Channel;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_set_channel_direct_call_hdl\n"));

	if (poid_par_priv->information_buf_len != sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	if (poid_par_priv->type_of_oid == QUERY_OID) {
		*((u32*)poid_par_priv->information_buf) = Adapter->mppriv.channel;
		return NDIS_STATUS_SUCCESS;
	}

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	Channel = *((u32*)poid_par_priv->information_buf);
	RT_TRACE(_module_mp_, _drv_notice_, ("oid_rt_pro_set_channel_direct_call_hdl: Channel=%d\n", Channel));
	if (Channel > 14)
		return NDIS_STATUS_NOT_ACCEPTED;
	Adapter->mppriv.channel = Channel;

	_irqlevel_changed_(&oldirql, LOWER);
	SetChannel(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_set_bandwidth_hdl(struct oid_par_priv *poid_par_priv)
{
	u16		bandwidth;
	u16		channel_offset;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	padapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_,
		 ("+oid_rt_set_bandwidth_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	bandwidth = *((u32*)poid_par_priv->information_buf);//4
	channel_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;

	if (bandwidth != HT_CHANNEL_WIDTH_40)
		bandwidth = HT_CHANNEL_WIDTH_20;
	padapter->mppriv.bandwidth = (u8)bandwidth;
	padapter->mppriv.prime_channel_offset = (u8)channel_offset;

	_irqlevel_changed_(&oldirql, LOWER);
	SetBandwidth(padapter);
	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("-oid_rt_set_bandwidth_hdl: bandwidth=%d channel_offset=%d\n",
		  bandwidth, channel_offset));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_antenna_bb_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		antenna;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_set_antenna_bb_hdl\n"));

	if (poid_par_priv->information_buf_len != sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	if (poid_par_priv->type_of_oid == SET_OID)
	{
		antenna = *(u32*)poid_par_priv->information_buf;

		Adapter->mppriv.antenna_tx = (u16)((antenna & 0xFFFF0000) >> 16);
		Adapter->mppriv.antenna_rx = (u16)(antenna & 0x0000FFFF);
		RT_TRACE(_module_mp_, _drv_notice_,
			 ("oid_rt_pro_set_antenna_bb_hdl: tx_ant=0x%04x rx_ant=0x%04x\n",
			  Adapter->mppriv.antenna_tx, Adapter->mppriv.antenna_rx));

		_irqlevel_changed_(&oldirql, LOWER);
		SetAntenna(Adapter);
		_irqlevel_changed_(&oldirql, RAISE);
	} else {
		antenna = (Adapter->mppriv.antenna_tx << 16)|Adapter->mppriv.antenna_rx;
		*(u32*)poid_par_priv->information_buf = antenna;
	}

_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_pro_set_tx_power_control_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		tx_pwr_idx;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_, ("+oid_rt_pro_set_tx_power_control_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len != sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	tx_pwr_idx = *((u32*)poid_par_priv->information_buf);
	if (tx_pwr_idx > MAX_TX_PWR_INDEX_N_MODE)
		return NDIS_STATUS_NOT_ACCEPTED;

	Adapter->mppriv.txpoweridx = (u8)tx_pwr_idx;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("oid_rt_pro_set_tx_power_control_hdl: idx=0x%2x\n",
		  Adapter->mppriv.txpoweridx));

	_irqlevel_changed_(&oldirql, LOWER);
	SetTxPower(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}

//------------------------------------------------------------------------------
//****************  oid_rtl_seg_81_80_20   section start ****************
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_query_tx_packet_sent_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid !=QUERY_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}

	if (poid_par_priv->information_buf_len == sizeof(ULONG)) {
		*(ULONG*)poid_par_priv->information_buf =  Adapter->mppriv.tx_pktcount;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	} else {
		status = NDIS_STATUS_INVALID_LENGTH;
	}

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_query_rx_packet_received_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != QUERY_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}
	RT_TRACE(_module_mp_, _drv_alert_, ("===> oid_rt_pro_query_rx_packet_received_hdl.\n"));
	if (poid_par_priv->information_buf_len == sizeof(ULONG)) {
		*(ULONG*)poid_par_priv->information_buf =  Adapter->mppriv.rx_pktcount;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
		RT_TRACE(_module_mp_, _drv_alert_, ("recv_ok:%d \n",Adapter->mppriv.rx_pktcount));
	} else {
		status = NDIS_STATUS_INVALID_LENGTH;
	}

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_query_rx_packet_crc32_error_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != QUERY_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}
	RT_TRACE(_module_mp_, _drv_alert_, ("===> oid_rt_pro_query_rx_packet_crc32_error_hdl.\n"));
	if (poid_par_priv->information_buf_len == sizeof(ULONG)) {
		*(ULONG*)poid_par_priv->information_buf =  Adapter->mppriv.rx_crcerrpktcount;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
		RT_TRACE(_module_mp_, _drv_alert_, ("recv_err:%d \n",Adapter->mppriv.rx_crcerrpktcount));
	} else {
		status = NDIS_STATUS_INVALID_LENGTH;
	}

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------

NDIS_STATUS oid_rt_pro_reset_tx_packet_sent_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}

	RT_TRACE(_module_mp_, _drv_alert_, ("===> oid_rt_pro_reset_tx_packet_sent_hdl.\n"));
	Adapter->mppriv.tx_pktcount = 0;

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_reset_rx_packet_received_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID)
	{
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}

	if (poid_par_priv->information_buf_len == sizeof(ULONG)) {
		Adapter->mppriv.rx_pktcount = 0;
		Adapter->mppriv.rx_crcerrpktcount = 0;
	} else {
		status = NDIS_STATUS_INVALID_LENGTH;
	}

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_reset_phy_rx_packet_count_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}

	_irqlevel_changed_(&oldirql, LOWER);
	ResetPhyRxPktCount(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_phy_rx_packet_received_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_, ("+oid_rt_get_phy_rx_packet_received_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len != sizeof(ULONG))
		return NDIS_STATUS_INVALID_LENGTH;

	_irqlevel_changed_(&oldirql, LOWER);
	*(ULONG*)poid_par_priv->information_buf = GetPhyRxPktReceived(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;

	RT_TRACE(_module_mp_, _drv_notice_, ("-oid_rt_get_phy_rx_packet_received_hdl: recv_ok=%d\n", *(ULONG*)poid_par_priv->information_buf));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_phy_rx_packet_crc32_error_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_, ("+oid_rt_get_phy_rx_packet_crc32_error_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;


	if (poid_par_priv->information_buf_len != sizeof(ULONG))
		return NDIS_STATUS_INVALID_LENGTH;

	_irqlevel_changed_(&oldirql, LOWER);
	*(ULONG*)poid_par_priv->information_buf = GetPhyRxPktCRC32Error(Adapter);
	_irqlevel_changed_(&oldirql, RAISE);

	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;

	RT_TRACE(_module_mp_, _drv_info_, ("-oid_rt_get_phy_rx_packet_crc32_error_hdl: recv_err=%d\n", *(ULONG*)poid_par_priv->information_buf));

_func_exit_;

	return status;
}
//****************  oid_rtl_seg_81_80_20   section end ****************
NDIS_STATUS oid_rt_pro_set_continuous_tx_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		bStartTest;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_set_continuous_tx_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	bStartTest = *((u32*)poid_par_priv->information_buf);

	_irqlevel_changed_(&oldirql, LOWER);
	SetContinuousTx(Adapter,(u8)bStartTest);
	if (bStartTest) {
		struct mp_priv *pmp_priv = &Adapter->mppriv;
		if (pmp_priv->tx.stop == 0) {
			pmp_priv->tx.stop = 1;
			DBG_871X("%s: pkt tx is running...\n", __func__);
			rtw_msleep_os(5);
		}
		pmp_priv->tx.stop = 0;
		pmp_priv->tx.count = 1;
		SetPacketTx(Adapter);
	}
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_pro_set_single_carrier_tx_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		bStartTest;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_alert_, ("+oid_rt_pro_set_single_carrier_tx_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	bStartTest = *((u32*)poid_par_priv->information_buf);

	_irqlevel_changed_(&oldirql, LOWER);
	SetSingleCarrierTx(Adapter, (u8)bStartTest);
	if (bStartTest) {
		struct mp_priv *pmp_priv = &Adapter->mppriv;
		if (pmp_priv->tx.stop == 0) {
			pmp_priv->tx.stop = 1;
			DBG_871X("%s: pkt tx is running...\n", __func__);
			rtw_msleep_os(5);
		}
		pmp_priv->tx.stop = 0;
		pmp_priv->tx.count = 1;
		SetPacketTx(Adapter);
	}
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_pro_set_carrier_suppression_tx_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		bStartTest;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_set_carrier_suppression_tx_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	bStartTest = *((u32*)poid_par_priv->information_buf);

	_irqlevel_changed_(&oldirql, LOWER);
	SetCarrierSuppressionTx(Adapter, (u8)bStartTest);
	if (bStartTest) {
		struct mp_priv *pmp_priv = &Adapter->mppriv;
		if (pmp_priv->tx.stop == 0) {
			pmp_priv->tx.stop = 1;
			DBG_871X("%s: pkt tx is running...\n", __func__);
			rtw_msleep_os(5);
		}
		pmp_priv->tx.stop = 0;
		pmp_priv->tx.count = 1;
		SetPacketTx(Adapter);
	}
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_pro_set_single_tone_tx_hdl(struct oid_par_priv *poid_par_priv)
{
	u32		bStartTest;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_alert_, ("+oid_rt_pro_set_single_tone_tx_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	bStartTest = *((u32*)poid_par_priv->information_buf);

	_irqlevel_changed_(&oldirql, LOWER);
	SetSingleToneTx(Adapter,(u8)bStartTest);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_pro_set_modulation_hdl(struct oid_par_priv* poid_par_priv)
{
	return 0;
}

NDIS_STATUS oid_rt_pro_trigger_gpio_hdl(struct oid_par_priv *poid_par_priv)
{
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);
	rtw_hal_set_hwreg(Adapter, HW_VAR_TRIGGER_GPIO_0, 0);
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//****************  oid_rtl_seg_81_80_00   section end ****************
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro8711_join_bss_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_read_register_hdl(struct oid_par_priv *poid_par_priv)
{
	pRW_Reg		RegRWStruct;
	u32		offset, width;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_,
		 ("+oid_rt_pro_read_register_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	RegRWStruct = (pRW_Reg)poid_par_priv->information_buf;
	offset = RegRWStruct->offset;
	width = RegRWStruct->width;

	if (offset > 0xFFF)
		return NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);

	switch (width) {
		case 1:
			RegRWStruct->value = rtw_read8(Adapter, offset);
			break;
		case 2:
			RegRWStruct->value = rtw_read16(Adapter, offset);
			break;
		default:
			width = 4;
			RegRWStruct->value = rtw_read32(Adapter, offset);
			break;
	}
	RT_TRACE(_module_mp_, _drv_notice_,
		 ("oid_rt_pro_read_register_hdl: offset:0x%04X value:0x%X\n",
		  offset, RegRWStruct->value));

	_irqlevel_changed_(&oldirql, RAISE);

	*poid_par_priv->bytes_rw = width;

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_write_register_hdl(struct oid_par_priv *poid_par_priv)
{
	pRW_Reg		RegRWStruct;
	u32		offset, width, value;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	padapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_,
		 ("+oid_rt_pro_write_register_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	RegRWStruct = (pRW_Reg)poid_par_priv->information_buf;
	offset = RegRWStruct->offset;
	width = RegRWStruct->width;
	value = RegRWStruct->value;

	if (offset > 0xFFF)
		return NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);

	switch (RegRWStruct->width)
	{
		case 1:
			if (value > 0xFF) {
				status = NDIS_STATUS_NOT_ACCEPTED;
				break;
			}
			rtw_write8(padapter, offset, (u8)value);
			break;
		case 2:
			if (value > 0xFFFF) {
				status = NDIS_STATUS_NOT_ACCEPTED;
				break;
			}
			rtw_write16(padapter, offset, (u16)value);
			break;
		case 4:
			rtw_write32(padapter, offset, value);
			break;
		default:
			status = NDIS_STATUS_NOT_ACCEPTED;
			break;
	}

	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_mp_, _drv_info_,
		 ("-oid_rt_pro_write_register_hdl: offset=0x%08X width=%d value=0x%X\n",
		  offset, width, value));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_burst_read_register_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_burst_write_register_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_write_txcmd_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}

//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_read16_eeprom_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}

//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_write16_eeprom_hdl (struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro8711_wi_poll_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro8711_pkt_loss_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_rd_attrib_mem_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_wr_attrib_mem_hdl (struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS  oid_rt_pro_set_rf_intfs_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_poll_rx_status_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_cfg_debug_message_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_data_rate_ex_hdl(struct oid_par_priv *poid_par_priv)
{
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+OID_RT_PRO_SET_DATA_RATE_EX\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, LOWER);

	if (rtw_setdatarate_cmd(Adapter, poid_par_priv->information_buf) !=_SUCCESS)
		status = NDIS_STATUS_NOT_ACCEPTED;

	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//-----------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_thermal_meter_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	u8 thermal = 0;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_get_thermal_meter_hdl\n"));

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	_irqlevel_changed_(&oldirql, LOWER);
	GetThermalMeter(Adapter, &thermal);
	_irqlevel_changed_(&oldirql, RAISE);

	*(u32*)poid_par_priv->information_buf = (u32)thermal;
	*poid_par_priv->bytes_rw = sizeof(u32);

_func_exit_;

	return status;
}
//-----------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_read_tssi_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_power_tracking_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);


_func_enter_;

	if (poid_par_priv->information_buf_len < sizeof(u8))
		return NDIS_STATUS_INVALID_LENGTH;

	_irqlevel_changed_(&oldirql, LOWER);
	if (poid_par_priv->type_of_oid == SET_OID) {
		u8 enable;

		enable = *(u8*)poid_par_priv->information_buf;
		RT_TRACE(_module_mp_, _drv_notice_,
			 ("+oid_rt_pro_set_power_tracking_hdl: enable=%d\n", enable));

		SetPowerTracking(Adapter, enable);
	} else {
		GetPowerTracking(Adapter, (u8*)poid_par_priv->information_buf);
	}
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//-----------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_basic_rate_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_qry_pwrstate_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_set_pwrstate_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_h2c_set_rate_table_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_h2c_get_rate_table_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}

//****************  oid_rtl_seg_87_12_00   section start ****************
NDIS_STATUS oid_rt_pro_encryption_ctrl_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_add_sta_info_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_dele_sta_info_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------

NDIS_STATUS oid_rt_pro_query_dr_variable_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_rx_packet_type_hdl(struct oid_par_priv *poid_par_priv)
{

	return NDIS_STATUS_SUCCESS;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_read_efuse_hdl(struct oid_par_priv *poid_par_priv)
{
	PEFUSE_ACCESS_STRUCT pefuse;
	u8 *data;
	u16 addr = 0, cnts = 0, max_available_size = 0;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(EFUSE_ACCESS_STRUCT))
		return NDIS_STATUS_INVALID_LENGTH;

	pefuse = (PEFUSE_ACCESS_STRUCT)poid_par_priv->information_buf;
	addr = pefuse->start_addr;
	cnts = pefuse->cnts;
	data = pefuse->data;

	RT_TRACE(_module_mp_, _drv_notice_,
		("+oid_rt_pro_read_efuse_hd: buf_len=%ld addr=%d cnts=%d\n",
		 poid_par_priv->information_buf_len, addr, cnts));

	EFUSE_GetEfuseDefinition(Adapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);

	if ((addr + cnts) > max_available_size) {
		RT_TRACE(_module_mp_, _drv_err_, ("!oid_rt_pro_read_efuse_hdl: parameter error!\n"));
		return NDIS_STATUS_NOT_ACCEPTED;
	}

	_irqlevel_changed_(&oldirql, LOWER);
	if (rtw_efuse_access(Adapter, _FALSE, addr, cnts, data) == _FAIL) {
		RT_TRACE(_module_mp_, _drv_err_, ("!oid_rt_pro_read_efuse_hdl: rtw_efuse_access FAIL!\n"));
		status = NDIS_STATUS_FAILURE;
	} else
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_write_efuse_hdl(struct oid_par_priv *poid_par_priv)
{
	PEFUSE_ACCESS_STRUCT pefuse;
	u8 *data;
	u16 addr = 0, cnts = 0, max_available_size = 0;
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	PADAPTER Adapter = (PADAPTER)(poid_par_priv->adapter_context);


_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	pefuse = (PEFUSE_ACCESS_STRUCT)poid_par_priv->information_buf;
	addr = pefuse->start_addr;
	cnts = pefuse->cnts;
	data = pefuse->data;

	RT_TRACE(_module_mp_, _drv_notice_,
		 ("+oid_rt_pro_write_efuse_hdl: buf_len=%ld addr=0x%04x cnts=%d\n",
		  poid_par_priv->information_buf_len, addr, cnts));

	EFUSE_GetEfuseDefinition(Adapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (PVOID)&max_available_size, _FALSE);

	if ((addr + cnts) > max_available_size) {
		RT_TRACE(_module_mp_, _drv_err_, ("!oid_rt_pro_write_efuse_hdl: parameter error"));
		return NDIS_STATUS_NOT_ACCEPTED;
	}

	_irqlevel_changed_(&oldirql, LOWER);
	if (rtw_efuse_access(Adapter, _TRUE, addr, cnts, data) == _FAIL)
		status = NDIS_STATUS_FAILURE;
	_irqlevel_changed_(&oldirql, RAISE);

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_rw_efuse_pgpkt_hdl(struct oid_par_priv *poid_par_priv)
{
	PPGPKT_STRUCT	ppgpkt;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

//	RT_TRACE(_module_mp_, _drv_info_, ("+oid_rt_pro_rw_efuse_pgpkt_hdl\n"));

	*poid_par_priv->bytes_rw = 0;

	if (poid_par_priv->information_buf_len < sizeof(PGPKT_STRUCT))
		return NDIS_STATUS_INVALID_LENGTH;

	ppgpkt = (PPGPKT_STRUCT)poid_par_priv->information_buf;

	_irqlevel_changed_(&oldirql, LOWER);

	if (poid_par_priv->type_of_oid == QUERY_OID)
	{
		RT_TRACE(_module_mp_, _drv_notice_,
			("oid_rt_pro_rw_efuse_pgpkt_hdl: Read offset=0x%x\n",\
			ppgpkt->offset));

		Efuse_PowerSwitch(Adapter, _FALSE, _TRUE);
		if (Efuse_PgPacketRead(Adapter, ppgpkt->offset, ppgpkt->data, _FALSE) == _TRUE)
			*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
		else
			status = NDIS_STATUS_FAILURE;
		Efuse_PowerSwitch(Adapter, _FALSE, _FALSE);
	} else {
		RT_TRACE(_module_mp_, _drv_notice_,
			("oid_rt_pro_rw_efuse_pgpkt_hdl: Write offset=0x%x word_en=0x%x\n",\
			ppgpkt->offset, ppgpkt->word_en));

		Efuse_PowerSwitch(Adapter, _TRUE, _TRUE);
		if (Efuse_PgPacketWrite(Adapter, ppgpkt->offset, ppgpkt->word_en, ppgpkt->data, _FALSE) == _TRUE)
			*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
		else
			status = NDIS_STATUS_FAILURE;
		Efuse_PowerSwitch(Adapter, _TRUE, _FALSE);
	}

	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_mp_, _drv_info_,
		 ("-oid_rt_pro_rw_efuse_pgpkt_hdl: status=0x%08X\n", status));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_efuse_current_size_hdl(struct oid_par_priv *poid_par_priv)
{
	u16 size;
	u8 ret;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len <sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	_irqlevel_changed_(&oldirql, LOWER);
	ret = efuse_GetCurrentSize(Adapter, &size);
	_irqlevel_changed_(&oldirql, RAISE);
	if (ret == _SUCCESS) {
		*(u32*)poid_par_priv->information_buf = size;
		*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;
	} else
		status = NDIS_STATUS_FAILURE;

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_efuse_max_size_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);

_func_enter_;

	if (poid_par_priv->type_of_oid != QUERY_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(u32))
		return NDIS_STATUS_INVALID_LENGTH;

	*(u32*)poid_par_priv->information_buf = efuse_GetMaxSize(Adapter);
	*poid_par_priv->bytes_rw = poid_par_priv->information_buf_len;

	RT_TRACE(_module_mp_, _drv_info_,
		 ("-oid_rt_get_efuse_max_size_hdl: size=%d status=0x%08X\n",
		  *(int*)poid_par_priv->information_buf, status));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_efuse_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status;

_func_enter_;

	RT_TRACE(_module_mp_, _drv_info_, ("+oid_rt_pro_efuse_hdl\n"));

	if (poid_par_priv->type_of_oid == QUERY_OID)
		status = oid_rt_pro_read_efuse_hdl(poid_par_priv);
	else
		status = oid_rt_pro_write_efuse_hdl(poid_par_priv);

	RT_TRACE(_module_mp_, _drv_info_, ("-oid_rt_pro_efuse_hdl: status=0x%08X\n", status));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_pro_efuse_map_hdl(struct oid_par_priv *poid_par_priv)
{
	u8		*data;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PADAPTER	Adapter = (PADAPTER)(poid_par_priv->adapter_context);
	u16	mapLen=0;

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_pro_efuse_map_hdl\n"));

	EFUSE_GetEfuseDefinition(Adapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (PVOID)&mapLen, _FALSE);

	*poid_par_priv->bytes_rw = 0;

	if (poid_par_priv->information_buf_len < mapLen)
		return NDIS_STATUS_INVALID_LENGTH;

	data = (u8*)poid_par_priv->information_buf;

	_irqlevel_changed_(&oldirql, LOWER);

	if (poid_par_priv->type_of_oid == QUERY_OID)
	{
		RT_TRACE(_module_mp_, _drv_info_,
			("oid_rt_pro_efuse_map_hdl: READ\n"));

		if (rtw_efuse_map_read(Adapter, 0, mapLen, data) == _SUCCESS)
			*poid_par_priv->bytes_rw = mapLen;
		else {
			RT_TRACE(_module_mp_, _drv_err_,
				("oid_rt_pro_efuse_map_hdl: READ fail\n"));
			status = NDIS_STATUS_FAILURE;
		}
	} else {
		// SET_OID
		RT_TRACE(_module_mp_, _drv_info_,
			("oid_rt_pro_efuse_map_hdl: WRITE\n"));

		if (rtw_efuse_map_write(Adapter, 0, mapLen, data) == _SUCCESS)
			*poid_par_priv->bytes_rw = mapLen;
		else {
			RT_TRACE(_module_mp_, _drv_err_,
				("oid_rt_pro_efuse_map_hdl: WRITE fail\n"));
			status = NDIS_STATUS_FAILURE;
		}
	}

	_irqlevel_changed_(&oldirql, RAISE);

	RT_TRACE(_module_mp_, _drv_info_,
		 ("-oid_rt_pro_efuse_map_hdl: status=0x%08X\n", status));

_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_set_crystal_cap_hdl(struct oid_par_priv *poid_par_priv)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	return status;
}

NDIS_STATUS oid_rt_set_rx_packet_type_hdl(struct oid_par_priv *poid_par_priv)
{
	u8		rx_pkt_type;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;

_func_enter_;

	RT_TRACE(_module_mp_, _drv_notice_, ("+oid_rt_set_rx_packet_type_hdl\n"));

	if (poid_par_priv->type_of_oid != SET_OID)
		return NDIS_STATUS_NOT_ACCEPTED;

	if (poid_par_priv->information_buf_len < sizeof(u8))
		return NDIS_STATUS_INVALID_LENGTH;

	rx_pkt_type = *((u8*)poid_par_priv->information_buf);//4

	RT_TRACE(_module_mp_, _drv_info_, ("rx_pkt_type: %x\n",rx_pkt_type ));
#if 0
	_irqlevel_changed_(&oldirql, LOWER);
#if 0
	rcr_val8 = rtw_read8(Adapter, 0x10250048);//RCR
	rcr_val8 &= ~(RCR_AB|RCR_AM|RCR_APM|RCR_AAP);

	if(rx_pkt_type == RX_PKT_BROADCAST){
		rcr_val8 |= (RCR_AB | RCR_ACRC32 );
	}
	else if(rx_pkt_type == RX_PKT_DEST_ADDR){
		rcr_val8 |= (RCR_AAP| RCR_AM |RCR_ACRC32);
	}
	else if(rx_pkt_type == RX_PKT_PHY_MATCH){
		rcr_val8 |= (RCR_APM|RCR_ACRC32);
	}
	else{
		rcr_val8 &= ~(RCR_AAP|RCR_APM|RCR_AM|RCR_AB|RCR_ACRC32);
	}
	rtw_write8(padapter, 0x10250048,rcr_val8);
#else
	rcr_val32 = rtw_read32(padapter, RCR);//RCR = 0x10250048
	rcr_val32 &= ~(RCR_CBSSID|RCR_AB|RCR_AM|RCR_APM|RCR_AAP);
#if 0
	if(rx_pkt_type == RX_PKT_BROADCAST){
		rcr_val32 |= (RCR_AB|RCR_AM|RCR_APM|RCR_AAP|RCR_ACRC32);
	}
	else if(rx_pkt_type == RX_PKT_DEST_ADDR){
		//rcr_val32 |= (RCR_CBSSID|RCR_AAP|RCR_AM|RCR_ACRC32);
		rcr_val32 |= (RCR_CBSSID|RCR_APM|RCR_ACRC32);
	}
	else if(rx_pkt_type == RX_PKT_PHY_MATCH){
		rcr_val32 |= (RCR_APM|RCR_ACRC32);
		//rcr_val32 |= (RCR_AAP|RCR_ACRC32);
	}
	else{
		rcr_val32 &= ~(RCR_AAP|RCR_APM|RCR_AM|RCR_AB|RCR_ACRC32);
	}
#else
	switch (rx_pkt_type)
	{
		case RX_PKT_BROADCAST :
			rcr_val32 |= (RCR_AB|RCR_AM|RCR_APM|RCR_AAP|RCR_ACRC32);
			break;
		case RX_PKT_DEST_ADDR :
			rcr_val32 |= (RCR_AB|RCR_AM|RCR_APM|RCR_AAP|RCR_ACRC32);
			break;
		case RX_PKT_PHY_MATCH:
			rcr_val32 |= (RCR_APM|RCR_ACRC32);
			break;
		default:
			rcr_val32 &= ~(RCR_AAP|RCR_APM|RCR_AM|RCR_AB|RCR_ACRC32);
			break;
	}

	if (rx_pkt_type == RX_PKT_DEST_ADDR) {
		padapter->mppriv.check_mp_pkt = 1;
	} else {
		padapter->mppriv.check_mp_pkt = 0;
	}
#endif
	rtw_write32(padapter, RCR, rcr_val32);

#endif
	_irqlevel_changed_(&oldirql, RAISE);
#endif
_func_exit_;

	return status;
}

NDIS_STATUS oid_rt_pro_set_tx_agc_offset_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}

NDIS_STATUS oid_rt_pro_set_pkt_test_mode_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}

unsigned int mp_ioctl_xmit_packet_hdl(struct oid_par_priv *poid_par_priv)
{
	PMP_XMIT_PARM pparm;
	PADAPTER padapter;
	struct mp_priv *pmp_priv;
	struct pkt_attrib *pattrib;

	RT_TRACE(_module_mp_, _drv_notice_, ("+%s\n", __func__));

	pparm = (PMP_XMIT_PARM)poid_par_priv->information_buf;
	padapter = (PADAPTER)poid_par_priv->adapter_context;
	pmp_priv = &padapter->mppriv;

	if (poid_par_priv->type_of_oid == QUERY_OID) {
		pparm->enable = !pmp_priv->tx.stop;
		pparm->count = pmp_priv->tx.sended;
	} else {
		if (pparm->enable == 0) {
			pmp_priv->tx.stop = 1;
		} else if (pmp_priv->tx.stop == 1) {
			pmp_priv->tx.stop = 0;
			pmp_priv->tx.count = pparm->count;
			pmp_priv->tx.payload = pparm->payload_type;
			pattrib = &pmp_priv->tx.attrib;
			pattrib->pktlen = pparm->length;
			_rtw_memcpy(pattrib->dst, pparm->da, ETH_ALEN);
			SetPacketTx(padapter);
		} else
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

#if 0
unsigned int mp_ioctl_xmit_packet_hdl(struct oid_par_priv *poid_par_priv)
{
	unsigned char *pframe, *pmp_pkt;
	struct ethhdr *pethhdr;
	struct pkt_attrib *pattrib;
	struct rtw_ieee80211_hdr *pwlanhdr;
	unsigned short *fctrl;
	int llc_sz, payload_len;
	struct mp_xmit_frame *pxframe=  NULL;
	struct mp_xmit_packet *pmp_xmitpkt = (struct mp_xmit_packet*)param;
	u8 addr3[] = {0x02, 0xE0, 0x4C, 0x87, 0x66, 0x55};

//	DBG_871X("+mp_ioctl_xmit_packet_hdl\n");

	pxframe = alloc_mp_xmitframe(&padapter->mppriv);
	if (pxframe == NULL)
	{
		DEBUG_ERR(("Can't alloc pmpframe %d:%s\n", __LINE__, __FILE__));
		return -1;
	}

	//mp_xmit_pkt
	payload_len = pmp_xmitpkt->len - 14;
	pmp_pkt = (unsigned char*)pmp_xmitpkt->mem;
	pethhdr = (struct ethhdr *)pmp_pkt;

	//DBG_871X("payload_len=%d, pkt_mem=0x%x\n", pmp_xmitpkt->len, (void*)pmp_xmitpkt->mem);

	//DBG_871X("pxframe=0x%x\n", (void*)pxframe);
	//DBG_871X("pxframe->mem=0x%x\n", (void*)pxframe->mem);

	//update attribute
	pattrib = &pxframe->attrib;
	memset((u8 *)(pattrib), 0, sizeof (struct pkt_attrib));
	pattrib->pktlen = pmp_xmitpkt->len;
	pattrib->ether_type = ntohs(pethhdr->h_proto);
	pattrib->hdrlen = 24;
	pattrib->nr_frags = 1;
	pattrib->priority = 0;
#ifndef CONFIG_MP_LINUX
	if(IS_MCAST(pethhdr->h_dest))
		pattrib->mac_id = 4;
	else
		pattrib->mac_id = 5;
#else
	pattrib->mac_id = 5;
#endif

	//
	memset(pxframe->mem, 0 , WLANHDR_OFFSET);
	pframe = (u8 *)(pxframe->mem) + WLANHDR_OFFSET;

	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	SetFrameSubType(pframe, WIFI_DATA);

	_rtw_memcpy(pwlanhdr->addr1, pethhdr->h_dest, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, pethhdr->h_source, ETH_ALEN);

	_rtw_memcpy(pwlanhdr->addr3, addr3, ETH_ALEN);

	pwlanhdr->seq_ctl = 0;
	pframe += pattrib->hdrlen;

	llc_sz= rtw_put_snap(pframe, pattrib->ether_type);
	pframe += llc_sz;

	_rtw_memcpy(pframe, (void*)(pmp_pkt+14),  payload_len);

	pattrib->last_txcmdsz = pattrib->hdrlen + llc_sz + payload_len;

	DEBUG_INFO(("issuing mp_xmit_frame, tx_len=%d, ether_type=0x%x\n", pattrib->last_txcmdsz, pattrib->ether_type));
	xmit_mp_frame(padapter, pxframe);

	return _SUCCESS;
}
#endif
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_set_power_down_hdl(struct oid_par_priv *poid_par_priv)
{
	u8		bpwrup;
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;

_func_enter_;

	if (poid_par_priv->type_of_oid != SET_OID) {
		status = NDIS_STATUS_NOT_ACCEPTED;
		return status;
	}

	RT_TRACE(_module_mp_, _drv_info_,
		 ("\n ===> Setoid_rt_set_power_down_hdl.\n"));

	_irqlevel_changed_(&oldirql, LOWER);

	bpwrup = *(u8 *)poid_par_priv->information_buf;
	//CALL  the power_down function
	_irqlevel_changed_(&oldirql, RAISE);

	//DEBUG_ERR(("\n <=== Query OID_RT_PRO_READ_REGISTER.
	//	Add:0x%08x Width:%d Value:0x%08x\n",RegRWStruct->offset,RegRWStruct->width,RegRWStruct->value));

_func_exit_;

	return status;
}
//------------------------------------------------------------------------------
NDIS_STATUS oid_rt_get_power_mode_hdl(struct oid_par_priv *poid_par_priv)
{
	return 0;
}
