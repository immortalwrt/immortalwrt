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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/


#ifdef MULTI_INF_SUPPORT

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>

/* Index 0 for Card_1, Index 1 for Card_2 */
VOID *adapt_list[MAX_NUM_OF_INF] = {NULL};

int multi_inf_adapt_reg(VOID *pAd)
{
	int status = 0;

	if (adapt_list[0] == NULL)
		adapt_list[0] = pAd;
	else if (adapt_list[1] == NULL)
		adapt_list[1] = pAd;
	else if (adapt_list[2] == NULL)
		adapt_list[2] = pAd;
	else {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "adapt_list assign error !\n");
		status = NDIS_STATUS_FAILURE;
	}

	return status;
}

int multi_inf_adapt_unreg(VOID *pAd)
{
	int status = 0;

	if (adapt_list[0] == pAd)
		adapt_list[0] = NULL;
	else if (adapt_list[1] == pAd)
		adapt_list[1] = NULL;
	else if (adapt_list[2] == pAd)
		adapt_list[2] = NULL;
	else {
		MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "adapt_list assign error !\n");
		status = NDIS_STATUS_FAILURE;
	}

	return status;
}

int multi_inf_get_count(void)
{
	int count = 0; /* use number 0 as default */
	int idx;

	for (idx = 0; idx < MAX_NUM_OF_INF; idx++) {
		if (adapt_list[idx] != NULL)
			count++;
	}

	if (count == 0)
		MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "failed to find nonempty adapt_list!\n");
	return count;
}

int multi_inf_active_cnt(void)
{
	int active_cnt = 0; /* use number 0 as default */
	int idx;

	for (idx = 0; idx < MAX_NUM_OF_INF; idx++) {
		if (adapt_list[idx] != NULL) {
			if (VIRTUAL_IF_NUM(adapt_list[idx]) == 0) {
			} else {
				active_cnt++;
			}
		}
	}
	if (active_cnt == 0)
		MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "failed to find nonempty adapt_list!\n");
	return active_cnt;
}

int multi_inf_get_idx(VOID *pAd)
{
	int idx = 0; /* use index 0 as default */

	for (idx = 0; idx < MAX_NUM_OF_INF; idx++) {
		if (pAd == adapt_list[idx])
			return idx;
	}

	MTWF_DBG(pAd, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "failed to find the index in adapt_list!\n");
	return idx;
}
EXPORT_SYMBOL(multi_inf_get_idx);

/* Driver module load/unload function */
static int __init wifi_drv_init_module(void)
{
	int status = 0;

	os_module_init();
#ifdef RTMP_RBUS_SUPPORT
	status = wbsys_module_init();

	if (status)
		MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Register RBUS device driver failed(%d)!\n", status);

#endif /* RTMP_RBUS_SUPPORT */

#ifdef RTMP_PCI_SUPPORT
	status = rt_pci_init_module();

	if (status)
		MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 "Register PCI device driver failed(%d)!\n", status);

#endif /* RTMP_PCI_SUPPORT */
	return status;
}


static void __exit wifi_drv_cleanup_module(void)
{
#ifdef RTMP_PCI_SUPPORT
	rt_pci_cleanup_module();
	MTWF_DBG(NULL, DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 "Unregister PCI device driver\n");
#endif /* RTMP_PCI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
	wbsys_module_exit();
#endif /* RTMP_RBUS_SUPPORT */
	os_module_exit();
}


module_init(wifi_drv_init_module);
module_exit(wifi_drv_cleanup_module);

#endif /* MULTI_INF_SUPPORT */

