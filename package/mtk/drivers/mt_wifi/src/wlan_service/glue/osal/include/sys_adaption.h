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
	sys_adaption.h
*/

#ifndef __SYS_ADAPTION_H__
#define __SYS_ADAPTION_H__

#include <linux/spinlock.h>
#include <linux/completion.h>
#include <linux/skbuff.h>
#include <linux/utsname.h>

/*****************************************************************************
 *	Type definition
 *****************************************************************************/
typedef signed char s_int8;
typedef signed short s_int16;
typedef signed int s_int32;
typedef signed long long s_int64;
typedef unsigned char u_int8;
typedef unsigned short u_int16;
typedef unsigned int u_int32;
typedef unsigned long long u_int64;

typedef signed char s_char;
typedef signed short s_short;
typedef signed long s_long;
typedef signed long long s_longlong;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned long long u_longlong;

typedef signed int *ps_int32;
typedef signed long long *ps_int64;
typedef unsigned char *pu_int8;
typedef unsigned short *pu_int16;
typedef unsigned int *pu_int32;
typedef unsigned long long *pu_int64;

typedef s_char *ps_char;
typedef u_char *pu_char;
typedef u_short *pu_short;
typedef s_long *ps_long;
typedef u_long *pu_long;

typedef unsigned char boolean;

#ifndef NULL
#define NULL	0
#endif

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

/* Spin lock */
typedef spinlock_t SERV_OS_SPIN_LOCK;

/* Completion */
typedef struct completion SERV_OS_COMPLETION;

/* Service task */
#define SERV_OS_TASK_NAME_LEN	16
typedef struct serv_os_task {
	char task_name[SERV_OS_TASK_NAME_LEN];
	void *priv_winfos;
	void *priv_configs;
	unsigned char task_killed;
	struct task_struct *kthread_task;
	wait_queue_head_t kthread_q;
	boolean kthread_running;
} SERV_OS_TASK;
typedef s_int32(*SERV_OS_TASK_CALLBACK) (u_long);

/*gen4m wlan oid call back function*/
typedef s_int32 (*wlan_oid_handler_t) (void *winfos,
	uint32_t oid_type,
	void *param,
	uint32_t param_len,
	void *rsp,
	void *rsp_data);

/*****************************************************************************
 *	Macro
 *****************************************************************************/
/* Service IP return status */
/* The first byte means module and the second byte means error status */
/* Success */
#define SERV_STATUS_SUCCESS				0x0000

/* Agent module: failure */
#define SERV_STATUS_AGENT_FAIL				0x0100
/* Agent module: invalid null pointer */
#define SERV_STATUS_AGENT_INVALID_NULL_POINTER		0x0101
/* Agent module: invalid band idx */
#define SERV_STATUS_AGENT_INVALID_BANDIDX		0x0102
/* Agent module: invalid length */
#define SERV_STATUS_AGENT_INVALID_LEN			0x0103
/* Agent module: invalid parameter */
#define SERV_STATUS_AGENT_INVALID_PARAM			0x0104
/* Agent module: not supported */
#define SERV_STATUS_AGENT_NOT_SUPPORTED			0x0105

/* Service test module: failure */
#define SERV_STATUS_SERV_TEST_FAIL			0x0200
/* Service test module: invalid null pointer */
#define SERV_STATUS_SERV_TEST_INVALID_NULL_POINTER	0x0201
/* Service test module: invalid band idx */
#define SERV_STATUS_SERV_TEST_INVALID_BANDIDX		0x0202
/* Service test module: invalid length */
#define SERV_STATUS_SERV_TEST_INVALID_LEN		0x0203
/* Service test module: invalid parameter */
#define SERV_STATUS_SERV_TEST_INVALID_PARAM		0x0204
/* Service test module: not supported */
#define SERV_STATUS_SERV_TEST_NOT_SUPPORTED		0x0205

/* Test engine module: failure */
#define SERV_STATUS_ENGINE_FAIL				0x0300
/* Test engine module: invalid null pointer */
#define SERV_STATUS_ENGINE_INVALID_NULL_POINTER		0x0301
/* Test engine module: invalid band idx */
#define SERV_STATUS_ENGINE_INVALID_BANDIDX		0x0302
/* Test engine module: invalid length */
#define SERV_STATUS_ENGINE_INVALID_LEN			0x0303
/* Test engine module: invalid parameter */
#define SERV_STATUS_ENGINE_INVALID_PARAM		0x0304
/* Test engine module: not supported */
#define SERV_STATUS_ENGINE_NOT_SUPPORTED		0x0305

/* Hal test mac module: failure */
#define SERV_STATUS_HAL_MAC_FAIL			0x0400
/* Hal test mac module: invalid padapter */
#define SERV_STATUS_HAL_MAC_INVALID_PAD			0x0401
/* Hal test mac module: invalid null pointer */
#define SERV_STATUS_HAL_MAC_INVALID_NULL_POINTER	0x0402
/* Hal test mac module: invalid band idx */
#define SERV_STATUS_HAL_MAC_INVALID_BANDIDX		0x0403
/* Hal test mac module: un-registered chip ops */
#define SERV_STATUS_HAL_MAC_INVALID_CHIPOPS		0x0404

/* Hal operation module: failure */
#define SERV_STATUS_HAL_OP_FAIL				0x0500
/* Hal operation module: failure to send fw command */
#define SERV_STATUS_HAL_OP_FAIL_SEND_FWCMD		0x0501
/* Hal operation module: failure to set mac behavior */
#define SERV_STATUS_HAL_OP_FAIL_SET_MAC			0x0502
/* Hal operation module: invalid padapter */
#define SERV_STATUS_HAL_OP_INVALID_PAD			0x0503
/* Hal operation module: invalid null pointer */
#define SERV_STATUS_HAL_OP_INVALID_NULL_POINTER		0x0504
/* Hal operation module: invalid band idx */
#define SERV_STATUS_HAL_OP_INVALID_BANDIDX		0x0505

/* Osal net adaption module: failure */
#define SERV_STATUS_OSAL_NET_FAIL			0x0600
/* Osal net adaption module: failure to send fw command */
#define SERV_STATUS_OSAL_NET_FAIL_SEND_FWCMD		0x0601
/* Osal net adaption module: failure to init wdev */
#define SERV_STATUS_OSAL_NET_FAIL_INIT_WDEV		0x0602
/* Osal net adaption module: failure to release wdev */
#define SERV_STATUS_OSAL_NET_FAIL_RELEASE_WDEV		0x0603
/* Osal net adaption module: failure to update wdev */
#define SERV_STATUS_OSAL_NET_FAIL_UPDATE_WDEV		0x0604
/* Osal net adaption module: failure to set channel */
#define SERV_STATUS_OSAL_NET_FAIL_SET_CHANNEL		0x0605
/* Osal net adaption module: invalid padapter */
#define SERV_STATUS_OSAL_NET_INVALID_PAD		0x0606
/* Osal net adaption module: invalid null pointer */
#define SERV_STATUS_OSAL_NET_INVALID_NULL_POINTER	0x0607
/* Osal net adaption module: invalid band idx */
#define SERV_STATUS_OSAL_NET_INVALID_BANDIDX		0x0608
/* Osal net adaption module: invalid length */
#define SERV_STATUS_OSAL_NET_INVALID_LEN		0x0609
/* Osal net adaption module: invalid parameter */
#define SERV_STATUS_OSAL_NET_INVALID_PARAM		0x060A

/* Osal sys adaption module: failure */
#define SERV_STATUS_OSAL_SYS_FAIL			0x0700
/* Osal sys adaption module: invalid padapter */
#define SERV_STATUS_OSAL_SYS_INVALID_PAD		0x0701
/* Osal sys adaption module: invalid null pointer */
#define SERV_STATUS_OSAL_SYS_INVALID_NULL_POINTER	0x0702
/* Osal sys adaption module: invalid band idx */
#define SERV_STATUS_OSAL_SYS_INVALID_BANDIDX		0x0703

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

/* Use bitmap to allow coexist of OP_MODE_TXFRAME and OP_MODE_RXFRAME */
#define fTEST_IDLE		(1 << 0)
#define fTEST_TX_ENABLE		(1 << 1)
#define fTEST_RX_ENABLE		(1 << 2)
#define fTEST_TXCONT_ENABLE	(1 << 3)
#define fTEST_TXCARR_ENABLE	(1 << 4)
#define fTEST_TXCARRSUPP_ENABLE	(1 << 5)
#define fTEST_MPS		(1 << 6)
#define fTEST_FFT_ENABLE	(1 << 7)
#define fTEST_EXIT		(1 << 8)
#define fTEST_IN_RFTEST		(1 << 9)
#define fTEST_IN_BF		(1 << 10)
#define fTEST_IN_ICAPOVERLAP	(1 << 11)

/* OPMODE */
#define fTEST_OPER_NORMAL_MODE		0
#define fTEST_OPER_RFTEST_MODE		1
#define fTEST_OPER_ICAP_MODE		2
#define fTEST_OPER_ICAP_OVERLAP		3
#define fTEST_OPER_WIFI_SPECTRUM	4

/* Stop Transmission */
#define OP_MODE_TXSTOP			((~(fTEST_TX_ENABLE))		\
					&(~(fTEST_TXCONT_ENABLE))	\
					&(~(fTEST_TXCARR_ENABLE))	\
					&(~(fTEST_TXCARRSUPP_ENABLE))	\
					&(~(fTEST_MPS)))
/* Stop Receiving Frames */
#define OP_MODE_RXSTOP			(~(fTEST_RX_ENABLE))
/* Enter/Reset ATE */
#define	OP_MODE_START			(fTEST_IDLE)
/* Stop/Exit ATE */
#define	OP_MODE_STOP			(fTEST_EXIT)
/* Continuous Transmit Frames (without time gap) */
#define	OP_MODE_TXCONT			((fTEST_TX_ENABLE)	\
					|(fTEST_TXCONT_ENABLE))
/* Transmit Carrier */
#define	OP_MODE_TXCARR			((fTEST_TX_ENABLE)	\
					|(fTEST_TXCARR_ENABLE))
/* Transmit Carrier Suppression (information without carrier) */
#define	OP_MODE_TXCARRSUPP		((fTEST_TX_ENABLE)	\
					|(fTEST_TXCARRSUPP_ENABLE))
/* Transmit Frames */
#define	OP_MODE_TXFRAME			(fTEST_TX_ENABLE)
/* Receive Frames */
#define	OP_MODE_RXFRAME			(fTEST_RX_ENABLE)
/* MPS */
#define	OP_MODE_MPS			((fTEST_TX_ENABLE)|(fTEST_MPS))
/* FFT */
#define OP_MODE_FFT			((fTEST_FFT_ENABLE)|(fTEST_IN_RFTEST))

/* Service debug level */
#define SERV_DBG_LVL_OFF	0
#define SERV_DBG_LVL_ERROR	1
#define SERV_DBG_LVL_WARN	2
#define SERV_DBG_LVL_TRACE	3
#define SERV_DBG_LVL_INFO	4
#define SERV_DBG_LVL_MAX	SERV_DBG_LVL_INFO

/* Debug category */
#define SERV_DBG_CAT_MISC	0	/* misc */
#define SERV_DBG_CAT_TEST	1	/* service test */
#define SERV_DBG_CAT_ENGN	2	/* service engine */
#define SERV_DBG_CAT_ADAPT	3	/* service adaption */
#define SERV_DBG_CAT_ALL	SERV_DBG_CAT_MISC
#define SERV_DBG_CAT_EN_ALL_MASK	0xFFFFFFFFu

extern s_int32 serv_dbg_lvl;

/* Debugging and printing related definitions and prototypes */
#define SERV_PRINT		printk
#define SERV_PRINT_MAC(addr)	\
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define SERV_LOG(category, level, fmt)					\
	do {								\
		if ((0x1 << category) & (SERV_DBG_CAT_EN_ALL_MASK))	\
			if (level <= serv_dbg_lvl)			\
				SERV_PRINT fmt;				\
	} while (0)

/* OS task related data structure and definition */
#define SERV_OS_TASK_GET(__pTask)		\
				(__pTask)
#define SERV_OS_TASK_GET_WINFOS(__pTask)	\
				((__pTask)->priv_winfos)
#define SERV_OS_TASK_GET_CONFIGS(__pTask)	\
				((__pTask)->priv_configs)
#define SERV_OS_TASK_IS_KILLED(__pTask)	\
				((__pTask)->task_killed)

#define SERV_OS_INIT_COMPLETION(__pCompletion)	\
			init_completion(__pCompletion)
#define SERV_OS_EXIT_COMPLETION(__pCompletion)	\
			complete(__pCompletion)
#define SERV_OS_COMPLETE(__pCompletion)		\
			complete(__pCompletion)
#define SERV_OS_WAIT_FOR_COMPLETION_TIMEOUT(__pCompletion, __Timeout)	\
			wait_for_completion_timeout(__pCompletion, __Timeout)

/* Spin_lock enhanced for service spin lock */
#define SERV_OS_ALLOCATE_SPIN_LOCK(__lock)		\
				spin_lock_init((spinlock_t *)(__lock))
#define SERV_OS_FREE_SPIN_LOCK(lock)			\
				do {} while (0)
#define SERV_OS_SEM_LOCK(__lock)			\
				spin_lock_bh((spinlock_t *)(__lock))

#define SERV_OS_SEM_UNLOCK(__lock)			\
				spin_unlock_bh((spinlock_t *)(__lock))

/* NTOHS/NTONS/NTOHL/NTONS */
#define SERV_OS_NTOHS(_val)	(ntohs((_val)))
#define SERV_OS_HTONS(_val)	(htons((_val)))
#define SERV_OS_NTOHL(_val)	(ntohl((_val)))
#define SERV_OS_HTONL(_val)	(htonl((_val)))

/* Service skb packet clone */
#define SERV_PKT_TO_OSPKT(_p)	((struct sk_buff *)(_p))
#define SERV_OS_PKT_CLONE(_pkt, _src, _flag)				\
	{								\
		_src = skb_clone(SERV_PKT_TO_OSPKT(_pkt), _flag);	\
	}

/* Array size */
#define SERV_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/*****************************************************************************
 *	Enum value definition
 *****************************************************************************/
#define MAX_SERV_THREAD_NUM 3


/*****************************************************************************
 *	Data struct definition
 *****************************************************************************/
/* Service thread cb for test */
struct test_thread_cb {
	boolean is_init;
	struct serv_os_task task;
	SERV_OS_SPIN_LOCK lock;
	u_char service_stat;
	s_int32 deq_cnt;
	SERV_OS_COMPLETION cmd_done;
	u_long cmd_expire;
};

/*****************************************************************************
 *	Function declaration
 *****************************************************************************/
void sys_ad_free_pkt(void *packet);
s_int32 sys_ad_alloc_mem(u_char **mem, u_long size);
void sys_ad_free_mem(void *mem);
void sys_ad_zero_mem(void *ptr, u_long length);
void sys_ad_set_mem(void *ptr, u_long length, u_char value);
void sys_ad_move_mem(void *dest, void *src, u_long length);
s_int32 sys_ad_cmp_mem(void *dest, void *src, u_long length);
s_int32 sys_ad_kill_os_task(struct serv_os_task *task);
s_int32 sys_ad_attach_os_task(
	struct serv_os_task *task, SERV_OS_TASK_CALLBACK fn, u_long arg);
s_int32 sys_ad_init_os_task(
	struct serv_os_task *task, char *task_name,
	void *priv_winfos, void *priv_configs);
boolean sys_ad_wait_os_task(
	void *reserved, struct serv_os_task *task, s_int32 *status);
void sys_ad_wakeup_os_task(struct serv_os_task *task);

void sys_ad_mem_dump32(void *ptr, u_long length);

#endif /* __SYS_ADAPTION_H__ */
