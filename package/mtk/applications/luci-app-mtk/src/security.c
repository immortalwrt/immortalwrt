#include "mtwifi.h"

const char *GetEncryModeStr(UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return "NONE";
	else if (IS_CIPHER_WEP(encryMode))
		return "WEP";
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return "TKIPAES";
	else if (IS_CIPHER_TKIP(encryMode))
		return "TKIP";
	else if (IS_CIPHER_CCMP128(encryMode))
		return "AES";
	else if (IS_CIPHER_CCMP256(encryMode))
		return "CCMP256";
	else if (IS_CIPHER_GCMP128(encryMode))
		return "GCMP128";
	else if (IS_CIPHER_GCMP256(encryMode))
		return "GCMP256";
	else if (IS_CIPHER_BIP_CMAC128(encryMode))
		return "BIP-CMAC128";
	else if (IS_CIPHER_BIP_CMAC256(encryMode))
		return "BIP-CMAC256";
	else if (IS_CIPHER_BIP_GMAC128(encryMode))
		return "BIP-GMAC128";
	else if (IS_CIPHER_BIP_GMAC256(encryMode))
		return "BIP-GMAC256";
	else
		return "Unknown";
}

const char *GetAuthModeStr(UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return "OPEN";
	else if (IS_AKM_SHARED(authMode))
		return "SHARED";
	else if (IS_AKM_AUTOSWITCH(authMode))
		return "WEPAUTO";
	else if (IS_AKM_WPANONE(authMode))
		return "WPANONE";
	else if (IS_AKM_FT_WPA2PSK(authMode) && IS_AKM_FT_SAE_SHA256(authMode))
		return "FT-WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return "WPA1WPA2";
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return "WPAPSKWPA2PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode))
		return "WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA2PSK_SHA256(authMode) && IS_AKM_WPA3PSK(authMode))
		return "WPA2PSKMIXWPA3PSK";
	else if (IS_AKM_FT_SAE_SHA256(authMode))
		return "FT-SAE";
	else if (IS_AKM_WPA3PSK(authMode))
		return "WPA3PSK";
	else if (IS_AKM_WPA1(authMode))
		return "WPA";
	else if (IS_AKM_WPA1PSK(authMode))
		return "WPAPSK";
	else if (IS_AKM_FT_WPA2(authMode))
		return "FT-WPA2";
	else if (IS_AKM_FT_WPA2PSK(authMode))
		return "FT-WPA2PSK";
	else if (IS_AKM_WPA3(authMode)) /* WPA3 will be always accompanied by WPA2, so it should put before the WPA2 */
		return "WPA3";
	else if (IS_AKM_WPA2(authMode))
		return "WPA2";
	else if (IS_AKM_WPA2(authMode) && IS_AKM_WPA2_SHA256(authMode))
		return "WPA2MIX";
	else if (IS_AKM_WPA2PSK(authMode))
		return "WPA2PSK";
	else if (IS_AKM_WPA3_192BIT(authMode))
		return "WPA3-192";
	else if (IS_AKM_OWE(authMode))
		return "OWE";
	else
		return "Unknown";
}
