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

#define pr_fmt(fmt) KBUILD_MODNAME "@(%s:%d) " fmt, __func__, __LINE__
#include <linux/delay.h>
#include "msg_thread.h"

/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

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

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
static void msg_opq_dump(const char *qName, struct msg_op_q *op_q);
static void _msg_opq_dump(const char *qName, struct msg_op_q *op_q);

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#define MSG_OP_SIZE(prb) ((prb)->size)
#define MSG_OP_MASK(prb) (MSG_OP_SIZE(prb) - 1)
#define MSG_OP_COUNT(prb) ((prb)->write - (prb)->read)
#define MSG_OP_FULL(prb) (MSG_OP_COUNT(prb) >= MSG_OP_SIZE(prb))
#define MSG_OP_EMPTY(prb) ((prb)->write == (prb)->read)

#define MSG_OP_INIT(prb, qsize) \
do { \
	(prb)->read = (prb)->write = 0; \
	(prb)->size = (qsize); \
} while (0)

#define MSG_OP_PUT(prb, value) \
do { \
	if (!MSG_OP_FULL(prb)) { \
		(prb)->queue[(prb)->write & MSG_OP_MASK(prb)] = value; \
		++((prb)->write); \
	} \
	else { \
		pr_warn("Message queue is full.\n"); \
	} \
} while (0)

#define MSG_OP_GET(prb, value) \
do { \
	if (!MSG_OP_EMPTY(prb)) { \
		value = (prb)->queue[(prb)->read & MSG_OP_MASK(prb)]; \
		++((prb)->read); \
		if (MSG_OP_EMPTY(prb)) { \
			(prb)->read = (prb)->write = 0; \
		} \
	} \
	else { \
		value = NULL; \
		pr_warn("Message queue is empty.\n"); \
	} \
} while (0)



#if defined(CONFIG_MTK_ENG_BUILD) || defined(CONFIG_MT_ENG_BUILD)
static bool msg_evt_opq_has_op(struct msg_op_q *op_q, struct msg_op *op)
{
	unsigned int rd;
	unsigned int wt;
	struct msg_op *tmp_op;

	rd = op_q->read;
	wt = op_q->write;

	while (rd != wt) {
		tmp_op = op_q->queue[rd & MSG_OP_MASK(op_q)];
		if (op == tmp_op)
			return true;
		rd++;
	}
	return false;
}
#endif

/*
 * Utility functions
 */
static int msg_evt_put_op_to_q(struct msg_op_q *op_q, struct msg_op *op)
{
	int ret;

	if (!op_q || !op) {
		pr_err("invalid input param: pOpQ(0x%p), pLxOp(0x%p)\n", op_q, op);
		return -1;
	}

	ret = osal_lock_sleepable_lock(&op_q->lock);
	if (ret) {
		pr_warn("osal_lock_sleepable_lock iRet(%d)\n", ret);
		return -2;
	}

#if defined(CONFIG_MTK_ENG_BUILD) || defined(CONFIG_MT_ENG_BUILD)
	if (msg_evt_opq_has_op(op_q, op)) {
		pr_err("Op(%p) already exists in queue(%p)\n", op, op_q);
		ret = -3;
	}
#endif

	/* acquire lock success */
	if (!MSG_OP_FULL(op_q))
		MSG_OP_PUT(op_q, op);
	else {
		pr_warn("MSG_OP_FULL(%p -> %p)\n", op, op_q);
		ret = -4;
	}

	osal_unlock_sleepable_lock(&op_q->lock);

	if (ret) {
		//osal_opq_dump("FreeOpQ", &g_conninfra_ctx.rFreeOpQ);
		//osal_opq_dump("ActiveOpQ", &g_conninfra_ctx.rActiveOpQ);
		return ret;
	}
	return 0;
}


/*
 * Utility functions
 */
static struct msg_op *msg_evt_get_op_from_q(struct msg_op_q *op_q)
{
	struct msg_op *op;
	int ret;

	if (op_q == NULL) {
		pr_err("pOpQ = NULL\n");
		return NULL;
	}

	ret = osal_lock_sleepable_lock(&op_q->lock);
	if (ret) {
		pr_err("osal_lock_sleepable_lock iRet(%d)\n", ret);
		return NULL;
	}

	/* acquire lock success */
	MSG_OP_GET(op_q, op);
	osal_unlock_sleepable_lock(&op_q->lock);

	if (op == NULL) {
		pr_warn("MSG_OP_GET(%p) return NULL\n", op_q);
	//osal_opq_dump("FreeOpQ", &g_conninfra_ctx.rFreeOpQ);
		//osal_opq_dump("ActiveOpQ", &g_conninfra_ctx.rActiveOpQ);
	}

	return op;
}

static void _msg_opq_dump(const char *qName, struct msg_op_q *op_q)
{
	/* Line format:
	 * [LogicalIdx(PhysicalIdx)]Address:OpId(Ref)(Result)-Info-OpData0,OpData1,OpData2,OpData3,OpData5_
	 *	[LogicalIdx]	max 10+2=12 chars (decimal)
	 *	(PhysicalIdx)	max 10+2=12 chars (decimal)
	 *	Address:	max 16+1=17 chars (hex)
	 *	OpId		max 10 chars (decimal)
	 *	(Ref)		max 2+2=4 chars (should only be 1 digit, reserve 2 in case of negative number)
	 *	(Result)	max 11+2=13 chars (signed decimal)
	 *	-Info-		max 8+2=10 chars (hex)
	 *	OpData,		max 16+1=17 chars (hex)
	 */
#define OPQ_DUMP_OP_PER_LINE 1
#define OPQ_DUMP_OPDATA_PER_OP 6
#define OPQ_DUMP_OP_BUF_SIZE (12 + 12 + 17 + 10 + 4 + 13 + 10 + (17 * (OPQ_DUMP_OPDATA_PER_OP)) + 1)
#define OPQ_DUMP_LINE_BUF_SIZE ((OPQ_DUMP_OP_BUF_SIZE * OPQ_DUMP_OP_PER_LINE) + 1)
	unsigned int rd;
	unsigned int wt;
	unsigned int idx = 0;
	unsigned int op_data_idx;
	unsigned int buf_idx;
	int printed;
	struct msg_op *op;
	char buf[OPQ_DUMP_LINE_BUF_SIZE];

	rd = op_q->read;
	wt = op_q->write;

	pr_info("%s(%p), sz:%u/%u, rd:%u, wt:%u\n",
			qName, op_q, RB_COUNT(op_q), RB_SIZE(op_q), rd, wt);
	while (rd != wt && idx < RB_SIZE(op_q)) {
		buf_idx = idx % OPQ_DUMP_OP_PER_LINE;
		op = op_q->queue[rd & RB_MASK(op_q)];

		if (buf_idx == 0) {
			printed = 0;
			buf[0] = 0;
		}

		if (op) {
			printed += snprintf(buf + printed, OPQ_DUMP_LINE_BUF_SIZE - printed,
						"[%u(%u)]%p:%u(%d)(%d)-%u-",
						idx,
						(rd & RB_MASK(op_q)),
						op,
						op->op.op_id,
						atomic_read(&op->ref_count),
						op->result,
						op->op.info_bit);
			for (op_data_idx = 0; op_data_idx < OPQ_DUMP_OPDATA_PER_OP; op_data_idx++)
				printed += snprintf(buf + printed, OPQ_DUMP_LINE_BUF_SIZE - printed,
						"%zx,", op->op.op_data[op_data_idx]);
			buf[printed-1] = ' ';
		} else {
			printed += snprintf(buf + printed, OPQ_DUMP_LINE_BUF_SIZE - printed,
						"[%u(%u)]%p ", idx, (rd & RB_MASK(op_q)), op);
		}
		buf[printed++] = ' ';

		if (buf_idx == OPQ_DUMP_OP_PER_LINE - 1  || rd == wt - 1) {
			buf[printed - 1] = 0;
			pr_info("%s\n", buf);
		}
		rd++;
		idx++;
	}
}

void msg_opq_dump(const char *qName, struct msg_op_q *op_q)
{
	int err;

	err = osal_lock_sleepable_lock(&op_q->lock);
	if (err) {
		pr_info("Failed to lock queue (%d)\n", err);
		return;
	}

	_msg_opq_dump(qName, op_q);

	osal_unlock_sleepable_lock(&op_q->lock);
}

/*
 *  msg_evt_thread API
 */

int msg_evt_put_op_to_free_queue(struct msg_thread_ctx *ctx, struct msg_op *op)
{
	if (msg_evt_put_op_to_q(&ctx->free_op_q, op))
		return -1;
	return 0;
}


struct msg_op *msg_evt_get_free_op(struct msg_thread_ctx *ctx)
{
	struct msg_op *op = NULL;

	if (ctx == NULL) {
		pr_warn("ctx is null.\n");
		return op;
	}
	op = msg_evt_get_op_from_q(&ctx->free_op_q);
	if (op)
		osal_memset(op, 0, osal_sizeof(struct msg_op));
	return op;
}

int msg_evt_put_op_to_active(struct msg_thread_ctx *ctx, struct msg_op *op)
{
	P_OSAL_SIGNAL signal = NULL;
	int wait_ret = -1;
	int ret = 0;

	do {
		if (!ctx || !op) {
			pr_err("msg_thread_ctx(0x%p), op(0x%p)\n", ctx, op);
			break;
		}

		signal = &op->signal;
		if (signal->timeoutValue) {
			op->result = -9;
			osal_signal_init(signal);
			atomic_set(&op->ref_count, 1);
		} else
			atomic_set(&op->ref_count, 0);

		/* Increment ref_count by 1 as wmtd thread will hold a reference also,
		 * this must be done here instead of on target thread, because
		 * target thread might not be scheduled until a much later time,
		 * allowing current thread to decrement ref_count at the end of function,
		 * putting op back to free queue before target thread has a chance to process.
		 */
		atomic_inc(&op->ref_count);

		/* put to active Q */
		ret = msg_evt_put_op_to_q(&ctx->active_op_q, op);
		if (ret) {
			pr_warn("put to active queue fail\n");
			atomic_dec(&op->ref_count);
			break;
		}

		/* wake up conninfra_cored */
		osal_trigger_event(&ctx->evt);

		if (signal->timeoutValue == 0) {
			//ret = -1;
			/* Not set timeout, don't wait */
			/* pr_info("[%s] timeout is zero", __func__);*/
			break;
		}

		/* check result */
		wait_ret = osal_wait_for_signal_timeout(signal, &ctx->thread);
		/*pr_info("osal_wait_for_signal_timeout:%d result=[%d]\n",
							wait_ret, op->result);*/

		if (wait_ret == 0)
			pr_warn("opId(%d) completion timeout\n", op->op.op_id);
		else if (op->result)
			pr_info("opId(%d) result:%d\n",
					op->op.op_id, op->result);

		/* op completes, check result */
		ret = op->result;
	} while (0);

	if (op != NULL && signal != NULL && signal->timeoutValue &&
		atomic_dec_and_test(&op->ref_count)) {
		/* put Op back to freeQ */
		msg_evt_put_op_to_free_queue(ctx, op);
	}

	return ret;
}


int msg_thread_send(struct msg_thread_ctx *ctx, int opid)
{
	return msg_thread_send_2(ctx, opid, 0, 0);
}

int msg_thread_send_1(struct msg_thread_ctx *ctx, int opid,
						size_t param1)
{
	return msg_thread_send_2(ctx, opid, param1, 0);
}

int msg_thread_send_2(struct msg_thread_ctx *ctx, int opid,
						size_t param1, size_t param2)
{
	struct msg_op *op = NULL;
	P_OSAL_SIGNAL signal;
	int ret;

	op = msg_evt_get_free_op(ctx);
	if (!op) {
		pr_err("[%s] can't get free op\n", __func__);
		return -1;
	}
	op->op.op_id = opid;
	op->op.op_data[0] = param1;
	op->op.op_data[1] = param2;

	signal = &op->signal;
	//signal->timeoutValue = timeout > 0 ? timeout : MSG_OP_TIMEOUT;
	signal->timeoutValue = 0;
	ret = msg_evt_put_op_to_active(ctx, op);

	return ret;
}

int msg_thread_send_wait(struct msg_thread_ctx *ctx, int opid,
						int timeout)
{
	return msg_thread_send_wait_3(ctx, opid, timeout, 0, 0, 0);
}

int msg_thread_send_wait_1(struct msg_thread_ctx *ctx,
							int opid, int timeout,
							size_t param1)
{
	return msg_thread_send_wait_3(ctx, opid, timeout, param1, 0, 0);
}


int msg_thread_send_wait_2(struct msg_thread_ctx *ctx,
							int opid, int timeout,
							size_t param1,
							size_t param2)
{
	return msg_thread_send_wait_3(ctx, opid, timeout, param1, param2, 0);
}

int msg_thread_send_wait_3(struct msg_thread_ctx *ctx,
							int opid, int timeout,
							size_t param1,
							size_t param2,
							size_t param3)
{
	struct msg_op *op = NULL;
	P_OSAL_SIGNAL signal;
	int ret;

	op = msg_evt_get_free_op(ctx);
	if (!op) {
		pr_err("can't get free op for 0x%x\n", opid);
		return -1;
	}
	op->op.op_id = opid;
	op->op.op_data[0] = param1;
	op->op.op_data[1] = param2;
	op->op.op_data[2] = param3;

	signal = &op->signal;
	signal->timeoutValue = timeout > 0 ? timeout : MSG_OP_TIMEOUT;
	ret = msg_evt_put_op_to_active(ctx, op);
	return ret;

}

void msg_op_history_save(struct osal_op_history *log_history, struct msg_op *op)
{
	struct osal_op_history_entry *entry = NULL;
	struct ring_segment seg;
	int index;
	unsigned long long sec = 0;
	unsigned long usec = 0;
	unsigned long flags;

	if (log_history->queue == NULL)
		return;

	osal_get_local_time(&sec, &usec);

	spin_lock_irqsave(&(log_history->lock), flags);
	RING_OVERWRITE_FOR_EACH(1, seg, &log_history->ring_buffer) {
		index = seg.ring_pt - log_history->ring_buffer.base;
		entry = &log_history->queue[index];
	}

	if (entry == NULL) {
		pr_info("Entry is null, size %d\n",
				RING_SIZE(&log_history->ring_buffer));
		spin_unlock_irqrestore(&(log_history->lock), flags);
		return;
	}

	entry->opbuf_address = op;
	entry->op_id = op->op.op_id;
	entry->opbuf_ref_count = atomic_read(&op->ref_count);
	entry->op_info_bit = op->op.info_bit;
	entry->param_0 = op->op.op_data[0];
	entry->param_1 = op->op.op_data[1];
	entry->param_2 = op->op.op_data[2];
	entry->param_3 = op->op.op_data[3];
	entry->ts = sec;
	entry->usec = usec;
	spin_unlock_irqrestore(&(log_history->lock), flags);
}

unsigned int msg_evt_wait_event_checker(P_OSAL_THREAD thread)
{
	struct msg_thread_ctx *ctx = NULL;

	if (thread) {
		ctx = (struct msg_thread_ctx *) (thread->pThreadData);
		return !MSG_OP_EMPTY(&ctx->active_op_q);
	}
	return 0;
}

int msg_evt_set_current_op(struct msg_thread_ctx *ctx, struct msg_op *op)
{
	ctx->cur_op = op;
	return 0;
}

int msg_evt_opid_handler(struct msg_thread_ctx *ctx, struct msg_op_data *op)
{
	int opid, ret;

	/*sanity check */
	if (op == NULL) {
		pr_warn("null op\n");
		return -1;
	}
	if (ctx == NULL) {
		pr_warn("null evt thread ctx\n");
		return -2;
	}

	opid = op->op_id;

	if (opid >= ctx->op_func_size) {
		pr_err("msg_evt_thread invalid OPID(%d)\n", opid);
		return -3;
	}

	if (ctx->op_func[opid] == NULL) {
		pr_err("null handler (%d)\n", opid);
		return -4;
	}
	ret = (*(ctx->op_func[opid])) (op);
	return ret;
}

static int msg_evt_thread(void *pvData)
{
	struct msg_thread_ctx *ctx = (struct msg_thread_ctx *)pvData;
	P_OSAL_EVENT evt = NULL;
	struct msg_op *op;
	int ret;

	if (ctx == NULL) {
		pr_err("msg_evt_thread (NULL)\n");
		return -1;
	}

	evt = &(ctx->evt);

	for (;;) {
		op = NULL;
		evt->timeoutValue = 0;

		osal_thread_wait_for_event(&ctx->thread, evt, msg_evt_wait_event_checker);

		if (osal_thread_should_stop(&ctx->thread)) {
			pr_info("msg_evt_thread thread should stop now...\n");
			/* TODO: clean up active opQ */
			break;
		}
		/* get Op from activeQ */
		op = msg_evt_get_op_from_q(&ctx->active_op_q);
		if (!op) {
			pr_warn("get op from activeQ fail\n");
			continue;
		}

		/* TODO: save op history */
		msg_op_history_save(&ctx->op_history, op);
		msg_evt_set_current_op(ctx, op);
		ret = msg_evt_opid_handler(ctx, &op->op);
		msg_evt_set_current_op(ctx, NULL);

		if (ret)
			pr_warn("opid (0x%x) failed, ret(%d)\n",
							op->op.op_id, ret);

		if (atomic_dec_and_test(&op->ref_count)) {
			/* msg_evt_free_op(ctx) */
			msg_evt_put_op_to_free_queue(ctx, op);
		} else if (op->signal.timeoutValue) {
			op->result = ret;
			osal_raise_signal(&op->signal);
		}
	}

	pr_debug("msg evt thread exists\n");
	return 0;
}

int msg_thread_dump(struct msg_thread_ctx *ctx)
{
	P_OSAL_THREAD p_thread;
	struct msg_op *cur_op;

	if (ctx == NULL) {
		pr_err("get NULL input\n");
		return 0;
	}
	p_thread = &ctx->thread;
	pr_info("Dump msg_thread_ctx: %s\n", p_thread->threadName);
	cur_op = ctx->cur_op;
	if (cur_op) {
		pr_info("cur_op: %x(%x)-%zx,%zx,%zx,%zx\n",
			cur_op->op.op_id, cur_op->op.info_bit,
			cur_op->op.op_data[0], cur_op->op.op_data[1],
			cur_op->op.op_data[2], cur_op->op.op_data[3]);
	}
	osal_dump_thread_state(p_thread->threadName);
	osal_op_history_print(&(ctx->op_history), p_thread->threadName);
	msg_opq_dump("ActiveOpQ", &ctx->active_op_q);

	return 0;
}

int msg_thread_init(struct msg_thread_ctx *ctx,
		const char *thread_name, const msg_opid_func *funcs,
		int op_size)
{
	int r = 0, i;
	P_OSAL_THREAD p_thread;

	osal_memset(ctx, 0, sizeof(struct msg_thread_ctx));

	ctx->op_func = funcs;
	ctx->op_func_size = op_size;

	/* init thread inst */
	p_thread = &ctx->thread;

	osal_strncpy(p_thread->threadName, thread_name,
			sizeof(p_thread->threadName));

	p_thread->pThreadData = (void *) ctx;
	p_thread->pThreadFunc = (void *) msg_evt_thread;
	r = osal_thread_create(p_thread);

	if (r) {
		pr_err("osal_thread_create(0x%p) fail(%d)\n", p_thread, r);
		return -1;
	}

	osal_event_init(&ctx->evt);
	osal_sleepable_lock_init(&ctx->active_op_q.lock);
	osal_sleepable_lock_init(&ctx->free_op_q.lock);

	/* Initialize op queue */
	MSG_OP_INIT(&ctx->free_op_q, MSG_THREAD_OP_BUF_SIZE);
	MSG_OP_INIT(&ctx->active_op_q, MSG_THREAD_OP_BUF_SIZE);

	/* Put all to free Q */
	for (i = 0; i < MSG_THREAD_OP_BUF_SIZE; i++) {
		osal_signal_init(&(ctx->op_q_inst[i].signal));
		msg_evt_put_op_to_free_queue(ctx, &(ctx->op_q_inst[i]));
	}

	osal_op_history_init(&ctx->op_history, 16);

	r = osal_thread_run(p_thread);
	if (r) {
		pr_err("osal_thread_run(evt_thread 0x%p) fail(%d)\n",
				p_thread, r);
		return -2;
	}
	return r;
}

int msg_thread_deinit(struct msg_thread_ctx *ctx)
{
	int r, i;
	P_OSAL_THREAD p_thraed = &ctx->thread;

	r = osal_thread_stop(p_thraed);
	if (r) {
		pr_err("osal_thread_stop(0x%p) fail(%d)\n", p_thraed, r);
		return -1;
	}

	for (i = 0; i < MSG_THREAD_OP_BUF_SIZE; i++)
		osal_signal_deinit(&(ctx->op_q_inst[i].signal));

	osal_sleepable_lock_deinit(&ctx->free_op_q.lock);
	osal_sleepable_lock_deinit(&ctx->active_op_q.lock);

	r = osal_thread_destroy(p_thraed);
	if (r) {
		pr_err("osal_thread_stop(0x%p) fail(%d)\n", p_thraed, r);
		return -2;
	}

	osal_memset(ctx, 0, sizeof(struct msg_thread_ctx));

	pr_debug("[%s] DONE\n", __func__);
	return 0;
}
