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
 * \brief  Declaration of library functions
 * Any definitions in this file will be shared among GLUE Layer and internal Driver Stack.
 */

#ifndef _OSAL_H_
#define _OSAL_H_

#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/wait.h>
#include "ring.h"
/*******************************************************************************
*                         C O M P I L E R   F L A G S
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

#define OS_BIT_OPS_SUPPORT 1

#ifndef MTK_CONN_BOOL_TRUE
#define MTK_CONN_BOOL_FALSE               ((MTK_CONN_BOOL) 0)
#define MTK_CONN_BOOL_TRUE                ((MTK_CONN_BOOL) 1)
#endif

#define _osal_inline_ inline

#define MAX_THREAD_NAME_LEN 16
#define MAX_HISTORY_NAME_LEN 16
#define OSAL_OP_BUF_SIZE    64


#if (defined(CONFIG_MTK_GMO_RAM_OPTIMIZE) && !defined(CONFIG_MTK_ENG_BUILD))
#define OSAL_OP_DATA_SIZE   8
#else
#define OSAL_OP_DATA_SIZE   32
#endif

#define DBG_LOG_STR_SIZE    256

#define osal_sizeof(x) sizeof(x)

#define osal_array_size(x) ARRAY_SIZE(x)

#ifndef NAME_MAX
#define NAME_MAX 256
#endif

#define WMT_OP_BIT(x) (0x1UL << x)
#define WMT_OP_HIF_BIT WMT_OP_BIT(0)

#define GET_BIT_MASK(value, mask) ((value) & (mask))
#define SET_BIT_MASK(pdest, value, mask) (*(pdest) = (GET_BIT_MASK(*(pdest), ~(mask)) | GET_BIT_MASK(value, mask)))
#define GET_BIT_RANGE(data, end, begin) ((data) & GENMASK(end, begin))
#define SET_BIT_RANGE(pdest, data, end, begin) (SET_BIT_MASK(pdest, data, GENMASK(end, begin)))

#define RB_LATEST(prb) ((prb)->write - 1)
#define RB_SIZE(prb) ((prb)->size)
#define RB_MASK(prb) (RB_SIZE(prb) - 1)
#define RB_COUNT(prb) ((prb)->write - (prb)->read)
#define RB_FULL(prb) (RB_COUNT(prb) >= RB_SIZE(prb))
#define RB_EMPTY(prb) ((prb)->write == (prb)->read)

#define RB_INIT(prb, qsize) \
do { \
	(prb)->read = (prb)->write = 0; \
	(prb)->size = (qsize); \
} while (0)

#define RB_PUT(prb, value) \
do { \
	if (!RB_FULL(prb)) { \
		(prb)->queue[(prb)->write & RB_MASK(prb)] = value; \
		++((prb)->write); \
	} \
	else { \
		pr_warn("RB is full!"); \
	} \
} while (0)

#define RB_GET(prb, value) \
do { \
	if (!RB_EMPTY(prb)) { \
		value = (prb)->queue[(prb)->read & RB_MASK(prb)]; \
		++((prb)->read); \
		if (RB_EMPTY(prb)) { \
			(prb)->read = (prb)->write = 0; \
		} \
	} \
	else { \
		value = NULL; \
		pr_warn("RB is empty!"); \
	} \
} while (0)

#define RB_GET_LATEST(prb, value) \
do { \
	if (!RB_EMPTY(prb)) { \
		value = (prb)->queue[RB_LATEST(prb) & RB_MASK(prb)]; \
		if (RB_EMPTY(prb)) { \
			(prb)->read = (prb)->write = 0; \
		} \
	} \
	else { \
		value = NULL; \
	} \
} while (0)



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

typedef int MTK_CONN_BOOL;

typedef int(*P_COND) (void *);

typedef struct _OSAL_UNSLEEPABLE_LOCK_ {
	spinlock_t lock;
	unsigned long flag;
} OSAL_UNSLEEPABLE_LOCK, *P_OSAL_UNSLEEPABLE_LOCK;

typedef struct _OSAL_SLEEPABLE_LOCK_ {
	struct mutex lock;
} OSAL_SLEEPABLE_LOCK, *P_OSAL_SLEEPABLE_LOCK;

typedef struct _OSAL_SIGNAL_ {
	struct completion comp;
	unsigned int timeoutValue;
	unsigned int timeoutExtension;	/* max number of timeout caused by thread not able to acquire CPU */
} OSAL_SIGNAL, *P_OSAL_SIGNAL;

typedef struct _OSAL_EVENT_ {
	wait_queue_head_t waitQueue;
	unsigned int timeoutValue;
	int waitFlag;

} OSAL_EVENT, *P_OSAL_EVENT;

/* Data collected from sched_entity and sched_statistics */
typedef struct _OSAL_THREAD_SCHEDSTATS_ {
	unsigned long long time;		/* when marked: the profiling start time(ms), when unmarked: total duration(ms) */
	unsigned long long exec;		/* time spent in exec (sum_exec_runtime) */
	unsigned long long runnable;	/* time spent in run-queue while not being scheduled (wait_sum) */
	unsigned long long iowait;		/* time spent waiting for I/O (iowait_sum) */
} OSAL_THREAD_SCHEDSTATS, *P_OSAL_THREAD_SCHEDSTATS;

typedef struct _OSAL_THREAD_ {
	struct task_struct *pThread;
	void *pThreadFunc;
	void *pThreadData;
	char threadName[MAX_THREAD_NAME_LEN];
} OSAL_THREAD, *P_OSAL_THREAD;


typedef struct _OSAL_FIFO_ {
	/*fifo definition */
	void *pFifoBody;
	spinlock_t fifoSpinlock;
	/*fifo operations */
	int (*FifoInit)(struct _OSAL_FIFO_ *pFifo, unsigned char *buf, unsigned int);
	int (*FifoDeInit)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoReset)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoSz)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoAvailSz)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoLen)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoIsEmpty)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoIsFull)(struct _OSAL_FIFO_ *pFifo);
	int (*FifoDataIn)(struct _OSAL_FIFO_ *pFifo, const void *buf, unsigned int len);
	int (*FifoDataOut)(struct _OSAL_FIFO_ *pFifo, void *buf, unsigned int len);
} OSAL_FIFO, *P_OSAL_FIFO;

typedef struct firmware osal_firmware;

typedef struct _OSAL_OP_DAT {
	unsigned int opId;		/* Event ID */
	unsigned int u4InfoBit;	/* Reserved */
	size_t au4OpData[OSAL_OP_DATA_SIZE];	/* OP Data */
} OSAL_OP_DAT, *P_OSAL_OP_DAT;

typedef struct _OSAL_LXOP_ {
	OSAL_OP_DAT op;
	OSAL_SIGNAL signal;
	int result;
	atomic_t ref_count;
} OSAL_OP, *P_OSAL_OP;

typedef struct _OSAL_LXOP_Q {
	OSAL_SLEEPABLE_LOCK sLock;
	unsigned int write;
	unsigned int read;
	unsigned int size;
	P_OSAL_OP queue[OSAL_OP_BUF_SIZE];
} OSAL_OP_Q, *P_OSAL_OP_Q;

typedef struct _OSAL_BIT_OP_VAR_ {
	unsigned long data;
	OSAL_UNSLEEPABLE_LOCK opLock;
} OSAL_BIT_OP_VAR, *P_OSAL_BIT_OP_VAR;

typedef unsigned int (*P_OSAL_EVENT_CHECKER) (P_OSAL_THREAD pThread);

struct osal_op_history_entry {
	void *opbuf_address;
	unsigned int op_id;
	unsigned int opbuf_ref_count;
	unsigned int op_info_bit;
	size_t param_0;
	size_t param_1;
	size_t param_2;
	size_t param_3;
	unsigned long long ts;
	unsigned long usec;
};

struct osal_op_history {
	struct ring ring_buffer;
	struct osal_op_history_entry *queue;
	spinlock_t lock;
	struct ring dump_ring_buffer;
	struct work_struct dump_work;
	unsigned char name[MAX_HISTORY_NAME_LEN];
};

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/
void FlashRead(char *name,unsigned char *value, unsigned long offset, unsigned long size);
void FlashWrite(char *name, unsigned char *p, unsigned long a, unsigned long b);

unsigned int osal_strlen(const char *str);
int osal_strcmp(const char *dst, const char *src);
int osal_strncmp(const char *dst, const char *src, unsigned int len);
char *osal_strcpy(char *dst, const char *src);
char *osal_strncpy(char *dst, const char *src, unsigned int len);
char *osal_strcat(char *dst, const char *src);
char *osal_strncat(char *dst, const char *src, unsigned int len);
char *osal_strchr(const char *str, unsigned char c);
char *osal_strsep(char **str, const char *c);
int osal_strtol(const char *str, unsigned int adecimal, long *res);
char *osal_strstr(char *str1, const char *str2);
char *osal_strnstr(char *str1, const char *str2, int n);

void osal_bug_on(unsigned int val);

int osal_snprintf(char *buf, unsigned int len, const char *fmt, ...);

int osal_sprintf(char *str, const char *format, ...);
void *osal_malloc(unsigned int size);
void osal_free(const void *dst);
void *osal_memset(void *buf, int i, unsigned int len);
void *osal_memcpy(void *dst, const void *src, unsigned int len);
void osal_memcpy_fromio(void *dst, const void *src, unsigned int len);
void osal_memcpy_toio(void *dst, const void *src, unsigned int len);
int osal_memcmp(const void *buf1, const void *buf2, unsigned int len);

int osal_sleep_ms(unsigned int ms);
int osal_udelay(unsigned int us);
int osal_usleep_range(unsigned long min, unsigned long max);

int osal_fifo_init(P_OSAL_FIFO pFifo, unsigned char *buffer, unsigned int size);
void osal_fifo_deinit(P_OSAL_FIFO pFifo);
int osal_fifo_reset(P_OSAL_FIFO pFifo);
unsigned int osal_fifo_in(P_OSAL_FIFO pFifo, unsigned char *buffer,
							unsigned int size);
unsigned int osal_fifo_out(P_OSAL_FIFO pFifo, unsigned char *buffer,
							unsigned int size);
unsigned int osal_fifo_len(P_OSAL_FIFO pFifo);
unsigned int osal_fifo_sz(P_OSAL_FIFO pFifo);
unsigned int osal_fifo_avail(P_OSAL_FIFO pFifo);
unsigned int osal_fifo_is_empty(P_OSAL_FIFO pFifo);
unsigned int osal_fifo_is_full(P_OSAL_FIFO pFifo);

#if defined(CONFIG_PROVE_LOCKING)
#define osal_unsleepable_lock_init(l) { spin_lock_init(&((l)->lock)); }
#else
int osal_unsleepable_lock_init(P_OSAL_UNSLEEPABLE_LOCK);
#endif
int osal_lock_unsleepable_lock(P_OSAL_UNSLEEPABLE_LOCK);
int osal_unlock_unsleepable_lock(P_OSAL_UNSLEEPABLE_LOCK);
int osal_unsleepable_lock_deinit(P_OSAL_UNSLEEPABLE_LOCK);

#if defined(CONFIG_PROVE_LOCKING)
#define osal_sleepable_lock_init(l) { mutex_init(&((l)->lock)); }
#else
int osal_sleepable_lock_init(P_OSAL_SLEEPABLE_LOCK);
#endif
int osal_lock_sleepable_lock(P_OSAL_SLEEPABLE_LOCK);
int osal_unlock_sleepable_lock(P_OSAL_SLEEPABLE_LOCK);
int osal_trylock_sleepable_lock(P_OSAL_SLEEPABLE_LOCK);
int osal_sleepable_lock_deinit(P_OSAL_SLEEPABLE_LOCK);

int osal_signal_init(P_OSAL_SIGNAL);
int osal_wait_for_signal(P_OSAL_SIGNAL);
int osal_wait_for_signal_timeout(P_OSAL_SIGNAL, P_OSAL_THREAD);
int osal_raise_signal(P_OSAL_SIGNAL);
int osal_signal_active_state(P_OSAL_SIGNAL pSignal);
int osal_signal_deinit(P_OSAL_SIGNAL);

int osal_event_init(P_OSAL_EVENT);
int osal_wait_for_event(P_OSAL_EVENT, P_COND, void*);
int osal_wait_for_event_timeout(P_OSAL_EVENT, P_COND, void*);
extern int osal_trigger_event(P_OSAL_EVENT);

int osal_event_deinit(P_OSAL_EVENT);
long osal_wait_for_event_bit_set(P_OSAL_EVENT pEvent, unsigned long *pState, unsigned int bitOffset);
long osal_wait_for_event_bit_clr(P_OSAL_EVENT pEvent, unsigned long *pState, unsigned int bitOffset);

int osal_thread_create(P_OSAL_THREAD);
int osal_thread_run(P_OSAL_THREAD);
int osal_thread_should_stop(P_OSAL_THREAD);
int osal_thread_stop(P_OSAL_THREAD);
/*int osal_thread_wait_for_event(P_OSAL_THREAD, P_OSAL_EVENT);*/
int osal_thread_wait_for_event(P_OSAL_THREAD, P_OSAL_EVENT, P_OSAL_EVENT_CHECKER);
/*check pOsalLxOp and OSAL_THREAD_SHOULD_STOP*/
int osal_thread_destroy(P_OSAL_THREAD);
int osal_thread_sched_mark(P_OSAL_THREAD, P_OSAL_THREAD_SCHEDSTATS schedstats);
int osal_thread_sched_unmark(P_OSAL_THREAD pThread, P_OSAL_THREAD_SCHEDSTATS schedstats);

int osal_clear_bit(unsigned int bitOffset, P_OSAL_BIT_OP_VAR pData);
int osal_set_bit(unsigned int bitOffset, P_OSAL_BIT_OP_VAR pData);
int osal_test_bit(unsigned int bitOffset, P_OSAL_BIT_OP_VAR pData);
int osal_test_and_clear_bit(unsigned int bitOffset, P_OSAL_BIT_OP_VAR pData);
int osal_test_and_set_bit(unsigned int bitOffset, P_OSAL_BIT_OP_VAR pData);

int osal_gettimeofday(int *sec, int *usec);
int osal_gettimeofday2(struct timeval *tv);

//int osal_printtimeofday(const unsigned char *prefix);
void osal_get_local_time(unsigned long long *sec, unsigned long *nsec);
unsigned long long osal_elapsed_us(unsigned long long ts, unsigned long usec);

void osal_buffer_dump(const unsigned char *buf, const unsigned char *title, unsigned int len, unsigned int limit);
void osal_buffer_dump_data(const unsigned int *buf, const unsigned char *title, const unsigned int len, const unsigned int limit,
			   const int flag);

unsigned int osal_op_get_id(P_OSAL_OP pOp);
MTK_CONN_BOOL osal_op_is_wait_for_signal(P_OSAL_OP pOp);
void osal_op_raise_signal(P_OSAL_OP pOp, int result);
void osal_set_op_result(P_OSAL_OP pOp, int result);
void osal_opq_dump(const char *qName, P_OSAL_OP_Q pOpQ);
void osal_opq_dump_locked(const char *qName, P_OSAL_OP_Q pOpQ);
MTK_CONN_BOOL osal_opq_has_op(P_OSAL_OP_Q pOpQ, P_OSAL_OP pOp);

int osal_ftrace_print(const char *str, ...);
int osal_ftrace_print_ctrl(int flag);

void osal_dump_thread_state(const unsigned char *name);
void osal_op_history_init(struct osal_op_history *log_history, int queue_size);
void osal_op_history_save(struct osal_op_history *log_history, P_OSAL_OP pOp);
void osal_op_history_print(struct osal_op_history *log_history, char *name);
/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/

#endif /* _OSAL_H_ */
