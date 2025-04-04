/*

*/
#include "rt_config.h"


CH_FREQ_MAP CH_HZ_ID_MAP[] = {
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
	{14, 2484},

	/*  UNII */
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
	{50, 5250},
	{52, 5260},
	{54, 5270},
	{56, 5280},
	{58, 5290},
	{60, 5300},
	{62, 5310},
	{64, 5320},
	{149, 5745},
	{151, 5755},
	{153, 5765},
	{155, 5775},
	{157, 5785},
	{159, 5795},
	{161, 5805},
	{165, 5825},
	{167, 5835},
	{169, 5845},
	{171, 5855},
	{173, 5865},
	{175, 5875},
	{177, 5885},

	/* HiperLAN2 */
	{100, 5500},
	{102, 5510},
	{104, 5520},
	{106, 5530},
	{108, 5540},
	{110, 5550},
	{112, 5560},
	{114, 5570},
	{116, 5580},
	{118, 5590},
	{120, 5600},
	{122, 5610},
	{124, 5620},
	{126, 5630},
	{128, 5640},
	{132, 5660},
	{134, 5670},
	{136, 5680},
	{138, 5690},
	{140, 5700},
	{142, 5710},
	{144, 5720},

	/* Japan MMAC */
	{34, 5170},
	{38, 5190},
	{42, 5210},
	{46, 5230},

	/*  Japan */
	{184, 4920},
	{188, 4940},
	{192, 4960},
	{196, 4980},

	{208, 5040},	/* Japan, means J08 */
	{212, 5060},	/* Japan, means J12 */
	{216, 5080},	/* Japan, means J16 */
};

INT	CH_HZ_ID_MAP_NUM = (sizeof(CH_HZ_ID_MAP) / sizeof(CH_FREQ_MAP));

CH_DESC Country_Region0_ChDesc_2GHZ[] = {
	{1, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_2GHZ[] = {
	{1, 13, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_2GHZ[] = {
	{10, 2, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_2GHZ[] = {
	{10, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_2GHZ[] = {
	{14, 1, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region5_ChDesc_2GHZ[] = {
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_2GHZ[] = {
	{3, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_2GHZ[] = {
	{5, 9, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region31_ChDesc_2GHZ[] = {
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 3, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region32_ChDesc_2GHZ[] = {
	{1, 11, CHANNEL_DEFAULT_PROP},
	{12, 2, CHANNEL_PASSIVE_SCAN},
	{}
};

CH_DESC Country_Region33_ChDesc_2GHZ[] = {
	{1, 14, CHANNEL_DEFAULT_PROP},
	{}
};

COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[] = {
	{REGION_0_BG_BAND, Country_Region0_ChDesc_2GHZ},
	{REGION_1_BG_BAND, Country_Region1_ChDesc_2GHZ},
	{REGION_2_BG_BAND, Country_Region2_ChDesc_2GHZ},
	{REGION_3_BG_BAND, Country_Region3_ChDesc_2GHZ},
	{REGION_4_BG_BAND, Country_Region4_ChDesc_2GHZ},
	{REGION_5_BG_BAND, Country_Region5_ChDesc_2GHZ},
	{REGION_6_BG_BAND, Country_Region6_ChDesc_2GHZ},
	{REGION_7_BG_BAND, Country_Region7_ChDesc_2GHZ},
	{REGION_31_BG_BAND, Country_Region31_ChDesc_2GHZ},
	{REGION_32_BG_BAND, Country_Region32_ChDesc_2GHZ},
	{REGION_33_BG_BAND, Country_Region33_ChDesc_2GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_2GHZ = sizeof(Country_Region_ChDesc_2GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_DESC Country_Region0_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_5GHZ[] = {
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_5GHZ[] = {
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};
CH_DESC Country_Region5_ChDesc_5GHZ[] = {
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_5GHZ[] = {
	{36, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	/* {100, 14, CHANNEL_DEFAULT_PROP}, */
	/* {149, 7, CHANNEL_DEFAULT_PROP}, */
	{}
};

CH_DESC Country_Region8_ChDesc_5GHZ[] = {
	{52, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region9_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region10_ChDesc_5GHZ[] = {
	{36, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region11_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 6, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

/* for FCC capable of using 144 , mapping of Country_Region1 */
CH_DESC Country_Region12_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{}
};
/* for FCC capable of using 144 , mapping of Country_Region7 */
CH_DESC Country_Region13_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};
/* for FCC capable of using 144 , mapping of Country_Region9 */
CH_DESC Country_Region14_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region15_ChDesc_5GHZ[] = {
	{149, 7, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region16_ChDesc_5GHZ[] = {
	{52, 4, CHANNEL_DEFAULT_PROP},
	{149, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region17_ChDesc_5GHZ[] = {
	{36, 4, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region18_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region19_ChDesc_5GHZ[] = {
	{56, 3, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region20_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 7, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region21_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 11, CHANNEL_DEFAULT_PROP},
	{149, 4, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region22_ChDesc_5GHZ[] = {
	{100, 11, CHANNEL_DEFAULT_PROP},
	{}
};

/* for JP capable of using 144 , mapping of Country_Region18 */
CH_DESC Country_Region23_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 4, CHANNEL_DEFAULT_PROP},
	{}
};

/* for JP capable of using 144 , mapping of Country_Region22 */
CH_DESC Country_Region24_ChDesc_5GHZ[] = {
	{100, 12, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region25_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 5, CHANNEL_DEFAULT_PROP},
	{132, 3, CHANNEL_DEFAULT_PROP},
	{149, 8, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region26_ChDesc_5GHZ[] = {
	{36, 8, CHANNEL_DEFAULT_PROP},
	{100, 12, CHANNEL_DEFAULT_PROP},
	{149, 8, CHANNEL_DEFAULT_PROP},
	{}
};


COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[] = {
	{REGION_0_A_BAND, Country_Region0_ChDesc_5GHZ},
	{REGION_1_A_BAND, Country_Region1_ChDesc_5GHZ},
	{REGION_2_A_BAND, Country_Region2_ChDesc_5GHZ},
	{REGION_3_A_BAND, Country_Region3_ChDesc_5GHZ},
	{REGION_4_A_BAND, Country_Region4_ChDesc_5GHZ},
	{REGION_5_A_BAND, Country_Region5_ChDesc_5GHZ},
	{REGION_6_A_BAND, Country_Region6_ChDesc_5GHZ},
	{REGION_7_A_BAND, Country_Region7_ChDesc_5GHZ},
	{REGION_8_A_BAND, Country_Region8_ChDesc_5GHZ},
	{REGION_9_A_BAND, Country_Region9_ChDesc_5GHZ},
	{REGION_10_A_BAND, Country_Region10_ChDesc_5GHZ},
	{REGION_11_A_BAND, Country_Region11_ChDesc_5GHZ},
	{REGION_12_A_BAND, Country_Region12_ChDesc_5GHZ},
	{REGION_13_A_BAND, Country_Region13_ChDesc_5GHZ},
	{REGION_14_A_BAND, Country_Region14_ChDesc_5GHZ},
	{REGION_15_A_BAND, Country_Region15_ChDesc_5GHZ},
	{REGION_16_A_BAND, Country_Region16_ChDesc_5GHZ},
	{REGION_17_A_BAND, Country_Region17_ChDesc_5GHZ},
	{REGION_18_A_BAND, Country_Region18_ChDesc_5GHZ},
	{REGION_19_A_BAND, Country_Region19_ChDesc_5GHZ},
	{REGION_20_A_BAND, Country_Region20_ChDesc_5GHZ},
	{REGION_21_A_BAND, Country_Region21_ChDesc_5GHZ},
	{REGION_22_A_BAND, Country_Region22_ChDesc_5GHZ},
	{REGION_23_A_BAND, Country_Region23_ChDesc_5GHZ},
	{REGION_24_A_BAND, Country_Region24_ChDesc_5GHZ},
	{REGION_25_A_BAND, Country_Region25_ChDesc_5GHZ},
	{REGION_26_A_BAND, Country_Region26_ChDesc_5GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_5GHZ = sizeof(Country_Region_ChDesc_5GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_DESC Country_Region0_ChDesc_6GHZ[] = {
	{1, 25, CHANNEL_DEFAULT_PROP},
	{101, 5, CHANNEL_DEFAULT_PROP},
	{121, 17, CHANNEL_DEFAULT_PROP},
	{189, 12, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region1_ChDesc_6GHZ[] = {
	{1, 25, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region2_ChDesc_6GHZ[] = {
	{101, 5, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region3_ChDesc_6GHZ[] = {
	{121, 17, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region4_ChDesc_6GHZ[] = {
	{189, 12, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region5_ChDesc_6GHZ[] = {
	{1, 25, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region6_ChDesc_6GHZ[] = {
	{1, 25, CHANNEL_DEFAULT_PROP},
	{}
};

CH_DESC Country_Region7_ChDesc_6GHZ[] = {
	{1, 25, CHANNEL_DEFAULT_PROP},
	{101, 3, CHANNEL_DEFAULT_PROP},
	{}
};


COUNTRY_REGION_CH_DESC Country_Region_ChDesc_6GHZ[] = {
	{REGION_0_A_BAND_6GHZ, Country_Region0_ChDesc_6GHZ},
	{REGION_1_A_BAND_6GHZ, Country_Region1_ChDesc_6GHZ},
	{REGION_2_A_BAND_6GHZ, Country_Region2_ChDesc_6GHZ},
	{REGION_3_A_BAND_6GHZ, Country_Region3_ChDesc_6GHZ},
	{REGION_4_A_BAND_6GHZ, Country_Region4_ChDesc_6GHZ},
	{REGION_5_A_BAND_6GHZ, Country_Region5_ChDesc_6GHZ},
	{REGION_6_A_BAND_6GHZ, Country_Region6_ChDesc_6GHZ},
	{REGION_7_A_BAND_6GHZ, Country_Region7_ChDesc_6GHZ},
	{}
};

UINT16 const Country_Region_GroupNum_6GHZ = sizeof(Country_Region_ChDesc_6GHZ) / sizeof(COUNTRY_REGION_CH_DESC);

CH_GROUP_DESC Channel_GRP[] = {
	{36, 4},
	{52, 4},
	{100, 12},
	{149, 5},
	{}
};

UCHAR const Channel_GRP_Num = sizeof(Channel_GRP) / sizeof(CH_GROUP_DESC);

UINT16 TotalChNum(PCH_DESC pChDesc)
{
	UINT16 TotalChNum = 0;

	while (pChDesc->FirstChannel) {
		TotalChNum += pChDesc->NumOfCh;
		pChDesc++;
	}

	return TotalChNum;
}

UCHAR GetChannel_5GHZ(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel) {
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index * 4;
		else {
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannel_2GHZ(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel) {
		if (index < pChDesc->NumOfCh)
			return pChDesc->FirstChannel + index;
		else {
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}

UCHAR GetChannelFlag(PCH_DESC pChDesc, UCHAR index)
{
	while (pChDesc->FirstChannel) {
		if (index < pChDesc->NumOfCh)
			return pChDesc->ChannelProp;
		else {
			index -= pChDesc->NumOfCh;
			pChDesc++;
		}
	}

	return 0;
}


/*Albania*/
CH_DESP Country_AL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Algeria*/
CH_DESP Country_DZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Argentina*/
CH_DESP Country_AR_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Armenia*/
CH_DESP Country_AM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Aruba*/
CH_DESP Country_AW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Australia*/
CH_DESP Country_AU_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 23, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Austria*/
CH_DESP Country_AT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Azerbaijan*/
CH_DESP Country_AZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bahamas*/
CH_DESP Country_BS_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 17, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 17, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bahrain*/
CH_DESP Country_BH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 33, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bangladesh*/
CH_DESP Country_BD_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 33, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Barbados*/
CH_DESP Country_BB_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 28, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Belarus*/
CH_DESP Country_BY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Belgium*/
CH_DESP Country_BE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Belize*/
CH_DESP Country_BZ_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Benin*/
CH_DESP Country_BJ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bhutan*/
CH_DESP Country_BT_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bolivia*/
CH_DESP Country_BO_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bosnia and Herzegovina*/
CH_DESP Country_BA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Botswana*/
CH_DESP Country_BW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Brazil*/
CH_DESP Country_BR_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Brunei Darussalam*/
CH_DESP Country_BN_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Bulgaria*/
CH_DESP Country_BG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Burkina Faso*/
CH_DESP Country_BF_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Burundi*/
CH_DESP Country_BI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Cambodia*/
CH_DESP Country_KH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Cameroon*/
CH_DESP Country_CM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Canada*/
CH_DESP Country_CA_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 24, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Central African Republic*/
CH_DESP Country_CF_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Chad*/
CH_DESP Country_TD_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Chile*/
CH_DESP Country_CL_ChDesp[] = {
	{ 1,   13, 21, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 21, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 21, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 21, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*China*/
CH_DESP Country_CN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  5, 33, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Colombia*/
CH_DESP Country_CO_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Comoros*/
CH_DESP Country_KM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Costa Rica*/
CH_DESP Country_CR_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Croatia*/
CH_DESP Country_HR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Cyprus*/
CH_DESP Country_CY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Czech Republic*/
CH_DESP Country_CZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2400~2483.5MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5470~5725MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Denmark*/
CH_DESP Country_DK_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Dominica*/
CH_DESP Country_DM_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Dominican Republic*/
CH_DESP Country_DO_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ecuador*/
CH_DESP Country_EC_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Egypt*/
CH_DESP Country_EG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*El Salvador*/
CH_DESP Country_SV_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Equatorial Guinea*/
CH_DESP Country_GQ_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Eritrea*/
CH_DESP Country_ER_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Estonia*/
CH_DESP Country_EE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ethiopia*/
CH_DESP Country_ET_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Fiji*/
CH_DESP Country_FJ_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Finland*/
CH_DESP Country_FI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Faroe Islands (Denmark)*/
CH_DESP Country_FO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*France*/
CH_DESP Country_FR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Gabon*/
CH_DESP Country_GA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Gambia*/
CH_DESP Country_GM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Georgia*/
CH_DESP Country_GE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Germany*/
CH_DESP Country_DE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2400~2483.5MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5470~5725MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ghana*/
CH_DESP Country_GH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Greece*/
CH_DESP Country_GR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Greenland*/
CH_DESP Country_GL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 20 */
	{ 0},			/* end*/
};
/*Grenada*/
CH_DESP Country_GD_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Guam*/
CH_DESP Country_GU_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Guatemala*/
CH_DESP Country_GT_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Guyana*/
CH_DESP Country_GY_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Haiti*/
CH_DESP Country_HT_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2462MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,	5, 23, BOTH, TRUE}, 	/*5490~5600MHz, Ch 100~116, Max BW: 40 */
	{ 132,	4, 23, BOTH, TRUE}, 	/*5650~5710MHz, Ch 132~144, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Honduras*/
CH_DESP Country_HN_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Hong Kong*/
CH_DESP Country_HK_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Hungary*/
CH_DESP Country_HU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,  Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Iceland*/
CH_DESP Country_IS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*India*/
CH_DESP Country_IN_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 144,  1, 23, BOTH, TRUE},	/*5710~5730MHz, Ch 144~144, Max BW: 40 */
	{ 149,  5, 23, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Indonesia*/
CH_DESP Country_ID_ChDesp[] = {
	{ 1,   13, 26, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Iran, Islamic Republic of*/
CH_DESP Country_IR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ireland*/
CH_DESP Country_IE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Israel*/
CH_DESP Country_IL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5150~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5350MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Italy*/
CH_DESP Country_IT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Jamaica*/
CH_DESP Country_JM_ChDesp[] = {
	{ 1,   11, 20, BOTH, FALSE},	/*2402~2462MHz, Ch 1~11,   Max BW: 40 */
	{ 149,  5, 28, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Japan*/
CH_DESP Country_JP_ChDesp[] = {
	{ 1,    14, 23, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 20 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  12, 23, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 0},			/* end*/
};
/*Jordan*/
CH_DESP Country_JO_ChDesp[] = {
	{ 1,  13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,  4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 0},			/* end*/
};
/*Kazakhstan*/
CH_DESP Country_KZ_ChDesp[] = {
	{ 1,  13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 0},			/* end*/
};
/*Kenya*/
CH_DESP Country_KE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE}, 	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE}, 	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Korea, Democratic People's Republic of*/
CH_DESP Country_KP_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, TRUE},	/*5160~5250MHz, Ch 36~48, Max BW: 40 */
	{ 36,   8, 20, BOTH, FALSE},	/*5170~5330MHz, Ch 36~64, Max BW: 40 */
	{ 100,  7, 30, BOTH, TRUE},	/*5490~5630MHz, Ch 100~124, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Korea, Republic of*/
CH_DESP Country_KR_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 20 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 20 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 20 */
	{ 100, 12, 23, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Kuwait*/
CH_DESP Country_KW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Kyrgyzstan*/
CH_DESP Country_KG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Latvia*/
CH_DESP Country_LV_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Lebanon*/
CH_DESP Country_LB_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 23, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Lesotho*/
CH_DESP Country_LS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Liberia*/
CH_DESP Country_LR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Libya*/
CH_DESP Country_LY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Liechtenstein*/
CH_DESP Country_LI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Lithuania*/
CH_DESP Country_LT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Luxembourg*/
CH_DESP Country_LU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Macao*/
CH_DESP Country_MO_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Macedonia, Republic of*/
CH_DESP Country_MK_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Madagascar*/
CH_DESP Country_MG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Malawi*/
CH_DESP Country_MW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Malaysia*/
CH_DESP Country_MY_ChDesp[] = {
	{ 1,   13, 26, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 30, BOTH, TRUE},	/*5490~5650MHz, Ch 100~128, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Maldives*/
CH_DESP Country_MV_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 20, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mali*/
CH_DESP Country_ML_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Malta*/
CH_DESP Country_MT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Marshall Islands*/
CH_DESP Country_MH_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mauritania*/
CH_DESP Country_MR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mauritius*/
CH_DESP Country_MU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mexico*/
CH_DESP Country_MX_ChDesp[] = {
	{ 1,   11, 33, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  5, 23, BOTH, TRUE},	/*5490~5590MHz, Ch 100~116, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Monaco*/
CH_DESP Country_MC_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Montenegro*/
CH_DESP Country_ME_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Morocco*/
CH_DESP Country_MA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Mozambique*/
CH_DESP Country_MZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nauru*/
CH_DESP Country_NR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nepal*/
CH_DESP Country_NP_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Netherlands*/
CH_DESP Country_NL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Netherlands Antilles*/
CH_DESP Country_AN_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*New Zealand*/
CH_DESP Country_NZ_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 36, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nicaragua*/
CH_DESP Country_NI_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Niger*/
CH_DESP Country_NE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Nigeria*/
CH_DESP Country_NG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Norway*/
CH_DESP Country_NO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Oman*/
CH_DESP Country_OM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 33, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Pakistan*/
CH_DESP Country_PK_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};

/*Palau*/
CH_DESP Country_PW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Panama*/
CH_DESP Country_PA_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 16, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 16, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Papua New Guinea*/
CH_DESP Country_PG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Paraguay*/
CH_DESP Country_PY_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Peru*/
CH_DESP Country_PE_ChDesp[] = {
	{ 1,   11, 26, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100,  8, 20, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Philippines*/
CH_DESP Country_PH_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 23, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Poland*/
CH_DESP Country_PL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Portuga*/
CH_DESP Country_PT_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Puerto Rico*/
CH_DESP Country_PR_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Qatar*/
CH_DESP Country_QA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Republic of the Congo*/
CH_DESP Country_CG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Romania*/
CH_DESP Country_RO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Russian Federation*/
CH_DESP Country_RU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 132,  4, 30, BOTH, TRUE},	/*5650~5710MHz, Ch 132~144, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 20 */
	{ 0},			/* end*/
};
/*Rwanda*/
CH_DESP Country_RW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Barth'elemy*/
CH_DESP Country_BL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 18, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 18, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Kitts and Nevis*/
CH_DESP Country_KN_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Lucia*/
CH_DESP Country_LC_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saint Vincent and the Grenadines*/
CH_DESP Country_VC_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Samoa*/
CH_DESP Country_WS_ChDesp[] = {
	{ 1,   13, 36, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 149,  4, 36, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*San Marino*/
CH_DESP Country_SM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Saudi Arabia*/
CH_DESP Country_SA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 13, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Senegal*/
CH_DESP Country_SN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Serbia*/
CH_DESP Country_RS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Seychelles*/
CH_DESP Country_SC_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sierra Leone*/
CH_DESP Country_SL_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Singapore*/
CH_DESP Country_SG_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Slovakia*/
CH_DESP Country_SK_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Slovenia*/
CH_DESP Country_SI_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Solomon Islands*/
CH_DESP Country_SB_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Somalia*/
CH_DESP Country_SO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*South Africa*/
CH_DESP Country_ZA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 13, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*South Sudan*/
CH_DESP Country_SS_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Spain*/
CH_DESP Country_ES_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sri Lanka*/
CH_DESP Country_LK_ChDesp[] = {
	{ 1,   13, 23, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 23, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sudan*/
CH_DESP Country_SD_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Swaziland*/
CH_DESP Country_SZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Sweden*/
CH_DESP Country_SE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Switzerland*/
CH_DESP Country_CH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Syrian Arab Republic*/
CH_DESP Country_SY_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Taiwan*/
CH_DESP Country_TW_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 24, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 24, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tajikistan*/
CH_DESP Country_TJ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Thailand*/
CH_DESP Country_TH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Togo*/
CH_DESP Country_TG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tonga*/
CH_DESP Country_TO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Trinidad and Tobago*/
CH_DESP Country_TT_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tunisia*/
CH_DESP Country_TN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Turkey*/
CH_DESP Country_TR_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Turkmenistan*/
CH_DESP Country_TM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Tuvalu*/
CH_DESP Country_TV_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Uganda*/
CH_DESP Country_UG_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Ukraine*/
CH_DESP Country_UA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 20, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 20, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 20, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 20, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*United Arab Emirates*/
CH_DESP Country_AE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*United Kingdom*/
CH_DESP Country_GB_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, IDOR, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, IDOR, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*United States*/
CH_DESP Country_US_ChDesp[] = {
	{ 1,   11, 36, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 36, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  8, 30, BOTH, FALSE},	/*5735~5885MHz, Ch 149~177, Max BW: 40 */
	{ 0},			/* end*/
};
/*Uruguay*/
CH_DESP Country_UY_ChDesp[] = {
	{ 1,   11, 30, BOTH, FALSE},	/*2402~2472MHz, Ch 1~11,   Max BW: 40 */
	{ 36,   4, 30, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 30, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 24, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 149,  5, 30, BOTH, FALSE},	/*5735~5835MHz, Ch 149~165, Max BW: 40 */
	{ 0},			/* end*/
};
/*Uzbekistan*/
CH_DESP Country_UZ_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Vanuatu*/
CH_DESP Country_VU_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 23, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Vatican City*/
CH_DESP Country_VA_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Venezuela*/
CH_DESP Country_VE_ChDesp[] = {
	{ 1,   13, 30, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Viet Nam*/
CH_DESP Country_VN_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 12, 30, BOTH, TRUE},	/*5490~5730MHz, Ch 100~144, Max BW: 40 */
	{ 149,  4, 30, BOTH, FALSE},	/*5735~5815MHz, Ch 149~161, Max BW: 40 */
	{ 0},			/* end*/
};
/*Western Sahara*/
CH_DESP Country_EH_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Yemen*/
CH_DESP Country_YE_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Zambia*/
CH_DESP Country_ZM_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};
/*Zimbabwe*/
CH_DESP Country_ZW_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/*2402~2482MHz, Ch 1~13,   Max BW: 40 */
	{ 36,   4, 23, BOTH, FALSE},	/*5170~5250MHz, Ch 36~48, Max BW: 40 */
	{ 52,   4, 23, BOTH, TRUE},	/*5250~5330MHz, Ch 52~64, Max BW: 40 */
	{ 100, 11, 30, BOTH, TRUE},	/*5490~5710MHz, Ch 100~140, Max BW: 40 */
	{ 0},			/* end*/
};

/* Group Region */
/*Europe*/
CH_DESP Country_EU_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13 */
	{ 36,   4, 17, BOTH, FALSE},	/* 5G band 1, ch 36~48*/
	{ 0},			/* end*/
};
/*North America*/
CH_DESP Country_NA_ChDesp[] = {
	{ 1,   11,	20, BOTH, FALSE},	/* 2.4 G, ch 1~11*/
	{ 36,   4,	16, IDOR, FALSE},	/* 5G band 1, ch 36~48*/
	{ 149,	5, 30, BOTH, FALSE},	/* 5G band 4, ch 149~165*/
	{ 0},			/* end*/
};
/*World Wide*/
CH_DESP Country_WO_ChDesp[] = {
	{ 1,   13, 20, BOTH, FALSE},	/* 2.4 G, ch 1~13*/
	{ 36,   4, 30, BOTH, FALSE},	/* 5G band 1, ch 36~48 */
	{ 149,	5, 30, BOTH, FALSE},	/* 5G band 4, ch 149~165 */
	{ 0},			/* end*/
};

CH_REGION ChRegion[] = {
	{"AL", CE, TRUE, Country_AL_ChDesp}, /* Albania */
	{"DZ", CE, TRUE, Country_DZ_ChDesp}, /* Algeria */
	{"AR", CE, TRUE, Country_AR_ChDesp}, /* Argentina */
	{"AM", CE, TRUE, Country_AM_ChDesp}, /* Armenia */
	{"AW", CE, TRUE, Country_AW_ChDesp}, /* Aruba */
	{"AU", CE, FALSE, Country_AU_ChDesp}, /* Australia */
	{"AT", CE, TRUE, Country_AT_ChDesp}, /* Austria */
	{"AZ", CE, TRUE, Country_AZ_ChDesp}, /* Azerbaijan */
	{"BS", CE, TRUE, Country_BS_ChDesp}, /* Bahamas */
	{"BH", CE, TRUE, Country_BH_ChDesp}, /* Bahrain */
	{"BD", CE, TRUE, Country_BD_ChDesp}, /* Bangladesh */
	{"BB", CE, TRUE, Country_BB_ChDesp}, /* Barbados */
	{"BY", CE, TRUE, Country_BY_ChDesp}, /* Belarus */
	{"BE", CE, TRUE, Country_BE_ChDesp}, /* Belgium */
	{"BZ", CE, TRUE, Country_BZ_ChDesp}, /* Belize */
	{"BJ", CE, TRUE, Country_BJ_ChDesp}, /* Benin */
	{"BT", CE, TRUE, Country_BT_ChDesp}, /* Bhutan */
	{"BO", CE, TRUE, Country_BO_ChDesp}, /* Bolivia */
	{"BA", CE, TRUE, Country_BA_ChDesp}, /* Bosnia and Herzegovina */
	{"BW", CE, TRUE, Country_BW_ChDesp}, /* Botswana */
	{"BR", CE, FALSE, Country_BR_ChDesp}, /* Brazil */
	{"BN", CE, TRUE, Country_BN_ChDesp}, /* Brunei Darussalam */
	{"BG", CE, TRUE, Country_BG_ChDesp}, /* Bulgaria */
	{"BF", CE, TRUE, Country_BF_ChDesp}, /* Burkina Faso */
	{"BI", CE, TRUE, Country_BI_ChDesp}, /* Burundi */
	{"KH", CE, TRUE, Country_KH_ChDesp}, /* Cambodia */
	{"CM", CE, TRUE, Country_CM_ChDesp}, /* Cameroon */
	{"CA", FCC, FALSE, Country_CA_ChDesp}, /* Canada */
	{"CF", FCC, FALSE, Country_CF_ChDesp}, /* Central African Republic */
	{"TD", FCC, FALSE, Country_TD_ChDesp}, /* Chad */
	{"CL", CE, TRUE, Country_CL_ChDesp}, /* Chile */
	{"CN", CHN, FALSE, Country_CN_ChDesp}, /* China */
	{"CO", CE, TRUE, Country_CO_ChDesp}, /* Colombia */
	{"KM", CE, TRUE, Country_KM_ChDesp}, /* Comoros */
	{"CR", CE, TRUE, Country_CR_ChDesp}, /* Costa Rica */
	{"HR", CE, TRUE, Country_HR_ChDesp}, /* Croatia */
	{"CY", CE, TRUE, Country_CY_ChDesp}, /* Cyprus */
	{"CZ", CE, TRUE, Country_CZ_ChDesp}, /* Czech Republic */
	{"DK", CE, TRUE, Country_DK_ChDesp}, /* Denmark */
	{"DM", CE, TRUE, Country_DM_ChDesp}, /* Dominica */
	{"DO", CE, TRUE, Country_DO_ChDesp}, /* Dominican Republic */
	{"EC", CE, TRUE, Country_EC_ChDesp}, /* Ecuador */
	{"EG", CE, TRUE, Country_EG_ChDesp}, /* Egypt */
	{"SV", CE, TRUE, Country_SV_ChDesp}, /* El Salvador */
	{"GQ", CE, TRUE, Country_GQ_ChDesp}, /* Equatorial Guinea */
	{"ER", CE, TRUE, Country_ER_ChDesp}, /* Eritrea */
	{"EE", CE, TRUE, Country_EE_ChDesp}, /* Estonia */
	{"ET", CE, TRUE, Country_ET_ChDesp}, /* Ethiopia */
	{"FJ", CE, TRUE, Country_FJ_ChDesp}, /* Fiji */
	{"FI", CE, TRUE, Country_FI_ChDesp}, /* Finland */
	{"FO", CE, TRUE, Country_FO_ChDesp}, /* Faroe Islands (Denmark) */
	{"FR", CE, TRUE, Country_FR_ChDesp}, /* France */
	{"GA", CE, TRUE, Country_GA_ChDesp}, /* Gabon */
	{"GM", CE, TRUE, Country_GM_ChDesp}, /* Gambia */
	{"GE", CE, TRUE, Country_GE_ChDesp}, /* Georgia */
	{"DE", CE, TRUE, Country_DE_ChDesp}, /* Germany */
	{"GH", CE, TRUE, Country_GH_ChDesp}, /* Ghana */
	{"GR", CE, TRUE, Country_GR_ChDesp}, /* Greece */
	{"GL", CE, TRUE, Country_GL_ChDesp}, /* Greenland */
	{"GD", CE, TRUE, Country_GD_ChDesp}, /* Grenada */
	{"GU", CE, TRUE, Country_GU_ChDesp}, /* Guam */
	{"GT", CE, TRUE, Country_GT_ChDesp}, /* Guatemala */
	{"GY", CE, TRUE, Country_GY_ChDesp}, /* Guyana */
	{"HT", CE, TRUE, Country_HT_ChDesp}, /* Haiti */
	{"HN", CE, TRUE, Country_HN_ChDesp}, /* Honduras */
	{"HK", CE, TRUE, Country_HK_ChDesp}, /* Hong Kong */
	{"HU", CE, TRUE, Country_HU_ChDesp}, /* Hungary */
	{"IS", CE, TRUE, Country_IS_ChDesp}, /* Iceland */
	{"IN", CE, TRUE, Country_IN_ChDesp}, /* India */
	{"ID", CE, TRUE, Country_ID_ChDesp}, /* Indonesia */
	{"IR", CE, TRUE, Country_IR_ChDesp}, /* Iran, Islamic Republic of */
	{"IE", CE, TRUE, Country_IE_ChDesp}, /* Ireland */
	{"IL", CE, FALSE, Country_IL_ChDesp}, /* Israel */
	{"IT", CE, TRUE, Country_IT_ChDesp}, /* Italy */
	{"JM", CE, TRUE, Country_JM_ChDesp}, /* Jamaica */
	{"JP", JAP, TRUE, Country_JP_ChDesp}, /* Japan */
	{"JO", CE, TRUE, Country_JO_ChDesp}, /* Jordan */
	{"KZ", CE, TRUE, Country_KZ_ChDesp}, /* Kazakhstan */
	{"KE", CE, TRUE, Country_KE_ChDesp}, /* Kenya */
	{"KP", CE, TRUE, Country_KP_ChDesp}, /* Korea, Democratic People's Republic of */
	{"KR", CE, FALSE, Country_KR_ChDesp}, /* Korea, Republic of */
	{"KW", CE, TRUE, Country_KW_ChDesp}, /* Kuwait */
	{"KG", CE, TRUE, Country_KG_ChDesp}, /* Kyrgyzstan */
	{"LV", CE, TRUE, Country_LV_ChDesp}, /* Latvia */
	{"LB", CE, TRUE, Country_LB_ChDesp}, /* Lebanon */
	{"LS", CE, TRUE, Country_LS_ChDesp}, /* Lesotho */
	{"LR", CE, TRUE, Country_LR_ChDesp}, /* Liberia */
	{"LY", CE, TRUE, Country_LY_ChDesp}, /* Libya */
	{"LI", CE, TRUE, Country_LI_ChDesp}, /* Liechtenstein */
	{"LT", CE, TRUE, Country_LT_ChDesp}, /* Lithuania */
	{"LU", CE, TRUE, Country_LU_ChDesp}, /* Luxembourg */
	{"MO", CE, TRUE, Country_MO_ChDesp}, /* Macao */
	{"MK", CE, TRUE, Country_MK_ChDesp}, /* Macedonia, Republic of */
	{"MG", CE, TRUE, Country_MG_ChDesp}, /* Madagascar */
	{"MW", CE, TRUE, Country_MW_ChDesp}, /* Malawi */
	{"MY", CE, TRUE, Country_MY_ChDesp}, /* Malaysia */
	{"MV", CE, TRUE, Country_MV_ChDesp}, /* Maldives */
	{"ML", CE, TRUE, Country_ML_ChDesp}, /* Mali */
	{"MT", CE, TRUE, Country_MT_ChDesp}, /* Malta */
	{"MH", CE, TRUE, Country_MH_ChDesp}, /* Marshall Islands */
	{"MR", CE, TRUE, Country_MR_ChDesp}, /* Mauritania */
	{"MU", CE, TRUE, Country_MU_ChDesp}, /* Mauritius */
	{"MX", CE, FALSE, Country_MX_ChDesp}, /* Mexico */
	{"MC", CE, TRUE, Country_MC_ChDesp}, /* Monaco */
	{"ME", CE, TRUE, Country_ME_ChDesp}, /* Montenegro */
	{"MA", CE, TRUE, Country_MA_ChDesp}, /* Morocco */
	{"MZ", CE, TRUE, Country_MZ_ChDesp}, /* Mozambique */
	{"NR", CE, TRUE, Country_NR_ChDesp}, /* Nauru */
	{"NP", CE, TRUE, Country_NP_ChDesp}, /* Nepal */
	{"NL", CE, TRUE, Country_NL_ChDesp}, /* Netherlands */
	{"AN", CE, TRUE, Country_AN_ChDesp}, /* Netherlands Antilles */
	{"NZ", CE, TRUE, Country_NZ_ChDesp}, /* New Zealand */
	{"NI", CE, TRUE, Country_NI_ChDesp}, /* Nicaragua */
	{"NE", CE, TRUE, Country_NE_ChDesp}, /* Niger */
	{"NG", CE, TRUE, Country_NG_ChDesp}, /* Nigeria */
	{"NO", CE, TRUE, Country_NO_ChDesp}, /* Norway */
	{"OM", CE, TRUE, Country_OM_ChDesp}, /* Oman */
	{"PK", CE, TRUE, Country_PK_ChDesp}, /* Pakistan */
	{"PW", CE, TRUE, Country_PW_ChDesp}, /* Palau */
	{"PA", CE, TRUE, Country_PA_ChDesp}, /* Panama */
	{"PG", CE, TRUE, Country_PG_ChDesp}, /* Papua New Guinea */
	{"PY", CE, TRUE, Country_PY_ChDesp}, /* Paraguay */
	{"PE", CE, TRUE, Country_PE_ChDesp}, /* Peru */
	{"PH", CE, TRUE, Country_PH_ChDesp}, /* Philippines */
	{"PL", CE, TRUE, Country_PL_ChDesp}, /* Poland */
	{"PT", CE, TRUE, Country_PT_ChDesp}, /* Portuga l*/
	{"PR", CE, TRUE, Country_PR_ChDesp}, /* Puerto Rico */
	{"QA", CE, TRUE, Country_QA_ChDesp}, /* Qatar */
	{"CG", CE, TRUE, Country_CG_ChDesp}, /* Republic of the Congo */
	{"RO", CE, TRUE, Country_RO_ChDesp}, /* Romania */
	{"RU", CE, FALSE, Country_RU_ChDesp}, /* Russian Federation */
	{"RW", CE, TRUE, Country_RW_ChDesp}, /* Rwanda */
	{"BL", CE, TRUE, Country_BL_ChDesp}, /* Saint Barth'elemy */
	{"KN", CE, TRUE, Country_KN_ChDesp}, /* Saint Kitts and Nevis */
	{"LC", CE, TRUE, Country_LC_ChDesp}, /* Saint Lucia */
	{"VC", CE, TRUE, Country_VC_ChDesp}, /* Saint Vincent and the Grenadines */
	{"WS", CE, TRUE, Country_WS_ChDesp}, /* Samoa */
	{"SM", CE, TRUE, Country_SM_ChDesp}, /* San Marino */
	{"SA", CE, TRUE, Country_SA_ChDesp}, /* Saudi Arabia */
	{"SN", CE, TRUE, Country_SN_ChDesp}, /* Senegal */
	{"RS", CE, TRUE, Country_RS_ChDesp}, /* Serbia */
	{"SC", CE, TRUE, Country_SC_ChDesp}, /* Seychelles */
	{"SL", CE, TRUE, Country_SL_ChDesp}, /* Sierra Leone */
	{"SG", CE, TRUE, Country_SG_ChDesp}, /* Singapore */
	{"SK", CE, TRUE, Country_SK_ChDesp}, /* Slovakia */
	{"SI", CE, TRUE, Country_SI_ChDesp}, /* Slovenia */
	{"SB", CE, TRUE, Country_SB_ChDesp}, /* Solomon Islands */
	{"SO", CE, TRUE, Country_SO_ChDesp}, /* Somalia */
	{"ZA", CE, FALSE, Country_ZA_ChDesp}, /* South Africa */
	{"SS", CE, TRUE, Country_SS_ChDesp}, /* South Sudan */
	{"ES", CE, TRUE, Country_ES_ChDesp}, /* Spain */
	{"LK", CE, TRUE, Country_LK_ChDesp}, /* Sri Lanka */
	{"SD", CE, TRUE, Country_SD_ChDesp}, /* Sudan */
	{"SZ", CE, TRUE, Country_SZ_ChDesp}, /* Swaziland */
	{"SE", CE, TRUE, Country_SE_ChDesp}, /* Sweden */
	{"CH", CE, TRUE, Country_CH_ChDesp}, /* Switzerland */
	{"SY", CE, TRUE, Country_SY_ChDesp}, /* Syrian Arab Republic */
	{"TW", FCC, FALSE, Country_TW_ChDesp}, /* Taiwan */
	{"TJ", CE, TRUE, Country_TJ_ChDesp}, /* Tajikistan */
	{"TH", CE, FALSE, Country_TH_ChDesp}, /* Thailand */
	{"TG", CE, TRUE, Country_TG_ChDesp}, /* Togo */
	{"TO", CE, TRUE, Country_TO_ChDesp}, /* Tonga */
	{"TT", CE, TRUE, Country_TT_ChDesp}, /* Trinidad and Tobago */
	{"TN", CE, TRUE, Country_TN_ChDesp}, /* Tunisia */
	{"TR", CE, TRUE, Country_TR_ChDesp}, /* Turkey */
	{"TM", CE, TRUE, Country_TM_ChDesp}, /* Turkmenistan */
	{"TV", CE, TRUE, Country_TV_ChDesp}, /* Tuvalu */
	{"UG", CE, TRUE, Country_UG_ChDesp}, /* Uganda */
	{"UA", CE, TRUE, Country_UA_ChDesp}, /* Ukraine */
	{"AE", CE, TRUE, Country_AE_ChDesp}, /* United Arab Emirates */
	{"GB", CE, TRUE, Country_GB_ChDesp}, /* United Kingdom */
	{"US", FCC, FALSE, Country_US_ChDesp}, /* United States */
	{"UY", CE, TRUE, Country_UY_ChDesp}, /* Uruguay */
	{"UZ", CE, TRUE, Country_UZ_ChDesp}, /* Uzbekistan */
	{"VU", CE, TRUE, Country_VU_ChDesp}, /* Vanuatu */
	{"VA", CE, TRUE, Country_VA_ChDesp}, /* Vatican City */
	{"VE", CE, TRUE, Country_VE_ChDesp}, /* Venezuela */
	{"VN", CE, TRUE, Country_VN_ChDesp}, /* Viet Nam */
	{"EH", CE, TRUE, Country_EH_ChDesp}, /* Western Sahara */
	{"YE", CE, TRUE, Country_YE_ChDesp}, /* Yemen */
	{"ZM", CE, TRUE, Country_ZM_ChDesp}, /* Zambia */
	{"ZW", CE, TRUE, Country_ZW_ChDesp}, /* Zimbabwe */
	{"EU", CE, TRUE, Country_EU_ChDesp}, /* Europe */
	{"NA", FCC, FALSE, Country_NA_ChDesp}, /* North America */
	{"WO", CE, FALSE, Country_WO_ChDesp}, /* World Wide */
	{"", 0, FALSE, NULL}, /* End */
};

#ifdef CONFIG_6G_SUPPORT
/*Korea, Republic of 6G*/
/*https://www.spinics.net/lists/wireless-regdb/msg01459.html*/
CH_DESP Country_KR6G_ChDesp[] = {
	{ 1,   59, 15, IDOR, FALSE},/*5925 ~ 7125MHz, Ch 1 ~ 59, Max BW: 160), (15db), NO-OUTDOOR */
	{ 0},			/* end*/
};

/*United States 6G*/
CH_DESP Country_US6G_ChDesp[] = {
	{ 1,   59, 12, IDOR, FALSE},/*5925 ~ 7125MHz, Ch 1 ~ 59, Max BW: 320), (15db), NO-OUTDOOR */
	{ 0},			/* end*/
};

/*World Wide 6G*/
CH_DESP Country_WO6G_ChDesp[] = {
	{ 1,   59, 15, IDOR, FALSE},/*5925 ~ 7125MHz, Ch 1 ~ 59, Max BW: 160), (15db), NO-OUTDOOR */
	{ 0},			/* end*/
};

/*add more countries need basis*/
CH_REGION ChRegion6G[] = {
	{"KR", CE, FALSE, Country_KR6G_ChDesp}, /* Korea, Republic of */
	{"US", FCC, FALSE, Country_US6G_ChDesp}, /* United States */
	{"WO", CE, FALSE, Country_WO6G_ChDesp}, /* World Wide */
	{"", 0, FALSE, NULL}, /* End */
};
#endif

PCH_REGION GetChRegion(
	IN PUCHAR CountryCode)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp((RTMP_STRING *) ChRegion[loop].CountReg, "") != 0) {
		if (strncmp((RTMP_STRING *) ChRegion[loop].CountReg, (RTMP_STRING *)CountryCode, 2) == 0) {
			pChRegion = &ChRegion[loop];
			break;
		}

		loop++;
	}

	/* Default: use WO*/
	if (pChRegion == NULL)
		pChRegion = GetChRegion("WO");

	return pChRegion;
}

#ifdef EXT_BUILD_CHANNEL_LIST

#ifdef CONFIG_6G_SUPPORT
PCH_REGION GetChRegion6G(
	IN PUCHAR CountryCode)
{
	INT loop = 0;
	PCH_REGION pChRegion = NULL;

	while (strcmp((RTMP_STRING *) ChRegion6G[loop].CountReg, "") != 0) {
		if (strncmp((RTMP_STRING *) ChRegion6G[loop].CountReg, (RTMP_STRING *)CountryCode, 2) == 0) {
			pChRegion = &ChRegion6G[loop];
			break;
		}

		loop++;
	}

	/* Default: use WO*/
	if (pChRegion == NULL)
		pChRegion = GetChRegion6G("WO");

	return pChRegion;
}
#endif

static VOID ChBandCheck(
	IN UCHAR rfic,
	OUT PUCHAR pChType)
{
	*pChType = 0;

#ifdef CONFIG_6G_SUPPORT
	if (rfic & RFIC_6GHZ)
		*pChType |= BAND_6G;
#endif

	if (rfic & RFIC_5GHZ)
		*pChType |= BAND_5G;

	if (rfic & RFIC_24GHZ)
		*pChType |= BAND_24G;

	if (*pChType == 0)
		*pChType = BAND_24G;
}

static UCHAR FillChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_DESP pChDesp,
	IN UCHAR Offset,
	IN UCHAR increment,
	IN UCHAR regulatoryDomain,
	struct wifi_dev *wdev)
{
	INT i, j;/* sachin - TODO, l; */
	UCHAR channel;
	USHORT PhyMode = wdev->PhyMode;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#if defined(CONFIG_AP_SUPPORT) || defined(RT_CFG80211_SUPPORT)
	UCHAR rfic = wmode_2_rfic(PhyMode);
	struct freq_oper oper;
	UCHAR bw = BW_20;
	if (hc_radio_query_by_rf(pAd, rfic, &oper) == HC_STATUS_OK)
		bw = oper.bw;
#endif
	j = Offset;

	for (i = 0; i < pChDesp->NumOfCh; i++) {
		channel = pChDesp->FirstChannel + i * increment;

		if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "JP", 2)) {
			/* for JP, ch14 can only be used when PhyMode is "B only" */
			if ((channel == 14) &&
				(!WMODE_EQUAL(PhyMode, WMODE_B)) &&
				((WMODE_CAP_2G(PhyMode)))) {
				pChDesp->NumOfCh--;
				break;
			}
		}

		/*New FCC spec restrict the used channel under DFS */
#ifdef CONFIG_AP_SUPPORT
		if (WMODE_CAP_5G(PhyMode)) {
			if ((pAd->CommonCfg.bIEEE80211H == 1) &&
				(pAd->CommonCfg.RDDurRegion == FCC) &&
#ifndef RT_CFG80211_SUPPORT
				(pAd->Dot11_H.bDFSIndoor == 1))
#else
				(pAd->Dot11_H[BandIdx].bDFSIndoor == 1))
#endif
				{
					if (RESTRICTION_BAND_1(pAd, channel, bw))
						continue;
				} else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
					   (pAd->CommonCfg.RDDurRegion == FCC) &&
#ifndef RT_CFG80211_SUPPORT
				(pAd->Dot11_H.bDFSIndoor == 0))
#else
				(pAd->Dot11_H[BandIdx].bDFSIndoor == 0))
#endif
				{
					if ((channel >= 100) && (channel <= 140))
						continue;
				}
		}
#endif /* CONFIG_AP_SUPPORT */
		/* sachin - TODO */
		pChCtrl->ChList[j].Channel = pChDesp->FirstChannel + i * increment;
		pChCtrl->ChList[j].MaxTxPwr = pChDesp->MaxTxPwr;
		pChCtrl->ChList[j].DfsReq = pChDesp->DfsReq;
		pChCtrl->ChList[j].RegulatoryDomain = regulatoryDomain;

#ifdef RT_CFG80211_SUPPORT
		CFG80211OS_ChanInfoInit(
			pAd->pCfg80211_CB,
			j,
			pChCtrl->ChList[j].Channel,
			pChCtrl->ChList[j].MaxTxPwr,
			WMODE_CAP_N(PhyMode),
			(bw == BW_20),
			PhyMode);
#endif /* RT_CFG80211_SUPPORT */
		j++;
	}
	pChCtrl->ChListNum = j;
#ifdef DOT11_N_SUPPORT
	for (i = Offset; i < pChCtrl->ChListNum; i++) {
		if (N_ChannelGroupCheck(pAd, pChCtrl->ChList[i].Channel, wdev))
			pChCtrl->ChList[i].Flags |= CHANNEL_40M_CAP;

#ifdef DOT11_VHT_AC
		if (vht80_channel_group(pAd, pChCtrl->ChList[i].Channel, wdev))
			pChCtrl->ChList[i].Flags |= CHANNEL_80M_CAP;

		if (vht160_channel_group(pAd, pChCtrl->ChList[i].Channel, wdev))
			pChCtrl->ChList[i].Flags |= CHANNEL_160M_CAP;
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */
	hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE);
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "\x1b[1;33m [FillChList] Test - pChCtrl->ChListNum = %d \x1b[m \n", pChCtrl->ChListNum);

	return j;
}


static UCHAR CeateChListByRf(RTMP_ADAPTER *pAd, UCHAR RfIC, PCH_REGION pChRegion, UCHAR Geography, UCHAR offset, struct wifi_dev *wdev)
{
	UCHAR i;
	PCH_DESP pChDesp;
	UCHAR ChType;
	UCHAR increment;
	UCHAR regulatoryDomain;

	if (hc_get_cur_rfic(wdev) == RfIC) {
		ChBandCheck(RfIC, &ChType);

		if (pAd->CommonCfg.pChDesp != NULL)
			pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
		else
			pChDesp = pChRegion->pChDesp;

		for (i = 0; pChDesp[i].FirstChannel != 0; i++) {
			if (pChDesp[i].FirstChannel == 0)
				break;

			if (ChType == BAND_5G) {
				if (pChDesp[i].FirstChannel <= 14)
					continue;
			} else if (ChType == BAND_24G) {
				if (pChDesp[i].FirstChannel > 14)
					continue;
			}

			if ((pChDesp[i].Geography == BOTH)
				|| (Geography == BOTH)
				|| (pChDesp[i].Geography == Geography)) {
				if (ChType == BAND_24G)
					increment = 1;
				else
					increment = 4;	/*4 for 5G and 6G*/

				if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
					regulatoryDomain = pAd->CommonCfg.DfsType;
				else
					regulatoryDomain = pChRegion->op_class_region;

				offset = FillChList(pAd, &pChDesp[i], offset, increment, regulatoryDomain, wdev);
			}
		}
#ifdef RT_CFG80211_SUPPORT
				if (ChType == BAND_6G) {
					if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB, NULL,  NULL, pChDesp) != 0)
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Update RegRule 6G failed!\n");
				} else if (ChType == BAND_5G) {
					if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB, NULL, pChDesp, NULL) != 0)
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Update RegRule 5G failed!\n");
				} else if (ChType == BAND_24G) {
					if (CFG80211OS_UpdateRegRuleByRegionIdx(pAd->pCfg80211_CB, pChDesp, NULL, NULL) != 0)
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Update RegRule 2.4G failed!\n");
				}
#endif /*RT_CFG80211_SUPPORT*/
	}

	return offset;
}


static inline VOID CreateChList(
	IN PRTMP_ADAPTER pAd,
	IN PCH_REGION pChRegion,
	IN UCHAR Geography,
	struct wifi_dev *wdev)
{
	UCHAR offset = 0;
	/* INT i,PhyIdx; */
	/* PCH_DESP pChDesp; */
	/* UCHAR ChType; */
	/* UCHAR increment; */
	/* UCHAR regulatoryDomain; */

	UCHAR BandIdx = HcGetBandByWdev(wdev);
	USHORT PhyMode = wdev->PhyMode;
	/* Get channel ctrl address */
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (pChRegion == NULL)
		return;

	/* Check state of channel list */
	if (hc_check_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_DONE)) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
			"BandIdx %d, channel list is already DONE\n", BandIdx);
		return;
	}

        /* Initialize channel list*/
	os_zero_mem(pChCtrl->ChList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));
	pChCtrl->ChListNum = 0;

	if (WMODE_CAP_6G(PhyMode))
		offset = CeateChListByRf(pAd, RFIC_6GHZ, pChRegion, Geography, offset, wdev);
	else if (WMODE_CAP_2G(PhyMode))
		offset = CeateChListByRf(pAd, RFIC_24GHZ, pChRegion, Geography, offset, wdev);
	else if (WMODE_CAP_5G(PhyMode))
		offset = CeateChListByRf(pAd, RFIC_5GHZ, pChRegion, Geography, offset, wdev);

}


VOID BuildChannelListEx(
	IN PRTMP_ADAPTER pAd,
        IN struct wifi_dev *wdev)
{
	PCH_REGION pChReg;
#ifdef CONFIG_6G_SUPPORT
	/*vikas:6G sprt*/
	USHORT PhyMode = wdev->PhyMode;
#endif

	pChReg = GetChRegion(pAd->CommonCfg.CountryCode);
	/*vikas:6G sprt*/
#ifdef CONFIG_6G_SUPPORT
	if (WMODE_CAP_6G(PhyMode))
		pChReg = GetChRegion6G(pAd->CommonCfg.CountryCode);
#endif

	CreateChList(pAd, pChReg, pAd->CommonCfg.Geography, wdev);
}

VOID BuildBeaconChList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen)
{
	INT i;
	ULONG TmpLen;
	PCH_REGION pChRegion;
	PCH_DESP pChDesp;
	UCHAR ChType;

	pChRegion = GetChRegion(pAd->CommonCfg.CountryCode);

	if (pChRegion == NULL)
		return;

	ChBandCheck(wmode_2_rfic(wdev->PhyMode), &ChType);
	*pBufLen = 0;

	if (pAd->CommonCfg.pChDesp != NULL)
		pChDesp = (PCH_DESP) pAd->CommonCfg.pChDesp;
	else
		pChDesp = pChRegion->pChDesp;

	for (i = 0; pChRegion->pChDesp[i].FirstChannel != 0; i++) {
		if (pChDesp[i].FirstChannel == 0)
			break;

		if (ChType == BAND_5G) {
			if (pChDesp[i].FirstChannel <= 14)
				continue;
		} else if (ChType == BAND_24G) {
			if (pChDesp[i].FirstChannel > 14)
				continue;
		}

		if ((pChDesp[i].Geography == BOTH) ||
			(pChDesp[i].Geography == pAd->CommonCfg.Geography)) {
			MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
							  1,					&pChDesp[i].FirstChannel,
							  1,					&pChDesp[i].NumOfCh,
							  1,					&pChDesp[i].MaxTxPwr,
							  END_OF_ARGS);
			*pBufLen += TmpLen;
		}
	}
}
#endif /* EXT_BUILD_CHANNEL_LIST */

BOOLEAN GetEDCCASupport(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN ret = FALSE;
	PCH_REGION pChReg;

	pChReg = GetChRegion(pAd->CommonCfg.CountryCode);

	if ((pChReg->op_class_region != FCC) && (pChReg->edcca_on == TRUE)) {
		/* actually need to check PM's table in CE country */
		ret = TRUE;
	}

	return ret;
}

UINT GetEDCCAStd(IN PUCHAR CountryCode, IN USHORT radioPhy)
{
//	UCHAR ClassRegion;
	UINT   u1EDCCAStd = EDCCA_Default;
//	ClassRegion = GetCountryRegionFromCountryCode(CountryCode);

	if (WMODE_CAP_6G(radioPhy)) {
		if ((strncmp("US", (RTMP_STRING *)CountryCode, 2) == 0) ||
			(strncmp("KR", (RTMP_STRING *)CountryCode, 2) == 0))
			u1EDCCAStd = EDCCA_Country_FCC6G;
//		else if (ClassRegion == CE) {
//			u1EDCCAStd = EDCCA_Country_ETSI;
//		}
	}
	return u1EDCCAStd;
}

UCHAR GetCountryRegionFromCountryCode(
	IN UCHAR *country_code)
{
	UCHAR ret = FCC;
	PCH_REGION pChReg;

	pChReg = GetChRegion(country_code);
	if (pChReg)
		ret = pChReg->op_class_region;
	return ret;
}

#ifdef DOT11_N_SUPPORT
static BOOLEAN IsValidChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel,
	IN struct wifi_dev *wdev)
{
	INT i;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel)
			break;
	}

	if (i == pChCtrl->ChListNum)
		return FALSE;
	else
		return TRUE;
}

static UCHAR GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction)
{
	CHAR ExtCh;

	if (Direction == EXTCHA_ABOVE)
		ExtCh = Channel + 4;
	else
		ExtCh = (Channel - 4) > 0 ? (Channel - 4) : 0;

	return ExtCh;
}

BOOLEAN ExtChCheck(
    IN PRTMP_ADAPTER pAd,
    IN UCHAR Channel,
    IN UCHAR Direction,
	IN struct wifi_dev *wdev)
{
	UCHAR ExtCh;

	/* Get extension channel by current direction */
	ExtCh = GetExtCh(Channel, Direction);

	/* Check whether current extension channel is in channel list or not */
	if (IsValidChannel(pAd, ExtCh, wdev))
		return TRUE;
	else
		return FALSE;
}

BOOLEAN N_ChannelGroupCheck(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN struct wifi_dev *wdev)
{
	BOOLEAN	RetVal = FALSE;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);

	switch (ch_band) {
	case CMD_CH_BAND_24G:
		do {
			UCHAR ExtCh;

			if (Channel == 14) {
				RetVal = FALSE;
				break;
			}

			ExtCh = GetExtCh(Channel, EXTCHA_ABOVE);

			if (IsValidChannel(pAd, ExtCh, wdev))
				RetVal = TRUE;
			else {
				ExtCh = GetExtCh(Channel, EXTCHA_BELOW);

				if (IsValidChannel(pAd, ExtCh, wdev))
					RetVal = TRUE;
			}
		} while (FALSE);
		break;

	case CMD_CH_BAND_5G:
	case CMD_CH_BAND_6G:
		RetVal = vht40_channel_group(pAd, Channel, wdev);
		break;

	default:
		break;
	}

	return RetVal;
}

static const UCHAR wfa_ht_ch_ext[] = {
	36, EXTCHA_ABOVE, 40, EXTCHA_BELOW,
	44, EXTCHA_ABOVE, 48, EXTCHA_BELOW,
	52, EXTCHA_ABOVE, 56, EXTCHA_BELOW,
	60, EXTCHA_ABOVE, 64, EXTCHA_BELOW,
	100, EXTCHA_ABOVE, 104, EXTCHA_BELOW,
	108, EXTCHA_ABOVE, 112, EXTCHA_BELOW,
	116, EXTCHA_ABOVE, 120, EXTCHA_BELOW,
	124, EXTCHA_ABOVE, 128, EXTCHA_BELOW,
	132, EXTCHA_ABOVE, 136, EXTCHA_BELOW,
	140, EXTCHA_ABOVE, 144, EXTCHA_BELOW,
	149, EXTCHA_ABOVE, 153, EXTCHA_BELOW,
	157, EXTCHA_ABOVE, 161, EXTCHA_BELOW,
	165, EXTCHA_ABOVE, 169, EXTCHA_BELOW,
	173, EXTCHA_ABOVE, 177, EXTCHA_BELOW,
	0, 0
};

static const UCHAR wfa_ht_ch_ext_6G[] = {
	1, EXTCHA_ABOVE, 5, EXTCHA_BELOW,
	9, EXTCHA_ABOVE, 13, EXTCHA_BELOW,
	17, EXTCHA_ABOVE, 21, EXTCHA_BELOW,
	25, EXTCHA_ABOVE, 29, EXTCHA_BELOW,
	33, EXTCHA_ABOVE, 37, EXTCHA_BELOW,
	41, EXTCHA_ABOVE, 45, EXTCHA_BELOW,
	49, EXTCHA_ABOVE, 53, EXTCHA_BELOW,
	57, EXTCHA_ABOVE, 61, EXTCHA_BELOW,
	65, EXTCHA_ABOVE, 69, EXTCHA_BELOW,
	73, EXTCHA_ABOVE, 77, EXTCHA_BELOW,
	81, EXTCHA_ABOVE, 85, EXTCHA_BELOW,
	89, EXTCHA_ABOVE, 93, EXTCHA_BELOW,
	97, EXTCHA_ABOVE, 101, EXTCHA_BELOW,
	105, EXTCHA_ABOVE, 109, EXTCHA_BELOW,
	113, EXTCHA_ABOVE, 117, EXTCHA_BELOW,
	121, EXTCHA_ABOVE, 125, EXTCHA_BELOW,
	129, EXTCHA_ABOVE, 133, EXTCHA_BELOW,
	137, EXTCHA_ABOVE, 141, EXTCHA_BELOW,
	145, EXTCHA_ABOVE, 149, EXTCHA_BELOW,
	153, EXTCHA_ABOVE, 157, EXTCHA_BELOW,
	161, EXTCHA_ABOVE, 165, EXTCHA_BELOW,
	169, EXTCHA_ABOVE, 173, EXTCHA_BELOW,
	177, EXTCHA_ABOVE, 181, EXTCHA_BELOW,
	185, EXTCHA_ABOVE, 189, EXTCHA_BELOW,
	193, EXTCHA_ABOVE, 197, EXTCHA_BELOW,
	201, EXTCHA_ABOVE, 205, EXTCHA_BELOW,
	209, EXTCHA_ABOVE, 213, EXTCHA_BELOW,
	217, EXTCHA_ABOVE, 221, EXTCHA_BELOW,
	225, EXTCHA_ABOVE, 229, EXTCHA_BELOW,
	0, 0
};

VOID ht_ext_cha_adjust(RTMP_ADAPTER *pAd, UCHAR prim_ch, UCHAR *ht_bw, UCHAR *ext_cha, struct wifi_dev *wdev)
{
	INT idx;
	UCHAR ch_band = wlan_config_get_ch_band(wdev);
	const UCHAR *ch_ext = NULL;

	if (ch_band == CMD_CH_BAND_5G)
		ch_ext = wfa_ht_ch_ext;
	else if (ch_band == CMD_CH_BAND_6G)
		ch_ext = wfa_ht_ch_ext_6G;

	if (*ht_bw == HT_BW_40) {
		if (ch_band == CMD_CH_BAND_5G || ch_band == CMD_CH_BAND_6G) {
			idx = 0;

			while (ch_ext[idx] != 0) {
				if (ch_ext[idx] == prim_ch &&
					IsValidChannel(pAd, GetExtCh(prim_ch, ch_ext[idx + 1]), wdev)) {
					*ext_cha = ch_ext[idx + 1];
					break;
				}

				idx += 2;
			};

			if (ch_ext[idx] == 0) {
				*ht_bw = HT_BW_20;
				*ext_cha = EXTCHA_NONE;
			}
		} else {
			do {
				UCHAR ExtCh;
				UCHAR Dir = *ext_cha;

				if (Dir == EXTCHA_NONE)
					Dir = EXTCHA_ABOVE;

				ExtCh = GetExtCh(prim_ch, Dir);

				if (IsValidChannel(pAd, ExtCh, wdev)) {
					*ext_cha = Dir;
					break;
				}

				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = GetExtCh(prim_ch, Dir);

				if (IsValidChannel(pAd, ExtCh, wdev)) {
					*ext_cha = Dir;
					break;
				}

				*ht_bw = HT_BW_20;
			} while (FALSE);

			if (prim_ch == 14)
				*ht_bw = HT_BW_20;
		}
	} else if (*ht_bw == HT_BW_20)
		*ext_cha = EXTCHA_NONE;
}

#endif /* DOT11_N_SUPPORT */


UINT8 GetCuntryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN USHORT PhyMode,
	IN struct wifi_dev *wdev,
	IN UCHAR ht_bw)
{
	int i;
	UINT8 channel = wdev->channel;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#if defined(SINGLE_SKU) || defined(ANTENNA_CONTROL_SUPPORT)
	UINT8 TxPath = pAd->Antenna.field.TxPath;
#endif /* defined(SINGLE_SKU) || defined(ANTENNA_CONTROL_SUPPORT) */

#ifdef ANTENNA_CONTROL_SUPPORT
	{
		UINT8 BandIdx = HcGetBandByWdev(wdev);
		if (pAd->bAntennaSetAPEnable[BandIdx])
			TxPath = pAd->TxStream[BandIdx];
	}
#endif /* ANTENNA_CONTROL_SUPPORT */

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel)
			break;
	}

	if (i == pChCtrl->ChListNum)
		return 30;

#ifdef SINGLE_SKU

	if (pAd->CommonCfg.bSKUMode == TRUE) {
		UINT deltaTxStreamPwr = 0;
#ifdef DOT11_N_SUPPORT

		if (WMODE_CAP_N(PhyMode) && (TxPath == 2))
			deltaTxStreamPwr = 3; /* If 2Tx case, antenna gain will increase 3dBm*/

#endif /* DOT11_N_SUPPORT */

		if (pChCtrl->ChList[i].RegulatoryDomain == FCC) {
			/* FCC should maintain 20/40 Bandwidth, and without antenna gain */
#ifdef DOT11_N_SUPPORT
			if (WMODE_CAP_N(PhyMode) &&
				(ht_bw == BW_40) &&
				(channel == 1 || channel == 11))
				return pChCtrl->ChList[i].MaxTxPwr - pAd->CommonCfg.BandedgeDelta - deltaTxStreamPwr;
			else
#endif /* DOT11_N_SUPPORT */
				return pChCtrl->ChList[i].MaxTxPwr - deltaTxStreamPwr;
		} else if (pChCtrl->ChList[i].RegulatoryDomain == CE)
			return pChCtrl->ChList[i].MaxTxPwr - pAd->CommonCfg.AntGain - deltaTxStreamPwr;
		else
			return 30;
	} else
#endif /* SINGLE_SKU */
		return pChCtrl->ChList[i].MaxTxPwr;
}

/* for OS_ABL */
VOID RTMP_MapChannelID2KHZ(
	IN UCHAR Ch,
	OUT UINT32 * pFreq)
{
	int chIdx;

	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++) {
		if ((Ch) == CH_HZ_ID_MAP[chIdx].channel) {
			(*pFreq) = CH_HZ_ID_MAP[chIdx].freqKHz * 1000;
			break;
		}
	}

	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pFreq) = 2412000;
}

/* for OS_ABL */
VOID RTMP_MapKHZ2ChannelID(
	IN ULONG Freq,
	OUT INT *pCh)
{
	int chIdx;

	for (chIdx = 0; chIdx < CH_HZ_ID_MAP_NUM; chIdx++) {
		if ((Freq) == CH_HZ_ID_MAP[chIdx].freqKHz) {
			(*pCh) = CH_HZ_ID_MAP[chIdx].channel;
			break;
		}
	}

	if (chIdx == CH_HZ_ID_MAP_NUM)
		(*pCh) = 1;
}

INT32 ChannelFreqToGroup(UINT32 ChannelFreq)
{
	INT32 GroupIndex = 0;

	if (ChannelFreq <= 2484) /* 2G CH14 = 2484 */
		GroupIndex = 0;
	else if ((ChannelFreq >= 4850) && (ChannelFreq <= 5140))
		GroupIndex = 1;
	else if ((ChannelFreq >= 5145) && (ChannelFreq <= 5250))
		GroupIndex = 2;
	else if ((ChannelFreq >= 5255) && (ChannelFreq <= 5360))
		GroupIndex = 3;
	else if ((ChannelFreq >= 5365) && (ChannelFreq <= 5470))
		GroupIndex = 4;
	else if ((ChannelFreq >= 5475) && (ChannelFreq <= 5580))
		GroupIndex = 5;
	else if ((ChannelFreq >= 5585) && (ChannelFreq <= 5690))
		GroupIndex = 6;
	else if ((ChannelFreq >= 5695) && (ChannelFreq <= 5800))
		GroupIndex = 7;
	else if ((ChannelFreq >= 5805) && (ChannelFreq <= 5950))
		GroupIndex = 8;
	else {
		GroupIndex = -1;
		MTWF_DBG(NULL, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR,
				 "Can't find group for [%d].\n", ChannelFreq);
	}

	return GroupIndex;
}


BOOLEAN MTChGrpValid(IN CHANNEL_CTRL *pChCtrl)
{
	return (pChCtrl->ChGrpABandChNum > 0 &&
		pChCtrl->ChGrpABandChNum < MAX_NUM_OF_CHANNELS &&
		pChCtrl->ChGrpABandEn != 0);
}

void MTSetChGrp(RTMP_ADAPTER *pAd, RTMP_STRING *buf)
{
	UCHAR ChGrpIdx, ChIdx, len_macptr2, BandIdx, TotalChNum, u1BandNum = 0;
	UCHAR *macptr, *macptr2, *buf_ChGrp;
	BOOLEAN bEnable;
	PCH_GROUP_DESC pChDescGrp;
	CHANNEL_CTRL *pChCtrl;
	struct wifi_dev *pwdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pwdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		pwdev = &pAd->StaCfg[MAIN_MBSSID].wdev;
	}
#endif

	if (buf == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "[MTSetChGrp] (buf == NULL) Not enough memory for dynamic allocating\n");
		return;
	}

#ifdef DBDC_MODE
	/*DBDC_MODE is enabled and only single band is used*/
	if (pAd->CommonCfg.dbdc_mode == FALSE)
		u1BandNum = 1;
	else
		u1BandNum = DBDC_BAND_NUM;
#else
	/*DBDC_MODE is NOT enabled (single band only)*/
	u1BandNum = DBDC_BAND_NUM;
#endif

	/* Find channel group of each HW band */
	for (BandIdx = 0, macptr2 = rstrtok(buf, ";"); macptr2; macptr2 = rstrtok(buf, ";"), BandIdx++) {
		/*Get channel control*/
		/* 2.4G + 5G */
		/* l1profile: (2Gprofile;5Gprofile) */
		/* If DEFAULT_5G_PROFILE is NOT enabled, (2Gprofile;5Gprofile) iNIC_ap_2g.dat (if 2g.dat is 2G) is set to HW band 0 */
		/* If DEFAULT_5G_PROFILE is enabled, (5Gprofile;2Gprofile) iNIC_ap_5g.dat is set to HW band 1 */
		/* ChannelGrp is only for 5G channel list */

		/* 5G + 5G */
		/* l1profile: (B0_5G_profile;B1_5G_profile) */
		/* If DEFAULT_5G_PROFILE is NOT enabled, (B0_5G_profile;B1_5G_profile) B0_5G_profile is set to HW band 0 */
		/* If DEFAULT_5G_PROFILE is enabled, (B1_5G_profile;B0_5G_profile) B1_5G_profile is set to HW band 0 */
		if (pAd->CommonCfg.eDBDC_mode == ENUM_DBDC_5G5G) {
			/* 5G + 5G */
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "[MTSetChGrp] ENUM_DBDC_5G5G: BandIdx = %d\n", BandIdx);
		} else {
			if (pwdev == NULL) {
				MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "Main Wdev is NULL!\n");
				return;
			}
			if (WMODE_CAP_5G(pwdev->PhyMode)) {/* 5G + 2G */
				if (BandIdx == 0 && (pAd->CommonCfg.dbdc_mode == TRUE))
					pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BAND1);/* [5G] + 2G */
				else
					pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BAND0);/* 5G + [2G] */
			} else
				pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);/* 2G + 5G or 2G only */
		}

		/*Initialize channel group*/
		NdisZeroMemory(pChCtrl->ChGrpABandChList, (MAX_NUM_OF_CHANNELS)*sizeof(UCHAR));
		TotalChNum = 0;
		pChCtrl->ChGrpABandChNum = 0;
		pChCtrl->ChGrpABandEn = 0;

		/*Copy channel group of current HW band for checking bit map*/
		len_macptr2 = strlen(macptr2) + 1;
		os_alloc_mem(NULL, (UCHAR **)&buf_ChGrp, len_macptr2);
		if (buf_ChGrp == NULL) {
			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "[MTSetChGrp] (buf_ChGrp == NULL) Not enough memory for dynamic allocating\n");
			return;
		}
		strncpy(buf_ChGrp, macptr2, len_macptr2);

		/*Check bit map of channel group of current HW band*/
		for (ChGrpIdx = 0, macptr = rstrtok(buf_ChGrp, ":"); (macptr && ChGrpIdx < Channel_GRP_Num); macptr = rstrtok(NULL, ":"), ChGrpIdx++) {
			bEnable = 0;
			bEnable = (UCHAR) simple_strtol(macptr, 0, 10);
			pChDescGrp = &(Channel_GRP[ChGrpIdx]);
			if (bEnable == 1) {
				if ((TotalChNum + pChDescGrp->NumOfCh) < MAX_NUM_OF_CHANNELS) {
					pChCtrl->ChGrpABandEn |= (1 << ChGrpIdx);
					for (ChIdx = 0; ChIdx < pChDescGrp->NumOfCh; ChIdx++) {
						if ((TotalChNum + ChIdx) < MAX_NUM_OF_CHANNELS) {
							pChCtrl->ChGrpABandChList[TotalChNum + ChIdx] = pChDescGrp->FirstChannel + (ChIdx * 4);
							MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "[MTSetChGrp] Test - ChCtrl[%d]->ChGrpABandChList[%d]=%d\n",
										BandIdx, TotalChNum + ChIdx, pChDescGrp->FirstChannel + (ChIdx * 4));
						}
					}
					TotalChNum += pChDescGrp->NumOfCh;
				}
			}
		}
		pChCtrl->ChGrpABandChNum = TotalChNum;

		/*Shift to the next channel group of HW band*/
		buf = buf + len_macptr2;

		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO, "[MTSetChGrp] BandIdx =%d, pChCtrl->ChGrpABandChNum=%d, ChGrp=%d/%d/%d/%d\n",
					BandIdx, pChCtrl->ChGrpABandChNum,
					((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND0)) ? 1 : 0),
					((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND1)) ? 1 : 0),
					((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND2)) ? 1 : 0),
					((pChCtrl->ChGrpABandEn & (CH_GROUP_BAND3)) ? 1 : 0));

		os_free_mem(buf_ChGrp);
	}
}

BOOLEAN MTChGrpChannelChk(
	IN CHANNEL_CTRL *pChCtrl,
	IN UCHAR ch)
{
	UCHAR ChGrpIdx;
	BOOLEAN result = FALSE;

	for (ChGrpIdx = 0; ChGrpIdx < pChCtrl->ChGrpABandChNum; ChGrpIdx++) {
		if (ch == pChCtrl->ChGrpABandChList[ChGrpIdx]) {
			result = TRUE;
			break;
		}
	}
	return result;
}
