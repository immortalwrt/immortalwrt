/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attempt
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	net_adaption.h
*/
#ifndef __NET_ADAPTION_H__
#define __NET_ADAPTION_H__

#include "sys_adaption.h"

/*****************************************************************************
 *	Type definition
 *****************************************************************************/


/*****************************************************************************
 *	Macro
 *****************************************************************************/
/* Service DBDC configuration */
#define TEST_DBDC_BAND0		0
#define TEST_DBDC_BAND1		1

#ifdef DBDC_MODE
#define IS_TEST_DBDC(_test_winfo)	_test_winfo->dbdc_mode
#define TEST_DBDC_BAND_NUM		2
#else
#define IS_TEST_DBDC(_test_winfo)	FALSE
#define TEST_DBDC_BAND_NUM		1
#endif

/* Packet */
#define SERV_LENGTH_802_11		24

/* For rx stat type */
#define SERV_TEST_RX_STAT_MACFCSERRCNT			0x1
#define SERV_TEST_RX_STAT_MAC_MDRDYCNT			0x2
#define SERV_TEST_RX_STAT_PHY_MDRDYCNT			0x3
#define SERV_TEST_RX_STAT_PHY_FCSERRCNT			0x4
#define SERV_TEST_RX_STAT_PD				0x5
#define SERV_TEST_RX_STAT_CCK_SIG_SFD			0x6
#define SERV_TEST_RX_STAT_OFDM_SIG_TAG			0x7
#define SERV_TEST_RX_STAT_RSSI				0x8
#define SERV_TEST_RX_RESET_PHY_COUNT			0x9
#define SERV_TEST_RX_RESET_MAC_COUNT			0xa
#define SERV_TEST_RX_STAT_RSSI_RX23			0xB
#define SERV_TEST_RX_STAT_ACI_HITL			0xC
#define SERV_TEST_RX_STAT_ACI_HITH			0xD
#define SERV_TEST_RX_STAT_MACFCSERRCNT_BAND1		0xE
#define SERV_TEST_RX_STAT_MAC_MDRDYCNT_BAND1		0xF
#define SERV_TEST_RX_STAT_MAC_RXLENMISMATCH		0x10
#define SERV_TEST_RX_STAT_MAC_RXLENMISMATCH_BAND1	0x11
#define SERV_TEST_RX_FIFO_FULL_COUNT			0x12
#define SERV_TEST_RX_FIFO_FULL_COUNT_BAND1		0x13
#define SERV_TEST_RX_STAT_PHY_MDRDYCNT_BAND1		0x14
#define SERV_TEST_RX_STAT_PHY_FCSERRCNT_BAND1		0x15
#define SERV_TEST_RX_STAT_PD_BAND1			0x16
#define SERV_TEST_RX_STAT_CCK_SIG_SFD_BAND1		0x17
#define SERV_TEST_RX_STAT_OFDM_SIG_TAG_BAND1		0x18
#define SERV_TEST_RX_ACI_HIT				0x19
#define SERV_TEST_RX_STAT_MAC_FCS_OK_COUNT		0x1A

/* MAC behavior control */
#define SERV_TEST_MAC_TX		1
#define SERV_TEST_MAC_RX		2
#define SERV_TEST_MAC_TXRX		3
#define SERV_TEST_MAC_TXRX_RXV		4
#define SERV_TEST_MAC_RXV		5
#define SERV_TEST_MAC_RX_RXV		6

/* Setting max packet length to 13311 after MT7615 */
#define TEST_PKT_LEN		13311

#define TEST_MAX_PATTERN_SIZE	128

#define TEST_MAX_PKT_LEN	1496
#define TEST_MIN_PKT_LEN	25
#define TEST_MAX_BKCR_NUM	30

/* For packet tx time, in unit of byte */
#define TEST_MAX_HT_AMPDU_LEN		65000
#define TEST_MAX_VHT_MPDU_LEN		6700	/* 11454 */
#define TEST_DEFAULT_MPDU_LEN		4095
#define TEST_MAX_MSDU_LEN		2304
#define TEST_MIN_MSDU_LEN		22
#define TEST_DEFAULT_MAC_HDR_LEN	24
#define TEST_QOS_MAC_HDR_LEN		26

/* For ipg and duty cycle, in unit of us */
#define TEST_SIG_EXTENSION		6
#define TEST_DEFAULT_SLOT_TIME		9
#define TEST_DEFAULT_SIFS_TIME		10
/* ICR: 7-bit, ATCR/TRCR limitation: 8-bit/9-bit */
#define TEST_MAX_SIFS_TIME		127
#define TEST_MAX_AIFSN			0xF
#define TEST_MIN_AIFSN			0x1
#define TEST_MAX_CW			0x10
#define TEST_MIN_CW			0x0
#define TEST_NORMAL_CLOCK_TIME		50	/* in uint of ns */
#define TEST_BBP_PROCESSING_TIME	1500	/* in uint of ns */

/* Spec related definition */
#define TEST_RIFS_TIME			2	/* 802.11n */
/* Refine to 60 from 360 us, 2018.05.09 */
#define TEST_EIFS_TIME			60

/* The expected enqueue packet number when rx event trigger */
#define TEST_ENQ_PKT_NUM		100

#define TEST_RXV_SIZE	9
#define TEST_ANT_NUM	4
#define TEST_USER_NUM	16

/* MAC address length */
#define SERV_MAC_ADDR_LEN	6

/* Wcid related */
#define SERV_WCID_ALL		0xFF

#ifndef IFNAMELEN
#define IFNAMELEN 16
#endif

#define SERV_IOCTLBUFF 2048

/* Test log dump type */
#define fTEST_LOG_RXV			(1 << TEST_LOG_RXV)
#define fTEST_LOG_RDD			(1 << TEST_LOG_RDD)
#define fTEST_LOG_RE_CAL		(1 << TEST_LOG_RE_CAL)
#define fTEST_LOG_RXINFO		(1 << TEST_LOG_RXINFO)
#define fTEST_LOG_TXDUMP		(1 << TEST_LOG_TXDUMP)
#define fTEST_LOG_TEST			(1 << TEST_LOG_TEST)
#define fTEST_LOG_TXSSHOW		(1 << TEST_LOG_TXSSHOW)

#define TEST_RDD_DUMP_SIZE		80

/* Test ant user select */
#define TEST_ANT_USER_SEL	0x80000000

/* Test mps item length/stat */
#define TEST_MPS_ITEM_LEN	1024
#define TEST_MPS_ITEM_RUNNING	(1<<0)
#define BITS(m, n)              (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))

#if defined(DOT11_HE_AX)
#define MAX_MULTI_TX_STA 16
#else
#define MAX_MULTI_TX_STA 2
#endif

/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
/* Service queue id */
enum {
	SERV_QID_AC_BK,
	SERV_QID_AC_BE,
	SERV_QID_AC_VI,
	SERV_QID_AC_VO,
	SERV_QID_HCCA,
	SERV_QID_BMC = 8,
	SERV_QID_MGMT = 13,
	SERV_QID_RX = 14,
	SERV_QID_CTRL = 16,
	SERV_QID_BCN = 17,
};

/* Service rx filter packet type */
enum {
	SERV_RX_FILTER_STBC_BCN_BC_MC = 1 << 0,
	SERV_RX_FILTER_FCS_ERROR = 1 << 1,
	/* drop 802.11 protocol version not is 0 */
	SERV_RX_FILTER_PROTOCOL_VERSION = 1 << 2,
	SERV_RX_FILTER_PROB_REQ = 1 << 3,
	/* drop all mcast frame */
	SERV_RX_FILTER_MC_ALL = 1 << 4,
	SERV_RX_FILTER_BC_ALL = 1 << 5,
	/* drop mcast frame that is not in the mcast table */
	SERV_RX_FILTER_MC_TABLE = 1 << 6,
	/* drop bc/mc packet matches the following condition:   */
	/* ToDS=0,FromDS=1,A3=OwnMAC0 or OwnMAC1        */
	SERV_RX_FILTER_BC_MC_OWN_MAC_A3 = 1 << 7,
	/* drop bc/mc packet matches the following condition:   */
	/* ToDS=0,FromDS=0,A3!=BSSID0 or BSSID1         */
	SERV_RX_FILTER_BC_MC_DIFF_BSSID_A3 = 1 << 8,
	/* drop bc/mc packet matches the following condition:   */
	/* ToDS=0,FromDS=1,A2!=BSSID0 or BSSID1         */
	SERV_RX_FILTER_BC_MC_DIFF_BSSID_A2 = 1 << 9,
	/* drop bcn packet and A3!=BSSID0 or BSSID1 */
	SERV_RX_FILTER_BCN_DIFF_BSSID = 1 << 10,
	/* drop control packets with reserve type */
	SERV_RX_FILTER_CTRL_RSV = 1 << 11,
	SERV_RX_FILTER_CTS = 1 << 12,
	SERV_RX_FILTER_RTS = 1 << 13,
	/* drop duplicate frame, BA session not includign in this filter */
	SERV_RX_FILTER_DUPLICATE = 1 << 14,
	/* drop not my BSSID0/1/2/3 if enabled */
	SERV_RX_FILTER_NOT_OWN_BSSID = 1 << 15,
	/* drop uncast packet not to OWN MAC 0/1/2/3/4 */
	SERV_RX_FILTER_NOT_OWN_UCAST = 1 << 16,
	/* drop diff bassid TIM broadcast */
	SERV_RX_FILTER_NOT_OWN_BTIM = 1 << 17,
	/*drop NDPA control frame */
	SERV_RX_FILTER_NDPA = 1 << 18,
};

/* Test DBDC band mode for QA */
enum test_band_mode {
	TEST_BAND_MODE_UNUSE = 0,
	TEST_BAND_MODE_SINGLE,
	TEST_BAND_MODE_DUAL
};

/* Test DBDC band type for QA */
enum test_band_type {
	TEST_BAND_TYPE_UNUSE = 0,
	TEST_BAND_TYPE_2_4G,
	TEST_BAND_TYPE_5G,
	TEST_BAND_TYPE_2_4G_5G,
	TEST_BAND_TYPE_6G,
	TEST_BAND_TYPE_2_4G_6G,
	TEST_BAND_TYPE_5G_6G,
	TEST_BAND_TYPE_2_4G_5G_6G,
};

/* Test DBDC enable for QA */
enum test_dbdc_enable {
	TEST_DBDC_DISABLE = 0,
	TEST_DBDC_ENABLE
};

/* Test backup CR type */
enum test_bk_cr_type {
	SERV_TEST_EMPTY_BKCR = 0,
	SERV_TEST_MAC_BKCR,
	SERV_TEST_HIF_BKCR,
	SERV_TEST_PHY_BKCR,
	SERV_TEST_HW_BKCR,
	SERV_TEST_MCU_BKCR,
	SERV_TEST_BKCR_TYPE_NUM
};

/* Test tx strategy */
enum test_tx_strategy {
	TEST_TX_STRA_TASKLET = 0,
	TEST_TX_STRA_THREAD
};

/* Test tx type */
enum test_tx_type {
	TEST_TX_TYPE_TXD = 0,
	TEST_TX_TYPE_WTBL
};

/* Test rx statistic type */
enum test_rx_stat_type {
	TEST_RX_STAT_RXV,
	TEST_RX_STAT_PER_PKT,
	TEST_RX_STAT_RESET_CNT,
	TEST_RX_STAT_COUNTER_802_11,
	TEST_RX_STAT_STAT_TYPE_NUM
};

/* Test bw definition */
enum test_bw_type {
	TEST_BW_20 = 0,
	TEST_BW_40,
	TEST_BW_80,
	TEST_BW_10,
	TEST_BW_5,
	TEST_BW_160C,
	TEST_BW_160NC,
	TEST_BW_NUM
};

/* Test HE LTF definition */
enum test_he_ltf_type {
	TEST_HE_LTF_X1,
	TEST_HE_LTF_X2,
	TEST_HE_LTF_X4,
};

/* Test HE GI definition */
enum test_he_gi_type {
	TEST_GI_8,
	TEST_GI_16,
	TEST_GI_32,
};

/* Test phy mode definition */
enum test_phy_mode_type {
	TEST_MODE_CCK = 0,
	TEST_MODE_OFDM,
	TEST_MODE_HTMIX,
	TEST_MODE_HTGREENFIELD,
	TEST_MODE_VHT,
	TEST_MODE_HE_24G,
	TEST_MODE_HE_5G,
	TEST_MODE_HE_SU = 8,
	TEST_MODE_HE_ER,
	TEST_MODE_HE_TB,
	TEST_MODE_HE_MU,
	TEST_MODE_VHT_MIMO,
	TEST_MODE_NUM
};

/* Test payload policy */
enum {
	TEST_USER_PAYLOAD = 0,
	TEST_FIXED_PAYLOAD,
	TEST_RANDOM_PAYLOAD
};

/* Test log dump type */
enum {
	TEST_LOG_RXV = 1,
	TEST_LOG_RDD,
	TEST_LOG_RE_CAL,
	TEST_LOG_TYPE_NUM,
	TEST_LOG_RXINFO,
	TEST_LOG_TXDUMP,
	TEST_LOG_TEST,
	TEST_LOG_TXSSHOW,
};

enum {
	TEST_LOG_OFF,
	TEST_LOG_ON,
	TEST_LOG_DUMP,
	TEST_LOG_CTRL_NUM,
};

enum {
	TEST_RX_STAT_BAND = 0,
	TEST_RX_STAT_PATH,
	TEST_RX_STAT_USER,
	TEST_RX_STAT_COMM,
	TEST_RX_STAT_NUM
};

/* test rxv action category */
enum {
	TEST_RXV_DUMP_START = 0,
	TEST_RXV_DUMP_STOP,
	TEST_RXV_DUMP_REPORT,
	TEST_RXV_DUMP_CLEAR_BUFFER,
	TEST_RXV_DUMP_ACTION_NUM
};

#define TEST_CCK_SHORT_PREAMBLE 0x8
#define TEST_CCK_RATE_MASK 0x3

/*****************************************************************************
 *	Data struct definition
 *****************************************************************************/
/* Service IOCTL related definitions */
struct serv_ioctl_input {
	union {
		s_char ifrn_name[IFNAMELEN];	/* if name, e.g. "eth0" */
	} ifr_ifrn;
	union {
		s_char *name;
		struct {
			s_char *pointer;
			u_int16 length;
			u_int16 flags;
		} data;
	} u;
};

/* Servoce spec related data struct */
struct GNU_PACKED serv_frame_control {
#ifdef RT_BIG_ENDIAN
	u_int16 order:1;	/* Strict order expected */
	u_int16 wep:1;		/* Wep data */
	u_int16 more_data:1;	/* More data bit */
	u_int16 pwr_mgmt:1;	/* Power management bit */
	u_int16 retry:1;	/* Retry status bit */
	u_int16 more_frag:1;	/* More fragment bit */
	u_int16 fr_ds:1;	/* From DS indication */
	u_int16 to_ds:1;	/* To DS indication */
	u_int16 sub_type:4;	/* MSDU subtype */
	u_int16 type:2;		/* MSDU type */
	u_int16 ver:2;		/* Protocol version */
#else
	u_int16 ver:2;		/* Protocol version */
	u_int16 type:2;		/* MSDU type, refer to FC_TYPE_XX */
	u_int16 sub_type:4;	/* MSDU subtype, refer to  SUBTYPE_XXX */
	u_int16 to_ds:1;	/* To DS indication */
	u_int16 fr_ds:1;	/* From DS indication */
	u_int16 more_frag:1;	/* More fragment bit */
	u_int16 retry:1;	/* Retry status bit */
	u_int16 pwr_mgmt:1;	/* Power management bit */
	u_int16 more_data:1;	/* More data bit */
	u_int16 wep:1;		/* Wep data */
	u_int16 order:1;	/* Strict order expected */
#endif				/* !RT_BIG_ENDIAN */
};

struct GNU_PACKED serv_hdr_802_11 {
	struct serv_frame_control fc;
	u_int16 duration;
	u_char addr1[6];
	u_char addr2[6];
	u_char addr3[6];
#ifdef RT_BIG_ENDIAN
	u_int16 sequence:12;
	u_int16 frag:4;
#else
	u_int16 frag:4;
	u_int16 sequence:12;
#endif				/* !RT_BIG_ENDIAN */
	u_char octet[0];
};

/* Service fw related information */
struct serv_fw_info {
	boolean ra_offload;
	u_int8 chip_id; /* different with top cr*/
	u_int8 eco_ver;
	u_int8 num_of_region;
	u_int8 format_ver;
	u_int8 format_flag;
	u_int8 ram_ver[10];
	u_int8 ram_built_date[15];
	u_int32 crc;
};

/* Service chip capability related definition */
struct serv_mcs_nss_caps {
	boolean g_band_256_qam;
	u_int8 max_nss;
	u_int8 max_vht_mcs;
	u_int8 bw160_max_nss;
};

struct serv_qos_caps {
	u_char wmm_hw_num;
	u_char wmm_detect_method;
	u_int32 txop_scenario;
	u_int32 current_txop;
	u_int32 default_txop;
};

struct serv_spe_map {
	u_int8 ant_sel;
	u_int8 spe_idx;
};

struct serv_spe_map_list {
	struct serv_spe_map *spe_map;
	u_int8 size;
};

struct serv_chip_cap {
	/* ------------------------ packet --------------------- */
	/* TxWI or LMAC TxD max size */
	u_int8 tx_wi_size;
	/* RxWI or LMAC RxD max size */
	u_int8 rx_wi_size;
	/* Tx Hw meta info size which including all hw info fields */
	u_int8 tx_hw_hdr_len;
	/* Rx Hw meta info size */
	u_int8 rx_hw_hdr_len;
	u_int8 num_of_tx_ring;
	u_int8 num_of_rx_ring;
	u_int16 tx_ring_size;
	u_int8 ht_ampdu_exp;
	u_int16 non_he_tx_ba_wsize;
	u_int8 max_mpdu_len;
	u_int8 vht_ampdu_exp;
	u_int16 he_tx_ba_wsize;
	u_int8 he_ampdu_exp;
	u_int32 efuse_size;
	struct serv_mcs_nss_caps mcs_nss;
	struct serv_qos_caps qos;
	struct serv_spe_map_list spe_map_list;
	boolean swq_per_band;
	boolean ra_offload;
	boolean support_6g;
};

/* Service channel configuration */
struct serv_channel_cfg {
	u_char ctrl_channel;
	/*Only used for 80+80 case */
	u_char ctrl_channel2;
	u_char central_channel;
	u_char bw;
	u_char tx_stream;
	u_char rx_stream;
	boolean scan;
	boolean dfs_check;
	u_char band_idx;
	u_char ch_band;
	u_int32 out_band_freq;
};

/* Test data rate map */
struct test_data_rate_map {
	u_char mcs;		/* MCS index */
	u_int32 tx_data_rate;	/* Data rate in K bit */
};

/* Test data aggregation threshold */
struct test_datalen_limit_map {
	u_char phy_mode;		/* MCS index */
	u_int32 amsdu_limit;	/* Data rate in K bit */
};

/* Test ant to spe_idx map */
struct test_ant_map {
	u_int32 ant_sel;
	u_int32 spe_idx;
};

/* Test rx filter */
/* TODO: factor out here for naming */
struct rx_filter_ctrl {
	u_int32 filterMask;
	boolean bPromiscuous;
	boolean bFrameReport;
	u_char u1BandIdx;
};

/* Test backup CR */
struct test_bk_cr {
	u_long offset;
	u_int32 val;
	enum test_bk_cr_type type;
};

#ifdef RT_BIG_ENDIAN
union _test_eeprom_antenna {
	struct {
		u_short RssiIndicationMode:1;	/* RSSI indication mode */
		u_short Rsv:1;
		u_short BoardType:2;		/* 0: mini card; 1: USB pen */
		u_short RfIcType:4;		/* see E2PROM document */
		u_short TxPath:4;		/* 1: 1T, 2: 2T, 3: 3T */
		u_short RxPath:4;		/* 1: 1R, 2: 2R, 3: 3R */
	} field;
	u_short word;
};
#else
union _test_eeprom_antenna {
	struct {
		u_short RxPath:4;		/* 1: 1R, 2: 2R, 3: 3R */
		u_short TxPath:4;		/* 1: 1T, 2: 2T, 3: 3T */
		u_short RfIcType:4;		/* see E2PROM document */
		u_short BoardType:2;		/* 0: mini card; 1: USB pen */
		u_short Rsv:1;
		u_short RssiIndicationMode:1;	/* RSSI indication mode */
	} field;
	u_short word;
};
#endif


/* Test backup params from normal */
struct test_backup_params {
	boolean en_tx_burst;
	boolean en_bss_coex;
	u_int16 bcn_prd;
	boolean premable;
	boolean greenap;
	union _test_eeprom_antenna antenna;
	u_int8 dbdc_band0_tx_path;
	u_int8 dbdc_band0_rx_path;
	u_int8 dbdc_band1_tx_path;
	u_int8 dbdc_band1_rx_path;
	u_int8 dbdc_band0_tx_num;
	u_int8 dbdc_band0_rx_num;
	u_int8 dbdc_band1_tx_num;
	u_int8 dbdc_band1_rx_num;
};

/* Test tx counters */
struct test_tx_statistic {
	u_int32 tx_cnt;
	u_int32 tx_done_cnt;	/* Tx DMA Done */
	u_int32 txed_cnt;
};

/* Test rx stat band info */
struct test_rx_stat_band_info {
	u_int32 mac_rx_fcs_err_cnt;
	u_int32 mac_rx_mdrdy_cnt;
	u_int32 mac_rx_len_mismatch;
	u_int32 mac_rx_fcs_ok_cnt;
	u_int32 phy_rx_fcs_err_cnt_cck;
	u_int32 phy_rx_fcs_err_cnt_ofdm;
	u_int32 phy_rx_pd_cck;
	u_int32 phy_rx_pd_ofdm;
	u_int32 phy_rx_sig_err_cck;
	u_int32 phy_rx_sfd_err_cck;
	u_int32 phy_rx_sig_err_ofdm;
	u_int32 phy_rx_tag_err_ofdm;
	u_int32 phy_rx_mdrdy_cnt_cck;
	u_int32 phy_rx_mdrdy_cnt_ofdm;
};

/* Test rx stat path info */
struct test_rx_stat_path_info {
	u_int32 rcpi;
	u_int32 rssi;
	u_int32 fagc_ib_rssi;
	u_int32 fagc_wb_rssi;
	u_int32 inst_ib_rssi;
	u_int32 inst_wb_rssi;
};

/* Test rx stat user info */
struct test_rx_stat_user_info {
	s_int32 freq_offset_from_rx;
	u_int32 snr;
	u_int32 fcs_error_cnt;
};

/* Test rx stat comm info */
struct test_rx_stat_comm_info {
	u_int32 rx_fifo_full;
	u_int32 aci_hit_low;
	u_int32 aci_hit_high;
	u_int32 mu_pkt_count;
	u_int32 sig_mcs;
	u_int32 sinr;
	u_int32 driver_rx_count;
};

/* Test rx stat */
struct test_rx_stat {
	struct test_rx_stat_band_info rx_st_band[TEST_DBDC_BAND_NUM];
	struct test_rx_stat_path_info rx_st_path[TEST_ANT_NUM];
	struct test_rx_stat_user_info rx_st_user[TEST_USER_NUM];
	struct test_rx_stat_comm_info rx_st_comm;
};

/* Test rx stat union */
struct test_rx_stat_u {
	union {
		struct test_rx_stat_band_info rx_st_band;
		struct test_rx_stat_path_info rx_st_path;
		struct test_rx_stat_user_info rx_st_user;
		struct test_rx_stat_comm_info rx_st_comm;
	} u;
};

struct GNU_PACKED test_rx_stat_leg {
	u_int32 mac_rx_fcs_err_cnt;
	u_int32 mac_rx_mdrdy_cnt;
	u_int32 phy_rx_fcs_err_cnt_cck;
	u_int32 phy_rx_fcs_err_cnt_ofdm;
	u_int32 phy_rx_pd_cck;
	u_int32 phy_rx_pd_ofdm;
	u_int32 phy_rx_sig_err_cck;
	u_int32 phy_rx_sfd_err_cck;
	u_int32 phy_rx_sig_err_ofdm;
	u_int32 phy_rx_tag_err_ofdm;
	u_int32 wb_rssi0;
	u_int32 ib_rssi0;
	u_int32 wb_rssi1;
	u_int32 ib_rssi1;
	u_int32 phy_rx_mdrdy_cnt_cck;
	u_int32 phy_rx_mdrdy_cnt_ofdm;
	u_int32 driver_rx_cnt;
	u_int32 rcpi0;
	u_int32 rcpi1;
	s_int32 freq_offset_rx;
	u_int32 rssi0;
	u_int32 rssi1;
	u_int32 rx_fifo_full;
	u_int32 mac_rx_len_mismatch;
	u_int32 mac_rx_fcs_err_cnt_band1;
	u_int32 mac_rx_mdrdy_cnt_band1;
	u_int32 fagc_ib_rssi[TEST_ANT_NUM];
	u_int32 fagc_wb_rssi[TEST_ANT_NUM];
	u_int32 inst_ib_rssi[TEST_ANT_NUM];
	u_int32 inst_wb_rssi[TEST_ANT_NUM];
	u_int32 aci_hit_low;
	u_int32 aci_git_high;
	u_int32 driver_rx_cnt1;
	u_int32 rcpi2;
	u_int32 rcpi3;
	u_int32 rssi2;
	u_int32 rssi3;
	u_int32 snr0;
	u_int32 snr1;
	u_int32 snr2;
	u_int32 snr3;
	u_int32 rx_fifo_full_band1;
	u_int32 mac_rx_len_mismatch_band1;
	u_int32 phy_rx_pd_cck_band1;
	u_int32 phy_rx_pd_ofdm_band1;
	u_int32 phy_rx_sig_err_cck_band1;
	u_int32 phy_rx_sfd_err_cck_band1;
	u_int32 phy_rx_sig_err_ofdm_band1;
	u_int32 phy_rx_tag_err_ofdm_band1;
	u_int32 phy_rx_mdrdy_cnt_cck_band1;
	u_int32 phy_rx_mdrdy_cnt_ofdm_band1;
	u_int32 phy_rx_fcs_err_cnt_cck_band1;
	u_int32 phy_rx_fcs_err_cnt_ofdm_band1;
	u_int32 mu_pkt_cnt;
	u_int32 sig_mcs;
	u_int32 sinr;
	u_int32 rxv_rssi;
	u_int32 mac_rx_fcs_ok_cnt;
	u_int32 leg_rssi_sub[8];
	s_int32 user_rx_freq_offset[TEST_USER_NUM];
	u_int32 user_snr[TEST_USER_NUM];
	u_int32 fcs_error_cnt[TEST_USER_NUM];
};

struct GNU_PACKED rxv_dump_ring_attr {
	u_int8 type_mask;
	u_int8 ring_idx;
	u_int8 valid_entry_num;
	u_int8 dump_entry_total_num;
};

/* For mobile temp use */
struct GNU_PACKED hqa_m_rx_stat {
	u_int32 mac_rx_fcs_err_cnt;
	u_int32 mac_rx_mdrdy_cnt;
	u_int32 phy_rx_fcs_err_cnt_cck;
	u_int32 phy_rx_fcs_err_cnt_ofdm;
	u_int32 phy_rx_pd_cck;
	u_int32 phy_rx_pd_ofdm;
	u_int32 phy_rx_sig_err_cck;
	u_int32 phy_rx_sfd_err_cck;
	u_int32 phy_rx_sig_err_ofdm;
	u_int32 phy_rx_tag_err_ofdm;
	u_int32 wb_rssi0;
	u_int32 ib_rssi0;
	u_int32 wb_rssi1;
	u_int32 ib_rssi1;
	u_int32 phy_rx_mdrdy_cnt_cck;
	u_int32 phy_rx_mdrdy_cnt_ofdm;
	u_int32 driver_rx_count;
	u_int32 rcpi0;
	u_int32 rcpi1;
	s_int32 freq_offset_from_rx;
	u_int32 rssi0;
	u_int32 rssi1;
	u_int32 rx_fifo_full;  /* out_of_resource */
	u_int32 mac_rx_len_mismatch;
	u_int32 mac_rx_fcs_err_cnt_band1;
	u_int32 mac_rx_mdrdy_cnt_band1;
	u_int32 fagc_ib_RSSSI0;
	u_int32 fagc_ib_RSSSI1;
	u_int32 fagc_ib_RSSSI2;
	u_int32 fagc_ib_RSSSI3;
	u_int32 fagc_wb_RSSSI0;
	u_int32 fagc_wb_RSSSI1;
	u_int32 fagc_wb_RSSSI2;
	u_int32 fagc_wb_RSSSI3;
	u_int32 inst_ib_RSSSI0;
	u_int32 inst_ib_RSSSI1;
	u_int32 inst_ib_RSSSI2;
	u_int32 inst_ib_RSSSI3;
	u_int32 inst_wb_RSSSI0;
	u_int32 inst_wb_RSSSI1;
	u_int32 inst_wb_RSSSI2;
	u_int32 inst_wb_RSSSI3;
	u_int32 aci_hit_low;
	u_int32 aci_hit_high;
	u_int32 driver_rx_count1;
	u_int32 rcpi2;
	u_int32 rcpi3;
	u_int32 rssi2;
	u_int32 rssi3;
	u_int32 snr0;
	u_int32 snr1;
	u_int32 snr2;
	u_int32 snr3;
	u_int32 rx_fifo_full_band1;
	u_int32 mac_rx_len_mismatch_band1;
	u_int32 phy_rx_pd_cck_band1;
	u_int32 phy_rx_pd_ofdm_band1;
	u_int32 phy_rx_sig_err_cck_band1;
	u_int32 phy_rx_sfd_err_cck_band1;
	u_int32 phy_rx_sig_err_ofdm_band1;
	u_int32 phy_rx_tag_err_ofdm_band1;
	u_int32 phy_rx_mdrdy_cnt_cck_band1;
	u_int32 phy_rx_mdrdy_cnt_ofdm_band1;
	u_int32 phy_rx_fcs_err_cnt_cck_band1;
	u_int32 phy_rx_fcs_err_cnt_ofdm_band1;
	u_int32 mu_pkt_count;
	u_int32 sig_mcs;
	u_int32 sinr;
	u_int32 rxv_rssi;
	/* u_int32 reserved[184]; */
	u_int32 phy_mdrdy;
	u_int32 noise_floor;
	u_int32 all_len_mismatch_ch_cnt_band0;
	u_int32 all_len_mismatch_ch_cnt_band1;
	u_int32 all_mac_mdrdy0;
	u_int32 all_mac_mdrdy1;
	u_int32 all_fcs_err0;
	u_int32 all_fcs_err1;
	u_int32 rx_ok0;
	u_int32 rx_ok1;
	u_int32 per0;
	u_int32 per1;
};

struct GNU_PACKED hqa_comm_rx_stat {
	union {
		struct hqa_m_rx_stat r_test_m_hqa_rx_stat;
	} u;
};

/* Test mps for service */
struct test_mps_setting {
	u_int32 tx_mode;
	u_int32 tx_ant;
	u_int32 mcs;
	u_int32 pkt_len;
	u_int32 pkt_cnt;
	u_int32 pwr;
	u_int32 nss;
	u_int32 pkt_bw;
};

struct test_mps_cb {
	SERV_OS_SPIN_LOCK lock;
	u_int32 mps_cnt;
	u_int32 stat;
	boolean setting_inuse;
	u_int32 ref_idx;
	struct test_mps_setting *mps_setting;
};

/* Test tx time feature parameters */
struct tx_time_param {
	/* The packet transmission time feature enable or disable */
	boolean pkt_tx_time_en;
	/* The target packet transmission time */
	u_int32 pkt_tx_time;
};

/* Test MPDU parameters */
struct tx_mpdu_info {
	/* MPDU length in byte */
	u_int32 tx_len;
	/* MSDU length in bye */
	u_int32 msdu_len;
	u_int32 hdr_len;
	/* count for aggregated MPDU */
	u_int32 mpdu_cnt;
	/* mark as QOS data required */
	u_int8 need_qos;
	/* mark as A-MSDU required */
	u_int8 need_amsdu;
	/* mark as AMPDU equired */
	u_int8 need_ampdu;
};

/* Test ipg feature parameters */
struct ipg_param {
	/* The target idle time */
	u_int32 ipg;
	/* Only OFDM/HT/VHT need to consider sig_ext */
	u_int8 sig_ext;
	u_int16 slot_time;
	u_int16 sifs_time;
	/* 0: AC_BK, 1: AC_BE, 2: AC_VI, 3: AC_VO */
	u_int8 ac_num;
	u_int8 aifsn;
	u_int16 cw;
	u_int16 txop;
};

/* Test tx power feature parameters */
struct test_txpwr_param {
	u_int32 ant_idx;
	u_int32 power;
	u_int32 channel;
	u_int32 band_idx;
	u_int32 ch_band;
};

/* Test off channel scan parameters */
struct test_off_ch_param {
	u_int32 ext_id;			/* ExtendId of command */
	u_int32 dbdc_idx;		/* DBDC index */
	u_int32 mntr_ch;		/* Monitoring channel */
	u_int32 is_aband;		/* 0: 2.4G channel, 1: 5G channel */
	u_int32 mntr_bw;		/* Bandwidth of monitoring channel */
	u_int32 mntr_tx_rx_pth;	/* Monitoring TX/RX stream path */
	u_int32 scan_mode;		/* ScanStart/ScanRunning/ScanStop */
};

/* Test band state for UI/driver communication */
struct test_band_state {
	/* The single/dual band which UI wants to show */
	u_int32 band_mode;
	/* The a/g band type which driver shall configure */
	u_int32 band_type;
};

struct test_ru_info {
	boolean valid;
	u_int32 aid;
	u_int32 allocation;
	u_int32 ru_index;
	u_int32 rate;
	u_int32 ldpc;
	u_int32 nss;
	u_int32 start_sp_st;
	u_int32 mpdu_length;
	s_int32 alpha;
	u_int32 ru_mu_nss;
	/* end of user input*/
	u_int32 t_pe;
	u_int32 afactor_init;
	u_int32 symbol_init;
	u_int32 excess;
	u_int32 dbps;
	u_int32 cbps;
	u_int32 dbps_s;
	u_int32 cbps_s;
	u_int32 pld;
	u_int32 avbits;
	u_int32 dbps_last;
	u_int32 cbps_last;
	u_int8 ldpc_extr_sym;
	u_int32 tx_time_x5;
	u_int8 pe_disamb;
	s_int16 punc;
	u_int32 l_len;
};

struct test_tx_info {
	u_int16 aid;
	u_int8 tx_mode;
	u_int8 bw;
	u_int8 stbc;
	u_int8 ldpc;
	u_int8 ltf;
	u_int8 gi;
	u_int8 mcs;
	u_int8 nss;
	u_int8 ibf;
	u_int8 ebf;
	struct tx_mpdu_info mpdu_info;
};

/* Tx stack entry for service */
struct test_tx_stack {
	u_int8 entry_limit;
	u_int8 index;
	u_int8 q_idx;
	u_int16 quota;
	u_int8 da[MAX_MULTI_TX_STA][SERV_MAC_ADDR_LEN];
	void *virtual_wtbl[MAX_MULTI_TX_STA];
	void *virtual_device[MAX_MULTI_TX_STA];
	void *pkt_skb[MAX_MULTI_TX_STA];
	struct test_tx_info tx_info[MAX_MULTI_TX_STA];
};

struct test_ru_allocatoin {
	/* maximum 8 sub-20MHz for 160/80+80 MHz bandwidth */
	u_int8 sub20[8];
};

/* Test configuration for service */
struct test_configuration {
	/* Test operated mode */
	u_int32 op_mode;

	/* Test packet */
	u_char *test_pkt;	/* Buffer for test packet */
	void *pkt_skb;
	u_int32 is_alloc_skb;

	/* OS related  */
	SERV_OS_COMPLETION tx_wait;
	u_char tx_status;	/* 0: task idle, 1: task is running */

	/* Hardware resource */
	u_char wdev_idx;
	u_char wmm_idx;
	u_short ac_idx;
	void *wdev[2];

	/* Wifi related */
	u_int8 wcid_ref;

	/* Tx frame */
	u_char template_frame[32];
	u_char addr1[MAX_MULTI_TX_STA][SERV_MAC_ADDR_LEN];
	u_char addr2[MAX_MULTI_TX_STA][SERV_MAC_ADDR_LEN];
	u_char addr3[MAX_MULTI_TX_STA][SERV_MAC_ADDR_LEN];
	u_char payload[TEST_MAX_PATTERN_SIZE];
	u_short dur;
	u_short seq;
	u_short hdr_len;	/* Header Length */
	u_int32 pl_len;
	u_int32 tx_len;
	u_int32 fixed_payload;	/* Normal:0,Repeat:1,Random:2 */

	/* Tx */
	u_int8 tx_strategy;
	u_int8 tx_method[TEST_MODE_NUM];
	struct test_tx_stack stack;
	struct test_ru_info ru_info_list[MAX_MULTI_TX_STA];
	u_int8 dmnt_ru_idx;
	struct test_ru_allocatoin ru_alloc;
	u_int8 rate_ctrl_type;
	u_int32 duty_cycle;
	struct tx_time_param tx_time_param;
	struct ipg_param ipg_param;
	u_int8 retry;

	/* Rx */
	u_char own_mac[SERV_MAC_ADDR_LEN];
	u_int8 rx_filter_en;
	u_int32 rx_filter_pkt_len;
	u_int32 mu_rx_aid;

	/* Test Tx statistic */
	boolean txs_enable;
	struct test_tx_statistic tx_stat;

	/* Phy */
	u_int32 backup_tx_ant;
	u_int32 tx_ant;
	u_int16 backup_rx_ant;
	u_int16 rx_ant;
	u_char channel;
	u_char ch_band;
	u_char ctrl_ch;
	u_int8 pri_sel;
	s_int8 ch_offset;
	u_char tx_mode;
	u_char bw;
	u_char per_pkt_bw;
	u_char mcs;
	u_char nss;
	u_char stbc;
	u_char ldpc;		/* 0:BCC 1:LDPC */
	u_char sgi;
	u_char preamble;
	u_char fagc_path;
	u_char tx_strm_num;	/* TX stream number for channel */
	u_char rx_strm_pth;	/* RX antenna path for channel */
	u_char backup_channel;
	u_char backup_phymode;
	u_char channel_2nd;
	u_int32 out_band_freq;
	u_int32 rf_freq_offset;
	u_int32 thermal_val;
	u_int8 max_pkt_ext;
	u_int64 hetb_rx_csd;
	u_int16 user_idx;

	/* Tx power */
	struct test_txpwr_param pwr_param;
	s_int8 tx_pwr[TEST_ANT_NUM];
	boolean tx_pwr_sku_en;	/* sku on/off status */
	boolean tx_pwr_percentage_en;	/* power percentage on/off status */
	boolean tx_pwr_backoff_en;	/* backoff on/off status */
	u_int32 tx_pwr_percentage_level;	/* tx power percentage level */

	/* Tx Tone */
	u_int32 tx_tone_en;
	u_int32 ant_idx;
	u_int32 tone_type;
	u_int32 tone_freq;
	u_int32 dc_offset_I;
	u_int32 dc_offset_Q;
	u_int32 rf_pwr;
	u_int32 digi_pwr;

	/* Continuous Tx */
	u_int32 ant_mask;
	u_int32 rate;
	u_int32 tx_fd_mode;

	/* Set Cfg on off */
	u_char log_type;
	u_char log_enable;

	/* MPS related */
	struct test_mps_cb mps_cb;

	/*iBF, eBF*/
	u_char ibf;
	u_char ebf;

	/* iBF phase cal */
	boolean fgEBfEverEnabled;
	boolean fgBw160;
	u_char *txbf_info;
	u_int32 txbf_info_len;
	u_char  iBFCalStatus;

	/* off ch scan */
	struct test_off_ch_param off_ch_param;
};

/* Test wlan information for service */
struct test_wlan_info {
	void *os_cookie;	/* save specific structure relative to OS */
	struct net_device *net_dev;
	u_int32 chip_id;

	/* fw info */
	struct serv_fw_info wm_fw_info;

	/* hdev */
	void *hdev_ctrl;

	/* DBDC */
	boolean dbdc_mode;

	/* Token HW resource */
	u_int32 pkt_tx_tkid_max;

	/* Chip cap */
	struct serv_chip_cap chip_cap;

	/* EEPROM related information */
	boolean use_efuse;
	u_char e2p_cur_mode;
	u_char e2p_access_mode;

	/*========== gen4m only ==========*/
	/* wlan oid handler */
	wlan_oid_handler_t oid_funcptr;

	/*connsys emi phy memory start addr*/
	phys_addr_t emi_phy_base;

	/*connsys emi total phy memory size*/
	unsigned long long emi_phy_size;
};

/* Test control register read/write for service */
struct test_register {
	u_int32 cr_addr;
	/* For read/write bulk cr usage */
	u_int16 cr_num;
	/* Determine size by cr_num */
	u_int32 *cr_val;
	/* For rf reg read/write only */
	u_int32 wf_sel;
};

/* Test eeprom read/write for service */
struct test_eeprom {
	u_int32 offset;
	/* For read/write bulk cr usage */
	u_int32 length;
	/* Determine size by length */
	u_int16 *value;
	/* For get efuse block count only */
	u_int32 efuse_free_block;
};

/* Pulse size * num of pulse = 8 * 32 for one event*/
#define TEST_RDD_LOG_SIZE 8
struct test_rdd_log {
	u_int32 prefix;
	u_int32 cnt;
	u_char by_pass;
	u_char buffer[TEST_RDD_LOG_SIZE];
};

struct test_recal_log {
	u_int32 cal_idx;
	u_int32 cal_type;
	u_int32 cr_addr;
	u_int32 cr_val;
};


struct test_log_dump_entry {
	u_int32 log_type;
	u_char un_dumped;
	union {
		/*struct _TEST_RXV_LOG rxv;*/
		struct test_rdd_log rdd;
		/*struct _TEST_LOG_RECAL re_cal;*/
	} log;
};

struct test_log_dump_cb {
	SERV_OS_SPIN_LOCK lock;
	u_char overwritable;
	boolean first_en;
	u_char is_dumping;
	u_char is_overwritten;
	s_int32 idx;
	s_int32 len;
	u_int32 recal_curr_type;
	struct test_log_dump_entry *entry;
};

/* Test operation hook handlers for service */
struct test_operation {
	s_int32 (*op_set_tr_mac)(
		struct test_wlan_info *winfos,
		s_int32 op_type, boolean enable, u_char band_idx);
	s_int32 (*op_set_tx_stream)(
		struct test_wlan_info *winfos,
		u_int32 stream_nums, u_char band_idx);
	s_int32 (*op_set_tx_path)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_set_rx_path)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_set_rx_filter)(
		struct test_wlan_info *winfos,
		struct rx_filter_ctrl rx_filter);
	s_int32 (*op_set_clean_persta_txq)(
		struct test_wlan_info *winfos,
		boolean sta_pause_enable,
		void *virtual_wtbl,
		u_char wdev_idx,
		u_char band_idx);
	s_int32 (*op_log_on_off)(
		struct test_wlan_info *winfos,
		struct test_log_dump_cb *log_cb,
		u_int32 log_type,
		u_int32 log_ctrl,
		u_int32 log_size);
	s_int32 (*op_dbdc_tx_tone)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_dbdc_tx_tone_pwr)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_dbdc_continuous_tx)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_get_tx_info)(
		struct test_wlan_info *winfos,
		struct test_configuration *test_configs_band0,
		struct test_configuration *test_configs_band1);
	s_int32 (*op_set_antenna_port)(
		struct test_wlan_info *winfos,
		u_int8 rf_mode_mask, u_int8 rf_port_mask,
		u_int8 ant_port_mask);
	s_int32 (*op_set_slot_time)(
		struct test_wlan_info *winfos,
		u_int8 slot_time, u_int8 sifs_time,
		u_int8 rifs_time, u_int16 eifs_time,
		u_char band_idx);
	s_int32 (*op_set_power_drop_level)(
		struct test_wlan_info *winfos,
		u_int8 pwr_drop_level, u_char band_idx);
	s_int32 (*op_set_rx_filter_pkt_len)(
		struct test_wlan_info *winfos,
		u_int8 enable, u_char band_idx, u_int32 rx_pkt_len);
	s_int32 (*op_get_antswap_capability)(
			struct test_wlan_info *winfos,
			u_int32 *antswap_support);
	s_int32 (*op_set_antswap)(
			struct test_wlan_info *winfos,
			u_int32 ant);
	s_int32 (*op_get_thermal_value)(
		struct test_wlan_info *winfos,
		struct test_configuration *test_configs);
	s_int32 (*op_set_freq_offset)(
		struct test_wlan_info *winfos,
		u_int32 freq_offset, u_char band_idx);
	s_int32 (*op_set_phy_counter)(
		struct test_wlan_info *winfos,
		s_int32 control, u_char band_idx);
	s_int32 (*op_set_rxv_index)(
		struct test_wlan_info *winfos,
		u_int8 group_1, u_int8 group_2, u_char band_idx);
	s_int32 (*op_set_fagc_path)(
		struct test_wlan_info *winfos,
		u_int8 path, u_char band_idx);
	s_int32 (*op_set_fw_mode)(
		struct test_wlan_info *winfos,
		u_char fw_mode);
	s_int32 (*op_set_rf_test_mode)(
		struct test_wlan_info *winfos,
		u_int32 op_mode, u_int8 icap_len, u_int16 rsp_len);
	s_int32 (*op_set_test_mode_start)(
		struct test_wlan_info *winfos);
	s_int32 (*op_set_test_mode_abort)(
		struct test_wlan_info *winfos);
	s_int32 (*op_start_tx)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_stop_tx)(
		struct test_wlan_info *winfos,
		u_char band_idx);
	s_int32 (*op_start_rx)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_stop_rx)(
		struct test_wlan_info *winfos,
		u_char band_idx);
	s_int32 (*op_set_channel)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_set_tx_content)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_configuration *configs);
	s_int32 (*op_set_preamble)(
		struct test_wlan_info *winfos,
		u_char mode);
	s_int32 (*op_set_system_bw)(
		struct test_wlan_info *winfos,
		u_char sys_bw);
	s_int32 (*op_set_per_pkt_bw)(
		struct test_wlan_info *winfos,
		u_char per_pkt_bw);
	s_int32 (*op_reset_txrx_counter)(
		struct test_wlan_info *winfos);
	s_int32 (*op_set_rx_vector_idx)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int32 group1,
		u_int32 group2);
	s_int32 (*op_get_rx_stat_leg)(
		struct test_wlan_info *winfos,
		struct test_rx_stat_leg *rx_stat);
	s_int32 (*op_get_rxv_dump_action)(
		struct test_wlan_info *winfos,
		u_int32 action,
		u_int32 type_mask);
	s_int32 (*op_get_rxv_dump_ring_attr)(
		struct test_wlan_info *winfos,
		struct rxv_dump_ring_attr *ring_attr);
	s_int32 (*op_get_rxv_dump_rxv_content)(
		struct test_wlan_info *winfos,
		u_int8 entry_idx,
		u_int32 *content_len,
		void *rxv_content);
	s_int32 (*op_get_rxv_content_len)(
		struct test_wlan_info *winfos,
		u_int8 type_mask,
		u_int8 rxv_sta_cnt,
		u_int16 *len);
	s_int32 (*op_set_fagc_rssi_path)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int32 fagc_path);
	s_int32 (*op_get_rx_statistics_all)(
		struct test_wlan_info *winfos,
		struct hqa_comm_rx_stat *hqa_rx_stat);
	s_int32 (*op_calibration_test_mode)(
		struct test_wlan_info *winfos,
		u_char mode);
	s_int32 (*op_set_icap_start)(
		struct test_wlan_info *winfos,
		u_int8 *data);
	s_int32 (*op_get_icap_status)(
		struct test_wlan_info *winfos,
		s_int32 *icap_stat);
	s_int32 (*op_get_icap_max_data_len)(
		struct test_wlan_info *winfos,
		u_long *max_data_len);
	s_int32 (*op_get_icap_data)(
		struct test_wlan_info *winfos,
		s_int32 *icap_cnt,
		s_int32 *icap_data,
		u_int32 wf_num,
		u_int32 iq_type);
	s_int32 (*op_do_cal_item)(
		struct test_wlan_info *winfos,
		u_int32 item, u_char band_idx);
	s_int32 (*op_set_band_mode)(
		struct test_wlan_info *winfos,
		struct test_band_state *band_state);
	s_int32 (*op_get_chipid)(
		struct test_wlan_info *winfos);
	s_int32 (*op_mps_set_seq_data)(
		struct test_wlan_info *winfos,
		u_int32 len,
		struct test_mps_setting *mps_setting);
	s_int32 (*op_set_tam_arb)(
		struct test_wlan_info *winfos,
		u_int8 arb_op_mode);
	s_int32 (*op_set_mu_cnt)(
		struct test_wlan_info *winfos,
		void *virtual_device,
		u_int32 count);
	s_int32 (*op_trigger_mu_counting)(
		struct test_wlan_info *winfos,
		void *virtual_device,
		boolean enable);
	s_int32 (*op_set_muru_manual)(
		void *virtual_device,
		struct test_wlan_info *winfos,
		struct test_configuration *configs);
	/* For test phy usage */
	s_int32 (*op_get_tx_pwr)(
		struct test_wlan_info *winfos,
		struct test_configuration *configs,
		u_char band_idx,
		u_char channel,
		u_char ant_idx,
		u_int32 *power);
	s_int32 (*op_set_tx_pwr)(
		struct test_wlan_info *winfos,
		struct test_configuration *configs,
		u_char band_idx,
		struct test_txpwr_param *pwr_param);
	s_int32 (*op_write_mac_bbp_reg)(
		struct test_wlan_info *winfos,
		struct test_register *regs);
	s_int32 (*op_read_bulk_mac_bbp_reg)(
		struct test_wlan_info *winfos,
		struct test_register *regs);
	s_int32 (*op_read_bulk_rf_reg)(
		struct test_wlan_info *winfos,
		struct test_register *regs);
	s_int32 (*op_write_bulk_rf_reg)(
		struct test_wlan_info *winfos,
		struct test_register *regs);
	s_int32 (*op_read_bulk_eeprom)(
		struct test_wlan_info *winfos,
		struct test_eeprom *eprms);
	s_int32 (*op_get_freq_offset)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int32 *freq_offset);
	s_int32 (*op_get_cfg_on_off)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int32 type,
		u_int32 *result);
	s_int32 (*op_get_tx_tone_pwr)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int32 ant_idx,
		u_int32 *power);
	s_int32 (*op_get_recal_cnt)(
		struct test_wlan_info *winfos,
		u_int32 *recal_cnt,
		u_int32 *recal_dw_num);
	s_int32 (*op_get_recal_content)(
		struct test_wlan_info *winfos,
		u_int32 *content);
	s_int32 (*op_get_rdd_content)(
		struct test_log_dump_cb *log_cb,
		u_int32 *content,
		u_int32 *total_cnt);
	s_int32 (*op_get_rdd_cnt)(
		struct test_log_dump_cb *log_cb,
		u_int32 *rdd_cnt,
		u_int32 *rdd_dw_num);
	s_int32 (*op_get_rxv_cnt)(
		struct test_wlan_info *winfos,
		u_int32 *rxv_cnt,
		u_int32 *rxv_dw_num);
	s_int32 (*op_get_rxv_content)(
		struct test_wlan_info *winfos,
		u_int32 dw_cnt,
		u_int32 *content);
	s_int32 (*op_get_thermal_val)(
		struct test_wlan_info *winfos,
		struct test_configuration *configs,
		u_char band_idx,
		u_int32 *value);
	s_int32 (*op_set_cal_bypass)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int32 cal_item);
	s_int32 (*op_set_cfg_on_off)(
		struct test_wlan_info *winfos,
		u_int8 type, u_int8 enable, u_char band_idx);
	s_int32 (*op_set_dpd)(
		struct test_wlan_info *winfos,
		u_int32 on_off,
		u_int32 wf_sel);
	s_int32 (*op_set_tssi)(
		struct test_wlan_info *winfos,
		u_int32 on_off,
		u_int32 wf_sel);
	s_int32 (*op_evt_rf_test_cb)(
		struct test_wlan_info *winfos,
		struct test_log_dump_cb *test_log_dump,
		u_int32 en_log,
		u_int8 *data,
		u_int32 length);
	s_int32 (*op_set_rdd_test)(
		struct test_wlan_info *winfos,
		u_int32 rdd_idx,
		u_int32 rdd_sel,
		u_int32 enable);
	s_int32 (*op_set_off_ch_scan)(
		struct test_wlan_info *winfos,
		struct test_configuration *configs,
		u_char band_idx,
		struct test_off_ch_param *off_ch_param);
	s_int32 (*op_get_rx_stat)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		struct test_rx_stat *rx_stat);
	s_int32 (*op_hetb_ctrl)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_char ctrl_type,
		boolean enable,
		u_int8 bw,
		u_int8 ltf_gi,
		u_int8 stbc,
		u_int8 pri_ru_idx,
		struct test_ru_info *ru_info);
	s_int32 (*op_set_ru_aid)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_int16 mu_rx_aid);
	s_int32 (*op_set_mutb_spe)(
		struct test_wlan_info *winfos,
		u_char band_idx,
		u_char phy_mode,
		u_int8 spe_idx);
	s_int32 (*op_get_wf_path_comb)(
		struct test_wlan_info *winfos,
		u_int8 band_idx,
		boolean dbdc_mode_en,
		u_int8 *path,
		u_int8 *path_len);
	s_int32 (*op_get_rx_stat_band)(
		struct test_wlan_info *winfos,
		u_int8 band_idx,
		u_int8 blk_idx,
		struct test_rx_stat_band_info *rx_st_band);
	s_int32 (*op_get_rx_stat_path)(
		struct test_wlan_info *winfos,
		u_int8 band_idx,
		u_int8 blk_idx,
		struct test_rx_stat_path_info *rx_st_path);
	s_int32 (*op_get_rx_stat_user)(
		struct test_wlan_info *winfos,
		u_int8 band_idx,
		u_int8 blk_idx,
		struct test_rx_stat_user_info *rx_st_user);
	s_int32 (*op_get_rx_stat_comm)(
		struct test_wlan_info *winfos,
		u_int8 band_idx,
		u_int8 blk_idx,
		struct test_rx_stat_comm_info *rx_st_comm);
	s_int32 (*op_set_rx_user_idx)(
		struct test_wlan_info *winfos,
		u_int8 band_idx,
		u_int16 user_idx);
	/* For test mac usage */
	s_int32 (*op_backup_and_set_cr)(
		struct test_wlan_info *winfos,
		struct test_bk_cr *bks,
		u_char band_idx);
	s_int32 (*op_restore_cr)(
		struct test_wlan_info *winfos,
		struct test_bk_cr *bks,
		u_char band_idx,
		u_char option);
	s_int32 (*op_set_ampdu_ba_limit)(
		struct test_wlan_info *winfos,
		u_int8 wmm_idx,
		u_int8 agg_limit);
	s_int32 (*op_set_sta_pause_cr)(
		struct test_wlan_info *winfos);
	s_int32 (*op_set_ifs_cr)(
		struct test_wlan_info *winfos,
		struct test_configuration *configs,
		u_char band_idx);
	s_int32 (*op_set_test_mode_dnlk_clean)(
		struct test_wlan_info *winfos);
	s_int32 (*op_set_test_mode_dnlk_2g)(
		struct test_wlan_info *winfos);
	s_int32 (*op_set_test_mode_dnlk_5g)(
		struct test_wlan_info *winfos);
	s_int32 (*op_group_prek)(
		struct test_wlan_info *winfos,
		u_char mode);
	s_int32 (*op_dpd_prek)(
		struct test_wlan_info *winfos,
		u_char mode);
#ifdef TXBF_SUPPORT
	s_int32 (*op_set_ibf_phase_cal_e2p_update)(
		struct test_wlan_info *winfos,
		u_char group_idx, boolean fgSx2, u_char update_type);
	s_int32 (*op_set_ibf_phase_cal_init)(
		struct test_wlan_info *winfos);
	s_int32 (*op_set_wite_txbf_pfmu_tag)(
		struct test_wlan_info *winfos, u_char prf_idx);
	s_int32 (*op_set_txbf_pfmu_tag_invalid)(
		struct test_wlan_info *winfos, boolean fg_invalid);
	s_int32 (*op_set_txbf_pfmu_tag_idx)(
		struct test_wlan_info *winfos, u_char pfmu_idx);
	s_int32 (*op_set_txbf_pfmu_tag_bf_type)(
		struct test_wlan_info *winfos, u_char bf_type);
	s_int32 (*op_set_txbf_pfmu_tag_dbw)(
		struct test_wlan_info *winfos, u_char dbw);
	s_int32 (*op_set_txbf_pfmu_tag_sumu)(
		struct test_wlan_info *winfos, u_char su_mu);
	s_int32 (*op_set_wrap_ibf_cal_get_ibf_mem_alloc)(
		struct test_wlan_info *winfos,
		u_char *pfmu_mem_row,
		u_char *pfmu_mem_col);
	s_int32 (*op_set_wrap_ibf_cal_get_ebf_mem_alloc)(
		struct test_wlan_info *winfos,
		u_char *pfmu_mem_row,
		u_char *pfmu_mem_col);
	s_int32 (*op_set_txbf_pfmu_tag_mem)(
		struct test_wlan_info *winfos,
		u_char *pfmu_mem_row,
		u_char *pfmu_mem_col);
	s_int32 (*op_set_txbf_pfmu_tag_matrix)(
		struct test_wlan_info *winfos,
		u_char nr,
		u_char nc,
		u_char ng,
		u_char lm,
		u_char cb,
		u_char he);
	s_int32 (*op_set_txbf_pfmu_tag_snr)(
		struct test_wlan_info *winfos,
		u_char *snr_sts);
	s_int32 (*op_set_txbf_pfmu_tag_smart_ant)(
		struct test_wlan_info *winfos,
		u_int32 smart_ant);
	s_int32 (*op_set_txbf_pfmu_tag_se_idx)(
		struct test_wlan_info *winfos,
		u_char se_idx);
	s_int32 (*op_set_txbf_pfmu_tag_rmsd_thrd)(
		struct test_wlan_info *winfos,
		u_char rmsd_thrd);
	s_int32 (*op_set_txbf_pfmu_tag_time_out)(
		struct test_wlan_info *winfos,
		u_char time_out);
	s_int32 (*op_set_txbf_pfmu_tag_desired_bw)(
		struct test_wlan_info *winfos,
		u_char desired_bw);
	s_int32 (*op_set_txbf_pfmu_tag_desired_nr)(
		struct test_wlan_info *winfos,
		u_char desired_nr);
	s_int32 (*op_set_txbf_pfmu_tag_desired_nc)(
		struct test_wlan_info *winfos,
		u_char desired_nc);
	s_int32 (*op_set_txbf_pfmu_data_write)(
		struct test_wlan_info *winfos,
		u_int16 *angle_input);
	s_int32 (*op_set_manual_assoc)(
		struct test_wlan_info *winfos,
		u_char *arg);
#endif
};


/* Test tmr for service */
struct test_tmr_info {
	u_int32 setting;
	u_int32 version;
	u_int32 through_hold;
	u_int32 iter;
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
s_int32 net_ad_init_thread(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	enum service_thread_list thread_idx);
s_int32 net_ad_release_thread(
	u_char thread_idx);
s_int32 net_ad_backup_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *test_bkcr,
	u_long offset, enum test_bk_cr_type type);
s_int32 net_ad_restore_cr(
	struct test_wlan_info *winfos,
	struct test_bk_cr *test_bkcr,
	u_long offset);
s_int32 net_ad_cfg_queue(
	struct test_wlan_info *winfos, boolean enable);
s_int32 net_ad_enter_normal(
	struct test_wlan_info *winfos,
	struct test_backup_params *configs);
s_int32 net_ad_exit_normal(
	struct test_wlan_info *winfos,
	struct test_backup_params *configs);
s_int32 net_ad_update_wdev(
	u_int8 band_idx,
	struct test_wlan_info *winfos,
	struct test_configuration *configs);
s_int32 net_ad_init_wdev(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx);
s_int32 net_ad_release_wdev(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx);
s_int32 net_ad_compose_pkt(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	s_int32 sta_idx, u_char *buf,
	u_int32 txlen, u_int32 hlen);
s_int32 net_ad_alloc_pkt(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_int32 mpdu_length,
	void **pkt_skb);
s_int32 net_ad_free_pkt(
	struct test_wlan_info *winfos,
	void *pkt_skb);
s_int32 net_ad_enq_pkt(
	struct test_wlan_info *winfos,
	u_short q_idx,
	void *virtual_wtbl,
	void *virtual_device,
	void *pkt);
s_int32 net_ad_post_tx(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_int8 band_idx,
	void *pkt);
s_int32 net_ad_rx_done_handle(
	struct test_wlan_info *winfos,
	void *rx_blk);
s_int32 net_ad_set_band_mode(
	struct test_wlan_info *winfos,
	struct test_band_state *band_state);
s_int32 net_ad_set_txpwr_sku(
	struct test_wlan_info *winfos,
	u_char sku_ctrl, u_char band_idx);
s_int32 net_ad_set_txpwr_power_drop(
	struct test_wlan_info *winfos,
	u_char power_drop,
	u_char band_idx);
s_int32 net_ad_set_txpwr_percentage(
	struct test_wlan_info *winfos,
	u_char percentage_ctrl,
	u_char band_idx);
s_int32 net_ad_set_txpwr_backoff(
	struct test_wlan_info *winfos,
	u_char backoff_ctrl, u_char band_idx);
s_int32 net_ad_init_txpwr(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char band_idx);
s_int32 net_ad_handle_mcs32(
	struct test_wlan_info *winfos,
	void *virtual_device, u_int8 bw);
s_int32 net_ad_cfg_wtbl(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_tx_info *tx_info);
s_int32 net_ad_set_wmm_param_by_qid(
	u_int8 wmm_idx,
	u_int8 q_idx,
	struct test_wlan_info *winfos,
	struct test_configuration *configs);
s_int32 net_ad_clean_sta_q(
	struct test_wlan_info *winfos,
	u_char wcid);
s_int32 net_ad_set_auto_resp(
	struct test_wlan_info *winfos,
	struct test_operation *ops,
	struct test_configuration *configs,
	u_char band_idx, u_char mode);
#ifdef TXBF_SUPPORT
s_int32 net_ad_set_bss_info(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char crl_band_idx,
	u_char *pBssid);
s_int32 net_ad_set_device_info(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	u_char *addr,
	u_char band_idx,
	u_char mode);
s_int32 net_ad_set_txbf_lna_gain(
	struct test_wlan_info *winfos,
	u_char lna_gain);
s_int32 mt_ad_set_txbf_tx_apply(
	struct test_wlan_info *winfos,
	u_char *arg);
s_int32 net_ad_set_ibf_phase_comp(
	struct test_wlan_info *winfos,
	u_char bw, u_char band, u_char dbdc_band_idx, u_char group_idx,
	boolean fg_read_from_e2p, boolean fg_dis_comp);
s_int32 net_ad_set_txbf_profile_tag_read(
	struct test_wlan_info *winfos,
	u_char pf_idx, boolean fg_bfer);
s_int32 net_ad_set_txbf_profile_tag_mcs_thrd(
	struct test_wlan_info *winfos,
	u_char *mcs_lss, u_char *mcs_sss);
s_int32 net_ad_set_sta_rec_bf_update(
	struct test_wlan_info *winfos,
	u_char *arg);
s_int32 net_ad_set_sta_rec_bf_read(
	struct test_wlan_info *winfos,
	u_char *arg);
s_int32 mt_ad_set_ibf_inst_cal(
	struct test_wlan_info *winfos, u_char group_idx, u_char group_l_m_h,
	u_char fg_sx2, u_char phase_cal, u_char phase_lna_gain_level);
s_int32 mt_ad_set_txbf_profile_data_write_20m_all(
	struct test_wlan_info *winfos,
	u_char profile_idx,
	u_char *data);
#endif /* TXBF_SUPPORT */
s_int32 net_ad_set_low_power(
	struct test_wlan_info *winfos, u_int32 control);
s_int32 net_ad_read_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 net_ad_write_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 net_ad_read_bulk_mac_bbp_reg(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	struct test_register *regs);
s_int32 net_ad_read_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
s_int32 net_ad_write_bulk_rf_reg(
	struct test_wlan_info *winfos,
	struct test_register *regs);
void net_ad_read_ca53_reg(struct test_register *regs);
void net_ad_write_ca53_reg(struct test_register *regs);
s_int32 net_ad_read_write_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms,
	boolean is_read);
s_int32 net_ad_read_write_bulk_eeprom(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms,
	boolean is_read);
s_int32 net_ad_get_free_efuse_block(
	struct test_wlan_info *winfos,
	struct test_eeprom *eprms);
s_int32 net_ad_w_cali_2_efuse(
	struct test_wlan_info *winfos,
	u_int8 *data);
s_int32 net_ad_mps_tx_operation(
	struct test_wlan_info *winfos,
	struct test_configuration *configs,
	boolean is_start_tx);
s_int32 net_ad_set_tmr(
	struct test_wlan_info *winfos,
	struct test_tmr_info *tmr_info);
s_int32 net_ad_get_rxv_stat(
	struct test_wlan_info *winfos,
	u_char ctrl_band_idx,
	struct test_rx_stat *rx_stat
	);
s_int32 net_ad_get_rxv_cnt(
	struct test_wlan_info *winfos,
	u_char ctrl_band_idx,
	u_int32 *byte_cnt);
s_int32 net_ad_get_rxv_content(
	struct test_wlan_info *winfos,
	u_char ctrl_band_idx,
	void *content);

s_int32 net_ad_alloc_wtbl(
	struct test_wlan_info *winfos,
	u_char *da,
	void *virtual_dev,
	void **virtual_wtbl,
	struct test_tx_info *tx_info);
s_int32 net_ad_free_wtbl(
	struct test_wlan_info *winfos,
	u_char *da,
	void *virtual_wtbl,
	struct test_tx_info *tx_info);
s_int32 net_ad_apply_wtbl(
	struct test_wlan_info *winfos,
	void *virtual_dev,
	void *virtual_wtbl);
s_int32 net_ad_match_wtbl(
	void *virtual_wtbl,
	u_int16 wcid);
s_int32 net_ad_get_band_idx(
	void *virtual_device,
	u_char *band_idx);
s_int32 net_ad_get_wmm_idx(
	void *virtual_device,
	u_int8 *wmm_idx);
s_int32 net_ad_get_omac_idx(
	struct test_wlan_info *winfos,
	void *virtual_device,
	u_char *omac_idx);
s_int32 net_ad_fill_phy_info(
	void *virtual_wtbl,
	struct test_tx_info *tx_info);
s_int32 net_ad_fill_spe_antid(
	struct test_wlan_info *winfos,
	void *virtual_wtbl,
	u_int8 spe_idx,
	u_int8 ant_pri);
s_int32 net_ad_get_speidx(
	struct test_wlan_info *winfos,
	u_int16 ant_sel,
	u_int8 *spe_idx);
s_int32 net_ad_rf_test_cb(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *test_log_dump,
	u_int32 en_log,
	u_int8 *data,
	u_int32 length);
s_int32 net_ad_insert_test_log(
	struct test_wlan_info *winfos,
	struct test_log_dump_cb *log_cb,
	u_int8 *log,
	u_int32 log_type,
	u_int32 len);
s_int32 net_ad_insert_rdd_log(
	struct test_log_dump_entry *entry,
	u_int8 *data,
	u_int32 len);
struct service_test *net_ad_wrap_service(
	void *ad);
s_int32 net_ad_set_preamble(
	struct test_wlan_info *winfos,
	boolean preamble);

s_int32 net_ad_get_virtual_dev(
	struct test_wlan_info *winfos,
	u_int8 band_idx,
	u_int8 wmm_idx,
	void **virtual_device);
#endif /* __NET_ADAPTION_H__ */
