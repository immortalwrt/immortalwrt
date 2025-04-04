/****************************************************************************

    Module Name:
	rt_os_net.h

	Abstract:
	All function prototypes are defined in NETIF modules.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RT_OS_NET_H__
#define __RT_OS_NET_H__

#include "chip/chip_id.h"
#include "hif/hif.h"

typedef VOID * (*RTMP_NET_ETH_CONVERT_DEV_SEARCH)(VOID * net_dev, UCHAR * pData);
typedef int (*RTMP_NET_PACKET_TRANSMIT)(VOID *pPacket);

#ifdef LINUX
#ifdef OS_ABL_FUNC_SUPPORT

/* ========================================================================== */
/* operators used in NETIF module */
/* Note: No need to put any compile option here */
typedef struct _RTMP_DRV_ABL_OPS {

	NDIS_STATUS (*RTMPAllocAdapterBlock)(PVOID handle, VOID **ppAdapter, INT type);
	VOID (*RTMPFreeAdapter)(VOID *pAd);

	BOOLEAN(*RtmpRaDevCtrlExit)(VOID *pAd);
	INT(*RtmpRaDevCtrlInit)(VOID *pAd, RTMP_INF_TYPE infType);
	INT(*RTMP_COM_IoctlHandle)(
		IN	VOID *pAd,
		IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
		IN	INT cmd,
		IN	USHORT subcmd,
		IN	VOID *pData,
		IN	ULONG Data);

	int (*RTMPSendPackets)(
		IN	NDIS_HANDLE MiniportAdapterContext,
		IN	PPNDIS_PACKET ppPacketArray,
		IN	UINT NumberOfPackets,
		IN	UINT32 PktTotalLen,
		IN	RTMP_NET_ETH_CONVERT_DEV_SEARCH Func);

	int (*P2P_PacketSend)(
		IN	PNDIS_PACKET				pPktSrc,
		IN	PNET_DEV					pDev,
		IN	RTMP_NET_PACKET_TRANSMIT	Func);

	INT(*RTMP_AP_IoctlHandle)(
		IN	VOID					*pAd,
		IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
		IN	INT						cmd,
		IN	USHORT					subcmd,
		IN	VOID					*pData,
		IN	ULONG					Data);

	INT(*RTMP_STA_IoctlHandle)(
		IN	VOID					*pAd,
		IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
		IN	INT						cmd,
		IN	USHORT					subcmd,
		IN	VOID					*pData,
		IN	ULONG					Data,
		IN  USHORT                  priv_flags);

	VOID (*RTMPDrvOpen)(VOID *pAd);
	VOID (*RTMPDrvClose)(VOID *pAd, VOID *net_dev);
	int (*mt_wifi_init)(VOID *pAd,  RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName);
} RTMP_DRV_ABL_OPS;

extern RTMP_DRV_ABL_OPS *pRtmpDrvOps;

VOID RtmpDrvOpsInit(
	OUT	VOID				*pDrvOpsOrg,
	INOUT	VOID				*pDrvNetOpsOrg,
	IN		RTMP_PCI_CONFIG * pPciConfig,
	IN		RTMP_USB_CONFIG * pUsbConfig);
#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */




/* ========================================================================== */
/* operators used in DRIVER module */
typedef void (*RTMP_DRV_USB_COMPLETE_HANDLER)(VOID *pURB);

typedef struct _RTMP_NET_ABL_OPS {


} RTMP_NET_ABL_OPS;

extern RTMP_NET_ABL_OPS *pRtmpDrvNetOps;

VOID RtmpNetOpsInit(VOID *pNetOpsOrg);
VOID RtmpNetOpsSet(VOID *pNetOpsOrg);


/* ========================================================================== */
#if defined(RTMP_MODULE_OS) && defined(OS_ABL_FUNC_SUPPORT)
/* for UTIL/NETIF module in OS ABL mode */

#define RTMPAllocAdapterBlock (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPAllocAdapterBlock)
#define RTMPFreeAdapter (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPFreeAdapter)
#define RtmpRaDevCtrlExit (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RtmpRaDevCtrlExit)
#define RtmpRaDevCtrlInit (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RtmpRaDevCtrlInit)
#define RTMPHandleInterrupt (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPHandleInterrupt)
#define RTMP_COM_IoctlHandle (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMP_COM_IoctlHandle)
#define RTMPSendPackets (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPSendPackets)
#define P2P_PacketSend (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->P2P_PacketSend)
#define RTMP_AP_IoctlHandle (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMP_AP_IoctlHandle)
#define RTMP_STA_IoctlHandle (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMP_STA_IoctlHandle)
#define RTMPDrvOpen (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPDrvOpen)
#define RTMPDrvClose (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPDrvClose)
#define mt_wifi_init (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->rt28xx_init)

#else /* RTMP_MODULE_OS && OS_ABL_FUNC_SUPPORT */

NDIS_STATUS RTMPAllocAdapterBlock(PVOID handle, VOID **ppAdapter, INT type);
VOID RTMPFreeAdapter(VOID *pAd);
BOOLEAN RtmpRaDevCtrlExit(VOID *pAd);
INT RtmpRaDevCtrlInit(VOID *pAd, RTMP_INF_TYPE infType);

INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);

int	RTMPSendPackets(
	IN	NDIS_HANDLE		MiniportAdapterContext,
	IN	PPNDIS_PACKET	ppPacketArray,
	IN	UINT			NumberOfPackets,
	IN	UINT32			PktTotalLen,
	IN	RTMP_NET_ETH_CONVERT_DEV_SEARCH	Func);

int P2P_PacketSend(
	IN	PNDIS_PACKET				pPktSrc,
	IN	PNET_DEV					pDev,
	IN	RTMP_NET_PACKET_TRANSMIT	Func);

#ifdef CONFIG_AP_SUPPORT
INT RTMP_AP_IoctlHandle(
	IN	VOID					*pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
INT RTMP_STA_IoctlHandle(
	IN	VOID					*pAd,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data,
	IN  USHORT                  priv_flags);
#endif /* CONFIG_STA_SUPPORT */

VOID RTMPDrvOpen(VOID *pAd);
VOID RTMPDrvClose(VOID *pAd, VOID *net_dev);

int mt_wifi_init(VOID *pAd, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName);
#ifdef CONFIG_WLAN_SERVICE
int mt_service_open(struct _RTMP_ADAPTER *ad);
int mt_service_init(struct _RTMP_ADAPTER *ad);
int mt_service_close(struct _RTMP_ADAPTER *ad);
#endif /* CONFIG_WLAN_SERVICE */

PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAd);
#endif /* RTMP_MODULE_OS */

/* ========================================================================== */
int virtual_if_init_handler(VOID *dev);
INT virtual_if_deinit_handler(VOID *dev);
int virtual_if_up_handler(VOID *dev);
int virtual_if_down_handler(VOID *dev);

static inline INT VIRTUAL_IF_INIT(VOID *pAd, VOID *pDev)
{
	RT_CMD_INF_UP_DOWN InfConf = {
		virtual_if_init_handler,
		virtual_if_deinit_handler,
		virtual_if_up_handler,
		virtual_if_down_handler,
		pDev
	};

	if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_INIT,
							 0, &InfConf, 0) != NDIS_STATUS_SUCCESS)
		return -1;

	return 0;
}

static inline VOID VIRTUAL_IF_DEINIT(VOID *pAd, VOID *pDev)
{
	RT_CMD_INF_UP_DOWN InfConf = {
		virtual_if_init_handler,
		virtual_if_deinit_handler,
		virtual_if_up_handler,
		virtual_if_down_handler,
		pDev
	};

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_DEINIT,
						 0, &InfConf, 0);
}

static inline INT VIRTUAL_IF_UP(VOID *pAd, VOID *pDev)
{
	RT_CMD_INF_UP_DOWN InfConf = {
		virtual_if_init_handler,
		virtual_if_deinit_handler,
		virtual_if_up_handler,
		virtual_if_down_handler,
		pDev
	};

	if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP,
							 0, &InfConf, 0) != NDIS_STATUS_SUCCESS)
		return -1;

	return 0;
}

static inline VOID VIRTUAL_IF_DOWN(VOID *pAd, VOID *pDev)
{
	RT_CMD_INF_UP_DOWN InfConf = {
		virtual_if_init_handler,
		virtual_if_deinit_handler,
		virtual_if_up_handler,
		virtual_if_down_handler,
		pDev
	};
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN,
						 0, &InfConf, 0);
	return;
}
#ifdef CONFIG_AP_SUPPORT
INT rt28xx_ap_ioctl(
	IN	VOID *net_dev_obj,
	IN	OUT	VOID *rq_obj,
	IN	INT			cmd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
INT rt28xx_sta_ioctl(
	IN	VOID	*net_dev_obj,
	IN	OUT	VOID *rq_obj,
	IN	INT			cmd);
#endif /* CONFIG_STA_SUPPORT */

#ifdef RTMP_MODULE_OS
PNET_DEV RtmpPhyNetDevInit(
	IN VOID						*pAd,
	IN RTMP_OS_NETDEV_OP_HOOK * pNetHook);

BOOLEAN RtmpPhyNetDevExit(
	IN VOID						*pAd,
	IN PNET_DEV					net_dev);

#endif /* RTMP_MODULE_OS && OS_ABL_FUNC_SUPPORT */

INT main_virtual_if_open(PNET_DEV pDev);
INT main_virtual_if_close(PNET_DEV pDev);

VOID RT28xx_MSTA_Init(VOID *pAd, PNET_DEV main_dev_p);
INT msta_virtual_if_open(PNET_DEV pDev);
INT msta_virtual_if_close(PNET_DEV pDev);
VOID RT28xx_MSTA_Remove(VOID *pAd);

VOID RT28xx_MBSS_Init(VOID *pAd, PNET_DEV main_dev_p);
INT mbss_virtual_if_open(PNET_DEV pDev);
INT mbss_virtual_if_close(PNET_DEV pDev);
VOID RT28xx_MBSS_Remove(VOID *pAd);


VOID RT28xx_WDS_Init(VOID *pAd, UCHAR band_idx, PNET_DEV net_dev);
INT wds_virtual_if_open(PNET_DEV pDev);
INT wds_virtual_if_close(PNET_DEV pDev);
VOID RT28xx_WDS_Remove(VOID *pAd);


VOID RT28xx_ApCli_Init(VOID *pAd, PNET_DEV main_dev_p);
INT apcli_virtual_if_open(PNET_DEV pDev);
INT apcli_virtual_if_close(PNET_DEV pDev);
VOID RT28xx_ApCli_Remove(VOID *pAd);

VOID RT28xx_Monitor_Init(VOID *pAd, PNET_DEV main_dev_p);
VOID RT28xx_Monitor_Remove(VOID *pAd);
VOID RTMP_Mesh_Init(VOID *pAd, PNET_DEV main_dev_p, RTMP_STRING *pHostName);
INT mesh_virtual_if_open(PNET_DEV pDev);
INT mesh_virtual_if_close(PNET_DEV pDev);
VOID RTMP_Mesh_Remove(VOID *pAd);

VOID RTMP_P2P_Init(VOID *pAd, PNET_DEV main_dev_p);
INT p2p_virtual_if_open(PNET_DEV pDev);
INT p2p_virtual_if_close(PNET_DEV pDev);
INT P2P_VirtualIF_PacketSend(
	IN PNDIS_PACKET	 skb_p,
	IN PNET_DEV		 dev_p);
VOID RTMP_P2P_Remove(VOID *pAd);

#if defined(IWCOMMAND_CFG80211_SUPPORT) &&  !defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType);
PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByName(VOID *pAdSrc, CHAR ucIfName[]);
#endif /* IWCOMMAND_CFG80211_SUPPORT && !RT_CFG80211_P2P_CONCURRENT_DEVICE */
INT32 rtmp_get_macPower(IN VOID *pAdSrc);

#ifdef RT_CFG80211_P2P_SUPPORT
#define CFG_P2PGO_ON(__pAd)  RTMP_CFG80211_VIF_P2P_GO_ON(__pAd)
#define CFG_P2PCLI_ON(__pAd) RTMP_CFG80211_VIF_P2P_CLI_ON(__pAd)

BOOLEAN RTMP_CFG80211_VIF_P2P_GO_ON(
	IN      VOID     *pAdSrc);

BOOLEAN RTMP_CFG80211_VIF_P2P_CLI_ON(
	IN      VOID     *pAdSrc);

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID RTMP_CFG80211_DummyP2pIf_Init(
	IN VOID		*pAdSrc);

VOID RTMP_CFG80211_DummyP2pIf_Remove(
	IN VOID		*pAdSrc);

BOOLEAN RTMP_CFG80211_VIF_ON(
	IN      VOID     *pAdSrc);


VOID RTMP_CFG80211_VirtualIF_CancelP2pClient(
	IN VOID                 *pAdSrc);

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(VOID *pAdSrc, UINT32 devType);

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_P2P_SUPPORT */

PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(
	IN      VOID     *pAdSrc,
	IN      UINT32    devType);

VOID RTMP_CFG80211_AddVifEntry(
	IN      VOID     *pAdSrc,
	IN      PNET_DEV pNewNetDev,
	IN      UINT32   DevType);

VOID RTMP_CFG80211_RemoveVifEntry(
	IN      VOID     *pAdSrc,
	IN      PNET_DEV pNewNetDev);

PNET_DEV RTMP_CFG80211_VirtualIF_Get(
	IN		VOID     *pAdSrc);

VOID RTMP_CFG80211_VirtualIF_Init(
	IN VOID         *pAd,
	IN CHAR * pIfName,
	IN UINT32        DevType,
	IN UINT32	flags);

VOID RTMP_CFG80211_VirtualIF_Remove(
	IN VOID         *pAd,
	IN	PNET_DEV	dev_p,
	IN  UINT32      DevType);

VOID RTMP_CFG80211_AllVirtualIF_Remove(
	IN VOID		*pAdSrc);

#ifdef CFG80211_MULTI_STA
BOOLEAN RTMP_CFG80211_MULTI_STA_ON(VOID *pAdSrc, PNET_DEV pNewNetDev);
VOID RTMP_CFG80211_MutliStaIf_Init(VOID *pAd);
VOID RTMP_CFG80211_MutliStaIf_Remove(VOID *pAd);
#endif /* CFG80211_MULTI_STA */

#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT
INT rt_android_private_command_entry(
	VOID *pAdSrc, struct net_device *net_dev, struct ifreq *ifr, int cmd);
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */


/* FOR communication with RALINK DRIVER module in NET module */
/* general */
#define RTMP_DRIVER_NET_DEV_GET(__pAd, __pNetDev)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_NETDEV_GET, 0, __pNetDev, 0)

#ifdef APCLI_CFG80211_SUPPORT
#define RTMP_DRIVER_APCLI_NET_DEV_GET(__pAd, __pNetDev)							\
		RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_APCLI_NETDEV_GET, 0, __pNetDev, 0)
#endif /* APCLI_CFG80211_SUPPORT */

#define RTMP_DRIVER_NET_DEV_SET(__pAd, __pNetDev)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_NETDEV_SET, 0, __pNetDev, 0)

#define RTMP_DRIVER_OP_MODE_GET(__pAd, __pOpMode)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_OPMODE_GET, 0, __pOpMode, 0)

#define RTMP_DRIVER_IW_STATS_GET(__pAd, __pIwStats)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET, 0, __pIwStats, 0)

#define RTMP_DRIVER_INF_STATS_GET(__pAd, __pInfStats)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_STATS_GET, 0, __pInfStats, 0)

#define RTMP_DRIVER_INF_TYPE_GET(__pAd, __pInfType)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_TYPE_GET, 0, __pInfType, 0)

#define RTMP_DRIVER_TASK_LIST_GET(__pAd, __pList)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_TASK_LIST_GET, 0, __pList, 0)

#define RTMP_DRIVER_NIC_NOT_EXIST_SET(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_NIC_NOT_EXIST, 0, NULL, 0)

#define RTMP_DRIVER_MCU_SLEEP_CLEAR(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR, 0, NULL, 0)

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

#define RTMP_DRIVER_USB_DEV_GET(__pAd, __pUsbDev)                                                       \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_DEV_GET, 0, __pUsbDev, 0)

#define RTMP_DRIVER_USB_INTF_GET(__pAd, __pUsbIntf)                                                     \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_INTF_GET, 0, __pUsbIntf, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_SET(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_END_DISSASSOCIATE(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_TEST(__pAd, __flag)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST, 0,  __flag, 0)

#define RTMP_DRIVER_ADAPTER_IDLE_RADIO_OFF_TEST(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST, 0,  __flag, 0)
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
#define RTMP_DRIVER_ADAPTER_RT28XX_WOW_STATUS(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS, 0, __flag, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_WOW_ENABLE(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_WOW_DISABLE(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE, 0, NULL, 0)
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

#endif /* CONFIG_PM */

#define RTMP_DRIVER_AP_SSID_GET(__pAd, pData)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_AP_BSSID_GET, 0, pData, 0)
#endif /* CONFIG_STA_SUPPORT */

#define RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_OFF(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_ON(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_SET(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR, 0, NULL, 0)

#define RTMP_DRIVER_VIRTUAL_INF_NUM_GET(__pAd, __pIfNum)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET, 0, __pIfNum, 0)

#define RTMP_DRIVER_CHANNEL_GET(__pAd, __pInfId, __Channel)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SIOCGIWFREQ, 0, __Channel, __pInfId)

#define RTMP_DRIVER_IOCTL_SANITY_CHECK(__pAd, __SetCmd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SANITY_CHECK, 0, __SetCmd, 0)

#define RTMP_DRIVER_BITRATE_GET(__pAd, __pBitRate)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SIOCGIWRATE, 0, __pBitRate, 0)

#define RTMP_DRIVER_MAIN_INF_CREATE(__pAd, __ppNetDev)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_MAIN_CREATE, 0, __ppNetDev, 0)

#define RTMP_DRIVER_MAIN_INF_GET(__pAd, __pInfId)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET, 0, __pInfId, 0)

#define RTMP_DRIVER_MAIN_INF_CHECK(__pAd, __InfId)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_MAIN_CHECK, 0, NULL, __InfId)

#define RTMP_DRIVER_P2P_INF_CHECK(__pAd, __InfId)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_P2P_CHECK, 0, NULL, __InfId)

#ifdef EXT_BUILD_CHANNEL_LIST
#define RTMP_DRIVER_SET_PRECONFIG_VALUE(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_SET_PRECONFIG_VALUE, 0, NULL, 0)
#endif /* EXT_BUILD_CHANNEL_LIST */


#ifdef RT_CFG80211_SUPPORT
/* General Part */
#define RTMP_DRIVER_CFG80211_REGISTER(__pNetDev) \
	{ \
		VOID *__pAd = NULL; \
		GET_PAD_FROM_NET_DEV(__pAd, __pNetDev); \
		RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_REGISTER, 0, __pNetDev, 0); \
	}

#define RTMP_DRIVER_CFG80211_START(__pAd)									\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_CFG80211_CFG_START, 0, NULL, 0)

#define RTMP_DRIVER_80211_UNREGISTER(__pAd, __pNetDev)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_UNREGISTER, 0, __pNetDev, 0)

#define RTMP_DRIVER_80211_CB_GET(__pAd, __ppCB)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CB_GET, 0, __ppCB, 0)

#define RTMP_DRIVER_80211_CB_SET(__pAd, __pCB)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CB_SET, 0, __pCB, 0)

#define RTMP_DRIVER_80211_RESET(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_RESET,  0, NULL, 0)

/* STA Part */
#define RTMP_DRIVER_80211_SCAN_CHANNEL_LIST_SET(__pAd, __pData, __Len) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CHANNEL_LIST_SET, 0, __pData, __Len)

#ifdef APCLI_CFG80211_SUPPORT
#define RTMP_DRIVER_80211_APCLI_SCAN(__pAd, __pData) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_APCLI_SITE_SURVEY, 0, __pData, 0)
#endif /* APCLI_CFG80211_SUPPORT */

#define RTMP_DRIVER_80211_SCAN(__pAd, __IfType)									\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_SCAN, 0, NULL, __IfType)

#define RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(__pAd, __isInit)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_SCAN_STATUS_LOCK_INIT, 0, NULL, __isInit)

#define RTMP_DRIVER_80211_SCAN_EXTRA_IE_SET(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_EXTRA_IES_SET,  0, NULL, 0)

#define RTMP_DRIVER_80211_GEN_IE_SET(__pAd, __pData, __Len)    \
	RTMP_STA_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWGENIE, 0, __pData, __Len, 0)

#define RTMP_DRIVER_80211_STA_KEY_ADD(__pAd, __pKeyInfo)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_STA_KEY_ADD, 0, __pKeyInfo, 0)

#define RTMP_DRIVER_80211_STA_KEY_DEFAULT_SET(__pAd, __pNdev, __KeyId)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_STA_KEY_DEFAULT_SET, 0, __pNdev, __KeyId)

#define RTMP_DRIVER_80211_POWER_MGMT_SET(__pAd, __enable)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_POWER_MGMT_SET, 0, NULL, __enable)

#ifdef WIFI_IAP_POWER_SAVE_FEATURE
#define RTMP_DRIVER_80211_AP_POWER_MGMT_SET(__pAd, __infwdev, __enable)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_POWER_MGMT_SET, 0, __infwdev, __enable)
#endif/*WIFI_IAP_POWER_SAVE_FEATURE*/

#define RTMP_DRIVER_80211_STA_LEAVE(__pAd, __pNdev)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_STA_LEAVE, 0, __pNdev, 0)

#define RTMP_DRIVER_80211_STA_GET(__pAd, __pStaInfo)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_STA_GET, 0, __pStaInfo, 0)
#ifdef WIFI_IAP_STA_DUMP_FEATURE
#define RTMP_DRIVER_80211AP_STA_GET(__pAd, __pStaInfo)					\
		RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_STA_GET, 0, __pStaInfo, 0)
#endif/*WIFI_IAP_STA_DUMP_FEATURE*/

#define RTMP_DRIVER_80211_CONNECT(__pAd, __pConnInfo, __devType)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CONNECT_TO, 0, __pConnInfo, __devType)

#ifdef SUPP_SAE_SUPPORT
#define RTMP_DRIVER_80211_CONNECT_PARAM(__pAd, __pConnParam, __staIndex)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_EXT_CONNECT, 0, __pConnParam, __staIndex)
#endif

#define RTMP_DRIVER_80211_IBSS_JOIN(__pAd, __pInfo)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_IBSS_JOIN, 0, __pInfo, 0)

#define RTMP_DRIVER_80211_PMKID_CTRL(__pAd, __pPmkidInfo)				\
	RTMP_STA_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWPMKSA, 0, __pPmkidInfo, 0, 0)
#ifdef CFG_TDLS_SUPPORT
/* new TDLS */
#define RTMP_DRIVER_80211_STA_TDLS_INSERT_DELETE_PENTRY(__pAd, __peerAddr, __entryOP)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_STA_TDLS_INSERT_PENTRY, 0, __peerAddr, __entryOP)

#define RTMP_DRIVER_80211_STA_TDLS_SET_KEY_COPY_FLAG(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_STA_TDLS_SET_KEY_COPY_FLAG,  0, NULL, 0)
#endif


/* Information Part */
#define RTMP_DRIVER_80211_BANDINFO_GET(__pAd, __pBandInfo)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_BANDINFO_GET, 0, __pBandInfo, 0)

#define RTMP_DRIVER_80211_CHANGE_BSS_PARM(__pAd, __pBssInfo) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CHANGE_BSS_PARM, 0, __pBssInfo, 0)

#define RTMP_DRIVER_80211_CHAN_SET(__pAd, __pChan)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CHAN_SET, 0, __pChan, 0)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
#ifdef WIFI_IAP_IW_SET_CHANNEL_FEATURE
#define RTMP_DRIVER_AP_80211_CHAN_SET(__pAd, __pChan)						\
		RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_CHAN_SET, 0, __pChan, 0)
#endif/*WIFI_IAP_IW_SET_CHANNEL_FEATURE*/
#endif/*KERNEL_VERSION(4, 0, 0)*/

#define RTMP_DRIVER_80211_RFKILL(__pAd, __pActive)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_RFKILL, 0, __pActive, 0)

#define RTMP_DRIVER_80211_REG_NOTIFY(__pAd, __pNotify)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_REG_NOTIFY_TO, 0, __pNotify, 0)

#define RTMP_DRIVER_80211_SURVEY_GET(__pAd, __pSurveyInfo)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_SURVEY_GET, 0, __pSurveyInfo, 0)

#define RTMP_DRIVER_80211_NETDEV_EVENT(__pAd, __pDev, __state)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_NETDEV_EVENT, 0, __pDev, __state)

/* AP Part */

#define RTMP_DRIVER_80211_BEACON_DEL(__pAd, __apidx) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_BEACON_DEL, 0, NULL, __apidx)


#define RTMP_DRIVER_80211_BEACON_ADD(__pAd, __pBeacon) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_BEACON_ADD, 0, __pBeacon, 0)

#ifdef HOSTAPD_HS_R2_SUPPORT
#define RTMP_DRIVER_80211_QOS_PARAM_SET(__pAd, __pQosMap, __apidx) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SET_QOS_PARAM, 0, __pQosMap, __apidx)
#endif

#define RTMP_DRIVER_80211_BEACON_SET(__pAd, __pBeacon) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_BEACON_SET, 0, __pBeacon, 0)

#define RTMP_DRIVER_80211_BITRATE_SET(__pAd, __pMask, __apidx) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_BITRATE_SET, 0, __pMask, __apidx)

#define RTMP_DRIVER_80211_AP_KEY_DEL(__pAd, __pKeyInfo) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_KEY_DEL, 0, __pKeyInfo, 0)

#define RTMP_DRIVER_80211_AP_KEY_ADD(__pAd, __pKeyInfo) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_KEY_ADD, 0, __pKeyInfo, 0)

#define RTMP_DRIVER_80211_RTS_THRESHOLD_ADD(__pAd, __Rts_thresold) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_RTS_THRESHOLD_ADD, 0, NULL, __Rts_thresold)

#define RTMP_DRIVER_80211_FRAG_THRESHOLD_ADD(__pAd, __Frag_thresold) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_FRAG_THRESHOLD_ADD, 0, NULL, __Frag_thresold)
#ifdef ACK_CTS_TIMEOUT_SUPPORT
#define RTMP_DRIVER_80211_ACK_THRESHOLD_ADD(__pAd, __Frag_thresold) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_ACK_THRESHOLD_ADD, 0, NULL, __Frag_thresold)
#endif/*ACK_CTS_TIMEOUT_SUPPORT*/

#define RTMP_DRIVER_80211_AP_KEY_DEFAULT_SET(__pAd, __pNdev, __KeyId)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_SET, 0, __pNdev, __KeyId)

#ifdef DOT11W_PMF_SUPPORT
#define RTMP_DRIVER_80211_AP_KEY_DEFAULT_MGMT_SET(__pAd, __pNdev, __KeyId)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_MGMT_SET, 0, __pNdev, __KeyId)
#endif /* DOT11W_PMF_SUPPORT */

#define RTMP_DRIVER_80211_AP_PROBE_RSP(__pAd, __pFrame, __Len) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_PROBE_RSP_EXTRA_IE, 0, __pFrame, __Len)

#define RTMP_DRIVER_80211_AP_ASSOC_RSP(__pAd, __pFrame, __Len) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_ASSOC_RSP_EXTRA_IE, 0, __pFrame, __Len)

#define RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(__pAd, __pMac, __Reg) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_PORT_SECURED,  0, __pMac, __Reg)

#ifdef HOSTAPD_MAP_SUPPORT /* This could be a generic fix */
#define RTMP_DRIVER_80211_AP_STA_DEL(__pAd, __pData, __Reason) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_STA_DEL, 0, __pData, __Reason)
#else
#define RTMP_DRIVER_80211_AP_STA_DEL(__pAd, __pMac, __Reason)  \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_STA_DEL, 0, __pMac, __Reason)
#endif /* HOSTAPD_MAP_SUPPORT */

#ifdef HOSTAPD_PMKID_IN_DRIVER_SUPPORT
#define RTMP_DRIVER_80211_AP_UPDATE_STA_PMKID(__pAd, __pData) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_AP_UPDATE_STA_PMKID, 0, __pData, 0)
#endif /*HOSTAPD_PMKID_IN_DRIVER_SUPPORT*/

/* ap */
#define RTMP_DRIVER_AP_BITRATE_GET(__pAd, __pConfig)							\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ, 0, __pConfig, 0)

#define RTMP_DRIVER_AP_MAIN_OPEN(__pAd)										\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MAIN_OPEN, 0, NULL, 0)


/* P2P Part */
#define RTMP_DRIVER_80211_ACTION_FRAME_REG(__pAd, __devPtr, __Reg) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_ACTION_FRAME_REG, 0, __devPtr, __Reg)

#define RTMP_DRIVER_80211_REMAIN_ON_CHAN_DUR_IMER_INIT(__pAd)                       \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_REMAIN_ON_CHAN_DUR_TIMER_INIT, 0, NULL, 0)

#define RTMP_DRIVER_80211_REMAIN_ON_CHAN_SET(__pAd, __pChan, __Duration)  \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_REMAIN_ON_CHAN_SET, 0, __pChan, __Duration)

#define RTMP_DRIVER_80211_CANCEL_REMAIN_ON_CHAN_SET(__pAd, __cookie) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CANCEL_REMAIN_ON_CHAN_SET, 0, NULL, __cookie)

#define RTMP_DRIVER_80211_CHANNEL_LOCK(__pAd, __Chan)                   \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CHANNEL_LOCK, 0, NULL, __Chan)

#define RTMP_DRIVER_80211_MGMT_FRAME_REG(__pAd, __devPtr, __Reg) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_REG, 0, __devPtr, __Reg)

#define RTMP_DRIVER_80211_MGMT_FRAME_SEND(__pAd, __pFrame, __Len)                       \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_SEND, 0, __pFrame, __Len)

#define RTMP_DRIVER_80211_P2P_CHANNEL_RESTORE(__pAd)				\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_CHANNEL_RESTORE,  0, NULL, 0)

#define RTMP_DRIVER_80211_STA_ASSSOC_IE_SET(__pAd, __pFrame, __Len)                       \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_P2PCLI_ASSSOC_IE_SET, 0, __pFrame, __Len)

#define RTMP_DRIVER_80211_P2P_CLIENT_KEY_ADD(__pAd, __pKeyInfo)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_P2P_CLIENT_KEY_ADD, 0, __pKeyInfo, 0)

/* VIF Part */
#define RTMP_DRIVER_80211_VIF_ADD(__pAd, __pInfo) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_VIF_ADD, 0, __pInfo, 0)

#define RTMP_DRIVER_80211_VIF_DEL(__pAd, __devPtr, __type) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_VIF_DEL, 0, __devPtr, __type)

#define RTMP_DRIVER_80211_VIF_CHG(__pAd, __pVifInfo)			\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_VIF_CHG, 0, __pVifInfo, 0)

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
#define RTMP_DRIVER_ADAPTER_MCC_DHCP_PROTECT_STATUS(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MCC_DHCP_PROTECT_STATUS, 0, __flag, 0)


#define RTMP_DRIVER_80211_SET_NOA(__pAd, __Devname)									\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_SET_NOA, 0, NULL, __Devname)
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */



#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
#define RTMP_DRIVER_80211_SEND_WIRELESS_EVENT(__pAd, __pMacAddr)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_SEND_WIRELESS_EVENT, 0, __pMacAddr, 0)
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#endif /* RT_CFG80211_SUPPORT */

/* mesh */
#define RTMP_DRIVER_MESH_REMOVE(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MESH_REMOVE, 0, NULL, 0)

/* inf ppa */
#define RTMP_DRIVER_INF_PPA_INIT(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_PPA_INIT, 0, NULL, 0)

#define RTMP_DRIVER_INF_PPA_EXIT(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_PPA_EXIT, 0, NULL, 0)

/* pci */
#define RTMP_DRIVER_PCI_SUSPEND(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCI_SUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_PCI_RESUME(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCI_RESUME, 0, NULL, 0)

#define RTMP_DRIVER_RBUS_SUSPEND(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_RBUS_SUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_RBUS_RESUME(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_RBUS_RESUME, 0, NULL, 0)

#define RTMP_DRIVER_PCI_CSR_SET(__pAd, __Address)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCI_CSR_SET, 0, NULL, __Address)

#define RTMP_DRIVER_PCIE_INIT(__pAd, __pConfig, __pPciDev)								\
	{																			\
		RT_CMD_PCIE_INIT *pConfig = __pConfig;						\
		pConfig->pPciDev = __pPciDev;											\
		pConfig->ConfigDeviceID = PCI_DEVICE_ID;								\
		pConfig->ConfigSubsystemVendorID = PCI_SUBSYSTEM_VENDOR_ID;			\
		pConfig->ConfigSubsystemID = PCI_SUBSYSTEM_ID;						\
		pConfig->pci_init_succeed = FALSE;						\
		RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCIE_INIT, 0, __pConfig, 0);\
	}

/* usb */
#define RTMP_DRIVER_USB_SUSPEND(__pAd, __bIsRunning)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_SUSPEND, 0, NULL, __bIsRunning)

#define RTMP_DRIVER_USB_RESUME(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_RESUME, 0, NULL, 0)

#define RTMP_DRIVER_USB_INIT(__pAd, __pUsbDev, __driver_info)	\
	do {	\
		RT_CMD_USB_INIT __Config, *__pConfig = &__Config;	\
		__pConfig->pUsbDev = __pUsbDev;	\
		__pConfig->driver_info = __driver_info;	\
		RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_INIT, 0, __pConfig, 0);	\
	} while (0)

#define RTMP_DRIVER_SDIO_INIT(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SDIO_INIT, 0, NULL, 0)


#define RTMP_DRIVER_CHIP_PREPARE(__pAd) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_CHIP_PREPARE, 0, NULL, 0)



/* ap */
#define RTMP_DRIVER_AP_BITRATE_GET(__pAd, __pConfig)							\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ, 0, __pConfig, 0)

/* sta */


#ifdef PROFILE_PATH_DYNAMIC
#define RTMP_DRIVER_PROFILEPATH_SET(__pAd, __Type)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PROFILEPATH_SET, 0, NULL, __Type, __Type)
#endif /* PROFILE_PATH_DYNAMIC */

#define RTMP_DRIVER_MAC_ADDR_GET(__pAd, __pMacAddr)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MAC_ADDR_GET, 0, __pMacAddr, 0)

#define RTMP_DRIVER_ADAPTER_CSO_SUPPORT_TEST(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_CSO_SUPPORT_TEST, 0,  __flag, 0)

#define RTMP_DRIVER_ADAPTER_TSO_SUPPORT_TEST(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_TSO_SUPPORT_TEST, 0,  __flag, 0)

#ifdef CONFIG_HAS_EARLYSUSPEND
#define RTMP_DRIVER_SET_SUSPEND_FLAG(__pAd) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SET_SUSPEND_FLAG, 0, NULL, 0)

#define RTMP_DRIVER_LOAD_FIRMWARE_CHECK(__pAd)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_LOAD_FIRMWARE_CHECK, 0, NULL, 0)

#define RTMP_DRIVER_OS_COOKIE_GET(__pAd, __os_cookie)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_OS_COOKIE_GET, 0, __os_cookie, 0)

#define RTMP_DRIVER_ADAPTER_REGISTER_EARLYSUSPEND(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_REGISTER_EARLYSUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_UNREGISTER_EARLYSUSPEND(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_UNREGISTER_EARLYSUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_CHECK_EARLYSUSPEND(__pAd, __flag)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_CHECK_EARLYSUSPEND, 0, __flag, 0)
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef ANTENNA_CONTROL_SUPPORT
#define RTMP_DRIVER_80211_SET_ANTENNA(__pAd, __pAntennaCfg) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_ANTENNA_CTRL, 0, __pAntennaCfg, 0)

#define RTMP_DRIVER_80211_GET_ANTENNA(__pAd, __pAntennaCfg) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_80211_ANTENNA_CTRL, 0, __pAntennaCfg, 1)
#endif /* ANTENNA_CONTROL_SUPPORT */

#endif /* __RT_OS_NET_H__ */

