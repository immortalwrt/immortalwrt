/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************
	Module Name:
	test_mac.h

*/
#ifndef __TEST_MAC_H__
#define __TEST_MAC_H__

#include "net_adaption.h"

enum {
	RESTORE_ALL = 1,
	RESTORE_TX,
	MAX_RESTORE_OPTION
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
s_int32 mt_test_mac_backup_and_set_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx);
s_int32 mt_test_mac_restore_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *bks,
	u_char band_idx,
	u_char option);
s_int32 mt_test_mac_set_ampdu_ba_limit(
	struct test_wlan_info *winfos,
	u_int8 wmm_idx,
	u_int8 agg_limit);
s_int32 mt_test_mac_set_sta_pause_cr(
	struct test_wlan_info *winfos);
s_int32 mt_test_mac_set_ifs_cr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx);

#endif /* __TEST_MAC_H__ */
