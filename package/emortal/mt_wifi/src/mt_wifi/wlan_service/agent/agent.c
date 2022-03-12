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
	agent.c
*/
#include "agent.h"

u_char *agnt_rstrtok;
u_char *agent_trtok(u_char *s, const u_char *ct)
{
	u_char *sbegin, *send;

	sbegin  = s ? s : agnt_rstrtok;

	if (!sbegin)
		return NULL;

	sbegin += strspn(sbegin, ct);

	if (*sbegin == '\0') {
		agnt_rstrtok = NULL;
		return NULL;
	}

	send = strpbrk(sbegin, ct);

	if (send && *send != '\0')
		*send++ = '\0';

	agnt_rstrtok = send;
	return sbegin;
}


/*****************************************************************************
 *	HQA DLL handler
 *****************************************************************************/
static s_int32 agent_cfg_find_next_token(struct agent_cfg_parse_state_s
				 *state)
{
	s_int8 *x = state->ptr;
	s_int8 *s;

	if (state->nexttoken) {
		s_int32 t = state->nexttoken;

		state->nexttoken = 0;
		return t;
	}

	for (;;) {
		switch (*x) {
		case 0:
			state->ptr = x;
			return AGENT_STATE_EOF;
		case '\n':
			x++;
			state->ptr = x;
			return AGENT_STATE_NEWLINE;
		case ' ':
		case ',':
		/*case ':':  should not including : , mac addr would be fail*/
		case '\t':
		case '\r':
			x++;
			continue;
		case '#':
			while (*x && (*x != '\n'))
				x++;
			if (*x == '\n') {
				state->ptr = x + 1;
				return AGENT_STATE_NEWLINE;
			}
			state->ptr = x;
			return AGENT_STATE_EOF;

		default:
			goto text;
		}
	}

textdone:
	state->ptr = x;
	*s = 0;
	return AGENT_STATE_TEXT;
text:
	state->text = s = x;
textresume:
	for (;;) {
		switch (*x) {
		case 0:
			goto textdone;
		case ' ':
		case ',':
		/* case ':': */
		case '\t':
		case '\r':
			x++;
			goto textdone;
		case '\n':
			state->nexttoken = AGENT_STATE_NEWLINE;
			x++;
			goto textdone;
		case '"':
			x++;
			for (;;) {
				switch (*x) {
				case 0:
					/* unterminated quoted thing */
					state->ptr = x;
					return AGENT_STATE_EOF;
				case '"':
					x++;
					goto textresume;
				default:
					*s++ = *x++;
				}
			}
			break;
		case '\\':
			x++;
			switch (*x) {
			case 0:
				goto textdone;
			case 'n':
				*s++ = '\n';
				break;
			case 'r':
				*s++ = '\r';
				break;
			case 't':
				*s++ = '\t';
				break;
			case '\\':
				*s++ = '\\';
				break;
			case '\r':
				/* \ <cr> <lf> -> line continuation */
				if (x[1] != '\n') {
					x++;
					continue;
				}
			case '\n':
				/* \ <lf> -> line continuation */
				x++;
				/* eat any extra whitespace */
				while ((*x == ' ') || (*x == '\t'))
					x++;
				continue;
			default:
				/* unknown escape -- just copy */
				*s++ = *x++;
			}
			continue;
		default:
			*s++ = *x++;
			state->textsize++;

		}
	}
	return AGENT_STATE_EOF;
}

static u_int32 agent_cfg_parse_argument(s_int8 *cmd_line,
				  s_int32 *argc, s_int8 *argv[])
{
	struct agent_cfg_parse_state_s state;
	s_int8 **args;
	s_int32 nargs;

	if (cmd_line == NULL || argc == NULL || argv == NULL) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("parameter is NULL: %p, %p, %p\n",
		cmd_line, argc, argv));

		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}
	args = argv;
	nargs = 0;
	state.ptr = cmd_line;
	state.nexttoken = 0;
	state.maxsize = 0;
	state.textsize = 0;
	state.text = 0;

	if (strnlen(cmd_line, 512) >= 512) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("cmd_line >= 512\n"));
		return SERV_STATUS_AGENT_INVALID_LEN;
	}

	for (;;) {
		switch (agent_cfg_find_next_token(&state)) {
		case AGENT_STATE_EOF:
			goto exit;
		case AGENT_STATE_NEWLINE:
			goto exit;
		case AGENT_STATE_TEXT:
			if (nargs < AGENT_CFG_ARGV_MAX)
				args[nargs++] = state.text;
			break;
		}
	}

exit:
	*argc = nargs;
	return SERV_STATUS_SUCCESS;
}


static s_int32 set_param_and_shift_buf(
	boolean convert, u_int32 size, u_char *in, u_char **out)
{
	if (!(*out)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("*buf NULL pointer with size=%u\n", size));
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (!in) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("out NULL pointer with size=%u\n", size));
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (convert) {
		if (size == sizeof(u_int32)) {
			u_int32 *tmp = (u_int32 *) in;

			*tmp = SERV_OS_HTONL(*tmp);
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_INFO,
			("%s: size=%u, val=%u\n", __func__, size, *tmp));
		} else if (size == sizeof(u_int16)) {
			u_int16 *tmp = (u_int16 *) in;

			*tmp = SERV_OS_HTONS(*tmp);
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_INFO,
			("%s: size=%u, val=%u\n", __func__, size, *tmp));
		} else {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: size %u not supported\n", __func__, size));
			return SERV_STATUS_AGENT_NOT_SUPPORTED;
		}
	}

	sys_ad_move_mem(*out, in, size);
	*out = *out + size;

	return SERV_STATUS_SUCCESS;
}

static s_int32 get_param_and_shift_buf(
	boolean convert, u_int32 size, u_char **buf, u_char *out)
{
	if (!(*buf)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("*buf NULL pointer with size=%u\n", size));
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	if (!out) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("out NULL pointer with size=%u\n", size));
		return SERV_STATUS_AGENT_INVALID_NULL_POINTER;
	}

	sys_ad_move_mem(out, *buf, size);
	*buf = *buf + size;

	if (!convert) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_INFO,
			("%s: size=%u", __func__, size));
		return SERV_STATUS_SUCCESS;
	}

	if (size == sizeof(u_int32)) {
		u_int32 *tmp = (u_int32 *) out;

		*tmp = SERV_OS_NTOHL(*tmp);
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_INFO,
		("%s: size=%u, val=%u\n", __func__, size, *tmp));
	} else if (size == sizeof(u_int16)) {
		u_int16 *tmp = (u_int16 *) out;

		*tmp = SERV_OS_NTOHS(*tmp);
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_INFO,
		("%s: size=%u, val=%u\n", __func__, size, *tmp));
	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_WARN,
		("%s: size %u not supported\n", __func__, size));
		return SERV_STATUS_AGENT_NOT_SUPPORTED;
	}

	return SERV_STATUS_SUCCESS;
}

static s_int32 update_hqa_frame(
	struct hqa_frame *hqa_frame, s_int32 length, s_int32 status)
{
	hqa_frame->length = SERV_OS_HTONS((length));
	status = SERV_OS_HTONS((status));
	sys_ad_move_mem(hqa_frame->data, &status, 2);
	return SERV_STATUS_SUCCESS;
}

static s_int32 legacy_function(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	/*
	 * Legacy function means older chips use only,
	 * but not support in following CONNAC2 projects.
	 */
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_WARN, ("%s\n", __func__));
	update_hqa_frame(hqa_frame, 2, SERV_STATUS_SUCCESS);
	return SERV_STATUS_SUCCESS;
}

static s_int32 todo_function(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_WARN, ("%s\n", __func__));
	update_hqa_frame(hqa_frame, 2, SERV_STATUS_SUCCESS);
	return SERV_STATUS_SUCCESS;
}

static s_int32 hqa_open_adapter(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_start(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_close_adapter(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_stop(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_tx_path(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS, value = 0;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 tx_ant = 0;

	if (hqa_frame->length > 2) {
		/* new format with band index,
		 * and data length extedned to 8 bytes
		 */
		/* tx path in bitwise */
		get_param_and_shift_buf(TRUE, sizeof(value),
				&data, (u_char *)&value);
		tx_ant = value;
		/* band index */
		get_param_and_shift_buf(TRUE, sizeof(value),
				&data, (u_char *)&value);
		band_idx = value;

		if ((tx_ant & TEST_ANT_USER_DEF) == 0) {
			if (band_idx && tx_ant > 0x3)
				tx_ant >>= 2;
		} else
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("%s: user_tx_path:0x%x, band:%d\n", __func__,
				tx_ant, band_idx));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: tx_path:0x%x, band:%d\n", __func__,
			tx_ant, band_idx));
	} else {
		/* legacy command format,
		 * data length is 2 bytes without band index
		 */
		get_param_and_shift_buf(TRUE, sizeof(tx_ant),
				&data, (u_char *)&tx_ant);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: tx_path:0x%x\n", __func__, tx_ant));
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, tx_ant, tx_ant, band_idx);

	ret = mt_serv_set_tx_path(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_rx_path(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS, value = 0;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int16 rx_ant = 0;

	if (hqa_frame->length > 2) {
		/* new format with band index,
		 * and data length extedned to 8 bytes
		 */
		/* rx path in bitwise */
		get_param_and_shift_buf(TRUE, sizeof(value),
				&data, (u_char *)&value);
		rx_ant = value;
		/* band index */
		get_param_and_shift_buf(TRUE, sizeof(value),
				&data, (u_char *)&value);
		band_idx = value;

		if (band_idx && rx_ant > 0x3)
			rx_ant >>= 2;

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: rx_path:%d, band:%d\n", __func__,
			rx_ant, band_idx));
	} else {
		/* legacy command format,
		 * data length is 2 bytes without band index
		 */
		get_param_and_shift_buf(TRUE, sizeof(rx_ant),
				&data, (u_char *)&rx_ant);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: rx_path:%d\n", __func__, rx_ant));
	}

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, rx_ant, rx_ant, band_idx);

	ret = mt_serv_set_rx_path(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_tx_power_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 power = 0, band_idx = 0, channel = 0;
	u_int32 ch_band = 0, ant_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(power),
				&data, (u_char *)&power);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(channel),
				&data, (u_char *)&channel);
	get_param_and_shift_buf(TRUE, sizeof(ch_band),
				&data, (u_char *)&ch_band);
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				&data, (u_char *)&ant_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ant_idx,
			(u_int32)ant_idx, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.power,
			(u_int32)power, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.channel,
			(u_int32)channel, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.band_idx,
			(u_int32)band_idx, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ch_band,
			(u_int32)ch_band, band_idx);

	ret = mt_serv_tx_power_operation(serv_test, SERV_TEST_TXPWR_SET_PWR);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: power=%u, band_idx=%u, channel=%u\n",
		__func__, power, band_idx, channel));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: ch_band=%u, ant_idx=%u\n",
		__func__, ch_band, ant_idx));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET0[] = {
	/* cmd id start from 0x1000 */
	{0x0,	hqa_open_adapter},
	{0x1,	hqa_close_adapter},
	{0x2,	legacy_function},
	{0x3,	legacy_function},
	{0x4,	legacy_function},
	{0x6,	legacy_function},
	{0x7,	legacy_function},
	{0x8,	legacy_function},
	{0xa,	legacy_function},
	{0xb,	hqa_set_tx_path},
	{0xc,	hqa_set_rx_path},
	{0xd,	legacy_function},
	{0xe,	legacy_function},
	{0xf,	legacy_function},
	{0x10,	legacy_function},
	{0x11,	hqa_set_tx_power_ext},
	{0x14,	legacy_function},
	{0x15,	todo_function},
	{0x18,	legacy_function}
};

static s_int32 hqa_set_preamble(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 tx_mode = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: Mode (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(tx_mode),
				&data, (u_char *)&tx_mode);

	/* Set parameters */
	/*
	 * 000: Legacy CCK
	 * 001: Legacy OFDM
	 * 010: HT Mixed mode
	 * 011: HT Green field mode
	 * 100: VHT mode
	 */
	CONFIG_SET_PARAM(serv_test, tx_mode, (u_char)tx_mode, band_idx);

	ret = mt_serv_set_preamble(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: phy_mode=%u\n", __func__, tx_mode));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_rate(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 mcs = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: MCS (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(mcs),
				&data, (u_char *)&mcs);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, mcs, (u_char)mcs, band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: mcs=%u\n", __func__, mcs));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_nss(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 nss = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: NSS (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(nss),
				&data, (u_char *)&nss);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, nss, (u_char)nss, band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: nss=%u\n", __func__, nss));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_system_bw(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 bw = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: BW (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(bw),
				&data, (u_char *)&bw);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, bw, (u_char)bw, band_idx);

	ret = mt_serv_set_system_bw(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: bw=%u\n", __func__, bw));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_per_pkt_bw(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 per_pkt_bw = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: Per-Pkt BW (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(per_pkt_bw),
				&data, (u_char *)&per_pkt_bw);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, per_pkt_bw, (u_char)per_pkt_bw, band_idx);

	ret = mt_serv_set_per_pkt_bw(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: per_pkt_bw=%u\n", __func__, per_pkt_bw));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_freq_offset(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 freq_offset = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: freq offset (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(freq_offset),
				&data, (u_char *)&freq_offset);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, rf_freq_offset,
			(u_int32)freq_offset, band_idx);

	ret = mt_serv_set_freq_offset(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: freq offset=%u\n", __func__, freq_offset));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_low_power(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 control = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(control),
				&data, (u_char *)&control);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: control=%u\n", __func__, control));

	/*
	 * 0: enter low power mode
	 * 1: leave low power mode
	*/
	ret = mt_serv_set_low_power(serv_test, control);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_get_antswap_capability(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 antswap_support;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR, ("%s\n", __func__));

	ret = mt_serv_get_antswap_capability(serv_test, &antswap_support);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("antswap_support = %x\n", antswap_support));

	antswap_support = SERV_OS_HTONL(antswap_support);
	sys_ad_move_mem(hqa_frame->data + 2, &antswap_support,
		sizeof(antswap_support));
	update_hqa_frame(hqa_frame, 2 + sizeof(antswap_support), ret);

	return ret;
}

static s_int32 hqa_set_antswap(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band, ant;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&band);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&ant);

	ret = mt_serv_set_antswap(serv_test, ant);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET1[] = {
	/* cmd id start from 0x1100 */
	{0x0,	legacy_function},
	{0x1,	hqa_set_preamble},
	{0x2,	hqa_set_rate},
	{0x3,	hqa_set_nss},
	{0x4,	hqa_set_system_bw},
	{0x5,	hqa_set_per_pkt_bw},
	{0x6,	legacy_function},
	{0x7,	hqa_set_freq_offset},
	{0x9,	legacy_function},
	{0xb,	hqa_low_power},
	{0xd,	hqa_get_antswap_capability},
	{0xe,	hqa_set_antswap}
};

static s_int32 hqa_reset_txrx_counter(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_reset_txrx_counter(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_cal_bypass(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 item = 0, band_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(item),
				&data, (u_char *)&item);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_set_cal_bypass(serv_test, item);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: item=%u, band_idx=%u\n",
		__func__, item, band_idx));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_rx_vector_idx(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0, group1 = 0, group2 = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(group1),
				&data, (u_char *)&group1);
	get_param_and_shift_buf(TRUE, sizeof(group2),
				&data, (u_char *)&group2);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_set_rx_vector_idx(serv_test, group1, group2);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_fagc_rssi_path(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0, fagc_path = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(fagc_path),
				&data, (u_char *)&fagc_path);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);
	CONFIG_SET_PARAM(serv_test, fagc_path, (u_char)fagc_path, band_idx);

	ret = mt_serv_set_fagc_rssi_path(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET2[] = {
	/* cmd id start from 0x1200 */
	{0x0,	hqa_reset_txrx_counter},
	{0x5,	legacy_function},
	{0x8,	hqa_cal_bypass},
	{0x9,	hqa_set_rx_vector_idx},
	{0xa,	hqa_set_fagc_rssi_path}
};

static s_int32 hqa_mac_bbp_reg_read(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_int32 cr_val;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->cr_addr);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_int32));
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_MAC_READ);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cr_addr=0x%08x, cr_val=0x%08x\n",
		__func__, test_regs->cr_addr, *test_regs->cr_val));

	cr_val = SERV_OS_HTONL(*test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &cr_val, sizeof(cr_val));
	update_hqa_frame(hqa_frame, 2 + sizeof(cr_val), ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_mac_bbp_reg_write(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_int32 cr_val;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->cr_addr);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&cr_val);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_int32));
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}
	sys_ad_move_mem(test_regs->cr_val, &cr_val, sizeof(cr_val));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_MAC_WRITE);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cr_addr=0x%08x, cr_val=0x%08x\n",
		__func__, test_regs->cr_addr, *test_regs->cr_val));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_mac_bbp_reg_bulk_read(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 cr_seq = 0;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_int32 cr_total_len, cr_value = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(test_regs->cr_addr),
				&data, (u_char *)&test_regs->cr_addr);
	get_param_and_shift_buf(TRUE, sizeof(test_regs->cr_num),
				&data, (u_char *)&test_regs->cr_num);

	/* Allocate cr_val memory */
	cr_total_len = test_regs->cr_num << 2;
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, cr_total_len);
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	if (test_regs->cr_num > 371) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: requested len is longer, make it smaller\n",
			__func__));
		ret = SERV_STATUS_AGENT_INVALID_LEN;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_REG_MAC_READ_BULK);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cr_addr=0x%08x, cr_num(unit: 4bytes)=%d\n",
		__func__, test_regs->cr_addr, test_regs->cr_num));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cr_val = 0x%08x\n", __func__, *test_regs->cr_val));

	for (cr_seq = 0; cr_seq < test_regs->cr_num; cr_seq++) {
		cr_value = test_regs->cr_val[cr_seq];
		test_regs->cr_val[cr_seq] = SERV_OS_HTONL(cr_value);
	}

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, test_regs->cr_val, cr_total_len);
	update_hqa_frame(hqa_frame, 2 + cr_total_len, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_rf_reg_bulk_read(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int16 cr_seq = 0;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_int32 cr_total_len, cr_value = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->wf_sel);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->cr_addr);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->cr_num);

	/* Allocate cr_val memory */
	cr_total_len = test_regs->cr_num << 2;
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, cr_total_len);
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_REG_RF_READ_BULK);

	for (cr_seq = 0; cr_seq < test_regs->cr_num; cr_seq++) {
		cr_value = test_regs->cr_val[cr_seq];
		test_regs->cr_val[cr_seq] = SERV_OS_HTONL(cr_value);
	}
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, test_regs->cr_val, cr_total_len);
	update_hqa_frame(hqa_frame, 2 + cr_total_len, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_rf_reg_bulk_write(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_int32 idx, cr_total_len;
	u_int32 *dst;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->wf_sel);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->cr_addr);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&test_regs->cr_num);

	/* Allocate cr_val memory */
	cr_total_len = test_regs->cr_num << 2;
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, cr_total_len);
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	for (idx = 0; idx < test_regs->cr_num; idx++) {
		dst = test_regs->cr_val + idx;
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *) dst);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: cr_addr=0x%08x, cr_val=0x%08x\n",
			__func__, test_regs->cr_addr, *dst));
	}

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_REG_RF_WRITE_BULK);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_read_eeprom(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_char *data = hqa_frame->data;
	u_int16 value;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int16),
				&data, (u_char *)&test_eprms->offset);

	/* Allocate value memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value, sizeof(u_int16));
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate eeprom memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_EEPROM_READ);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: offset=0x%04x, value=0x%04x\n",
		__func__, test_eprms->offset, *test_eprms->value));

	value = SERV_OS_HTONS(*test_eprms->value);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &value, sizeof(value));
	update_hqa_frame(hqa_frame, 2 + sizeof(value), ret);

	/* Free value memory */
	sys_ad_free_mem(test_eprms->value);

	return ret;
}

static s_int32 hqa_write_eeprom(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_char *data = hqa_frame->data;
	u_int16 value;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int16),
				&data, (u_char *)&test_eprms->offset);
	get_param_and_shift_buf(TRUE, sizeof(u_int16),
				&data, (u_char *)&value);

	/* Allocate value memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value, sizeof(u_int16));
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate eeprom memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}
	sys_ad_move_mem(test_eprms->value, &value, sizeof(value));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_EEPROM_WRITE);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: offset=0x%04x, value=0x%04x\n",
		__func__, test_eprms->offset, *test_eprms->value));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	/* Free value memory */
	sys_ad_free_mem(test_eprms->value);

	return ret;
}

static void memcpy_eeprom(u_char *dst, u_char *src, u_long len)
{
	u_long i;
	u_short *p_dst, *p_src;

	p_dst = (u_short *) dst;
	p_src = (u_short *) src;

	for (i = 0; i < (len >> 1); i++) {
		*p_dst = SERV_OS_NTOHS(*p_src);
		p_dst++;
		p_src++;
	}

	if ((len % 2) != 0) {
		sys_ad_move_mem(p_dst, p_src, (len % 2));
		*p_dst = SERV_OS_NTOHS(*p_dst);
	}
}

static s_int32 hqa_read_bulk_eeprom(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int32 eeprom_size;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	eeprom_size = serv_test->test_winfo->chip_cap.efuse_size;
	if (test_eprms->offset + test_eprms->length <= eeprom_size) {
		/* Allocate value memory */
		ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value,
				       test_eprms->length);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				 ("%s: allocate eeprom memory fail\n",
				  __func__));
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

			/* Update hqa_frame with response: status (2 bytes) */
			update_hqa_frame(hqa_frame, 2, ret);

			return ret;
		}

		ret = mt_serv_reg_eprm_operation(serv_test,
						SERV_TEST_EEPROM_READ_BULK);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: offset=0x%04x, length=%d\n",
			 __func__, test_eprms->offset, test_eprms->length));

		/* Update hqa_frame with response: status (2 bytes) */
		memcpy_eeprom(hqa_frame->data + 2,
			      (u_char *)test_eprms->value, test_eprms->length);
	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: exceed eeprom size (offset=0x%04x, size=%d)\n",
			__func__, test_eprms->offset+test_eprms->length,
			eeprom_size));
		test_eprms->length = 0;
		ret = SERV_STATUS_AGENT_INVALID_LEN;
	}
	update_hqa_frame(hqa_frame, 2 + test_eprms->length, ret);

	/* Free value memory */
	sys_ad_free_mem(test_eprms->value);

	return ret;
}

static s_int32 hqa_read_bulk_eeprom_v1(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_char *data = hqa_frame->data;
	u_short w_offset = 0, w_length = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_short),
				&data, (u_char *)&w_offset);
	get_param_and_shift_buf(TRUE, sizeof(u_short),
				&data, (u_char *)&w_length);
	test_eprms->offset = w_offset;
	test_eprms->length = w_length;

	return hqa_read_bulk_eeprom(serv_test, hqa_frame);
}

static s_int32 hqa_read_bulk_eeprom_v2(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_char *data = hqa_frame->data;
	u_int dw_offset = 0, dw_length = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int),
				&data, (u_char *)&dw_offset);
	get_param_and_shift_buf(TRUE, sizeof(u_int),
				&data, (u_char *)&dw_length);
	test_eprms->offset = dw_offset;
	test_eprms->length = dw_length;

	return hqa_read_bulk_eeprom(serv_test, hqa_frame);
}

static s_int32 hqa_write_bulk_eeprom(
	struct service_test *serv_test, u_char *data)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_int32 eeprom_size;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s offset: %u, legnth: %u\n", __func__,
			test_eprms->offset, test_eprms->length));

	eeprom_size = serv_test->test_winfo->chip_cap.efuse_size;
	if (test_eprms->offset + test_eprms->length <= eeprom_size) {
		/* Allocate value memory */
		ret = sys_ad_alloc_mem((pu_char *)&test_eprms->value,
				       test_eprms->length);
		if (ret)
			goto err_out;

		memcpy_eeprom((u_char *)test_eprms->value,
			      data, test_eprms->length);

		ret = mt_serv_reg_eprm_operation(serv_test,
						SERV_TEST_EEPROM_WRITE_BULK);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: offset=0x%04x, length=%d\n", __func__,
			 test_eprms->offset, test_eprms->length));
	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: exceed eeprom size (offset=0x%04x, size=%d)\n",
			__func__, test_eprms->offset+test_eprms->length,
			eeprom_size));
		test_eprms->length = 0;
		ret = SERV_STATUS_AGENT_INVALID_LEN;
	}

err_out:
	if (test_eprms->value) {
		/* Free value memory */
		sys_ad_free_mem(test_eprms->value);
	}

	return ret;
}

static s_int32 hqa_write_bulk_eeprom_v1(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_char *data = hqa_frame->data;
	u_short w_offset = 0, w_length = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_short),
				&data, (u_char *)&w_offset);
	get_param_and_shift_buf(TRUE, sizeof(u_short),
				&data, (u_char *)&w_length);
	test_eprms->offset = w_offset;
	test_eprms->length = w_length;

	ret = hqa_write_bulk_eeprom(serv_test,
				    &hqa_frame->data[sizeof(u_short)*2]);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_write_bulk_eeprom_v2(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_eeprom *test_eprms = &serv_test->test_eprm;
	u_char *data = hqa_frame->data;
	u_int dw_offset = 0, dw_length = 0;

	/* offset is 4-byte for v2 */
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(u_int),
				&data, (u_char *)&dw_offset);
	get_param_and_shift_buf(TRUE, sizeof(u_int),
				&data, (u_char *)&dw_length);
	test_eprms->offset = dw_offset;
	test_eprms->length = dw_length;

	ret = hqa_write_bulk_eeprom(serv_test,
				    &hqa_frame->data[sizeof(u_int)*2]);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_check_efuse_mode(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 efuse_mode;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	if (WINFO_GET_PARAM(serv_test, use_efuse))
		efuse_mode = 1;
	else
		efuse_mode = 0;

	efuse_mode = SERV_OS_HTONL(efuse_mode);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: efuse_mode=%u\n", __func__, efuse_mode));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &efuse_mode, sizeof(efuse_mode));
	update_hqa_frame(hqa_frame, 6, ret);

	return ret;
}

static s_int32 hqa_get_free_efuse_block(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 free_block;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_reg_eprm_operation(serv_test,
					SERV_TEST_EEPROM_GET_FREE_EFUSE_BLOCK);

	free_block = EEPROM_GET_PARAM(serv_test, efuse_free_block);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: efuse_free_block=%u\n", __func__, free_block));

	free_block = SERV_OS_HTONL(free_block);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &free_block, sizeof(free_block));
	update_hqa_frame(hqa_frame, 2 + sizeof(free_block), ret);

	return ret;
}

static s_int32 hqa_get_tx_power(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 power = 0, band_idx = 0, channel = 0;
	u_int32 ch_band = 0, ant_idx = 0, efuse_offset = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(channel),
				&data, (u_char *)&channel);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(ch_band),
				&data, (u_char *)&ch_band);
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				&data, (u_char *)&ant_idx);

	/* set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ant_idx,
			(u_int32)ant_idx, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.channel,
			(u_int32)channel, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.band_idx,
			(u_int32)band_idx, band_idx);
	CONFIG_SET_PARAM(serv_test, pwr_param.ch_band,
			(u_int32)ch_band, band_idx);

	ret = mt_serv_tx_power_operation(serv_test, SERV_TEST_TXPWR_GET_PWR);

	power = CONFIG_GET_PARAM(serv_test, pwr_param.power, band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: power=%u, band_idx=%u, channel=%u\n",
		__func__, power, band_idx, channel));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: ch_band=%u, ant_idx=%u\n",
		__func__, ch_band, ant_idx));

	/* update hqa_frame with response: status (2 bytes) */
	efuse_offset = SERV_OS_HTONL(efuse_offset);
	sys_ad_move_mem(hqa_frame->data + 2, &efuse_offset,
			sizeof(efuse_offset));
	power = SERV_OS_HTONL(power);
	sys_ad_move_mem(hqa_frame->data + 2 + 4, &power, sizeof(power));
	update_hqa_frame(hqa_frame,
		2 + sizeof(power) + sizeof(efuse_offset), ret);

	return ret;
}

static s_int32 hqa_set_cfg_on_off(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 type, enable, band_idx;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(type),
				&data, (u_char *)&type);
	get_param_and_shift_buf(TRUE, sizeof(enable),
				&data, (u_char *)&enable);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);
	CONFIG_SET_PARAM(serv_test, log_type, (u_char)type, band_idx);
	CONFIG_SET_PARAM(serv_test, log_enable, (u_char)enable, band_idx);

	ret = mt_serv_set_cfg_on_off(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: type=%u, enable=%u, band_idx=%u\n",
		__func__, type, enable, band_idx));

	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_get_freq_offset(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 freq_offset = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_get_freq_offset(serv_test, &freq_offset);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: freq offset: %d\n", __func__, freq_offset));

	/* update hqa_frame with response: status (2 bytes) */
	freq_offset = SERV_OS_HTONL(freq_offset);
	sys_ad_move_mem(hqa_frame->data + 2, &freq_offset, sizeof(freq_offset));
	update_hqa_frame(hqa_frame, 2 + sizeof(freq_offset), ret);

	return ret;
}

static s_int32 hqa_dbdc_tx_tone(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_tx_tone param;
	u_char *data = hqa_frame->data;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				&data, (u_char *)&param.band_idx);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_tone_en),
				&data, (u_char *)&param.tx_tone_en);
	get_param_and_shift_buf(TRUE, sizeof(param.ant_idx),
				&data, (u_char *)&param.ant_idx);
	get_param_and_shift_buf(TRUE, sizeof(param.tone_type),
				&data, (u_char *)&param.tone_type);
	get_param_and_shift_buf(TRUE, sizeof(param.tone_freq),
				&data, (u_char *)&param.tone_freq);
	get_param_and_shift_buf(TRUE, sizeof(param.dc_offset_I),
				&data, (u_char *)&param.dc_offset_I);
	get_param_and_shift_buf(TRUE, sizeof(param.dc_offset_Q),
				&data, (u_char *)&param.dc_offset_Q);
	get_param_and_shift_buf(TRUE, sizeof(param.band),
				&data, (u_char *)&param.band);
	get_param_and_shift_buf(TRUE, sizeof(param.rf_pwr),
				&data, (u_char *)&param.rf_pwr);
	get_param_and_shift_buf(TRUE, sizeof(param.digi_pwr),
				&data, (u_char *)&param.digi_pwr);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_tone_en,
			(u_int32)param.tx_tone_en, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ant_idx,
			(u_int32)param.ant_idx, param.band_idx);
	CONFIG_SET_PARAM(serv_test, tone_type,
			(u_int32)param.tone_type, param.band_idx);
	CONFIG_SET_PARAM(serv_test, tone_freq,
			(u_int32)param.tone_freq, param.band_idx);
	CONFIG_SET_PARAM(serv_test, dc_offset_I,
			(u_int32)param.dc_offset_I, param.band_idx);
	CONFIG_SET_PARAM(serv_test, dc_offset_Q,
			(u_int32)param.dc_offset_Q, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ch_band,
			(u_char)param.band, param.band_idx);
	CONFIG_SET_PARAM(serv_test, rf_pwr,
			(u_int32)param.rf_pwr, param.band_idx);
	CONFIG_SET_PARAM(serv_test, digi_pwr,
			(u_int32)param.digi_pwr, param.band_idx);

	ret = mt_serv_dbdc_tx_tone(serv_test);
	ret = mt_serv_dbdc_tx_tone_pwr(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u, tx_tone_en=%u, ant_idx=0x%x\n",
		__func__, param.band_idx, param.tx_tone_en, param.ant_idx));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: tone_type=%u, tone_freq=%u, dc_offset_I=0x%x\n",
		__func__, param.tone_type, param.tone_freq,
		param.dc_offset_I));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: dc_offset_Q=%u, band=%u, rf_pwr=0x%x, digi_pwr=0x%x\n",
		__func__, param.dc_offset_Q, param.band, param.rf_pwr,
		param.digi_pwr));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_dbdc_continuous_tx(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_continuous_tx param;
	u_char *data = hqa_frame->data;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				&data, (u_char *) &param.band_idx);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_tone_en),
				&data, (u_char *) &param.tx_tone_en);
	get_param_and_shift_buf(TRUE, sizeof(param.ant_mask),
				&data, (u_char *) &param.ant_mask);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_mode),
				&data, (u_char *) &param.tx_mode);
	get_param_and_shift_buf(TRUE, sizeof(param.bw),
				&data, (u_char *) &param.bw);
	get_param_and_shift_buf(TRUE, sizeof(param.pri_ch),
				&data, (u_char *) &param.pri_ch);
	get_param_and_shift_buf(TRUE, sizeof(param.rate),
				&data, (u_char *) &param.rate);
	get_param_and_shift_buf(TRUE, sizeof(param.central_ch),
				&data, (u_char *) &param.central_ch);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_fd_mode),
				&data, (u_char *) &param.tx_fd_mode);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_tone_en,
			(u_int32)param.tx_tone_en, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ant_mask,
			(u_int32)param.ant_mask, param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_mode,
			(u_char)param.tx_mode, param.band_idx);
	CONFIG_SET_PARAM(serv_test, bw,
			(u_char)param.bw, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ctrl_ch,
			(u_char)param.pri_ch, param.band_idx);
	CONFIG_SET_PARAM(serv_test, rate,
			(u_int32)param.rate, param.band_idx);
	CONFIG_SET_PARAM(serv_test, channel,
			(u_char)param.central_ch, param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_fd_mode,
			(u_int32)param.tx_fd_mode, param.band_idx);

	ret = mt_serv_dbdc_continuous_tx(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: param band_idx=%u, tx_tone_en=%u, ant_mask=0x%x\n",
		__func__, param.band_idx, param.tx_tone_en, param.ant_mask));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: param phy_mode=%u, bw=%u, pri_ch=%u\n",
		__func__, param.tx_mode, param.bw, param.pri_ch));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: param rate=%u, central_ch=%u, tx_fd_mode=%u\n",
		__func__, param.rate, param.central_ch, param.tx_fd_mode));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_rx_filter_pkt_len(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0, enable = 0, rx_pkt_len = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(enable),
				&data, (u_char *)&enable);
	get_param_and_shift_buf(TRUE, sizeof(rx_pkt_len),
				&data, (u_char *)&rx_pkt_len);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);
	CONFIG_SET_PARAM(serv_test, rx_filter_en,
			(u_int8)enable, band_idx);
	CONFIG_SET_PARAM(serv_test, rx_filter_pkt_len,
			(u_int32)rx_pkt_len, band_idx);

	ret = mt_serv_set_rx_filter_pkt_len(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_get_tx_info(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 tx_cnt0 = 0, tx_cnt1 = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_get_tx_info(serv_test);

	/* Get parameters */
	tx_cnt0 = CONFIG_GET_PARAM(serv_test, tx_stat.tx_done_cnt,
				TEST_DBDC_BAND0);
	tx_cnt0 = SERV_OS_HTONL(tx_cnt0);
	if (IS_TEST_DBDC(serv_test->test_winfo)) {
		tx_cnt1 = CONFIG_GET_PARAM(serv_test, tx_stat.tx_done_cnt,
					TEST_DBDC_BAND1);
		tx_cnt1 = SERV_OS_HTONL(tx_cnt1);
	}

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem((hqa_frame->data + 2),
			&tx_cnt0, sizeof(tx_cnt0));
	sys_ad_move_mem((hqa_frame->data + 2 + sizeof(tx_cnt0)),
			&tx_cnt1, sizeof(tx_cnt1));
	update_hqa_frame(hqa_frame,
			2 + sizeof(tx_cnt0) + sizeof(tx_cnt1), ret);

	return ret;
}

static s_int32 hqa_get_cfg_on_off(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	s_int32 type = 0, band_idx = 0;
	u_int32 result = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(type),
				&data, (u_char *)&type);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	ret = mt_serv_get_cfg_on_off(serv_test, type, &result);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: type=%u, result=%u\n", __func__, type, result));

	/* Update hqa_frame with response: status (2 bytes) */
	result = SERV_OS_HTONL(result);
	sys_ad_move_mem(hqa_frame->data + 2, &result, sizeof(result));
	update_hqa_frame(hqa_frame, 2 + sizeof(result), ret);

	return ret;
}

static s_int32 hqa_ca53_reg_read(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_long cr_val;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Use u_long here to get 4bytes firmly */
	get_param_and_shift_buf(TRUE, sizeof(u_long),
				&data, (u_char *)&test_regs->cr_addr);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_long));
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_CA53_READ);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cr_addr=0x%08x, cr_val=0x%08x\n",
		__func__, test_regs->cr_addr, *test_regs->cr_val));

	cr_val = SERV_OS_HTONL(*test_regs->cr_val);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &cr_val, sizeof(cr_val));
	update_hqa_frame(hqa_frame, 2 + sizeof(cr_val), ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_ca53_reg_write(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_register *test_regs = &serv_test->test_reg;
	u_char *data = hqa_frame->data;
	u_int32 cr_val;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Use u_long here to get 4bytes firmly */
	get_param_and_shift_buf(TRUE, sizeof(u_long),
				&data, (u_char *)&test_regs->cr_addr);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&cr_val);

	/* Allocate cr_val memory */
	ret = sys_ad_alloc_mem((pu_char *)&test_regs->cr_val, sizeof(u_int32));
	if (ret) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: allocate register memory fail\n", __func__));
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;

		/* Update hqa_frame with response: status (2 bytes) */
		update_hqa_frame(hqa_frame, 2, ret);

		return ret;
	}
	sys_ad_move_mem(test_regs->cr_val, &cr_val, sizeof(cr_val));

	ret = mt_serv_reg_eprm_operation(serv_test, SERV_TEST_REG_CA53_WRITE);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cr_addr=0x%08x, cr_val=0x%08x\n",
		__func__, test_regs->cr_addr, *test_regs->cr_val));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	/* Free cr_val memory */
	sys_ad_free_mem(test_regs->cr_val);

	return ret;
}

static s_int32 hqa_get_tx_tone_pwr(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 power = 0, ant_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* request format type */
	get_param_and_shift_buf(TRUE, sizeof(ant_idx),
				&data, (u_char *)&ant_idx);

	ret = mt_serv_get_tx_tone_pwr(serv_test, ant_idx, &power);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: ant_idx: %d, power: %d\n",
		__func__, ant_idx, power));

	/* update hqa_frame with response: status (2 bytes) */
	power = SERV_OS_HTONL(power);
	sys_ad_move_mem(hqa_frame->data + 2, &power, sizeof(power));
	update_hqa_frame(hqa_frame, 2 + sizeof(power), ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET3[] = {
	/* cmd id start from 0x1300 */
	{0x0,	hqa_mac_bbp_reg_read},
	{0x1,	hqa_mac_bbp_reg_write},
	{0x2,	hqa_mac_bbp_reg_bulk_read},
	{0x3,	hqa_rf_reg_bulk_read},
	{0x4,	hqa_rf_reg_bulk_write},
	{0x5,	hqa_read_eeprom},
	{0x6,	hqa_write_eeprom},
	{0x7,	hqa_read_bulk_eeprom_v1},
	{0x8,	hqa_write_bulk_eeprom_v1},
	{0x9,	hqa_check_efuse_mode},
	{0xa,	hqa_get_free_efuse_block},
	{0xd,	hqa_get_tx_power},
	{0xe,	hqa_set_cfg_on_off},
	{0xf,	hqa_get_freq_offset},
	{0x10,	hqa_dbdc_tx_tone},
	{0x11,	hqa_dbdc_continuous_tx},
	{0x12,	hqa_set_rx_filter_pkt_len},
	{0x13,	hqa_get_tx_info},
	{0x14,	hqa_get_cfg_on_off},
	{0x15,	legacy_function},
	{0x17,	legacy_function},
	{0x18,	hqa_ca53_reg_read},
	{0x19,	hqa_ca53_reg_write},
	{0x1a,	hqa_get_tx_tone_pwr},
	{0x1b,	hqa_read_bulk_eeprom_v2},
	{0x1c,	hqa_write_bulk_eeprom_v2}
};

static s_int32 hqa_get_thermal_val(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 value = 0;
	u_char band_idx;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* request format type */
	band_idx = serv_test->ctrl_band_idx;

	ret = mt_serv_get_thermal_val(serv_test, band_idx, &value);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: value: %d\n", __func__, value));

	/* update hqa_frame with response: status (2 bytes) */
	value = SERV_OS_HTONL(value);
	sys_ad_move_mem(hqa_frame->data + 2, &value, sizeof(value));
	update_hqa_frame(hqa_frame, 2 + sizeof(value), ret);

	return ret;
}

static struct hqa_cmd_entry CMD_SET4[] = {
	/* cmd id start from 0x1400 */
	{0x1,	hqa_get_thermal_val}
};

static s_int32 hqa_get_fw_info(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct serv_fw_info *fw_info = NULL;
	u_char op_mode;
	u_int8 loop, month = 0;
	u_char date[8], time[6];
	u_char *kernel_info = NULL;
	u_char *month_array[12] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	fw_info = WINFO_GET_PADDR(serv_test, wm_fw_info);
	sys_ad_zero_mem(&date[0], sizeof(date));
	sys_ad_zero_mem(&time[0], sizeof(time));

	/* Get information from kernel */
	for (loop = 0; loop < 12; loop++) {
		kernel_info = strstr(utsname()->version, month_array[loop]);

		if (kernel_info)
			break;
	}

	/* SW package build time */
	sys_ad_move_mem(&time[0], kernel_info + 7, 2);
	sys_ad_move_mem(&time[2], kernel_info + 10, 2);
	sys_ad_move_mem(&time[4], kernel_info + 13, 2);

	/* SW package build date */
	sys_ad_move_mem(&date[0], kernel_info + 20, 4);
	sys_ad_move_mem(&date[6], kernel_info + 4, 2);

	for (loop = 0; loop < 12; loop++) {
		if (sys_ad_cmp_mem(month_array[loop], kernel_info, 3) == 0) {
			month = loop + 1;
			break;
		}
	}

	date[4] = month / 10 % 10 + '0';
	date[5] = month % 10 + '0';

	/* Update hqa_frame with response: status (2 bytes) */
	/* The 1 byte for op_mode doesn't use anymore but still keep it */
	sys_ad_move_mem((hqa_frame->data + 2), &op_mode, sizeof(op_mode));
	sys_ad_move_mem((hqa_frame->data + 2 + sizeof(op_mode)),
			&date, sizeof(date));
	sys_ad_move_mem((hqa_frame->data + 2 + sizeof(op_mode) + sizeof(date)),
			&time, sizeof(time));
	sys_ad_move_mem((hqa_frame->data + 2 + sizeof(op_mode)
			+ sizeof(date) + sizeof(time)),
			&fw_info->ram_built_date[0],
			sizeof(fw_info->ram_built_date));
	update_hqa_frame(hqa_frame, (2+1+8+6+15), ret);

	return ret;
}

static s_int32 hqa_set_stbc(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 stbc = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: Enable (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(stbc),
				&data, (u_char *)&stbc);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, stbc, (u_char)stbc, band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: stbc enable=%u\n", __func__, stbc));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_short_gi(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int32 sgi = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type: Enable (4 bytes) */
	get_param_and_shift_buf(TRUE, sizeof(sgi),
				&data, (u_char *)&sgi);

	/* Set parameters */
	CONFIG_SET_PARAM(serv_test, sgi, (u_char)sgi, band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: sgi enable=%u\n", __func__, sgi));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_dpd(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 on_off = 0, wf_sel = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(on_off),
				&data, (u_char *)&on_off);
	get_param_and_shift_buf(TRUE, sizeof(wf_sel),
				&data, (u_char *)&wf_sel);

	ret = mt_serv_set_dpd(serv_test, on_off, wf_sel);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: on_off: %d, wf_sel: %d\n", __func__, on_off, wf_sel));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_set_tssi_onoff(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 on_off = 0, wf_sel = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(on_off),
				&data, (u_char *)&on_off);
	get_param_and_shift_buf(TRUE, sizeof(wf_sel),
				&data, (u_char *)&wf_sel);

	ret = mt_serv_set_tssi(serv_test, on_off, wf_sel);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: on_off: %d\n", __func__, on_off));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

#if 0	/* legacy function RX statistic report per band/mac/phy */
static s_int32 hqa_get_rx_statistics_leg(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_rx_stat_leg rx_stat;
	struct test_rx_stat_leg test_rx_stat;
	u_char dw_cnt = 0, dw_idx = 0;
	u_char *ptr2 = NULL;
	u_int32 *ptr = NULL;
	u_int32 buf;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_get_rx_stat_leg(serv_test, &test_rx_stat);
	sys_ad_move_mem(&rx_stat, &test_rx_stat,
			sizeof(struct hqa_rx_stat_leg));
	dw_cnt = sizeof(struct hqa_rx_stat_leg) >> 2;

	for (dw_idx = 0, ptr = (u_int32 *)&rx_stat, ptr2 = hqa_frame->data + 2;
			dw_idx < dw_cnt; dw_idx++, ptr++, ptr2 += 4) {
		buf = SERV_OS_HTONL(*ptr);
		sys_ad_move_mem(ptr2, &buf, sizeof(u_int32));
	}

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2 + sizeof(struct hqa_rx_stat_leg), ret);

	return ret;
}
#endif

static s_int32 hqa_get_rx_statistics_all(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	u_int32 buf_size = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 type_mask = 0, band_idx = 0, type_num = 0, length;
	u_int32 blk_idx = 0, type_idx = 0, buf = 0;
	u_int32 dw_idx = 0, dw_cnt = 0;
	u_int32 *ptr2 = NULL;
	struct test_rx_stat_u *rx_stat = NULL;
	boolean dbdc_mode = FALSE;
	u_int8 path[TEST_ANT_NUM] = {0};
	u_int8 path_len = 0;
	u_int8 *ptr = NULL;
	u_char *data = hqa_frame->data;
	struct hqa_rx_stat_resp_format st_form[SERV_RX_STAT_TYPE_NUM] = {
	 {SERV_RX_STAT_TYPE_BAND, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_band_info)},
	 {SERV_RX_STAT_TYPE_PATH, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_path_info)},
	 {SERV_RX_STAT_TYPE_USER, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_user_info)},
	 {SERV_RX_STAT_TYPE_COMM, 0, 0, 0,
		 sizeof(struct hqa_rx_stat_comm_info)}
	};

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Request format type */
	get_param_and_shift_buf(TRUE, sizeof(type_mask),
				&data, (u_char *)&type_mask);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* check dbdc mode condition */
	dbdc_mode = IS_TEST_DBDC(serv_test->test_winfo);

	/* sanity check for band index param */
	if ((!dbdc_mode) && (band_idx != TEST_DBDC_BAND0))
		goto error2;

	/* check wifi path combination for specific band */
	ret = mt_serv_get_wf_path_comb(serv_test,
			band_idx,
			dbdc_mode,
			path,
			&path_len);

	/* TODO: check user combination for specific band  */
	/* host driver not support related utility */
	/* if need this function, need host driver support */

	/* update item mask for each type */
	st_form[SERV_RX_STAT_TYPE_BAND].item_mask = BIT(band_idx);
	for (blk_idx = 0; blk_idx < path_len; blk_idx++)
		st_form[SERV_RX_STAT_TYPE_PATH].item_mask |= BIT(path[blk_idx]);
	for (blk_idx = 0; blk_idx < TEST_USER_NUM; blk_idx++)
		st_form[SERV_RX_STAT_TYPE_USER].item_mask |= BIT(blk_idx);
	st_form[SERV_RX_STAT_TYPE_COMM].item_mask = BIT(0);

	/* update block count for each type */
	for (type_idx = SERV_RX_STAT_TYPE_BAND;
		type_idx < SERV_RX_STAT_TYPE_NUM; type_idx++) {
		for (blk_idx = 0; blk_idx < 32; blk_idx++) {
			if (st_form[type_idx].item_mask & BIT(blk_idx))
				st_form[type_idx].blk_cnt++;
		}
	}

	ptr = hqa_frame->data + 2 + sizeof(type_num);

	/* allocate dynamic memory for rx stat info */
	ret = sys_ad_alloc_mem((pu_char *)&rx_stat,
			sizeof(struct test_rx_stat_u));
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	for (type_idx = SERV_RX_STAT_TYPE_BAND;
			type_idx < SERV_RX_STAT_TYPE_NUM; type_idx++) {
		if (type_mask & BIT(type_idx)) {
			type_num++;
			length = st_form[type_idx].blk_cnt *
				st_form[type_idx].blk_size;

			/* fill in type */
			buf = SERV_OS_HTONL(st_form[type_idx].type);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(st_form[type_idx].type);
			buf_size += sizeof(st_form[type_idx].type);

			/* fill in version */
			buf = SERV_OS_HTONL(st_form[type_idx].version);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(st_form[type_idx].version);
			buf_size += sizeof(st_form[type_idx].version);

			/* fill in item mask */
			buf = SERV_OS_HTONL(st_form[type_idx].item_mask);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(st_form[type_idx].item_mask);
			buf_size += sizeof(st_form[type_idx].item_mask);

			/* fill in length */
			buf = SERV_OS_HTONL(length);
			sys_ad_move_mem(ptr, &buf, sizeof(buf));
			ptr += sizeof(length);
			buf_size += sizeof(length);

			for (blk_idx = 0; blk_idx < 32; blk_idx++) {
				if (st_form[type_idx].item_mask
						& BIT(blk_idx)) {
					/* service handle for rx stat info */
					mt_serv_get_rx_stat(serv_test,
						band_idx,
						blk_idx,
						type_idx,
						rx_stat);

					ptr2 = (u_int32 *) rx_stat;
					dw_cnt = st_form[type_idx].blk_size
						>> 2;
					for (dw_idx = 0; dw_idx < dw_cnt;
							dw_idx++, ptr2++,
							ptr += 4) {
						/* endian transform */
						buf = SERV_OS_HTONL(*ptr2);
						/* fill in block content */
						sys_ad_move_mem(ptr, &buf,
								sizeof(buf));
					}

					buf_size += st_form[type_idx].blk_size;
				}
			}
		}
	}

	/* free allocated memory */
	sys_ad_free_mem(rx_stat);

	/* fill in type num */
	ptr = hqa_frame->data + 2;
	buf = SERV_OS_HTONL(type_num);
	sys_ad_move_mem(ptr, &buf, sizeof(buf));
	buf_size += sizeof(type_num);
	update_hqa_frame(hqa_frame, (2 + buf_size), ret);

	return ret;

error1:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: memory allocation fail for rx stat.\n",
		__func__));
	update_hqa_frame(hqa_frame, 2, ret);
	return ret;

error2:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: invalid band index for non-dbdc mode.\n",
		__func__));
	update_hqa_frame(hqa_frame, 2, ret);
	return ret;
}

static s_int32 hqa_get_rxv_dump(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 version = 0, action = 0;
	u_int32 rxv_cnt = 0, rxv_content_len = 0, type_mask = 0;
	/* u_int32 rxv_cnt_max = 100; */
	u_int32 rxv_content_len_max = 1000;
	u_int32 buf = 0;
	u_int8 *ptr = NULL, *ptr3 = NULL;
	u_int32 *ptr2 = NULL;
	u_int32 buf_size = 0;
	u_int32 mem_buf_len = 0;
	u_int16 len = 0, dw_cnt = 0, dw_idx = 0;
	u_int8 rxv_type = 0, entry_idx = 0;
	s_int8 idx = 0;
	struct rxv_dump_ring_attr ring_attr;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* request format type */
	get_param_and_shift_buf(TRUE, sizeof(version),
				&data, (u_char *)&version);
	get_param_and_shift_buf(TRUE, sizeof(action),
				&data, (u_char *)&action);

	switch (action) {
	case TEST_RXV_DUMP_START:
		get_param_and_shift_buf(TRUE, sizeof(type_mask),
			&data, (u_char *)&type_mask);
		break;

	case TEST_RXV_DUMP_STOP:
	case TEST_RXV_DUMP_REPORT:
		break;

	case TEST_RXV_DUMP_CLEAR_BUFFER:
	default:
		goto error4;
	}

	if ((action == TEST_RXV_DUMP_START) || (action == TEST_RXV_DUMP_STOP)) {
		ret = mt_serv_rxv_dump_action(serv_test, action, type_mask);
		if (ret != SERV_STATUS_SUCCESS)
			goto error5;
	} else if (action == TEST_RXV_DUMP_REPORT) {
		ptr = hqa_frame->data + 2;
		buf = SERV_OS_HTONL(version);
		sys_ad_move_mem(ptr, &buf, sizeof(version));
		buf_size += sizeof(version);
		ptr += sizeof(version);

		/* request rxv dump ctrl info */
		ret = mt_serv_get_rxv_dump_ring_attr(serv_test, &ring_attr);
		if (ret != SERV_STATUS_SUCCESS)
			goto error6;

		/* compute buffer memory size needed */
		for (rxv_type = 0; rxv_type < TEST_RXV_CONTENT_NUM;
			rxv_type++) {
			if (ring_attr.type_mask & BIT(rxv_type)) {
				mt_serv_get_rxv_content_len(serv_test,
					rxv_type, 1, &len);
				mem_buf_len += len;
			}
		}

		/* allocate dynamic memory for rx stat info */
		ret = sys_ad_alloc_mem((pu_char *)&ptr2, mem_buf_len);
		if (ret != SERV_STATUS_SUCCESS)
			goto error1;

		/* keep pointer for further update value */
		ptr3 = ptr;
		buf_size += sizeof(rxv_cnt);
		ptr += sizeof(rxv_cnt);

		buf_size += sizeof(rxv_content_len);
		ptr += sizeof(rxv_content_len);

		for (idx = ring_attr.ring_idx - ring_attr.valid_entry_num;
			idx < ring_attr.ring_idx; idx++) {

			entry_idx = idx;
			if (idx < 0)
				entry_idx += ring_attr.dump_entry_total_num;

			ret = mt_serv_get_rxv_dump_content(serv_test,
				entry_idx, ptr2, &len);
			if (ret != SERV_STATUS_SUCCESS)
				goto error2;

			/* check buffer length validity */
			if (rxv_content_len > rxv_content_len_max)
				goto error3;

			dw_cnt = len >> 2;
			for (dw_idx = 0; dw_idx < dw_cnt;
				dw_idx++, ptr2++, ptr += 4) {
				/* endian transform */
				buf = SERV_OS_HTONL(*ptr2);
				/* fill in block content */
				sys_ad_move_mem(ptr, &buf, sizeof(buf));
			}

			/* increment rxv count */
			rxv_cnt++;

			/* rxv content length */
			rxv_content_len += len;

			/* increment used buffer size */
			buf_size += len;

			sys_ad_zero_mem(ptr2, mem_buf_len);
		}

		/* free allocated memory */
		sys_ad_free_mem(ptr2);
		ptr2 = NULL;

		/* restore pointer address for update parameter */
		ptr = ptr3;
		buf = SERV_OS_HTONL(rxv_cnt);
		sys_ad_move_mem(ptr, &buf, sizeof(rxv_cnt));
		ptr += sizeof(rxv_cnt);

		buf = SERV_OS_HTONL(rxv_content_len);
		sys_ad_move_mem(ptr, &buf, sizeof(rxv_content_len));
		ptr += sizeof(rxv_content_len);

		/* clear rxv content buffer */
		mt_serv_rxv_dump_action(serv_test,
			TEST_RXV_DUMP_CLEAR_BUFFER, type_mask);
	}

	update_hqa_frame(hqa_frame, (2 + buf_size), ret);
	return ret;

error1:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: memory allocation fail for rxv content.\n",
		__func__));
	update_hqa_frame(hqa_frame, 2, ret);
	return ret;

error2:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: query rxv content fail.\n",
		__func__));
	/* free allocated memory */
	sys_ad_free_mem(ptr2);
	update_hqa_frame(hqa_frame, 2, ret);
	return ret;

error3:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: rxv content length is larger than max buffer size.\n",
		__func__));
	/* free allocated memory */
	sys_ad_free_mem(ptr2);
	update_hqa_frame(hqa_frame, 2, ret);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;

error4:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: invalid action type.\n",
		__func__));
	update_hqa_frame(hqa_frame, 2, ret);
	return SERV_STATUS_AGENT_NOT_SUPPORTED;

error5:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: rxv dump action fail for action(%d), type_mask(%d).\n",
		__func__, action, type_mask));
	update_hqa_frame(hqa_frame, 2, ret);
	return SERV_STATUS_AGENT_FAIL;

error6:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: rxv dump ring attr fail.\n",
		__func__));
	update_hqa_frame(hqa_frame, 2, ret);
	return SERV_STATUS_AGENT_FAIL;
}

static s_int32 hqa_calibration_test_mode(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config;
	u_char *data = hqa_frame->data;
	u_int32 mode, icap_len, resp_len = 2;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(mode),
				&data, (u_char *)&mode);
	get_param_and_shift_buf(TRUE, sizeof(icap_len),
				&data, (u_char *)&icap_len);

	/* Set parameters */
	test_config = &serv_test->test_config[serv_test->ctrl_band_idx];

	if (test_config) {
		if (mode == fTEST_OPER_NORMAL_MODE) {
			test_config->op_mode &=
				~(fTEST_FFT_ENABLE | fTEST_IN_RFTEST);
		} else if (mode == fTEST_OPER_RFTEST_MODE) {
			test_config->op_mode |= fTEST_IN_RFTEST;
		} else if (mode == fTEST_OPER_ICAP_MODE) {
			test_config->op_mode |= fTEST_IN_RFTEST;
		} else {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: Mode = %d error!!!\n", __func__, mode));
		}
	} else
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: Mode = test_config is null!!!\n", __func__));

	ret = mt_serv_calibration_test_mode(serv_test, mode);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}

static s_int32 hqa_do_cal_item(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 item = 0, band_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(item),
				&data, (u_char *)&item);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	serv_test->ctrl_band_idx = (u_char)band_idx;

	ret = mt_serv_do_cal_item(serv_test, item);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: item: %d, band_idx: %d\n", __func__, item, band_idx));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_tmr_setting(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_tmr_info *tmr_info = &serv_test->test_tmr;
	u_char *data = hqa_frame->data;

	get_param_and_shift_buf(TRUE, sizeof(tmr_info->setting),
				&data, (u_char *)&tmr_info->setting);
	get_param_and_shift_buf(TRUE, sizeof(tmr_info->version),
				&data, (u_char *)&tmr_info->version);
	get_param_and_shift_buf(TRUE, sizeof(tmr_info->through_hold),
				&data, (u_char *)&tmr_info->through_hold);
	get_param_and_shift_buf(TRUE, sizeof(tmr_info->iter),
				&data, (u_char *)&tmr_info->iter);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: setting=%d, version=%d, through_hold=%d, iter=%d\n",
		__func__, tmr_info->setting, tmr_info->version,
		tmr_info->through_hold, tmr_info->iter));

	ret = mt_serv_set_tmr(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_get_chipid(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 chip_id;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	ret = mt_serv_get_chipid(serv_test);

	chip_id = WINFO_GET_PARAM(serv_test, chip_id);
	chip_id = SERV_OS_HTONL(chip_id);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &chip_id, sizeof(chip_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(chip_id), ret);

	return ret;
}

static s_int32 hqa_mps_set_seq_data(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *data = hqa_frame->data;
	u_int32 *param = NULL;
	u_int32 band_idx, idx, value;
	u_int16 len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	len = hqa_frame->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&value);
		param[idx] = value;
	}

	/* Set parameters */
	serv_test->ctrl_band_idx = (u_char)band_idx;
	test_config = &serv_test->test_config[band_idx];
	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	mps_cb->mps_cnt = len;
	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++) {
		mps_setting[idx+1].tx_mode = (param[idx] & 0x0F000000) >> 24;
		mps_setting[idx+1].tx_ant = (param[idx] & 0x00FFFF00) >> 8;
		mps_setting[idx+1].mcs = (param[idx] & 0x000000FF);
	}
	sys_ad_free_mem(param);

	ret = mt_serv_mps_set_seq_data(serv_test);

err:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
		__func__, band_idx, len,
		test_config->op_mode, mps_cb->mps_cnt));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_set_payload_length(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *data = hqa_frame->data;
	u_int32 *param = NULL;
	u_int32 band_idx, idx, value;
	u_int16 len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	len = hqa_frame->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&value);
		param[idx] = value;
	}

	/* Set parameters */
	serv_test->ctrl_band_idx = (u_char)band_idx;
	test_config = &serv_test->test_config[band_idx];
	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++) {
		if (param[idx] > TEST_MAX_PKT_LEN)
			param[idx] = TEST_MAX_PKT_LEN;
		else if (param[idx] < TEST_MIN_PKT_LEN)
			param[idx] = TEST_MIN_PKT_LEN;

		mps_setting[idx+1].pkt_len = param[idx];
	}
	sys_ad_free_mem(param);
err:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
		__func__, band_idx, len,
		test_config->op_mode, mps_cb->mps_cnt));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_set_packet_count(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *data = hqa_frame->data;
	u_int32 *param = NULL;
	u_int32 band_idx, idx, value;
	u_int16 len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	len = hqa_frame->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&value);
		param[idx] = value;
	}

	/* Set parameters */
	serv_test->ctrl_band_idx = (u_char)band_idx;
	test_config = &serv_test->test_config[band_idx];
	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].pkt_cnt = param[idx];

	sys_ad_free_mem(param);
err:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
		__func__, band_idx, len,
		test_config->op_mode, mps_cb->mps_cnt));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_set_power_gain(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *data = hqa_frame->data;
	u_int32 *param = NULL;
	u_int32 band_idx, idx, value;
	u_int16 len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	len = hqa_frame->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&value);
		param[idx] = value;
	}

	/* Set parameters */
	serv_test->ctrl_band_idx = (u_char)band_idx;
	test_config = &serv_test->test_config[band_idx];
	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].pwr = param[idx];

	sys_ad_free_mem(param);
err:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
		__func__, band_idx, len,
		test_config->op_mode, mps_cb->mps_cnt));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_start(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band_idx;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_mps_operation(serv_test, SERV_TEST_MPS_START_TX);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_stop(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band_idx;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_mps_operation(serv_test, SERV_TEST_MPS_STOP_TX);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_check_efuse_mode_type(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 e2p_cur_mode = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	e2p_cur_mode = (u_int32)WINFO_GET_PARAM(serv_test, e2p_cur_mode);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: e2p_cur_mode=0x%x\n", __func__, e2p_cur_mode));

	e2p_cur_mode = SERV_OS_HTONL(e2p_cur_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &e2p_cur_mode,
			sizeof(e2p_cur_mode));
	update_hqa_frame(hqa_frame, 2 + sizeof(e2p_cur_mode), ret);

	return ret;
}

static s_int32 hqa_check_efuse_nativemode_type(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 e2p_access_mode = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	e2p_access_mode = (u_int32)WINFO_GET_PARAM(serv_test, e2p_access_mode);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: e2p_access_mode=0x%x\n", __func__, e2p_access_mode));

	e2p_access_mode = SERV_OS_HTONL(e2p_access_mode);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &e2p_access_mode,
			sizeof(e2p_access_mode));
	update_hqa_frame(hqa_frame, 2 + sizeof(e2p_access_mode), ret);

	return ret;
}

static s_int32 hqa_set_band_mode(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_band_state *band_state;
	u_char *data = hqa_frame->data;
	u_int32 band_mode = 0, band_type = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_mode),
				&data, (u_char *)&band_mode);
	get_param_and_shift_buf(TRUE, sizeof(band_type),
				&data, (u_char *)&band_type);

	/* Set parameters */
	band_state = SERV_GET_PADDR(serv_test, test_bstat);
	BSTATE_SET_PARAM(serv_test, band_mode, band_mode);
	BSTATE_SET_PARAM(serv_test, band_type, band_type);

	ret = mt_serv_set_band_mode(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_mode=%u, band_type=%u\n",
		__func__, band_mode, band_type));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_get_band_mode(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_band_state *band_state;
	u_char *data = hqa_frame->data;
	u_int32 band_type = 0, band_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	band_state = SERV_GET_PADDR(serv_test, test_bstat);
	serv_test->ctrl_band_idx = (u_char)band_idx;
	ret = mt_serv_get_band_mode(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_type=%u\n",
		__func__, band_type));

	band_type = SERV_OS_HTONL(band_state->band_type);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &band_type, sizeof(band_type));
	update_hqa_frame(hqa_frame, 2 + sizeof(band_type), ret);

	return ret;
}

static s_int32 hqa_rdd_start(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 rdd_num = 0;
	u_int32 rdd_sel = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(rdd_num),
				&data, (u_char *)&rdd_num);
	get_param_and_shift_buf(TRUE, sizeof(rdd_sel),
				&data, (u_char *)&rdd_sel);

	ret = mt_serv_set_rdd_on_off(serv_test, rdd_num, rdd_sel, TRUE);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: rdd_num: %d, rdd_sel: %d\n",
		__func__, rdd_num, rdd_sel));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_rdd_stop(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 rdd_num = 0;
	u_int32 rdd_sel = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(rdd_num),
				&data, (u_char *)&rdd_num);
	get_param_and_shift_buf(TRUE, sizeof(rdd_sel),
				&data, (u_char *)&rdd_sel);

	ret = mt_serv_set_rdd_on_off(serv_test, rdd_num, rdd_sel, FALSE);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: rdd_num: %d, rdd_sel: %d\n",
		__func__, rdd_num, rdd_sel));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_log_on_off(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0, log_type = 0, log_ctrl = 0, log_size = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(log_type),
				&data, (u_char *)&log_type);
	get_param_and_shift_buf(TRUE, sizeof(log_ctrl),
				&data, (u_char *)&log_ctrl);
	get_param_and_shift_buf(TRUE, sizeof(log_size),
				&data, (u_char *)&log_size);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_log_on_off(serv_test, log_type, log_ctrl, log_size);

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_set_nss(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *data = hqa_frame->data;
	u_int32 *param = NULL;
	u_int32 band_idx, idx, value;
	u_int16 len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *) &band_idx);

	len = hqa_frame->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *) &value);
		param[idx] = value;
	}

	/* Set parameters */
	serv_test->ctrl_band_idx = (u_char)band_idx;
	test_config = &serv_test->test_config[band_idx];
	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].nss = param[idx];

	sys_ad_free_mem(param);
err:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
		__func__, band_idx, len,
		test_config->op_mode, mps_cb->mps_cnt));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_mps_set_per_packet_bw(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct test_configuration *test_config = NULL;
	struct test_mps_cb *mps_cb = NULL;
	struct test_mps_setting *mps_setting = NULL;
	u_char *data = hqa_frame->data;
	u_int32 *param = NULL;
	u_int32 band_idx, idx, value;
	u_int16 len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	len = hqa_frame->length / sizeof(u_int32) - 1;
	if ((len > TEST_MPS_ITEM_LEN) || (len == 0)) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err;
	}

	ret = sys_ad_alloc_mem((u_char **)&param, sizeof(u_int32) * len);
	if (ret) {
		ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
		goto err;
	}

	for (idx = 0; idx < len; idx++) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *) &value);

		switch (value) {
		case 0:
			value = TEST_BW_20;
			break;

		case 1:
			value = TEST_BW_40;
			break;

		case 2:
			value = TEST_BW_80;
			break;

		case 3:
			value = TEST_BW_10;
			break;

		case 4:
			value = TEST_BW_5;
			break;

		case 5:
			value = TEST_BW_160C;
			break;

		case 6:
			value = TEST_BW_160NC;
			break;

		default:
			value = TEST_BW_20;
			break;
		}

		param[idx] = value;
	}

	/* Set parameters */
	serv_test->ctrl_band_idx = (u_char)band_idx;
	test_config = &serv_test->test_config[band_idx];
	mps_cb = &test_config->mps_cb;

	if (test_config->op_mode & fTEST_MPS) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err;
	}

	if (!mps_cb->mps_setting && !mps_cb->mps_cnt) {
		mps_cb->mps_cnt = len;
		ret = sys_ad_alloc_mem((u_char **)&mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_NULL_POINTER;
			goto err;
		}
		sys_ad_zero_mem(mps_cb->mps_setting,
				sizeof(struct test_mps_setting) * (len+1));
	}

	mps_setting = mps_cb->mps_setting;

	for (idx = 0; idx < len; idx++)
		mps_setting[idx+1].pkt_bw = param[idx];

	sys_ad_free_mem(param);
err:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%d, len=%d, op_mode=0x%x, mps_cnt=%d\n",
		__func__, band_idx, len,
		test_config->op_mode, mps_cb->mps_cnt));

	/* Update hqa_frame with response: status (2 bytes) */
	update_hqa_frame(hqa_frame, 2, ret);

	return ret;
}

static s_int32 hqa_icap_ctrl(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32 icap_stat = 0;
	u_char *data = hqa_frame->data;
	u_int32 wf_num = 0, iq_type = 0;
	u_int32 control = 0, resp_len = 2;
	u_int32 value = 0, i = 0;
	u_long max_data_len = 0;
	u_char src_addr[SERV_MAC_ADDR_LEN];
	s_int32 *icap_data = NULL;
	s_int32 *icap_data_cnt = NULL;
	struct hqa_rbist_cap_start icap_info;

	get_param_and_shift_buf(TRUE, sizeof(control),
				&data, (u_char *)&control);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: control: %d\n", __func__, control));

	switch (control) {
	case 1:
		/* clear memory */
		sys_ad_zero_mem(&icap_info, sizeof(icap_info));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.trig),
			&data, (u_char *)&(icap_info.trig));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.ring_cap_en),
			&data, (u_char *)&(icap_info.ring_cap_en));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.trig_event),
			&data, (u_char *)&(icap_info.trig_event));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_node),
			&data, (u_char *)&(icap_info.cap_node));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_len),
			&data, (u_char *)&(icap_info.cap_len));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_stop_cycle),
			&data, (u_char *)&(icap_info.cap_stop_cycle));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.bw),
			&data, (u_char *)&(icap_info.bw));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.mac_trig_event),
			&data, (u_char *)&(icap_info.mac_trig_event));

		get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
			&data, (u_char *)src_addr);

		get_param_and_shift_buf(TRUE, sizeof(icap_info.band_idx),
			&data, (u_char *)&(icap_info.band_idx));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.phy_idx),
			&data, (u_char *)&(icap_info.phy_idx));

		get_param_and_shift_buf(TRUE, sizeof(icap_info.cap_src),
			&data, (u_char *)&(icap_info.cap_src));

		/* source address lsb */
		for (i = 0; i < 4; i++)
			icap_info.src_addr_lsb |= ((src_addr[i]) << (i << 3));

		/* source address msb */
		for (i = 0; i < 2; i++)
			icap_info.src_addr_msb |= ((src_addr[i+4]) << (i << 3));
		icap_info.src_addr_msb |= ((0x1) << 16);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: trig = 0x%08x, ring_cap_en = 0x%08x\n"
		, __func__, icap_info.trig, icap_info.ring_cap_en));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: trig_event = 0x%08x, cap_node = 0x%08x\n"
		, __func__, icap_info.trig_event, icap_info.cap_node));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cap_len = 0x%08x, cap_stop_cycle = 0x%08x\n"
		, __func__, icap_info.cap_len, icap_info.cap_stop_cycle));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: bw = 0x%08x, mac_trig_event = 0x%08x\n"
		, __func__, icap_info.bw, icap_info.mac_trig_event));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: src_addr_msb = 0x%08x, src_addr_lsb = 0x%08x\n"
		, __func__, icap_info.src_addr_msb, icap_info.src_addr_lsb));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx = 0x%08x, phy_idx = 0x%08x\n"
		, __func__, icap_info.band_idx, icap_info.phy_idx));

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: cap_src = 0x%08x\n", __func__, icap_info.cap_src));

		ret = mt_serv_set_icap_start(serv_test, &icap_info);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s : mt_serv_set_icap_start is failed!!\n"
			, __func__));
			goto error1;
		}
		break;

	case 2:
		ret = mt_serv_get_icap_status(serv_test, &icap_stat);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s : mt_serv_get_icap_status is failed!!\n"
			, __func__));
			goto error1;
		} else {
			ret = icap_stat;
			update_hqa_frame(hqa_frame, resp_len, ret);
			return SERV_STATUS_SUCCESS;
		}
		break;

	case 3:
		get_param_and_shift_buf(TRUE, sizeof(wf_num),
			&data, (u_char *)&(wf_num));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: wf_num: %d\n", __func__, wf_num));

		get_param_and_shift_buf(TRUE, sizeof(iq_type),
			&data, (u_char *)&(iq_type));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: iq_type: %d\n", __func__, iq_type));

		ret = sys_ad_alloc_mem((u_char **)&icap_data_cnt
				, sizeof(s_int32));
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s : Not enough memory for dynamic allocating!!\n"
			, __func__));
			goto error1;
		}
		sys_ad_zero_mem(icap_data_cnt, sizeof(s_int32));

		ret = mt_serv_get_icap_max_data_len(serv_test, &max_data_len);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s : mt_serv_get_icap_max_data_len is failed!!\n"
			, __func__));
			goto error1;
		}

		ret = sys_ad_alloc_mem((u_char **)&icap_data, max_data_len);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s : Not enough memory for dynamic allocating!!\n"
			, __func__));
			goto error1;
		}
		sys_ad_zero_mem(icap_data, max_data_len);

		ret = mt_serv_get_icap_data(serv_test,
				icap_data_cnt, icap_data, wf_num, iq_type);
		if (ret) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s : mt_serv_get_icap_data is not supported!!\n"
			, __func__));
			goto error1;
		}

		value = SERV_OS_HTONL(control);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);
		value = SERV_OS_HTONL(wf_num);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);
		value = SERV_OS_HTONL(iq_type);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);
		value = SERV_OS_HTONL(*icap_data_cnt);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);

		for (i = 0; i < *icap_data_cnt; i++) {
			value = SERV_OS_HTONL(icap_data[i]);
			sys_ad_move_mem(hqa_frame->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);
		}
		break;

	default:
		break;
	}

error1:
	if (ret)
		ret = SERV_STATUS_AGENT_NOT_SUPPORTED;

	update_hqa_frame(hqa_frame, resp_len, ret);

	if (icap_data_cnt)
		sys_ad_free_mem(icap_data_cnt);

	if (icap_data)
		sys_ad_free_mem(icap_data);

	return ret;
}

static s_int32 hqa_get_dump_recal(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 6;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0;
	u_int32 recal_cnt = 0, recal_dw_num = 0;
	u_int32 *content = NULL;
	u_int32 value = 0, dw_cnt = 0, i = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	serv_test->ctrl_band_idx = (u_char)band_idx;

	ret = mt_serv_get_recal_cnt(serv_test,
		&recal_cnt, &recal_dw_num);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx: %d, rxv_cnt: %d, rxv_dw_num: %d\n",
		__func__, band_idx, recal_cnt, recal_dw_num));

	dw_cnt = recal_cnt * recal_dw_num;

	ret = sys_ad_alloc_mem((u_char **)&content,
		sizeof(*content) * dw_cnt);
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	ret = mt_serv_get_recal_content(serv_test, content);

	/* Update hqa_frame with response: status (2 bytes) */
	value = SERV_OS_HTONL(recal_cnt);
	sys_ad_move_mem(hqa_frame->data + 2,
		&value, sizeof(value));

	for (i = 0; i < dw_cnt; i++, content++) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: content[%d]: 0x%x\n", __func__, i, *content));

		value = SERV_OS_HTONL(*content);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));

		resp_len += sizeof(value);
	}

	/* Free memory */
	sys_ad_free_mem(content);

	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;

error1:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: dynamic memory allocate fail!!\n", __func__));
	if (content)
		sys_ad_free_mem(content);
	/* TODO: respond to application for error handle */
	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}

static s_int32 hqa_get_dump_rxv_legacy(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0;
	u_int32 rxv_cnt = 0, rxv_dw_num = 0;
	u_int32 *content = NULL;
	u_int32 value = 0, dw_cnt = 0, i = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	serv_test->ctrl_band_idx = (u_char)band_idx;

	ret = mt_serv_get_rxv_cnt(serv_test, &rxv_cnt, &rxv_dw_num);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx: %d, rxv_cnt: %d, rxv_dw_num: %d\n",
		__func__, band_idx, rxv_cnt, rxv_dw_num));

	dw_cnt = rxv_cnt * rxv_dw_num;
	ret = sys_ad_alloc_mem((u_char **)&content,
		sizeof(*content) * dw_cnt);
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	ret = mt_serv_get_rxv_content(serv_test, dw_cnt, content);

	/* Update hqa_frame with response: Count (2 bytes) */
	value = SERV_OS_HTONL(rxv_cnt);
	sys_ad_move_mem(hqa_frame->data + resp_len,
		&value, sizeof(value));

	resp_len += sizeof(value);

	for (i = 0; i < dw_cnt; i += 4, content++) {
		value = SERV_OS_HTONL(*content);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: resp_len = %d\n", __func__, resp_len));

	/* Free memory */
	sys_ad_free_mem(content);

	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;

error1:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: dynamic memory allocate fail!!\n", __func__));
	if (content)
		sys_ad_free_mem(content);
	/* TODO: respond to application for error handle */
	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}

static s_int32 hqa_get_dump_rdd(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_char *data = hqa_frame->data;
	u_int32 band_idx = 0;
	u_int32 rdd_cnt = 0, rdd_dw_num = 0;
	u_int32 *content = NULL;
	u_int32 value = 0, i = 0, total_cnt = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	serv_test->ctrl_band_idx = (u_char)band_idx;

	ret = mt_serv_get_rdd_cnt(serv_test, &rdd_cnt, &rdd_dw_num);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx: %d, pulse number: %d, rdd buffer size: %d\n",
		__func__, band_idx, rdd_cnt, rdd_dw_num));

	ret = sys_ad_alloc_mem((u_char **)&content,
		sizeof(*content) * rdd_dw_num);
	if (ret != SERV_STATUS_SUCCESS)
		goto error1;

	ret = mt_serv_get_rdd_content(serv_test, content, &total_cnt);

	if (total_cnt > 0) {
		/* Update hqa_frame with response: status (2 bytes) */
		/* Response format:
		 * cmd type + cmd ID + length + Sequence +
		 * data:
		 * status (2 bytes) +
		 * [count (4 bytes)] + value1 (4 bytes) + value2 (4 bytes)
		 */
		/* Count = Total number of 4 bytes RDD values divided by 2 */
		value = SERV_OS_HTONL(total_cnt/2);
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&value, sizeof(value));
		resp_len += sizeof(value);

		for (i = 0; i < total_cnt; i++, content++) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("%s: content[%d]: 0x%08x\n",
				__func__, i, *content));

			value = SERV_OS_HTONL(*content);
			sys_ad_move_mem(hqa_frame->data + resp_len,
				&value, sizeof(value));
			resp_len += sizeof(value);
		}

	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: total_cnt %d\n", __func__, total_cnt));
		sys_ad_move_mem(hqa_frame->data + resp_len,
			&total_cnt, sizeof(total_cnt));
		resp_len += sizeof(total_cnt);
	}

	/* Free memory */
	sys_ad_free_mem(content);

	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;

error1:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		("%s: dynamic memory allocate fail!!\n", __func__));
	if (content)
		sys_ad_free_mem(content);
	/* TODO: respond to application for error handle */
	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}

static s_int32 hqa_get_hetb_info(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_char band_idx = serv_test->ctrl_band_idx;
	u_int32 afactor = 0, ldpc_str_sym = 0, pe_disamb = 0;
	u_int32 tx_pe = 0, l_sig_len = 0, value = 0;
	u_int8 dmnt_ru_idx = 0;
	struct test_ru_info *ru_info = NULL;

	dmnt_ru_idx = CONFIG_GET_PARAM(serv_test, dmnt_ru_idx, band_idx);
	ru_info = CONFIG_GET_PADDR(serv_test,
					 ru_info_list[dmnt_ru_idx],
					 band_idx);
	afactor = ru_info->afactor_init;
	ldpc_str_sym = ru_info->ldpc_extr_sym;
	pe_disamb = ru_info->pe_disamb;
	tx_pe = ru_info->t_pe;
	l_sig_len = ru_info->l_len;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: band_idx:0x%x, dmnt_ru_idx:0x%x afactor:0x%x\n",
			__func__, band_idx, dmnt_ru_idx, afactor));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("ldpc_str_sym:0x%x, pe_disamb:0x%x\n",
			ldpc_str_sym, pe_disamb));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("tx_pe:0x%x,l_sig_len:0x%x\n",
			tx_pe, l_sig_len));

	/* The response array should be a-factor,
	 *				    ldpc extra symbol,
	 *				    PE disambiguilty,
	 *				    TX PE,
	 *				    L-SIG length
	 */
	value = SERV_OS_HTONL(afactor);
	sys_ad_move_mem(hqa_frame->data + 2, &value, sizeof(value));
	value = SERV_OS_HTONL(ldpc_str_sym);
	sys_ad_move_mem(hqa_frame->data + 2 + sizeof(value), &value,
				sizeof(value));
	value = SERV_OS_HTONL(pe_disamb);
	sys_ad_move_mem(hqa_frame->data + 2 + sizeof(value)*2,
				&value, sizeof(value));
	value = SERV_OS_HTONL(tx_pe);
	sys_ad_move_mem(hqa_frame->data + 2 + sizeof(value)*3, &value,
				sizeof(value));
	value = SERV_OS_HTONL(l_sig_len);
	sys_ad_move_mem(hqa_frame->data + 2 + sizeof(value)*4, &value,
				sizeof(value));
	resp_len += 5*sizeof(u_int32);

	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}

static s_int32 hqa_translate_ru_allocation(
	u_int32 user_ru_allocation,
	u_int32 *allocation)
{
	u_int8 i = 0;

	*allocation = 0;
	for (i = 0 ; i < sizeof(u_int32)*2 ; i++) {
		*allocation |= ((user_ru_allocation & 0x1) << i);
		user_ru_allocation >>= 4;
	}

	return SERV_STATUS_SUCCESS;
}

static s_int32 hqa_set_ru_info(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_char band_idx = serv_test->ctrl_band_idx;
	u_int32 len = 0, seg_sta_cnt[2] = {0}, sta_seq = 0, value = 0;
	u_char param_cnt = 0, segment_idx = 0, param_loop = 0;
	u_char *data = hqa_frame->data;
	u_int32 mpdu_length = 0;
	struct test_ru_allocatoin *ru_allocation = NULL;
	struct test_ru_info *ru_info = NULL;

	len = hqa_frame->length;
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   &data,
				   (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   &data,
				   (u_char *)&seg_sta_cnt[0]);
	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   &data,
				   (u_char *)&seg_sta_cnt[1]);
	len -= sizeof(u_int32)*3;		/* array length */

	if (seg_sta_cnt[0]+seg_sta_cnt[1] == 0) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	len /= (seg_sta_cnt[0]+seg_sta_cnt[1]);	/* per ru length */
	param_cnt = len/sizeof(u_int32);	/* param count */
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		 ("%s: Band:%d [ru_segment 0]:%d, [ru_segment 1]:%d\n",
		 __func__, band_idx, seg_sta_cnt[0], seg_sta_cnt[1]));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		 ("\t\tparameters count:%d\n", param_cnt));

	mpdu_length = CONFIG_GET_PARAM(serv_test, tx_len, band_idx);
	ru_allocation = CONFIG_GET_PADDR(serv_test, ru_alloc, band_idx);
	ru_info = CONFIG_GET_PADDR(serv_test, ru_info_list[0], band_idx);
	sys_ad_zero_mem(ru_info, sizeof(struct test_ru_info)*MAX_MULTI_TX_STA);
	sys_ad_set_mem(ru_allocation, sizeof(*ru_allocation), 0xff);

	/* for maximum bw 80+80/160, 2 segments only */
	for (sta_seq = 0;
	     sta_seq < seg_sta_cnt[0]+seg_sta_cnt[1];
	     sta_seq++) {
		param_loop = param_cnt;

		if (sta_seq < seg_sta_cnt[0])
			segment_idx = 0;
		else
			segment_idx = 1;

		ru_info[sta_seq].valid = TRUE;
		/* ru caterogy */
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		/* ru allocation */
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		hqa_translate_ru_allocation(value,
				    &ru_info[sta_seq].allocation);
		/* aid */
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		ru_info[sta_seq].aid = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		ru_info[sta_seq].ru_index = (value << 1) | segment_idx;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		ru_info[sta_seq].rate = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		ru_info[sta_seq].ldpc = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		ru_info[sta_seq].nss = value;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		ru_info[sta_seq].start_sp_st = value-1;
		get_param_and_shift_buf(TRUE,
					   sizeof(u_int32),
					   &data,
					   (u_char *)&value);
		param_loop--;
		if (value > 24)
			ru_info[sta_seq].mpdu_length = value;
		else
			ru_info[sta_seq].mpdu_length = mpdu_length;

		if (param_loop) {
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   &data,
						   (u_char *)&value);
			param_loop--;
			ru_info[sta_seq].alpha = value;
		}

		if (param_loop) {
			get_param_and_shift_buf(TRUE,
						   sizeof(u_int32),
						   &data,
						   (u_char *)&value);
			param_loop--;
			ru_info[sta_seq].ru_mu_nss = value;
		}

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			 ("%s: ru_segment[%d][0x%x]: ru_idx:%d\n",
			 __func__, segment_idx,
			 ru_info[sta_seq].allocation,
			 ru_info[sta_seq].ru_index >> 1));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			 ("\t\t\t\trate:%x, ldpc:%d\n",
			 ru_info[sta_seq].rate,
			 ru_info[sta_seq].ldpc));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			 ("\t\t\t\tnss:%d, mimo nss:%d\n",
			 ru_info[sta_seq].nss,
			 ru_info[sta_seq].ru_mu_nss));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			 ("\t\t\t\t start spatial stream:%d,\n",
			 ru_info[sta_seq].start_sp_st));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			 ("\t\t\t\tmpdu length=%d, alpha:%d\n",
			 ru_info[sta_seq].mpdu_length,
			 ru_info[sta_seq].alpha));
	}

err_out:
	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}

static s_int32 hqa_w_cali(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 resp_len = 2;
	u_char *data = hqa_frame->data;
	u_int32 storage = 0;

	get_param_and_shift_buf(TRUE,
				   sizeof(u_int32),
				   &data,
				   (u_char *)&storage);

	mt_serv_store_cali(serv_test, storage, data+4);
	update_hqa_frame(hqa_frame, resp_len, ret);

	return ret;
}


static struct hqa_cmd_entry CMD_SET5[] = {
	/* cmd id start from 0x1500 */
	{0x0,	hqa_get_fw_info},
	{0x1,	legacy_function},
	{0x2,	hqa_set_stbc},
	{0x3,	hqa_set_short_gi},
	{0x4,	hqa_set_dpd},
	{0x5,	hqa_set_tssi_onoff},
	{0x6,	legacy_function},
	{0x7,	legacy_function},
	{0x8,	legacy_function},
	{0x9,	hqa_calibration_test_mode},
	{0xa,	hqa_do_cal_item},
	{0xf,	hqa_tmr_setting},
	{0x10,	legacy_function},
	{0x11,	hqa_w_cali},
	{0x12,	legacy_function},
	{0x13,	legacy_function},
	{0x14,	hqa_get_chipid},
	{0x15,	hqa_mps_set_seq_data},
	{0x16,	hqa_mps_set_payload_length},
	{0x17,	hqa_mps_set_packet_count},
	{0x18,	hqa_mps_set_power_gain},
	{0x19,	hqa_mps_start},
	{0x1a,	hqa_mps_stop},
	{0x1c,	hqa_get_rx_statistics_all},
	{0x21,	legacy_function},
	{0x22,	hqa_check_efuse_mode_type},
	{0x23,	hqa_check_efuse_nativemode_type},
	{0x24,	legacy_function},
	{0x25,	legacy_function},
	{0x26,	legacy_function},
	{0x27,	legacy_function},
	{0x28,	legacy_function},
	{0x29,	legacy_function},
	{0x2c,	hqa_set_band_mode},
	{0x2d,	hqa_get_band_mode},
	{0x2e,	hqa_rdd_start},
	{0x2f,	hqa_rdd_stop},
	{0x31,	legacy_function},
	{0x32,	legacy_function},
	{0x33,	hqa_log_on_off},
	{0x34,	legacy_function},
	{0x35,	legacy_function},
	{0x36,	hqa_mps_set_nss},
	{0x37,	hqa_mps_set_per_packet_bw},
	{0x80,	hqa_icap_ctrl},
	{0x81,	hqa_get_dump_recal},
	{0x82,	hqa_get_dump_rxv_legacy},
	{0x83,	hqa_get_dump_rdd},
	{0x84,	hqa_get_rxv_dump},
	{0x91,	hqa_get_hetb_info},
	{0x94,	hqa_set_ru_info}
};

static s_int32 hqa_set_channel_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_set_ch param;
	u_char *data = hqa_frame->data;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(param.ext_id),
				&data, (u_char *)&param.ext_id);
	get_param_and_shift_buf(TRUE, sizeof(param.num_param),
				&data, (u_char *)&param.num_param);
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				&data, (u_char *)&param.band_idx);
	get_param_and_shift_buf(TRUE, sizeof(param.central_ch0),
				&data, (u_char *)&param.central_ch0);
	get_param_and_shift_buf(TRUE, sizeof(param.central_ch1),
				&data, (u_char *)&param.central_ch1);
	get_param_and_shift_buf(TRUE, sizeof(param.sys_bw),
				&data, (u_char *)&param.sys_bw);
	get_param_and_shift_buf(TRUE, sizeof(param.perpkt_bw),
				&data, (u_char *)&param.perpkt_bw);
	get_param_and_shift_buf(TRUE, sizeof(param.pri_sel),
				&data, (u_char *)&param.pri_sel);
	get_param_and_shift_buf(TRUE, sizeof(param.reason),
				&data, (u_char *)&param.reason);
	get_param_and_shift_buf(TRUE, sizeof(param.ch_band),
				&data, (u_char *)&param.ch_band);
	get_param_and_shift_buf(TRUE, sizeof(param.out_band_freq),
				&data, (u_char *)&param.out_band_freq);

	if (param.band_idx >= TEST_DBDC_BAND_NUM)
		ret = SERV_STATUS_AGENT_INVALID_BANDIDX;

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)param.band_idx);
	CONFIG_SET_PARAM(serv_test, channel,
			(u_char)param.central_ch0, param.band_idx);
	CONFIG_SET_PARAM(serv_test, channel_2nd,
			(u_char)param.central_ch1, param.band_idx);
	CONFIG_SET_PARAM(serv_test, per_pkt_bw,
			(u_char)param.perpkt_bw, param.band_idx);
	CONFIG_SET_PARAM(serv_test, bw,
			(u_char)param.sys_bw, param.band_idx);
	CONFIG_SET_PARAM(serv_test, pri_sel,
			(u_char)param.pri_sel, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ch_band,
			(u_char)param.ch_band, param.band_idx);
	CONFIG_SET_PARAM(serv_test, out_band_freq,
			(u_int32)param.out_band_freq, param.band_idx);

	ret = mt_serv_set_channel(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx: %d, ch0: %d, ch1: %d, sys_bw: %d, ",
		__func__, param.band_idx, param.central_ch0,
		param.central_ch1, param.sys_bw));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("perpkt_bw: %d, pri_sel: %d, ch_band: %d\n",
		param.perpkt_bw, param.pri_sel, param.ch_band));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &param.ext_id, 4);
	update_hqa_frame(hqa_frame, 6, ret);

	return ret;
}

static s_int32 hqa_set_txcontent_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_tx_content param;
	struct serv_hdr_802_11 *phdr = NULL;
	u_char *data = hqa_frame->data;
	u_char *payload, sta_seq = 0;
	boolean enable = FALSE;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(param.ext_id),
				&data, (u_char *)&param.ext_id);
	get_param_and_shift_buf(TRUE, sizeof(param.num_param),
				&data, (u_char *)&param.num_param);
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				&data, (u_char *)&param.band_idx);
	get_param_and_shift_buf(TRUE, sizeof(param.fc),
				&data, (u_char *)&param.fc);
	get_param_and_shift_buf(TRUE, sizeof(param.dur),
				&data, (u_char *)&param.dur);
	get_param_and_shift_buf(TRUE, sizeof(param.seq),
				&data, (u_char *)&param.seq);
	get_param_and_shift_buf(TRUE, sizeof(param.fixed_payload),
				&data, (u_char *)&param.fixed_payload);
	get_param_and_shift_buf(TRUE, sizeof(param.txlen),
				&data, (u_char *)&param.txlen);
	get_param_and_shift_buf(TRUE, sizeof(param.payload_len),
				&data, (u_char *)&param.payload_len);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				&data, param.addr1);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				&data, param.addr2);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				&data, param.addr3);

	/* 52 for the size before payload */
	if (param.payload_len > TEST_MAX_PATTERN_SIZE)
		param.payload_len = TEST_MAX_PATTERN_SIZE;

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)param.band_idx);
	for (sta_seq = 0; sta_seq < MAX_MULTI_TX_STA ; sta_seq++) {
		param.addr1[5] +=
		sta_seq+(param.band_idx*(MAX_MULTI_TX_STA/TEST_DBDC_BAND_NUM));
		CONFIG_SET_PADDR(serv_test, addr1[sta_seq], param.addr1,
				SERV_MAC_ADDR_LEN, param.band_idx);
		CONFIG_SET_PADDR(serv_test, addr2[sta_seq], param.addr2,
				SERV_MAC_ADDR_LEN, param.band_idx);
		CONFIG_SET_PADDR(serv_test, addr3[sta_seq], param.addr3,
				SERV_MAC_ADDR_LEN, param.band_idx);
	}
	CONFIG_SET_PARAM(serv_test, dur,
			(u_short)param.dur, param.band_idx);
	CONFIG_SET_PARAM(serv_test, seq,
			(u_short)param.seq, param.band_idx);
	CONFIG_SET_PARAM(serv_test, pl_len,
			(u_int32)param.payload_len, param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_len,
			(u_int32)param.txlen, param.band_idx);
	CONFIG_SET_PARAM(serv_test, fixed_payload,
			(u_int32)param.fixed_payload, param.band_idx);

	payload = CONFIG_GET_PADDR(serv_test, payload[0], param.band_idx);
	phdr = (struct serv_hdr_802_11 *)CONFIG_GET_PADDR(serv_test,
							template_frame,
							param.band_idx);
	sys_ad_move_mem(&phdr->fc, &param.fc, sizeof(phdr->fc));
	phdr->duration = (u_int16)param.dur;
	phdr->sequence = (u_int16)param.seq;

	/* Error check for txlen and payload_len */
	if ((param.txlen == 0) || (param.payload_len == 0)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: txlen/payload_len=%u/%u can't be 0!!\n",
			__func__, param.txlen, param.payload_len));
		return SERV_STATUS_AGENT_INVALID_LEN;
	}

	/* Packet tx time feature implementation */
	enable = CONFIG_GET_PARAM(serv_test, tx_time_param.pkt_tx_time_en,
				param.band_idx);
	if (enable == TRUE) {
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time,
				(u_int32)param.txlen, param.band_idx);
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: fc=0x%04x, dur=%u, seq=%u, plen=%u\n",
			__func__, param.fc, param.dur, param.seq,
			param.payload_len));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: pkt_tx_time=%u, genpkt=%u\n",
			__func__, param.txlen,
			param.fixed_payload));
	} else {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: fc=0x%04x, dur=%u, seq=%u, plen=%u\n",
			__func__, param.fc, param.dur, param.seq,
			param.payload_len));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: txlen=%u, genpkt=%u\n",
			__func__, param.txlen, param.fixed_payload));
	}

	get_param_and_shift_buf(FALSE, param.payload_len, &data, payload);

	ret = mt_serv_set_tx_content(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, SERV_PRINT_MAC(param.addr1)));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, SERV_PRINT_MAC(param.addr2)));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: addr3=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, SERV_PRINT_MAC(param.addr3)));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &param.ext_id,
			sizeof(param.ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(param.ext_id), ret);

	return ret;
}

static s_int32 hqa_start_tx_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_tx param;
	u_char *data = hqa_frame->data;
	u_char ant_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(param.ext_id),
				&data, (u_char *)&param.ext_id);
	get_param_and_shift_buf(TRUE, sizeof(param.num_param),
				&data, (u_char *)&param.num_param);
	get_param_and_shift_buf(TRUE, sizeof(param.band_idx),
				&data, (u_char *)&param.band_idx);
	get_param_and_shift_buf(TRUE, sizeof(param.pkt_cnt),
				&data, (u_char *)&param.pkt_cnt);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_mode),
				&data, (u_char *)&param.tx_mode);
	get_param_and_shift_buf(TRUE, sizeof(param.rate),
				&data, (u_char *)&param.rate);
	get_param_and_shift_buf(TRUE, sizeof(param.pwr),
				&data, (u_char *)&param.pwr);
	get_param_and_shift_buf(TRUE, sizeof(param.stbc),
				&data, (u_char *)&param.stbc);
	get_param_and_shift_buf(TRUE, sizeof(param.ldpc),
				&data, (u_char *)&param.ldpc);
	get_param_and_shift_buf(TRUE, sizeof(param.ibf),
				&data, (u_char *)&param.ibf);
	get_param_and_shift_buf(TRUE, sizeof(param.ebf),
				&data, (u_char *)&param.ebf);
	get_param_and_shift_buf(TRUE, sizeof(param.wlan_id),
				&data, (u_char *)&param.wlan_id);
	get_param_and_shift_buf(TRUE, sizeof(param.aifs),
				&data, (u_char *)&param.aifs);
	get_param_and_shift_buf(TRUE, sizeof(param.gi),
				&data, (u_char *)&param.gi);
	get_param_and_shift_buf(TRUE, sizeof(param.tx_path),
				&data, (u_char *)&param.tx_path);
	get_param_and_shift_buf(TRUE, sizeof(param.nss),
				&data, (u_char *)&param.nss);

	if (!param.pkt_cnt)
		param.pkt_cnt = 0x8fffffff;

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_stat.tx_cnt,
			(u_int32)param.pkt_cnt, param.band_idx);
	CONFIG_SET_PARAM(serv_test, tx_mode,
			(u_char)param.tx_mode, param.band_idx);
	CONFIG_SET_PARAM(serv_test, mcs,
			(u_char)param.rate, param.band_idx);
	CONFIG_SET_PARAM(serv_test, stbc,
			(u_char)param.stbc, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ldpc,
			(u_char)param.ldpc, param.band_idx);
	if (param.tx_path & TEST_ANT_USER_DEF)
		CONFIG_SET_PARAM(serv_test, tx_ant,
				(u_int32)param.tx_path, param.band_idx);
	CONFIG_SET_PARAM(serv_test, nss,
			(u_char)param.nss, param.band_idx);
	CONFIG_SET_PARAM(serv_test, sgi,
			(u_char)param.gi, param.band_idx);
	CONFIG_SET_PARAM(serv_test, wcid_ref,
			(u_int8)param.wlan_id, param.band_idx);
	CONFIG_SET_PARAM(serv_test, ipg_param.ipg,
			(u_int32)param.aifs, param.band_idx);
	for (ant_idx = 0; ant_idx < TEST_ANT_NUM; ant_idx++)
		CONFIG_SET_PARAM(serv_test, tx_pwr[ant_idx],
			(u_int32)param.pwr, param.band_idx);
	if (mt_serv_submit_tx(serv_test) != SERV_STATUS_SUCCESS)
		goto err_out;

	ret = mt_serv_start_tx(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u, pkt_cnt=0x%4x, phy=%u\n",
		__func__, param.band_idx, param.pkt_cnt, param.tx_mode));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: pwr=%u, mcs=%u, stbc=%u, ldpc=%u\n",
		__func__, param.pwr, param.rate, param.stbc, param.ldpc));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: ibf=%u, ebf=%u, wlan_id=%u, aifs=%u\n",
		__func__, param.ibf, param.ebf, param.wlan_id, param.aifs));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: gi=%u, nss=%u\n",
		__func__, param.gi, param.nss));

	if (param.tx_path & TEST_ANT_USER_DEF)
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: tx_path=0x%x\n",
			__func__, param.tx_path));

err_out:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, &param.ext_id,
			sizeof(param.ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(param.ext_id), ret);

	return ret;
}

static s_int32 hqa_start_rx_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_char own_mac[SERV_MAC_ADDR_LEN];
	u_int32 ext_id = 0, param_num = 0, band_idx = 0, rx_path = 0;
	u_int32 user_idx = 0, tx_mode = 0, ltf_gi = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	sys_ad_zero_mem(own_mac, SERV_MAC_ADDR_LEN);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&param_num);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(FALSE, SERV_MAC_ADDR_LEN,
				&data, (u_char *)&own_mac);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&rx_path);
	get_param_and_shift_buf(TRUE, sizeof(u_int32), &data,
				(u_char *)CONFIG_GET_PADDR(serv_test,
							   mu_rx_aid,
							   band_idx));
	if (param_num > 3) {
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&tx_mode);
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&ltf_gi);
		get_param_and_shift_buf(TRUE, sizeof(u_int32),
					&data, (u_char *)&user_idx);

		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: param num:%d\n", __func__, param_num));
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: \ttx_mode:%d, ltf_gi:%d, user_idx:%d\n",
			__func__, tx_mode, ltf_gi, user_idx));
	}

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);
	CONFIG_SET_PADDR(serv_test, own_mac, own_mac,
			SERV_MAC_ADDR_LEN, band_idx);
#if 0	/* remove rx_path setting while start Rx,
	 * it should be done prior to switch channel
	 */
	CONFIG_SET_PARAM(serv_test, rx_ant, (u_int16)rx_path, band_idx);
#endif
	CONFIG_SET_PARAM(serv_test, tx_mode, tx_mode, band_idx);
	CONFIG_SET_PARAM(serv_test, sgi, ltf_gi, band_idx);
	CONFIG_SET_PARAM(serv_test, user_idx, (u_int16)user_idx, band_idx);

	ret = mt_serv_start_rx(serv_test);

	if (ret)
		return ret;

	if (!serv_test->engine_offload) {
		ret = mt_engine_set_auto_resp(serv_test->test_winfo,
					serv_test->test_op,
					&serv_test->test_config[band_idx],
					band_idx, 1);
		if (ret)
			return ret;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: param num=%u, band_idx=%u\n",
		__func__, param_num, band_idx));
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__func__, SERV_PRINT_MAC(own_mac)));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_stop_tx_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 ext_id = 0, param_num = 0, band_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(param_num),
				&data, (u_char *)&param_num);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_stop_tx(serv_test);
	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

	ret = mt_serv_revert_tx(serv_test);
	if (ret != SERV_STATUS_SUCCESS)
		goto err_out;

err_out:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u\n", __func__, band_idx));
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_stop_rx_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 ext_id = 0, param_num = 0, band_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(param_num),
				&data, (u_char *)&param_num);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	ret = mt_serv_stop_rx(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u\n", __func__, band_idx));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_set_tx_time(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 ext_id = 0, is_tx_time = 0, band_idx = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(band_idx),
				&data, (u_char *)&band_idx);
	get_param_and_shift_buf(TRUE, sizeof(is_tx_time),
				&data, (u_char *)&is_tx_time);

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)band_idx);

	/* 0: use tx length, 1: use tx time */
	if (is_tx_time == 1)
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time_en,
				TRUE, band_idx);
	else {
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time_en,
				FALSE, band_idx);
		/* Reset to 0 when start tx everytime */
		CONFIG_SET_PARAM(serv_test, tx_time_param.pkt_tx_time,
				0, band_idx);
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: band_idx=%u, is_tx_time=%u\n",
		__func__, band_idx, is_tx_time));

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(ext_id), ret);

	return ret;
}

static s_int32 hqa_off_ch_scan(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char *data = hqa_frame->data;
	u_int32 ext_id = 0, dbdc_idx = 0, mntr_ch = 0;
	u_int32 is_aband = 0, mntr_bw = 0, mntr_tx_rx_pth = 0;
	u_int32 scan_mode = 0;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE, ("%s\n", __func__));

	/* Get parameters from command frame */
	/* Data sequences of command frame:
	*  ext_id (4 bytes) + dbdc_idx (4 bytes) + mntr_ch (4 bytes) +
	*  is_aband (4 bytes) + mntr_bw (4 bytes) +
	*  mntr_tx_rx_pth (4 bytes) + scan_mode (4 bytes)
	*/
	get_param_and_shift_buf(TRUE, sizeof(ext_id),
			&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(dbdc_idx),
			&data, (u_char *)&dbdc_idx);
	get_param_and_shift_buf(TRUE, sizeof(mntr_ch),
			&data, (u_char *)&mntr_ch);
	get_param_and_shift_buf(TRUE, sizeof(is_aband),
			&data, (u_char *)&is_aband);
	get_param_and_shift_buf(TRUE, sizeof(mntr_bw),
			&data, (u_char *)&mntr_bw);
	get_param_and_shift_buf(TRUE, sizeof(mntr_tx_rx_pth),
			&data, (u_char *)&mntr_tx_rx_pth);
	get_param_and_shift_buf(TRUE, sizeof(scan_mode),
			&data, (u_char *)&scan_mode);

	switch (mntr_bw) {
	case HQA_BAND_WIDTH_20:
		mntr_bw = TEST_BW_20;
		break;

	case HQA_BAND_WIDTH_40:
		mntr_bw = TEST_BW_40;
		break;

	case HQA_BAND_WIDTH_80:
		mntr_bw = TEST_BW_80;
		break;

	case HQA_BAND_WIDTH_10:
		mntr_bw = TEST_BW_10;
		break;

	case HQA_BAND_WIDTH_5:
		mntr_bw = TEST_BW_5;
		break;

	case HQA_BAND_WIDTH_160:
		mntr_bw = TEST_BW_160C;
		break;

	case HQA_BAND_WIDTH_8080:
		mntr_bw = TEST_BW_160NC;
		break;

	default:
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s: bw 0x%x is out of range\n", __func__, mntr_bw));
		mntr_bw = TEST_BW_20;
		break;
	}

	/* Set parameters */
	SERV_SET_PARAM(serv_test, ctrl_band_idx, (u_char)dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.ext_id,
			(u_int32)ext_id, dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.dbdc_idx,
			(u_int32)dbdc_idx, dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.mntr_ch,
			(u_int32)mntr_ch, dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.is_aband,
			(u_int32)is_aband, dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.mntr_bw,
			(u_int32)mntr_bw, dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.mntr_tx_rx_pth,
			(u_int32)mntr_tx_rx_pth, dbdc_idx);
	CONFIG_SET_PARAM(serv_test, off_ch_param.scan_mode,
			(u_int32)scan_mode, dbdc_idx);

	ret = mt_serv_set_off_ch_scan(serv_test);

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2, (u_char *) &ext_id,
			sizeof(ext_id));
	update_hqa_frame(hqa_frame, 2 + sizeof(ext_id), ret);

	return ret;
}

#ifdef TXBF_SUPPORT
static s_int32 hqa_get_ibf_status_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  *data = hqa_frame->data;
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_char  *txbf_info = CONFIG_GET_PARAM(serv_test,
						txbf_info,
						ctrl_band_idx);
	u_int16 hqa_len;
	u_int32 oper_mode;
	u_int32 ext_id = 0;
	u_int32 status = 0;


	get_param_and_shift_buf(TRUE, sizeof(ext_id), &data, (u_char *)&ext_id);

	/* Get calibration status */
	status = CONFIG_GET_PARAM(serv_test, iBFCalStatus, ctrl_band_idx);

	/* Update op mode */
	oper_mode = CONFIG_GET_PARAM(serv_test, op_mode, ctrl_band_idx);
	oper_mode &= ~fTEST_IN_BF;
	CONFIG_SET_PARAM(serv_test, op_mode, oper_mode, ctrl_band_idx);

	/* Free memory */
	if (txbf_info) {
		sys_ad_free_mem(txbf_info);
		txbf_info = NULL;
	}

	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	status = SERV_OS_HTONL(status);
	sys_ad_move_mem((hqa_frame->data + hqa_len), &status, sizeof(status));
	hqa_len += sizeof(status);
	update_hqa_frame(hqa_frame, hqa_len, ret);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s: HQA iBF phase cal ID = %d and status = %d\n",
		__func__, ext_id, status));

	return ret;
}


static s_int32 hqa_set_ibf_value_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  *data = hqa_frame->data;
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_char  *txbf_info = CONFIG_GET_PARAM(serv_test,
						txbf_info, ctrl_band_idx);
	u_int16 hqa_len;
	u_int32 oper_mode;
	u_int32 ext_id = 0;
	u_int32 action = 0;
	u_int32 arg[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	char    *cmd;


	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s: Band index = %d\n",
		__func__, ctrl_band_idx));

	txbf_info = NULL;

	ret = sys_ad_alloc_mem((u_char **)&cmd,
				sizeof(char) * (HQA_BF_STR_SIZE));
	if (ret != SERV_STATUS_SUCCESS) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("CMD can't be created!!\n"));
		goto HQA_IBF_CMD_FAIL;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s: cmd ptr = 0x%p\n",
		__func__, &cmd));

	sys_ad_zero_mem(cmd, sizeof(char) * (HQA_BF_STR_SIZE));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&action);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[0]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[1]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[2]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[3]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[4]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[5]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[6]);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&arg[7]);

	switch (action) {
	case HQA_TXBF_INIT:
		sprintf(cmd, "%d", (u_char)arg[0]);
		mt_agent_set_txbf_dut_init_proc(serv_test, cmd);
		break;

	case HQA_CHANNEL:
		if (arg[1] == 1)
			sprintf(cmd, "%d:1", (u_char)arg[0]);
		else
			sprintf(cmd, "%d", (u_char)arg[0]);

		mt_agent_set_channel(serv_test, cmd);
		break;

	case HQA_TX_MCS:
		CONFIG_SET_PARAM(serv_test, mcs, (u_char)arg[0], ctrl_band_idx);
		break;

	case HQA_TX_POW0:
		sprintf(cmd, "%d", (u_char)arg[0]);
		mt_agent_set_pwr(serv_test, cmd);
		break;

	case HQA_TX_ANT:
		sprintf(cmd, "%d", (u_char)arg[0]);
		mt_agent_set_txant(serv_test, cmd);
		break;

	case HQA_RX_FRAME:
		mt_serv_stop_tx(serv_test);
		mt_serv_start_rx(serv_test);
		break;

	case HQA_RX_ANT:
		sprintf(cmd, "%d", (u_char)arg[0]);
		mt_agent_set_rxant(serv_test, cmd);
		break;

	case HQA_TXBF_LNA_GAIN:
		sprintf(cmd, "%d", (u_char)arg[0]);
		mt_agent_set_txbf_lna_gain(serv_test, cmd);
		break;

	case HQA_IBF_PHASE_COMP:
		/* BW:DBDC idx:Group:Read from E2P:Dis compensation */
		sprintf(cmd, "%02x:%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3], arg[4]);
		ret = mt_agent_set_ibf_phase_comp(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_IBF_PHASE_COMP is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_TX:
		arg[2] = 0; /* for test purpose */

		/* fgBf:WLAN idx:Txcnt */
		sprintf(cmd, "%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3]);
		ret = mt_agent_set_tx_pkt_with_ibf(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_IBF_TX is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_PROF_UPDATE:
		/* Pfmu idx:Nr:Nc */
		sprintf(cmd, "%02x:%02x:%02x", arg[0], arg[1], arg[2]);
		ret = mt_agent_set_ibf_profile_update(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_IBF_PROF_UPDATE is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		sys_ad_zero_mem(cmd, sizeof(char) * (HQA_BF_STR_SIZE));

		/* Wlan Id:EBf:IBf:Mu:PhaseCalFlg */
		sprintf(cmd, "01:00:01:00:01");
		ret = mt_agent_set_txbf_tx_apply(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" IBF flag setting in WTBL is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_EBF_PROF_UPDATE:
		/* Pfmu idx:Nr:Nc */
		sprintf(cmd, "%02x:%02x:%02x", arg[0], arg[1], arg[2]);
		ret = mt_agent_set_ebf_profile_update(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_EBF_PROF_UPDATE is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_INST_CAL:
		arg[4] = 0; /* for test purpose */

		/* Update op mode */
		oper_mode = CONFIG_GET_PARAM(serv_test, op_mode, ctrl_band_idx);
		oper_mode |= fTEST_IN_BF;
		CONFIG_SET_PARAM(serv_test, op_mode, oper_mode, ctrl_band_idx);

		/* Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level */
		sprintf(cmd, "%02x:%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3], arg[4]);
		ret = mt_agent_set_ibf_inst_cal(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_IBF_GD_CAL is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_IBF_INST_VERIFY:
		oper_mode = CONFIG_GET_PARAM(serv_test, op_mode, ctrl_band_idx);
		oper_mode |= fTEST_IN_BF;
		CONFIG_SET_PARAM(serv_test, op_mode, oper_mode, ctrl_band_idx);

		arg[3] = 4; /* iBF phase verification with instrument */
		arg[4] = 1; /* Force LNA gain is middle gain */

		/* Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level */
		sprintf(cmd, "%02x:%02x:%02x:%02x:%02x",
				arg[0], arg[1], arg[2], arg[3], arg[4]);
		ret = mt_agent_set_ibf_inst_cal(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_IBF_INST_VERIFY is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	case HQA_TXBF_GD_INIT:
		break;

	case HQA_IBF_PHASE_E2P_UPDATE:
		/* Group idx:fgSX2:E2P update type */
		sprintf(cmd, "%02x:%02x:%02x", arg[0], arg[1], arg[2]);
		ret = mt_agent_set_ibf_phase_cal_e2p_update(serv_test, cmd);

		if (ret != SERV_STATUS_SUCCESS) {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
				(" ATE_IBF_PHASE_E2P_UPDATE is failed!!\n"));
			goto HQA_IBF_CMD_FAIL;
		}

		break;

	default:
		break;
	}

HQA_IBF_CMD_FAIL:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	update_hqa_frame(hqa_frame, hqa_len, ret);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: End of hqa_set_ibf_value_ext()!!\n", __func__));

	if (cmd)
		sys_ad_free_mem(cmd);

	return ret;
}


static s_int32 hqa_update_ibf_chan_prof_update_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  *data = hqa_frame->data;
	u_int16 hqa_len;
	u_int32 ext_id = 0;
	u_int32 pfmu_idx, subcarr_idx, fg_final_data;
	s_int32 h11, angle_h11, h21, angle_h21, h31, angle_h31, h41, angle_h41;
	char    *cmd;


	ret = sys_ad_alloc_mem((u_char **)&cmd,
				sizeof(char) * (HQA_BF_STR_SIZE));
	if (ret != SERV_STATUS_SUCCESS) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("CMD can't be created!!\n"));
		goto HQA_PROFILE_UPDATE_FAIL;
	}

	sys_ad_zero_mem(cmd, sizeof(char) * (HQA_BF_STR_SIZE));

	get_param_and_shift_buf(TRUE, sizeof(ext_id),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&pfmu_idx);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&subcarr_idx);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&fg_final_data);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&h11);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&angle_h11);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&h21);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&angle_h21);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&h31);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&angle_h31);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&h41);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&angle_h41);

	sprintf(cmd, "%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x",
	pfmu_idx, subcarr_idx, fg_final_data, h11, angle_h11, h21, angle_h21,
			h31, angle_h31, h41, angle_h41);
	ret = mt_agent_set_txbf_chan_profile_update(serv_test, cmd);

	if (ret == FALSE) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			(" hqa_update_ibf_chan_prof_update_ext is failed!!\n"));
		goto HQA_PROFILE_UPDATE_FAIL;
	}

HQA_PROFILE_UPDATE_FAIL:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	update_hqa_frame(hqa_frame, hqa_len, ret);

	if (cmd)
		sys_ad_free_mem(cmd);

	return ret;
}


static s_int32 hqa_update_ibf_chan_prof_update_all_ext(
	struct service_test *serv_test, struct hqa_frame *hqa_frame)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  *data = hqa_frame->data;
	u_int16 hqa_len;
	u_int32 ext_id = 0;
	u_int32 pfmu_idx, temp;

	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&ext_id);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&pfmu_idx);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&temp);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&temp);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&temp);
	get_param_and_shift_buf(TRUE, sizeof(u_int32),
				&data, (u_char *)&temp);

	ret = mt_serv_set_txbf_profile_data_Write_20m_all(
						serv_test, pfmu_idx, data);
	if (ret != SERV_STATUS_SUCCESS) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
		(" hqa_update_ibf_chan_prof_update_all_ext is failed!!\n"));
		goto HQA_PROFILE_UPDATE_FAIL;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: str:%d\n", __func__, pfmu_idx));

HQA_PROFILE_UPDATE_FAIL:
	/* Update hqa_frame with response: status (2 bytes) */
	sys_ad_move_mem(hqa_frame->data + 2,
			(u_char *) &ext_id, sizeof(ext_id));
	hqa_len = 2 + sizeof(ext_id);
	update_hqa_frame(hqa_frame, hqa_len, ret);

	return ret;
}


s_int32 mt_agent_set_ibf_phase_cal_e2p_update(
	struct service_test *serv_test, u_char *arg)
{
	u_int32  i;
	u_char   group_idx, update_type, *value;
	u_int32  buf[3];
	boolean  fgSx2;
	s_int32  ret = SERV_STATUS_SUCCESS;


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 8) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
					value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	group_idx   = (u_char)buf[0];
	fgSx2       = (u_char)buf[1];
	update_type = (u_char)buf[2];

	ret = mt_serv_set_ibf_phase_cal_e2p_update(
						serv_test,
						group_idx,
						fgSx2,
						update_type);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_lna_gain(
	struct service_test *serv_test,
	u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_int32 lna_gain;


	if (kstrtol(arg, 10, (long *)&lna_gain) == 0)
		ret = mt_serv_set_txbf_lna_gain(serv_test, (u_char)lna_gain);
	else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}


s_int32 mt_agent_set_txbf_da(
	struct service_test *serv_test, u_char *arg)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	u_int16 wcid = 1;
	u_char *addr = NULL;
	u_int32 Octet;
	u_long  buf[6];
	char   *mac_tok = NULL, *dash_ptr = NULL, *mac_str = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: Da = %s\n", __func__, arg));

	dash_ptr = strstr(arg, "-");
	if (dash_ptr) {
		mac_str = dash_ptr+1;
		*dash_ptr = '\0';
		/* sscanf(arg, "%d", &Octet); */
		ret = kstrtouint(arg, 10, &Octet);
		if (ret) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
		wcid = Octet;
	} else
		mac_str = arg;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(mac_str) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	addr = (u_char *)CONFIG_GET_PADDR(serv_test, addr1[wcid-1], ctrl_band_idx);

	for (Octet = 0, mac_tok = agent_trtok(arg, ":");
			mac_tok; mac_tok = agent_trtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(mac_tok) != 2) ||
			(!serv_isxdigit(*mac_tok)) ||
			(!serv_isxdigit(*(mac_tok + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_LEN;
			goto err_out;
		}

		if (kstrtol(mac_tok, 16, (u_long *)&buf[Octet])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		addr[Octet] = (u_char)buf[Octet];
		Octet++;
	}

	/* sanity check */
	if (Octet != SERV_MAC_ADDR_LEN)
		ret = SERV_STATUS_AGENT_INVALID_LEN;

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_sa(
	struct service_test *serv_test, u_char *addr, u_char *arg)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	char    *value;
	s_int32 Octet;
	u_long  buf[6];
	s_int32 ret = SERV_STATUS_SUCCESS;


	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: Sa = %s\n", __func__, arg));

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	addr = (u_char *)CONFIG_GET_PADDR(serv_test, addr3[0], ctrl_band_idx);

	for (Octet = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[Octet])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		addr[Octet] = (u_char)buf[Octet];
		Octet++;
	}

	/* sanity check */
	if (Octet != SERV_MAC_ADDR_LEN) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	ret = mt_serv_set_txbf_sa(serv_test, addr);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_bssid(
	struct service_test *serv_test, u_char *arg)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	char    *value;
	s_int32 Octet;
	u_long  buf[6];
	u_char  *addr = NULL;
	s_int32 ret = SERV_STATUS_SUCCESS;


	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: BSSID = %s\n", __func__, arg));

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_LEN;
		goto err_out;
	}

	addr = (u_char *)CONFIG_GET_PADDR(serv_test, addr2[0], ctrl_band_idx);

	for (Octet = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		/* sanity check */
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_LEN;
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[Octet])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		addr[Octet] = (u_char)buf[Octet];
		Octet++;
	}

	/* sanity check */
	if (Octet != SERV_MAC_ADDR_LEN)
		ret = SERV_STATUS_AGENT_INVALID_LEN;

err_out:
	return ret;
}


s_int32 mt_agent_set_bss_info(
	struct service_test *serv_test, u_char *arg)
{
	u_char  bssid[6];
	u_long  buf[6];
	char    *value;
	u_char i;
	s_int32 ret = SERV_STATUS_SUCCESS;


	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		bssid[i] = (u_char)buf[i];
		i++;
	}

	ret = mt_serv_set_bss_info(serv_test, bssid);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_tx_apply(
	struct service_test *serv_test, u_char *arg)
{
	char	*value;
	u_long	buf[5];
	u_char  in_arg[5];
	s_int32 ret = SERV_STATUS_SUCCESS;
	s_int32	i;


	if (strlen(arg) != 14) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		in_arg[i] = (u_char)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_tx_apply(serv_test, in_arg);

err_out:
	return ret;
}


s_int32 mt_agent_set_ibf_phase_comp(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  bw,  group_idx,   dbdc_band_idx, *value;
	u_long  buf[6];
	boolean fg_read_from_e2p, fg_dis_comp,   fg_bw_160nc, fg_jp_band;

	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if ((strlen(arg) != 14) && (strlen(arg) != 17)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	bw               = (u_char)buf[0];
	dbdc_band_idx    = (u_char)buf[1];
	group_idx        = (u_char)buf[2];
	fg_read_from_e2p = (u_char)buf[3];
	fg_dis_comp      = (u_char)buf[4];
	fg_bw_160nc     = (i == 6) ? (u_char)buf[4] : 0;
	fg_jp_band      = (group_idx == 1) ? 1 : 0;

	ret = mt_serv_set_ibf_phase_comp(serv_test,
					bw,
					fg_jp_band,
					dbdc_band_idx,
					group_idx,
					fg_read_from_e2p,
					fg_dis_comp);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_profile_tag_read(
	struct service_test *serv_test, u_char *arg)
{
	u_char  pf_idx;
	boolean fg_bfer;
	u_long  buf[2];
	char	*value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char	i;


	if (strlen(arg) != 5) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pf_idx  = (u_char)buf[0];
	fg_bfer = (u_char)buf[1];

	ret = mt_serv_set_txbf_profile_tag_read(serv_test, pf_idx, fg_bfer);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_profile_tag_write(
	struct service_test *serv_test, u_char *arg)
{
	u_long prf_idx;


	if (kstrtol(arg, 10, (u_long *)&prf_idx) == 0)
		return mt_serv_set_txbf_profile_tag_write(
							serv_test,
							(u_char)prf_idx);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_profile_tag_invalid(
	struct service_test *serv_test, u_char *arg)
{
	u_long fg_invalid;


	if (kstrtol(arg, 10, (u_long *)&fg_invalid) == 0)
		return mt_serv_set_txbf_profile_tag_invalid(
							serv_test,
							(u_char)fg_invalid);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_profile_tag_idx(
	struct service_test *serv_test, u_char *arg)
{
	u_long profile_idx;


	if (kstrtol(arg, 10, (u_long *)&profile_idx) == 0)
		return mt_serv_set_txbf_profile_tag_idx(
							serv_test,
							(u_char)profile_idx);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_bf_type(
	struct service_test *serv_test, u_char *arg)
{
	u_long bf_type;


	if (kstrtol(arg, 10, (u_long *)&bf_type) == 0)
		return mt_serv_set_txbf_pfmu_tag_bf_type(
							serv_test,
							(u_char)bf_type);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_dbw(
	struct service_test *serv_test, u_char *arg)
{
	u_long dbw;


	if (kstrtol(arg, 10, (u_long *)&dbw) == 0)
		return mt_serv_set_txbf_pfmu_tag_dbw(
						serv_test,
						(u_char)dbw);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_sumu(
	struct service_test *serv_test, u_char *arg)
{
	u_long su_mu;


	if (kstrtol(arg, 10, (u_long *)&su_mu) == 0)
		return mt_serv_set_txbf_pfmu_tag_sumu(
						serv_test,
						(u_char)su_mu);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_mem(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[8];
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  pfmu_mem_col[4], pfmu_mem_row[4];


	/* mem col0:row0:col1:row1:col2:row2:col3:row3 */
	if (strlen(arg) != 23) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pfmu_mem_col[0] = (u_char)buf[0];
	pfmu_mem_row[0] = (u_char)buf[1];
	pfmu_mem_col[1] = (u_char)buf[2];
	pfmu_mem_row[1] = (u_char)buf[3];
	pfmu_mem_col[2] = (u_char)buf[4];
	pfmu_mem_row[2] = (u_char)buf[5];
	pfmu_mem_col[3] = (u_char)buf[6];
	pfmu_mem_row[3] = (u_char)buf[7];

	ret = mt_serv_set_txbf_pfmu_tag_mem(
				serv_test,
				pfmu_mem_row,
				pfmu_mem_col);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_matrix(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[6];
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  nr, nc, ng, lm, cb, he;


	/* nrow:nol:ng:LM:CodeBook:HtcExist */
	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	nr = (u_char)buf[0];
	nc = (u_char)buf[1];
	ng = (u_char)buf[2];
	lm = (u_char)buf[3];
	cb = (u_char)buf[4];
	he = (u_char)buf[5];

	ret = mt_serv_set_txbf_pfmu_tag_matrix(
					serv_test,
					nr,
					nc,
					ng,
					lm,
					cb,
					he);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_snr(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[8];
	char    snr[8];
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;


	if ((strlen(arg) != 11) && (strlen(arg) != 23)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		snr[i] = (u_char)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_pfmu_tag_snr(serv_test, snr);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_smart_ant(
	struct service_test *serv_test, u_char *arg)
{
	u_long SmartAnt;


	if (kstrtol(arg, 10, (u_long *)&SmartAnt) == 0)
		return mt_serv_set_txbf_pfmu_tag_smart_ant(
							serv_test,
							SmartAnt);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_se_idx(
	struct service_test *serv_test, u_char *arg)
{
	u_long se_idx;


	if (kstrtol(arg, 10, (u_long *)&se_idx) == 0)
		return mt_serv_set_txbf_pfmu_tag_se_idx(
							serv_test,
							se_idx);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_rmsd_thrd(
	struct service_test *serv_test, u_char *arg)
{
	u_long rmsd_thrd;


	if (kstrtol(arg, 10, (u_long *)&rmsd_thrd) == 0)
		return mt_serv_set_txbf_pfmu_tag_rmsd_thrd(
							serv_test,
							(u_char)rmsd_thrd);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_mcs_thrd(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[6];
	char    *value;
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  mcs_lss[3], mcs_sss[3];


	if (strlen(arg) != 17) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	mcs_lss[0] = (u_char)buf[0];
	mcs_sss[0] = (u_char)buf[1];
	mcs_lss[1] = (u_char)buf[2];
	mcs_sss[1] = (u_char)buf[3];
	mcs_lss[2] = (u_char)buf[4];
	mcs_sss[2] = (u_char)buf[5];

	ret = mt_serv_set_txbf_pfmu_tag_mcs_thrd(
						serv_test,
						mcs_lss,
						mcs_sss);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_pfmu_tag_time_out(
	struct service_test *serv_test, u_char *arg)
{
	u_long time_out;


	if (kstrtol(arg, 10, (u_long *)&time_out) == 0)
		return mt_serv_set_txbf_pfmu_tag_time_out(
							serv_test,
							(u_char)time_out);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_desired_bw(
	struct service_test *serv_test, u_char *arg)
{
	u_long desired_bw;


	if (kstrtol(arg, 10, (u_long *)&desired_bw) == 0)
		return mt_serv_set_txbf_pfmu_tag_desired_bw(
							serv_test,
							(u_char)desired_bw);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_desired_nr(
	struct service_test *serv_test, u_char *arg)
{
	u_long desired_nr;


	if (kstrtol(arg, 10, (u_long *)&desired_nr) == 0)
		return mt_serv_set_txbf_pfmu_tag_desired_nr(
							serv_test,
							(u_char)desired_nr);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_tag_desired_nc(
	struct service_test *serv_test, u_char *arg)
{
	u_long desired_nc;


	if (kstrtol(arg, 10, (u_long *)&desired_nc) == 0)
		return mt_serv_set_txbf_pfmu_tag_desired_nc(
							serv_test,
							(u_char)desired_nc);
	else
		return SERV_STATUS_AGENT_INVALID_PARAM;
}


s_int32 mt_agent_set_txbf_pfmu_data_write(
	struct service_test *serv_test, u_char *arg)
{
	u_long  buf[18];
	u_int16 pfmu_data[18];
	s_int32 ret = SERV_STATUS_SUCCESS;
	char    *value;
	u_char  i;


	sys_ad_zero_mem(buf, 36);

	/* Profile Select : Subcarrier Select */
	if (strlen(arg) != 60) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":"), i++) {
		if ((!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		pfmu_data[i] = (u_int16)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_pfmu_data_write(serv_test, pfmu_data);

err_out:
	return ret;
}


s_int32 mt_agent_set_sta_rec_bf_update(
	struct service_test *serv_test, u_char *arg)
{
	return mt_serv_set_sta_rec_bf_update(serv_test, arg);
}


s_int32 mt_agent_set_sta_rec_bf_read(
	struct service_test *serv_test, u_char *arg)
{
	return mt_serv_set_sta_rec_bf_read(serv_test, arg);
}


s_int32 mt_agent_set_manual_assoc(
	struct service_test *serv_test, u_char *arg)
{
	return mt_serv_set_manual_assoc(serv_test, arg);
}


s_int32 mt_agent_set_tx_pkt_with_ibf(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  wlan_idx, tx_cnt, *value;
	u_long  buf[4];
	boolean fg_bf, fg_tx_param_update;


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 11) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	fg_bf    = (u_char)buf[0];
	wlan_idx = (u_char)buf[1];
	tx_cnt   = (u_char)buf[2];
	fg_tx_param_update = (u_char)buf[3];

	ret = mt_serv_set_tx_pkt_with_ibf(
					serv_test,
					wlan_idx,
					tx_cnt,
					fg_bf,
					fg_tx_param_update);

err_out:
	return ret;
}


s_int32 mt_agent_set_ibf_profile_update(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  nc, pfmu_idx, *value;
	u_long  buf[3];


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 8) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pfmu_idx = (u_char)buf[0];
	nc       = (u_char)buf[2];
	ret = mt_serv_set_ibf_profile_update(serv_test, pfmu_idx, nc);

err_out:
	return ret;
}


s_int32 mt_agent_set_ebf_profile_update(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_char  i;
	u_char  nc, pfmu_idx, *value;
	u_long  buf[3];


	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 8) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	pfmu_idx = (u_char)buf[0];
	nc       = (u_char)buf[2];
	ret = mt_serv_set_ebf_profile_update(serv_test, pfmu_idx, nc);

err_out:
	return ret;
}


s_int32 mt_agent_set_ibf_inst_cal(
	struct service_test *serv_test, u_char *arg)
{
	u_char   i;
	u_char   group_idx, group_l_m_h, *value;
	u_long   buf[5];
	boolean  fg_sx2;
	u_char   phase_cal, phase_lna_gain_level;
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 14) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) != 2) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i++])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}
	}

	group_idx            = (u_char)buf[0];
	group_l_m_h          = (u_char)buf[1];
	fg_sx2               = (u_char)buf[2];
	phase_cal            = (u_char)buf[3];
	phase_lna_gain_level = (u_char)buf[4];

	ret = mt_serv_set_ibf_inst_cal(
				serv_test,
				group_idx,
				group_l_m_h,
				fg_sx2,
				phase_cal,
				phase_lna_gain_level);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_chan_profile_update(
	struct service_test *serv_test, u_char *arg)
{
	u_char  i;
	u_char  *value;
	u_long  buf[11] = {0};
	u_int16 pfmu_data[11] = {0};
	s_int32 ret = SERV_STATUS_SUCCESS;

	if (arg == NULL) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	if (strlen(arg) != 43) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	for (i = 0, value = agent_trtok(arg, ":");
			value; value = agent_trtok(NULL, ":")) {
		if ((strlen(value) > 3) ||
			(!serv_isxdigit(*value)) ||
			(!serv_isxdigit(*(value + 1)))) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;  /*Invalid*/
			goto err_out;
		}

		if (kstrtol(value, 16, (u_long *)&buf[i])) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		pfmu_data[i] = (u_int16)buf[i];
		i++;
	}

	ret = mt_serv_set_txbf_chan_profile_update(serv_test, pfmu_data);

err_out:
	return ret;
}


s_int32 mt_agent_set_txbf_dut_init_proc(
	struct service_test *serv_test, u_char *arg)
{
	u_char  ctrl_band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);
	struct  test_operation *op = serv_test->test_op;
	struct  test_wlan_info *winfos = serv_test->test_winfo;
	u_long  in_value;
	boolean fg_dbdc;
	char    cmdStr[24], addr[6];
	u_char  tx_path = 0;
	s_int32 ret = SERV_STATUS_SUCCESS;


	if (kstrtol(arg, 10, (u_long *)&in_value)) {
		ret = SERV_STATUS_AGENT_INVALID_PARAM;
		goto err_out;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s: cmd ptr = 0x%p\n",
		__func__, &arg));

	fg_dbdc = (boolean)(in_value & 1);

	/* set ATEDA=00:11:11:11:11:11 */
	snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x11, 0x11, 0x11, 0x11, 0x11);
	mt_agent_set_txbf_da(serv_test, cmdStr);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: mt_agent_set_txbf_da()!!\n", __func__));

	/* set ATESA=00:22:22:22:22:22 */
	snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x22, 0x22, 0x22, 0x22, 0x22);
	mt_agent_set_txbf_sa(serv_test, addr, cmdStr);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: mt_serv_set_device_info()!!\n", __func__));

	/* set ATEBSSID=00:22:22:22:22:22 */
	snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x",
			0x22, 0x22, 0x22, 0x22, 0x22);
	mt_agent_set_txbf_bssid(serv_test, cmdStr);

	mt_agent_set_bss_info(serv_test, cmdStr);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: mt_agent_set_bss_info()!!\n", __func__));

	/* set ATETXMODE=2 */
	CONFIG_SET_PARAM(serv_test, tx_mode, 2, ctrl_band_idx);
	/* set ATETXBW=0 */
	mt_agent_set_bw(serv_test, "0");
	/* set ATETXGI=0 */
	CONFIG_SET_PARAM(serv_test, sgi, 0, ctrl_band_idx);
	/* Set IPG = 100 */
	CONFIG_SET_PARAM(serv_test, ipg_param.ipg, 100, ctrl_band_idx);
	/* Enable i/eBF */
	CONFIG_SET_PARAM(serv_test, ibf, TRUE, ctrl_band_idx);
	CONFIG_SET_PARAM(serv_test, ebf, TRUE, ctrl_band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s: Set txmode, mcs, bw, sgi, ibf and ebf!!\n", __func__));

	if (fg_dbdc) {
		if (ctrl_band_idx == 0) {
			/* set ATETXANT=3 2T */
			mt_agent_set_txant(serv_test, "3");
			/* set ATERXANT=3  2R*/
			mt_agent_set_rxant(serv_test, "3");
		} else {
			/* set ATETXANT=12 2T */
			mt_agent_set_txant(serv_test, "12");
			/* set ATERXANT=12  2R*/
			mt_agent_set_rxant(serv_test, "12");
		}

		/* set ATETXMCS=15 */
		CONFIG_SET_PARAM(serv_test, mcs, 15, ctrl_band_idx);
	} else {
		tx_path = CONFIG_GET_PARAM(serv_test, tx_ant, ctrl_band_idx);
		switch (tx_path) {
		case SERV_TX_PATH_2:
			/* set ATETXANT=3 2T */
			mt_agent_set_txant(serv_test, "3");
			/* set ATERXANT=3  2R*/
			mt_agent_set_rxant(serv_test, "3");

			/* set ATETXMCS=15 */
			CONFIG_SET_PARAM(serv_test, mcs, 15, ctrl_band_idx);
			break;

		case SERV_TX_PATH_3:
			/* set ATETXANT=7 3T */
			mt_agent_set_txant(serv_test, "7");
			/* set ATERXANT=7  3R*/
			mt_agent_set_rxant(serv_test, "7");

			/* set ATETXMCS=23 */
			CONFIG_SET_PARAM(serv_test, mcs, 23, ctrl_band_idx);
			break;

		case SERV_TX_PATH_4:
		default:
			/* set ATETXANT=15 4T */
			mt_agent_set_txant(serv_test, "15");
			/* set ATERXANT=15  4R*/
			mt_agent_set_rxant(serv_test, "15");

			/* set ATETXMCS=31 */
			CONFIG_SET_PARAM(serv_test, mcs, 31, ctrl_band_idx);
			break;
		}
	}

	CONFIG_SET_PARAM(serv_test, fgEBfEverEnabled, 0, ctrl_band_idx);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: Set tx path = %d\n!!\n", __func__, tx_path));

	if (mt_serv_submit_tx(serv_test) != SERV_STATUS_SUCCESS) {
		ret = SERV_STATUS_AGENT_FAIL;
		goto err_out;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: mt_serv_submit_tx()!!\n", __func__));

	/* Enable Tx MAC HW before trigger sounding */
	if (op->op_set_tr_mac) {
		ret = op->op_set_tr_mac(
				winfos,
				SERV_TEST_MAC_TX,
				TRUE,
				ctrl_band_idx);
	} else {
		ret = SERV_STATUS_SERV_TEST_NOT_SUPPORTED;
		goto err_out;
	}

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
			 ("%s: op_set_tr_mac()!!\n", __func__));

	/* Init iBF phase calibration */
	if (((in_value >> 4) & 1) == 0)
		ret = mt_serv_set_ibf_inst_cal_init(serv_test);

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_OFF,
		("%s: mt_serv_set_ibf_inst_cal_init()!!\n", __func__));

err_out:
	return ret;
}
#endif /* TXBF_SUPPORT */


static struct hqa_cmd_entry CMD_SET6[] = {
	/* cmd id start from 0x1600 */
	{0x1,	hqa_set_channel_ext},
	{0x2,	hqa_set_txcontent_ext},
	{0x3,	hqa_start_tx_ext},
	{0x4,	hqa_start_rx_ext},
	{0x5,	hqa_stop_tx_ext},
	{0x6,	hqa_stop_rx_ext},
	{0x7,	legacy_function},
#ifdef TXBF_SUPPORT
	{0x8,	hqa_set_ibf_value_ext},
	{0x9,	hqa_get_ibf_status_ext},
	{0xa,	hqa_update_ibf_chan_prof_update_ext},
	{0xb,	legacy_function},
	{0xc,	hqa_update_ibf_chan_prof_update_all_ext},
#endif /* TXBF_SUPPORT */
	{0x26,	hqa_set_tx_time},
	{0x27,	hqa_off_ch_scan}
};

static struct hqa_cmd_table CMD_TABLES[] = {
	{
		CMD_SET0,
		SERV_ARRAY_SIZE(CMD_SET0),
		0x1000,
	}
	,
	{
		CMD_SET1,
		SERV_ARRAY_SIZE(CMD_SET1),
		0x1100,
	}
	,
	{
		CMD_SET2,
		SERV_ARRAY_SIZE(CMD_SET2),
		0x1200,
	}
	,
	{
		CMD_SET3,
		SERV_ARRAY_SIZE(CMD_SET3),
		0x1300,
	}
	,
	{
		CMD_SET4,
		SERV_ARRAY_SIZE(CMD_SET4),
		0x1400,
	}
	,
	{
		CMD_SET5,
		SERV_ARRAY_SIZE(CMD_SET5),
		0x1500,
	}
	,
	{
		CMD_SET6,
		SERV_ARRAY_SIZE(CMD_SET6),
		0x1600,
	}
	,
};

static struct priv_hqa_cmd_id_mapping priv_hqa_cmd_mapping[] = {
	{"OpenAdapter", 0x1000,
	{0} },
	{"CloseAdapter", 0x1001,
	{0} },
	{"DBDCStartTX", 0x1600,
	{4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4} },
	{"DBDCStartRX", 0x1600,
	{4, 4, 4, 6, 4} },
	{"DBDCStopTX", 0x1600,
	{4, 4, 4} },
	{"DBDCStopRX", 0x1600,
	{4, 4, 4} },
	{"DBDCSetChannel", 0x1600,
	{4, 4, 4, 4, 4, 4, 4, 4, 4, 4} },
	{"DBDCSetTXContent", 0x1600,
	{4, 4, 4, 4, 4, 4, 4, 4, 4, 6, 6, 6} },
	{"GetRXStatisticsAll", 0x1506,
	{0} },
	{"CalibrationTestMode", 0x1509,
	{4, 4} },
	{"CapWiFiSpectrum", 0x1580,
	{4, 4, 4, 4, 4, 4, 4, 4, 4, 6} },
};

s_int32 mt_agent_hqa_cmd_string_parser(
	s_int8 *hqa_frame_string, struct hqa_frame *hqa_frame)
{
	s_int8 i = 0, j = 0, pattern_found = 0;
	s_int8 *this_para = NULL;
	s_int32 i4argc = 0;
	s_int8 *apc_argv[AGENT_CFG_ARGV_MAX] = { 0 };
	u_char tmpdata[100] = { 0 };
	s_int8 tmp_mac[SERV_MAC_ADDR_LEN] = { 0 };
	u_int16 tmp_length = 0;
	u_int32 tmp_value = 0;
	u_char *data = NULL;
	u_int16 ret = 0;
	u_int8 parasize = 0;

	for (i = 0; i < sizeof(priv_hqa_cmd_mapping) / sizeof(struct
			priv_hqa_cmd_id_mapping); i++) {
		if (strncasecmp(hqa_frame_string,
			priv_hqa_cmd_mapping[i].cmd_str,
			strlen(priv_hqa_cmd_mapping[i].cmd_str)) == 0) {

			/*Command Found in table*/
			pattern_found = 1;

			/*Parsing the parameters if it has*/
			this_para = strstr(hqa_frame_string, "=");
			if (this_para) {
				/*Roll over "=", get Parameters y,y,y,y...*/
				this_para++;
				agent_cfg_parse_argument(this_para,
				&i4argc, apc_argv);
				/*debug use*/
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
				("argc is %i\n", i4argc));
				for (j = 0 ; j < i4argc; j++) {
					SERV_LOG(SERV_DBG_CAT_TEST,
					SERV_DBG_LVL_TRACE,
					("argv is %s\n", apc_argv[j++]));
				}
			}

			break;
		}
	}

	/*pack the hqa command*/
	if (pattern_found) {
		if (i4argc) {
			data = tmpdata;
			for (j = 0; j < i4argc; j++) {
			parasize = priv_hqa_cmd_mapping[i].para_size[j];
			tmp_length += parasize;

				if (parasize == SERV_MAC_ADDR_LEN) {
					ret = sscanf(apc_argv[j],
					"%x:%x:%x:%x:%x:%x",
					(unsigned int *)&tmp_mac[0],
					(unsigned int *)&tmp_mac[1],
					(unsigned int *)&tmp_mac[2],
					(unsigned int *)&tmp_mac[3],
					(unsigned int *)&tmp_mac[4],
					(unsigned int *)&tmp_mac[5]);

					if (ret)
					set_param_and_shift_buf(FALSE, parasize,
					(u_char *)&tmp_mac, &data);
				} else if (parasize == 4) {
					ret = kstrtou32(apc_argv[j], 0,
					&tmp_value);
					set_param_and_shift_buf(TRUE, parasize,
					(u_char *)&tmp_value, &data);
				} else if (parasize == 2) {
					ret = kstrtou16(apc_argv[j], 0,
					(u_int16 *)&tmp_value);
					set_param_and_shift_buf(TRUE, parasize,
					(u_char *)&tmp_value, &data);
				} else {
					ret = kstrtou8(apc_argv[j], 0,
					(u_int8 *)&tmp_value);
					set_param_and_shift_buf(TRUE, parasize,
					(u_char *)&tmp_value, &data);
				}
			}
		}

		hqa_frame->magic_no = SERV_OS_HTONL(TEST_CMD_MAGIC_NO);
		hqa_frame->type = 0;
		hqa_frame->id = SERV_OS_HTONS(priv_hqa_cmd_mapping[i].cmd_id);
		hqa_frame->length = tmp_length;
		hqa_frame->sequence = 0;
		memcpy(hqa_frame->data, tmpdata, tmp_length);

		/*debug use*/
		data = (u_char *)hqa_frame;
		for (j = 0; j < sizeof(struct hqa_frame)/sizeof(u_char); j++) {
			if (!(j % 16))
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					("\n"));
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					(" 0x%02x ", *data++));
		}

		return SERV_STATUS_SUCCESS;
	} else
		return SERV_STATUS_AGENT_NOT_SUPPORTED;
}

s_int32 mt_agent_hqa_cmd_handler(
	struct service *serv, struct hqa_frame_ctrl *hqa_frame_ctrl)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test;
	u_int16 cmd_id;
	u_int32 table_idx = 0, ext_cmd_id = 0;
	u_int32 magic_no = 0;
	struct hqa_frame *hqa_frame = NULL;

	if (hqa_frame_ctrl->type == 1) {
		ret = mt_agent_hqa_cmd_string_parser(
		hqa_frame_ctrl->hqa_frame_comm.hqa_frame_string,
		hqa_frame_ctrl->hqa_frame_comm.hqa_frame_eth);
		/*can return for debug*/
		/*return ret;*/
		if (ret != SERV_STATUS_SUCCESS)
			return ret;
	}

	hqa_frame = hqa_frame_ctrl->hqa_frame_comm.hqa_frame_eth;

	magic_no = SERV_OS_NTOHL(hqa_frame->magic_no);
	serv_test = (struct service_test *)serv->serv_handle;

	if (magic_no != TEST_CMD_MAGIC_NO)
		return SERV_STATUS_AGENT_INVALID_PARAM;

	hqa_frame->length = SERV_OS_NTOHS(hqa_frame->length);

	cmd_id = SERV_OS_NTOHS(hqa_frame->id);
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_INFO,
		("%s: command id=0x%02x\n", __func__, cmd_id));

	while (table_idx < (sizeof(CMD_TABLES) / sizeof(struct hqa_cmd_table))) {
		if ((cmd_id & 0xff00) == CMD_TABLES[table_idx].cmd_offset) {
			u_int32 cmd_loop = 0;
			struct hqa_cmd_entry *cmd_set = NULL;

			cmd_set = CMD_TABLES[table_idx].cmd_set;
			/* Fix me, should align with other command set */
			if (CMD_TABLES[table_idx].cmd_offset == 0x1600) {
				sys_ad_move_mem(&ext_cmd_id,
						hqa_frame->data,
						sizeof(ext_cmd_id));
				cmd_id = SERV_OS_NTOHL(ext_cmd_id);
			}

			cmd_id &= 0xff;
			while (cmd_loop < CMD_TABLES[table_idx].cmd_set_size) {
				if (cmd_id == cmd_set[cmd_loop].index) {
					ret = cmd_set[cmd_loop].handler(
						serv_test, hqa_frame);

					goto done;
				} else
					cmd_loop++;
			}
		} else
			table_idx++;
	}

done:
	if (cmd_id == TEST_CMD_REQ) {
		hqa_frame->type = TEST_CMD_RSP;
		/* hqa_frame->type = 0x8005; */
	} else
		hqa_frame->type = TEST_CMDRSP;

	return ret;
}

/*****************************************************************************
 *	iwpriv command handler
 *****************************************************************************/

static struct hqa_frame hqa_cmd_frame;

static struct agent_cli_act_handler cli_act_cmds[] = {
	{"ATESTART", mt_serv_start},
	{"APSTOP", mt_serv_start},
	{"ATESTOP", mt_serv_stop},
	{"APSTART", mt_serv_stop},
	{"TXCOMMIT", mt_serv_submit_tx},
	{"TXREVERT", mt_serv_revert_tx},
	{"TXFRAME", mt_serv_start_tx},
	{"TXSTOP", mt_serv_stop_tx},
	{"RXFRAME", mt_serv_start_rx},
	{"RXSTOP", mt_serv_stop_rx},
	{"DNLKCLEAN", mt_serv_dnlk_clean},
	{"DNLK2G", mt_serv_dnlk_2g},
	{"DNLK5G", mt_serv_dnlk_5g},
	{"GROUPREKClean", mt_serv_group_prek_clean},
	{"GROUPREKDump", mt_serv_group_prek_dump},
	{"GROUPREK", mt_serv_group_prek},
	{"DPDClean", mt_serv_dpd_prek_clean},
	{"DPDDump", mt_serv_dpd_prek_dump},
	{"DPD5G", mt_serv_dpd_prek_5g},
	{"DPD2G", mt_serv_dpd_prek_2g},
	{"DPDREK", mt_serv_dpd_prek},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_act(u_char *name, struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct service_test *serv_test = NULL;
	struct agent_cli_act_handler *entry = cli_act_cmds;

	serv_test = (struct service_test *)serv->serv_handle;

	while (strlen(entry->name)) {
		if (strcmp(name, entry->name) == 0)
			ret = entry->handler(serv_test);

		entry++;
	}

	return ret;
}

static struct agent_cli_set_w_handler cli_set_w_cmds[] = {
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_set_w(u_char *name, struct service *serv, u_char *param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	struct agent_cli_set_w_handler *entry = cli_set_w_cmds;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_int16 value = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	if (kstrtol(param, 10, &str2value) == 0) {
		value = str2value;
		value = SERV_OS_HTONS(value);
		sys_ad_move_mem(hqa_cmd->data, &value, sizeof(u_int16));

		while (strlen(entry->name)) {
			if (strcmp(name, entry->name) == 0)
				ret = entry->handler(
				(struct service_test *)serv->serv_handle,
					hqa_cmd);

			entry++;
		}
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}

static struct agent_cli_set_dw_handler cli_set_dw_cmds[] = {
	{"ATETXMCS", hqa_set_rate},
	{"ATETXNSS", hqa_set_nss},
	{"ATENSS", hqa_set_nss},
	{"ATETXSTBC", hqa_set_stbc},
	{"ATETXMODE", hqa_set_preamble},
	{"ATETXGI", hqa_set_short_gi},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_set_dw(u_char *name, struct service *serv, u_char *param)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	struct agent_cli_set_dw_handler *entry = cli_set_dw_cmds;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_int32 value = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));
	if (kstrtol(param, 10, &str2value) == 0) {
		value = str2value;
		value = SERV_OS_HTONL(value);
		sys_ad_move_mem(hqa_cmd->data, &value, sizeof(u_int32));

		while (strlen(entry->name)) {
			if (strcmp(name, entry->name) == 0)
				ret = entry->handler(
				(struct service_test *)serv->serv_handle,
					hqa_cmd);

			entry++;
		}
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}

s_int32 mt_agent_set_bw(struct service_test *serv_test, u_char *arg)
{
	u_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	u_int32 param[2] = {0};
	u_int8 i = 0;
	u_char *value;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_char *data = hqa_cmd->data;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: Bw = %s\n", __func__, arg));

	for (i = 0, value = agent_trtok(arg, ":");
		value;
		value = agent_trtok(NULL, ":"), i++) {
		if (i == 2)
			break;

		if (kstrtol(value, 10, &str2value)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		param[i] = str2value;
	}

	if (param[1] == 0) {
		param[1] = param[0];
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
			("%s: DBW is not specified, alligned with CBW = %d\n",
			 __func__, param[0]));
	}

	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&param[0], &data);
	hqa_set_system_bw(serv_test, hqa_cmd);

	data = hqa_cmd->data;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&param[1], &data);
	hqa_set_per_pkt_bw(serv_test, hqa_cmd);

err_out:
	return ret;
}

s_int32 mt_agent_set_ctrl_band(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	u_int8 value = 0;

	if (kstrtol(arg, 10, &str2value) == 0) {
		value = str2value;
		SERV_SET_PARAM(serv_test, ctrl_band_idx, value);
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}

s_int32 mt_agent_set_pwr(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long input = 0;
	u_int32 value = 0;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_char *data = hqa_cmd->data;

	if (kstrtol(arg, 10, (long *)&input) == 0) {
		/* power */
		value = input;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* band index */
		value = serv_test->ctrl_band_idx;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* channel */
		value = CONFIG_GET_PARAM(serv_test, channel,
					SERV_GET_PARAM(serv_test,
							ctrl_band_idx));
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* channel band */
		value = CONFIG_GET_PARAM(serv_test, ch_band,
					SERV_GET_PARAM(serv_test,
					ctrl_band_idx));
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		/* ant index */
		value = 0;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);

		hqa_set_tx_power_ext(serv_test, hqa_cmd);
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}

s_int32 mt_agent_set_channel(
	struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	u_long str2value = 0;
	u_int32 value = 0;
	u_int32 input[4] = {0};
	u_int8 i = 0;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_char *data = hqa_cmd->data, *tok = NULL;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: control_band_idx:%x, Channel = %s\n",
		__func__, serv_test->ctrl_band_idx, arg));

	for (i = 0, tok = agent_trtok(arg, ":");
		tok;
		tok = agent_trtok(NULL, ":"), i++) {
		if (i == 4)
			break;

		if (kstrtol(tok, 10, &str2value)) {
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
			goto err_out;
		}

		input[i] = str2value;
	}

	/* For backward compatibility */
	if (input[0] >= 36 && input[0] <= 181) {
		if (input[1] == 0) {
			input[1] = 1; /* channel_band 5G as 1*/
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
	("\x1b[41m%s(): 5G Channel:%d, then force Channel_Band:%d !!\x1b[m\n",
				__func__, input[0], input[1]));
		}
	}

	/* ext id */
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* parameters count */
	value = 8;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* band index */
	value = serv_test->ctrl_band_idx;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* central channel index */
	value = input[0];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* 2nd central channel index */
	value = input[3];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* system bandwidth */
	value = CONFIG_GET_PARAM(serv_test, bw,
				serv_test->ctrl_band_idx);
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* data band width */
	value = CONFIG_GET_PARAM(serv_test, per_pkt_bw,
				serv_test->ctrl_band_idx);
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* primary offset */
	value = input[2];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* reason */
	value = 0;
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	/* channel band */
	value = input[1];
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);
	value = 0;
	/* out band frequency */
	set_param_and_shift_buf(TRUE, sizeof(u_int32),
				(u_char *)&value, &data);

	hqa_set_channel_ext(serv_test, hqa_cmd);

err_out:
	return ret;
}

s_int32 mt_agent_set_ru_cli(struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_AGENT_INVALID_PARAM;
	u_char *value = NULL, i = 0, band_idx, input_cnt = 0;
	struct test_ru_info *ru_info = NULL;

	band_idx = SERV_GET_PARAM(serv_test, ctrl_band_idx);

	ru_info = (struct test_ru_info *)CONFIG_GET_PADDR(serv_test,
							ru_info_list[0],
							band_idx);

	if (strlen(arg) > 0) {
		sys_ad_zero_mem(ru_info,
				sizeof(struct test_ru_info)*MAX_MULTI_TX_STA);

		for (i = 0, value = agent_trtok(arg, ":");
			value;
			value = agent_trtok((char *)NULL, ":"), i++) {
			input_cnt = sscanf(value,
					"%4x-%d-%d-%d-%d-%d-%d-%d-%d",
					&ru_info[i].allocation,
					&ru_info[i].aid,
					&ru_info[i].ru_index,
					&ru_info[i].rate,
					&ru_info[i].ldpc,
					&ru_info[i].nss,
					&ru_info[i].start_sp_st,
					&ru_info[i].mpdu_length,
					&ru_info[i].alpha);

			if (strlen(value) > 0 && input_cnt == 9) {
				ru_info[i].valid = TRUE;

				if (ru_info[i].mpdu_length == 0)
					ru_info[i].mpdu_length =
					CONFIG_GET_PARAM(serv_test,
							tx_len,
							band_idx);

				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					 ("%s: segment[%d]: alloc:%04x\n",
					  __func__, (ru_info[i].ru_index & 0x1),
					  ru_info[i].allocation));
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					 ("%s:\t\t\tru_idx:%d, length:%d,\n",
					  __func__, (ru_info[i].ru_index >> 1),
					  ru_info[i].mpdu_length));
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					 ("%s:\t\t\talpha:%d, rate:0x%x,\n",
					  __func__, ru_info[i].alpha,
					  ru_info[i].rate));
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
					 ("%s: \t\t\tldpc:%d, nss:%d\n",
					  __func__, ru_info[i].ldpc,
					  ru_info[i].nss));
			} else {
				SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
					("Invalid format, %s ignored!\n", arg));
				goto err_out;
			}
		}

		ret = SERV_STATUS_SUCCESS;
	}

err_out:
	return ret;
}

s_int32 mt_agent_set_txant(struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_char *data = hqa_cmd->data;
	u_int32 value = 0;
	u_long str2value = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: TX PATH = %s\n", __func__, arg));

	if (!strchr(arg, ':')) {
		hqa_cmd->length = 2*sizeof(u_int32);

		if (kstrtol(arg, 10, &str2value) == 0) {
			value = str2value;
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&value, &data);
			value = SERV_GET_PARAM(serv_test, ctrl_band_idx);
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&value, &data);
			hqa_set_tx_path(serv_test, hqa_cmd);
		} else
			ret = SERV_STATUS_AGENT_INVALID_PARAM;
	} else {
		u_char i = 0, *tok = NULL;
		u_int32 input[2];

		for (i = 0, tok = agent_trtok(arg, ":");
			tok;
			tok = agent_trtok(NULL, ":"), i++) {
			if (i == 2)
				break;

			if (kstrtol(tok, 10, &str2value)) {
				ret = SERV_STATUS_AGENT_INVALID_PARAM;
				goto err_out;
			}

			input[i] = str2value;
		}

		if (input[0])	{/* user specify SPE index */
			input[1] |= TEST_ANT_USER_DEF;

			hqa_cmd->length = 2*sizeof(u_int32);
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&input[1], &data);
			value = SERV_GET_PARAM(serv_test, ctrl_band_idx);
			set_param_and_shift_buf(TRUE, sizeof(u_int32),
						(u_char *)&value, &data);
			hqa_set_tx_path(serv_test, hqa_cmd);
		}
	}

err_out:
	return ret;
}

s_int32 mt_agent_set_rxant(struct service_test *serv_test, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct hqa_frame *hqa_cmd = &hqa_cmd_frame;
	u_char *data = hqa_cmd->data;
	u_int32 value = 0;
	u_long str2value = 0;

	sys_ad_zero_mem(hqa_cmd, sizeof(*hqa_cmd));

	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s: RX PATH = %s\n", __func__, arg));

	hqa_cmd->length = 2*sizeof(u_int32);

	if (kstrtol(arg, 10, (long *)&str2value) == 0) {
		value = str2value;
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);
		value = SERV_GET_PARAM(serv_test, ctrl_band_idx);
		set_param_and_shift_buf(TRUE, sizeof(u_int32),
					(u_char *)&value, &data);

		hqa_set_rx_path(serv_test, hqa_cmd);
	} else
		ret = SERV_STATUS_AGENT_INVALID_PARAM;

	return ret;
}

static struct agent_cli_set_ext_handler cli_set_ext_cmds[] = {
	{"ATECTRLBANDIDX", mt_agent_set_ctrl_band},
	{"ATETXPOW0", mt_agent_set_pwr},
	{"ATECHANNEL", mt_agent_set_channel},
	{"ATETXBW", mt_agent_set_bw},
	{"ATERUINFO", mt_agent_set_ru_cli},
	{"ATETXANT", mt_agent_set_txant},
	{"ATERXANT", mt_agent_set_rxant},
	{"", NULL}	/* the last entry */
};

s_int32 mt_agent_cli_set_ext(u_char *name, struct service *serv, u_char *arg)
{
	s_int32 ret = SERV_STATUS_SUCCESS;
	struct agent_cli_set_ext_handler *entry = cli_set_ext_cmds;

	while (strlen(entry->name)) {
		if (strcmp(name, entry->name) == 0)
			ret = entry->handler(
				(struct service_test *)serv->serv_handle,
				arg);

		entry++;
	}

	return ret;
}

/*****************************************************************************
 *	Service init/exit handler
 *****************************************************************************/
s_int32 mt_agent_init_service(struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	switch (serv->serv_id) {
	case SERV_HANDLE_TEST:
		ret = mt_serv_init_test(
			(struct service_test *)serv->serv_handle);
		break;

	default:
		return SERV_STATUS_AGENT_FAIL;
	}

	return ret;
}

s_int32 mt_agent_exit_service(struct service *serv)
{
	s_int32 ret = SERV_STATUS_SUCCESS;

	switch (serv->serv_id) {
	case SERV_HANDLE_TEST:
		ret = mt_serv_exit_test(
			(struct service_test *)serv->serv_handle);
		break;

	default:
		return SERV_STATUS_AGENT_FAIL;
	}

	return ret;
}
