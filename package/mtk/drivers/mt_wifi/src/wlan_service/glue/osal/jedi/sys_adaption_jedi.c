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
	sys_adaption_jedi.c
*/
#include "rt_config.h"
#include "sys_adaption.h"

s_int32 serv_dbg_lvl = SERV_DBG_LVL_WARN;

/*****************************************************************************
 *	Packet free related functions
 *****************************************************************************/
void sys_ad_free_pkt(void *packet)
{
	if (packet) {
		dev_kfree_skb_any(SERV_PKT_TO_OSPKT(packet));
		packet = NULL;
	}
}

/*****************************************************************************
 *	OS memory allocate/manage/free related functions
 *****************************************************************************/
s_int32 sys_ad_alloc_mem(u_char **mem, u_long size)
{
	*mem = (pu_char) kmalloc(size, GFP_ATOMIC);

	if (*mem)
		return SERV_STATUS_SUCCESS;
	else
		return SERV_STATUS_OSAL_SYS_FAIL;
}

void sys_ad_free_mem(void *mem)
{
	ASSERT(mem);
	kfree(mem);
}

void sys_ad_zero_mem(void *ptr, u_long length)
{
	memset(ptr, 0, length);
}

void sys_ad_set_mem(void *ptr, u_long length, u_char value)
{
	memset(ptr, value, length);
}

void sys_ad_move_mem(void *dest, void *src, u_long length)
{
	memmove(dest, src, length);
}

s_int32 sys_ad_cmp_mem(void *dest, void *src, u_long length)
{
	return memcmp(dest, src, length);
}

/*****************************************************************************
 *	OS task create/manage/kill related functions
 *****************************************************************************/
static inline s_int32 _sys_ad_kill_os_task(struct serv_os_task *task)
{
	s_int32 ret = SERV_STATUS_OSAL_SYS_FAIL;

	if (task->kthread_task) {
		if (kthread_stop(task->kthread_task) == 0) {
			task->kthread_task = NULL;
			ret = SERV_STATUS_SUCCESS;
		} else {
			SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_WARN,
				("%s kthread_task %s stop failed\n", __func__,
				task->task_name));
		}
	} else
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_WARN,
			("%s null kthread_task %s\n", __func__,
			task->task_name));

	return ret;
}

static inline s_int32 _sys_ad_attach_os_task(
	IN struct serv_os_task *task, IN SERV_OS_TASK_CALLBACK fn,
	IN u_long arg)
{
	s_int32 status = SERV_STATUS_SUCCESS;

	task->task_killed = 0;

	if (task->kthread_task) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s non-null kthread_task %s\n", __func__,
			task->task_name));
		status = SERV_STATUS_OSAL_SYS_FAIL;
		goto done;
	}

	task->kthread_task = kthread_run((cast_fn) fn, (void *)arg,
					task->task_name);

	if (IS_ERR(task->kthread_task)) {
		SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_ERROR,
			("%s kthread_run %s err %ld\n", __func__,
			task->task_name, PTR_ERR(task->kthread_task)));
		task->kthread_task = NULL;
		status = SERV_STATUS_OSAL_SYS_FAIL;
		goto done;
	}

done:
	SERV_LOG(SERV_DBG_CAT_TEST, SERV_DBG_LVL_TRACE,
		("%s %s end %d\n", __func__, task->task_name, status));

	return status;
}

static inline s_int32 _sys_ad_init_os_task(
	IN struct serv_os_task *task, IN char *task_name,
	IN void *priv_winfos, IN void *priv_configs)
{
	s_int32 len;

	ASSERT(task);
	len = strlen(task_name);
	len = len > (SERV_OS_TASK_NAME_LEN - 1)
		? (SERV_OS_TASK_NAME_LEN - 1) : len;
	os_move_mem(&task->task_name[0], task_name, len);

	if (priv_winfos)
		task->priv_winfos = priv_winfos;

	if (priv_configs)
		task->priv_configs = priv_configs;

	init_waitqueue_head(&(task->kthread_q));

	return SERV_STATUS_SUCCESS;
}

boolean _sys_ad_wait_os_task(
	IN void *reserved, IN struct serv_os_task *task, IN s_int32 *status)
{
	RTMP_WAIT_EVENT_INTERRUPTIBLE((*status), task);

	if ((task->task_killed == 1) || ((*status) != 0))
		return FALSE;

	return TRUE;
}

s_int32 sys_ad_kill_os_task(struct serv_os_task *task)
{
	return _sys_ad_kill_os_task(task);
}

s_int32 sys_ad_attach_os_task(
	struct serv_os_task *task, SERV_OS_TASK_CALLBACK fn, u_long arg)
{
	return _sys_ad_attach_os_task(task, fn, arg);
}

s_int32 sys_ad_init_os_task(
	struct serv_os_task *task, char *task_name,
	void *priv_winfos, void *priv_configs)
{
	return _sys_ad_init_os_task(task, task_name,
				priv_winfos, priv_configs);
}

boolean sys_ad_wait_os_task(
	void *reserved, struct serv_os_task *task, s_int32 *status)
{
	return _sys_ad_wait_os_task(reserved, task, status);
}

VOID sys_ad_wakeup_os_task(struct serv_os_task *task)
{
	WAKE_UP(task);
}
