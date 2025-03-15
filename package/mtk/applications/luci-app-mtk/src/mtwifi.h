#ifndef __MTWIFI_H
#define __MTWIFI_H

#define USHORT  unsigned short
#define UCHAR   unsigned char
#define ULONG	unsigned long
#define UINT8	unsigned char
#define UINT16	unsigned short
#define UINT32	unsigned int
#define INT32	int
#define INT 	int

#include <lua.h>							   /* Always include this */
#include <lauxlib.h>						   /* Always include this */
#include <lualib.h>							/* Always include this */

typedef union _HTTRANSMIT_SETTING_FIX {
	struct {
		USHORT MCS:6;
		USHORT ldpc:1;
		USHORT BW:2;
		USHORT ShortGI:2;
		USHORT STBC:1;
		USHORT eTxBF:1;
		USHORT iTxBF:1;
		USHORT MODE:4;
	} field;
	UINT32 word;
} HTTRANSMIT_SETTING_FIX, *PHTTRANSMIT_SETTING_FIX;

typedef struct _RT_802_11_MAC_ENTRY_FIX {
	unsigned char           ApIdx;
	unsigned char           Addr[6];
	unsigned short          Aid;
	unsigned char           Psm;     // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char           MimoPs;  // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	signed char             AvgRssi0;
	signed char             AvgRssi1;
	signed char             AvgRssi2;
	signed char             AvgRssi3;
	unsigned int            ConnectedTime;
	HTTRANSMIT_SETTING_FIX      TxRate;
	HTTRANSMIT_SETTING_FIX      LastRxRate;
	short                   StreamSnr[3];
	short                   SoundingRespSnr[3];
	UINT32					EncryMode;
	UINT32					AuthMode;
} RT_802_11_MAC_ENTRY_FIX;

#define MAX_NUMBER_OF_MAC               544

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long            Num;
	RT_802_11_MAC_ENTRY_FIX      Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE_FIX;

#define IF_NAMESIZE			16
#define SIOCIWFIRSTPRIV			0x8BE0
#define RT_PRIV_IOCTL				(SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT	(SIOCIWFIRSTPRIV + 0x1F)
#define RTPRIV_IOCTL_GSITESURVEY		(SIOCIWFIRSTPRIV + 0x0D)
#define OID_GET_WMODE			0x099E
#define OID_GET_CPU_TEMPERATURE		0x09A1

#define MODE_CCK 0
#define MODE_OFDM 1
#define MODE_HTMIX 2
#define MODE_HTGREENFIELD 3
#define MODE_VHT 4
#define MODE_HE 5
#define MODE_HE_5G 6
#define MODE_HE_24G 7
#define MODE_HE_SU	8
#define MODE_HE_EXT_SU	9
#define MODE_HE_TRIG	10
#define MODE_HE_MU	11

#define TMI_TX_RATE_OFDM_6M     11
#define TMI_TX_RATE_OFDM_9M     15
#define TMI_TX_RATE_OFDM_12M    10
#define TMI_TX_RATE_OFDM_18M    14
#define TMI_TX_RATE_OFDM_24M    9
#define TMI_TX_RATE_OFDM_36M    13
#define TMI_TX_RATE_OFDM_48M    8
#define TMI_TX_RATE_OFDM_54M    12

#define TMI_TX_RATE_CCK_1M_LP   0
#define TMI_TX_RATE_CCK_2M_LP   1
#define TMI_TX_RATE_CCK_5M_LP   2
#define TMI_TX_RATE_CCK_11M_LP  3

#define TMI_TX_RATE_CCK_2M_SP   5
#define TMI_TX_RATE_CCK_5M_SP   6
#define TMI_TX_RATE_CCK_11M_SP  7

enum oid_bw {
	BAND_WIDTH_20,
	BAND_WIDTH_40,
	BAND_WIDTH_80,
	BAND_WIDTH_160,
	BAND_WIDTH_10,
	BAND_WIDTH_5,
	BAND_WIDTH_8080,
	BAND_WIDTH_BOTH,
	BAND_WIDTH_25,
	BAND_WIDTH_20_242TONE,
	BAND_WIDTH_NUM
};

#define BW_20		BAND_WIDTH_20
#define BW_40		BAND_WIDTH_40
#define BW_80		BAND_WIDTH_80
#define BW_160		BAND_WIDTH_160
#define BW_10		BAND_WIDTH_10
#define BW_5		BAND_WIDTH_5
#define BW_8080		BAND_WIDTH_8080
#define BW_25		BAND_WIDTH_25
#define BW_20_242TONE	BAND_WIDTH_20_242TONE
#define BW_NUM		BAND_WIDTH_NUM

typedef enum _SEC_CIPHER_MODE {
	SEC_CIPHER_NONE,
	SEC_CIPHER_WEP40,
	SEC_CIPHER_WEP104,
	SEC_CIPHER_WEP128,
	SEC_CIPHER_TKIP,
	SEC_CIPHER_CCMP128,
	SEC_CIPHER_CCMP256,
	SEC_CIPHER_GCMP128,
	SEC_CIPHER_GCMP256,
	SEC_CIPHER_BIP_CMAC128,
	SEC_CIPHER_BIP_CMAC256,
	SEC_CIPHER_BIP_GMAC128,
	SEC_CIPHER_BIP_GMAC256,
	SEC_CIPHER_WPI_SMS4, /* WPI SMS4 support */
	SEC_CIPHER_MAX /* Not a real mode, defined as upper bound */
} SEC_CIPHER_MODE;

#define IS_CIPHER_NONE(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_NONE)) > 0)
#define IS_CIPHER_WEP40(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_WEP40)) > 0)
#define IS_CIPHER_WEP104(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP104)) > 0)
#define IS_CIPHER_WEP128(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP128)) > 0)
#define IS_CIPHER_WEP(_Cipher)              (((_Cipher) & ((1 << SEC_CIPHER_WEP40) | (1 << SEC_CIPHER_WEP104) | (1 << SEC_CIPHER_WEP128))) > 0)
#define IS_CIPHER_TKIP(_Cipher)              (((_Cipher) & (1 << SEC_CIPHER_TKIP)) > 0)
#define IS_CIPHER_WEP_TKIP_ONLY(_Cipher)     ((IS_CIPHER_WEP(_Cipher) || IS_CIPHER_TKIP(_Cipher)) && (_Cipher < (1 << SEC_CIPHER_CCMP128)))
#define IS_CIPHER_CCMP128(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP128)) > 0)
#define IS_CIPHER_CCMP256(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP256)) > 0)
#define IS_CIPHER_GCMP128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP128)) > 0)
#define IS_CIPHER_GCMP256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP256)) > 0)
#define IS_CIPHER_BIP_CMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC128)) > 0)
#define IS_CIPHER_BIP_CMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC256)) > 0)
#define IS_CIPHER_BIP_GMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC128)) > 0)
#define IS_CIPHER_BIP_GMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC256)) > 0)

/* 802.11 authentication and key management */
typedef enum _SEC_AKM_MODE {
	SEC_AKM_OPEN,
	SEC_AKM_SHARED,
	SEC_AKM_AUTOSWITCH,
	SEC_AKM_WPA1, /* Enterprise security over 802.1x */
	SEC_AKM_WPA1PSK,
	SEC_AKM_WPANone, /* For Win IBSS, directly PTK, no handshark */
	SEC_AKM_WPA2, /* Enterprise security over 802.1x */
	SEC_AKM_WPA2PSK,
	SEC_AKM_FT_WPA2,
	SEC_AKM_FT_WPA2PSK,
	SEC_AKM_WPA2_SHA256,
	SEC_AKM_WPA2PSK_SHA256,
	SEC_AKM_TDLS,
	SEC_AKM_SAE_SHA256,
	SEC_AKM_FT_SAE_SHA256,
	SEC_AKM_SUITEB_SHA256,
	SEC_AKM_SUITEB_SHA384,
	SEC_AKM_FT_WPA2_SHA384,
	SEC_AKM_WAICERT, /* WAI certificate authentication */
	SEC_AKM_WAIPSK, /* WAI pre-shared key */
	SEC_AKM_OWE,
	SEC_AKM_FILS_SHA256,
	SEC_AKM_FILS_SHA384,
	SEC_AKM_WPA3, /* WPA3(ent) = WPA2(ent) + PMF MFPR=1 => WPA3 code flow is same as WPA2, the usage of SEC_AKM_WPA3 is to force pmf on */
	SEC_AKM_MAX /* Not a real mode, defined as upper bound */
} SEC_AKM_MODE;

#define IS_AKM_OPEN(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_OPEN)) > 0)
#define IS_AKM_SHARED(_AKMMap)                       ((_AKMMap & (1 << SEC_AKM_SHARED)) > 0)
#define IS_AKM_AUTOSWITCH(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_AUTOSWITCH)) > 0)
#define IS_AKM_WPA1(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_WPA1)) > 0)
#define IS_AKM_WPA1PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA1PSK)) > 0)
#define IS_AKM_WPANONE(_AKMMap)                  ((_AKMMap & (1 << SEC_AKM_WPANone)) > 0)
#define IS_AKM_WPA2(_AKMMap)                          ((_AKMMap & (1 << SEC_AKM_WPA2)) > 0)
#define IS_AKM_WPA2PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA2PSK)) > 0)
#define IS_AKM_FT_WPA2(_AKMMap)                     ((_AKMMap & (1 << SEC_AKM_FT_WPA2)) > 0)
#define IS_AKM_FT_WPA2PSK(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_FT_WPA2PSK)) > 0)
#define IS_AKM_WPA2_SHA256(_AKMMap)            ((_AKMMap & (1 << SEC_AKM_WPA2_SHA256)) > 0)
#define IS_AKM_WPA2PSK_SHA256(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_WPA2PSK_SHA256)) > 0)
#define IS_AKM_TDLS(_AKMMap)                             ((_AKMMap & (1 << SEC_AKM_TDLS)) > 0)
#define IS_AKM_SAE_SHA256(_AKMMap)                ((_AKMMap & (1 << SEC_AKM_SAE_SHA256)) > 0)
#define IS_AKM_FT_SAE_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_FT_SAE_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA384(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA384)) > 0)
#define IS_AKM_FT_WPA2_SHA384(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_FT_WPA2_SHA384)) > 0)
#define IS_AKM_WPA3(_AKMMap)	 ((_AKMMap & (1 << SEC_AKM_WPA3)) > 0)
#define IS_AKM_WPA3PSK(_AKMMap) (IS_AKM_SAE_SHA256(_AKMMap))
#define IS_AKM_WPA3_192BIT(_AKMMap)	(IS_AKM_SUITEB_SHA384(_AKMMap))
#define IS_AKM_OWE(_AKMMap) ((_AKMMap & (1 << SEC_AKM_OWE)) > 0)

int get_macaddr(lua_State *L);
int convert_string_display(lua_State *L);
int StaInfo(lua_State *L);
int getWMOde(lua_State *L);
int getTxPower(lua_State *L);
int getTempature(lua_State *L);
int getChannel(lua_State *L);
int scanResult(lua_State *L);
void getRate(HTTRANSMIT_SETTING_FIX HTSetting, ULONG *fLastTxRxRate);
void get_rate_he(UINT8 mcs, UINT8 bw, UINT8 nss, UINT8 dcm, ULONG *last_tx_rate);
UINT32 cck_to_mcs(UINT32 mcs);
const char *GetEncryModeStr(UINT32 encryMode);
const char *GetAuthModeStr(UINT32 authMode);

#endif
