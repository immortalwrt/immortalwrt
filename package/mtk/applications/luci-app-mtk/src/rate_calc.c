#include "mtwifi.h"

#define MAX_NUM_HE_BANDWIDTHS 4
#define MAX_NUM_HE_SPATIAL_STREAMS 4
#define MAX_NUM_HE_MCS_ENTRIES 12

UINT32 cck_to_mcs(UINT32 mcs) {
	UINT32 ret = 0;
	if (mcs == TMI_TX_RATE_CCK_1M_LP)
		ret = 0;
	else if (mcs == TMI_TX_RATE_CCK_2M_LP)
		ret = 1;
	else if (mcs == TMI_TX_RATE_CCK_5M_LP)
		ret = 2;
	else if (mcs == TMI_TX_RATE_CCK_11M_LP)
		ret = 3;
	else if (mcs == TMI_TX_RATE_CCK_2M_SP)
		ret = 1;
	else if (mcs == TMI_TX_RATE_CCK_5M_SP)
		ret = 2;
	else if (mcs == TMI_TX_RATE_CCK_11M_SP)
		ret = 3;

	return ret;
}

static UINT16 he_mcs_phyrate_mapping_table[MAX_NUM_HE_BANDWIDTHS][MAX_NUM_HE_SPATIAL_STREAMS][MAX_NUM_HE_MCS_ENTRIES] = {
	{ /*20 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 8,
			 17,
			 25,
			 34,
			 51,
			 68,
			 77,
			 86,
			 103,
			 114,
			 129,
			 143
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 17,
			 34,
			 51,
			 68,
			 103,
			 137,
			 154,
			 172,
			 206,
			 229,
			 258,
			 286
		},
		/* 3 SS */
		{
			/* DCM 0 */
			 25,
			 51,
			 77,
			 103,
			 154,
			 206,
			 232,
			 258,
			 309,
			 344,
			 387,
			 430
		},
		/* 4 SS */
		{
			/* DCM 0 */
			 34,
			 68,
			 103,
			 137,
			 206,
			 275,
			 309,
			 344,
			 412,
			 458,
			 516,
			 573
		}
	},
	{ /*40 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 17,
			 34,
			 51,
			 68,
			 103,
			 137,
			 154,
			 172,
			 206,
			 229,
			 258,
			 286
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 34,
			 68,
			 103,
			 137,
			 206,
			 275,
			 309,
			 344,
			 412,
			 458,
			 516,
			 573

		},
		/* 3 SS */
		{
			/* DCM 0 */
			 51,
			 103,
			 154,
			 206,
			 309,
			 412,
			 464,
			 516,
			 619,
			 688,
			 774,
			 860

		},
		/* 4 SS */
		{
			/* DCM 0 */
			 68,
			 137,
			 206,
			 275,
			 412,
			 550,
			 619,
			 688,
			 825,
			 917,
			 1032,
			 1147
		}
	},
	{ /*80 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 36,
			 72,
			 108,
			 144,
			 216,
			 288,
			 324,
			 360,
			 432,
			 480,
			 540,
			 600
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 72,
			 144,
			 216,
			 288,
			 432,
			 576,
			 648,
			 720,
			 864,
			 960,
			 1080,
			 1201
		},
		/* 3 SS */
		{
			/* DCM 0 */
			 108,
			 216,
			 324,
			 432,
			 648,
			 864,
			 972,
			 1080,
			 1297,
			 1441,
			 1621,
			 1801
		},
		/* 4 SS */
		{
			/* DCM 0 */
			 144,
			 288,
			 432,
			 576,
			 864,
			 1152,
			 1297,
			 1141,
			 1729,
			 1921,
			 2161,
			 2401
		}
	},
	{ /*160 Mhz*/
		/* 1 SS */
		{
			/* DCM 0*/
			 72,
			 144,
			 216,
			 288,
			 432,
			 576,
			 648,
			 720,
			 864,
			 960,
			 1080,
			 1201
		},
		/* 2 SS */
		{
			/* DCM 0 */
			 144,
			 288,
			 432,
			 576,
			 864,
			 1152,
			 1297,
			 1441,
			 1729,
			 1921,
			 2161,
			 2401
		},
		/* 3 SS */
		{
			/* DCM 0 */
			 216,
			 432,
			 648,
			 864,
			 1297,
			 1729,
			 1945,
			 2161,
			 2594,
			 2882,
			 3242,
			 3602
		},
		/* 4 SS */
		{
			/* DCM 0 */
			 288,
			 576,
			 864,
			 1152,
			 1729,
			 2305,
			 2594,
			 2882,
			 3458,
			 3843,
			 4323,
			 4803
		},
	}

};

void get_rate_he(UINT8 mcs, UINT8 bw, UINT8 nss, UINT8 dcm, ULONG *last_tx_rate)
{
	ULONG value = 0;

	if (nss == 0)
		nss = 1;

	if (mcs >= MAX_NUM_HE_MCS_ENTRIES)
		mcs = MAX_NUM_HE_MCS_ENTRIES - 1;

	if (nss > MAX_NUM_HE_SPATIAL_STREAMS)
		nss = MAX_NUM_HE_SPATIAL_STREAMS;

	if (bw >= MAX_NUM_HE_BANDWIDTHS)
		bw = MAX_NUM_HE_BANDWIDTHS - 1;

	nss--;

	value = he_mcs_phyrate_mapping_table[bw][nss][mcs];
	/*In spec data rate when DCM =1 is half of the data rate when DCM = 0*/
	if (dcm && value)
		value = value / 2 ;

	*last_tx_rate = (ULONG)value;

	return;
}

static INT32 getLegacyOFDMMCSIndex(UINT8 MCS)
{
	INT32 mcs_index = MCS;

	if (MCS == 0xb)
		mcs_index = 0;
	else if (MCS == 0xf)
		mcs_index = 1;
	else if (MCS == 0xa)
		mcs_index = 2;
	else if (MCS == 0xe)
		mcs_index = 3;
	else if (MCS == 0x9)
		mcs_index = 4;
	else if (MCS == 0xd)
		mcs_index = 5;
	else if (MCS == 0x8)
		mcs_index = 6;
	else if (MCS == 0xc)
		mcs_index = 7;

	return mcs_index;
}

static INT MCSMappingRateTable[] = {
	2,  4, 11, 22, 12,  18,  24,  36, 48,  72,  96, 108, 109, 110, 111, 112,/* CCK and OFDM */
	13, 26, 39, 52, 78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260,
	39, 78, 117, 156, 234, 312, 351, 390, /* BW 20, 800ns GI, MCS 0~23 */
	27, 54, 81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540,
	81, 162, 243, 324, 486, 648, 729, 810, /* BW 40, 800ns GI, MCS 0~23 */
	14, 29, 43, 57, 87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288,
	43, 87, 130, 173, 260, 317, 390, 433, /* BW 20, 400ns GI, MCS 0~23 */
	30, 60, 90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600,
	90, 180, 270, 360, 540, 720, 810, 900, /* BW 40, 400ns GI, MCS 0~23 */

	/*for 11ac:20 Mhz 800ns GI*/
	6,  13, 19, 26,  39,  52,  58,  65,  78,  90,     /*1ss mcs 0~8*/
	13, 26, 39, 52,  78,  104, 117, 130, 156, 180,     /*2ss mcs 0~8*/
	19, 39, 58, 78,  117, 156, 175, 195, 234, 260,   /*3ss mcs 0~9*/
	26, 52, 78, 104, 156, 208, 234, 260, 312, 360,     /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 800ns GI*/
	13,	27,	40,	54,	 81,  108, 121, 135, 162, 180,   /*1ss mcs 0~9*/
	27,	54,	81,	108, 162, 216, 243, 270, 324, 360,   /*2ss mcs 0~9*/
	40,	81,	121, 162, 243, 324, 364, 405, 486, 540,  /*3ss mcs 0~9*/
	54,	108, 162, 216, 324, 432, 486, 540, 648, 720, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 800ns GI*/
	29,	58,	87,	117, 175, 234, 263, 292, 351, 390,   /*1ss mcs 0~9*/
	58,	117, 175, 243, 351, 468, 526, 585, 702, 780, /*2ss mcs 0~9*/
	87,	175, 263, 351, 526, 702, 0,	877, 1053, 1170, /*3ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 800ns GI*/
	58,	117, 175, 234, 351, 468, 526, 585, 702, 780, /*1ss mcs 0~9*/
	117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560, /*2ss mcs 0~9*/
	175, 351, 526, 702, 1053, 1404, 1579, 1755, 2160, 0, /*3ss mcs 0~8*/
	234, 468, 702, 936, 1404, 1872, 2106, 2340, 2808, 3120, /*4ss mcs 0~9*/

	/*for 11ac:20 Mhz 400ns GI*/
	7,	14,	21,	28,  43,  57,   65,	 72,  86,  100,    /*1ss mcs 0~8*/
	14,	28,	43,	57,	 86,  115,  130, 144, 173, 200,    /*2ss mcs 0~8*/
	21,	43,	65,	86,	 130, 173,  195, 216, 260, 288,  /*3ss mcs 0~9*/
	28,	57,	86,	115, 173, 231,  260, 288, 346, 400,    /*4ss mcs 0~8*/

	/*for 11ac:40 Mhz 400ns GI*/
	15,	30,	45,	60,	 90,  120,  135, 150, 180, 200,  /*1ss mcs 0~9*/
	30,	60,	90,	120, 180, 240,  270, 300, 360, 400,  /*2ss mcs 0~9*/
	45,	90,	135, 180, 270, 360,  405, 450, 540, 600, /*3ss mcs 0~9*/
	60,	120, 180, 240, 360, 480,  540, 600, 720, 800, /*4ss mcs 0~9*/

	/*for 11ac:80 Mhz 400ns GI*/
	32,	65,	97,	130, 195, 260,  292, 325, 390, 433,  /*1ss mcs 0~9*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*2ss mcs 0~9*/
	97,	195, 292, 390, 585, 780,  0,	 975, 1170, 1300, /*3ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*4ss mcs 0~9*/

	/*for 11ac:160 Mhz 400ns GI*/
	65,	130, 195, 260, 390, 520,  585, 650, 780, 866, /*1ss mcs 0~9*/
	130, 260, 390, 520, 780, 1040,	1170, 1300, 1560, 1733, /*2ss mcs 0~9*/
	195, 390, 585, 780, 1170, 1560,	1755, 1950, 2340, 0, /*3ss mcs 0~8*/
	260, 520, 780, 1040, 1560, 2080,	2340, 2600, 3120, 3466, /*4ss mcs 0~9*/

	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37
}; /* 3*3 */


void getRate(HTTRANSMIT_SETTING_FIX HTSetting, ULONG *fLastTxRxRate)
{
	UINT8					Antenna = 0;
	UINT8					MCS = HTSetting.field.MCS;
	int rate_count = sizeof(MCSMappingRateTable) / sizeof(int);
	int rate_index = 0;
	int value = 0;

	if (HTSetting.field.MODE >= MODE_VHT) {
		MCS = HTSetting.field.MCS & 0xf;
		Antenna = (HTSetting.field.MCS >> 4) + 1;

		if (HTSetting.field.BW == BW_20) {
			rate_index = 112 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_40) {
			rate_index = 152 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_80) {
			rate_index = 192 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		} else if (HTSetting.field.BW == BW_160) {
			rate_index = 232 + ((Antenna - 1) * 10) +
						 ((UCHAR)HTSetting.field.ShortGI * 160) +
						 ((UCHAR)MCS);
		}
	} else {
		if (HTSetting.field.MODE >= MODE_HTMIX) {
			MCS = HTSetting.field.MCS;

			if ((HTSetting.field.MODE == MODE_HTMIX)
				|| (HTSetting.field.MODE == MODE_HTGREENFIELD))
				Antenna = (MCS >> 3) + 1;

			/* map back to 1SS MCS , multiply by antenna numbers later */
			if (MCS > 7)
				MCS %= 8;

			rate_index = 16 + ((UCHAR)HTSetting.field.BW * 24) + ((UCHAR)HTSetting.field.ShortGI * 48) + ((UCHAR)MCS);
		} else {
			if (HTSetting.field.MODE == MODE_OFDM)
				rate_index = getLegacyOFDMMCSIndex(HTSetting.field.MCS) + 4;
			else if (HTSetting.field.MODE == MODE_CCK)
				rate_index = (UCHAR)(HTSetting.field.MCS);
		}
	}

	if (rate_index < 0)
		rate_index = 0;

	if (rate_index >= rate_count)
		rate_index = rate_count - 1;

	if (HTSetting.field.MODE < MODE_VHT)
		value = (MCSMappingRateTable[rate_index] * 5) / 10;
	else
		value =  MCSMappingRateTable[rate_index];

	if (HTSetting.field.MODE >= MODE_HTMIX && HTSetting.field.MODE < MODE_VHT)
		value *= Antenna;

	*fLastTxRxRate = (ULONG)value;
	return;
}
