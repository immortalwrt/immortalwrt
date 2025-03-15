#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/wireless.h>

#include "mtwifi.h"

int luaopen_ioctl_helper(lua_State *L)
{
	lua_register(L,"c_get_macaddr",get_macaddr);
	lua_register(L,"c_convert_string_display",convert_string_display);
	lua_register(L,"c_StaInfo",StaInfo);
	lua_register(L,"c_getWMode",getWMOde);
	lua_register(L,"c_getTempature",getTempature);
	lua_register(L,"c_scanResult",scanResult);
	lua_register(L,"c_getTxPower",getTxPower);
	lua_register(L,"c_getChannel",getChannel);
	return 0;
}

int scanResult(lua_State *L)
{
	int socket_id;
	const char *interface = luaL_checkstring(L, 1);
	const char *tmp_idx = luaL_checkstring(L, 2);
	struct iwreq wrq;
	char *data = NULL;
	unsigned int data_len = 5000;

	if((data = (char *)malloc(data_len)) == NULL){
		fprintf(stderr, "%s: malloc failed\n", __func__);
		return -1;
	}
	memset(data, 0, data_len);
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) {
		perror("socket() failed");
		free(data);
		return socket_id;
	}

	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", interface);
	snprintf(data, data_len, "%s", tmp_idx);
	wrq.u.data.length = data_len;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	if (ioctl(socket_id, RTPRIV_IOCTL_GSITESURVEY, &wrq) < 0) {
		fprintf(stderr, "ioctl -> RTPRIV_IOCTL_GSITESURVEY Fail !");
		close(socket_id);
		free(data);
		return -1;
	}
	lua_newtable(L);
	lua_pushstring(L, "scanresult");  /* push key */
	lua_pushstring(L, data);  /* push value */
	lua_settable(L, -3);
	close(socket_id);
	free(data);

	return 1;
}

static unsigned int get_temp(const char *interface)
{
	int socket_id;
	struct iwreq wrq;
	unsigned int tempature = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) {
		perror("socket() failed");
		return socket_id;
	}

	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", interface);
	wrq.u.data.length = sizeof(tempature);
	wrq.u.data.pointer = &tempature;
	wrq.u.data.flags = OID_GET_CPU_TEMPERATURE;
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
		fprintf(stderr, "%s: ioctl fail\n", __func__);
	close(socket_id);

	return tempature;
}

int getTempature(lua_State *L)
{
	char tempstr[5] = {0};
	const char *interface = luaL_checkstring(L, 1);
	snprintf(tempstr, sizeof(tempstr), "%d", get_temp(interface));
	lua_newtable(L);
	lua_pushstring(L, "tempature");  /* push key */
	lua_pushstring(L, tempstr);  /* push value */
	lua_settable(L, -3);
	/* Returning one table which is already on top of Lua stack. */
	return 1;
}

static unsigned int get_txpower(const char *interface)
{
	int socket_id;
	struct iwreq wrq;
	int txpower = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) {
		perror("socket() failed");
		return socket_id;
	}

	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", interface);
	wrq.u.txpower.flags = 0;

	if(ioctl(socket_id, SIOCGIWTXPOW, &wrq) >= 0) {
		txpower = wrq.u.txpower.value;
	} else {
		fprintf(stderr, "%s: ioctl fail\n", __func__);
	}

	close(socket_id);

	return txpower;
}

int getTxPower(lua_State *L)
{
	char tempstr[5] = {0};
	const char *interface = luaL_checkstring(L, 1);
	snprintf(tempstr, sizeof(tempstr), "%d", get_txpower(interface));
	lua_newtable(L);
	lua_pushstring(L, "txpower");  /* push key */
	lua_pushstring(L, tempstr);  /* push value */
	lua_settable(L, -3);
	/* Returning one table which is already on top of Lua stack. */
	return 1;
}

static unsigned int get_w_mode(const char *interface)
{
	int socket_id;
	struct iwreq wrq;
	unsigned char data = 0;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) {
		perror("socket() failed");
		return socket_id;
	}

	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", interface);
	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = &data;
	wrq.u.data.flags = OID_GET_WMODE;
	if( ioctl(socket_id, RT_PRIV_IOCTL, &wrq) == -1)
		fprintf(stderr, "%s: ioctl fail\n", __func__);
	close(socket_id);

	return data;
}

int get_macaddr(lua_State *L)
{
	const char *ifname = luaL_checkstring(L, 1);
	struct ifreq ifr;
	char *ptr;
	int skfd;
	static char if_hw[18] = {0};

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		//printf(stderr, "%s: open socket error\n", __func__);
		return skfd;
	}
	snprintf(ifr.ifr_name, IF_NAMESIZE, "%s", ifname);
	if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl fail\n", __func__);
		return -1;
	}

	ptr = (char *)&ifr.ifr_addr.sa_data;
	sprintf(if_hw, "%02X:%02X:%02X:%02X:%02X:%02X",
			(ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
			(ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));
	close(skfd);

	lua_newtable(L);
	lua_pushstring(L, "macaddr");  /* push key */
	lua_pushstring(L, if_hw);  /* push value */
	lua_settable(L, -3);
	/* Returning one table which is already on top of Lua stack. */
	return 1;
}

static int get_channel(const char *interface)
{
	int socket_id;
	struct iwreq wrq;
	int channel = -1;
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_id < 0) {
		perror("socket() failed");
		return socket_id;
	}

	snprintf(wrq.ifr_name, sizeof(wrq.ifr_name), "%s", interface);

	if(ioctl(socket_id, SIOCGIWFREQ, &wrq) >= 0) {
		channel = wrq.u.freq.m;
	} else {
		fprintf(stderr, "%s: ioctl fail\n", __func__);
	}

	close(socket_id);

	return channel;
}

int getChannel(lua_State *L)
{
	char tempstr[5] = {0};
	const char *interface = luaL_checkstring(L, 1);

	snprintf(tempstr, sizeof(tempstr), "%d", get_channel(interface));
	lua_newtable(L);
	lua_pushstring(L, "channel");  /* push key */
	lua_pushstring(L, tempstr);  /* push value */
	lua_settable(L, -3);
	/* Returning one table which is already on top of Lua stack. */
	return 1;
}

int getWMOde(lua_State *L)
{
	char w_mode[5];
	const char *interface = luaL_checkstring(L, 1);
	snprintf(w_mode, sizeof(w_mode), "%d", get_w_mode(interface));
	lua_newtable(L);
	lua_pushstring(L, "getwmode");  /* push key */
	lua_pushstring(L, w_mode);  /* push value */
	lua_settable(L, -3);
	/* Returning one table which is already on top of Lua stack. */
	return 1;
}

int convert_string_display(lua_State *L)
{
#define BUF_SIZE	256
	int  len, i;
	char buffer[BUF_SIZE];		// 33(characters in SSID) * 6(maximum length of a HTML entity)  = 198 + 1(null character) = 199
	char *pOut,*pBufLimit;
	const char *str = luaL_checkstring(L, 1);

	memset(buffer,0,BUF_SIZE);
	len = strlen(str);
	pOut = &buffer[0];
	pBufLimit = &buffer[BUF_SIZE - 1];
	for (i = 0; i < len && (pBufLimit - pOut) >=7; i++) { // 6(maximum length of a HTML entity) + 1(null character) = 7
		switch (str[i]) {
		case 38:
			sprintf(pOut, "&amp;");		// '&'
			pOut += 5;
			break;

		case 60:
			sprintf(pOut, "&lt;");		// '<'
			pOut += 4;
			break;

		case 62:
			sprintf(pOut, "&gt;");		// '>'
			pOut += 4;
			break;

		case 34:
			sprintf(pOut, "&#34;");		// '"'
			pOut += 5;
			break;

		case 39:
			sprintf(pOut, "&#39;");		// '''
			pOut += 5;
			break;
		case 32:
			sprintf(pOut, "&nbsp;");	// ' '
			pOut += 6;
			break;

		default:
			if ((str[i]>=0) && (str[i]<=31)) {
				//Device Control Characters
				sprintf(pOut, "&#%02d;", str[i]);
				pOut += 5;
			} else if ((str[i]==39) || (str[i]==47) || (str[i]==59) || (str[i]==92)) {
				// ' / ; (backslash)
				sprintf(pOut, "&#%02d;", str[i]);
				pOut += 5;
			} else if (str[i]>=127) {
				//Device Control Characters
				sprintf(pOut, "&#%03d;", str[i]);
				pOut += 6;
			} else {
				*pOut = str[i];
				pOut++;
			}
			break;
		}
	}
	*pOut = '\0';
	lua_newtable(L);
	lua_pushstring(L, "output");  /* push key */
	lua_pushstring(L, buffer);  /* push value */
	lua_settable(L, -3);
	return 1;
}

int StaInfo(lua_State *L)
{
	int i, s;
	struct iwreq iwr;
	RT_802_11_MAC_TABLE_FIX *table;
	char tmpBuff[128] = {0};
	char *phyMode[12] = {"CCK", "OFDM", "MM", "GF", "VHT", "HE",
		"HE5G", "HE2G", "HE_SU", "HE_EXT_SU", "HE_TRIG", "HE_MU"};
	const char *interface = luaL_checkstring(L, 1);

	table = (RT_802_11_MAC_TABLE_FIX *)malloc(sizeof(RT_802_11_MAC_TABLE_FIX));
	if (!table)
		return -ENOMEM;

	memset(table, 0, sizeof(RT_802_11_MAC_TABLE_FIX));

	s = socket(AF_INET, SOCK_DGRAM, 0);

	snprintf(iwr.ifr_name, IFNAMSIZ, "%s", interface);

	iwr.u.data.pointer = table;

	if (s < 0) {
		free(table);
		return 0;
	}

	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		free(table);
		close(s);
		return 0;
	}

	close(s);

	/* Creates parent table of size table.Num array elements: */
	lua_createtable(L, table->Num, 0);

	for (i = 0; i < table->Num; i++) {

		lua_pushnumber(L, i);

		RT_802_11_MAC_ENTRY_FIX *pe = &(table->Entry[i]);

		HTTRANSMIT_SETTING_FIX RxRate;
		HTTRANSMIT_SETTING_FIX TxRate;
		RxRate.word = pe->LastRxRate.word;
		TxRate.word = pe->TxRate.word;

		unsigned int mcs = TxRate.field.MCS;
		unsigned int nss = 0;
		unsigned long DataRate = 0;

		unsigned int mcs_r = RxRate.field.MCS;
		unsigned int nss_r = 0;
		unsigned long DataRate_r = 0;

		int hr, min, sec;

		hr = pe->ConnectedTime/3600;
		min = (pe->ConnectedTime % 3600)/60;
		sec = pe->ConnectedTime - hr*3600 - min*60;

		/* Creates first child table of size 34 non-array elements: */
		lua_createtable(L, 0, 34);

		// MAC Address
		snprintf(tmpBuff, sizeof(tmpBuff), "%02X:%02X:%02X:%02X:%02X:%02X", pe->Addr[0], pe->Addr[1], pe->Addr[2], pe->Addr[3],
				pe->Addr[4], pe->Addr[5]);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "MacAddr");

		// AID, Power Save mode, MIMO Power Save
		snprintf(tmpBuff, sizeof(tmpBuff), "%d", pe->Aid);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Aid");

		snprintf(tmpBuff, sizeof(tmpBuff), "%d", pe->Psm);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Psm");

		snprintf(tmpBuff, sizeof(tmpBuff), "%d", pe->MimoPs);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "MimoPs");

		// TX Rate NSS-MCS
		if (TxRate.field.MODE >= MODE_VHT) {
			nss = ((mcs & (0x3 << 4)) >> 4) + 1;
			mcs = mcs & 0xF;
			snprintf(tmpBuff, sizeof(tmpBuff), "%dSS-MCS%d", nss, mcs);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "Mcs");
		} else {
			mcs = mcs & 0x3f;
			snprintf(tmpBuff, sizeof(tmpBuff), "MCS%d", mcs);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "Mcs");
		}

		// TX BW
		if (TxRate.field.BW == BW_20) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 20);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "Bw");
		} else if (TxRate.field.BW == BW_40) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 40);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "Bw");
		} else if (TxRate.field.BW == BW_80) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 80);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "Bw");
		} else if (TxRate.field.BW == BW_160) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 160);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "Bw");
		}

		//TX SGI
		if (TxRate.field.MODE < MODE_HTMIX) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%s", "LGI");
		} else if (TxRate.field.MODE >= MODE_HE) {
			if (TxRate.field.ShortGI == 0)
				snprintf(tmpBuff, sizeof(tmpBuff), "%s", "SGI");
			else if (TxRate.field.ShortGI == 1)
				snprintf(tmpBuff, sizeof(tmpBuff), "%s", "MGI");
			else if (TxRate.field.ShortGI == 2)
				snprintf(tmpBuff, sizeof(tmpBuff), "%s", "LGI");
		} else {
			snprintf(tmpBuff, sizeof(tmpBuff), "%s", TxRate.field.ShortGI ? "SGI" : "LGI");
		}
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Gi");

		//TX phy mode
		snprintf(tmpBuff, sizeof(tmpBuff), "%s", phyMode[TxRate.field.MODE]);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "PhyMode");

		//TX STBC
		snprintf(tmpBuff, sizeof(tmpBuff), "%s", TxRate.field.STBC? "STBC": " ");
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Stbc");

		// TxBF configuration
		snprintf(tmpBuff, sizeof(tmpBuff), "%c", TxRate.field.iTxBF ? 'I': '-');
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "iTxBF");

		snprintf(tmpBuff, sizeof(tmpBuff), "%c", TxRate.field.eTxBF ? 'E': '-');
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "eTxBF");

		// RSSI
		snprintf(tmpBuff, sizeof(tmpBuff), "%d", (int)(pe->AvgRssi0));
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "AvgRssi0");

		snprintf(tmpBuff, sizeof(tmpBuff), "%d", (int)(pe->AvgRssi1));
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "AvgRssi1");

		snprintf(tmpBuff, sizeof(tmpBuff), "%d", (int)(pe->AvgRssi2));
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "AvgRssi2");

		snprintf(tmpBuff, sizeof(tmpBuff), "%d", (int)(pe->AvgRssi3));
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "AvgRssi3");

		// Per Stream SNR
		snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->StreamSnr[0]*0.25);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "StreamSnr0");
		snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->StreamSnr[1]*0.25); //mcs>7? pe->StreamSnr[1]*0.25: 0.0);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "StreamSnr1");
		snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->StreamSnr[2]*0.25); //mcs>15? pe->StreamSnr[2]*0.25: 0.0);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "StreamSnr2");

		// Sounding Response SNR
		if (TxRate.field.eTxBF) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->SoundingRespSnr[0]*0.25);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "SoundingRespSnr0");
			snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->SoundingRespSnr[1]*0.25);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "SoundingRespSnr1");
			snprintf(tmpBuff, sizeof(tmpBuff), "%0.1f", pe->SoundingRespSnr[2]*0.25);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "SoundingRespSnr2");
		}

		// Last RX NSS-MCS
		if (RxRate.field.MODE >= MODE_VHT) {
			nss_r = (((mcs_r & (0x3 << 4)) >> 4) + 1) / (RxRate.field.STBC + 1);
			mcs_r = mcs_r & 0xF;
			snprintf(tmpBuff, sizeof(tmpBuff), "%dSS-MCS%d", nss_r, mcs_r);
		} else if (RxRate.field.MODE >= MODE_HTMIX) {
			mcs_r = mcs_r & 0x3f;
			snprintf(tmpBuff, sizeof(tmpBuff), "MCS%d", mcs_r);
		} else if (RxRate.field.MODE == MODE_OFDM) {
			mcs_r = mcs_r & 0xf;
			snprintf(tmpBuff, sizeof(tmpBuff), "MCS%d", mcs_r);
			RxRate.field.MCS = mcs_r;
		} else if (RxRate.field.MODE == MODE_CCK) {
			mcs_r = cck_to_mcs(mcs_r & 0x7);
			snprintf(tmpBuff, sizeof(tmpBuff), "MCS%d", mcs_r);
			RxRate.field.MCS = mcs_r;
		}

		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "LastMcs");

		if (RxRate.field.BW == BW_20) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 20);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "LastBw");
		} else if (RxRate.field.BW == BW_40) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 40);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "LastBw");
		} else if (RxRate.field.BW == BW_80) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 80);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "LastBw");
		} else if (RxRate.field.BW == BW_160) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%d", 160);
			lua_pushstring(L, tmpBuff);
			lua_setfield(L, -2, "LastBw");
		}

		if (RxRate.field.MODE < MODE_HTMIX) {
			snprintf(tmpBuff, sizeof(tmpBuff), "%s", "LGI");
		} else if (RxRate.field.MODE >= MODE_HE) {
			if (RxRate.field.ShortGI == 0)
				snprintf(tmpBuff, sizeof(tmpBuff), "%s", "SGI");
			else if (RxRate.field.ShortGI == 1)
				snprintf(tmpBuff, sizeof(tmpBuff), "%s", "MGI");
			else if (RxRate.field.ShortGI == 2)
				snprintf(tmpBuff, sizeof(tmpBuff), "%s", "LGI");
		} else {
			snprintf(tmpBuff, sizeof(tmpBuff), "%s", RxRate.field.ShortGI ? "SGI" : "LGI");
		}
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "LastGi");

		snprintf(tmpBuff, sizeof(tmpBuff), "%s", phyMode[RxRate.field.MODE]);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "LastPhyMode");

		snprintf(tmpBuff, sizeof(tmpBuff), "%s", RxRate.field.STBC ? "STBC": " ");
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "LastStbc");

		// Connect time
		snprintf(tmpBuff, sizeof(tmpBuff), "%02d", hr);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Hr");

		snprintf(tmpBuff, sizeof(tmpBuff), "%02d", min);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Min");

		snprintf(tmpBuff, sizeof(tmpBuff), "%02d", sec);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "Sec");

		if (TxRate.field.MODE >= MODE_HE) {
			get_rate_he((mcs & 0xf), TxRate.field.BW, nss, 0, &DataRate);
			if (TxRate.field.ShortGI == 1)
				DataRate = (DataRate * 967) >> 10;
			else if (TxRate.field.ShortGI == 2)
				DataRate = (DataRate * 870) >> 10;
		} else {
			getRate(TxRate, &DataRate);
		}

		snprintf(tmpBuff, sizeof(tmpBuff), "%ld", DataRate);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "TxRate");

		if (RxRate.field.MODE >= MODE_HE) {
			get_rate_he((mcs_r & 0xf), RxRate.field.BW, nss_r, 0, &DataRate_r);
			if (RxRate.field.ShortGI == 1)
				DataRate_r = (DataRate_r * 967) >> 10;
			else if (RxRate.field.ShortGI == 2)
				DataRate_r = (DataRate_r * 870) >> 10;
		} else {
			getRate(RxRate, &DataRate_r);
		}

		snprintf(tmpBuff, sizeof(tmpBuff), "%ld", DataRate_r);
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "RxRate");

		snprintf(tmpBuff, sizeof(tmpBuff), "%s", GetEncryModeStr(pe->EncryMode));
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "EncryptMode");

		snprintf(tmpBuff, sizeof(tmpBuff), "%s", GetAuthModeStr(pe->AuthMode));
		lua_pushstring(L, tmpBuff);
		lua_setfield(L, -2, "AuthMode");

		lua_settable(L, -3);
	}
	free(table);
	return 1;
}
