#ifndef __WIFI_MD_COEX_H__
#define __WIFI_MD_COEX_H__

/* For wifi and md coex in colgin project*/
#ifdef WIFI_MD_COEX_SUPPORT

#define APCCCI_DRIVER_FW 0x0100
#define FW_DRIVER_APCCCI 0x0200
#define REGISTER_WIFI_MD_DTB 0x0300

#define DTB_IDX_2G 0x10
#define DTB_IDX_5G 0x20
#define DTB_IDX_DBDC 0x30

/* Export API function */
int send_idx_to_wifi_md_coex(struct _RTMP_ADAPTER *pAd);
int register_wifi_md_coex(struct _RTMP_ADAPTER *pAd);
int unregister_wifi_md_coex(struct _RTMP_ADAPTER *pAd);

extern int register_wifi_md_coex_notifier(struct notifier_block *nb);
extern int unregister_wifi_md_coex_notifier(struct notifier_block *nb);
extern int call_wifi_md_coex_notifier(unsigned long event, void *msg);

#endif /* WIFI_MD_COEX_SUPPORT */
#endif /* __WIFI_MD_COEX_H__ */

