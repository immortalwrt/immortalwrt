/**
*kvr_def.h --- 80211 kvrh related macro and struct
*for new feature implement
*/

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#ifndef RT_PRIV_IOCTL
#define	RT_PRIV_IOCTL			0x8BE1
#endif
#ifndef OID_GET_SET_TOGGLE
#define	OID_GET_SET_TOGGLE		0x8000
#endif
#ifndef OID_GET_SET_FROM_UI
#define	OID_GET_SET_FROM_UI		0x4000
#endif

#ifndef BIT
#define BIT(x) (1<<(x))
#endif

#define SUPPORT_11K_CAP BIT(0)
#define SUPPORT_11V_CAP BIT(1)
#define SUPPORT_11R_CAP BIT(2)

#define SUC (0)              /*ioctl command successfully */
#ifndef EINVAL
#define	EINVAL		(-30000) /* invalid ioctl parameter */
#endif
#ifndef EFAULT
#define	EFAULT		(-30002) /* driver internal error when processing ioctl command */
#endif
#ifndef ENOMEM
#define	ENOMEM		(-30005) /* no memory when allocate memory in Wi-Fi driver */
#endif
#ifndef ENOTCONN
#define	ENOTCONN	(-30007) /* the station is not associated */
#endif

#define OID_KVR_BASE		0x1100

#define OID_80211H_CAPABILITY_QUERY		0x1130
#define OID_KVR_CAPABILITY_QUERY		0x0002

#define RT_QUERY_11H_CAPABILITY	\
	(OID_GET_SET_FROM_UI|OID_80211H_CAPABILITY_QUERY) /*0x5130*/
#define RT_QUERY_KVR_CAPABILITY	\
	(OID_GET_SET_FROM_UI|OID_KVR_BASE|OID_KVR_CAPABILITY_QUERY) /*0x5102*/

/*
 * struct _KVR_CAPABILITY - command data structure for checking capability of 80211k,v,r of connecting sta
 * @StaMac: mac address of peer station connecting to the current ap
 * @KVRCap: bitmask of the KVR capability of STA.
 *			note that SUPPORT_11K_CAP(0x01)/ SUPPORT_11V_CAP(0x02)/ SUPPORT_11R_CAP(0x04)is allowed
 */
typedef struct  _KVR_CAPABILITY {
	unsigned char  StaMac[6];
	unsigned char  KVRCap;
} KVR_CAPABILITY, *PKVR_CAPABILITY;

/**
 * struct  _IEEE_80211H_CAPABILITY - command data structure for checking capability of 80211h of connecting sta
 * @StaMac: mac address of peer station connecting to the current ap
 * @IsSupport80211h: set to 1 if sta support 80211h(dfs&tpc), otherwise 0
 */
typedef struct  _IEEE_80211H_CAPABILITY {
	unsigned char StaMac[6];
	unsigned char IsSupport80211h;
} IEEE_80211H_CAPABILITY, *PIEEE_80211H_CAPABILITY;

#endif /* _INTERFACE_H_ */

