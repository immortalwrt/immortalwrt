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
/****************************************************************************
 ****************************************************************************

    Module Name:
	ap_ioctl.c

    Abstract:
    IOCTL related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/
#define RTMP_MODULE_OS

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/wireless.h>
#include "rtmp_def.h"

#if (KERNEL_VERSION(4, 12, 0) <= LINUX_VERSION_CODE)
static const struct iw_ioctl_description ap_standard_ioctl[] = {
	[IW_IOCTL_IDX(SIOCSIWCOMMIT)] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[IW_IOCTL_IDX(SIOCGIWNAME)] = {
		.header_type	= IW_HEADER_TYPE_CHAR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWNWID)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[IW_IOCTL_IDX(SIOCGIWNWID)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWFREQ)] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[IW_IOCTL_IDX(SIOCGIWFREQ)] = {
		.header_type	= IW_HEADER_TYPE_FREQ,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWMODE)] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[IW_IOCTL_IDX(SIOCGIWMODE)] = {
		.header_type	= IW_HEADER_TYPE_UINT,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWSENS)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWSENS)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWRANGE)] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[IW_IOCTL_IDX(SIOCGIWRANGE)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_range),
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWPRIV)] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[IW_IOCTL_IDX(SIOCGIWPRIV)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_priv_args),
		.max_tokens	= 16,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[IW_IOCTL_IDX(SIOCSIWSTATS)] = {
		.header_type	= IW_HEADER_TYPE_NULL,
	},
	[IW_IOCTL_IDX(SIOCGIWSTATS)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= sizeof(struct iw_statistics),
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWSPY)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr),
		.max_tokens	= IW_MAX_SPY,
	},
	[IW_IOCTL_IDX(SIOCGIWSPY)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_SPY,
	},
	[IW_IOCTL_IDX(SIOCSIWTHRSPY)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[IW_IOCTL_IDX(SIOCGIWTHRSPY)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct iw_thrspy),
		.min_tokens	= 1,
		.max_tokens	= 1,
	},
	[IW_IOCTL_IDX(SIOCSIWAP)] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
	},
	[IW_IOCTL_IDX(SIOCGIWAP)] = {
		.header_type	= IW_HEADER_TYPE_ADDR,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWMLME)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_mlme),
		.max_tokens	= sizeof(struct iw_mlme),
	},
	[IW_IOCTL_IDX(SIOCGIWAPLIST)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= sizeof(struct sockaddr) +
				  sizeof(struct iw_quality),
		.max_tokens	= IW_MAX_AP,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[IW_IOCTL_IDX(SIOCSIWSCAN)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= 0,
		.max_tokens	= sizeof(struct iw_scan_req),
	},
	[IW_IOCTL_IDX(SIOCGIWSCAN)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_SCAN_MAX_DATA,
		.flags		= IW_DESCR_FLAG_NOMAX,
	},
	[IW_IOCTL_IDX(SIOCSIWESSID)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
		.flags		= IW_DESCR_FLAG_EVENT,
	},
	[IW_IOCTL_IDX(SIOCGIWESSID)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
		.flags		= IW_DESCR_FLAG_DUMP,
	},
	[IW_IOCTL_IDX(SIOCSIWNICKN)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
	},
	[IW_IOCTL_IDX(SIOCGIWNICKN)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ESSID_MAX_SIZE,
	},
	[IW_IOCTL_IDX(SIOCSIWRATE)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWRATE)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWRTS)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWRTS)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWFRAG)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWFRAG)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWTXPOW)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWTXPOW)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWRETRY)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWRETRY)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWENCODE)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_EVENT | IW_DESCR_FLAG_RESTRICT,
	},
	[IW_IOCTL_IDX(SIOCGIWENCODE)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_ENCODING_TOKEN_MAX,
		.flags		= IW_DESCR_FLAG_DUMP | IW_DESCR_FLAG_RESTRICT,
	},
	[IW_IOCTL_IDX(SIOCSIWPOWER)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWPOWER)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWGENIE)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IW_IOCTL_IDX(SIOCGIWGENIE)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.max_tokens	= IW_GENERIC_IE_MAX,
	},
	[IW_IOCTL_IDX(SIOCSIWAUTH)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCGIWAUTH)] = {
		.header_type	= IW_HEADER_TYPE_PARAM,
	},
	[IW_IOCTL_IDX(SIOCSIWENCODEEXT)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[IW_IOCTL_IDX(SIOCGIWENCODEEXT)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_encode_ext),
		.max_tokens	= sizeof(struct iw_encode_ext) +
				  IW_ENCODING_TOKEN_MAX,
	},
	[IW_IOCTL_IDX(SIOCSIWPMKSA)] = {
		.header_type	= IW_HEADER_TYPE_POINT,
		.token_size	= 1,
		.min_tokens	= sizeof(struct iw_pmksa),
		.max_tokens	= sizeof(struct iw_pmksa),
	},
};
static const unsigned int ap_standard_ioctl_num = ARRAY_SIZE(ap_standard_ioctl);

#ifdef CONFIG_WEXT_PRIV
/* size (in bytes) of the various private data types */
static const char ap_iw_priv_type_size[] = {
	0,							/* IW_PRIV_TYPE_NONE */
	1,							/* IW_PRIV_TYPE_BYTE */
	1,							/* IW_PRIV_TYPE_CHAR */
	0,							/* Not defined */
	sizeof(__u32),				/* IW_PRIV_TYPE_INT */
	sizeof(struct iw_freq),		/* IW_PRIV_TYPE_FLOAT */
	sizeof(struct sockaddr),	/* IW_PRIV_TYPE_ADDR */
	0,							/* Not defined */
};

static int ap_get_priv_size(__u16 args)
{
	int	num = args & IW_PRIV_SIZE_MASK;
	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;

	return num * ap_iw_priv_type_size[type];
}

static int ap_adjust_priv_size(__u16 args, struct iw_point *iwp)
{
	int	num = iwp->length;
	int	max = args & IW_PRIV_SIZE_MASK;
	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;

	if (max < num)
		num = max;

	return num * ap_iw_priv_type_size[type];
}

static int ap_get_priv_descr_and_size(struct net_device *dev, unsigned int cmd,
										const struct iw_priv_args **descrp)
{
	const struct iw_priv_args *descr;
	int i, extra_size;

	descr = NULL;
	for (i = 0; i < dev->wireless_handlers->num_private_args; i++) {
		if (cmd == dev->wireless_handlers->private_args[i].cmd) {
			descr = &dev->wireless_handlers->private_args[i];
			break;
		}
	}

	extra_size = 0;
	if (descr) {
		if (IW_IS_GET(cmd)) {
			/* size of get arguments */
			extra_size = ap_get_priv_size(descr->get_args);

			if ((descr->get_args & IW_PRIV_SIZE_FIXED) &&
			   (extra_size <= IFNAMSIZ))
				extra_size = 0;
		}
	}
	*descrp = descr;
	return extra_size;
}
#endif

INT ap_iw_handler(struct net_device *dev, struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	INT ret;
	struct iwreq *wreq = CONTAINER_OF(wrqu, struct iwreq, u);
	const struct iw_ioctl_description *	standard_descr;

	if (dev->netdev_ops->ndo_do_ioctl) {
		ret = dev->netdev_ops->ndo_do_ioctl(dev, (struct ifreq *)wreq, info->cmd);
	} else {
		return -EOPNOTSUPP;
	}

	if (ret != NDIS_STATUS_SUCCESS || !IW_IS_GET(info->cmd) || NULL == extra)
		return ret;

	/* fill info to extra for get command */
	if (info->cmd < SIOCIWFIRSTPRIV) {
		if (IW_IOCTL_IDX(info->cmd) >= ap_standard_ioctl_num)
			return -EOPNOTSUPP;
		standard_descr = &(ap_standard_ioctl[IW_IOCTL_IDX(info->cmd)]);
		if (IW_HEADER_TYPE_POINT == standard_descr->header_type)
			ret = copy_from_user(extra, wrqu->data.pointer,
						wrqu->data.length * standard_descr->token_size);
	} else {
#ifdef CONFIG_WEXT_PRIV
		INT extra_size;
		const struct iw_priv_args *private_descr;

		extra_size = ap_get_priv_descr_and_size(dev, info->cmd, &private_descr);
		if (extra_size != 0) {
			if (!(private_descr->get_args & IW_PRIV_SIZE_FIXED))
				extra_size = ap_adjust_priv_size(private_descr->get_args, &wrqu->data);
			ret = copy_from_user(extra, wrqu->data.pointer, extra_size);
		}
#endif
	}

	return ret;
}

static iw_handler ap_handler[] = {
	ap_iw_handler,	/* SIOCSIWCOMMIT */
	ap_iw_handler,	/* SIOCGIWNAME */
	ap_iw_handler,	/* SIOCSIWNWID */
	ap_iw_handler,	/* SIOCGIWNWID */
	ap_iw_handler,	/* SIOCSIWFREQ */
	ap_iw_handler,	/* SIOCGIWFREQ */
	ap_iw_handler,	/* SIOCSIWMODE */
	ap_iw_handler,	/* SIOCGIWMODE */
	ap_iw_handler,	/* SIOCSIWSENS */
	ap_iw_handler,	/* SIOCGIWSENS */
	ap_iw_handler,	/* SIOCSIWRANGE */
	ap_iw_handler,	/* SIOCGIWRANGE */
	ap_iw_handler,	/* SIOCSIWPRIV */
	ap_iw_handler,	/* SIOCGIWPRIV */
	ap_iw_handler,	/* SIOCSIWSTATS */
	ap_iw_handler,	/* SIOCGIWSTATS */
	ap_iw_handler,	/* SIOCSIWSPY */
	ap_iw_handler,	/* SIOCGIWSPY */
	ap_iw_handler,	/* SIOCSIWTHRSPY */
	ap_iw_handler,	/* SIOCGIWTHRSPY */
	ap_iw_handler,	/* SIOCSIWAP */
	ap_iw_handler,	/* SIOCGIWAP */
	ap_iw_handler,	/* SIOCSIWMLME */
	ap_iw_handler,	/* SIOCGIWAPLIST */
	ap_iw_handler,	/* SIOCSIWSCAN */
	ap_iw_handler,	/* SIOCGIWSCAN */
	ap_iw_handler,	/* SIOCSIWESSID */
	ap_iw_handler,	/* SIOCGIWESSID */
	ap_iw_handler,	/* SIOCSIWNICKN */
	ap_iw_handler,	/* SIOCGIWNICKN */
	ap_iw_handler,	/* 0x8B1E */
	ap_iw_handler,	/* 0x8B1F */
	ap_iw_handler,	/* SIOCSIWRATE */
	ap_iw_handler,	/* SIOCGIWRATE */
	ap_iw_handler,	/* SIOCSIWRTS */
	ap_iw_handler,	/* SIOCGIWRTS */
	ap_iw_handler,	/* SIOCSIWFRAG */
	ap_iw_handler,	/* SIOCGIWFRAG */
	ap_iw_handler,	/* SIOCSIWTXPOW */
	ap_iw_handler,	/* SIOCGIWTXPOW */
	ap_iw_handler,	/* SIOCSIWRETRY */
	ap_iw_handler,	/* SIOCGIWRETRY */
	ap_iw_handler,	/* SIOCSIWENCODE */
	ap_iw_handler,	/* SIOCGIWENCODE */
	ap_iw_handler,	/* SIOCSIWPOWER */
	ap_iw_handler,	/* SIOCGIWPOWER */
	ap_iw_handler,	/* SIOCSIWMODUL */
	ap_iw_handler,	/* SIOCGIWMODUL */
	ap_iw_handler,	/* SIOCSIWGENIE */
	ap_iw_handler,	/* SIOCGIWGENIE */
	ap_iw_handler,	/* SIOCSIWAUTH */
	ap_iw_handler,	/* SIOCGIWAUTH */
	ap_iw_handler,	/* SIOCSIWENCODEEXT */
	ap_iw_handler,	/* SIOCGIWENCODEEXT */
	ap_iw_handler,	/* SIOCSIWPMKSA */
};

#ifdef CONFIG_WEXT_PRIV
static iw_handler ap_priv_handler[] = {
	ap_iw_handler,	/* SIOCIWFIRSTPRIV + 0x00 */
	ap_iw_handler,	/* RT_PRIV_IOCTL = SIOCIWFIRSTPRIV + 0x01 */
	ap_iw_handler,	/* RTPRIV_IOCTL_SET = SIOCIWFIRSTPRIV + 0x02 */
	ap_iw_handler,	/* RTPRIV_IOCTL_BBP = SIOCIWFIRSTPRIV + 0x03 */
    ap_iw_handler,	/* SIOCIWFIRSTPRIV + 0x04 */
	ap_iw_handler,	/* RTPRIV_IOCTL_MAC = SIOCIWFIRSTPRIV + 0x05 */
	ap_iw_handler,	/* SIOCIWFIRSTPRIV + 0x06 */
	ap_iw_handler,	/* RTPRIV_IOCTL_E2P = SIOCIWFIRSTPRIV + 0x07 */
	ap_iw_handler,	/* RTPRIV_IOCTL_ATE = SIOCIWFIRSTPRIV + 0x08 */
	ap_iw_handler,	/* RTPRIV_IOCTL_STATISTICS = SIOCIWFIRSTPRIV + 0x09 */
	ap_iw_handler,	/* RTPRIV_IOCTL_ADD_PMKID_CACHE = SIOCIWFIRSTPRIV + 0x0a */
	ap_iw_handler,	/* MTPRIV_IOCTL_META_SET_EM = SIOCIWFIRSTPRIV + 0x0b */
	ap_iw_handler,	/* RTPRIV_IOCTL_RADIUS_DATA = SIOCIWFIRSTPRIV + 0x0c */
	ap_iw_handler,	/* RTPRIV_IOCTL_GSITESURVEY = SIOCIWFIRSTPRIV + 0x0d */
    ap_iw_handler,	/* RT_PRIV_IOCTL_EXT = SIOCIWFIRSTPRIV + 0x0e */
	ap_iw_handler,	/* RTPRIV_IOCTL_GET_MAC_TABLE = SIOCIWFIRSTPRIV + 0x0f */
	ap_iw_handler,	/* RTPRIV_IOCTL_STATIC_WEP_COPY = SIOCIWFIRSTPRIV + 0x10 */
	ap_iw_handler,	/* RTPRIV_IOCTL_SHOW = SIOCIWFIRSTPRIV + 0x11 */
	ap_iw_handler,	/* RTPRIV_IOCTL_WSC_PROFILE = SIOCIWFIRSTPRIV + 0x12 */
	ap_iw_handler,	/* RTPRIV_IOCTL_RF = SIOCIWFIRSTPRIV + 0x13 */
	ap_iw_handler,	/* RTPRIV_IOCTL_SET_WSC_PROFILE_U32_ITEM = SIOCIWFIRSTPRIV + 0x14 */
	ap_iw_handler,	/* RTPRIV_IOCTL_STATISTICS = SIOCIWFIRSTPRIV + 0x15 */
	ap_iw_handler,	/* RTPRIV_IOCTL_SET_WSC_PROFILE_STRING_ITEM = SIOCIWFIRSTPRIV + 0x16 */
	ap_iw_handler,	/* MTPRIV_IOCTL_RD = SIOCIWFIRSTPRIV + 0x17 */
    ap_iw_handler,	/* RTPRIV_IOCTL_SET_FT_PARAM = SIOCIWFIRSTPRIV + 0x18 */
	ap_iw_handler,	/* RTPRIV_IOCTL_SET_WSCOOB = SIOCIWFIRSTPRIV + 0x19 */
	ap_iw_handler,	/* RTPRIV_IOCTL_WSC_CALLBACK = SIOCIWFIRSTPRIV + 0x1a */
	ap_iw_handler,	/* RTPRIV_IOCTL_RX_STATISTICS = SIOCIWFIRSTPRIV + 0x1b */
	ap_iw_handler,	/* SIOCIWFIRSTPRIV + 0x1c */
	ap_iw_handler,	/* RTPRIV_IOCTL_GET_DRIVER_INFO = SIOCIWFIRSTPRIV + 0x1d */
	ap_iw_handler,	/* RTPRIV_IOCTL_STA_VLAN = SIOCIWFIRSTPRIV + 0x1e */
	ap_iw_handler	/* RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT = SIOCIWFIRSTPRIV + 0x1f */
};
#endif
#endif

struct iw_priv_args ap_priv_tab[] = {
	{
		RTPRIV_IOCTL_SET,
		/* 1024 --> 1024 + 512 */
		/* larger size specific to allow 64 ACL MAC addresses to be set up all at once. */
		IW_PRIV_TYPE_CHAR | 1536, 0,
		"set"
	},
	{
		RTPRIV_IOCTL_SHOW,
		IW_PRIV_TYPE_CHAR | 1024, 0,
		"show"
	},
	{
		RTPRIV_IOCTL_PHY_STATE,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"phystate"
	},
	{
		RTPRIV_IOCTL_GSITESURVEY,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"get_site_survey"
	},
	{
		RTPRIV_IOCTL_SET_WSCOOB,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"set_wsc_oob"
	},
	{
		RTPRIV_IOCTL_GET_MAC_TABLE,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"get_mac_table"
	},
	{
		RTPRIV_IOCTL_GET_DRIVER_INFO,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"get_driverinfo"
	},
	{
		RTPRIV_IOCTL_E2P,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"e2p"
	},
#if defined(DBG) || (defined(BB_SOC) && defined(CONFIG_ATE))
	{
		RTPRIV_IOCTL_BBP,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"bbp"
	},
	{
		RTPRIV_IOCTL_MAC,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"mac"
	},
#ifdef RTMP_RF_RW_SUPPORT
	{
		RTPRIV_IOCTL_RF,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"rf"
	},
#endif /* RTMP_RF_RW_SUPPORT */
#endif /* defined(DBG) ||(defined(BB_SOC) && defined(CONFIG_ATE)) */

#ifdef WSC_AP_SUPPORT
	{
		RTPRIV_IOCTL_WSC_PROFILE,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"get_wsc_profile"
	},
#endif /* WSC_AP_SUPPORT */
	{
		RTPRIV_IOCTL_QUERY_BATABLE,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"get_ba_table"
	},
	{
		RTPRIV_IOCTL_STATISTICS,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"stat"
	},
	{
		MTPRIV_IOCTL_RD,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"rd"
	},
	{
		RTPRIV_IOCTL_RX_STATISTICS,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | 1024,
		"rx"
	}
};

#ifdef CONFIG_APSTA_MIXED_SUPPORT
const struct iw_handler_def rt28xx_ap_iw_handler_def = {
#if (KERNEL_VERSION(4, 12, 0) <= LINUX_VERSION_CODE)
	.standard = ap_handler,
	.num_standard = ARRAY_SIZE(ap_handler),
#endif
#ifdef CONFIG_WEXT_PRIV
#if (KERNEL_VERSION(4, 12, 0) <= LINUX_VERSION_CODE)
	.private = ap_priv_handler,
	.num_private = ARRAY_SIZE(ap_priv_handler),
#endif
	.private_args = (struct iw_priv_args *) ap_priv_tab,
	.num_private_args = ARRAY_SIZE(ap_priv_tab),
#endif /* CONFIG_WEXT_PRIV */
#if IW_HANDLER_VERSION >= 7
	.get_wireless_stats = rt28xx_get_wireless_stats,
#endif
};
#endif /* CONFIG_APSTA_MIXED_SUPPORT */


INT rt28xx_ap_ioctl(void *net_dev_obj, void *data_obj, int cmd) /* snowpin for ap/sta */
{
	struct net_device *net_dev;
	VOID			*pAd = NULL;
    struct iwreq	*wrqin = (struct iwreq *) data_obj;
	RTMP_IOCTL_INPUT_STRUCT rt_wrq, *wrq = &rt_wrq;
	INT				Status = NDIS_STATUS_SUCCESS;
	USHORT			subcmd;
	INT			apidx = 0;
	UINT32		org_len;
	RT_CMD_AP_IOCTL_CONFIG IoctlConfig, *pIoctlConfig = &IoctlConfig;

	net_dev = (struct net_device *)net_dev_obj;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL) {
		/* if 1st open fail, pAd will be free;
		 *  So the net_dev->priv will be NULL in 2rd open
		 */
		return -ENETDOWN;
	}
	memcpy(wrq->ifr_ifrn.ifrn_name, wrqin->ifr_ifrn.ifrn_name, IFNAMSIZ);

	wrq->u.data.pointer = wrqin->u.data.pointer;
	wrq->u.data.length = wrqin->u.data.length;
	org_len = wrq->u.data.length;
	pIoctlConfig->Status = 0;
	pIoctlConfig->net_dev = net_dev;
	pIoctlConfig->wdev = RTMP_OS_NETDEV_GET_WDEV(net_dev);
	pIoctlConfig->priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	if (wrqin->u.data.length)
	pIoctlConfig->pCmdData = wrqin->u.data.pointer;
	else
		pIoctlConfig->pCmdData = NULL;
	pIoctlConfig->cmd_data_len = wrqin->u.data.length;
	pIoctlConfig->CmdId_RTPRIV_IOCTL_SET = RTPRIV_IOCTL_SET;
	pIoctlConfig->name = net_dev->name;
	pIoctlConfig->apidx = 0;

	if ((cmd != SIOCGIWPRIV) &&
		RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_PREPARE, 0,
							pIoctlConfig, 0) != NDIS_STATUS_SUCCESS) {
		/* prepare error */
		Status = pIoctlConfig->Status;
		goto LabelExit;
	}

	apidx = pIoctlConfig->apidx;

	/*+ patch for SnapGear Request even the interface is down */
	if (cmd == SIOCGIWNAME) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IOCTL::SIOCGIWNAME\n"));
		RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SIOCGIWNAME, 0, wrqin->u.name, 0);
		return Status;
	} /*- patch for SnapGear */


	switch (cmd) {
#ifdef WCX_SUPPORT
#else

	case RTPRIV_IOCTL_ATE: {
		RTMP_COM_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_ATE, 0, wrqin->ifr_name, 0);
	}
	break;
#endif

#ifdef DYNAMIC_VLAN_SUPPORT
	case RTPRIV_IOCTL_STA_VLAN:
		{
			RT_CMD_AP_STA_VLAN	sta_vlan_param;
			struct iw_point *erq = &wrqin->u.data;
			if (erq->pointer) {
				if (copy_from_user(&sta_vlan_param, erq->pointer, erq->length)) {
					Status = -EFAULT;
				} else {
					printk("STA Addr %02x %02x %02x %02x %02x %02x Vlan ID %d\n",
						sta_vlan_param.sta_addr[0], sta_vlan_param.sta_addr[1], sta_vlan_param.sta_addr[2],
						sta_vlan_param.sta_addr[3], sta_vlan_param.sta_addr[4], sta_vlan_param.sta_addr[5],
						sta_vlan_param.vlan_id);
					RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_SET_STA_VLAN, 0, &sta_vlan_param, sizeof(RT_CMD_AP_STA_VLAN));
				}
			}

		}
		break;
#endif

#ifdef HOSTAPD_11R_SUPPORT
	case RTPRIV_IOCTL_SET_FT_PARAM:
		{
			RT_CMD_AP_11R_PARAM ap_11r_params;
			struct iw_point *erq = &wrqin->u.data;

			if (erq->length <= 12) {
				Status = 0;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("Set FT Param ioctl call failed due to length:%d\n", erq->length));
			} else {
				if (erq->pointer) {
					if (copy_from_user(&ap_11r_params, erq->pointer, erq->length))
						Status = -EFAULT;
					else
						RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_SET_FT_PARAM, 0,
									&ap_11r_params, sizeof(RT_CMD_AP_11R_PARAM));
				}
			}
		}
		break;
#endif

	case SIOCGIFHWADDR:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IOCTLIOCTLIOCTL::SIOCGIFHWADDR\n"));
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_AP_SIOCGIFHWADDR, 0, NULL, 0);
		/*  if (pObj->ioctl_if < MAX_MBSSID_NUM(pAd)) */
		/* strcpy((RTMP_STRING *) wrq->u.name, (RTMP_STRING *) pAd->ApCfg.MBSSID[pObj->ioctl_if].Bssid); */
		break;

	case SIOCSIWESSID:  /*Set ESSID */
		break;

	case SIOCGIWESSID: { /*Get ESSID */
		RT_CMD_AP_IOCTL_SSID IoctlSSID, *pIoctlSSID = &IoctlSSID;
		struct iw_point *erq = &wrqin->u.essid;
		PCHAR pSsidStr = NULL;

		erq->flags = 1;
		/*erq->length = pAd->ApCfg.MBSSID[pObj->ioctl_if].SsidLen; */
		pIoctlSSID->priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
		pIoctlSSID->apidx = apidx;
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_AP_SIOCGIWESSID, 0, pIoctlSSID, 0);
		pSsidStr = (PCHAR)pIoctlSSID->pSsidStr;
		erq->length = pIoctlSSID->length;

		if ((erq->pointer) && (pSsidStr != NULL)) {
			/*if(copy_to_user(erq->pointer, pAd->ApCfg.MBSSID[pObj->ioctl_if].Ssid, erq->length)) */
			if (copy_to_user(erq->pointer, pSsidStr, erq->length)) {
				Status = RTMP_IO_EFAULT;
				break;
			}
		}

		/* The below code tries to access user space buffer directly,
		 * hence remove it .
		 */

	}
	break;

	case SIOCGIWNWID: /* get network id */
	case SIOCSIWNWID: /* set network id (the cell) */
		Status = RTMP_IO_EOPNOTSUPP;
		break;

	case SIOCGIWFREQ: { /* get channel/frequency (Hz) */
		ULONG Channel;

		RTMP_DRIVER_CHANNEL_GET(pAd, pIoctlConfig->apidx, &Channel);
		wrqin->u.freq.m = Channel; /*wdev->channel; */
		wrqin->u.freq.e = 0;
		wrqin->u.freq.i = 0;
	}
	break;

	case SIOCSIWFREQ: /*set channel/frequency (Hz) */
		Status = RTMP_IO_EOPNOTSUPP;
		break;

	case SIOCGIWNICKN:
	case SIOCSIWNICKN: /*set node name/nickname */
		Status = RTMP_IO_EOPNOTSUPP;
		break;

	case SIOCGIWRATE: { /*get default bit rate (bps) */
		RT_CMD_IOCTL_RATE IoctlRate, *pIoctlRate = &IoctlRate;

		pIoctlRate->priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
		RTMP_DRIVER_BITRATE_GET(pAd, pIoctlRate);
		wrqin->u.bitrate.value = pIoctlRate->BitRate;
		wrqin->u.bitrate.disabled = 0;
	}
	break;

	case SIOCSIWRATE:  /*set default bit rate (bps) */
	case SIOCGIWRTS:  /* get RTS/CTS threshold (bytes) */
	case SIOCSIWRTS:  /*set RTS/CTS threshold (bytes) */
	case SIOCGIWFRAG:  /*get fragmentation thr (bytes) */
	case SIOCSIWFRAG:  /*set fragmentation thr (bytes) */
	case SIOCGIWENCODE:  /*get encoding token & mode */
	case SIOCSIWENCODE:  /*set encoding token & mode */
		Status = RTMP_IO_EOPNOTSUPP;
		break;

	case SIOCGIWAP: { /*get access point MAC addresses */
		/*				PCHAR pBssidStr; */
		wrqin->u.ap_addr.sa_family = ARPHRD_ETHER;
		/*memcpy(wrqin->u.ap_addr.sa_data, &pAd->ApCfg.MBSSID[pObj->ioctl_if].Bssid, ETH_ALEN); */
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_AP_SIOCGIWAP, 0,
							wrqin->u.ap_addr.sa_data, RT_DEV_PRIV_FLAGS_GET(net_dev));
	}
	break;

	case SIOCGIWMODE:  /*get operation mode */
		if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_APCLI)
			wrqin->u.mode = IW_MODE_INFRA;		/* ApCli Mode. */
		else
			wrqin->u.mode = IW_MODE_MASTER;		/* AP Mode. */
		break;

	case SIOCSIWAP:  /*set access point MAC addresses */
	case SIOCSIWMODE:  /*set operation mode */
	case SIOCGIWSENS:   /*get sensitivity (dBm) */
	case SIOCSIWSENS:	/*set sensitivity (dBm) */
	case SIOCGIWPOWER:  /*get Power Management settings */
	case SIOCSIWPOWER:  /*set Power Management settings */
	case SIOCGIWTXPOW:  /*get transmit power (dBm) */
	case SIOCSIWTXPOW:  /*set transmit power (dBm) */

	case SIOCGIWRANGE:	/*Get range of parameters */
	case SIOCGIWRETRY:	/*get retry limits and lifetime */
	case SIOCSIWRETRY:	/*set retry limits and lifetime */
		Status = RTMP_IO_EOPNOTSUPP;
		break;
	case RT_PRIV_IOCTL:
	case RT_PRIV_IOCTL_EXT: {
		subcmd = wrqin->u.data.flags;
		Status = RTMP_AP_IoctlHandle(pAd, wrq, CMD_RT_PRIV_IOCTL, subcmd, wrqin->u.data.pointer, 0);
	}
	break;

	case SIOCGIWPRIV:
		if (wrqin->u.data.pointer) {
			if (access_ok(wrqin->u.data.pointer, sizeof(ap_priv_tab)) != TRUE)
				break;

			if ((ARRAY_SIZE(ap_priv_tab)) <= wrq->u.data.length) {
				wrqin->u.data.length = ARRAY_SIZE(ap_priv_tab);

				if (copy_to_user(wrqin->u.data.pointer, ap_priv_tab, sizeof(ap_priv_tab)))
					Status = RTMP_IO_EFAULT;
			} else
				Status = RTMP_IO_E2BIG;
		}

		break;

	case RTPRIV_IOCTL_SET: {
		if (access_ok(wrqin->u.data.pointer, wrqin->u.data.length) == TRUE)
			Status = RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_SET, 0, NULL, 0);
	}
	break;

	case RTPRIV_IOCTL_SHOW: {
		if (access_ok(wrqin->u.data.pointer, wrqin->u.data.length) == TRUE)
			Status = RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_SHOW, 0, NULL, 0);
	}
	break;

	case RTPRIV_IOCTL_PHY_STATE: {
		if (access_ok(wrqin->u.data.pointer, wrqin->u.data.length) == TRUE)
			Status = RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_PHY_STATE, 0, NULL, 0);
	}
	break;

#ifdef WSC_AP_SUPPORT

	case RTPRIV_IOCTL_SET_WSCOOB:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_SET_WSCOOB, 0, NULL, 0);
		break;
#endif/*WSC_AP_SUPPORT*/

	/* modified by Red@Ralink, 2009/09/30 */
	case RTPRIV_IOCTL_GET_MAC_TABLE:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_GET_MAC_TABLE, 0, NULL, 0);
		break;

	case RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, 0, NULL, 0);
		break;
		/* end of modification */

	case RTPRIV_IOCTL_GET_DRIVER_INFO:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_GET_DRIVER_INFO, 0, NULL, 0);
		break;

#ifdef AP_SCAN_SUPPORT

	case RTPRIV_IOCTL_GSITESURVEY:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_GSITESURVEY, 0, NULL, 0);
		break;
#endif /* AP_SCAN_SUPPORT */


	case RTPRIV_IOCTL_STATISTICS:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_STATISTICS, 0, NULL, 0);
		break;

	case MTPRIV_IOCTL_RD:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_MTPRIV_IOCTL_RD, 0, NULL, 0);
		break;

	case RTPRIV_IOCTL_RX_STATISTICS:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_RX_STATISTICS, 0, NULL, 0);
		break;
#ifdef WSC_AP_SUPPORT

	case RTPRIV_IOCTL_WSC_PROFILE:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_WSC_PROFILE, 0, NULL, 0);
		break;
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT

	case RTPRIV_IOCTL_QUERY_BATABLE:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_QUERY_BATABLE, 0, NULL, 0);
		break;
#endif /* DOT11_N_SUPPORT */

	case RTPRIV_IOCTL_E2P:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_E2P, 0, NULL, 0);
		break;
#if defined(DBG) || (defined(BB_SOC) && defined(CONFIG_ATE))
	case RTPRIV_IOCTL_BBP:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_BBP, 0, NULL, 0);
		break;

	case RTPRIV_IOCTL_MAC:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_MAC, 0, NULL, 0);
		break;
#ifdef RTMP_RF_RW_SUPPORT

	case RTPRIV_IOCTL_RF:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_RF, 0, NULL, 0);
		break;
#endif /* RTMP_RF_RW_SUPPORT */
#endif /* defined(DBG) ||(defined(BB_SOC) && defined(CONFIG_ATE)) */

#ifdef WIFI_DIAG
	case RTPRIV_IOCTL_GET_PROCESS_INFO:
		RTMP_AP_IoctlHandle(pAd, wrq, CMD_RTPRIV_IOCTL_GET_PROCESS_INFO, 0, NULL, 0);
		break;
#endif

	default:
		/*			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("IOCTL::unknown IOCTL's cmd = 0x%08x\n", cmd)); */
		Status = RTMP_IO_EOPNOTSUPP;
		break;
	}

LabelExit:

	if (Status != 0) {
		RT_CMD_STATUS_TRANSLATE(Status);
	} else {
		/*
		 *	If wrq length is modified, we reset the lenght of origin wrq;
		 *
		 *	Or we can not modify it because the address of wrq->u.data.length
		 *	maybe same as other union field, ex: iw_range, etc.
		 *
		 *	if the length is not changed but we change it, the value for other
		 *	union will also be changed, this is not correct.
		 */
		if (wrq->u.data.length != org_len)
			wrqin->u.data.length = wrq->u.data.length;
	}

	return Status;
}
