#ifdef DOT11_N_SUPPORT
#include "rt_config.h"

typedef enum {
	Reserved,
	FREQ_2G407,
	FREQ_2G414,
	FREQ_3G00,
	FREQ_3G0025,
	FREQ_4G00,
	FREQ_4G0025,
	FREQ_4G85,
	FREQ_4G89,
	FREQ_4G9375,
	FREQ_5G00,
	FREQ_5G0025,
	FREQ_5G94
} START_FREQ;

/*
	definition for Behavior limits set
	Reference 11n/11ac spec Table D-2
*/
typedef enum {
	NOMADICBEHAVIOR = (1 << 1),
	LICENSEEXEMPTBEHAVIOR = (1 << 10),
	PRIMARYCHANNELLOWERBEHAVIOR = (1 << 13),
	PRIMARYCHANNELUPPERBEHAVIOR = (1 << 14),
	CCA_EDBEHAVIOR = (1 << 15),
	DFS_50_100_BEHAVIOR = (1 << 16),
	ITS_NONMOBILE_OPERATIONS = (1 << 17),
	ITS_MOBILE_OPERATIONS = (1 << 18),
	PLUS_80 = (1 << 19),
	USEEIRPFORVHTTXPOWENV = (1 << 20),
	COMMON = 0xffffff
} BEHAVIOR_LIMITS_SET;


typedef struct {
	UCHAR		reg_class;			/* regulatory class */
	UCHAR		global_class;		/* Global operating class */
	START_FREQ	start_freq;			/* Channel starting frequency*/
	UCHAR		spacing;			/* 0: 20Mhz, 1: 40Mhz */
	UCHAR		channel_set[16];	/* max 15 channels, use 0 as terminator */
	BEHAVIOR_LIMITS_SET behavior_limit_set; /* Behavior limits set */
} REG_CLASS, *PREG_CLASS;

typedef struct {
	UCHAR		reg_class;			/* regulatory class */
	UCHAR		global_class;		/* Global operating class */
	START_FREQ	start_freq;			/* Channel starting frequency*/
	UCHAR		spacing;			/* 0: 20Mhz, 1: 40Mhz */
	UCHAR		channel_set[16];	/* max 15 channels, use 0 as terminator */
	UCHAR       center_freq[16];	/* max 15 channels, use 0 as terminator */
	BEHAVIOR_LIMITS_SET behavior_limit_set; /* Behavior limits set */
} REG_CLASS_VHT, *PREG_CLASS_VHT;

typedef struct {
	UCHAR		reg_class;			/* regulatory class */
	UCHAR		global_class;		/* Global operating class */
	START_FREQ	start_freq;			/* Channel starting frequency*/
	UCHAR		spacing;			/* 0: 20MHz, 1: 40MHz, 2: 80 MHz, 3: 160 MHz */
	UCHAR		channel_set[60];	/* max 59 channels, use 0 as terminator */
	UCHAR		center_freq[60];	/* max 59 channels, use 0 as terminator */
	BEHAVIOR_LIMITS_SET behavior_limit_set; /* Behavior limits set */
} REG_CLASS_HE, *PREG_CLASS_HE;

#define OP_CLASS_81 81
#define OP_CLASS_84 84
#define OP_CLASS_115 115
#define OP_CLASS_127 127
#define OP_CLASS_129 129
#define OP_CLASS_125 125
#define OP_CLASS_130 130
#define OP_CLASS_135 135
#ifdef MAP_R2
#define TX_MAX_STREAM 4
#define RX_MAX_STREAM 4
#endif

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
#ifdef MAP_6E_SUPPORT
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_6GHZ[];
#endif

/*
	Table E-1혰Operating classes in the United States  (11N)
*/
REG_CLASS reg_class_fcc[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																DFS_50_100_BEHAVIOR},
	{3,  124, FREQ_5G00,   BW_20, {149, 153, 157, 161, 0},															NOMADICBEHAVIOR},
	{4,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						DFS_50_100_BEHAVIOR},
	{5,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 173, 177, 0},						LICENSEEXEMPTBEHAVIOR},
	{6,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0},												COMMON},
	{7,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0},												COMMON},
	{8,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0},															COMMON},
	{9,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0},															COMMON},
	{10, 101, FREQ_4G85,   BW_20, {21, 25, 0},																		COMMON},
	{11, 101, FREQ_4G85,   BW_20, {21, 25, 0},																		COMMON},
	{12,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0},											LICENSEEXEMPTBEHAVIOR},
	{13,  94, FREQ_3G00,   BW_20, {133, 137, 0},																	CCA_EDBEHAVIOR},
	{14,  95, FREQ_3G00,   BW_10, {132, 134, 136, 138, 0},															CCA_EDBEHAVIOR},
	{15,  96, FREQ_3G0025, BW_5,  {131, 132, 133, 134, 135, 136, 137, 138, 0},										CCA_EDBEHAVIOR},
	{16,   0, FREQ_5G0025, BW_5,  {170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},	ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{18,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0},					ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{22, 116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{23, 119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{24, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{25, 126, FREQ_5G00,   BW_40, {149, 157, 165, 173, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{26, 126, FREQ_5G00,   BW_40, {149, 157, 165, 173},																	LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{27, 117, FREQ_5G00,   BW_40, {40, 48, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{28, 120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{29, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},													NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{30, 127, FREQ_5G00,   BW_40, {153, 161, 169, 177, 0},																	NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{31, 127, FREQ_5G00,   BW_40, {153, 161, 169, 177, 0},																	LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{32,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{33,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{128, 128, FREQ_5G00,  BW_80, {0},																			COMMON},
	{129, 129, FREQ_5G00,  BW_80, {0},																			COMMON},
	{130, 130, FREQ_5G00,  BW_80, {0},																			COMMON},

	{0, 0, 0, 0, {0}, 0}			/* end */
};

/*
	Table E-2혰Operating classes in Europe (11N)
*/
REG_CLASS reg_class_ce[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																NOMADICBEHAVIOR},
	{3,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						COMMON},
	{4,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0},									LICENSEEXEMPTBEHAVIOR},
	{5,  116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{6,  119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{7,  122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR},
	{8,  117, FREQ_5G00,   BW_40, {40, 48, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{9,  120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{10, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},													PRIMARYCHANNELUPPERBEHAVIOR}, /* 10 */
	{11,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{12,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{13,   0, FREQ_5G0025, BW_5,  {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{14,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{15,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0},					ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{16,   0, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17, 125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 0},												COMMON},
	{0, 0, 0, 0, {0}, 0}			/* end */
};

/*
	Table E-3혰Operating classes in Japan (11N)
*/
REG_CLASS reg_class_jp[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {34, 36, 38, 40, 42, 44, 46, 48, 0},												COMMON},/* 1 */
	{2,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{3,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{4,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{5,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{6,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0},																	COMMON},
	{7,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{8,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{9,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{10, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},/* 10 */
	{11, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0},														COMMON},
	{12, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{13, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{14, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{15, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0},																COMMON},
	{16, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{17, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{18, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{19, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{20, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0},												COMMON},
	{21, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{22, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{23, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{24, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0},														COMMON},
	{25, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{26, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{27, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{28, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{29, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0},									COMMON},
	{30,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0},									LICENSEEXEMPTBEHAVIOR},
	{31,  82, FREQ_2G414,  BW_25, {14, 0},																			LICENSEEXEMPTBEHAVIOR},
	{32, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																COMMON},
	{33, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																COMMON},
	{34, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 0},						DFS_50_100_BEHAVIOR},
	{35, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						DFS_50_100_BEHAVIOR},
	{36, 116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{37, 119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{38, 119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{39, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 140, 0},														PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{40, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0},														PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{41, 117, FREQ_5G00,   BW_40, {40, 48, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{42, 120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{43, 120, FREQ_5G00,   BW_40, {56, 64, 0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{44, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 144, 0},														PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{45, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0},														PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{46, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{47, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{48, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{49, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{50, 104, FREQ_4G00,   BW_40, {184, 192, 0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{51, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{52, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{53, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{54, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{55, 105, FREQ_4G00,   BW_40, {188, 196, 0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{56,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{57,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{58, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0},						NOMADICBEHAVIOR | LICENSEEXEMPTBEHAVIOR},
	{0, 0, 0, 0, {0}, 0}			/* end */
};

REG_CLASS reg_class_cn[] = {
	{0, 0, 0, 0, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0},																USEEIRPFORVHTTXPOWENV},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0},																DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{3,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 0},						USEEIRPFORVHTTXPOWENV},
	{4,  116, FREQ_5G00,   BW_40, {36, 44, 0},																		PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{5,  119, FREQ_5G00,   BW_40, {52, 60, 0},																		PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{6,  126, FREQ_5G00,   BW_40, {149, 157, 0},																	PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{7,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0},									LICENSEEXEMPTBEHAVIOR},
	{8,   83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{9,   84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{0, 0, 0, 0, {0}, 0}			/* end */
};

#ifdef DOT11_VHT_AC
/*
	Table E-1혰Operating classes in the United States  (11AC)
*/
REG_CLASS_VHT reg_class_vht_fcc[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},																DFS_50_100_BEHAVIOR},
	{3,  124, FREQ_5G00,   BW_20, {149, 153, 157, 161, 0}, {0},														NOMADICBEHAVIOR},
	{4,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 0}, {0},				DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{5,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 173, 177, 0}, {0},                                                    LICENSEEXEMPTBEHAVIOR},
	{6,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0}, {0},												COMMON},
	{7,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0}, {0},												COMMON},
	{8,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0}, {0},															COMMON},
	{9,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0}, {0},														COMMON},
	{10, 101, FREQ_4G85,   BW_20, {21, 25, 0}, {0},																		COMMON},
	{11, 101, FREQ_4G85,   BW_20, {21, 25, 0}, {0},																		COMMON},
	{12,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0}, {0},											LICENSEEXEMPTBEHAVIOR},
	{13,  94, FREQ_3G00,   BW_20, {133, 137, 0}, {0},																	CCA_EDBEHAVIOR},
	{14,  95, FREQ_3G00,   BW_10, {132, 134, 136, 138, 0}, {0},															CCA_EDBEHAVIOR},
	{15,  96, FREQ_3G0025, BW_5,  {131, 132, 133, 134, 135, 136, 137, 138, 0}, {0},									CCA_EDBEHAVIOR},
	{16,   0, FREQ_5G0025, BW_5,  {170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},	ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{18,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0}, {0},				ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{22, 116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{23, 119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{24, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 140, 0},	{0},											DFS_50_100_BEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{25, 126, FREQ_5G00,   BW_40, {149, 157, 165, 173, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{26, 126, FREQ_5G00,   BW_40, {149, 157, 165, 173}, {0},																	LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{27, 117, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{28, 120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{29, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 144, 0}, {0},												NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{30, 127, FREQ_5G00,   BW_40, {153, 161, 169, 177, 0}, {0},																	NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{31, 127, FREQ_5G00,   BW_40, {153, 161, 169, 177, 0},	{0},																LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{32,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 0}, {0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{33,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 0}, {0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 138, 155, 171, 0},													USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 114, 163, 0},																	USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 138, 155, 171, 0},													PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};


/*
	Table E-2혰Operating classes in Europe (11AC)
*/
REG_CLASS_VHT reg_class_vht_ce[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},																NOMADICBEHAVIOR},
	{3,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						COMMON},
	{4,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},									LICENSEEXEMPTBEHAVIOR},
	{5,  116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{6,  119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{7,  122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0}, {0},													PRIMARYCHANNELLOWERBEHAVIOR},
	{8,  117, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{9,  120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{10, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0}, {0},													PRIMARYCHANNELUPPERBEHAVIOR},
	{11,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{12,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{13,   0, FREQ_5G0025, BW_5,  {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{14,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{15,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0}, {0},					ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{16,   0, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17, 125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 0}, {0},												COMMON},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 0},															USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 114, 0},																	USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 0},															PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

/*
	Table E-3혰Operating classes in Japan (11N)
*/
REG_CLASS_VHT reg_class_vht_jp[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {34, 36, 38, 40, 42, 44, 46, 48, 0}, {0},											COMMON},
	{2,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																	COMMON},
	{3,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{4,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{5,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{6,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{7,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{8,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{9,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{10, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{11, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{12, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{13, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{14, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{15, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{16, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{17, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{18, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{19, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{20, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{21, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{22, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{23, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{24, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{25, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{26, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{27, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{28, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{29, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{30,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},									LICENSEEXEMPTBEHAVIOR},
	{31,  82, FREQ_2G414,  BW_25, {14, 0}, {0},																		LICENSEEXEMPTBEHAVIOR},
	{32, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},															COMMON},
	{33, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},															COMMON},
	{34, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 0}, {0},						DFS_50_100_BEHAVIOR},
	{35, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						DFS_50_100_BEHAVIOR},
	{36, 116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{37, 119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{38, 119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{39, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 140, 0}, {0},													PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{40, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0}, {0},													PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{41, 117, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{42, 120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{43, 120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{44, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 144, 0}, {0},													PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{45, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0}, {0},													PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{46, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{47, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{48, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{49, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{50, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{51, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{52, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{53, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{54, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{55, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{56,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{57,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{58, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						NOMADICBEHAVIOR | LICENSEEXEMPTBEHAVIOR},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 138, 0},															USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 114, 0},																	USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 0},															PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

/*
*	Table E-4뾑perating classes in Global
*/
REG_CLASS_VHT reg_class_global[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{81,  0, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},								COMMON},
	{82,  0, FREQ_2G414,  BW_25, {14, 0}, {0},																		COMMON},
	{83,  0, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},												PRIMARYCHANNELLOWERBEHAVIOR},
	{84,  0, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},											PRIMARYCHANNELUPPERBEHAVIOR},
	{94,  0, FREQ_3G00,   BW_20, {133, 137, 0}, {0},																CCA_EDBEHAVIOR},
	{95,  0, FREQ_3G00,   BW_10, {132, 134, 136, 138, 0}, {0},														CCA_EDBEHAVIOR},
	{96,  0, FREQ_3G0025, BW_5,  {131, 132, 133, 134, 135, 136, 137, 138, 0}, {0},									CCA_EDBEHAVIOR},
	{101, 0, FREQ_4G85,   BW_20, {21, 25, 0}, {0},																	COMMON},
	{102, 0, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0}, {0},														COMMON},
	{103, 0, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0}, {0},											COMMON},
	{104, 0, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																PRIMARYCHANNELLOWERBEHAVIOR},
	{105, 0, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																PRIMARYCHANNELUPPERBEHAVIOR},
	{106, 0, FREQ_4G00,   BW_20, {191, 195, 0}, {0},																COMMON},
	{107, 0, FREQ_4G00,   BW_10, {189, 191, 193, 195, 197, 0}, {0},													COMMON},
	{108, 0, FREQ_4G0025, BW_5,  {188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 0}, {0},						COMMON},
	{109, 0, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{110, 0, FREQ_4G00,   BW_10, {183, 184, 185, 186, 187, 188, 189, 0}, {0},										COMMON},
	{111, 0, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{112, 0, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{113, 0, FREQ_5G00,   BW_10, {7, 8, 9, 10, 11, 0}, {0},															COMMON},
	{114, 0, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{115, 0, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},															USEEIRPFORVHTTXPOWENV},
	{116, 0, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{117, 0, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{118, 0, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},															DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{119, 0, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{120, 0, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{121, 0, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 0}, {0},				DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{122, 0, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 140, 0}, {0},											PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{123, 0, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 144, 0}, {0},											PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{124, 0, FREQ_5G00,   BW_20, {149, 153, 157, 161, 0}, {0},														NOMADICBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{125, 0, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 0}, {0},                                            LICENSEEXEMPTBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{126, 0, FREQ_5G00,   BW_40, {149, 157, 0}, {0},																PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{127, 0, FREQ_5G00,   BW_40, {153, 161, 0}, {0},																PRIMARYCHANNELUPPERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{128, 0, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 138, 155, 0},												USEEIRPFORVHTTXPOWENV},
	{129, 0, FREQ_5G00,   BW_160, {0}, {50, 114, 0},																USEEIRPFORVHTTXPOWENV},
	{130, 0, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 138, 155, 0},												PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

REG_CLASS_VHT reg_class_vht_cn[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},																USEEIRPFORVHTTXPOWENV},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},																DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{3,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 0}, {0},                                                    USEEIRPFORVHTTXPOWENV},
	{4,  116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{5,  119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{6,  126, FREQ_5G00,   BW_40, {149, 157, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{7,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},									LICENSEEXEMPTBEHAVIOR},
	{8,   83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{9,   84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 155, 0},																USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 0},																			USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 155, 0},																PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
/*
	Table E-1혰Operating classes in the United States  (11AC)
*/
REG_CLASS_HE reg_class_he_fcc[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},																DFS_50_100_BEHAVIOR},
	{3,  124, FREQ_5G00,   BW_20, {149, 153, 157, 161, 0}, {0},														NOMADICBEHAVIOR},
	{4,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 0}, {0},				DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{5,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 173, 177, 0}, {0},                                                    LICENSEEXEMPTBEHAVIOR},
	{6,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0}, {0},												COMMON},
	{7,  103, FREQ_4G9375, BW_5,  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0}, {0},												COMMON},
	{8,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0}, {0},															COMMON},
	{9,  102, FREQ_4G89,   BW_10, {11, 13, 15, 17, 19, 0}, {0},														COMMON},
	{10, 101, FREQ_4G85,   BW_20, {21, 25, 0}, {0},																		COMMON},
	{11, 101, FREQ_4G85,   BW_20, {21, 25, 0}, {0},																		COMMON},
	{12,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0}, {0},											LICENSEEXEMPTBEHAVIOR},
	{13,  94, FREQ_3G00,   BW_20, {133, 137, 0}, {0},																	CCA_EDBEHAVIOR},
	{14,  95, FREQ_3G00,   BW_10, {132, 134, 136, 138, 0}, {0},															CCA_EDBEHAVIOR},
	{15,  96, FREQ_3G0025, BW_5,  {131, 132, 133, 134, 135, 136, 137, 138, 0}, {0},									CCA_EDBEHAVIOR},
	{16,   0, FREQ_5G0025, BW_5,  {170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},	ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{18,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0}, {0},				ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{22, 116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{23, 119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{24, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 140, 0},	{0},											DFS_50_100_BEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{25, 126, FREQ_5G00,   BW_40, {149, 157, 165, 173, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{26, 126, FREQ_5G00,   BW_40, {149, 157, 165, 173, 0}, {0},																	LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{27, 117, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{28, 120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{29, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 144, 0}, {0},												NOMADICBEHAVIOR | DFS_50_100_BEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{30, 127, FREQ_5G00,   BW_40, {153, 161, 169, 177, 0}, {0},																	NOMADICBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{31, 127, FREQ_5G00,   BW_40, {153, 161, 169, 177, 0},	{0},																LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{32,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 0}, {0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{33,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 0}, {0},														LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{128, 128, FREQ_5G00,  BW_80, {0}, {42, 58, 106, 122, 138, 155, 171, 0},													USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,  BW_160, {0}, {50, 114, 163, 0},																	USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,  BW_80, {0}, {42, 58, 106, 122, 138, 155, 171, 0},													PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{131, 131, FREQ_5G94,  BW_20, {0}, {1, 5, 9, 13, 17, 21, 25, 29, 33, 37,
										41, 45, 49, 53, 57, 61, 65, 69, 73, 77,
										81, 85, 89, 93, 97, 101, 105, 109, 113, 117,
										121, 125, 129, 133, 137, 141, 145, 149, 153, 157,
										161, 165, 169, 173, 177, 181, 185, 189, 193, 197,
										201, 205, 209, 213, 217, 221, 225, 229, 233, 0},								COMMON},
	{132, 132, FREQ_5G94,  BW_40, {0}, {3, 11, 19, 27, 35, 43, 51, 59, 67, 75,
										83, 91, 99, 107, 115, 123, 131, 139, 147, 155,
										163, 171, 179, 187, 195, 203, 211, 219, 227, 0},								COMMON},
	{133, 133, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151,
										167, 183, 199, 215, 0},															COMMON},
	{134, 134, FREQ_5G94,  BW_160, {0}, {15, 47, 79, 111, 143, 175, 207, 0},											COMMON},
	{135, 135, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 0},				PLUS_80},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};


/*
	Table E-2혰Operating classes in Europe (11AC)
*/
REG_CLASS_HE reg_class_he_ce[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},																COMMON},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},																NOMADICBEHAVIOR},
	{3,  121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						COMMON},
	{4,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},									LICENSEEXEMPTBEHAVIOR},
	{5,  116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{6,  119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{7,  122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0}, {0},													PRIMARYCHANNELLOWERBEHAVIOR},
	{8,  117, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{9,  120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{10, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0}, {0},													PRIMARYCHANNELUPPERBEHAVIOR},
	{11,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{12,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{13,   0, FREQ_5G0025, BW_5,  {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{14,   0, FREQ_5G00,   BW_10, {171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 0}, {0},		ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{15,   0, FREQ_5G00,   BW_20, {172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 0}, {0},					ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{16,   0, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						ITS_NONMOBILE_OPERATIONS | ITS_MOBILE_OPERATIONS},
	{17, 125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 169, 0}, {0},												COMMON},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 0},															USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 114, 0},																	USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 0},															PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{131, 131, FREQ_5G94,  BW_20, {0}, {1, 5, 9, 13, 17, 21, 25, 29, 33, 37,
										41, 45, 49, 53, 57, 61, 65, 69, 73, 77,
										81, 85, 89, 93, 97, 101, 105, 109, 113, 117,
										121, 125, 129, 133, 137, 141, 145, 149, 153, 157,
										161, 165, 169, 173, 177, 181, 185, 189, 193, 197,
										201, 205, 209, 213, 217, 221, 225, 229, 233, 0},								COMMON},
	{132, 132, FREQ_5G94,  BW_40, {0}, {3, 11, 19, 27, 35, 43, 51, 59, 67, 75,
										83, 91, 99, 107, 115, 123, 131, 139, 147, 155,
										163, 171, 179, 187, 195, 203, 211, 219, 227, 0},								COMMON},
	{133, 133, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151,
										167, 183, 199, 215, 0},														COMMON},
	{134, 134, FREQ_5G94,  BW_160, {0}, {15, 47, 79, 111, 143, 175, 207, 0},											COMMON},
	{135, 135, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 0},				PLUS_80},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

/*
	Table E-3혰Operating classes in Japan (11N)
*/
REG_CLASS_HE reg_class_he_jp[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {34, 36, 38, 40, 42, 44, 46, 48, 0}, {0},											COMMON},
	{2,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																	COMMON},
	{3,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{4,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{5,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{6,  112, FREQ_5G00,   BW_20, {8, 12, 16, 0}, {0},																COMMON},
	{7,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{8,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{9,  109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{10, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{11, 109, FREQ_4G00,   BW_20, {184, 188, 192, 196, 0}, {0},														COMMON},
	{12, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{13, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{14, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{15, 113, FREQ_5G00,   BW_10, {7, 8, 9, 11, 0}, {0},																COMMON},
	{16, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{17, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{18, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{19, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{20, 110, FREQ_4G00,   BW_10, {183, 184, 185, 187, 188, 189, 0}, {0},											COMMON},
	{21, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{22, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{23, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{24, 114, FREQ_5G0025, BW_5,  {6, 7, 8, 9, 10, 11, 0}, {0},														COMMON},
	{25, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{26, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{27, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{28, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{29, 111, FREQ_4G0025, BW_5,  {182, 183, 184, 185, 186, 187, 188, 189, 0}, {0},									COMMON},
	{30,  81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},									LICENSEEXEMPTBEHAVIOR},
	{31,  82, FREQ_2G414,  BW_25, {14, 0}, {0},																		LICENSEEXEMPTBEHAVIOR},
	{32, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},															COMMON},
	{33, 118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},															COMMON},
	{34, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 0}, {0},						DFS_50_100_BEHAVIOR},
	{35, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						DFS_50_100_BEHAVIOR},
	{36, 116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{37, 119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{38, 119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR},
	{39, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 140, 0}, {0},													PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{40, 122, FREQ_5G00,   BW_40, {100, 108, 116, 124, 132, 0}, {0},													PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{41, 117, FREQ_5G00,   BW_40, {40, 48, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{42, 120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{43, 120, FREQ_5G00,   BW_40, {56, 64, 0}, {0},																		PRIMARYCHANNELUPPERBEHAVIOR},
	{44, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 144, 0}, {0},													PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{45, 123, FREQ_5G00,   BW_40, {104, 112, 120, 128, 136, 0}, {0},													PRIMARYCHANNELUPPERBEHAVIOR | DFS_50_100_BEHAVIOR},
	{46, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{47, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{48, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{49, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{50, 104, FREQ_4G00,   BW_40, {184, 192, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR},
	{51, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{52, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{53, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{54, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{55, 105, FREQ_4G00,   BW_40, {188, 196, 0}, {0},																	PRIMARYCHANNELUPPERBEHAVIOR},
	{56,  83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{57,  84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{58, 121, FREQ_5G00,   BW_20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}, {0},						NOMADICBEHAVIOR | LICENSEEXEMPTBEHAVIOR},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 138, 0},												USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 114, 0},																	USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 106, 122, 0},															PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{131, 131, FREQ_5G94,  BW_20, {0}, {1, 5, 9, 13, 17, 21, 25, 29, 33, 37,
										41, 45, 49, 53, 57, 61, 65, 69, 73, 77,
										81, 85, 89, 93, 97, 101, 105, 109, 113, 117,
										121, 125, 129, 133, 137, 141, 145, 149, 153, 157,
										161, 165, 169, 173, 177, 181, 185, 189, 193, 197,
										201, 205, 209, 213, 217, 221, 225, 229, 233, 0},								COMMON},
	{132, 132, FREQ_5G94,  BW_40, {0}, {3, 11, 19, 27, 35, 43, 51, 59, 67, 75,
										83, 91, 99, 107, 115, 123, 131, 139, 147, 155,
										163, 171, 179, 187, 195, 203, 211, 219, 227, 0},								COMMON},
	{133, 133, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151,
										167, 183, 199, 215, 0},														COMMON},
	{134, 134, FREQ_5G94,  BW_160, {0}, {15, 47, 79, 111, 143, 175, 207, 0},											COMMON},
	{135, 135, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 0},				PLUS_80},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

REG_CLASS_HE reg_class_he_cn[] = {
	{0, 0, 0, 0, {0}, {0}, 0},			/* Invlid entry */
	{1,  115, FREQ_5G00,   BW_20, {36, 40, 44, 48, 0}, {0},																USEEIRPFORVHTTXPOWENV},
	{2,  118, FREQ_5G00,   BW_20, {52, 56, 60, 64, 0}, {0},																DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{3,  125, FREQ_5G00,   BW_20, {149, 153, 157, 161, 165, 0}, {0},                                                    USEEIRPFORVHTTXPOWENV},
	{4,  116, FREQ_5G00,   BW_40, {36, 44, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{5,  119, FREQ_5G00,   BW_40, {52, 60, 0}, {0},																		PRIMARYCHANNELLOWERBEHAVIOR | DFS_50_100_BEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{6,  126, FREQ_5G00,   BW_40, {149, 157, 0}, {0},																	PRIMARYCHANNELLOWERBEHAVIOR | USEEIRPFORVHTTXPOWENV},
	{7,   81, FREQ_2G407,  BW_25, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},									LICENSEEXEMPTBEHAVIOR},
	{8,   83, FREQ_2G407,  BW_40, {1, 2, 3, 4, 5, 6, 7, 8, 9, 0}, {0},													LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELLOWERBEHAVIOR},
	{9,   84, FREQ_2G407,  BW_40, {5, 6, 7, 8, 9, 10, 11, 12, 13, 0}, {0},												LICENSEEXEMPTBEHAVIOR | PRIMARYCHANNELUPPERBEHAVIOR},
	{128, 128, FREQ_5G00,   BW_80, {0}, {42, 58, 155, 0},																USEEIRPFORVHTTXPOWENV},
	{129, 129, FREQ_5G00,   BW_160, {0}, {50, 0},																			USEEIRPFORVHTTXPOWENV},
	{130, 130, FREQ_5G00,   BW_80, {0}, {42, 58, 155, 0},																PLUS_80 | USEEIRPFORVHTTXPOWENV},
	{131, 131, FREQ_5G94,  BW_20, {0}, {1, 5, 9, 13, 17, 21, 25, 29, 33, 37,
										41, 45, 49, 53, 57, 61, 65, 69, 73, 77,
										81, 85, 89, 93, 97, 101, 105, 109, 113, 117,
										121, 125, 129, 133, 137, 141, 145, 149, 153, 157,
										161, 165, 169, 173, 177, 181, 185, 189, 193, 197,
										201, 205, 209, 213, 217, 221, 225, 229, 233, 0},								COMMON},
	{132, 132, FREQ_5G94,  BW_40, {0}, {3, 11, 19, 27, 35, 43, 51, 59, 67, 75,
										83, 91, 99, 107, 115, 123, 131, 139, 147, 155,
										163, 171, 179, 187, 195, 203, 211, 219, 227, 0},								COMMON},
	{133, 133, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151,
										167, 183, 199, 215, 0},														COMMON},
	{134, 134, FREQ_5G94,  BW_160, {0}, {15, 47, 79, 111, 143, 175, 207, 0},											COMMON},
	{135, 135, FREQ_5G94,  BW_80, {0}, {7, 23, 39, 55, 71, 87, 103, 119, 135, 151, 167, 183, 199, 215, 0},				PLUS_80},
	{0, 0, 0, 0, {0}, {0}, 0}			/* end */
};

#endif /* DOT11_VHT_AC */


BOOLEAN is_channel_in_channelset(UCHAR *ChannelSet, UCHAR Channel)
{
	int index = 0;

	while (ChannelSet[index] != 0) {
		/* TODO: Need to consider behavior limits set!! */
		if (ChannelSet[index] == Channel)
			return TRUE;

		index++;
	}

	return FALSE;
}

UCHAR get_channel_set_num(UCHAR *ChannelSet)
{
	int index = 0;

	if (ChannelSet == NULL)
		return 0;

	while (ChannelSet[index] != 0)
		index++;

	return index;
}

PVOID get_reg_table_by_country(UCHAR *CountryCode, USHORT PhyMode)
{
	UCHAR reg_region;
	PVOID reg_class_table = NULL;
	reg_region = GetCountryRegionFromCountryCode(CountryCode);
#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		switch (reg_region) {
		case CE:
			reg_class_table = (PVOID)reg_class_he_ce;
			break;
		case JAP:
			reg_class_table = (PVOID)reg_class_he_jp;
			break;
		case CHN:
			reg_class_table = (PVOID)reg_class_he_cn;
			break;
		case FCC:
		default:
			reg_class_table = (PVOID)reg_class_he_fcc;
			break;
		}
	} else
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		switch (reg_region) {
		case CE:
			reg_class_table = (PVOID)reg_class_vht_ce;
			break;

		case JAP:
			reg_class_table = (PVOID)reg_class_vht_jp;
			break;

		case CHN:
			reg_class_table = (PVOID)reg_class_vht_cn;
			break;

		case FCC:
		default:
			reg_class_table = (PVOID)reg_class_vht_fcc;
			break;
		}
	} else
#endif /* DOT11_VHT_AC */
	{
		switch (reg_region) {
		case CE:
			reg_class_table = (PVOID)reg_class_ce;
			break;

		case JAP:
			reg_class_table = (PVOID)reg_class_jp;
			break;

		case CHN:
			reg_class_table = (PVOID)reg_class_cn;
			break;

		case FCC:
		default:
			reg_class_table = (PVOID)reg_class_fcc;
			break;
		}
	}
	return reg_class_table;
}

VOID *get_reg_class_tab_by_reg_class(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode)
{
	int i = 1; /* skip Invlid entry */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		PREG_CLASS_HE reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		if (reg_class_he) {
			do {
				/* find  channel_set */
				if ((reg_class_he[i].reg_class == RegulatoryClass)
					|| (reg_class_he[i].global_class == RegulatoryClass)) {
						return &reg_class_he[i];
				}
				i++;
			} while (reg_class_he[i].reg_class != 0);
		}
	} else
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		PREG_CLASS_VHT reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		if (reg_class_vht) {
			do {
				/* find  channel_set */
				if ((reg_class_vht[i].reg_class == RegulatoryClass)
					|| (reg_class_vht[i].global_class == RegulatoryClass)) {
					return &reg_class_vht[i];
				}

				i++;
			} while (reg_class_vht[i].reg_class != 0);
		}
	} else
#endif /* DOT11_VHT_AC */
	{
		PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		if (reg_class) {
			do {
				/* find  channel_set */
				if ((reg_class[i].reg_class == RegulatoryClass)
					|| (reg_class[i].global_class == RegulatoryClass)) {
					return &reg_class[i];
				}
				i++;
			} while (reg_class[i].reg_class != 0);
		}
	}

	return NULL;
}

PUCHAR get_channelset_by_reg_class(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode)
{
	PUCHAR channelset = NULL;
	VOID *reg_class_tab = NULL;

	reg_class_tab = get_reg_class_tab_by_reg_class(pAd, RegulatoryClass, PhyMode);

	if (reg_class_tab == NULL)
		return NULL;

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		PREG_CLASS_HE reg_class_he = (PREG_CLASS_HE)reg_class_tab;

		if (RegulatoryClass > 127 && RegulatoryClass < 136)
			channelset = reg_class_he->center_freq;
		else
			channelset = reg_class_he->channel_set;
	} else
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		PREG_CLASS_VHT reg_class_vht = (PREG_CLASS_VHT)reg_class_tab;

		if (RegulatoryClass > 127 && RegulatoryClass < 131) /* reference operating classes table (802.11) */
			channelset = reg_class_vht->center_freq;
		else
			channelset = reg_class_vht->channel_set;
	} else
#endif /* DOT11_VHT_AC */
	{
		PREG_CLASS reg_class = (PREG_CLASS)reg_class_tab;

		/* find  channel_set */
		channelset = reg_class->channel_set;
	}

	return channelset;
}

BOOLEAN get_spacing_by_reg_class(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode,
	OUT UCHAR *spacing)
{
	VOID *reg_class_tab = NULL;
	PREG_CLASS reg_class = NULL;

	reg_class_tab = get_reg_class_tab_by_reg_class(pAd, RegulatoryClass, PhyMode);

	if (reg_class_tab == NULL)
		return FALSE;

	reg_class = (PREG_CLASS)reg_class_tab;

	/* find spacing */
	*spacing = reg_class->spacing; /* all the spacing in the same position in n/ac/ax structure */

	return TRUE;;
}


BOOLEAN is_channel_in_channelset_by_reg_class(
	IN RTMP_ADAPTER * pAd,
	IN UINT8 RegulatoryClass,
	IN USHORT PhyMode,
	IN UCHAR Channel)
{
	PUCHAR channelset = NULL;

	channelset = get_channelset_by_reg_class(pAd, RegulatoryClass, PhyMode);

	if (channelset == NULL)
		return FALSE;

	return is_channel_in_channelset(channelset, Channel);
}


#ifdef DFS_CAC_R2
int get_cac_mode (IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam, struct wifi_dev *wdev)
{
	if (pDfsParam->bDedicatedZeroWaitSupport) {
		return DEDICATED_CAC;
	} else if (pDfsParam->bDedicatedZeroWaitSupport &&
		wlan_config_get_rx_stream(wdev) == RX_MAX_STREAM &&
		wlan_config_get_tx_stream(wdev) == TX_MAX_STREAM) {
		return REDUCED_MIMO_CAC;
	}

	return CONTINUOUS_CAC;
}
void wapp_get_cac_cap(IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	struct cac_capability_lib *cac_cap)
{
	/*TODO: Raghav : Need to re-write this code for harrier.*/
	UCHAR i = 0, j = 0, k = 0, l = 0, m = 0, dfs_ch = 0, channel = 0, band_idx = 0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	BOOLEAN ret = FALSE;
	PCHANNEL_CTRL pChCtrl = NULL;
	UCHAR vht_bw = 0;
	UCHAR ht_bw = 0;

	vht_bw = wlan_config_get_vht_bw(wdev);
	ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_HE_AX
	if ((WMODE_CAP_AX(wdev->PhyMode))) {
		PREG_CLASS_HE reg_class = (PREG_CLASS_HE)get_reg_table_by_country(
			pAd->CommonCfg.CountryCode, wdev->PhyMode);
		if (reg_class) {
			do {
			/* find  channel_set */
				ret = FALSE;
				if (reg_class[i].start_freq == FREQ_5G00) {
					if ((reg_class[i].spacing == BW_80) ||
						(reg_class[i].spacing == BW_160))
						channel = reg_class[i].center_freq[j];
					else
						channel = reg_class[i].channel_set[j];
					for (j = 0; channel != 0;) {
#ifdef DOT11_VHT_AC
					if ((reg_class[i].spacing == BW_80) && (vht_bw >= VHT_BW_80)) {
						ret = RadarChannelCheck(pAd,
							(reg_class[i].center_freq[j] - 2));
					} else if ((reg_class[i].spacing == BW_160) && (vht_bw >= VHT_BW_160)) {
						ret = RadarChannelCheckWrapper160(pAd, reg_class[i].center_freq[j] - 14);
					} else if ((reg_class[i].spacing == BW_40) && (ht_bw >= HT_BW_40)) {
						if ((RadarChannelCheck(pAd, reg_class[i].channel_set[j]) &&
							RadarChannelCheck(pAd, reg_class[i].channel_set[j] + 4) &&
							(reg_class[i].behavior_limit_set & PRIMARYCHANNELLOWERBEHAVIOR)) ||
							(RadarChannelCheck(pAd, reg_class[i].channel_set[j]) &&
							RadarChannelCheck(pAd, reg_class[i].channel_set[j] - 4) &&
							(reg_class[i].behavior_limit_set & PRIMARYCHANNELUPPERBEHAVIOR)))
							ret = TRUE;
						else
							ret = FALSE;
					} else if (reg_class[i].spacing == BW_20)
#endif
					{
						ret = RadarChannelCheck(pAd,
							reg_class[i].channel_set[j]);
					}
					if (ret == TRUE) {
						dfs_ch = TRUE;
						cac_cap->opcap[l].op_class = reg_class[i].global_class;
						cac_cap->opcap[l].ch_num++;
					if ((reg_class[i].spacing == BW_80) ||
						(reg_class[i].spacing == BW_160))
						cac_cap->opcap[l].ch_list[k] =
							reg_class[i].center_freq[j];
					else
						cac_cap->opcap[l].ch_list[k] =
								reg_class[i].channel_set[j];

					if ((pAd->CommonCfg.RDDurRegion == CE) &&
					DfsCacRestrictBandForCentralCh(pAd, wdev,
					reg_class[i].spacing,
					cac_cap->opcap[l].ch_list[k], 0)) {
						cac_cap->opcap[l].cac_time[k] = 650;
					} else
						cac_cap->opcap[l].cac_time[k] = 65;
					band_idx = HcGetBandByWdev(wdev);
					pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
					for (m = 0; m < pChCtrl->ChListNum; m++) {
						if (reg_class[i].spacing == BW_80 || reg_class[i].spacing == BW_160) {
							if (pChCtrl->ChList[m].Channel ==
								reg_class[i].center_freq[j] - 2) {
								cac_cap->opcap[l].non_occupancy_remain[k] =
								pChCtrl->ChList[m].NonOccupancy;
								break;
							}
						} else {
							if (pChCtrl->ChList[m].Channel ==
								reg_class[i].channel_set[j]) {
								cac_cap->opcap[l].non_occupancy_remain[k] =
								pChCtrl->ChList[m].NonOccupancy;
								break;
							}
						}
					}
					k++;
					}
					j++;
					if ((reg_class[i].spacing == BW_80) ||
						(reg_class[i].spacing == BW_160))
						channel = reg_class[i].center_freq[j];
					else
						channel = reg_class[i].channel_set[j];
				}
				k = 0;
				j = 0;
				if (dfs_ch == TRUE) {
					cac_cap->op_class_num += 1;
					l++;
				}
				dfs_ch = FALSE;
				}
			i++;
			} while (reg_class[i].reg_class != 0);
		}
	} else
#endif
	if ((WMODE_CAP_AC(wdev->PhyMode))) {
		PREG_CLASS_VHT reg_class = (PREG_CLASS_VHT)get_reg_table_by_country(
			pAd->CommonCfg.CountryCode, wdev->PhyMode);
		if (reg_class) {
			do {
				ret = FALSE;
			/* find  channel_set */
				if (reg_class[i].start_freq == FREQ_5G00) {
					if ((reg_class[i].spacing == BW_80) ||
						(reg_class[i].spacing == BW_160))
						channel = reg_class[i].center_freq[j];
					else
						channel = reg_class[i].channel_set[j];
					for (j = 0; channel != 0;) {
#ifdef DOT11_VHT_AC
					if ((reg_class[i].spacing == BW_80) && (vht_bw >= VHT_BW_80)) {
						ret = RadarChannelCheck(pAd,
							(reg_class[i].center_freq[j] - 2));
					} else if ((reg_class[i].spacing == BW_160) && (vht_bw >= VHT_BW_160)) {
						ret = RadarChannelCheckWrapper160(pAd, reg_class[i].center_freq[j] - 14);
					} else if ((reg_class[i].spacing == BW_40) && (ht_bw >= HT_BW_40)) {
						if ((RadarChannelCheck(pAd, reg_class[i].channel_set[j]) &&
							RadarChannelCheck(pAd, reg_class[i].channel_set[j] + 4) &&
							(reg_class[i].behavior_limit_set & PRIMARYCHANNELLOWERBEHAVIOR)) ||
							(RadarChannelCheck(pAd, reg_class[i].channel_set[j]) &&
							RadarChannelCheck(pAd, reg_class[i].channel_set[j] - 4) &&
							(reg_class[i].behavior_limit_set & PRIMARYCHANNELUPPERBEHAVIOR)))
							ret = TRUE;
						else
							ret = FALSE;
					} else if (reg_class[i].spacing == BW_20)
#endif
					{
						ret = RadarChannelCheck(pAd,
							reg_class[i].channel_set[j]);
					}
					if (ret == TRUE) {
						dfs_ch = TRUE;
						cac_cap->opcap[l].op_class = reg_class[i].global_class;
						cac_cap->opcap[l].ch_num++;
					if ((reg_class[i].spacing == BW_80) ||
						(reg_class[i].spacing == BW_160))
						cac_cap->opcap[l].ch_list[k] =
							reg_class[i].center_freq[j];
					else
						cac_cap->opcap[l].ch_list[k] =
								reg_class[i].channel_set[j];

					if ((pAd->CommonCfg.RDDurRegion == CE) &&
					DfsCacRestrictBandForCentralCh(pAd, wdev,
					reg_class[i].spacing,
					cac_cap->opcap[l].ch_list[k], 0)) {
						cac_cap->opcap[l].cac_time[k] = 650;
					} else
						cac_cap->opcap[l].cac_time[k] = 65;
					band_idx = HcGetBandByWdev(wdev);
					pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
					for (m = 0; m < pChCtrl->ChListNum; m++) {
						if (reg_class[i].spacing == BW_80 || reg_class[i].spacing == BW_160) {
							if (pChCtrl->ChList[m].Channel ==
								reg_class[i].center_freq[j] - 2) {
								cac_cap->opcap[l].non_occupancy_remain[k] =
								pChCtrl->ChList[m].NonOccupancy;
								break;
							}
						} else {
							if (pChCtrl->ChList[m].Channel ==
								reg_class[i].channel_set[j]) {
								cac_cap->opcap[l].non_occupancy_remain[k] =
								pChCtrl->ChList[m].NonOccupancy;
								break;
							}
						}
					}
					k++;
					}
					j++;
					if ((reg_class[i].spacing == BW_80) ||
						(reg_class[i].spacing == BW_160))
						channel = reg_class[i].center_freq[j];
					else
						channel = reg_class[i].channel_set[j];
				}
				k = 0;
				j = 0;
				if (dfs_ch == TRUE) {
					cac_cap->op_class_num += 1;
					l++;
				}
				dfs_ch = FALSE;
				}
			i++;
			} while (reg_class[i].reg_class != 0);
		}
	} else {
		PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(
			pAd->CommonCfg.CountryCode, wdev->PhyMode);
			do {
				ret = FALSE;
				/* find  channel_set */
				if (reg_class[i].start_freq == FREQ_5G00) {
					channel = reg_class[i].channel_set[j];
					for (j = 0; channel != 0;) {
						if ((reg_class[i].spacing == BW_40) && (ht_bw >= HT_BW_40)) {
							if ((RadarChannelCheck(pAd, reg_class[i].channel_set[j]) &&
								RadarChannelCheck(pAd, reg_class[i].channel_set[j] + 4) &&
								(reg_class[i].behavior_limit_set & PRIMARYCHANNELLOWERBEHAVIOR)) ||
								(RadarChannelCheck(pAd, reg_class[i].channel_set[j]) &&
								RadarChannelCheck(pAd, reg_class[i].channel_set[j] - 4) &&
								(reg_class[i].behavior_limit_set & PRIMARYCHANNELUPPERBEHAVIOR)))
								ret = TRUE;
							else
								ret = FALSE;
						} else if (reg_class[i].spacing == BW_20) {
							ret = RadarChannelCheck(pAd,
								reg_class[i].channel_set[j]);
						}
						if (ret == TRUE) {
							dfs_ch = TRUE;
							cac_cap->opcap[l].op_class =
								reg_class[i].global_class;
							cac_cap->opcap[l].ch_num++;
							cac_cap->opcap[l].ch_list[k] =
								reg_class[i].channel_set[j];
						if ((pAd->CommonCfg.RDDurRegion == CE)
							&& DfsCacRestrictBandForCentralCh(pAd, wdev, reg_class[i].spacing,
										cac_cap->opcap[l].ch_list[k], 0)) {
							cac_cap->opcap[l].cac_time[k] = 650;
						} else
							cac_cap->opcap[l].cac_time[k] = 65;
						band_idx = HcGetBandByWdev(wdev);
						pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
						for (m = 0; m < pChCtrl->ChListNum; m++) {
							if (pChCtrl->ChList[m].Channel ==
								reg_class[i].channel_set[j]) {
								cac_cap->opcap[l].non_occupancy_remain[k] =
								pChCtrl->ChList[m].NonOccupancy;
								break;
							}
						}
					k++;
					}
				j++;
				channel = reg_class[i].channel_set[j];
			}
			k = 0;
			j = 0;
			if (dfs_ch == TRUE) {
				cac_cap->op_class_num += 1;
				l++;
			}
			dfs_ch = FALSE;
			}
			i++;
		} while (reg_class[i].reg_class != 0);
	}
	cac_cap->cac_mode = get_cac_mode(pAd, pDfsParam, wdev);
}
#endif


UINT BW_VALUE[] = {20, 40, 80, 160, 10, 5, 162, 60, 25};
UCHAR get_regulatory_class(RTMP_ADAPTER *pAd, UCHAR Channel, USHORT PhyMode, struct wifi_dev *wdev)
{
	int i = 1; /* skip Invlid entry */
	UCHAR regclass = 0;
	UINT16 bw = BW_20;
	UCHAR cfg_ht_bw = wlan_operate_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	UCHAR cfg_vht_bw = wlan_operate_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
	UCHAR cfg_he_bw = wlan_operate_get_he_bw(wdev);
#endif /*DOT11_HE_AX*/

	UCHAR reg_domain = pAd->reg_domain;
	if (WMODE_CAP_N(PhyMode)) {
		if (cfg_ht_bw)
			bw = BW_40;
		else
			bw = BW_20;
	}

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(PhyMode)) {
		switch (cfg_vht_bw) {
		case VHT_BW_80:
			bw = BW_80;
			break;

		case VHT_BW_160:
			bw = BW_160;
			break;

		case VHT_BW_8080:
			bw = BW_8080;
			break;

		case VHT_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		switch (cfg_he_bw) {
		case HE_BW_80:
			bw = BW_80;
			break;
		case HE_BW_160:
			bw = BW_160;
			break;
		case HE_BW_8080:
			bw = BW_8080;
			break;
		case HE_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}
#endif /* DOT11_HE_AX */


	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
			"Channel=%d,HT_BW=%d bw=%d\n", Channel, cfg_ht_bw, bw);

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		PREG_CLASS_HE reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		UCHAR ch_band = wlan_config_get_ch_band(wdev);
		if (reg_class_he) {

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "ch_band=%d\n", ch_band);

			do {
				if (BW_VALUE[reg_class_he[i].spacing] >= BW_VALUE[bw]) {

					if ((ch_band == CMD_CH_BAND_6G) && (reg_class_he[i].start_freq != FREQ_5G94)) {
						i++;
						continue;
					}

					/* find channel_set */
					if (is_channel_in_channelset(reg_class_he[i].channel_set, Channel) ||
						(is_channel_in_channelset(reg_class_he[i].center_freq, he_cent_ch_freq(Channel, cfg_he_bw, ch_band)))) {
						regclass = (reg_domain == REG_LOCAL) ? \
						reg_class_he[i].reg_class : \
						reg_class_he[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class_he[i].reg_class != 0);
		}
		i = 1;
		if (!regclass) {
			do {
				if (BW_VALUE[reg_class_global[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class_global[i].channel_set, Channel) ||
						(is_channel_in_channelset(reg_class_global[i].center_freq, he_cent_ch_freq(Channel, cfg_he_bw, ch_band)))) {
						regclass = reg_class_global[i].reg_class;
						break;
					}
				}
				i++;
			} while (reg_class_global[i].reg_class != 0);
		}
	} else
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		PREG_CLASS_VHT reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		UCHAR ch_band = wlan_config_get_ch_band(wdev);
		if (reg_class_vht) {

			do {
				if (BW_VALUE[reg_class_vht[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class_vht[i].channel_set, Channel) ||
						(is_channel_in_channelset(reg_class_vht[i].center_freq, vht_cent_ch_freq(Channel, cfg_vht_bw, ch_band)))) {
						regclass = (reg_domain == REG_LOCAL) ? \
						reg_class_vht[i].reg_class : \
						reg_class_vht[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class_vht[i].reg_class != 0);
		}
		i = 1;
		if (!regclass) {
			do {
				if (BW_VALUE[reg_class_global[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class_global[i].channel_set, Channel) ||
						(is_channel_in_channelset(reg_class_global[i].center_freq, vht_cent_ch_freq(Channel, cfg_vht_bw, ch_band)))) {
						regclass = reg_class_global[i].reg_class;
						break;
					}
				}
				i++;
			} while (reg_class_global[i].reg_class != 0);
		}
	} else
#endif /* DOT11_VHT_AC */
	{
		PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		if (reg_class) {
			do {
				if (BW_VALUE[reg_class[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class[i].channel_set, Channel)) {
						regclass = (reg_domain == REG_LOCAL) ? \
						reg_class[i].reg_class : \
						reg_class[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class[i].reg_class != 0);
		}
	}

	/* ASSERT(regclass); */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "regulatory class is %d\n", regclass);
	return regclass;
}

INT get_operating_class_list(RTMP_ADAPTER *pAd, UCHAR Channel, USHORT PhyMode, struct wifi_dev *wdev, UCHAR *SuppClasslist, INT *Len)
{
	UCHAR SupportedClass[] = {81, 83, 84, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129
#ifdef CONFIG_6G_SUPPORT
				, 131
#endif
				};
	memcpy(SuppClasslist, SupportedClass, sizeof(SupportedClass));
	*Len = sizeof(SupportedClass);
	return TRUE;
}

UCHAR get_regulatory_class_for_newCh(RTMP_ADAPTER *pAd, UCHAR NewCh, USHORT PhyMode, struct wifi_dev *wdev)
{
	int i = 1; /* skip Invlid entry */
	UCHAR regclass = 0;
	UCHAR bw = BW_20;
	UCHAR reg_cap_bw = get_channel_bw_cap(wdev, NewCh);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
#ifdef DOT11_HE_AX
	UCHAR cfg_he_bw = wlan_config_get_he_bw(wdev);
#endif /*DOT11_HE_AX*/

	UCHAR reg_domain = pAd->reg_domain;
	if (WMODE_CAP_N(PhyMode)) {
		if (cfg_ht_bw)
			bw = BW_40;
		else
			bw = BW_20;
	}

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(PhyMode)) {
		switch (cfg_vht_bw) {
		case VHT_BW_80:
			bw = BW_80;
			break;

		case VHT_BW_160:
			bw = BW_160;
			break;

		case VHT_BW_8080:
			bw = BW_8080;
			break;

		case VHT_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}

#endif /* DOT11_VHT_AC */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		switch (cfg_he_bw) {
		case HE_BW_80:
			bw = BW_80;
			break;
		case HE_BW_160:
			bw = BW_160;
			break;
		case HE_BW_8080:
			bw = BW_8080;
			break;
		case HE_BW_2040:
			if (cfg_ht_bw == BW_40)
				bw = BW_40;
			else
				bw = BW_20;
		default:
			break;
		}
	}
#endif /* DOT11_HE_AX */

	/* if bw capability of NewCh is lower than .dat bw config, bw should follow reg_cap_bw*/
	if (bw > reg_cap_bw) {
		if (!(bw == BW_8080 && (reg_cap_bw == BW_80 || reg_cap_bw == BW_160))) {
			bw = reg_cap_bw;

		/*after bw is adjusted, cfg_he_bw/cfg_vht_bw also need adjust*/
			switch (bw) {
			case BW_20:
				cfg_vht_bw = VHT_BW_2040;
				cfg_he_bw = HE_BW_2040;
				break;
			case BW_40:
				cfg_vht_bw = VHT_BW_2040;
				cfg_he_bw = HE_BW_2040;
				break;
			case BW_80:
				cfg_vht_bw = VHT_BW_80;
				cfg_he_bw = HE_BW_80;
				break;
			case BW_160:
				cfg_vht_bw = VHT_BW_160;
				cfg_he_bw = HE_BW_160;
			default:
				break;
			}
		}
	}

	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG,
			"Channel=%d,HT_BW=%d bw=%d\n", NewCh, cfg_ht_bw, bw);

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		PREG_CLASS_HE reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
		UCHAR ch_band = wlan_config_get_ch_band(wdev);
		if (reg_class_he) {

			MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "ch_band=%d\n", ch_band);

			do {
				if (BW_VALUE[reg_class_he[i].spacing] >= BW_VALUE[bw]) {

					if ((ch_band == CMD_CH_BAND_6G) && (reg_class_he[i].start_freq != FREQ_5G94)) {
						i++;
						continue;
					}

					/* find channel_set */
					if (is_channel_in_channelset(reg_class_he[i].channel_set, NewCh) ||
						(is_channel_in_channelset(reg_class_he[i].center_freq, he_cent_ch_freq(NewCh, cfg_he_bw, ch_band)))) {
						regclass = (reg_domain == REG_LOCAL) ? \
						reg_class_he[i].reg_class : \
						reg_class_he[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class_he[i].reg_class != 0);
		}
		i = 1;
		if (!regclass) {
			do {
				if (BW_VALUE[reg_class_global[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class_global[i].channel_set, NewCh) ||
						(is_channel_in_channelset(reg_class_global[i].center_freq, he_cent_ch_freq(NewCh, cfg_he_bw, ch_band)))) {
						regclass = reg_class_global[i].reg_class;
						break;
					}
				}
				i++;
			} while (reg_class_global[i].reg_class != 0);
		}
	} else
#endif /* DOT11_HE_AX */
#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(PhyMode)) {
		PREG_CLASS_VHT reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
		UCHAR ch_band = wlan_config_get_ch_band(wdev);
		if (reg_class_vht) {

			do {
				if (BW_VALUE[reg_class_vht[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class_vht[i].channel_set, NewCh) ||
						(is_channel_in_channelset(reg_class_vht[i].center_freq, vht_cent_ch_freq(NewCh, cfg_vht_bw, ch_band)))) {
						regclass = (reg_domain == REG_LOCAL) ? \
						reg_class_vht[i].reg_class : \
						reg_class_vht[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class_vht[i].reg_class != 0);
		}
		i = 1;
		if (!regclass) {
			do {
				if (BW_VALUE[reg_class_global[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class_global[i].channel_set, NewCh) ||
						(is_channel_in_channelset(reg_class_global[i].center_freq, he_cent_ch_freq(NewCh, cfg_he_bw, ch_band)))) {
						regclass = reg_class_global[i].reg_class;
						break;
					}
				}
				i++;
			} while (reg_class_global[i].reg_class != 0);
		}
	} else
#endif /* DOT11_VHT_AC */
	{
		PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		if (reg_class) {
			do {
				if (BW_VALUE[reg_class[i].spacing] >= BW_VALUE[bw]) {
					/* find  channel_set */
					if (is_channel_in_channelset(reg_class[i].channel_set, NewCh)) {
						regclass = (reg_domain == REG_LOCAL) ? \
						reg_class[i].reg_class : \
						reg_class[i].global_class;
						break;
					}
				}
				i++;
			} while (reg_class[i].reg_class != 0);
		}
	}

	/* ASSERT(regclass); */
	MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_DEBUG, "regulatory class is %d\n", regclass);
	return regclass;
}

INT Show_ChannelSet_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	int class_idx = 1; /* skip Invlid entry */
	int Channel = 0, Type = 0, phymode_idx = 0, OpClass = 0;
	RTMP_STRING *arg0_type = NULL, *arg1_channel = NULL, *arg2_phymode_idx = NULL;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);

	arg0_type = strsep(&arg, ":"); /*0: find op class by channel, 1: show channel set */
	arg1_channel = strsep(&arg, ":");
	arg2_phymode_idx = strsep(&arg, ":"); /* 0: AN, 1: AC, 2: AX */

	if (arg0_type == NULL || arg1_channel == NULL || arg2_phymode_idx == NULL) {
		MTWF_PRINT("Invalid parameters\n");
		MTWF_PRINT("TYPE:CHANNEL:ISAC\n");
		return FALSE;
	}

	Type = os_str_tol(arg0_type, 0, 10);
	Channel = os_str_tol(arg1_channel, 0, 10);
	phymode_idx = os_str_tol(arg2_phymode_idx, 0, 10);
	MTWF_PRINT("Type = %d Channel = %d phymode_idx = %d\n", Type, Channel, phymode_idx);

	if (Type != 0) {

		switch (phymode_idx) {
		case 0:
			OpClass = get_regulatory_class(pAd, Channel, WMODE_AN, wdev);
			break;

#ifdef DOT11_VHT_AC
		case 1:
			OpClass = get_regulatory_class(pAd, Channel, WMODE_AC, wdev);
			break;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		case 2:
			OpClass = get_regulatory_class(pAd, Channel, (WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G), wdev);
			break;
#endif /* DOT11_HE_AX */

		default:
			OpClass = get_regulatory_class(pAd, Channel, WMODE_AN, wdev);
			break;
		}

		MTWF_PRINT("OpClass(%d) Channel(%d)\n", OpClass, Channel);

		return TRUE;
	} else {
		MTWF_PRINT("\n%-8s%-8s%-16s\n", "Regclass", "Spacing", "Channelset/CenterFreq");

		switch (phymode_idx) {
		case 0: {
			PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, WMODE_AN);

			do {
				int chset_idx = 0;

				MTWF_PRINT("%-8d%-8d", reg_class[class_idx].reg_class, reg_class[class_idx].spacing);

				while (reg_class[class_idx].channel_set[chset_idx] != 0) {
					MTWF_PRINT("%-2d,", reg_class[class_idx].channel_set[chset_idx]);
					chset_idx++;
				}

				class_idx++;
				MTWF_PRINT("\n");
			} while (reg_class[class_idx].reg_class != 0);

		}
		break;

#ifdef DOT11_VHT_AC
		case 1: {
			PREG_CLASS_VHT reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, WMODE_AC);

			do {
				int chset_idx = 0, cntr_freq_idx = 0;

				MTWF_PRINT("%-8d%-8d", reg_class_vht[class_idx].reg_class, reg_class_vht[class_idx].spacing);

				while (reg_class_vht[class_idx].channel_set[chset_idx] != 0) {
					MTWF_PRINT("%-2d,", reg_class_vht[class_idx].channel_set[chset_idx]);
					chset_idx++;
				}

				MTWF_PRINT("/");

				while (reg_class_vht[class_idx].center_freq[cntr_freq_idx] != 0) {
					MTWF_PRINT("%-2d,", reg_class_vht[class_idx].center_freq[cntr_freq_idx]);
					cntr_freq_idx++;
				}

				class_idx++;
				MTWF_PRINT("\n");
			} while (reg_class_vht[class_idx].reg_class != 0);

		}
		break;
#endif /* DOT11_VHT_AC */
#ifdef DOT11_HE_AX
		case 2: {
			PREG_CLASS_HE reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, (WMODE_AX_24G | WMODE_AX_5G | WMODE_AX_6G));

			do {
				int chset_idx = 0, cntr_freq_idx = 0;

				MTWF_PRINT("%-8d%-8d", reg_class_he[class_idx].reg_class, reg_class_he[class_idx].spacing);

				while (reg_class_he[class_idx].channel_set[chset_idx] != 0) {
					MTWF_PRINT("%-2d,", reg_class_he[class_idx].channel_set[chset_idx]);
					chset_idx++;
				}

				MTWF_PRINT("/");

				while (reg_class_he[class_idx].center_freq[cntr_freq_idx] != 0) {
					MTWF_PRINT("%-2d,", reg_class_he[class_idx].center_freq[cntr_freq_idx]);
					cntr_freq_idx++;
				}

				class_idx++;
				MTWF_PRINT("\n");
			} while (reg_class_he[class_idx].reg_class != 0);
		}
		break;
#endif /* DOT11_HE_AX */

		default: {
			PREG_CLASS reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, WMODE_AN);

			do {
				int chset_idx = 0;

				MTWF_PRINT("%-8d%-8d", reg_class[class_idx].reg_class, reg_class[class_idx].spacing);

				while (reg_class[class_idx].channel_set[chset_idx] != 0) {
					MTWF_PRINT("%-2d,", reg_class[class_idx].channel_set[chset_idx]);
					chset_idx++;
				}

				class_idx++;
				MTWF_PRINT("\n");
			} while (reg_class[class_idx].reg_class != 0);
		}
		break;

		}

	}

	return TRUE;
}

INT Set_Reg_Domain_Proc(
	IN	PRTMP_ADAPTER			pAd,
       IN      RTMP_STRING     *arg)
{
	pAd->reg_domain = simple_strtol(arg, 0, 10);
	/* 0: REG_LOCAL  1:REG_GLOBAL */
	MTWF_PRINT("reg_domain = %u\n", pAd->reg_domain);
	return TRUE;
}

VOID get_reg_class_list_for_6g(
	IN RTMP_ADAPTER * pAd,
	IN USHORT PhyMode,
	PUCHAR reg_class_value)
{
	PREG_CLASS_HE reg_class = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	UINT i = 0, j = 0;

	if (reg_class == NULL) {
		MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_ERROR, "reg_class is NULL for 6G!!!\n");
		return;
	}

	for (i = 1; reg_class[i].reg_class != 0 && j < 5; i++) {
		if ((reg_class[i].reg_class > OP_CLASS_130 &&
					reg_class[i].reg_class <= OP_CLASS_135)) {
			reg_class_value[j] = reg_class[i].reg_class;
			j++;
		}
	}
}

#ifdef CONFIG_MAP_SUPPORT
static UCHAR ChannelPreferredSanity_160M(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR op_class1,
	IN UCHAR op_class2,
	IN UCHAR center_channel)
{
	int i, j, match = 0;
	UCHAR *opChList1 = NULL, *opChList2 = NULL;
	UCHAR opChListLen1, opChListLen2;
	CHANNEL_CTRL *pChCtrl = NULL;
	UCHAR BandIdx = 0;
	UCHAR diff = 0;

	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (op_class1) {
		opChList1 = get_channelset_by_reg_class(pAd, op_class1, wdev->PhyMode);
		opChListLen1 = get_channel_set_num(opChList1);

		for (i = 0; i < opChListLen1; i++) {
			diff = ABS(opChList1[i], center_channel);
			if (diff == 14 || diff == 10 || diff == 6 || diff == 2) {
				for (j = 0; j < pChCtrl->ChListNum; j++) {
					if (opChList1[i] == pChCtrl->ChList[j].Channel)
						match++;
				}
			}
		}
	}

	if (op_class2) {
		opChList2 = get_channelset_by_reg_class(pAd, op_class2, wdev->PhyMode);
		opChListLen2 = get_channel_set_num(opChList2);

		for (i = 0; i < opChListLen2; i++) {
			diff = ABS(opChList2[i], center_channel);
			if (diff == 14 || diff == 10 || diff == 6 || diff == 2) {
				for (j = 0; j < pChCtrl->ChListNum; j++) {
					if (opChList2[i] == pChCtrl->ChList[j].Channel)
						match++;
				}
			}
		}
	}

	if (match == 8)
		return 1;

	return 0;
}

static UCHAR ChannelPreferredSanity_80M(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR op_class,
	IN UCHAR center_channel)
{
	int i, j, match = 0;
	UCHAR *opChList = NULL;
	UCHAR opChListLen;
	CHANNEL_CTRL *pChCtrl = NULL;
	UCHAR BandIdx = 0;
	UCHAR diff = 0;

	opChList = get_channelset_by_reg_class(pAd, op_class, wdev->PhyMode);
	opChListLen = get_channel_set_num(opChList);
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	for (i = 0; i < opChListLen; i++) {
		diff = ABS(opChList[i], center_channel);
		if (diff == 6 || diff == 2) {
			for (j = 0; j < pChCtrl->ChListNum; j++) {
				if (opChList[i] == pChCtrl->ChList[j].Channel)
					match++;
			}
		}
	}

	if (match == 4)
		return 1;

	return 0;
}

static UCHAR ChannelPreferredSanity(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR channel,
	IN UCHAR op_class)
{
	int i;
	UCHAR BandIdx = 0;
	CHANNEL_CTRL *pChCtrl = NULL;

	if (op_class == 128) {
		if (channel == 42)
			return ChannelPreferredSanity_80M(pAd, wdev, 115, channel);
		else if (channel == 58)
			return ChannelPreferredSanity_80M(pAd, wdev, 118, channel);
		else if (channel == 106)
			return ChannelPreferredSanity_80M(pAd, wdev, 121, channel);
		else if (channel == 122)
			return ChannelPreferredSanity_80M(pAd, wdev, 121, channel);
		else if (channel == 138)
			return ChannelPreferredSanity_80M(pAd, wdev, 121, channel);
		else if (channel == 155)
			return ChannelPreferredSanity_80M(pAd, wdev, 125, channel);
	} else if (op_class == 129) {
		if (channel == 50)
			return ChannelPreferredSanity_160M(pAd, wdev, 115, 118, channel);
		else if (channel == 114)
			return ChannelPreferredSanity_160M(pAd, wdev, 121, 0, channel);
	} else {
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (channel == pChCtrl->ChList[i].Channel)
				return 1;
		}
	}

	return 0;
}
#ifdef MAP_6E_SUPPORT
UCHAR map_set_op_class_info_6g(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	wdev_op_class_info_ext *op_class)
{
	UCHAR i = 0, j = 0, k = 0, op_index = 0, seen = 0;
	UCHAR chnNum = 0;
	UCHAR vht_bw;
	UCHAR ht_bw;
	USHORT PhyMode = wdev->PhyMode;
	PREG_CLASS reg_class = NULL;
	PREG_CLASS_VHT reg_class_vht = NULL;
#ifdef DOT11_HE_AX
	PREG_CLASS_HE reg_class_he = NULL;
	PCH_DESC pChDesc;
	UCHAR chValid;
	UCHAR center_freq;
#endif /* DOT11_HE_AX */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else
#endif /* DOT11_HE_AX */
		if (WMODE_CAP_AC(PhyMode))
			reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
		else if (WMODE_CAP_2G(PhyMode) || WMODE_CAP_5G(PhyMode))
			reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
		else
			return 0;

	vht_bw = wlan_config_get_vht_bw(wdev);
	ht_bw = wlan_config_get_ht_bw(wdev);
	for (i = 1;; i++) {
		chnNum = 0;
		seen = 0;
#ifdef DOT11_HE_AX
		if (!(WMODE_CAP_AX(PhyMode)))
			break;

		if (reg_class_he[i].reg_class == 0) {
			/*End of Op Class Table Reached*/
			break;
		}
		for (k = 0; k < op_index; k++) {
			if (reg_class_he[i].global_class == op_class->opClassInfoExt[k].op_class) {
				/*This is a duplicate entry in opclass list*/
				seen = 1;
				break;
			}
		}

		if (seen)
			continue;

		if (vht_bw != VHT_BW_160 && reg_class_he[i].global_class == 134) {
			continue;
		}

		if (vht_bw < VHT_BW_80 && reg_class_he[i].global_class == 133) {
			continue;
		}

		if (vht_bw != VHT_BW_8080 && reg_class_he[i].global_class == 135) {
			continue;
		}

		if (ht_bw != HT_BW_40 && reg_class_he[i].global_class == 132) {
			continue;
		}

		if ((reg_class_he[i].global_class >= 131 &&
			reg_class_he[i].global_class <= 135)){
			op_class->opClassInfoExt[op_index].op_class = reg_class_he[i].global_class;
			for (j = 0; j <= MAX_NUM_OF_CHANNELS; j++) {
				if ((op_class->opClassInfoExt[op_index].op_class == 131 ||
					op_class->opClassInfoExt[op_index].op_class == 132) && reg_class_he[i].center_freq[j] != 0) {
					//if (reg_class_he[i].center_freq[j] != 0) {
						//if (ChannelPreferredSanity(pAd, wdev,
						//	reg_class_he[i].center_freq[j])) {
					if (1) {
						op_class->opClassInfoExt[op_index].ch_list[chnNum] =
							reg_class_he[i].center_freq[j];
						chnNum++;
					}
				} else {
					/*80Mhz centre frequencies*/
					if (reg_class_he[i].center_freq[j] != 0) {
						chValid = 0;
						pChDesc = Country_Region_ChDesc_6GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
						center_freq = reg_class_he[i].center_freq[j];
						while (pChDesc->FirstChannel) {
							if ((center_freq >= pChDesc->FirstChannel) &&
								(center_freq < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
								chValid = 1;
								break;
							}
							pChDesc++;
						}
						if (chValid) {
							op_class->opClassInfoExt[op_index].ch_list[chnNum] =
								reg_class_he[i].center_freq[j];
							chnNum++;
						}
					}
				}
			}
		}
#endif /* DOT11_HE_AX */
		if (chnNum) {
			op_class->opClassInfoExt[op_index].num_of_ch = chnNum;
			op_index++;
		}
	}
	return op_index;
}

UCHAR map_set_op_class_info(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	wdev_op_class_info_ext *op_class)
{
	UCHAR i = 0, j = 0, k = 0, op_index = 0, seen = 0;
	UCHAR chnNum = 0;
	UCHAR vht_bw = 0;
	UCHAR ht_bw = 0;
	USHORT PhyMode = wdev->PhyMode;
	PREG_CLASS reg_class = NULL;
	PREG_CLASS_VHT reg_class_vht = NULL;
#ifdef DOT11_HE_AX
	PREG_CLASS_HE reg_class_he = NULL;
	PCH_DESC pChDesc;
	UCHAR chValid;
	UCHAR center_freq;
#endif /* DOT11_HE_AX */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else
#endif /* DOT11_HE_AX */
	if (WMODE_CAP_AC(PhyMode)) {
		reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else if (WMODE_CAP_2G(PhyMode) || WMODE_CAP_5G(PhyMode)) {
		reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else
		return 0;

	vht_bw = wlan_config_get_vht_bw(wdev);
	ht_bw = wlan_config_get_ht_bw(wdev);
	for (i = 1;; i++) {
		chnNum = 0;
		seen = 0;
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(PhyMode)) {
			if (reg_class_he[i].reg_class == 0) {
				/*End of Op Class Table Reached*/
				break;
			}
			for (k = 0; k < op_index; k++) {
				if (reg_class_he[i].global_class == op_class->opClassInfoExt[k].op_class) {
					/*This is a duplicate entry in opclass list*/
					seen = 1;
					break;
				}
			}
			if (seen) {
				continue;
			}

			if (vht_bw != VHT_BW_160 && reg_class_he[i].global_class == 129) {
				continue;
			}

			if (vht_bw < VHT_BW_80 && reg_class_he[i].global_class == 128) {
				continue;
			}

			if (vht_bw != VHT_BW_8080 && reg_class_he[i].global_class == 130) {
				continue;
			}

			if (ht_bw != HT_BW_40 && ((reg_class_he[i].global_class == 116) || (reg_class_he[i].global_class == 117) ||
				(reg_class_he[i].global_class == 119) || (reg_class_he[i].global_class == 120) ||
				(reg_class_he[i].global_class == 122) || (reg_class_he[i].global_class == 123) ||
				(reg_class_he[i].global_class == 126) || (reg_class_he[i].global_class == 127) ||
				(reg_class_he[i].global_class == 83) || (reg_class_he[i].global_class == 84))) {
				continue;
			}

			if ((reg_class_he[i].global_class >= OP_CLASS_81 &&
				reg_class_he[i].global_class <= OP_CLASS_84) ||
				(reg_class_he[i].global_class >= OP_CLASS_115 &&
				reg_class_he[i].global_class <= OP_CLASS_130)){
				op_class->opClassInfoExt[op_index].op_class = reg_class_he[i].global_class;
				for (j = 0; j <= 14; j++) {
					/*Check for 20Mhz and 40Mhz classes only, otherwise channel sanity will fail*/
					if (op_class->opClassInfoExt[op_index].op_class <= OP_CLASS_127) {
						if (reg_class_he[i].channel_set[j] != 0) {
							if (ChannelPreferredSanity(pAd, wdev, reg_class_he[i].channel_set[j],
									reg_class_he[i].global_class)) {
								if ((UNII4BandSupport(pAd) == FALSE) &&
									op_class->opClassInfoExt[op_index].op_class > OP_CLASS_125 &&
									reg_class_he[i].channel_set[j] >= 163) {
									continue;
								}
								op_class->opClassInfoExt[op_index].ch_list[chnNum] =
									reg_class_he[i].channel_set[j];
								chnNum++;
							}
						}
					} else {
						/*80Mhz centre frequencies*/
						if (reg_class_he[i].center_freq[j] != 0 && ChannelPreferredSanity(pAd, wdev,
								reg_class_he[i].center_freq[j], reg_class_he[i].global_class)) {
							chValid = 0;
							pChDesc = Country_Region_ChDesc_5GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
							center_freq = reg_class_he[i].center_freq[j];
							while (pChDesc->FirstChannel) {
								if ((center_freq >= pChDesc->FirstChannel) &&
									(center_freq < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
									if (op_class->opClassInfoExt[op_index].op_class == OP_CLASS_129 &&
										pChDesc->NumOfCh < 8) {
										pChDesc++;
										continue;
									}
									chValid = 1;
									break;
								}
								pChDesc++;
							}
							if ((UNII4BandSupport(pAd) == FALSE) && (center_freq >= 163) && (reg_class_he[i].global_class > OP_CLASS_125))
								chValid = 0;
							if (chValid) {
								op_class->opClassInfoExt[op_index].ch_list[chnNum] =
									reg_class_he[i].center_freq[j];
								chnNum++;
							}
						}
					}
				}
			}
		} else
#endif /* DOT11_HE_AX */
		if (WMODE_CAP_AC(PhyMode)) {
			if (reg_class_vht[i].reg_class == 0) {
				/*End of Op Class Table Reached*/
				break;
			}
			for (k = 0; k < op_index; k++) {
				if (reg_class_vht[i].global_class == op_class->opClassInfoExt[k].op_class) {
					/*This is a duplicate entry in opclass list*/
					seen = 1;
					break;
				}
			}
			if (seen) {
				continue;
			}
			if ((reg_class_vht[i].global_class >= OP_CLASS_81 &&
				reg_class_vht[i].global_class <= OP_CLASS_84) ||
				(reg_class_vht[i].global_class >= OP_CLASS_115 &&
				reg_class_vht[i].global_class <= OP_CLASS_130)){
				op_class->opClassInfoExt[op_index].op_class = reg_class_vht[i].global_class;
				for (j = 0; j <= 14; j++) {
					/*Check for 20Mhz and 40Mhz classes only, otherwise channel sanity will fail*/
					if (op_class->opClassInfoExt[op_index].op_class <= OP_CLASS_127) {
						if (reg_class_vht[i].channel_set[j] != 0) {
							if (ChannelPreferredSanity(pAd, wdev,
								reg_class_vht[i].channel_set[j], reg_class_vht[i].global_class)) {
								op_class->opClassInfoExt[op_index].ch_list[chnNum] =
									reg_class_vht[i].channel_set[j];
								chnNum++;
							}
						}
					} else {
						/*80Mhz centre frequencies*/
						if (reg_class_vht[i].center_freq[j] != 0) {
								op_class->opClassInfoExt[op_index].ch_list[chnNum] =
									reg_class_vht[i].center_freq[j];
								chnNum++;
						}
					}
				}
			}
		} else if ((WMODE_CAP_2G(PhyMode) ||
			WMODE_CAP_5G(PhyMode))) {
			if (reg_class[i].reg_class == 0) {
				break;
			}
			for (k = 0; k < op_index; k++) {
				if (reg_class[i].global_class == op_class->opClassInfoExt[k].op_class) {
					seen = 1;
					break;
				}
			}
			if (seen) {
				continue;
			}
			if ((reg_class[i].global_class >= OP_CLASS_115 &&
				reg_class[i].global_class <= OP_CLASS_130) ||
				(reg_class[i].global_class >= OP_CLASS_81 &&
				reg_class[i].global_class <= OP_CLASS_84)){
				op_class->opClassInfoExt[op_index].op_class = reg_class[i].global_class;
				for (j = 0; j <= 14; j++) {
					if (reg_class[i].channel_set[j] != 0) {
						if (ChannelPreferredSanity(pAd, wdev,
							reg_class[i].channel_set[j], reg_class[i].global_class)) {
							op_class->opClassInfoExt[op_index].ch_list[chnNum] =
								reg_class[i].channel_set[j];
							chnNum++;
						}
					}
				}
			}
		}
		if (chnNum) {
			op_class->opClassInfoExt[op_index].num_of_ch = chnNum;
			op_index++;
		}
	}
	return op_index;
}
#else
UCHAR map_set_op_class_info(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	wdev_op_class_info *op_class)
{
	UCHAR i = 0, j = 0, k = 0, op_index = 0, seen = 0;
	UCHAR chnNum = 0;
	UCHAR vht_bw = 0;
	USHORT PhyMode = wdev->PhyMode;
	PREG_CLASS reg_class = NULL;
	PREG_CLASS_VHT reg_class_vht = NULL;
#ifdef DOT11_HE_AX
	PREG_CLASS_HE reg_class_he = NULL;
	PCH_DESC pChDesc;
	UCHAR chValid;
	UCHAR center_freq;
#endif /* DOT11_HE_AX */

#ifdef DOT11_HE_AX
	if (WMODE_CAP_AX(PhyMode)) {
		reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else
#endif /* DOT11_HE_AX */
	if (WMODE_CAP_AC(PhyMode)) {
		reg_class_vht = (PREG_CLASS_VHT)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else if (WMODE_CAP_2G(PhyMode) || WMODE_CAP_5G(PhyMode)) {
		reg_class = (PREG_CLASS)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);
	} else
		return 0;

	vht_bw = wlan_config_get_vht_bw(wdev);
	for (i = 1;; i++) {
		chnNum = 0;
		seen = 0;
#ifdef DOT11_HE_AX
		if (WMODE_CAP_AX(PhyMode)) {
			if (reg_class_he[i].reg_class == 0) {
				/*End of Op Class Table Reached*/
				break;
			}
			for (k = 0; k < op_index; k++) {
				if (reg_class_he[i].global_class == op_class->opClassInfo[k].op_class) {
					/*This is a duplicate entry in opclass list*/
					seen = 1;
					break;
				}
			}
			if (seen) {
				continue;
			}

			if (vht_bw != VHT_BW_160 && reg_class_he[i].global_class == 129) {
				continue;
			}

			if (vht_bw < VHT_BW_80 && reg_class_he[i].global_class == 128) {
				continue;
			}
			if ((reg_class_he[i].global_class >= OP_CLASS_81 &&
				reg_class_he[i].global_class <= OP_CLASS_84) ||
				(reg_class_he[i].global_class >= OP_CLASS_115 &&
				reg_class_he[i].global_class <= OP_CLASS_130)){
				op_class->opClassInfo[op_index].op_class = reg_class_he[i].global_class;
				for (j = 0; j <= 14; j++) {
					/*Check for 20Mhz and 40Mhz classes only, otherwise channel sanity will fail*/
					if (op_class->opClassInfo[op_index].op_class <= OP_CLASS_127) {
						if (reg_class_he[i].channel_set[j] != 0) {
							if (ChannelPreferredSanity(pAd, wdev, reg_class_he[i].channel_set[j],
								reg_class_he[i].global_class)) {
								if ((UNII4BandSupport(pAd) == FALSE) &&
									op_class->opClassInfo[op_index].op_class > OP_CLASS_125 &&
									reg_class_he[i].channel_set[j] >= 163) {
									continue;
								}
								op_class->opClassInfo[op_index].ch_list[chnNum] =
									reg_class_he[i].channel_set[j];
								chnNum++;
							}
						}
					} else {
						/*80Mhz centre frequencies*/
						if (reg_class_he[i].center_freq[j] != 0) {
							chValid = 0;
							pChDesc = Country_Region_ChDesc_5GHZ[pAd->CommonCfg.CountryRegionForABand].pChDesc;
							center_freq = reg_class_he[i].center_freq[j];
							while(pChDesc->FirstChannel) {
								if ((center_freq >= pChDesc->FirstChannel) &&
									(center_freq < (pChDesc->FirstChannel + (4*pChDesc->NumOfCh)))) {
									if (op_class->opClassInfo[op_index].op_class == OP_CLASS_129 &&
										pChDesc->NumOfCh < 8) {
										pChDesc++;
										continue;
									}
									chValid = 1;
									break;
								}
								pChDesc++;
							}
							if ((UNII4BandSupport(pAd) == FALSE) && (center_freq >= 163) && (reg_class_he[i].global_class > OP_CLASS_125))
								chValid = 0;
							if(chValid){
								op_class->opClassInfo[op_index].ch_list[chnNum] =
									reg_class_he[i].center_freq[j];
								chnNum++;
							}
						}
					}
				}
			}
		} else
#endif /* DOT11_HE_AX */
		if (WMODE_CAP_AC(PhyMode)) {
			if (reg_class_vht[i].reg_class == 0) {
				/*End of Op Class Table Reached*/
				break;
			}
			for (k = 0; k < op_index; k++) {
				if (reg_class_vht[i].global_class == op_class->opClassInfo[k].op_class) {
					/*This is a duplicate entry in opclass list*/
					seen = 1;
					break;
				}
			}
			if (seen) {
				continue;
			}
			if ((reg_class_vht[i].global_class >= OP_CLASS_81 &&
				reg_class_vht[i].global_class <= OP_CLASS_84) ||
				(reg_class_vht[i].global_class >= OP_CLASS_115 &&
				reg_class_vht[i].global_class <= OP_CLASS_130)){
				op_class->opClassInfo[op_index].op_class = reg_class_vht[i].global_class;
				for (j = 0; j <= 14; j++) {
					/*Check for 20Mhz and 40Mhz classes only, otherwise channel sanity will fail*/
					if (op_class->opClassInfo[op_index].op_class <= OP_CLASS_127) {
						if (reg_class_vht[i].channel_set[j] != 0) {
							if (ChannelPreferredSanity(pAd, wdev,
								reg_class_vht[i].channel_set[j], reg_class_vht[i].global_class)) {
								op_class->opClassInfo[op_index].ch_list[chnNum] =
									reg_class_vht[i].channel_set[j];
								chnNum++;
							}
						}
					} else {
						/*80Mhz centre frequencies*/
						if (reg_class_vht[i].center_freq[j] != 0) {
								op_class->opClassInfo[op_index].ch_list[chnNum] =
									reg_class_vht[i].center_freq[j];
								chnNum++;
						}
					}
				}
			}
		} else if ((WMODE_CAP_2G(PhyMode) ||
			WMODE_CAP_5G(PhyMode))) {
			if (reg_class[i].reg_class == 0) {
				break;
			}
			for (k = 0; k < op_index; k++) {
				if (reg_class[i].global_class == op_class->opClassInfo[k].op_class) {
					seen = 1;
					break;
				}
			}
			if (seen) {
				continue;
			}
			if ((reg_class[i].global_class >= OP_CLASS_115 &&
				reg_class[i].global_class <= OP_CLASS_130) ||
				(reg_class[i].global_class >= OP_CLASS_81 &&
				reg_class[i].global_class <= OP_CLASS_84)){
				op_class->opClassInfo[op_index].op_class = reg_class[i].global_class;
				for (j = 0; j <= 14; j++) {
					if (reg_class[i].channel_set[j] != 0) {
						if (ChannelPreferredSanity(pAd, wdev,
							reg_class[i].channel_set[j], reg_class[i].global_class)) {
							op_class->opClassInfo[op_index].ch_list[chnNum] =
								reg_class[i].channel_set[j];
							chnNum++;
						}
					}
				}
			}
		}
		if (chnNum) {
			op_class->opClassInfo[op_index].num_of_ch = chnNum;
			op_index++;
		}
	}
	return op_index;
}
#endif /* MAP_6E_SUPPORT */

#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11_HE_AX
BOOLEAN is_ru26_disable_channel(RTMP_ADAPTER *pAd, UCHAR Channel, USHORT PhyMode)
{
	int i = 1; /* skip Invlid entry */

	if (WMODE_CAP_AX(PhyMode)) {
		PREG_CLASS_HE reg_class_he = (PREG_CLASS_HE)get_reg_table_by_country(pAd->CommonCfg.CountryCode, PhyMode);

		if (reg_class_he) {
			do {
				/* find  channel_set */
				if (is_channel_in_channelset(reg_class_he[i].channel_set, Channel)) {
					if ((reg_class_he[i].behavior_limit_set != COMMON) && (reg_class_he[i].behavior_limit_set & DFS_50_100_BEHAVIOR) != 0) {
						MTWF_DBG(pAd, DBG_CAT_CHN, CATCHN_CHN, DBG_LVL_INFO,
						 "Channel=%d, CountryCode %s, reg_class %d  hit RU26 disable\n",
						  Channel, pAd->CommonCfg.CountryCode, reg_class_he[i].reg_class);
						return TRUE;
					}
				}
				i++;
			} while (reg_class_he[i].reg_class != 0);
		}
	}

	return FALSE;
}

#endif /* DOT11_HE_AX */
#endif /* DOT11_N_SUPPORT */


