#include <inttypes.h>
#include "iwinfo.h"
#include "iwinfo_wext.h"
#include "mtwifi.h"

#include "iwinfo_mtk_ccode.c"

static inline int mtk_ioctl(const char *ifname, int cmd, struct iwreq *wrq)
{
	strncpy(wrq->ifr_name, ifname, IFNAMSIZ);
	return iwinfo_ioctl(cmd, wrq);
}

static const char *mtk_dev2phy(const char *devname)
{
	const char *phy = NULL;
	struct uci_section *s;

	if (strstr(devname,"ra") || strstr(devname,"apcli"))
		return devname;

	s = iwinfo_uci_get_radio(devname, "mtwifi");
	if (!s)
		goto out;
	
	phy = uci_lookup_option_string(uci_ctx, s, "phy");

out:
	iwinfo_uci_free();
	return phy;
}

static int mtk_probe(const char *dev)
{
	const char *phy = NULL;
	struct uci_section *s;

	if (strstr(dev,"ra") || strstr(dev,"apcli"))
		return true;

	s = iwinfo_uci_get_radio(dev, "mtwifi");
	if (!s)
		goto out;

	phy = uci_lookup_option_string(uci_ctx, s, "phy");
	if (phy) {
		iwinfo_uci_free();
		return true;
	}

out:
	iwinfo_uci_free();
	return false;
}

static void mtk_close(void)
{
	iwinfo_close();
}

static int mtk_is_ifup(const char *ifname)
{
	struct ifreq ifr;

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	if (iwinfo_ioctl(SIOCGIFFLAGS, &ifr) >= 0)
	{
		if (ifr.ifr_flags & IFF_UP)
			return 1;
	}

	return 0;
}

static int mtk_get_mode(const char *dev, int *buf)
{
	struct iwreq wrq;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if(mtk_ioctl(ifname, SIOCGIWMODE, &wrq) >= 0)
	{
		switch(wrq.u.mode)
		{
			case 1:
				*buf = IWINFO_OPMODE_ADHOC;
				break;

			case 2:
				*buf = IWINFO_OPMODE_CLIENT;
				break;

			case 3:
				*buf = IWINFO_OPMODE_MASTER;
				break;

			case 6:
				*buf = IWINFO_OPMODE_MONITOR;
				break;

			default:
				*buf = IWINFO_OPMODE_UNKNOWN;
				break;
		}

		return 0;
	}

	return -1;
}

static int mtk_get_ssid(const char *dev, char *buf)
{
	struct iwreq wrq = {};
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	wrq.u.essid.pointer = buf;
	wrq.u.essid.length  = IW_ESSID_MAX_SIZE;

	if(mtk_ioctl(ifname, SIOCGIWESSID, &wrq) >= 0)
		return 0;

	return -1;
}

static int mtk_get_bssid(const char *dev, char *buf)
{
	struct iwreq wrq;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if(mtk_ioctl(ifname, SIOCGIWAP, &wrq) >= 0)
	{
		sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
			(uint8_t)wrq.u.ap_addr.sa_data[0], (uint8_t)wrq.u.ap_addr.sa_data[1],
			(uint8_t)wrq.u.ap_addr.sa_data[2], (uint8_t)wrq.u.ap_addr.sa_data[3],
			(uint8_t)wrq.u.ap_addr.sa_data[4], (uint8_t)wrq.u.ap_addr.sa_data[5]);

		return 0;
	}

	return -1;
}

static int mtk_get_bitrate(const char *dev, int *buf)
{
	struct iwreq wrq;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if(mtk_ioctl(ifname, SIOCGIWRATE, &wrq) >= 0)
	{
		*buf = (wrq.u.bitrate.value / 1000);
		return 0;
	}

	return -1;
}

static int mtk_get_channel(const char *dev, int *buf)
{
	struct iwreq wrq;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (mtk_ioctl(ifname, SIOCGIWFREQ, &wrq) >= 0)
	{
		*buf = wrq.u.freq.m;
		return 0;
	}

	return -1;
}

static int mtk_get_center_chan1(const char *dev, int *buf)
{
	/* Not Supported */
	return -1;
}

static int mtk_get_center_chan2(const char *dev, int *buf)
{
	/* Not Supported */
	return -1;
}

static int mtk_get_frequency(const char *dev, int *buf)
{
	int channel;
	struct iwreq wrq;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (mtk_ioctl(ifname, SIOCGIWFREQ, &wrq) >= 0)
	{
		channel = wrq.u.freq.m;

		if (channel <= 0)
			return -1;

		if (channel > 14) {
			if (channel >= 182 && channel <= 196)
				*buf = 4000 + channel * 5;
			else
				*buf = 5000 + channel * 5;
		} else if (channel == 14) {
			*buf = 2484;
		} else {
			*buf = 2407 + channel * 5;
		}

		return 0;
	}

	return -1;
	
}

static int mtk_get_txpower(const char *dev, int *buf)
{
	struct iwreq wrq;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	wrq.u.txpower.flags = 0;

	if(mtk_ioctl(ifname, SIOCGIWTXPOW, &wrq) >= 0)
	{
		*buf = wrq.u.txpower.value;
		return 0;
	}

	return -1;
}

static int mtk_get_signal(const char *dev, int *buf)
{
	return mtk_get_txpower(dev, buf);
}

static int mtk_get_noise(const char *dev, int *buf)
{
	return -1;
}

static int mtk_get_quality(const char *dev, int *buf)
{
	*buf = 100;
	return 0;
}

static int mtk_get_quality_max(const char *dev, int *buf)
{
	*buf = 100;
	return 0;
}

static void fill_rate_info(HTTRANSMIT_SETTING HTSetting, struct iwinfo_rate_entry *re,
	unsigned int mcs, unsigned int nss)
{
	unsigned long DataRate = 0;

	if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE < MODE_HE)
	{
		if (HTSetting.field.ShortGI)
			re->is_short_gi = 1;
	}

	if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE <= MODE_HTGREENFIELD)
		re->is_ht = 1;
	else if (HTSetting.field.MODE == MODE_VHT)
		re->is_vht = 1;
	else if (HTSetting.field.MODE >= MODE_HE)
		re->is_he = 1;

	if (HTSetting.field.MODE >= MODE_HE)
		re->he_gi = HTSetting.field.ShortGI;

	if (HTSetting.field.BW == BW_20)
		re->mhz = 20;
	else if (HTSetting.field.BW == BW_40)
		re->mhz = 40;
	else if (HTSetting.field.BW == BW_80)
		re->mhz = 80;
	else if (HTSetting.field.BW == BW_160)
		re->mhz = 160;

	re->is_40mhz = (re->mhz == 40);

	if (HTSetting.field.MODE >= MODE_HE) {
		get_rate_he((mcs & 0xf), HTSetting.field.BW, nss, 0, &DataRate);
		if (HTSetting.field.ShortGI == 1)
			DataRate = (DataRate * 967) >> 10;
		else if (HTSetting.field.ShortGI == 2)
			DataRate = (DataRate * 870) >> 10;
	} else {
		getRate(HTSetting, &DataRate);
	}
	re->rate = (uint32_t)(DataRate * 1000);
}

static void mtk_parse_rateinfo(RT_802_11_MAC_ENTRY *pe, 
	struct iwinfo_rate_entry *rx_rate, struct iwinfo_rate_entry *tx_rate)
{
	HTTRANSMIT_SETTING TxRate;
	HTTRANSMIT_SETTING RxRate;

	unsigned int mcs = 0;
	unsigned int nss = 0;

	unsigned int mcs_r = 0;
	unsigned int nss_r = 0;

	TxRate.word = pe->TxRate.word;
	RxRate.word = pe->LastRxRate.word;

	mcs = TxRate.field.MCS;
	mcs_r = RxRate.field.MCS;

	if (TxRate.field.MODE >= MODE_VHT) {
		nss = ((mcs & (0x3 << 4)) >> 4) + 1;
		mcs = mcs & 0xF;
		tx_rate->nss = nss;
	} else {
		mcs = mcs & 0x3f;
		tx_rate->nss = 1;
	}
	tx_rate->mcs = mcs;

	if (RxRate.field.MODE >= MODE_VHT) {
		nss_r = (((mcs_r & (0x3 << 4)) >> 4) + 1) / (RxRate.field.STBC + 1);
		mcs_r = mcs_r & 0xF;
		rx_rate->nss = nss_r;
	} else {
		rx_rate->nss = 1;
		if (RxRate.field.MODE >= MODE_HTMIX) {
			mcs_r = mcs_r & 0x3f;
		} else if (RxRate.field.MODE == MODE_OFDM) {
			mcs_r = mcs_r & 0xf;
			RxRate.field.MCS = mcs_r;
		} else if (RxRate.field.MODE == MODE_CCK) {
			mcs_r = cck_to_mcs(mcs_r & 0x7);
			RxRate.field.MCS = mcs_r;
		}
	}
	rx_rate->mcs = mcs_r;

	fill_rate_info(TxRate, tx_rate, mcs, nss);
	fill_rate_info(RxRate, rx_rate, mcs_r, nss_r);
}

static int mtk_get_assoclist(const char *dev, char *buf, int *len)
{
	struct iwreq wrq = {};
	RT_802_11_MAC_TABLE *table;
	int i;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	table = calloc(1, sizeof(RT_802_11_MAC_TABLE));
	if (!table)
		return -1;

	wrq.u.data.pointer = (caddr_t)table;
	wrq.u.data.length  = sizeof(RT_802_11_MAC_TABLE);

	if (mtk_ioctl(ifname, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &wrq) < 0) {
		free(table);
		return -1;
	}

	*len = 0;

	for (i = 0; i < table->Num; i++) {
		RT_802_11_MAC_ENTRY *pe = &(table->Entry[i]);
		struct iwinfo_assoclist_entry *e = (struct iwinfo_assoclist_entry *)buf + i;

		memcpy(e->mac, pe->Addr, 6);
		e->signal = pe->AvgRssi0;
		e->signal_avg = pe->AvgRssi0;
		e->connected_time = pe->ConnectedTime;
		mtk_parse_rateinfo(pe, &e->rx_rate, &e->tx_rate);

		*len += sizeof(struct iwinfo_assoclist_entry);
	}

	free(table);
	return 0;
}

static int mtk_get_txpwrlist(const char *dev, char *buf, int *len)
{
	return -1;
}

static int mtk_get_scanlist_dump(const char *ifname, int index, char *data, size_t len)
{
	struct iwreq wrq = {};

	snprintf(data, len, "%d", index);

	wrq.u.data.pointer = data;
	wrq.u.data.length = len;
	wrq.u.data.flags = GET_MAC_TABLE_STRUCT_FLAG_RAW_SSID;

	return mtk_ioctl(ifname, RTPRIV_IOCTL_GSITESURVEY, &wrq);
}

enum {
	SCAN_DATA_CH,
	SCAN_DATA_SSID,
	SCAN_DATA_BSSID,
	SCAN_DATA_SECURITY,
	SCAN_DATA_RSSI,
	SCAN_DATA_SIG,
	SCAN_DATA_NT,
	SCAN_DATA_SSID_LEN,
	SCAN_DATA_MAX
};

static int mtk_get_scanlist(const char *dev, char *buf, int *len)
{
	struct iwinfo_scanlist_entry *e = (struct iwinfo_scanlist_entry *)buf;
	char *data = NULL;
	unsigned int data_len = 5000;
	int offsets[SCAN_DATA_MAX];
	char cmd[128];
	int index = 0;
	int total = -1;
	char *pos;
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	*len = 0;

	if ((data = (char *)malloc(data_len)) == NULL)
		return -1;

	sprintf(cmd, "iwpriv %s set SiteSurvey=", ifname);
	system(cmd);

	sleep(5);

	while (1) {
		memset(data, 0, data_len);
		if (mtk_get_scanlist_dump(ifname, index, data, sizeof(data))) {
			free(data);
			return -1;
		}

		//printf("%s\n", data);

		sscanf(data, "\nTotal=%d", &total);

		strtok(data, "\n");
		pos = strtok(NULL, "\n");

		offsets[SCAN_DATA_CH] = strstr(pos, "Ch ") - pos;
		offsets[SCAN_DATA_SSID] = strstr(pos, "SSID ") - pos;
		offsets[SCAN_DATA_BSSID] = strstr(pos, "BSSID ") - pos;
		offsets[SCAN_DATA_SECURITY] = strstr(pos, "Security ") - pos;
		offsets[SCAN_DATA_RSSI] = strstr(pos, "Rssi") - pos;
		offsets[SCAN_DATA_SIG] = strstr(pos, "Siganl") - pos;
		offsets[SCAN_DATA_NT] = strstr(pos, "NT") - pos;
		offsets[SCAN_DATA_SSID_LEN] = strstr(pos, "SSID_Len") - pos;

		while (1) {
			struct iwinfo_crypto_entry *crypto = &e->crypto;
			const char *security;
			uint8_t *mac = e->mac;
			int ssid_len;

			pos = strtok(NULL, "\n");
			if (!pos)
				break;

			sscanf(pos, "%d", &index);

			if (strncmp(pos + offsets[SCAN_DATA_NT], "In", 2))
				continue;

			security = pos + offsets[SCAN_DATA_SECURITY];
			if (!strstr(security, "PSK") && !strstr(security, "OPEN") && !strstr(security, "OWE"))
				continue;

			memset(crypto, 0, sizeof(struct iwinfo_crypto_entry));

			if (strstr(security, "PSK") || strstr(security, "OWE")) {
				crypto->enabled = true;

				if (strstr(security, "WPAPSK")) {
					crypto->wpa_version |= 1 << 0;
					crypto->auth_suites |= IWINFO_KMGMT_PSK;
				}

				if (strstr(security, "WPA2PSK")) {
					crypto->wpa_version |= 1 << 1;
					crypto->auth_suites |= IWINFO_KMGMT_PSK;
				}

				if (strstr(security, "WPA3PSK")) {
					crypto->wpa_version |= 1 << 2;
					crypto->auth_suites |= IWINFO_KMGMT_SAE;
				}

				if (strstr(security, "OWE")) {
					crypto->wpa_version |= 1 << 2;
					crypto->auth_suites |= IWINFO_KMGMT_OWE;
				}

				if (strstr(security, "AES")) {
					crypto->pair_ciphers |= IWINFO_CIPHER_CCMP;
				}

				if (strstr(security, "CCMP256")) {
					crypto->pair_ciphers |= IWINFO_CIPHER_CCMP256;
				}

				if (strstr(security, "TKIP")) {
					crypto->pair_ciphers |= IWINFO_CIPHER_TKIP;
				}

				if (strstr(security, "GCMP128")) {
					crypto->pair_ciphers |= IWINFO_CIPHER_GCMP;
				}

				if (strstr(security, "GCMP256")) {
					crypto->pair_ciphers |= IWINFO_CIPHER_GCMP256;
				}
			}

			e->mode = IWINFO_OPMODE_MASTER;

			sscanf(pos + offsets[SCAN_DATA_CH], "%"SCNu8, &e->channel);
			sscanf(pos + offsets[SCAN_DATA_RSSI], "%"SCNu8, &e->signal);
			sscanf(pos + offsets[SCAN_DATA_SIG], "%"SCNu8, &e->quality);
			e->quality_max = 100;

			sscanf(pos + offsets[SCAN_DATA_BSSID], "%02"SCNx8":%02"SCNx8":%02"SCNx8":%02"SCNx8":%02"SCNx8":%02"SCNx8"",
				mac + 0, mac + 1, mac + 2, mac + 3, mac + 4, mac + 5);

			sscanf(pos + offsets[SCAN_DATA_SSID_LEN], "%d", &ssid_len);
			memcpy(e->ssid, pos + offsets[SCAN_DATA_SSID], ssid_len);

			*len += sizeof(struct iwinfo_scanlist_entry);
			e++;

			if (index + 1 == total)
				break;
		}

		if (index + 1 == total)
			break;
		else
			index++;
	}

	free(data);
	return 0;
}

static double wext_freq2float(const struct iw_freq *in)
{
	int		i;
	double	res = (double) in->m;
	for(i = 0; i < in->e; i++) res *= 10;
	return res;
}

static inline int wext_freq2mhz(const struct iw_freq *in)
{
	if( in->e == 6 )
	{
		return in->m;
	}
	else
	{
		return (int)(wext_freq2float(in) / 1000000);
	}
}

static int mtk_get_freqlist(const char *dev, char *buf, int *len)
{
	struct iwreq wrq;
	struct iw_range range;
	struct iwinfo_freqlist_entry entry;
	const char* ifname;
	int i, bl;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (!mtk_is_ifup(ifname))
		return -1;

	wrq.u.data.pointer = (caddr_t) &range;
	wrq.u.data.length  = sizeof(struct iw_range);
	wrq.u.data.flags   = 0;

	if (mtk_ioctl(ifname, SIOCGIWRANGE, &wrq) >= 0)
	{
		bl = 0;

		for (i = 0; i < range.num_frequency; i++)
		{
			entry.mhz        = wext_freq2mhz(&range.freq[i]);
			entry.channel    = range.freq[i].i;
			entry.restricted = 0;

			memcpy(&buf[bl], &entry, sizeof(struct iwinfo_freqlist_entry));
			bl += sizeof(struct iwinfo_freqlist_entry);
		}

		*len = bl;
		return 0;
	}

	return -1;
}

static int mtk_get_country(const char *dev, char *buf)
{
	const char *ifname;
	char data[4] = {0};
	struct iwreq wrq;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	wrq.u.data.length = sizeof(data);
	wrq.u.data.pointer = &data;
	wrq.u.data.flags = OID_802_11_COUNTRYCODE;

	if (mtk_ioctl(ifname, RT_PRIV_IOCTL, &wrq) >= 0)
	{
		memcpy(buf, data, 2);
		return 0;
	}
	return -1;
}

static int mtk_get_countrylist(const char *dev, char *buf, int *len)
{
	int count = sizeof(mtk_country_codes)/sizeof(mtk_country_codes[0]);
	struct iwinfo_country_entry *c = (struct iwinfo_country_entry *)buf;

	for (int i=0; i<count; i++) {
		c->iso3166 = mtk_country_codes[i][0]<<8 | mtk_country_codes[i][1];
		snprintf(c->ccode, sizeof(c->ccode), "%s", mtk_country_codes[i]);
		c++;
	}

	*len = (count * sizeof(struct iwinfo_country_entry));
	return 0;
}

static int mtk_get_hwmodelist(const char *dev, int *buf)
{
	const char *ifname;
	char chans[IWINFO_BUFSIZE] = { 0 };
	struct iwinfo_freqlist_entry *e = NULL;
	struct uci_section *s;
	const char* band = NULL;
	int len = 0;

	*buf = 0;

	/* get hwmode base on uci band config */
	s = iwinfo_uci_get_radio(dev, "mtwifi");
	if (!s)
		goto uciout;
	
	band = uci_lookup_option_string(uci_ctx, s, "band");

uciout:
	iwinfo_uci_free();

	if (band) {
		if (!strcmp(band,"2g"))
			*buf = (IWINFO_80211_N | IWINFO_80211_AX);
		else if (!strcmp(band,"5g"))
			*buf = (IWINFO_80211_AC | IWINFO_80211_AX);
		return 0;
	}

	/* get hwmode base on iwrange */
	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (!mtk_get_freqlist(ifname, chans, &len))
	{
		for (e = (struct iwinfo_freqlist_entry *)chans; e->channel; e++ )
		{
			if (e->channel <= 14 ) //2.4Ghz
			{
				*buf = (IWINFO_80211_N | IWINFO_80211_AX);
			}
			else //5Ghz
			{
				*buf = (IWINFO_80211_AC | IWINFO_80211_AX);
			}
		}

		return 0;
	}

	return -1;
}

static int mtk_get_htmodelist(const char *dev, int *buf)
{
	const char *ifname;
	char chans[IWINFO_BUFSIZE] = { 0 };
	struct iwinfo_freqlist_entry *e = NULL;
	struct uci_section *s;
	const char* band = NULL;
	int len = 0;

	*buf = 0;

	/* get htmode base on uci band config */
	s = iwinfo_uci_get_radio(dev, "mtwifi");
	if (!s)
		goto uciout;
	
	band = uci_lookup_option_string(uci_ctx, s, "band");

uciout:
	iwinfo_uci_free();

	if (band) {
		if (!strcmp(band,"2g"))
			*buf = (IWINFO_HTMODE_HT20 | IWINFO_HTMODE_HT40 | IWINFO_HTMODE_HE20 | IWINFO_HTMODE_HE40);
		else if (!strcmp(band,"5g"))
			*buf = (IWINFO_HTMODE_VHT20 | IWINFO_HTMODE_VHT40 | IWINFO_HTMODE_VHT80 | IWINFO_HTMODE_VHT160
			| IWINFO_HTMODE_HE20 | IWINFO_HTMODE_HE40 | IWINFO_HTMODE_HE80 | IWINFO_HTMODE_HE160);
		return 0;
	}

	/* get htmode base on iwrange */
	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (!mtk_get_freqlist(ifname, chans, &len))
	{
		for (e = (struct iwinfo_freqlist_entry *)chans; e->channel; e++ )
		{
			if (e->channel <= 14 ) //2.4Ghz
			{
				*buf = (IWINFO_HTMODE_HT20 | IWINFO_HTMODE_HT40 | IWINFO_HTMODE_HE20 | IWINFO_HTMODE_HE40);
			}
			else //5Ghz
			{
				*buf = (IWINFO_HTMODE_VHT20 | IWINFO_HTMODE_VHT40 | IWINFO_HTMODE_VHT80 | IWINFO_HTMODE_VHT160
				| IWINFO_HTMODE_HE20 | IWINFO_HTMODE_HE40 | IWINFO_HTMODE_HE80 | IWINFO_HTMODE_HE160);
			}
		}

		return 0;
	}

	return -1;
}

static int mtk_get_htmode(const char *dev, int *buf)
{
	const char *ifname;
	struct iwreq wrq;
	unsigned char bw = 0;
	unsigned long wmode = 0;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (!mtk_is_ifup(ifname))
		return -1;

	wrq.u.data.length = sizeof(bw);
	wrq.u.data.pointer = &bw;
	wrq.u.data.flags = OID_802_11_BW;

	if (mtk_ioctl(ifname, RT_PRIV_IOCTL, &wrq) >= 0)
	{
		wrq.u.data.length = sizeof(wmode);
		wrq.u.data.pointer = &wmode;
		wrq.u.data.flags = RT_OID_802_11_PHY_MODE;

		if (mtk_ioctl(ifname, RT_PRIV_IOCTL, &wrq) >= 0)
		{
			if (WMODE_CAP_AX(wmode)) {
				switch (bw) {
					case BW_20: *buf = IWINFO_HTMODE_HE20; break;
					case BW_40: *buf = IWINFO_HTMODE_HE40; break;
					case BW_80: *buf = IWINFO_HTMODE_HE80; break;
					case BW_8080: *buf = IWINFO_HTMODE_HE80_80; break;
					case BW_160: *buf = IWINFO_HTMODE_HE160; break;
				}
			} else if (WMODE_CAP_AC(wmode)) {
				switch (bw) {
					case BW_20: *buf = IWINFO_HTMODE_VHT20; break;
					case BW_40: *buf = IWINFO_HTMODE_VHT40; break;
					case BW_80: *buf = IWINFO_HTMODE_VHT80; break;
					case BW_8080: *buf = IWINFO_HTMODE_VHT80_80; break;
					case BW_160: *buf = IWINFO_HTMODE_VHT160; break;
				}
			} else if (WMODE_CAP_N(wmode)) {
				switch (bw) {
					case BW_20: *buf = IWINFO_HTMODE_HT20; break;
					case BW_40: *buf = IWINFO_HTMODE_HT40; break;
				}
			} else {
				*buf = IWINFO_HTMODE_NOHT;
			}
			return 0;
		}
	}

	return -1;
}

static int mtk_get_encryption(const char *dev, char *buf)
{
	const char *ifname;
	struct iwreq wrq;
	struct security_info secinfo;
	unsigned int authMode, encryMode;
	struct iwinfo_crypto_entry *c = (struct iwinfo_crypto_entry *)buf;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	if (!mtk_is_ifup(ifname))
		return -1;

	wrq.u.data.length = sizeof(secinfo);
	wrq.u.data.pointer = &secinfo;
	wrq.u.data.flags = OID_802_11_SECURITY_TYPE;

	if (mtk_ioctl(ifname, RT_PRIV_IOCTL, &wrq) >= 0)
	{
		authMode = secinfo.auth_mode;
		encryMode = secinfo.encryp_type;

		c->auth_algs |= IWINFO_AUTH_OPEN;
		if (IS_AKM_SHARED(authMode))
			c->auth_algs |= IWINFO_AUTH_SHARED;			

		if (IS_AKM_WPA1(authMode) || IS_AKM_FT_WPA2(authMode) || IS_AKM_WPANONE(authMode) ||
			IS_AKM_WPA3(authMode) || IS_AKM_WPA2(authMode) || IS_AKM_WPA3_192BIT(authMode))
			c->auth_suites |= IWINFO_KMGMT_8021x;

		if (IS_AKM_WPA1PSK(authMode) || IS_AKM_WPA2PSK(authMode) || IS_AKM_FT_WPA2PSK(authMode) ||
			IS_AKM_WPA2PSK_SHA256(authMode))
			c->auth_suites |= IWINFO_KMGMT_PSK;

		if (IS_AKM_FT_SAE_SHA256(authMode) || IS_AKM_SAE_SHA256(authMode))
			c->auth_suites |= IWINFO_KMGMT_SAE;

		if (IS_AKM_OWE(authMode))
			c->auth_suites |= IWINFO_KMGMT_OWE;

		if (IS_AKM_WPA1(authMode) || IS_AKM_WPA1PSK(authMode))
			c->wpa_version |= 1 << 0;

		if (IS_AKM_WPA2(authMode) || IS_AKM_WPA2PSK(authMode) || IS_AKM_FT_WPA2(authMode) ||
			IS_AKM_FT_WPA2PSK(authMode) || IS_AKM_WPA2_SHA256(authMode) || IS_AKM_WPA2PSK_SHA256(authMode) ||
			IS_AKM_FT_WPA2_SHA384(authMode))
			c->wpa_version |= 1 << 1;

		if (IS_AKM_WPA3(authMode) || IS_AKM_WPA3PSK(authMode) ||
			IS_AKM_WPA3_192BIT(authMode) || IS_AKM_OWE(authMode))
			c->wpa_version |= 1 << 2;

		if (IS_CIPHER_NONE(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_NONE;

		if (IS_CIPHER_WEP40(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_WEP40;

		if (IS_CIPHER_WEP104(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_WEP104;

		if (IS_CIPHER_TKIP(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_TKIP;

		if (IS_CIPHER_CCMP128(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_CCMP;

		if (IS_CIPHER_GCMP128(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_GCMP;

		if (IS_CIPHER_CCMP256(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_CCMP256;

		if (IS_CIPHER_GCMP256(encryMode))
			c->pair_ciphers |= IWINFO_CIPHER_GCMP256;

		c->group_ciphers = c->pair_ciphers;

		if (IS_AKM_OPEN(authMode) && IS_CIPHER_NONE(encryMode))
			c->enabled = 0;
		else
			c->enabled = 1;

		return 0;
	}

	return -1;
}

static int mtk_get_phyname(const char *dev, char *buf)
{
	const char *ifname;

	ifname = mtk_dev2phy(dev);
	if (!ifname)
		return -1;

	strcpy(buf, ifname);

	return 0;
}

static int mtk_get_mbssid_support(const char *dev, int *buf)
{
	*buf = 1;
	return 0;
}

static int mtk_get_l1profile_attr(const char *attr, char *data, int len)
{
	FILE *fp;
	char *key, *val, buf[512];

	fp = fopen(MTK_L1_PROFILE_PATH, "r");
	if (!fp)
		return -1;

	while (fgets(buf, sizeof(buf), fp))
	{
		key = strtok(buf, " =\n");
		val = strtok(NULL, "\n");
		
		if (!key || !val || !*key || *key == '#')
			continue;

		if (!strcmp(key, attr))
		{
			//printf("l1profile key=%s, val=%s\n", key, val);
			snprintf(data, len, "%s", val);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);
	return -1;
}

static int mtk_get_hardware_id_from_l1profile(struct iwinfo_hardware_id *id)
{
	const char *attr = "INDEX0";
	char buf[16] = {0};

	if (mtk_get_l1profile_attr(attr, buf, sizeof(buf)) < 0)
		return -1;
	
	if (!strcmp(buf, "MT7981")) {
		id->vendor_id = 0x14c3;
		id->device_id = 0x7981;
		id->subsystem_vendor_id = id->vendor_id;
		id->subsystem_device_id = id->device_id;
	} else if (!strcmp(buf, "MT7986")) {
		id->vendor_id = 0x14c3;
		id->device_id = 0x7986;
		id->subsystem_vendor_id = id->vendor_id;
		id->subsystem_device_id = id->device_id;
	} else {
		return -1;
	}

	return 0;
}

static int mtk_get_hardware_id(const char *dev, char *buf)
{
	struct iwinfo_hardware_id *id = (struct iwinfo_hardware_id *)buf;
	int ret = 0;

	memset(id, 0, sizeof(*id));

	ret = iwinfo_hardware_id_from_mtd(id);
	if (ret != 0)
		ret = mtk_get_hardware_id_from_l1profile(id);

	return ret;
}

static const struct iwinfo_hardware_entry *
mtk_get_hardware_entry(const char *dev)
{
	struct iwinfo_hardware_id id;

	if (mtk_get_hardware_id(dev, (char *)&id))
		return NULL;

	return iwinfo_hardware(&id);
}

static int mtk_get_hardware_name(const char *dev, char *buf)
{
	const struct iwinfo_hardware_entry *hw;

	if (!(hw = mtk_get_hardware_entry(dev)))
		sprintf(buf, "%s", "MediaTek");
	else
		sprintf(buf, "%s %s", hw->vendor_name, hw->device_name);

	return 0;
}

static int mtk_get_txpower_offset(const char *dev, int *buf)
{
	/* Stub */
	*buf = 0;
	return -1;
}

static int mtk_get_frequency_offset(const char *dev, int *buf)
{
	/* Stub */
	*buf = 0;
	return -1;
}

const struct iwinfo_ops mtk_ops = {
	.name             = "mtk",
	.probe            = mtk_probe,
	.channel          = mtk_get_channel,
	.center_chan1     = mtk_get_center_chan1,
	.center_chan2     = mtk_get_center_chan2,
	.frequency        = mtk_get_frequency,
	.frequency_offset = mtk_get_frequency_offset,
	.txpower          = mtk_get_txpower,
	.txpower_offset   = mtk_get_txpower_offset,
	.bitrate          = mtk_get_bitrate,
	.signal           = mtk_get_signal,
	.noise            = mtk_get_noise,
	.quality          = mtk_get_quality,
	.quality_max      = mtk_get_quality_max,
	.mbssid_support   = mtk_get_mbssid_support,
	.hwmodelist       = mtk_get_hwmodelist,
	.htmodelist       = mtk_get_htmodelist,
	.htmode           = mtk_get_htmode,
	.mode             = mtk_get_mode,
	.ssid             = mtk_get_ssid,
	.bssid            = mtk_get_bssid,
	.country          = mtk_get_country,
	.countrylist      = mtk_get_countrylist,
	.hardware_id      = mtk_get_hardware_id,
	.hardware_name    = mtk_get_hardware_name,
	.encryption       = mtk_get_encryption,
	.phyname          = mtk_get_phyname,
	.assoclist        = mtk_get_assoclist,
	.txpwrlist        = mtk_get_txpwrlist,
	.scanlist         = mtk_get_scanlist,
	.freqlist         = mtk_get_freqlist,
	.close            = mtk_close,
};
