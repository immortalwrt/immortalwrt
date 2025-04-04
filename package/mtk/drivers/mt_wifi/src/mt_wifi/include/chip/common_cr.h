#ifndef __COMMON_CR_H__
#define __COMMON_CR_H__
/*
WTBL definition
Copy form BORA hal_wtbl_rom.c
*/

typedef enum _ENUM_WTBL_TYPE_T {
	WTBL_TYPE_LMAC = 0,	/** WTBL in LMAC */
	WTBL_TYPE_UMAC = 1,	/** WTBL in UMAC */
	WTBL_TYPE_KEY = 2,	/** Key Table */
	MAX_NUM_WTBL_TYPE
} ENUM_WTBL_TYPE_T;

typedef enum _ENUM_OWNERSHIP_CR_TYPE_T {
	OWNERSHIP_CR_TYPE_OWN = 0,
	OWNERSHIP_CR_TYPE_OWN_INT_STS = 1,
	OWNERSHIP_CR_TYPE_NUM
} ENUM_OWNERSHIP_CR_TYPE_T;

#define LWTBL_LEN_IN_DW 32
#define UWTBL_LEN_IN_DW 8

#define IO_R_32(_addr) io_r_32(pAd, _addr)
#define IO_W_32(_addr, _val) RTMP_IO_WRITE32(pAd->hdev_ctrl, _addr, _val)

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define WIFI_LWTBL_BASE                 WF_WTBLON_TOP_BASE
#define WIFI_UWTBL_BASE                 WF_UWTBL_TOP_BASE


#define CONFIG_WIFI_RAM_HW_WFDMA        1
#define CONFIG_WIFI_RAM_RU_NUM_PARA     32
#define CFG_SUPPORT_MURU                1
#define MAX_RUSCORE_RECORD_NUM          7
#define MAX_TID_NUM                     8
#define CFG_RU_STA_SORTING              1
#define RAM_BAND_NUM                    2
#define CFG_STA_REC_NUM                 MAX_LEN_OF_MAC_TABLE
#define CFG_MURU_MIPS_ESTIMATION        0
#define MAX_MU_NUM_PER_PPDU             16
#define MU_ENTRY_MAX                    175
#define MAX_BA_GRP                      2


/***** WTBL(LMAC) *****/
/* WTBL Group - Peer Information */
/* DW 0 */
#define WTBL_MUAR_IDX_MASK              BITS(16, 21)
#define WTBL_MUAR_IDX_OFFSET            16
#define WTBL_RCA1                       BIT(22)
#define WTBL_KID_MASK                   BITS(23, 24)
#define WTBL_KID_OFFSET                 23
#define WTBL_RCID                       BIT(25)
#define WTBL_FROM_DS                    BIT(26)
#define WTBL_TO_DS                      BIT(27)
#define WTBL_RV                         BIT(28)
#define WTBL_RCA2                       BIT(29)
#define WTBL_WPI_FLAG                   BIT(30)


/* WTBL Group - TxRx Capability/Information */
/* DW 2 */
#define WTBL_AID12_MASK                 BITS(0, 11)
#define WTBL_AID12_OFFSET               0
#define WTBL_GID_SU                     BIT(12)
#define WTBL_SPP_EN                     BIT(13)
#define WTBL_WPI_EVEN                   BIT(14)
#define WTBL_AAD_OM                     BIT(15)
#define WTBL_CIPHER_SUITE_MASK          BITS(16, 20)
#define WTBL_CIPHER_SUITE_OFFSET        16
#define WTBL_CIPHER_SUITE_IGTK_MASK     BITS(21, 22)
#define WTBL_CIPHER_SUITE_IGTK_OFFSET   21
#define WTBL_SW                         BIT(24)
#define WTBL_UL                         BIT(25)
#define WTBL_TX_POWER_SAVE_STATUS       BIT(26)
#define WTBL_QOS                        BIT(27)
#define WTBL_HT                         BIT(28)
#define WTBL_VHT                        BIT(29)
#define WTBL_HE                         BIT(30)
#define WTBL_MESH                       BIT(31)

/* DW 3 */
#define WTBL_WMM_Q_MASK                 BITS(0, 1)
#define WTBL_WMM_Q_OFFSET               0
#define WTBL_RXD_DUP_MODE_MASK          BITS(2, 3)
#define WTBL_RXD_DUP_MODE_OFFSET        2
#define WTBL_VLAN2ETH                   BIT(4)
#define WTBL_BEAM_CHG                   BIT(5)
#define WTBL_DIS_BA256                  BIT(6)
#define WTBL_PFMU_IDX_MASK              BITS(8, 15)
#define WTBL_PFMU_IDX_OFFSET            8
#define WTBL_ULPF_IDX_MASK              BITS(16, 23)
#define WTBL_ULPF_IDX_OFFSET            16
#define WTBL_RIBF                       BIT(24)
#define WTBL_ULPF                       BIT(25)
#define WTBL_IGN_FBK                    BIT(26)
#define WTBL_TBF                        BIT(29)
#define WTBL_TBF_VHT                    BIT(30)
#define WTBL_TBF_HE                     BIT(31)

/* DW 4 */
#define WTBL_ANT_ID_MASK                BITS(0, 23)
#define WTBL_ANT_ID_STS0_MASK           BITS(0, 2)
#define WTBL_ANT_ID_STS0_OFFSET         0
#define WTBL_ANT_ID_STS1_MASK           BITS(3, 5)
#define WTBL_ANT_ID_STS1_OFFSET         3
#define WTBL_ANT_ID_STS2_MASK           BITS(6, 8)
#define WTBL_ANT_ID_STS2_OFFSET         6
#define WTBL_ANT_ID_STS3_MASK           BITS(9, 11)
#define WTBL_ANT_ID_STS3_OFFSET         9
#define WTBL_ANT_ID_STS4_MASK           BITS(12, 14)
#define WTBL_ANT_ID_STS4_OFFSET         12
#define WTBL_ANT_ID_STS5_MASK           BITS(15, 17)
#define WTBL_ANT_ID_STS5_OFFSET         15
#define WTBL_ANT_ID_STS6_MASK           BITS(18, 20)
#define WTBL_ANT_ID_STS6_OFFSET         18
#define WTBL_ANT_ID_STS7_MASK           BITS(21, 23)
#define WTBL_ANT_ID_STS7_OFFSET         21
#define WTBL_CASCAD                     BIT(24)
#define WTBL_LDPC_HT                    BIT(25)
#define WTBL_LDPC_HT_OFFSET             25
#define WTBL_LDPC_VHT                   BIT(26)
#define WTBL_LDPC_VHT_OFFSET            26
#define WTBL_LDPC_HE                    BIT(27)
#define WTBL_LDPC_HE_OFFSET             27
#define WTBL_DIS_RHTR                   BIT(28)
#define WTBL_ALL_ACK                    BIT(29)
#define WTBL_DROP                       BIT(30)
#define WTBL_ACK_EN                     BIT(31)

/* DW 5 */
#define WTBL_AF_MASK                    BITS(0, 2)
#define WTBL_AF_OFFSET                  0
#define WTBL_AF_HE_MASK                 BITS(3, 4)
#define WTBL_AF_HE_OFFSET               3
#define WTBL_RTS                        BIT(5)
#define WTBL_SMPS                       BIT(6)
#define WTBL_DYN_BW                     BIT(7)
#define WTBL_MMSS_MASK                  BITS(8, 10)
#define WTBL_MMSS_OFFSET                8
#define WTBL_USR                        BIT(11)
#define WTBL_SR_RATE_MASK               BITS(12, 14)
#define WTBL_SR_RATE_OFFSET             12
#define WTBL_SR_ABORT                   BIT(15)
#define WTBL_TX_POWER_OFFSET_MASK       BITS(16, 21)
#define WTBL_TX_POWER_OFFSET_OFFSET     16
#define WTBL_MPDU_SIZE_MASK             BITS(22, 23)
#define WTBL_MPDU_SIZE_OFFSET           22
#define WTBL_PE_MASK                    BITS(24, 25)
#define WTBL_PE_OFFSET                  24
#define WTBL_DOPPL                      BIT(26)
#define WTBL_TXOP_PS_CAP                BIT(27)
#define WTBL_DONOT_UPDATE_I_PSM         BIT(28)
#define WTBL_I_PSM                      BIT(29)
#define WTBL_PSM                        BIT(30)
#define WTBL_SKIP_TX                    BIT(31)

/* DW 6 */
#define WTBL_BA_WIN_SIZE_TID_MASK       BITS(0, 3)
#define WTBL_BA_WIN_SIZE_TID_LEN        4
#define WTBL_BA_WIN_SIZE_TID_MASK_ALL   BITS(0, 31)

/* DW 7 */
#define WTBL_CBRN_MASK                  BITS(0, 2)
#define WTBL_CBRN_OFFSET                0
#define WTBL_DBNSS_EN                   BIT(3)
#define WTBL_BAF_EN                     BIT(4)
#define WTBL_RDGBA                      BIT(5)
#define WTBL_RDG                        BIT(6)
#define WTBL_SPE_IDX_MASK               BITS(7, 11)
#define WTBL_SPE_IDX_OFFSET             7
#define WTBL_G2                         BIT(12)
#define WTBL_G2_OFFSET                  12
#define WTBL_G4                         BIT(13)
#define WTBL_G4_OFFSET                  13
#define WTBL_G8                         BIT(14)
#define WTBL_G8_OFFSET                  14
#define WTBL_G16                        BIT(15)
#define WTBL_G16_OFFSET                 15
#define WTBL_G2_LTF_MASK                BITS(16, 17)
#define WTBL_G2_LTF_OFFSET              16
#define WTBL_G4_LTF_MASK                BITS(18, 19)
#define WTBL_G4_LTF_OFFSET              18
#define WTBL_G8_LTF_MASK                BITS(20, 21)
#define WTBL_G8_LTF_OFFSET              20
#define WTBL_G16_LTF_MASK               BITS(22, 23)
#define WTBL_G16_LTF_OFFSET             22
#define WTBL_G2_HE_MASK                 BITS(24, 25)
#define WTBL_G2_HE_OFFSET               24
#define WTBL_G4_HE_MASK                 BITS(26, 27)
#define WTBL_G4_HE_OFFSET               26
#define WTBL_G8_HE_MASK                 BITS(28, 29)
#define WTBL_G8_HE_OFFSET               28
#define WTBL_G16_HE_MASK                BITS(30, 31)
#define WTBL_G16_HE_OFFSET              30

/* WTBL Group - Auto Rate */
/* DW 8 */
#define WTBL_FAIL_CNT_AC0_MASK          BITS(0, 4)
#define WTBL_FAIL_CNT_AC0_OFFSET        0
#define WTBL_FAIL_CNT_AC1_MASK          BITS(5, 9)
#define WTBL_FAIL_CNT_AC1_OFFSET        5
#define WTBL_FAIL_CNT_AC2_MASK          BITS(10, 14)
#define WTBL_FAIL_CNT_AC2_OFFSET        10
#define WTBL_FAIL_CNT_AC3_MASK          BITS(15, 19)
#define WTBL_FAIL_CNT_AC3_OFFSET        15
#define WTBL_PARTIAL_AID_MASK           BITS(20, 28)
#define WTBL_PARTIAL_AID_OFFSET         20
#define WTBL_CHK_PER                    BIT(31)

/* DW 9 */
#define WTBL_RX_AVG_MPDU_SIZE_MASK      BITS(0, 13)
#define WTBL_RX_AVG_MPDU_SIZE_OFFSET    0
#define WTBL_PRITX_SW_MODE              BIT(16)
#define WTBL_PRITX_PLR                  BIT(17)
#define WTBL_PRITX_DCM                  BIT(18)
#define WTBL_PRITX_ER160                BIT(19)
#define WTBL_PRITX_ERSU                 BIT(20)
#define WTBL_FCAP_20_TO_160_MHZ         BITS(21, 22)
#define WTBL_FCAP_20_TO_160_MHZ_OFFSET  21
#define WTBL_MPDU_FAIL_CNT_MASK         BITS(23, 25)
#define WTBL_MPDU_FAIL_CNT_OFFSET       23
#define WTBL_MPDU_OK_CNT_MASK           BITS(26, 28)
#define WTBL_MPDU_OK_CNT_OFFSET         26
#define WTBL_RATE_IDX_MASK              BITS(29, 31)
#define WTBL_RATE_IDX_OFFSET            29

/* DW 10*/
#define WTBL_RATE1_MASK                 BITS(0, 13)
#define WTBL_RATE1_OFFSET               0
#define WTBL_RATE2_MASK                 BITS(16, 29)
#define WTBL_RATE2_OFFSET               16

/* DW 11 */
#define WTBL_RATE3_MASK                 BITS(0, 13)
#define WTBL_RATE3_OFFSET               0
#define WTBL_RATE4_MASK                 BITS(16, 29)
#define WTBL_RATE4_OFFSET               16

/* DW 12 */
#define WTBL_RATE5_MASK                 BITS(0, 13)
#define WTBL_RATE5_OFFSET               0
#define WTBL_RATE6_MASK                 BITS(16, 29)
#define WTBL_RATE6_OFFSET               16

/* DW 13 */
#define WTBL_RATE7_MASK                 BITS(0, 13)
#define WTBL_RATE7_OFFSET               0
#define WTBL_RATE8_MASK                 BITS(16, 29)
#define WTBL_RATE8_OFFSET               16


/* WTBL Group - Rate Counter */
/* DW 14 */
#define WTBL_RATE1_TX_CNT_MASK          BITS(0, 15)
#define WTBL_RATE1_TX_CNT_OFFSET        0
#define WTBL_RATE1_FAIL_CNT_MASK        BITS(16, 31)
#define WTBL_RATE1_FAIL_CNT_OFFSET      16

/* DW 15 */
#define WTBL_RATE2_OK_CNT_MASK          BITS(0, 15)
#define WTBL_RATE2_OK_CNT_OFFSET        0
#define WTBL_RATE3_OK_CNT_MASK          BITS(16, 31)
#define WTBL_RATE3_OK_CNT_OFFSET        16

/* DW 16 */
#define WTBL_CURRENT_BW_TX_CNT_MASK     BITS(0, 15)
#define WTBL_CURRENT_BW_TX_CNT_OFFSET   0
#define WTBL_CURRENT_BW_FAIL_CNT_MASK   BITS(16, 31)
#define WTBL_CURRENT_BW_FAIL_CNT_OFFSET 16

/* DW 17 */
#define WTBL_OTHER_BW_TX_CNT_MASK       BITS(0, 15)
#define WTBL_OTHER_BW_TX_CNT_OFFSET     0
#define WTBL_OTHER_BW_FAIL_CNT_MASK     BITS(16, 31)
#define WTBL_OTHER_BW_FAIL_CNT_OFFSET   16

/* WTBL Group - PPDU Counter */
/* DW 18 */
#define WTBL_RTS_OK_CNT_MASK            BITS(0, 15)
#define WTBL_RTS_OK_CNT_OFFSET          0
#define WTBL_RTS_FAIL_CNT_MASK          BITS(16, 31)
#define WTBL_RTS_FAIL_CNT_OFFSET        16

/* DW 19 */
#define WTBL_DATA_RETRY_CNT_MASK        BITS(0, 15)
#define WTBL_DATA_RETRY_CNT_OFFSET      0
#define WTBL_MGNT_RETRY_CNT_MASK        BITS(16, 31)
#define WTBL_MGNT_RETRY_CNT_OFFSET      16


/* WTBL Group - Rx Statistics Counter */
/* DW 28 */
#define WTBL_OM_INFO_MASK               BITS(0, 11)
#define WTBL_OM_INFO_OFFSET             0
#define WTBL_OM_RXD_DUP_MODE            BIT(12)

/* DW 29 */
#define WTBL_USER_RSSI_MASK             BITS(0, 8)
#define WTBL_USER_RSSI_OFFSET           0
#define WTBL_USER_SNR_MASK              BITS(9, 14)
#define WTBL_USER_SNR_OFFSET            9
#define WTBL_RAPID_REACTION_RATE_MASK   BITS(16, 26)
#define WTBL_RAPID_REACTION_RATE_OFFSET 16
#define WTBL_HT_AMSDU                   BIT(30)
#define WTBL_AMSDU_CROSS_LG             BIT(31)

/* DW 30 */
#define WTBL_RESP_RCPI0_MASK            BITS(0, 7)
#define WTBL_RESP_RCPI0_OFFSET          0
#define WTBL_RESP_RCPI1_MASK            BITS(8, 15)
#define WTBL_RESP_RCPI1_OFFSET          8
#define WTBL_RESP_RCPI2_MASK            BITS(16, 23)
#define WTBL_RESP_RCPI2_OFFSET          16
#define WTBL_RESP_RCPI3_MASK            BITS(24, 31)
#define WTBL_RESP_RCPI3_OFFSET          24

/* DW 31 */
#define WTBL_RESP_RCPI4_MASK            BITS(0, 7)
#define WTBL_RESP_RCPI4_OFFSET          0
#define WTBL_RESP_RCPI5_MASK            BITS(8, 15)
#define WTBL_RESP_RCPI5_OFFSET          8
#define WTBL_RESP_RCPI6_MASK            BITS(16, 23)
#define WTBL_RESP_RCPI6_OFFSET          16
#define WTBL_RESP_RCPI7_MASK            BITS(24, 31)
#define WTBL_RESP_RCPI7_OFFSET          24

/* DW 32 */
#define WTBL_SNR_RX0_MASK               BITS(0, 5)
#define WTBL_SNR_RX0_OFFSET             0
#define WTBL_SNR_RX1_MASK               BITS(6, 11)
#define WTBL_SNR_RX1_OFFSET             6
#define WTBL_SNR_RX2_MASK               BITS(12, 17)
#define WTBL_SNR_RX2_OFFSET             12
#define WTBL_SNR_RX3_MASK               BITS(18, 23)
#define WTBL_SNR_RX3_OFFSET             18

/* DW 33 */
#define WTBL_SNR_RX4_MASK               BITS(0, 5)
#define WTBL_SNR_RX4_OFFSET             0
#define WTBL_SNR_RX5_MASK               BITS(6, 11)
#define WTBL_SNR_RX5_OFFSET             6
#define WTBL_SNR_RX6_MASK               BITS(12, 17)
#define WTBL_SNR_RX6_OFFSET             12
#define WTBL_SNR_RX7_MASK               BITS(18, 23)
#define WTBL_SNR_RX7_OFFSET             18


/***** WTBL(UMAC) *****/
/* WTBL Group - Packet Number */
/* DW 0 */
#define WTBL_PN0_MASK                   BITS(0, 7)
#define WTBL_PN0_OFFSET                 0
#define WTBL_PN1_MASK                   BITS(8, 15)
#define WTBL_PN1_OFFSET                 8
#define WTBL_PN2_MASK                   BITS(16, 23)
#define WTBL_PN2_OFFSET                 16
#define WTBL_PN3_MASK                   BITS(24, 31)
#define WTBL_PN3_OFFSET                 24

/* DW 1 */
#define WTBL_PN4_MASK                   BITS(0, 7)
#define WTBL_PN4_OFFSET                 0
#define WTBL_PN5_MASK                   BITS(8, 15)
#define WTBL_PN5_OFFSET                 8

/* WTBL Group - Serial Number */
/* DW 1 */
#define WTBL_COM_SN_MASK                BITS(16, 27)
#define WTBL_COM_SN_OFFSET              16

/* DW 2 */
#define WTBL_TID0_AC0_SN_MASK           BITS(0, 11)
#define WTBL_TID0_AC0_SN_OFFSET         0
#define WTBL_TID1_AC1_SN_MASK           BITS(12, 23)
#define WTBL_TID1_AC1_SN_OFFSET         12
#define WTBL_TID2_AC2_SN_0_7_MASK       BITS(24, 31)
#define WTBL_TID2_AC2_SN_0_7_OFFSET     24

/* DW 3 */
#define WTBL_TID2_AC2_SN_8_11_MASK      BITS(0, 3)
#define WTBL_TID2_AC2_SN_8_11_OFFSET    0
#define WTBL_TID3_AC3_SN_MASK           BITS(4, 15)
#define WTBL_TID3_AC3_SN_OFFSET         4
#define WTBL_TID4_SN_MASK               BITS(16, 27)
#define WTBL_TID4_SN_OFFSET             16
#define WTBL_TID5_SN_0_3_MASK           BITS(28, 31)
#define WTBL_TID5_SN_0_3_OFFSET         28

/* DW 4 */
#define WTBL_TID5_SN_4_11_MASK          BITS(0, 7)
#define WTBL_TID5_SN_4_11_OFFSET        0
#define WTBL_TID6_SN_MASK               BITS(8, 19)
#define WTBL_TID6_SN_OFFSET             8
#define WTBL_TID7_SN_MASK               BITS(20, 31)
#define WTBL_TID7_SN_OFFSET             20

/* UWTBL DW 5 */
#define WTBL_KEY_LINK_DW_KEY_LOC0_MASK        BITS(0, 10)
#define WTBL_KEY_LINK_DW_KEY_LOC0_OFFSET      0
#define WTBL_KEY_LINK_DW_KEY_LOC1_MASK        BITS(16, 26)
#define WTBL_KEY_LINK_DW_KEY_LOC1_OFFSET      16
#define WTBL_QOS_MASK                         BIT(27)
#define WTBL_QOS_OFFSET                       27
#define WTBL_HT_VHT_HE_MASK                   BIT(28)
#define WTBL_HT_VHT_HE_OFFSET                 28

/* UWTBL DW 6 */
#define WTBL_AMSDU_LEN_MASK                    BITS(0, 5)
#define WTBL_AMSDU_LEN_OFFSET                  0
#define WTBL_AMSDU_NUM_MASK                    BITS(6, 8)
#define WTBL_AMSDU_NUM_OFFSET                  6
#define WTBL_AMSDU_EN_MASK                     BIT(9)
#define WTBL_AMSDU_EN_OFFSET                   9


/***** WTBL(LMAC) DW Offset *****/
/* LMAC WTBL Group - Peer Unique Information */
#define WTBL_GROUP_PEER_INFO_DW_0               0
#define WTBL_GROUP_PEER_INFO_DW_1               1

/* WTBL Group - TxRx Capability/Information */
#define WTBL_GROUP_TRX_CAP_DW_2                 2
#define WTBL_GROUP_TRX_CAP_DW_3                 3
#define WTBL_GROUP_TRX_CAP_DW_4                 4
#define WTBL_GROUP_TRX_CAP_DW_5                 5
#define WTBL_GROUP_TRX_CAP_DW_6                 6
#define WTBL_GROUP_TRX_CAP_DW_7                 7
#define WTBL_GROUP_TRX_CAP_DW_8                 8
#define WTBL_GROUP_TRX_CAP_DW_9                 9

/* WTBL Group - Auto Rate Table*/
#define WTBL_GROUP_AUTO_RATE_1_2                10
#define WTBL_GROUP_AUTO_RATE_3_4                11
#define WTBL_GROUP_AUTO_RATE_5_6                12
#define WTBL_GROUP_AUTO_RATE_7_8                13

/* WTBL Group - Tx Counter */
#define WTBL_GROUP_TX_CNT_LINE_1                14
#define WTBL_GROUP_TX_CNT_LINE_2                15
#define WTBL_GROUP_TX_CNT_LINE_3                16
#define WTBL_GROUP_TX_CNT_LINE_4                17
#define WTBL_GROUP_TX_CNT_LINE_5                18
#define WTBL_GROUP_TX_CNT_LINE_6                19

/* WTBL Group - Admission Control Counter */
#define WTBL_GROUP_ADM_CNT_LINE_1               20
#define WTBL_GROUP_ADM_CNT_LINE_2               21
#define WTBL_GROUP_ADM_CNT_LINE_3               22
#define WTBL_GROUP_ADM_CNT_LINE_4               23
#define WTBL_GROUP_ADM_CNT_LINE_5               24
#define WTBL_GROUP_ADM_CNT_LINE_6               25
#define WTBL_GROUP_ADM_CNT_LINE_7               26
#define WTBL_GROUP_ADM_CNT_LINE_8               27

/* WTBL Group - Rx Statistics Counter */
#define WTBL_GROUP_RX_STAT_CNT_LINE_1           28
#define WTBL_GROUP_RX_STAT_CNT_LINE_2           29
#define WTBL_GROUP_RX_STAT_CNT_LINE_3           30
#define WTBL_GROUP_RX_STAT_CNT_LINE_4           31
#define WTBL_GROUP_RX_STAT_CNT_LINE_5           32
#define WTBL_GROUP_RX_STAT_CNT_LINE_6           33


/***** WTBL(UMAC) DW Offset *****/
/* UWTBL Group - Serial Number */
#define UWTBL_PN_DW_0                           0
#define UWTBL_PN_SN_DW_1                        1
#define UWTBL_SN_DW_2                           2
#define UWTBL_SN_DW_3                           3
#define UWTBL_SN_DW_4                           4

/* UWTBL Group - Key Link */
#define UWTBL_KEY_LINK_DW                       5

/* UWTBL Group - HW AMSDU */
#define UWTBL_HW_AMSDU_DW                       6


#define INVALID_KEY_ENTRY                      WTBL_KEY_LINK_DW_KEY_LOC0_MASK
#define ONE_KEY_ENTRY_LEN_IN_DW                8

/***** Key Table operation command value(WF_UWTBL_TOP_KTCR_OPERATION_ADDR) *****/
#define UWTBL_TOP_KTCR_OPERATION_DELETE     0
#define UWTBL_TOP_KTCR_OPERATION_ALLOCATE   1
#define UWTBL_TOP_KTCR_OPERATION_SEARCH     2

/***** AMSDU HW Setting *****/
#define MAX_AMSDU_LEN       32
#define MAX_AMSDU_STA_NUM   8

enum {
	FMAC_RXV_GROUP1 = 0,
	FMAC_RXV_GROUP2,
	FMAC_RXV_GROUP3,
	FMAC_RXV_GROUP5,
	FMAC_RXV_GROUP_MAX
};

#define MURU_MAX_PFID_NUM 8
#define MURU_MAX_GROUP_CN 3
#define EXE_IN_INET 0
/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/
#ifdef TXBF_SUPPORT
#ifdef RT_BIG_ENDIAN

union txbf_pfmu_tag1 {
	struct {
		UINT32 rmsd               : 3; /* [31:29]   : RMSD value from CE */
		UINT32 invalid_prof       : 1; /* [28]      : 0:default, 1: This profile number is invalid by SW */
		UINT32 reserved           : 2; /* [27:26]   : Reserved */
		UINT32 ngroup             : 2; /* [25:24]   : Ngroup */
		UINT32 codebook           : 2; /* [23:22]   : Code book */
		UINT32 ncol               : 3; /* [21:19]   : Ncol 3bits for 8x8 */
		UINT32 nrow               : 3; /* [18:16]   : Nrow 3bits for 8x8 */
		UINT32 su_mu              : 1; /* [15]      : 0:SU, 1: MU */
		UINT32 lm                 : 2; /* [14:13]   : 0/1/2/3: Legacy/HT/VHT/HE */
		UINT32 dbw                : 2; /* [12:11]   : 0/1/2/3: DW20/40/80/160NC */
		UINT32 txbf               : 1; /* [10]      : 0: iBF, 1: eBF */
		UINT32 profile_id         : 10;/* [9:0]     : 0 ~ 1023 */

		UINT32 mem_addr2_row_id   : 10;/* [63:54]   : row index : 0 ~ 63 */
		UINT32 mem_addr2_col_id   : 6; /* [53:48]   : column index : 0 ~ 5 */
		UINT32 mem_addr1_row_id   : 10;/* [47:38]   : row index : 0 ~ 63 */
		UINT32 mem_addr1_col_id   : 6; /* [37:32]   : column index : 0 ~ 5 */

		UINT32 mem_addr4_row_id   : 10;/* [95:86]   : row index : 0 ~ 63 */
		UINT32 mem_addr4_col_id   : 6; /* [85:80]   : column index : 0 ~ 5 */
		UINT32 mem_addr3_row_id   : 10;/* [79:70]   : row index : 0 ~ 63 */
		UINT32 mem_addr3_col_id   : 6; /* [69:64]   : column index : 0 ~ 5 */

		UINT32 reserved3          : 15;/* [127:113] : Reserved */
		UINT32 mob_cal_en         : 1; /* [112]     : Mobility detection calculation enable */
		UINT32 reserved2          : 1; /* [111]     : Reserved */
		UINT32 ru_end_id          : 7; /* [110:104] : 0~73, only for HE profile (V matrix RU index) */
		UINT32 reserved1          : 1; /* [103]     : Reserved */
		UINT32 ru_start_id        : 7; /* [102:96]  : 0~73, only for HE profile (V matrix RU index) */

		UINT32 snr_sts3           : 8; /* [159:152] : SNR_STS3 */
		UINT32 snr_sts2           : 8; /* [151:144] : SNR_STS2 */
		UINT32 snr_sts1           : 8; /* [143:136] : SNR_STS1 */
		UINT32 snr_sts0           : 8; /* [135:128] : SNR_STS0 */

		UINT32 snr_sts7           : 8; /* [191:184] : SNR_STS7 */
		UINT32 snr_sts6           : 8; /* [183:176] : SNR_STS6 */
		UINT32 snr_sts5           : 8; /* [175:168] : SNR_STS5 */
		UINT32 snr_sts4           : 8; /* [167:160] : SNR_STS4 */
	} field;
	UINT32 raw_data[7];
};

union txbf_pfmu_tag2 {
	struct {
		UINT32 reserved       : 3; /* [31:29]  : Reserved */
		UINT32 se_idx         : 5; /* [28:24]  : SE index */
		UINT32 smart_ant      : 24;/* [23:0]   : Smart Ant config */

		UINT32 reserved3      : 8; /* [63:56]  : Reserved */
		UINT32 ibf_timeout    : 8; /* [55:48]  : iBF timeout limit */
		UINT32 reserved2      : 5; /* [47:43]  : Reserved */
		UINT32 rmsd_thd       : 3; /* [42:40]  : RMSD Threshold */
		UINT32 reserved1      : 8; /* [39:32]  : Reserved */

		UINT32 ibf_ru         : 8; /* [95:88]  : Desired RX packet RU index, only for HE profile (OFDMA data RU index, not V matrix RU index) */
		UINT32 ibf_nrow       : 3; /* [87:85]  : iBF desired Nrow = 1 ~ 8 */
		UINT32 ibf_ncol       : 3; /* [84:82]  : iBF desired Ncol = 1 ~ 8 */
		UINT32 ibf_dbw        : 2; /* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
		UINT32 reserved4      : 16;/* [79:64]  : Reserved */

		UINT32 reserved6      : 16;/* [127:112] : Reserved */
		UINT32 reserved5      : 1; /* [111]     : Reserved */
		UINT32 mob_lq_result  : 7; /* [110:104] : Mobility detection calculation result. U1.6 */
		UINT32 mob_delta_t    : 8; /* [103:96]  : Mobility detection delta T value. Resolution: 1ms. Max = 255ms */
	} field;
	UINT32 raw_data[7];
};

union txbf_low_seg_angle {
	struct {
		/* DATA 0 */
		UINT32 psi31 : 7;
		UINT32 phi21 : 9;
		UINT32 psi21 : 7;
		UINT32 phi11 : 9;

		/* DATA 1 */
		UINT32 psi51 : 7;
		UINT32 phi41 : 9;
		UINT32 psi41 : 7;
		UINT32 phi31 : 9;

		/* DATA 2 */
		UINT32 psi71 : 7;
		UINT32 phi61 : 9;
		UINT32 psi61 : 7;
		UINT32 phi51 : 9;

		/* DATA 3 */
		UINT32 psi32 : 7;
		UINT32 phi22 : 9;
		UINT32 psi81 : 7;
		UINT32 phi71 : 9;

		/* DATA 4 */
		UINT32 psi52 : 7;
		UINT32 phi42 : 9;
		UINT32 psi42 : 7;
		UINT32 phi32 : 9;

		/* DATA 5 */
		UINT32 psi72 : 7;
		UINT32 phi62 : 9;
		UINT32 psi62 : 7;
		UINT32 phi52 : 9;

		/* DATA 6 */
		UINT32 psi43 : 7;
		UINT32 phi33 : 9;
		UINT32 psi82 : 7;
		UINT32 phi72 : 9;

		/* DATA 7 */
		UINT32 psi63 : 7;
		UINT32 phi53 : 9;
		UINT32 psi53 : 7;
		UINT32 phi43 : 9;

		/* DATA 8 */
		UINT32 psi83 : 7;
		UINT32 phi73 : 9;
		UINT32 psi73 : 7;
		UINT32 phi63 : 9;

		/* DATA 9 */
		UINT32 psi64 : 7;
		UINT32 phi54 : 9;
		UINT32 psi54 : 7;
		UINT32 phi44 : 9;

		/* DATA 10 */
		UINT32 psi84 : 7;
		UINT32 phi74 : 9;
		UINT32 psi74 : 7;
		UINT32 phi64 : 9;

		/* DATA 11 */
		UINT32 psi75 : 7;
		UINT32 phi65 : 9;
		UINT32 psi65 : 7;
		UINT32 phi55 : 9;

		/* DATA 12 */
		UINT32 psi76 : 7;
		UINT32 phi66 : 9;
		UINT32 psi85 : 7;
		UINT32 phi75 : 9;

		/* DATA 13 */
		UINT32 psi87 : 7;
		UINT32 phi77 : 9;
		UINT32 psi86 : 7;
		UINT32 phi76 : 9;
	} field;
	UINT32 raw_data[14];
};

union txbf_high_seg_angle {
	struct {
		/* DATA 14 */
		UINT32 psi31 : 7;
		UINT32 phi21 : 9;
		UINT32 psi21 : 7;
		UINT32 phi11 : 9;

		/* DATA 15 */
		UINT32 psi51 : 7;
		UINT32 phi41 : 9;
		UINT32 psi41 : 7;
		UINT32 phi31 : 9;

		/* DATA 16 */
		UINT32 psi71 : 7;
		UINT32 phi61 : 9;
		UINT32 psi61 : 7;
		UINT32 phi51 : 9;

		/* DATA 17 */
		UINT32 psi32 : 7;
		UINT32 phi22 : 9;
		UINT32 psi81 : 7;
		UINT32 phi71 : 9;

		/* DATA 18 */
		UINT32 psi52 : 7;
		UINT32 phi42 : 9;
		UINT32 psi42 : 7;
		UINT32 phi32 : 9;

		/* DATA 19 */
		UINT32 psi72 : 7;
		UINT32 phi62 : 9;
		UINT32 psi62 : 7;
		UINT32 phi52 : 9;

		/* DATA 20 */
		UINT32 psi43 : 7;
		UINT32 phi33 : 9;
		UINT32 psi82 : 7;
		UINT32 phi72 : 9;

		/* DATA 21 */
		UINT32 psi63 : 7;
		UINT32 phi53 : 9;
		UINT32 psi53 : 7;
		UINT32 phi43 : 9;

		/* DATA 22 */
		UINT32 psi83 : 7;
		UINT32 phi73 : 9;
		UINT32 psi73 : 7;
		UINT32 phi63 : 9;

		/* DATA 23 */
		UINT32 psi64 : 7;
		UINT32 phi54 : 9;
		UINT32 psi54 : 7;
		UINT32 phi44 : 9;

		/* DATA 24 */
		UINT32 psi84 : 7;
		UINT32 phi74 : 9;
		UINT32 psi74 : 7;
		UINT32 phi64 : 9;

		/* DATA 25 */
		UINT32 psi75 : 7;
		UINT32 phi65 : 9;
		UINT32 psi65 : 7;
		UINT32 phi55 : 9;

		/* DATA 26 */
		UINT32 psi76 : 7;
		UINT32 phi66 : 9;
		UINT32 psi85 : 7;
		UINT32 phi75 : 9;

		/* DATA 27 */
		UINT32 psi87 : 7;
		UINT32 phi77 : 9;
		UINT32 psi86 : 7;
		UINT32 phi76 : 9;
	} field;
	UINT32 raw_data[14];
};

union txbf_bfer_low_seg_dsnr {
	struct {
		/* DATA 28*/
		UINT32 dsnr07 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr00 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_high_seg_dsnr {
	struct {
		/* DATA 29*/
		UINT32 dsnr07 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr00 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfee_low_seg_dsnr {
	struct {
		/* DATA 28*/
		UINT32 dsnr03     : 2;
		UINT32 dsnr02     : 10;
		UINT32 dsnr01     : 10;
		UINT32 dsnr00     : 10;

		/* DATA 29 */
		UINT32 dsnr06     : 4;
		UINT32 dsnr05     : 10;
		UINT32 dsnr04     : 10;
		UINT32 dsnr03_msb : 8

		/* DATA 30 */
		UINT32 reserved   : 16
		UINT32 dsnr07     : 10;
		UINT32 dsnr06_msb : 6;
	} field;
	UINT32 raw_data[3];
};

union txbf_bfee_high_seg_dsnr {
	struct {
		/* DATA 30*/
		UINT32 dsnr01     : 6;
		UINT32 dsnr00     : 10;
		UINT32 reserved   : 16;

		/* DATA 31 */
		UINT32 dsnr04     : 8;
		UINT32 dsnr03     : 10;
		UINT32 dsnr02     : 10;
		UINT32 dsnr01_msb : 4;


		/* DATA 32 */
		UINT32 dsnr07     : 10;
		UINT32 dsnr06     : 10;
		UINT32 dsnr05     : 10;
		UINT32 dsnr04_msb : 2;
	} field;
	UINT32 raw_data[3];
};

#else
union txbf_pfmu_tag1 {
	struct {
		UINT32 profile_id         : 10;/* [9:0]     : 0 ~ 1023 */
		UINT32 txbf               : 1; /* [10]      : 0: iBF, 1: eBF */
		UINT32 dbw                : 2; /* [12:11]   : 0/1/2/3: DW20/40/80/160NC */
		UINT32 lm                 : 2; /* [14:13]   : 0/1/2/3: Legacy/HT/VHT/HE */
		UINT32 su_mu              : 1; /* [15]      : 0:SU, 1: MU */
		UINT32 nrow               : 3; /* [18:16]   : Nrow 3bits for 8x8 */
		UINT32 ncol               : 3; /* [21:19]   : Ncol 3bits for 8x8 */
		UINT32 codebook           : 2; /* [23:22]   : Code book */
		UINT32 ngroup             : 2; /* [25:24]   : Ngroup */
		UINT32 reserved           : 2; /* [27:26]   : Reserved */
		UINT32 invalid_prof       : 1; /* [28]      : 0:default, 1: This profile number is invalid by SW */
		UINT32 rmsd               : 3; /* [31:29]   : RMSD value from CE */

		UINT32 mem_addr1_col_id   : 6; /* [37:32]   : column index : 0 ~ 5 */
		UINT32 mem_addr1_row_id   : 10;/* [47:38]   : row index : 0 ~ 63 */
		UINT32 mem_addr2_col_id   : 6; /* [53:48]   : column index : 0 ~ 5 */
		UINT32 mem_addr2_row_id   : 10;/* [63:54]   : row index : 0 ~ 63 */

		UINT32 mem_addr3_col_id   : 6; /* [69:64]   : column index : 0 ~ 5 */
		UINT32 mem_addr3_row_id   : 10;/* [79:70]   : row index : 0 ~ 63 */
		UINT32 mem_addr4_col_id   : 6; /* [85:80]   : column index : 0 ~ 5 */
		UINT32 mem_addr4_row_id   : 10;/* [95:86]   : row index : 0 ~ 63 */

		UINT32 ru_start_id        : 7; /* [102:96]  : 0~73, only for HE profile (V matrix RU index) */
		UINT32 reserved1          : 1; /* [103]     : Reserved */
		UINT32 ru_end_id          : 7; /* [110:104] : 0~73, only for HE profile (V matrix RU index) */
		UINT32 reserved2          : 1; /* [111]     : Reserved */
		UINT32 mob_cal_en         : 1; /* [112]     : Mobility detection calculation enable */
		UINT32 reserved3          : 15;/* [127:111] : Reserved */

		UINT32 snr_sts0           : 8; /* [135:128] : SNR_STS0 */
		UINT32 snr_sts1           : 8; /* [143:136] : SNR_STS1 */
		UINT32 snr_sts2           : 8; /* [151:144] : SNR_STS2 */
		UINT32 snr_sts3           : 8; /* [159:152] : SNR_STS3 */

		UINT32 snr_sts4           : 8; /* [167:160] : SNR_STS4 */
		UINT32 snr_sts5           : 8; /* [175:168] : SNR_STS5 */
		UINT32 snr_sts6           : 8; /* [183:176] : SNR_STS6 */
		UINT32 snr_sts7           : 8; /* [191:184] : SNR_STS7 */
	} field;
	UINT32 raw_data[7];
};

union txbf_pfmu_tag2 {
	struct {
		UINT32 smart_ant      : 24;/* [23:0]   : Smart Ant config */
		UINT32 se_idx         : 5; /* [28:24]  : SE index */
		UINT32 reserved       : 3; /* [31:29]  : Reserved */

		UINT32 reserved1      : 8; /* [39:32]  : Reserved */
		UINT32 rmsd_thd       : 3; /* [42:40]  : RMSD Threshold */
		UINT32 reserved2      : 5; /* [47:43]  : Reserved */
		UINT32 ibf_timeout    : 8; /* [55:48]  : iBF timeout limit */
		UINT32 reserved3      : 8; /* [63:56]  : Reserved */

		UINT32 reserved4      : 16;/* [79:64]  : Reserved */
		UINT32 ibf_dbw        : 2; /* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
		UINT32 ibf_ncol       : 3; /* [84:82]  : iBF desired Ncol = 1 ~ 8 */
		UINT32 ibf_nrow       : 3; /* [87:85]  : iBF desired Nrow = 1 ~ 8 */
		UINT32 ibf_ru         : 8; /* [95:88]  : Desired RX packet RU index, only for HE profile (OFDMA data RU index, not V matrix RU index) */

		UINT32 mob_delta_t    : 8; /* [103:96]	: Mobility detection delta T value. Resolution: 1ms. Max = 255ms */
		UINT32 mob_lq_result  : 7; /* [110:104] : Mobility detection calculation result. U1.6 */
		UINT32 reserved5      : 1; /* [111] 	: Reserved */
		UINT32 reserved6      : 16;/* [127:112] : Reserved */
	} field;
	UINT32 raw_data[7];
};

union txbf_low_seg_angle {
	struct {
		/* DATA 0 */
		UINT32 phi11 : 9;
		UINT32 psi21 : 7;
		UINT32 phi21 : 9;
		UINT32 psi31 : 7;

		/* DATA 1 */
		UINT32 phi31 : 9;
		UINT32 psi41 : 7;
		UINT32 phi41 : 9;
		UINT32 psi51 : 7;

		/* DATA 2 */
		UINT32 phi51 : 9;
		UINT32 psi61 : 7;
		UINT32 phi61 : 9;
		UINT32 psi71 : 7;

		/* DATA 3 */
		UINT32 phi71 : 9;
		UINT32 psi81 : 7;
		UINT32 phi22 : 9;
		UINT32 psi32 : 7;

		/* DATA 4 */
		UINT32 phi32 : 9;
		UINT32 psi42 : 7;
		UINT32 phi42 : 9;
		UINT32 psi52 : 7;

		/* DATA 5 */
		UINT32 phi52 : 9;
		UINT32 psi62 : 7;
		UINT32 phi62 : 9;
		UINT32 psi72 : 7;

		/* DATA 6 */
		UINT32 phi72 : 9;
		UINT32 psi82 : 7;
		UINT32 phi33 : 9;
		UINT32 psi43 : 7;

		/* DATA 7 */
		UINT32 phi43 : 9;
		UINT32 psi53 : 7;
		UINT32 phi53 : 9;
		UINT32 psi63 : 7;

		/* DATA 8 */
		UINT32 phi63 : 9;
		UINT32 psi73 : 7;
		UINT32 phi73 : 9;
		UINT32 psi83 : 7;

		/* DATA 9 */
		UINT32 phi44 : 9;
		UINT32 psi54 : 7;
		UINT32 phi54 : 9;
		UINT32 psi64 : 7;

		/* DATA 10 */
		UINT32 phi64 : 9;
		UINT32 psi74 : 7;
		UINT32 phi74 : 9;
		UINT32 psi84 : 7;

		/* DATA 11 */
		UINT32 phi55 : 9;
		UINT32 psi65 : 7;
		UINT32 phi65 : 9;
		UINT32 psi75 : 7;

		/* DATA 12 */
		UINT32 phi75 : 9;
		UINT32 psi85 : 7;
		UINT32 phi66 : 9;
		UINT32 psi76 : 7;

		/* DATA 13 */
		UINT32 phi76 : 9;
		UINT32 psi86 : 7;
		UINT32 phi77 : 9;
		UINT32 psi87 : 7;
	} field;
	UINT32 raw_data[14];
};

union txbf_high_seg_angle {
	struct {
		/* DATA 14 */
		UINT32 phi11 : 9;
		UINT32 psi21 : 7;
		UINT32 phi21 : 9;
		UINT32 psi31 : 7;

		/* DATA 15 */
		UINT32 phi31 : 9;
		UINT32 psi41 : 7;
		UINT32 phi41 : 9;
		UINT32 psi51 : 7;

		/* DATA 16 */
		UINT32 phi51 : 9;
		UINT32 psi61 : 7;
		UINT32 phi61 : 9;
		UINT32 psi71 : 7;

		/* DATA 17 */
		UINT32 phi71 : 9;
		UINT32 psi81 : 7;
		UINT32 phi22 : 9;
		UINT32 psi32 : 7;

		/* DATA 18 */
		UINT32 phi32 : 9;
		UINT32 psi42 : 7;
		UINT32 phi42 : 9;
		UINT32 psi52 : 7;

		/* DATA 19 */
		UINT32 phi52 : 9;
		UINT32 psi62 : 7;
		UINT32 phi62 : 9;
		UINT32 psi72 : 7;

		/* DATA 20 */
		UINT32 phi72 : 9;
		UINT32 psi82 : 7;
		UINT32 phi33 : 9;
		UINT32 psi43 : 7;

		/* DATA 21 */
		UINT32 phi43 : 9;
		UINT32 psi53 : 7;
		UINT32 phi53 : 9;
		UINT32 psi63 : 7;

		/* DATA 22 */
		UINT32 phi63 : 9;
		UINT32 psi73 : 7;
		UINT32 phi73 : 9;
		UINT32 psi83 : 7;

		/* DATA 23 */
		UINT32 phi44 : 9;
		UINT32 psi54 : 7;
		UINT32 phi54 : 9;
		UINT32 psi64 : 7;

		/* DATA 24 */
		UINT32 phi64 : 9;
		UINT32 psi74 : 7;
		UINT32 phi74 : 9;
		UINT32 psi84 : 7;

		/* DATA 25 */
		UINT32 phi55 : 9;
		UINT32 psi65 : 7;
		UINT32 phi65 : 9;
		UINT32 psi75 : 7;

		/* DATA 26 */
		UINT32 phi75 : 9;
		UINT32 psi85 : 7;
		UINT32 phi66 : 9;
		UINT32 psi76 : 7;

		/* DATA 27 */
		UINT32 phi76 : 9;
		UINT32 psi86 : 7;
		UINT32 phi77 : 9;
		UINT32 psi87 : 7;
	} field;
	UINT32 raw_data[14];
};

union txbf_bfer_low_seg_dsnr {
	struct {
		/* DATA 28*/
		UINT32 dsnr00 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr07 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfer_high_seg_dsnr {
	struct {
		/* DATA 29*/
		UINT32 dsnr00 : 4;
		UINT32 dsnr01 : 4;
		UINT32 dsnr02 : 4;
		UINT32 dsnr03 : 4;
		UINT32 dsnr04 : 4;
		UINT32 dsnr05 : 4;
		UINT32 dsnr06 : 4;
		UINT32 dsnr07 : 4;
	} field;
	UINT32 raw_data[1];
};

union txbf_bfee_low_seg_dsnr {
	struct {
		/* DATA 28*/
		UINT32 dsnr00     : 10;
		UINT32 dsnr01     : 10;
		UINT32 dsnr02     : 10;
		UINT32 dsnr03     : 2;

		/* DATA 29 */
		UINT32 dsnr03_msb : 8;
		UINT32 dsnr04     : 10;
		UINT32 dsnr05     : 10;
		UINT32 dsnr06     : 4;

		/* DATA 30 */
		UINT32 dsnr06_msb : 6;
		UINT32 dsnr07     : 10;
		UINT32 reserved   : 16;
	} field;
	UINT32 raw_data[3];
};

union txbf_bfee_high_seg_dsnr {
	struct {
		/* DATA 30*/
		UINT32 reserved   : 16;
		UINT32 dsnr00     : 10;
		UINT32 dsnr01     : 6;

		/* DATA 31 */
		UINT32 dsnr01_msb : 4;
		UINT32 dsnr02     : 10;
		UINT32 dsnr03     : 10;
		UINT32 dsnr04     : 8;

		/* DATA 32 */
		UINT32 dsnr04_msb : 2;
		UINT32 dsnr05     : 10;
		UINT32 dsnr06     : 10;
		UINT32 dsnr07     : 10;
	} field;
	UINT32 raw_data[3];
};

#endif

union txbf_bfer_pfmu_data {
	struct {
		union txbf_low_seg_angle rLowSegAng;
		union txbf_high_seg_angle rHighSegAng;
		union txbf_bfer_low_seg_dsnr rLowSegSnr;
		union txbf_bfer_high_seg_dsnr rHighSegSnr;
	} field;
	UINT32 raw_data[33];
};

union txbf_bfee_pfmu_data {
	struct {
		union txbf_low_seg_angle rLogSegAng;
		union txbf_high_seg_angle rHighSegAng;
		union txbf_bfee_low_seg_dsnr rLowSegSnr;
		union txbf_bfee_high_seg_dsnr rHighSegSnr;
	} field;
	UINT32 raw_data[33];
};
#endif /* TXBF_SUPPORT */

union hetb_rx_cmm {
	struct {
		ULONGLONG tigger_type:4;
		ULONGLONG ul_length:12;
		ULONGLONG cascade_ind:1;
		ULONGLONG cs_required:1;
		ULONGLONG ul_bw:2;
		ULONGLONG gi_ltf:2;
		ULONGLONG mimo_ltf:1;
		ULONGLONG ltf_sym_midiam:3;
		ULONGLONG stbc:1;
		ULONGLONG ldpc_extra_sym:1;
		ULONGLONG ap_tx_pwr:6;
		ULONGLONG t_pe:3;
		ULONGLONG spt_reuse:16;
		ULONGLONG doppler:1;
		ULONGLONG sig_a_reserved:9;
		ULONGLONG reserved:1;
	} field;
	ULONGLONG cmm_info;
};

union hetb_rx_usr {
	struct {
		UINT32 uid:8;
		UINT32 non_sf_enb:1;
		UINT32 nss:3;
		UINT32 allocation:8;
		UINT32 coding:1;
		UINT32 mcs:4;
		UINT32 dcm:1;
		UINT32 ss_allocation:6;
	} field;
	UINT32 usr_info;
};

union hetb_tx_usr {
	struct {
		UINT32 aid:12;
		UINT32 allocation:8;
		UINT32 coding:1;
		UINT32 mcs:4;
		UINT32 dcm:1;
		UINT32 ss_allocation:6;
	} field;
	UINT32 usr_info;
};

/* MURU local data - _rMuru_Local_Data */
#define OFFSET_OF(_type, _field)    ((size_t)(&((_type *)0)->_field))

#define CONFIG_WIFI_RAM_MURU_MAX_USER_PPDU	16
#define MAX_USER_IN_PPDU        (CONFIG_WIFI_RAM_MURU_MAX_USER_PPDU)

#define MAX_DATA_AC_NUM                 4 /*DL + UL Data*/

typedef enum _ENUM_MURU_AC_NUM_T {
	MURU_AC_NUM_0 = 0,
	MURU_AC_NUM_1 = 1,
	MURU_AC_NUM_2 = 2,
	MURU_AC_NUM_3 = 3,
	MURU_AC_NUM_MAX = 4
} ENUM_MURU_AC_NUM_T, *P_ENUM_MURU_AC_NUM_T;

typedef enum _MURU_DUSCH_WEIGHT {
	MURU_DUSCH_WEIGHT_0_1 = 0x0,
	MURU_DUSCH_WEIGHT_1_4 = 0x1,
	MURU_DUSCH_WEIGHT_2_1 = 0x2,
	MURU_DUSCH_WEIGHT_2_2 = 0x3,
	MURU_DUSCH_WEIGHT_1_2 = 0x4,
	MURU_DUSCH_WEIGHT_4_1 = 0x5,
	MURU_DUSCH_WEIGHT_1_0 = 0x6,
	MURU_DUSCH_WEIGHT_1_1 = 0x7,
	MURU_DUSCH_WEIGHT_16_1 = 0x8,
	MURU_DUSCH_WEIGHT_3_1,
	MURU_DUSCH_WEIGHT_5_1,
	MURU_DUSCH_WEIGHT_6_1,
	MURU_DUSCH_WEIGHT_7_1,
	MURU_DUSCH_WEIGHT_NULL
} MURU_DUSCH_WEIGHT_T;

typedef enum _ENUM_MUM_GRP_USR_CAP_T {
	MUM_GRP_USR_VHT_CAP = 0x0,
	MUM_GRP_USR_HE_DLFUMUM_CAP = 0x1,
	MUM_GRP_USR_HE_DLPBMUM_CAP = 0x2,
	MUM_GRP_USR_HE_ULFBMUM_CAP = 0x3,
	MUM_GRP_USR_HE_ULPBMUM_CAP = 0x4,
	MUM_MAX_GRP_USR_CAP        = 0x5
} ENUM_MUM_GRP_USR_CAP_T, *P_ENUM_MUM_GRP_USR_CAP_T;

typedef struct _LINK_T {
	UINT_32 prNext;			/* Set Host 8B pointer to 4B */
	UINT_32 prPrev;			/* Set Host 8B pointer to 4B */
	UINT_32 u4NumElem;
} LINK_T, *P_LINK_T;

typedef struct _MURU_TPC_MAN_PARA_T {
	INT_8 ai1ManTargetRssi[MAX_USER_IN_PPDU];
} MURU_TPC_MAN_PARA_T, P_MURU_TPC_MAN_PARA_T;

typedef struct _MURU_PARA_T {
	BOOLEAN fgPingPongAlgo;
	BOOLEAN fgSu;
	BOOLEAN fg256BitMap;
	BOOLEAN fgUlBsrp;
	BOOLEAN fgTxcmdsnd;
	BOOLEAN fgTpc;
	BOOLEAN fgTpcManualMode;
	UINT_16 u2fixedTPNum;
	UINT_8  u1UlMpduCntPolicy;
	UINT_8  u1DelayPolicy;
	MURU_TPC_MAN_PARA_T rTpcManPara;
	BOOLEAN fgTpcOptMode;
	UINT_8  u1TxCmdQLen[MAX_DATA_AC_NUM];
	BOOLEAN fgTBSuAdaptiveLSIGLen;
	BOOLEAN fgSRState;
	UINT_8 u1TypeCDelayReq;
	UINT_32 u4BsrTruncateThr;
	UINT_16 u2MaxStaCntLimit;
	BOOLEAN fgPreGrp;
	BOOLEAN fgTxopBurst;
	INT_16 i2PsdDiffThr;
	UINT_8 u1SplPriority;
	UINT_8 u1DlSolictAckPolicy;
} MURU_PARA_T, *P_MURU_PARA_T;

typedef struct _MURU_QLEN_INFO_T {
	UINT_32 au4DLQlen[MURU_AC_NUM_MAX];
	UINT_32 au4ULQlen[MURU_AC_NUM_MAX];
	UINT_32 u4TotDLQlenAllAc;
	UINT_32 u4TotULQlenAllAc;
	UINT_32 u4BsrTruncateThr;
} MURU_QLEN_INFO_T, *P_MURU_QLEN_INFO_T;

typedef struct _MURU_TXCMD_CTRL_T {
	BOOLEAN fgGlobalPreLoad;
	INT_16  i2PuPreGrpMaxPsd_dBm;
} MURU_TXCMD_CTRL_T, *P_MURU_TXCMD_CTRL_T;

typedef struct _MURU_RU_TONE_PLAN_DBG_T {
	UINT_8 u1ToneStr[12];
	UINT_32 prToneGroup;	/* Set Host 8B pointer to 4B */
} MURU_RU_TONE_PLAN_DBG_T, *P_MURU_RU_TONE_PLAN_DBG_T;

typedef struct _MURU_TP_DBG_CTRL_T {
	LINK_T rMuruTpDbgFreeList;
	LINK_T rMuruTpDbgUsedList;
	MURU_RU_TONE_PLAN_DBG_T rTonePlanDbg[8];
	BOOLEAN fgTpDbgEn;
	UINT_16 u2TpDbgShowPeriod;
	UINT_8 u1TonePlanDbgIdx;
	UINT_8 u1Resv[1];
} MURU_TP_DBG_CTRL_T, *P_MURU_TP_DBG_CTRL_T;

typedef struct _MURU_LOCAL_DATA_T {
	/*MURU local Control Parameters*/
	MURU_PARA_T         rMuruPara;

	/*DL and UL Scheduler*/
	MURU_DUSCH_WEIGHT_T eDuSchWeight;

	/*Qlen Info maintained*/
	MURU_QLEN_INFO_T    rQlenInfo;
	UINT_16 u2MuruSplHeadWlanId;

	CMD_MURU_BSRP_CTRL  rExt_Cmd_Bsrp_Ctrl;
	CMD_MURU_HESND_CTRL rExt_Cmd_HeSnd_Ctrl;
	CMD_MURU_CERT_SEND_FRAME_CTRL rExt_Cmd_Cert_Send_frame_Ctrl;
	MURU_TXCMD_CTRL_T   rMuru_TxCmd_Ctrl;
	BOOLEAN fgMumUl;
	BOOLEAN fgTwtNonCriticalTxReq;

	MURU_TP_DBG_CTRL_T  rMuru_TpDbg_Ctrl;

	/*Retry STA List*/
	LINK_T  _rRetryForPktDropStaList[MURU_AC_NUM_MAX];
	LINK_T  _rRetryForPktFreeList;
} MURU_LOCAL_DATA_T, *P_MURU_LOCAL_DATA_T;

/*MURU part*/
typedef struct _MURU_GLOBAL_INFO_T {
	UINT_8  u1TxdNum;
	UINT_8  u1Qid;
	UINT_8  u1TxcmdType; /*not used*/
	BOOLEAN fgSpl;
	UINT_8  u1PresentSpTblIdx;
	BOOLEAN fgTv;
	BOOLEAN fgDbdcIdx;
	BOOLEAN fgPreload;
	BOOLEAN fgTxop;
	UINT_8  u1OwnMac;
	BOOLEAN fgIgnoreBw;
	BOOLEAN fgSmartAnt;
	UINT_8  u1AggPolicy;
	UINT_8  u1Bandwidth;
	UINT_32 u4AntId;
	UINT_8  u1SerialId;
	UINT_8  u1SpeIdx;
	BOOLEAN fgOptionalBackoff;
} MURU_GLOBAL_INFO_T, *P_MURU_GLOBAL_INFO_T;

typedef  struct _PROT_RU_INFO_T {
	UINT_16  u2Aid;
	UINT_8   u1RuAlloc;
} PROT_RU_INFO_T, *P_PROT_RU_INFO_T;

typedef struct _MURU_PROTECT_INFO_T {
	UINT_8   u1Protect;
	UINT_8   u1StaCnt;
	BOOLEAN  fgCascadeIdx;
	BOOLEAN  fgCsRequired;
	UINT_8   u1TfPad;
	UINT_8   u1Rate;
	UINT_8   u1TxMode;
	UINT_8   u1Nsts;
	BOOLEAN  fgCoding;
	BOOLEAN  fgDoppler;
	PROT_RU_INFO_T rProtRuInfo[CONFIG_WIFI_RAM_RU_NUM_PARA];
} MURU_PROTECT_INFO_T, *P_MURU_PROTECT_INFO_T;

typedef struct _MURU_USER_INFO_T {
	UINT_16  u2TxPowerAlpha;
	BOOLEAN  fgCoding;
	UINT_16  u2WlanId;
	UINT_8   u1MuMimoGroup;
	UINT_8   u1MuMimoSpatial;
	UINT_8   u1StartStream;
	BOOLEAN  fgMultiTid;
	BOOLEAN  fgRuAllocBn;
	UINT_8   u1RuAlloc;
	UINT_8   u1AckGroup;
	BOOLEAN  fgSuBar;
	BOOLEAN  fgMuBar;
	BOOLEAN  fgCbSta;
	BOOLEAN  fgAggOld;
	BOOLEAN  fgPreload;
	UINT_8   u1Rate;
	UINT_8   u1Nsts;
	UINT_8   u1LpCtrl;
	BOOLEAN  fgContentCh;
	UINT_8   u1AckPol;
	UINT_16  u2SrRate;
	UINT_16  u2RuRatio;
	BOOLEAN  fgSplPrimaryUser;
	UINT_8   u1AcSeq;
	UINT_8   u1AcNum;
	UINT_16  u2BarRuRatio;
	UINT_16  u2LSigLen;
	UINT_8   u1Bw;
	UINT_8   u1Ac0Ratio;
	UINT_8   u1Ac1Ratio;
	UINT_8   u1Ac2Ratio;
	UINT_8   u1Ac3Ratio;
	UINT_8   u1BarRate;
	UINT_8   u1BarMode;
	UINT_8   u1BarNsts;
	UINT_8   u1BaType;
	BOOLEAN  fgCsRequired;
	UINT_8   u1LtfType;
	UINT_8   u1LtfSym;
	BOOLEAN  fgStbc;
	BOOLEAN  fgLdpcExtraSym;
	UINT_8   u1PktExt;
	BOOLEAN  fgCoding2;
	BOOLEAN  fgDcm;
	BOOLEAN  fgBarAckPol;
	BOOLEAN  fgAckRuAllocBn;
	UINT_8   u1AckRuAlloc;
	UINT_8   u1AckMcs;
	UINT_8   u1SsAlloc;
	UINT_8   u1TargetRssi;
	BOOLEAN  fgDoppler;
	BOOLEAN  fgBf;
	UINT_8   u1TidInfo;
	UINT_16  u2SpatialReuse;
} MURU_USER_INFO_T, *P_MURU_USER_INFO_T;

typedef  struct _MURU_TX_DATA_T {
	UINT_8  u1Rxv;
	BOOLEAN fgRsp;
	BOOLEAN fgPsIgnore;
	UINT_8  u1SigBCh1StaCnt;
	UINT_8  u1SigBCh2StaCnt;
	UINT_8  u1StaCnt;
	UINT_8  u1SigBSym;
	UINT_8  u1SigBMcs;
	BOOLEAN fgRa;
	BOOLEAN fgSigBDcm;
	BOOLEAN fgSigBCompress;
	UINT_8  u1LtfSym;
	UINT_8  u1Gi;
	BOOLEAN fgStbc;
	BOOLEAN fgCmdPower;
	UINT_16 u2MuPpduDur;
	UINT_8  u1TxPower;
	UINT_8  aucRuAlloc[8];
	BOOLEAN fgDoppler;
	UINT_8  u1PrimaryUserIdx;
	UINT_8  u1Ltf;
	UINT_8  u1TfPad;
	UINT_8  u1Mu0UserPosition;
	UINT_8  u1Mu1UserPosition;
	UINT_8  u1Mu2UserPosition;
	UINT_8  u1Mu3UserPosition;
	UINT_8  u1MuGroupId;
	BOOLEAN fgRu26dSigBCh1;
	BOOLEAN fgRu26uSigBCh2;
	UINT_8  u1TxMode;
	BOOLEAN fgDynamicBw;
	UINT_8  u1PreamblePuncture;
	UINT_8  u1MuUser;
	UINT_16 u2ProtectionDuration;
	UINT_16 u2ResponseDuration;
	MURU_USER_INFO_T arTxcmdUser[CONFIG_WIFI_RAM_RU_NUM_PARA];
} MURU_TX_DATA_T, *P_MURU_TX_DATA_T;

typedef struct _MURU_USER_ACK_INFO_T {
	UINT_16 u2StaId;
	UINT_16 u2AckTxPowerAlpha;
	BOOLEAN fgCoding;
	BOOLEAN fgContentCh;
	UINT_16 u2WlanId;
	BOOLEAN fgRuAllocBn;
	UINT_8  u1RuAlloc;
	UINT_8  u1Rate;
	UINT_8  u1Nsts;
	UINT_8  u1RuAllNss;
	UINT_16 u2RuRatio;
	BOOLEAN fgSfEnable;
	UINT_8  u1Ac;
	BOOLEAN fgSplPrimaryUser;
} MURU_USER_ACK_INFO_T, *P_MURU_USER_ACK_INFO_T;

typedef enum _ENUM_MEM_TYPE_T {
	MEM_TYPE_NONE = 0,
	MEM_TYPE_PLE,
	MEM_TYPE_PSE,
#if (CONFIG_WIFI_RAM_HW_WFDMA == 1)
	MEM_TYPE_PKT_DLM,
#endif
} ENUM_MEM_TYPE_T;

typedef struct _FRAME_BUF_INFO_T {
	ENUM_MEM_TYPE_T eMemType;
	UINT_32         pucBuffer;
	union PKT_ADDR	{
#if (CONFIG_WIFI_RAM_HW_WFDMA == 1)
		UINT_32 u4WfPktAddr;
#endif
		UINT_16 u2Fid;
	} addr;
} FRAME_BUF_INFO_T, *P_FRAME_BUF_INFO_T;

typedef struct _MURU_TX_TRIG_DATA_T {
	UINT_8  u1Rxv;
	UINT_8  u1StaCnt;
	UINT_8  u1BaPol;
	BOOLEAN fgPriOrder;
	UINT_8  u1SplAc;
	UINT_8  u1PreambPunc;
	UINT_8  u1AckTxMode;
	UINT_8  u1TrigType;
	UINT_32 u4RxHetbCfg1;
	UINT_32 u4RxHetbCfg2;
	UINT_8  u1TfPad;
	UINT_16 u2LSigLen;
	UINT_8  u1SigBCh1StaCnt;
	UINT_8  u1SigBSym;
	UINT_8  u1SigBMcs;
	BOOLEAN fgSigBDcm;
	BOOLEAN fgSigBCompress;
	UINT_8  u1LtfSym;
	UINT_8  u1Gi;
	BOOLEAN fgStbc;
	BOOLEAN fgDoppler;
	BOOLEAN fgCmdPower;
	UINT_8  u1SigBCh2StaCnt;
	UINT_16 u2MuPpduDur;
	UINT_8  u1Ltf;
	BOOLEAN fgRu26dSigBCh1;
	BOOLEAN fgRu26uSigBCh2;
	UINT_8  au1RuAlloc[8];
	UINT_8  u1AckTxPower;
	UINT_8  u1SsnUser;
	UINT_8  u1MuUser;

	FRAME_BUF_INFO_T    rTxDBufInfo;    /* including the Pointer to the associated buffer */
	FRAME_BUF_INFO_T    rFixFidBufInfo; /* including the Pointer to the associated buffer */
	UINT_16 u2MsduId;
	MURU_USER_ACK_INFO_T rTxcmdUserAck[CONFIG_WIFI_RAM_RU_NUM_PARA];
} MURU_TX_TRIG_DATA_T, *P_MURU_TX_TRIG_DATA_T;

/* A collection of fields related to SR in RXRPT */
typedef struct _SR_RXRPT_T {
	BOOLEAN fgIsFromCmdrptTx;
	UINT_8 u1SrBand;
	UINT_8 u1SrEntry;
	UINT_32 u4User0Addr2;
	UINT_32 u4TimeStamp;
	INT_32 i4SrPeriodRemain;
} SR_RXRPT_T, *P_SR_RXRPT_T;

typedef struct _TXCMD_DEPOT_TXUSER_T {
	UINT_8 u1Ac;
	UINT_8 u1Rate;
	UINT_8 u1Nsts;
	UINT_8 u1Stbc;
	UINT_8 u1Gi;
	UINT_8 u1RuAlloc;
	BOOLEAN fgRuAllocBn;
	UINT_8 u1MuMimoGrp;
	UINT_16 u1RuTreeMapArrayIdx;
	UINT_8  u1RuMapArrayIdx;
	UINT_16 u2WlanId;
	UINT_16 u2MumGrpIdx;
	UINT_8 u1TargetRssi;
	BOOLEAN  fgCoding;
} TXCMD_DEPOT_TXUSER_T, *P_TXCMD_DEPOT_TXUSER_T;

typedef struct _TXCMD_DEPOT_T {
	BOOLEAN fgIsOccupied;
	UINT_8 u1SerialId;
	UINT_8 u1TxMode;
	UINT_8 u1StaCnt;
	UINT_8 u1TxcmdType;
	UINT_8 u1Ac;
	UINT_8 u1Qid;
	UINT_8 u1SchType;
	UINT_8 u1Bandwidth;
	BOOLEAN fgRa;
	BOOLEAN fgIsTwt;
	UINT_8 u1AckPol;
	UINT_32 u4MuPpduDuration;
	ENUM_DBDC_BN_T eBandIdx;
	BOOLEAN fgIsMuRts;
	SR_RXRPT_T rSrRxrpt;
	TXCMD_DEPOT_TXUSER_T arTxUser[MAX_NUM_TXCMD_TX_USER];
} TXCMD_DEPOT_T, *P_TXCMD_DEPOT_T;

typedef struct _MURU_TX_INFO_T {
	/* MURU Global Info */
	MURU_GLOBAL_INFO_T rGlobalData;

	/* MURU Protect Info */
	MURU_PROTECT_INFO_T rProtectData;

	/* MURU TX data Info */
	MURU_TX_DATA_T	rSxnTxData;

	/* MURU TRIG data Info */
	MURU_TX_TRIG_DATA_T rSxnTrigData;

	/* TXCMD Depot */
	/*P_TXCMD_DEPOT_T prTxcmdDepot;*/
	UINT32             prTxcmdDepot;
} MURU_TX_INFO_T, *P_MURU_TX_INFO_T;

typedef struct _PER_USER_DATA_INFO {
	UINT_16 u2WlanId;
	BOOLEAN fgUserPreLoad;
	UINT_8  u1MuMimoGrp;
	UINT_8  u1RuAlloc;
	UINT_8 u1RuTreeMapArrayIdx;
	UINT_8 u1RuMapArrayIdx;
	BOOLEAN fgRuAllocBn;
	UINT_8  u1MuMimoSpatial;
	UINT_8  u1StartStream;
	UINT_8  u1RateMode; /* CCk, HT, VHT */
	UINT_8  u1Nss;
	UINT_8 u1StartSpatialStream;
	UINT_8  u1Mcs;
	UINT_8  u1Gi;
	BOOLEAN fgLdpc;
	UINT_16 u2WeightFactor;
	UINT_8  u1SrMcs;
	UINT_8  u1UpperMCS;
	BOOLEAN fgDcm;
	UINT_16 u2RuRatio;
	UINT_8 u1RuAllNss;
	BOOLEAN fgAggOld;
	BOOLEAN fgCB;
	UINT_8  u1AckBw;
	UINT_8  u1AcSeq;
	UINT_8  u1AcNum;
	UINT_16 u2BarRuRatio;
	UINT_8  u1AcRatio[4];
	UINT_16 u2MumGrpIdx;
	UINT_8  u2MumGrpStaCnt;
	UINT_8  u1LtfType;
	BOOLEAN fgSplPrimaryUser;
	UINT_8  u1BfType;

	/* BA use */
	UINT_8  u1AckPol;
	UINT_8  u1AckGrp;
	BOOLEAN fgSuBar;
	BOOLEAN fgMuBar;
	UINT_8  u1BarRate;
	UINT_8  u1BarMode;
	UINT_8  u1BarNsts;
	UINT_8  u1BaType;
	UINT_32 u4BaMuPpduDur;
	UINT_32 u4BaLSigDur;
	BOOLEAN fgBaDcm;
	BOOLEAN fgBaStbc;
	UINT_8  u1AckRuAlloc;
	BOOLEAN fgAckRuAllocBn;
	UINT_8  u1AckMcs;
	UINT_8  u1AckNss;
	BOOLEAN fgAckLdpc;
	UINT_8  u1BarAckPol;
	UINT_8  u1SsAaloc;
	/*TPC info*/
	UINT_8  u1TargetRssi;
	UINT_8  u1TidInfo;
	UINT_16 u2EffSnr;

	/*TPA info*/
	UINT_16 u2TxPwrAlpha_dB;

	/* algo. use */
	UINT_32 u4RuScore;
	UINT_32 u4StaMuPpduDur;
	BOOLEAN fgLargeRu;
} PER_USER_INFO, *P_PER_USER_INFO;

typedef enum _ENUM_TXCMD_TYPE_T {
	TXCMD_HE_TRIG_DATA        = 0x0,
	TXCMD_HE_TX_DATA          = 0x1,
	TXCMD_HE_PROT_TRIG_DATA   = 0x2,
	TXCMD_HE_PROT_TX_DATA     = 0x3,
	TXCMD_HE_SOUNDING         = 0x4,
	TXCMD_SW_PACKET           = 0x5,
	TXCMD_NON_HE_TX_DATA      = 0x6,
	TXCMD_NON_HE_PROT_TX_DATA = 0x7,
	TXCMD_HE_PROT_SOUNDING    = 0x8
} ENUM_TXCMD_TYPE_T, *P_ENUM_TXCMD_TYPE_T;

typedef enum _MURU_TF_PAD_T {
	MURU_TF_PAD_0_US = 0,
	MURU_TF_PAD_8_US = 1,
	MURU_TF_PAD_16_US = 2,
} MURU_TF_PAD_T, *P_MURU_TF_PAD_T;

typedef struct _PROT_RU_INFO {
	UINT_16 aid             : 12,
			ruAlloc         : 4;
} PROT_RU_INFO, *P_PROT_RU_INFO;

typedef struct _MURU_ALGO_PROTSEC_INFO_T {
	UINT_8 u1ProtType;
	UINT_8 u1ProtStaCnt;
	UINT_8 u1ProtBw;
	UINT_8 u1ProtTxMode;
	UINT_8 u1ProtRate;
	UINT_8 u1ProtNsts;
	UINT_8 u1ProtCoding;
	PROT_RU_INFO arProtRuInfo[MAX_USER_IN_PPDU];

	BOOLEAN fgOptionalBackoff;
	MURU_TF_PAD_T eProtTfPadType;
	BOOLEAN fgDoppler; /* share with all Sections */
} MURU_ALGO_PROTSEC_INFO_T, *P_MURU_ALGO_PROTSEC_INFO_T;

typedef enum _ENUM_TX_MODE_T {
	TXCMD_TX_MODE_LEGACY_CCK = 0x0,
	TXCMD_TX_MODE_LEGACY_OFDMA = 0x1,
	TXCMD_TX_MODE_HT_MIXED = 0x2,
	TXCMD_TX_MODE_HT_GREEN_FIELD = 0x3,
	TXCMD_TX_MODE_VHT = 0x4,
	TXCMD_TX_MODE_HE_SU = 0x8,
	TXCMD_TX_MODE_HE_EXT_SU = 0x9,
	TXCMD_TX_MODE_HE_TRIG = 0xA,
	TXCMD_TX_MODE_HE_MU = 0xB
} ENUM_TX_MODE_T;

typedef enum _ENUM_RU_SCH_T {
	SCH_T_NONE = 0x0,
	DL_DATA = 0x1,
	UL_DATA = 0x2,
	UL_BSRP = 0x3,
	UL_HE_SND_TF = 0x4,
	DL_SW_PKT = 0x5,
} ENUM_RU_SCH_T, *P_ENUM_RU_SCH_T;

typedef struct _MURU_RU_TP_DBG_T {
	LINK_T  rMuruTpDbgEntry;
	UINT_32 u4SelCnt;
	UINT_32 u4Score;
	UINT_32 u4TotBitsOfThisTP;
	UINT_32 u4PpduTxDur;
	UINT_32 u4StaCnt;
	UINT_16 u2TonePlanIdx;
	UINT_8  u1Resv[2];
	struct MURU_RU_TONE_PLAN *prRuTonePlan;
} MURU_RU_TP_DBG_T, *P_MURU_RU_TP_DBG_T;

struct MURU_RU_TONE_PLAN {
	UINT_16 ruCnt;          /* Total RU cnt */
	UINT_16 largeRuSize;        /* The RU cnt which is equal or more than 106 tones */
	UINT_16 smallRuSize;        /* The RU cnt which is smaller than 106 tones*/
	UINT_8  ruAlloc[8];     /* The tone plan transformation of per 20Mhz RU allocation signaling common part */
	UINT_8 *tpMap;          /* The array which contains the RU set of this tone plan */
	MURU_RU_TP_DBG_T *prTpDbg;
};

typedef struct _SPL_USER_INFO {
	UINT_32 headPktLen  : 10,   /* DW0 */
			rsv1        : 3,
			frag        : 1,
			hdPktNotAgg : 1,
			hdPktRetry  : 1,
			priority    : 4,
			powerSave   : 1,
			updated     : 1,
			wlanId      : 10;
	UINT_32 hdPktDelay  : 10,   /* DW1 */
			rsv2        : 2,
			ppduLen     : 20;
	UINT_32 rxTotQLen   : 16,   /* DW2 */
			txTotQLen   : 16;
	UINT_32 totPktCnt   : 16,   /* DW3 */
			ac4Twt      : 4,
			quota       : 8,
			rsv3        : 1,
			rxEarlyEnd    : 1,
			txEarlyEnd    : 1,
			txed        : 1;
} SPL_USER_INFO, *P_SPL_USER_INFO;

typedef struct _SPL_T {
	UINT_32 rxByteCnt   : 16,   /* DW0 */
			staCount    : 8,
			rsv1        : 3,
			pktType     : 5;
	UINT_32 ac          : 4,    /* DW1 */
			SpTblIdx    : 4,
			tv          : 1,
			roundEnd    : 1,
			SubRoundEnd : 1,
			SplGenMode  : 4,
			notEmpty    : 1,
			rsv2        : 16;
	UINT_32 rsv3;               /* DW2 */
	UINT_32 timeStamp   : 16,    /* DW3 */
			rsv4        : 16;
	SPL_USER_INFO splUser[20]; /* DW4~ */
} SPL_T, *P_SPL_T;

typedef enum _ENUM_BAND_T {
	BAND_NULL = 0000000,
	BAND_2G4 = 2407000,
	BAND_5G0 = 5000000,
	BAND_4G9375 = 4937500,
	BAND_4G89 = 4890000,
	BAND_4G85 = 4850000,
	BAND_4G = 4000000,
	BAND_5G0025 = 5002500,
	BAND_4G0025 = 4002500
} ENUM_BAND_T, *P_ENUM_BAND_T;

typedef struct _MURU_ALLOC_DATA_INFO_T {
	UINT_32 fgBcRu: 1;
	UINT_32 fgSpl: 1;
	UINT_32 fgExp: 1;
	UINT_32 fgTxopBurst: 1;
	UINT_32 fgTxopFailRu: 1;
	UINT_32 fgGlobalPreLoad: 1;
	UINT_32 fgIB: 1;
	UINT_32 fgBs: 1;
	UINT_32 fgSigbDcm: 1; /*share with Section TRIG-Data (uplink MU-OFDMA-BA)*/
	UINT_32 fgRa: 1;
	UINT_32 fgAckLdpcExtra: 1;
	UINT_32 fg26D: 1;
	UINT_32 fg26U: 1;
	UINT_32 fgTrigPO: 1;
	UINT_32 fgMaxScore: 1;
	UINT_32 fgSplPrimaryUser: 1;

	ENUM_BAND_T eBand;
	/* global section */
	UINT_8  u1AggPol;
	UINT_8  u1Ac;
	ENUM_TXCMD_TYPE_T txCmdType;
	UINT_8  u1SerialId;
	UINT_8  u1SpeIdx;
	/* protect section */
	MURU_ALGO_PROTSEC_INFO_T rMuruAlgoProtSec;

	/* Section TX-Data */
	UINT_8  u1SigbSym;
	UINT_8  u1LtfSym;
	UINT_8  u1SigbMcs;  /* share with Section TRIG-Data (uplink MU-OFDMA-BA)  */
	UINT_8  u1GiType;
	UINT_8  u1LtfType;
	UINT_8  u1StaCnt;
	ENUM_TX_MODE_T eTxMode;
	UINT_8 u1AckGiType;     /* share with Section TRIG-Data (uplink MU-OFDMA-BA)  */
	UINT_8 u1AckLtfType;    /* share with Section TRIG-Data (uplink MU-OFDMA-BA)  */
	UINT_8 u1AckMaxNss;
	MURU_TF_PAD_T eTxSecTfPadType;
	/*DL TX Power Allocatioin Info*/
	UINT_8  u1TxPwr_dBm;
	UINT_8  u1Bw;
	UINT_8  u1PrimaryUserIdx;
	UINT_16 u2LongPpduWlanId;
	UINT_8  u1LongPpduUsrIdx;/*userinfo idx in prRuRsp, not wlanid*/
	UINT_32 u4MuPpduDuration;/*for TRIG Section, it would be L-SIG Len*/
	UINT_32 u4MaxBaMuPpduDur;
	UINT_32 u4MaxBaDurForLSig;
	UINT_8  au1MuUp[4];
	UINT_8  u1GrpId;

	/* Section TX-Data Per User Info */
	PER_USER_INFO userInfo[MAX_USER_IN_PPDU];

	/* Section TRIG-Data */
	UINT_8  u1TrigBaPL;
	UINT_8  u1TfType;
	UINT_8  u1TrigSplAc;
	UINT_8  u1TrigAckBw;
	UINT_8  u1TrigAckTxPwr;
	UINT_8  u1TrigAckTxMode;
	UINT_32 u4LSigLength;
	MURU_TF_PAD_T eTrigSecTfPadType;
	UINT_8  ucTfPe;
	/*Per User Ack Info uses PER_USER_INFO in Tx-Data Sec*/

	/*RU Algo Use Begin*/
	UINT_8  u1TotMumGrpCnt;
	UINT_8  au1RuMumGrpStaNum[7]; /* Only RU61~67 support MUMIMO */
	ENUM_RU_SCH_T eSchType;
	UINT_8  u1OperateBw;
	UINT_8  u1HavmDLULIdx;  /* HAVM DL/UL scheduling */
	UINT_8  u1SplStaCnt;
	UINT_16 u2TonePlanIdx;
	UINT_16 u2TypeAStaCnt;
	UINT_16 u2TypeBStaCnt;
	UINT_32 u4MaxHeadTime;
	UINT_32 u4MaxScore;
	UINT_32 u4SuScore;
	UINT_32 u4MuScore;
	UINT_32 u4TotBitsOfThisTP;
	UINT_32 u4PpduTxDur;
	UINT_32 u4MuPpduUtilization;
	/*struct MURU_RU_TONE_PLAN *prRuTonePlan;*/
	UINT_32 prRuTonePlan;
	/*P_SPL_T prSplPkt;*/
	UINT_32 prSplPkt;
	UINT_8  u1AckPol;
	/*RU Algo Use End*/

	/*SR Algo Use Begin*/
	BOOLEAN fgSr;
	/*P_SR_RXRPT_T prSrRxrpt;*/
	UINT_32 prSrRxrpt;
	/*SR Algo Use End*/

} MURU_ALLOC_DATA_INFO_T, *P_MURU_ALLOC_DATA_INFO_T;

typedef enum _ENUM_MURU_HE_STA_STATE {
	MURU_HE_STA_STATE_CONTENTION     = 0x0,
	MURU_HE_STA_STATE_TRIG           = 0x1,
	MURU_HE_STA_STATE_MAX            = 0x2,
} ENUM_MURU_HE_STA_STATE, *P_ENUM_MURU_HE_STA_STATE;

/*speed up guscore calculation*/
typedef struct _MURU_RUSCORE_RECORD_T {
	UINT_32 u4StaMuPpduDur;
	UINT_32 u4RuScore;
	UINT_8  u1Mcs;
	UINT_8  u1Nss;
	UINT_32 u4BitRate;
} MURU_RUSCORE_RECORD_T, *P_MURU_RUSCORE_RECORD_T;

typedef enum _ENUM_MURU_MAX_AGG_SIZE_T {
	MURU_MAX_AGG_SIZE_64 = 0,
	MURU_MAX_AGG_SIZE_256,
} ENUM_MURU_MAX_AGG_SIZE_T, *P_ENUM_MURU_MAX_AGG_SIZE_T;

typedef enum _MURU_BUF_TYPE {
	MURU_BUF_TYPE_C = 0,
	MURU_BUF_TYPE_B = 1,
	MURU_BUF_TYPE_A = 2,
	MURU_BUF_TYPE_NUM = 3
} MURU_BUF_TYPE, *P_MURU_BUF_TYPE;

typedef struct _LINK_ENTRY_S {
	/* struct _LINK_ENTRY_S *prNext, *prPrev; */
	UINT32 prNext, prPrev;
} LINK_ENTRY_S, *P_LINK_ENTRY_S;

typedef struct _MURU_RETRY_FOR_PKT_T {
	LINK_ENTRY_S rRetryForPktDropEntry;
	UINT_32 prRuSta;
	BOOL fgDropPkt[MURU_AC_NUM_MAX];
	UINT_32 u4AggStatus;
	UINT_8  u1RefCnt;
	UINT_8  u1Resv[3];
} MURU_RETRY_FOR_PKT_T, *P_MURU_RETRY_FOR_PKT_T;

typedef struct _STA_MURU_RECORD_T {
	/*------------------------------------------------------------------------------------------*/
	/* MU RU record*/
	/*------------------------------------------------------------------------------------------*/
	ENUM_TX_MODE_T eStaRecCapMode;

#if (CFG_SUPPORT_MURU == 1)
#if (CFG_RU_STA_SORTING == 1)
	INT_16 u2NextWlanId;
#endif

	UINT_8  u1Bw;
	UINT_8  u1TxBw;
	UINT_16 u2WlanId;
	UINT_16 u2StaIdx;
	UINT_16 u2NextStaRecIdxbySPL;
	UINT_16 u2NextStaRecIdxQlen;
	UINT_16 u2NextStaRecIdxbyBSRP;

	/*STA Capability*/
	UINT_8 u1MaxNss;
	UINT_8 u1BandIdx;
	UINT_8 u1MumCapBitmap;

	/*************input to RU*********/
	UINT_8  u1BfType;
	UINT_8  u1Mcs;
	UINT_8  u1Nss;
	UINT_8  u1Gi;
	UINT_8  u1Ecc;
	UINT_8  u1HeLtf;
	UINT_8  u1Stbc;
	UINT_8  au1Priority[MURU_AC_NUM_MAX];
	UINT_8  afgNonEmptyState[MURU_AC_NUM_MAX];
	UINT_8  u1DlDepCmd[MURU_AC_NUM_MAX]; /*Has be scheduled in DL AC TxCmd Queue for independent txcmd*/
	UINT_8  u1UlDepCmd[MURU_AC_NUM_MAX]; /*Has be scheduled in UL AC TxCmd Queue for independent txcmd*/
	UINT_8  u1Ac;

	/*BSRP Begin*/
	UINT_8  u1BsrpPeriod;
	UINT_8  u1BsrpMaxPeriod;
	UINT_8  u1BsrpMissCnt;
	UINT_8  u1BsrpHitCnt;
	/*BSRP End*/

	/*DL UL WRR Scheduler Begin*/
	UINT_8  au1DlQuantum[MURU_AC_NUM_MAX];
	UINT_8  au1UlQuantum[MURU_AC_NUM_MAX];
	/*DL UL WRR Scheduler End*/
	UINT_8  u1DelayWeight;
	UINT_8  u1HeSndPeriod;
	UINT_8  u1HeSndMaxPeriod;
	UINT_8  u1SuccSoundingCounter;
	UINT_8  u1FailSoundingCounter;
	UINT_8  u1UlMuGrp;
	/* MUPPDU RTS retry limit */
	BOOL fgRtsForMuPpduRetry[MURU_AC_NUM_MAX];
	/*MU TPC Begin*/
	UINT_8  u1LastTxMcs;
	INT_8   i1UlPwrHeadroom_dB;
	UINT_8  u1MinTxPwrNonHitCnt;
	BOOLEAN afgCanNotAgg[MURU_AC_NUM_MAX];

	/*MU TPC Begin*/
	INT_16  i2MinRssi_dBm;
	UINT_8 u1LastRuBitmapIdx;
	INT_16  i2LastPerUserRssi_dBm;
	INT_16  i2PreGrpMaxPsd_dBm;
	INT_16  i2RssiOffset_dB;
	UINT_8  u1RxRptLastBw;
	UINT_8  u1RxRptLastTxMcs;
	INT_16  i2RxRptLastPerUserRssi_dBm;

	UINT_16 u2DelayReq;
	UINT_16 ul_agg_max_cnt;
	UINT_16 u2AvgRxOneMpduBytes; /* unit bit for UL Agg cnt estimation */
	UINT_16 u2AvgRxOneMpduBytes_diff; /* unit bit for UL Agg cnt estimation */

	UINT_16 au2MpduCntInPpdu[MURU_AC_NUM_MAX]; /*ppdu cnt*/
	UINT_16 u2SplId;
	UINT_16 au2RxAvgMpduSize[MURU_AC_NUM_MAX];
	UINT_16 au2CurrMsn[MURU_AC_NUM_MAX];
	UINT_16 au2NextSsn[MURU_AC_NUM_MAX];
	UINT_16 au2BaWin[MURU_AC_NUM_MAX];
	UINT_16 au2NewMpduCntInPpdu[MURU_AC_NUM_MAX];
	UINT_16 au2RxPer[MURU_AC_NUM_MAX];
	UINT_16 au2HeadPktLen[MURU_AC_NUM_MAX]; /* unit byte */
	UINT_16 au2DlHeadPktDelay[MURU_AC_NUM_MAX];
	UINT_16 au2UlHeadPktDelay[MURU_AC_NUM_MAX];
	UINT_16 au2RxAvgLongTermMpduSize[MURU_AC_NUM_MAX];
	UINT_16 u2WeightFactor;

	UINT_8 u1DisableBsrpByAc : 7;
	UINT_8 fgLastRxFrmTrig : 1;
	UINT_8 u1UlTypeACnt : 4;
	UINT_8 u1UlNonTypeACnt : 4;
	ENUM_TX_MODE_T eDataTxMode;
	MURU_BUF_TYPE ePpduType; /*type A, type B, or type C traffic*/
	MURU_BUF_TYPE eDL_LTPpduType[MURU_AC_NUM_MAX];
	MURU_BUF_TYPE eUL_LTPpduType[MURU_AC_NUM_MAX];
	/*STA State*/
	ENUM_MURU_HE_STA_STATE eHeStaStae;
	ENUM_BAND_T eBand;
	/* DropPkt by retry limit */
	/* P_MURU_RETRY_FOR_PKT_T prRetryForPktInfo; */
	UINT_32 prRetryForPktInfo;

	UINT_32 fgIsAddBaForAnyTIDsOfAC0 : 1;
	UINT_32 fgIsAddBaForAnyTIDsOfAC1 : 1;
	UINT_32 fgIsAddBaForAnyTIDsOfAC2 : 1;
	UINT_32 fgIsAddBaForAnyTIDsOfAC3 : 1;
	UINT_32 fgNonAggressiveRA : 1;
	UINT_32 fgBsrpCandidate : 1;
	UINT_32 fgBsrpTriggerCurPPDU : 1;
	UINT_32 fgBsrpHasSentInBasicTF : 1;
	UINT_32 fg20MOnlyCap : 1;
	UINT_32 fgHeSndCandidate : 1;
	UINT_32 fgHeSndTriggerCurPPDU : 1;
	UINT_32 fgPsMode : 1;
	UINT_32 fgUlSuSnd : 1;
	UINT_32 fgSrAbortBit : 1;
	UINT_32 fgDepCmd : 1; /*for use of algo. MIPs improvement*/
	UINT_32 fgIsTpcInfoValid : 1;
	UINT_32 fgRxRptIsTpcInfoValid: 1;
	UINT_32 fgIsTriggerred : 1;
	UINT_32 fgTcp : 1;
	UINT_32 fgMinTxPwrFlag : 1;
	UINT_32 fgHaveHitMinTxPwrFg : 1;
	UINT_32 fgAssocOk: 1;
	UINT_32 fgNeedBroadcastRU: 1; /*this STA has to TX by Broadcast RU*/
	UINT_32 fgHasRuAssign: 1; /*Has assigned RU to this STA*/
#if (EXE_IN_INET == 1)
	UINT_32 fgHeDlFBMUMCap: 1;
	UINT_32 fgHeDlPBMUMCap: 1;
	UINT_32 fgHeUlPBMUMCap: 1;
	UINT_32 fgHeUlFBMUMCap: 1;
	UINT_32 fgVhtDlMumCap: 1;
	UINT_32 fgUlMuCap: 1;
	UINT_32 fgReserved: 2;
#else
	UINT_32 fgReserved: 8;
#endif

    /* Last RX Rate for STA */
	UINT_8  u1RxRate;
	UINT_8  u1RxMode;
	UINT_8  u1RxNsts;
	UINT_8  u1RxGi;
	UINT_8  u1RxStbc;
	UINT_8  u1RxCoding;
	UINT_8  u1RxBW;
	/*SPL Qlen Info*/
	UINT_32 au4DlTotQlenBytes[MURU_AC_NUM_MAX];
	UINT_32 au4UlTotQlenBytes[MURU_AC_NUM_MAX];   /* unit bytes */
	UINT_32 u4UlTotAllQlenBytes;
	UINT_32 au4HeadPktTime[MURU_AC_NUM_MAX];        /* headPktTime unit is us */
	UINT_32 au4ByesInPpdu[MURU_AC_NUM_MAX]; /*ppdu length*/
	UINT_32 au4UlSchTimeStamp[MURU_AC_NUM_MAX];
	/************RU operation*********/
	UINT_32 u4RuScore;
	UINT_32 u4StaMuPpduDur;      /* staMuPpduTime unit is us */
	UINT_32 u4BsrpSetTimeStamp; /*ms*/

	/* MUMIMO */
	LINK_ENTRY_S rBsrpLinkEntry;

	UINT_32 u4TpcAgingTime;
	UINT_32  au4TidQueueSizeBytes[MAX_TID_NUM];

	/*Traffic prediction*/
	UINT_32 au4TotBytesTxInArrival[MURU_AC_NUM_MAX];
	UINT_32 au4TotBytesRxInArrival[MURU_AC_NUM_MAX];
	/*throughput monitor*/
	UINT_32 au4TotBytesTxInService[MURU_AC_NUM_MAX];
	UINT_32 au4ServiceBytesTxPerSecond[MURU_AC_NUM_MAX];
	UINT_32 au4TotBytesRxInService[MURU_AC_NUM_MAX];
	UINT_32 au4ServiceBytesRxPerSecond[MURU_AC_NUM_MAX];

	UINT_8 u1MuRtsFailStatusBuf;
	UINT_8 u1MuRtsFailScore;
	UINT_16 u2MuRtsInactiveTimer;

	MURU_RUSCORE_RECORD_T arRuScoreRrd[MAX_RUSCORE_RECORD_NUM];
#endif
} STA_MURU_RECORD_T, *P_STA_MURU_RECORD_T;

typedef struct _MURU_PURE_STACAP_INFO {
	MURU_STA_DL_OFDMA rDlOfdma;
	MURU_STA_UL_OFDMA rUlOfdma;
	MURU_STA_DL_MIMO rDlMimo;
	MURU_STA_UL_MIMO rUlMimo;
} MURU_PURE_STACAP_INFO, *P_MURU_PURE_STACAP_INFO;

typedef struct _MU_TX_STAT_INFO_T {
	UINT_32 u4SuccessCnt[MURU_MAX_GROUP_CN][MURU_MAX_PFID_NUM];
	UINT_32 u4TotalCnt[MURU_MAX_GROUP_CN][MURU_MAX_PFID_NUM];
} MU_TX_STAT_INFO_T, *P_MU_TX_STAT_INFO_T;

typedef struct _MU_TX_STAT_INFO_LINK_T {
	MU_TX_STAT_INFO_T DownLink;
	MU_TX_STAT_INFO_T UpLink;
} MU_TX_STAT_INFO_LINK_T, *P_MU_TX_STAT_INFO_LINK_T;

#define MAX_SPL_BACKUP_LEN   sizeof(SPL_T)
#define MAX_TXCMD_BACKUP_LEN 200

typedef enum _ENUM_WH_SPL_GEN_MODE_T {
	WH_SPL_TXCMD_DONE_TX_MODE = 0x0,
	WH_SPL_TXCMD_DONE_RX_MODE = 0x1,
	WH_SPL_TXCMD_DONE_TWT_DL_MODE = 0x2,
	WH_SPL_TXCMD_DONE_TWT_UL_MODE = 0x3,
	WH_SPL_IO_TX_MODE = 0x4,
	WH_SPL_IO_RX_MODE = 0x5,
	WH_SPL_TWT_DL_MODE = 0x6,
	WH_SPL_TWT_UL_MODE = 0x7,
	WH_SPL_CHNL_NONEMPTY_TX_MODE = 0x8,
	WH_SPL_CHNL_NONEMPTY_RX_MODE = 0x9,
	WH_SPL_BWREFILL_TX_MODE = 0xa,
	WH_SPL_BWREFILL_RX_MODE = 0xb,
	WH_SPL_ARB_B0_MODE = 0xc,
	WH_SPL_ARB_B1_MODE = 0xd,
	WH_SPL_PRELPAD_MODE = 0xe,
	WH_SPL_PM_CHG_DET_MODE = 0xf,
	WH_SPL_GEN_MODE_INVAILD,
	WH_SPL_GEN_MODE_NUM
} ENUM_WH_SPL_GEN_MODE_T;

typedef enum _ENUM_WH_TXCMD_QUE_T {
	WH_TXC_AC00 = 0,
	WH_TXC_AC01,
	WH_TXC_AC02,
	WH_TXC_AC03,
	WH_TXC_AC10,
	WH_TXC_AC11,
	WH_TXC_AC12,
	WH_TXC_AC13,
	WH_TXC_AC20,
	WH_TXC_AC21,
	WH_TXC_AC22,
	WH_TXC_AC23,
	WH_TXC_AC30,
	WH_TXC_AC31,
	WH_TXC_AC32,
	WH_TXC_AC33,
	WH_TXC_ALTX0,
	WH_TXC_TF0,
	WH_TXC_TWT_TSF_TF0,
	WH_TXC_TWT_DL0,
	WH_TXC_TWT_UL0,
	WH_TXC_QUE_NUM
} ENUM_WH_TXCMD_QUE_T;

typedef struct _MURU_SPL_BACKUP {
	UINT_8      u1SplBuf[MAX_SPL_BACKUP_LEN];
	UINT_8      u1TxCmdBuf[MAX_TXCMD_BACKUP_LEN];
} MURU_SPL_BACKUP, *P_MURU_SPL_BACKUP;

typedef enum _ENUM_MURU_TONE_PLAN_POLICY_T {
	MURU_NORMAL_TP_POLICY = 0,
	MURU_FIXED_2RU_TP_POLICY,
	MURU_FIXED_4RU_TP_POLICY,
	MURU_FIXED_8RU_TP_POLICY,
	MURU_FIXED_16RU_TP_POLICY,
} ENUM_MURU_TONE_PLAN_POLICY_T, *P_ENUM_MURU_TONE_PLAN_POLICY_T;

enum {
	MURU_CMDRPT_TX_SUCCESS = 0,
	MURU_CMDRPT_TX_RTS_FAIL,
	MURU_CMDRPT_TX_PIFS_TIMEOUT,
	MURU_CMDRPT_TX_RESV1,
	MURU_CMDRPT_TX_TWT_TIMEOUT,
	MURU_CMDRPT_TX_MDRDY_TIMEOUT,
	MURU_CMDRPT_TX_STATUS_CNT
};

typedef enum _ENUM_MURU_EST_T	{
	MURU_EST_SPL_TXC = 0,
	MURU_EST_TIMER_CNT,
	MURU_EST_TXC_EMPTY = MURU_EST_TIMER_CNT,
	MURU_EST_SPL,
	MURU_EST_CMDRPT_TX,
	MURU_EST_CNT
} ENUM_MURU_EST_T;

typedef struct _USERINFO_TRIGCMDRPT {
	UINT_16 u2WlanID;
	UINT_8 u1FailReson;
	UINT_16 u2RxAvgMpduSize;
	UINT_16 u2Padding; /*Bytes, u2NonTailDelimter + u2TailDelimter*/
	UINT_16 u2RxTotBytes;
	UINT_16 u2RxMaxMPDUBytes;
	UINT_16 u2RxAcSuccessByte[MURU_AC_NUM_MAX];
	UINT_16 u2RxTotMpduCnt;
	UINT_16 u2RxSuccMpduCnt;
	UINT_16 u2ACQlen[MURU_AC_NUM_MAX];
} USERINFO_TRIGCMDRPT, *P_USERINFO_TRIGCMDRPT;

typedef struct _USERINFO_TXGCMDRPT {
	UINT_16 u2WlanID;
	UINT_16 u2Airtime;
	UINT_8 u1FailReson;
	BOOL fgTCP;
	UINT_16 u2Padding; /*Bytes, u2NonTailDelimter + u2TailDelimter*/
	UINT_16 u2TxTotBytes;
	UINT_16 u2TxMaxMPDUBytes;
	UINT_16 u2TxTotMpduCnt;
	UINT_16 u2TxSuccMpduCnt;
} USERINFO_TXGCMDRPT, *P_USERINFO_TXGCMDRPT;

typedef struct _TX_CMDRPT {
	UINT_8 u1SeriID;
	UINT_8 u1StaCnt;
	UINT_8 u1TxBw;
	UINT_8 u1Status;
	UINT_32 u4TimeStamp;
	USERINFO_TXGCMDRPT userInfo[MAX_MU_NUM_PER_PPDU];
} TX_CMDRPT, *P_TX_CMDRPT;

typedef struct _TRIG_CMDRPT {
	UINT_8 u1SeriID;
	UINT_8 u1StaCnt;
	UINT_8 u1TxBw;
	UINT_8 u1Status;
	UINT_32 u4TimeStamp;
	USERINFO_TRIGCMDRPT userInfo[MAX_MU_NUM_PER_PPDU];
} TRIG_CMDRPT, *P_TRIG_CMDRPT;

typedef struct _MURU_STACNT_STAT_T {
	UINT_16 u2NonHtStaCnt;
	UINT_16 u2HtStaCnt;
	UINT_16 u2VhtStaCnt;
	UINT_16 u2VhtMumStaCnt; /*HE STA with MU-MIMO capability which repeat with HeStaCnt*/
	UINT_16 u2HeStaCnt;
	UINT_16 u2HeMumStaCnt; /*HE STA with MU-MIMO capability which repeat with HeStaCnt*/
	UINT_16 u2TotStaCnt;
} MURU_STACNT_STAT_T, *P_MURU_STACNT_STAT_T;

typedef struct _MURU_BAND_STATISTICS_T {
	UINT_16 au2DL_LT_TypeA_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2DL_LT_TypeB_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2DL_LT_TypeC_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2UL_LT_TypeA_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2UL_LT_TypeB_StaCnt[MURU_AC_NUM_MAX];
	UINT_16 au2UL_LT_TypeC_StaCnt[MURU_AC_NUM_MAX];
	BOOL    fgRxCntDoTrig;
	MURU_STACNT_STAT_T rStaCnt;
} MURU_BAND_STATISTICS_T, *P_MURU_BAND_STATISTICS_T;

typedef struct _MURU_MUGRP_ENTRY_T {
	UINT_8  u1UserCnt;
	UINT_16 au2WlanId[4];
	INT_32  i4InitOffset[4];
	UINT_8  au1Mcs[4];
	UINT_8  au1BbpMcs[4];
	BOOL    fgHasSounding;
	double  initDataRate[4];     /* dataRate by sounding result or initial setting */
	double  currDataRate[4];     /* Tx data rate between sounding periods */
	double score;
	UINT_32 au4MuQLen[4];
	UINT_8 au1Ac[4];
	UINT_16 u2next;
} MURU_MUGRP_ENTRY_T, *P_MURU_MUGRP_ENTRY_T;

typedef struct _MURU_TAILCMD_T {
	UINT_16 u2SerialId;
} MURU_TAILCMD_T, *P_MURU_TAILCMD_T;

typedef struct _MURU_EST_CMDRPT_TX_T {
	UINT_32 u4StationCnt[MAX_MU_NUM_PER_PPDU];
	UINT_32 u4Status[MURU_CMDRPT_TX_STATUS_CNT];
	UINT_32 u4TotPpduDur;
	UINT_32 u4CmdRptCnt;
} MURU_EST_CMDRPT_TX_T, *P_MURU_EST_CMDRPT_TX_T;

typedef enum _ENUM_DBDC_BN_S {
	S_ENUM_BAND_0,
	S_ENUM_BAND_1,
	S_ENUM_BAND_NUM,
	S_ENUM_BAND_ALL
} ENUM_DBDC_BN_S, *P_ENUM_DBDC_BN_S;

typedef struct _MURU_ALGO_EST_T {
	UINT_8  u1AlgoEstEn;
	UINT_8  u1Resv;
	UINT_16 u2AlgoEstPeriod;
	UINT_32 u4EstStartTime[MURU_EST_TIMER_CNT];
	UINT_32 u4EstEndTime[MURU_EST_TIMER_CNT];
	UINT_32 u4EstTotalTime[MURU_EST_TIMER_CNT];
	UINT_32 u4EstCnt[MURU_EST_TIMER_CNT];
	UINT_32 u4TxCmdEmptyEvntCnt;
	UINT_32 u4SplCnt[WH_SPL_GEN_MODE_NUM];
	ENUM_DBDC_BN_S eEstBand;
	/* MURU_EST_CMDRPT_TX_T *prEstCmdRptTx; */
	UINT32  prEstCmdRptTx;
} MURU_ALGO_EST_T, *P_MURU_ALGO_EST_T;

typedef struct _MURU_SHARE_DATA_T {
	/*for double pointer */
	MURU_ALLOC_DATA_INFO_T rRuAllocData;
	MURU_ALLOC_DATA_INFO_T rTempRuAllocData;
#if (CFG_MURU_MIPS_ESTIMATION == 1)
	MURU_ALLOC_DATA_INFO_T rTestData;
#endif
	/* P_MURU_ALLOC_DATA_INFO_T prRuallocRst; */
	UINT32      prRuallocRst;
	UINT_8      u1PrimaryAc;
	UINT_8      u1PrimaryStaIdx;	/* #256STA */
	UINT_8      u1Qid;
	TX_CMDRPT   rTxCmdRpt;
	TRIG_CMDRPT rTRIG_CMDRPT;
	UINT_8      u2MuRuMaxSplCnt;
	UINT_8      u1MaxStaCntInPpdu;
	UINT_16     u2TypeAStaCnt;
	UINT_16     u2TypeBStaCnt;
	UINT_16     u2TypeCStaCnt;

	BOOL fgPassDueTo20MhzOnlySta;
	/*BW capability*/
	BOOL fg20MhzOperatingDynamicRuAlgo;
	UINT_8 u1Head20MhzOnlySplUsrIdx;
	BOOL fg20MhzOnlyDoPrimaryUserAlways;
	UINT_16 u2PuOriWlanIdx;
	UINT_16 u2Pu20MHzWlanIdx;
	/* 2.4GHz */
	UINT_16 u2HE40Mhz2Dot4GHz_StaCnt;
	UINT_16 u2In40MMuPpduIn2Dot4GHz_HE20MOnlyStaCnt;
	/* 5GHz */
	UINT_16 u2HE4080M_5GHz_StaCnt; /*this would be independent with 160Mhz*/
	UINT_16 u2HE160M_5GHz_StaCnt;
	UINT_16 u2HE1608080M_5GHz_StaCnt;
	UINT_16 u2HE4080M_IN_1608080HEPPDU_5GHz_StaCnt;

	/*DBDC Support*/
	MURU_BAND_STATISTICS_T eBandStatistics[RAM_BAND_NUM];
	ENUM_DBDC_BN_S eBandIdx;
	UINT_8         u1GlobalBw;
	BOOLEAN        fgBsrpBandRequest[RAM_BAND_NUM];
	ENUM_DBDC_BN_S eLastBsrpBandTx;
	MURU_TAILCMD_T rTailCmdInfo;
	/*Currently, Assume WlanIdx = StaRuIdx*/
	STA_MURU_RECORD_T arStaRuRecord[CFG_STA_REC_NUM];
	UINT_8 u1BaGrp[MAX_BA_GRP];
	UINT_8 u1PuBw;
	ENUM_TXCMD_TYPE_T eTxCmdTye;
	MURU_BUF_TYPE ePuRuBuftype;
	BOOL fgUplink;
	BOOL fgUlSnd;
	ENUM_RU_SCH_T eSchtype;
	UINT_8 u1LastBSRPStaIdx;	/* #256STA */

	/* Profiling parameters */
	/* MURU_ALGO_EST_T *prAlgoEst; */
	UINT32   prAlgoEst;
	/*Default Value and Support Ext Cmd*/
	/*Algo Ctrl Section*/
	UINT_32 u4MaxRuAlgoTimeOut;
	UINT_8  u1PpduDurBias;
	UINT_8  u1PreGrp;
	UINT_8  u1MuRtsRule;
	UINT_8  u1MuRtsFailScoreThr;
	UINT_16 u2MuRtsInactiveTimerThr;
	BOOL    fgTxopBurst;
	BOOL    fgOptionalBackoff;
	INT_16  i2PsdDiffThr;
	/*Global and Protection Section*/
	BOOL    fgExp;
	UINT_8  u1Pdc;            /*PPDU Duration Control*/
	BOOL    fgProt;
	UINT_32 u4ProtFrameThr;   /*@us SU PPDU Duration > the thr => Enable RTS/CTS*/
	BOOL    fgForceMuRTS;
	UINT_8  u1ProtRuAlloc;    /*61~68*/
	BOOL    fgFixedRate;      /*0:use RA module rate, 1: use RU fixed rate*/
	/*Tx Data Section*/
	UINT_8  u1TxDataSec_Bw;   /*To Do, 0~7, 0: full 20Mhz, 1: full 40Mhz, 2: full 80Mhz, 3: 160Mhz*/
	UINT_32 u4TxDataSec_MuPpduDur; /*0: follow algo, otherwise, */

	/*TRIG Data Section*/
	UINT_8  u1TrigSec_BA_Policy;
	UINT_8  u1TrigSec_Global_BA_BW;
	UINT_32 u4TrigSec_Global_BA_Dur;

	/*Ext CmdEnd*/
	ENUM_MURU_TONE_PLAN_POLICY_T eTonePlanPolicy;
	UINT_8  u1FixedMcs;
	UINT_8  u1FixedNss;
	UINT_8  u1FixedBaMcs;
	UINT_8  u1FixedBaNss;
	UINT_32 u4PpduDuration; /*0 : No limit, 1~5484 us*/
	BOOL    fgUlMuBa;

	UINT_16 u2UlAvgMpduCnt; /*0: depends on algo. estimation, otherwise, fixed Mpdu Cnt*/
	UINT_32 u4UlAvgMpduSize;/*0: depends on algo. estimation, otherwise, fixed Mpdu Size*/
	UINT_8  u1MaxMuNum;
	UINT_8  u1TypeA_SwPdaPolicy; /*0: max t-put, 1: max Ppdu duration, 2: follow PLE Primary User*/
	UINT_8  u1TypeB_SwPdaPolicy; /*0: max t-put, 1: max Ppdu duration, 2: follow PLE Primary User*/
	UINT_16 u2MpduByte;          /*value = MPDU bytes used for ideal RA's PER calculation*/
	UINT_16 u2QidNeedsDlSplTrigger; /* bitwise record Qid which needs IO trigger DL SPL, SPL lost war */
	UINT_16 u2NonBsrpCount;   /* counter for bsrp debug record */
	MURU_SPL_BACKUP rSplDbgBackup[WH_TXC_ALTX0];
	UINT_8  u1SplBackupSeq;
	UINT_32 u4AcBitmapPreviousBsrp; /* per AC record if previous TXCMD is BSRP, for E2 cmdRpt lost war  */
	UINT_32 u4AcBitmapPreviousDlData; /* per AC record if previous TXCMD is DL, for E2 cmdRpt lost war  */
	UINT_8  u1TriggerTypeOfBsrpTimer;  /* assign TriggerType to UL_BSRP, default type is BSRP */
	UINT_8  u1DisableBsrpTimer;  /* 1: disable UL_BSRP even when fixTpNum > 0 */
	UINT_8  u1DisableULData;  /* 1: disable UL_DATA even when fixTpNum > 0 */
	BOOLEAN fgSr;  /*TRUE: SR SU TX in SR period, FALSE: after muruSuDataAlloc */
	UINT_32  u4SplWcidMissCnt;
	UINT_32  u4SplWcidMissCnt1;
	BOOLEAN fgBroadcastRu[RAM_BAND_NUM];  /* TRUE: 20TU probe rsp by broadcast RU, FALSE: by ALTX (algo don't care) */
	UINT_16 u2BroadcastRuWcid[RAM_BAND_NUM]; /* per band unicast wcid faked by broadcast RU */

	BOOLEAN fgDlOfdmaEn[RAM_BAND_NUM]; /* Switch control of DLOFDMA */

	BOOLEAN fgUlOfdmaEn[RAM_BAND_NUM]; /* Switch control of ULOFDMA */
	BOOLEAN fgDlMimoEn[RAM_BAND_NUM]; /* Switch control of DLMIMO */
	BOOLEAN fgUlMimoEn[RAM_BAND_NUM]; /* Switch control of ULMIMO */
	UINT_8  u1TpStaCntThPri80;
	UINT_8  u1TpStaCntThSec80;
	UINT_8  u1TpStaCntThFB160;
	BOOLEAN fgIsApTriggerMode[RAM_BAND_NUM]; /* perBandAP trigger/contetion mode record */
	UINT_8  fgTestCase562;
	UINT_16 u2MuEdcaTimerInTriggerMode;  /* MUEDCA value in Trigger mode AP BCN, default max 255 = 2sec */
	UINT_8  u1MuEdcaMaxDelayCnt;
} MURU_SHARE_DATA_T, *P_MURU_SHARE_DATA_T;

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/* This address is not generated by CODA and might be different by project */
#define WIFI_WTBL_BASE                  0x820D8000

#define LWTBL_CONFIG(_wlanIdx) \
	IO_W_32(WF_WTBLON_TOP_WDUCR_ADDR, \
		((_wlanIdx >> 7) & WF_WTBLON_TOP_WDUCR_GROUP_MASK) << WF_WTBLON_TOP_WDUCR_GROUP_SHFT)

#define LWTBL_IDX2BASE(_wlanIdx, _DW) \
	(WIFI_WTBL_BASE | ((_wlanIdx & 0x7F) << 8) | (_DW & 0x3F) << 2)

#define UWTBL_CONFIG(_wlanIdx) \
	IO_W_32(WF_UWTBL_TOP_WDUCR_ADDR, \
		((_wlanIdx >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK) << WF_UWTBL_TOP_WDUCR_GROUP_SHFT)

#define UWTBL_IDX2BASE(_wlanIdx, _DW) \
	(WIFI_UWTBL_BASE | 0x2000 | ((_wlanIdx & 0x7F) << 6) | (_DW & 0xF) << 2)

#define KEYTBL_CONFIG(_key_loc) \
	IO_W_32(WF_UWTBL_TOP_WDUCR_ADDR, \
		(WF_UWTBL_TOP_WDUCR_TARGET_MASK | (((_key_loc >> 7) & WF_UWTBL_TOP_WDUCR_GROUP_MASK) << WF_UWTBL_TOP_WDUCR_GROUP_SHFT)))

#define KEYTBL_IDX2BASE(_key_loc, _DW) \
	(WIFI_UWTBL_BASE | 0x2000 | ((_key_loc & 0x7F) << 6) | (_DW & 0xF) << 2)

enum HETB_TX_CTRL {
	HETB_TX_CFG = 0,
	HETB_TX_START = 1,
	HETB_TX_STOP = 2
};

/*
end of WTBL definition
Copy form BORA hal_wtbl_rom.c
*/
#endif /* __COMMON_CR_H__ */
