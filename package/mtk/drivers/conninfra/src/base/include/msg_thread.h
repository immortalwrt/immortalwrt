/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */
/*! \file
*    \brief  Declaration of library functions
*
*    Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
*/

#ifndef _BASE_MSG_THREAD_H_
#define _BASE_MSG_THREAD_H_

#include "osal.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


#define MSG_THREAD_OP_DATA_SIZE   8
#define MSG_THREAD_OP_BUF_SIZE  64

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

struct msg_op_data {
	unsigned int op_id;		/* Event ID */
	unsigned int info_bit;	/* Reserved */
	size_t op_data[MSG_THREAD_OP_DATA_SIZE];	/* OP Data */
};

struct msg_op {
	struct msg_op_data op;
	OSAL_SIGNAL signal;
	int result;
	atomic_t ref_count;
};


struct msg_op_q {
	OSAL_SLEEPABLE_LOCK lock;
	unsigned int write;
	unsigned int read;
	unsigned int size;
	struct msg_op *queue[MSG_THREAD_OP_BUF_SIZE];
};

typedef OSAL_OP_DAT msg_evt_op;
typedef int(*msg_opid_func) (struct msg_op_data *);

struct msg_thread_ctx {
	OSAL_THREAD thread; /* core thread */
	OSAL_EVENT evt;

	struct msg_op_q free_op_q;		/* free op queue */
	struct msg_op_q active_op_q;	/* active op queue */
	struct msg_op op_q_inst[MSG_THREAD_OP_BUF_SIZE];	/* real op instances */
	struct msg_op *cur_op;	/* current op */

	int op_func_size;
	const msg_opid_func *op_func;

	struct osal_op_history op_history;
};


/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

#define MSG_OP_TIMEOUT 20000

int msg_thread_init(struct msg_thread_ctx *ctx, const char *name,
				const msg_opid_func *func, int op_size);
int msg_thread_deinit(struct msg_thread_ctx *ctx);

/* timeout:
 *    0: default value (by MSG_OP_TIMEOUT define)
 *    >0: cutom timeout (ms)
 */
int msg_thread_send(struct msg_thread_ctx *ctx, int opid);
int msg_thread_send_1(struct msg_thread_ctx *ctx, int opid,
						size_t param1);
int msg_thread_send_2(struct msg_thread_ctx *ctx, int opid,
						size_t param1, size_t param2);

int msg_thread_send_wait(struct msg_thread_ctx *ctx, int opid,
						int timeout);
int msg_thread_send_wait_1(struct msg_thread_ctx *ctx, int opid,
							int timeout, size_t param1);
int msg_thread_send_wait_2(struct msg_thread_ctx *ctx, int opid,
							int timeout, size_t param1, size_t param2);
int msg_thread_send_wait_3(struct msg_thread_ctx *ctx, int opid, int timeout, size_t param1,
							size_t param2,size_t param3);

int msg_thread_dump(struct msg_thread_ctx *ctx);

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif				/* _BASE_MSG_THREAD_H_ */
