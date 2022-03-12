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
	sys_adaption_gen4m.c
*/
#include "sys_adaption.h"
#include "gl_os.h"
#include "gl_kal.h"
#include "gl_wext.h"
#include "precomp.h"
#include "debug.h"

#define UNUSED(x) ((void)(x))
/*****************************************************************************
 *	Packet free related functions
 *****************************************************************************/
void sys_ad_free_pkt(void *packet)
{

}

/*****************************************************************************
 *	OS memory allocate/manage/free related functions
 *****************************************************************************/
s_int32 sys_ad_alloc_mem(u_char **mem, u_long size)
{
	*mem = (pu_char)kalMemAlloc(size, PHY_MEM_TYPE);

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
	kalMemZero(ptr, length);
}

void sys_ad_set_mem(void *ptr, u_long length, u_char value)
{
	kalMemSet(ptr, value, length);
}


void sys_ad_move_mem(void *dest, void *src, u_long length)
{
	kalMemMove(dest, src, length);
}

s_int32 sys_ad_cmp_mem(void *dest, void *src, u_long length)
{
	return kalMemCmp(dest, src, length);
}

/*****************************************************************************
 *	OS task create/manage/kill related functions
 *****************************************************************************/
s_int32 sys_ad_kill_os_task(struct serv_os_task *task)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 sys_ad_attach_os_task(
	struct serv_os_task *task, SERV_OS_TASK_CALLBACK fn, u_long arg)
{
	return SERV_STATUS_SUCCESS;
}

s_int32 sys_ad_init_os_task(
	struct serv_os_task *task, char *task_name,
	void *priv_winfos, void *priv_configs)
{
	return SERV_STATUS_SUCCESS;
}

boolean sys_ad_wait_os_task(
	void *reserved, struct serv_os_task *task, s_int32 *status)
{
	return SERV_STATUS_SUCCESS;
}

void sys_ad_wakeup_os_task(struct serv_os_task *task)
{
	UNUSED(task);
}
/*****************************************************************************
 *	OS debug functions
 *****************************************************************************/
void sys_ad_mem_dump32(void *ptr, u_long length)
{
	dumpMemory32((s_int32 *)ptr, length);
}
