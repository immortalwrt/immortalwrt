
#ifndef __HIF_BASE_H__
#define __HIF_BASE_H__

enum {
	TX_RING_HIGH_TO_HIGH,
	TX_RING_HIGH_TO_LOW,
	TX_RING_LOW_TO_LOW,
	TX_RING_LOW_TO_HIGH,
	TX_RING_UNKNOW_CHANGE,
};

enum PACKET_TYPE {
	TX_DATA,
	TX_DATA_HIGH_PRIO,
	TX_MGMT,
	TX_ALTX,
	TX_CMD,
	TX_FW_DL,
	TX_DATA_PS,
	RX_PPE_VALID,
	PACKET_TYPE_NUM,
};

enum resource_attr {
	HIF_TX_DATA,
	HIF_TX_CMD,
	HIF_TX_CMD_WM, /* direct path to WMCPU, only exist for WFDMA arch with 2 CPU */
	HIF_TX_FWDL,
	HIF_RX_DATA,
	HIF_RX_EVENT,
	RING_ATTR_NUM
};

enum event_type {
	HOST_MSDU_ID_RPT = (1 << 0),
};

#endif /*__HIF_BASE_H__*/
