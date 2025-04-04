/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2020-2021  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __QOS_H__
#define __QOS_H__


#ifdef QOS_R1

#define IS_QOSR1_ENABLE(pAd) (pAd->bQoSR1Enable == 1)

#define MT2_PEER_ROBUST_AV_CATE 19
#ifdef QOS_R2
#define ACT_SCS_REQ	0
#define ACT_SCS_RSP	1

/*OUI Type*/
#define QOS_ACT_FRM_OUI_TYPE	0x1A
#define QOS_MGMT_IE_OUI_TYPE	0x22
#define WFA_CAPA_IE_OUI_TYPE	0x23

/*OUI Subtype*/
#define DSCP_POLICY_QRY		0
#define DSCP_POLICY_REQ		1
#define DSCP_POLICY_RSP		2

#endif
#define ACT_MSCS_REQ	4
#define ACT_MSCS_RSP	5
#define MT2_MLME_MSCS_RSP	41
#define MT2_MLME_MSCS_REQ	42

/*status code*/
#define  SUCCESS 0
#define MTK_OUI_LEN                   3
#define  REQUEST_DECLINED 37
#define  INSUFFICIENT_TCLAS_PROCESSING_RESOURCES 57
#define	 TCLAS_PROCESSING_TERMINATED             97
#define SCS_REQ_TYPE_ADD 0
#define SCS_REQ_TYPE_REMOVE 1
#define SCS_REQ_TYPE_CHANGE 2
#define SCS_REQ_TYPE_UNKNOWN 3

#define STREAM_TIMEOUT	(60 * 1000000 / 1024)

#define MAX_FILTER_LEN	32
enum {
	MSCS_CLASSIFIER_TYPE0 = 0,
	MSCS_CLASSIFIER_TYPE1 = 1,
	MSCS_CLASSIFIER_TYPE2 = 2,
	MSCS_CLASSIFIER_TYPE3 = 3,
	MSCS_CLASSIFIER_TYPE4 = 4,
	MSCS_CLASSIFIER_TYPE5 = 5,
	MSCS_CLASSIFIER_TYPE10 = 10,
};
enum {
	DABS_ANNOUNCE = 0,
	DABS_REQ = 1,
	DABS_RSP = 2,
	DABS_CFM = 3,
};
/*dabs key bit map*/
#define GENERAL_KEY     (1 << 0) /*group key bit 1*/
#define DABS_VERSION 1
#define DABS_RETRY_LIMIT 3
#define DABS_WAIT_TIME  3000 /* 2 sec */
#define SUCCESS 0

#define NO_MIC 0
#define MIC_AP 1
#define MIC_STA 2

#define DABS_TIMER_RUNNING  0

enum {
	MIC_OK	= 0,
	MIC_FAIL = 1,
	NO_MATCH_KEY = 2,
};

typedef struct GNU_PACKED _FRAME_MSCSRSP_ACTION {
	HEADER_802_11   Hdr;
	UCHAR   Category;
	UCHAR   RobustAction;
	UCHAR	DialogToken;
	USHORT	Status;
	UCHAR   Data[0];
} FRAME_MSCSRSP_ACTION, *PFRAME_MSCSRSP_ACTION;

typedef struct GNU_PACKED _FRAME_DABS_ACTION {
	HEADER_802_11   Hdr;
	UCHAR   Category;
	UCHAR   Org_id[MTK_OUI_LEN];
	UCHAR 	version;
	UCHAR	status;
	UCHAR   trans_id;
	UCHAR 	DABStype;
	USHORT  randnumber;
	USHORT 	u2MIC;
	USHORT  key_number;
	USHORT 	reserved;
	UINT32  keybitmap[4];
} FRAME_DABS_ACTION, *PFRAME_DABS_ACTION;

typedef struct GNU_PACKED _MSCS_DESC {
	UCHAR	ElementID;
	UCHAR   Length;
	UCHAR   ElementID_Ext;
	UCHAR   ReqType;
	USHORT  UserPrioCtrl;
	UINT	StreamTimeOut;
	UCHAR   Data[0];
} MSCS_DESC, *PMSCS_DESC;

typedef struct GNU_PACKED _TCLAS_ELEM {
	UCHAR	ElementID;
	UCHAR   Length;
	UCHAR   UP_or_ElementID_Ext;
} TCLAS_ELEM, *PTCLAS_ELEM;

/*todo:need add every class type*/
typedef struct GNU_PACKED _FRAME_CLASS_TYPE_4_ipv4 {
	UCHAR classifierType;
	UCHAR classifierMask;
	UCHAR   Version;
	UINT32	srcipv4;
	UINT32	destipv4;
	USHORT  srcPort;
	USHORT	destPort;
	UCHAR	DSCP;
	UCHAR	protocol_or_nextHeader;
	UCHAR	Reserved;
} FRAME_CLASS_TYPE_4_ipv4, *PFRAME_CLASS_TYPE_4_ipv4;

typedef struct GNU_PACKED _TSPEC_ELEM {
	UCHAR	ElmentID;
	UCHAR   Length;
	UCHAR   TSInfo[3];
	USHORT  NrmlMSDUSize;
	USHORT	MaxMSDUSize;
	UINT	MiniServiceInterval;
	UINT	MaxServiceInterval;
	UINT	InactivityInterval;
	UINT	SuspensionInterval;
	UINT	ServiceStartTime;
	UINT	MiniDataRate;
	UINT	MeanDataRate;
	UINT	PeakDataRate;
	UINT	BurstSize;
	UINT	DelayBound;
	UINT	MiniPHYRate;
	USHORT	SurplusBandwidthAllowance;
	USHORT	MediumTime;
	UCHAR	DMGAttributes[0];
} TSPEC_ELEM, *PTSPEC_ELEM;

struct GNU_PACKED wapp_qos_action_frame {
	u8 src[MAC_ADDR_LEN];
	u32 frmid;
	u32 chan;
	u32 frm_len;
	u8 frm[0];
};

struct GNU_PACKED classifier_header {
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
};

struct GNU_PACKED classifier_type3 {
	u8 cs_type;	/*Classifier Type*/
	u8 cs_mask;	/*Classifier Mask*/
	u16	filterOffset;
	u8	filterlen;
	u8	filterValue[MAX_FILTER_LEN];
	u8	filterMask[MAX_FILTER_LEN];
};

struct GNU_PACKED classifier_type10 {
	u8 cs_type;	/*Classifier Type*/
	u8 protocolInstance;
	union {
		u8	protocol;
		u8	nextheader;
	} u;
	u8	filterlen;
	u8	filterValue[MAX_FILTER_LEN];
	u8	filterMask[MAX_FILTER_LEN];
};

struct GNU_PACKED classifier_parameter {
	u8 sta_mac[MAC_ADDR_LEN];
	u8 requet_type;
	u8 up_bitmap;
	u8 up_limit;
	u32 timeout;
	union {
		struct classifier_header header;
		struct classifier_type3 type3;
		struct classifier_type10 type10;
	} cs;
};

struct GNU_PACKED wapp_vend_spec_classifier_para_report {
	UINT32 id;
	UCHAR sta_mac[MAC_ADDR_LEN];
	UCHAR requet_type;
	UCHAR up;
	UCHAR delay_bound;
	UCHAR protocol;
	UINT32 timeout;
	ULONG expired;
	UCHAR version;
	union {
		UINT32 ipv4;
		UCHAR ipv6[16];
	} srcIp;
	union {
		UINT32 ipv4;
		UCHAR ipv6[16];
	} destIp;
	UINT16 srcPort;
	UINT16 destPort;
};

INT Set_QoSR1Enable_Proc(PRTMP_ADAPTER pAd, char *arg);
INT QoS_Config_for_RDUT_Test(struct _RTMP_ADAPTER *pAd, char *arg);
INT Show_QoS_MapIE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT QoS_send_action_frame(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, struct action_frm_data *frm);
UCHAR QoS_parse_mscs_descriptor_ies(struct wifi_dev *wdev, UCHAR *srcAddr, UCHAR *buf, UCHAR buflen);
void QoS_PeerRobust_AV_Action(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem);
void QoS_get_default_mscs_descriptor(UCHAR *tmpbuf, UCHAR *ielen);
#ifdef QOS_R2
INT Set_ScsEnable_Proc(PRTMP_ADAPTER pAd, char *arg);
INT Set_DSCPPolicyEnable_Proc(PRTMP_ADAPTER pAd, char *arg);
void QoS_Build_WFACapaIE(UCHAR *tmpbuf, UCHAR *ielen, UCHAR bCapa);
INT Set_QoSMgmtCapa_Proc(PRTMP_ADAPTER pAd, char *arg);
INT Set_QoSMapCapa_Proc(PRTMP_ADAPTER pAd, char *arg);
#ifdef DSCP_PRI_SUPPORT
void QoS_Init_DSCP2UP_Mapping(PRTMP_ADAPTER pAd);
#endif
void QoS_MapIE_Config(PRTMP_ADAPTER pAd, UINT8 bssidx, char *IE, UINT32 IELen);

#endif

#if defined(MSCS_PROPRIETARY) || defined(QOS_R2)
void Peer_Vendor_Spec_Action(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem);
void Peer_DABS_Cfg_Timeout(struct _RTMP_ADAPTER *pAd, struct _MLME_QUEUE_ELEM *Elem);

#endif

#ifdef MSCS_PROPRIETARY
DECLARE_TIMER_FUNCTION(RTMPDABSretry);
INT Set_DABSkeybitmap_Proc(PRTMP_ADAPTER pAd, char *arg);
INT Set_Dabs_Drop_Thre_Proc(PRTMP_ADAPTER pAd, char *arg);
BOOLEAN QoS_mtk_prop_mscs_ies_parse(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
		UCHAR *srcAddr, UCHAR *buf, ULONG len, UINT8 *reasoncode);
void QoS_send_mscs_rsp(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UINT8 Dialogtoken,
		UINT8 reasoncode, struct wapp_vend_spec_classifier_para_report *classifier_para);
void Send_DABS_Announce
	(struct _RTMP_ADAPTER *pAd, UINT16 wcid);
void Send_DABS_Rsp
	(struct _RTMP_ADAPTER *pAd, UINT16 wcid, UCHAR status, UINT32 *keybitmap);
void wext_send_vendor_spec_tclas_elment(struct wifi_dev *wdev,
	UCHAR *vend_spec_classifier_para_report, ULONG len);

#endif /*MSCS_PROPRIETARY*/
#endif /*QOS_R1*/


#endif /*__QOS_H__*/
