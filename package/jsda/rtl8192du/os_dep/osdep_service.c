/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/


#define _OSDEP_SERVICE_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <linux/vmalloc.h>
#ifdef RTK_DMP_PLATFORM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
#include <linux/pageremap.h>
#endif
#endif

#define RT_TAG	'1178'

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 0))
static inline ssize_t call_read_iter(struct file *file, struct kiocb *kio,
				     struct iov_iter *iter)
{
	return file->f_op->read_iter(kio, iter);
}
#endif

#ifdef DBG_MEMORY_LEAK
#include <asm/atomic.h>
atomic_t _malloc_cnt = ATOMIC_INIT(0);
atomic_t _malloc_size = ATOMIC_INIT(0);
#endif /* DBG_MEMORY_LEAK */


static ssize_t new_sync_read(struct file *filp, void __user *buf, __kernel_size_t len, loff_t *ppos)
{
        struct iovec iov;
        struct kiocb kiocb;
        struct iov_iter iter;
        ssize_t ret;

	iov.iov_base = buf;
	iov.iov_len = len;
        init_sync_kiocb(&kiocb, filp);
        kiocb.ki_pos = *ppos;
        iov_iter_init(&iter, READ, &iov, 1, len);

        ret = call_read_iter(filp, &kiocb, &iter);
        BUG_ON(ret == -EIOCBQUEUED);
        *ppos = kiocb.ki_pos;
        return ret;
}

static ssize_t __vfs_read_alt(struct file *file, char __user *buf, size_t count,
                   loff_t *pos)
{
        if (file->f_op->read)
                return file->f_op->read(file, buf, count, pos);
        else if (file->f_op->read_iter)
                return new_sync_read(file, (void *)buf, (__kernel_size_t)count, pos);
        else
                return -EINVAL;
}
/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code){
	if(error_code >=0)
		return _SUCCESS;

	switch(error_code) {
		//case -ETIMEDOUT:
		//	return RTW_STATUS_TIMEDOUT;
		default:
			return _FAIL;
	}
}

u32 rtw_atoi(u8* s)
{

	int num=0,flag=0;
	int i;
	for(i=0;i<=strlen(s);i++)
	{
	  if(s[i] >= '0' && s[i] <= '9')
		 num = num * 10 + s[i] -'0';
	  else if(s[0] == '-' && i==0)
		 flag =1;
	  else
		  break;
	 }

	if(flag == 1)
	   num = num * -1;

	 return(num);

}

inline u8* _rtw_vmalloc(u32 sz)
{
	u8	*pbuf;
	pbuf = vmalloc(sz);

#ifdef DBG_MEMORY_LEAK
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

inline u8* _rtw_zvmalloc(u32 sz)
{
	u8	*pbuf;
	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);
	return pbuf;
}

inline void _rtw_vmfree(u8 *pbuf, u32 sz)
{
	vfree(pbuf);
#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */
}

u8* _rtw_malloc(u32 sz)
{

	u8	*pbuf=NULL;

#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		pbuf = (u8 *)dvr_malloc(sz);
	else
#endif
		pbuf = kmalloc(sz,in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);

#ifdef DBG_MEMORY_LEAK
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;

}


u8* _rtw_zmalloc(u32 sz)
{
	u8	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {

		memset(pbuf, 0, sz);
	}

	return pbuf;
}

void	_rtw_mfree(u8 *pbuf, u32 sz)
{

#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		dvr_free(pbuf);
	else
#endif
		kfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */

}


inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

inline int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
	skb->dev = ndev;
	return netif_rx(skb);
}

void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		_rtw_skb_free(skb);
}

inline void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	return usb_alloc_coherent(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#else
	return usb_buffer_alloc(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#endif
}

inline void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	usb_free_coherent(dev, size, addr, dma);
#else
	usb_buffer_free(dev, size, addr, dma);
#endif
}

#ifdef DBG_MEM_ALLOC

struct rtw_mem_stat {
	ATOMIC_T alloc; // the memory bytes we allocate currently
	ATOMIC_T peak; // the peak memory bytes we allocate
	ATOMIC_T alloc_cnt; // the alloc count for alloc currently
	ATOMIC_T alloc_err_cnt; // the error times we fail to allocate memory
};

struct rtw_mem_stat rtw_mem_type_stat[mstat_tf_idx(MSTAT_TYPE_MAX)];
#ifdef RTW_MEM_FUNC_STAT
struct rtw_mem_stat rtw_mem_func_stat[mstat_ff_idx(MSTAT_FUNC_MAX)];
#endif

char *MSTAT_TYPE_str[] = {
	"VIR",
	"PHY",
	"SKB",
	"USB",
};

#ifdef RTW_MEM_FUNC_STAT
char *MSTAT_FUNC_str[] = {
	"UNSP",
	"IO",
	"TXIO",
	"RXIO",
	"TX",
	"RX",
};
#endif

void rtw_mstat_dump(void *sel)
{
	int i;
	int value_t[4][mstat_tf_idx(MSTAT_TYPE_MAX)];
#ifdef RTW_MEM_FUNC_STAT
	int value_f[4][mstat_ff_idx(MSTAT_FUNC_MAX)];
#endif

	int vir_alloc, vir_peak, vir_alloc_err, phy_alloc, phy_peak, phy_alloc_err;
	int tx_alloc, tx_peak, tx_alloc_err, rx_alloc, rx_peak, rx_alloc_err;

	for(i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
		value_t[0][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].alloc));
		value_t[1][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].peak));
		value_t[2][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].alloc_cnt));
		value_t[3][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].alloc_err_cnt));
	}

	#ifdef RTW_MEM_FUNC_STAT
	for(i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
		value_f[0][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].alloc));
		value_f[1][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].peak));
		value_f[2][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].alloc_cnt));
		value_f[3][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].alloc_err_cnt));
	}
	#endif

	DBG_871X_SEL_NL(sel, "===================== MSTAT =====================\n");
	DBG_871X_SEL_NL(sel, "%4s %10s %10s %10s %10s\n", "TAG", "alloc", "peak", "aloc_cnt", "err_cnt");
	DBG_871X_SEL_NL(sel, "-------------------------------------------------\n");
	for(i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
		DBG_871X_SEL_NL(sel, "%4s %10d %10d %10d %10d\n", MSTAT_TYPE_str[i], value_t[0][i], value_t[1][i], value_t[2][i], value_t[3][i]);
	}
	#ifdef RTW_MEM_FUNC_STAT
	DBG_871X_SEL_NL(sel, "-------------------------------------------------\n");
	for(i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
		DBG_871X_SEL_NL(sel, "%4s %10d %10d %10d %10d\n", MSTAT_FUNC_str[i], value_f[0][i], value_f[1][i], value_f[2][i], value_f[3][i]);
	}
	#endif
}

void rtw_mstat_update(const enum mstat_f flags, const MSTAT_STATUS status, u32 sz)
{
	static u32 update_time = 0;
	int peak, alloc;
	int i;

	/* initialization */
	if(!update_time) {
		for(i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
			ATOMIC_SET(&(rtw_mem_type_stat[i].alloc), 0);
			ATOMIC_SET(&(rtw_mem_type_stat[i].peak), 0);
			ATOMIC_SET(&(rtw_mem_type_stat[i].alloc_cnt), 0);
			ATOMIC_SET(&(rtw_mem_type_stat[i].alloc_err_cnt), 0);
		}
		#ifdef RTW_MEM_FUNC_STAT
		for(i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
			ATOMIC_SET(&(rtw_mem_func_stat[i].alloc), 0);
			ATOMIC_SET(&(rtw_mem_func_stat[i].peak), 0);
			ATOMIC_SET(&(rtw_mem_func_stat[i].alloc_cnt), 0);
			ATOMIC_SET(&(rtw_mem_func_stat[i].alloc_err_cnt), 0);
		}
		#endif
	}

	switch(status) {
		case MSTAT_ALLOC_SUCCESS:
			ATOMIC_INC(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc_cnt));
			alloc = ATOMIC_ADD_RETURN(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc), sz);
			peak=ATOMIC_READ(&(rtw_mem_type_stat[mstat_tf_idx(flags)].peak));
			if (peak<alloc)
				ATOMIC_SET(&(rtw_mem_type_stat[mstat_tf_idx(flags)].peak), alloc);

			#ifdef RTW_MEM_FUNC_STAT
			ATOMIC_INC(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc_cnt));
			alloc = ATOMIC_ADD_RETURN(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc), sz);
			peak=ATOMIC_READ(&(rtw_mem_func_stat[mstat_ff_idx(flags)].peak));
			if (peak<alloc)
				ATOMIC_SET(&(rtw_mem_func_stat[mstat_ff_idx(flags)].peak), alloc);
			#endif
			break;

		case MSTAT_ALLOC_FAIL:
			ATOMIC_INC(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc_err_cnt));
			#ifdef RTW_MEM_FUNC_STAT
			ATOMIC_INC(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc_err_cnt));
			#endif
			break;

		case MSTAT_FREE:
			ATOMIC_DEC(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc_cnt));
			ATOMIC_SUB(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc), sz);
			#ifdef RTW_MEM_FUNC_STAT
			ATOMIC_DEC(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc_cnt));
			ATOMIC_SUB(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc), sz);
			#endif
			break;
	};

	//if (rtw_get_passing_time_ms(update_time) > 5000) {
	//	rtw_mstat_dump(RTW_DBGDUMP);
		update_time=rtw_get_current_time();
	//}
}

#ifndef SIZE_MAX
	#define SIZE_MAX (~(size_t)0)
#endif

struct mstat_sniff_rule {
	enum mstat_f flags;
	size_t lb;
	size_t hb;
};

struct mstat_sniff_rule mstat_sniff_rules[] = {
	{MSTAT_TYPE_PHY, 4097, SIZE_MAX},
};

int mstat_sniff_rule_num = sizeof(mstat_sniff_rules)/sizeof(struct mstat_sniff_rule);

bool match_mstat_sniff_rules(const enum mstat_f flags, const size_t size)
{
	int i;
	for (i = 0; i<mstat_sniff_rule_num; i++) {
		if (mstat_sniff_rules[i].flags == flags
				&& mstat_sniff_rules[i].lb <= size
				&& mstat_sniff_rules[i].hb >= size)
			return _TRUE;
	}

	return _FALSE;
}

inline u8* dbg_rtw_vmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8  *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p=_rtw_vmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline u8* dbg_rtw_zvmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p=_rtw_zvmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline void dbg_rtw_vmfree(u8 *pbuf, u32 sz, const enum mstat_f flags, const char *func, const int line)
{

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	_rtw_vmfree((pbuf), (sz));

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, sz
	);
}

inline u8* dbg_rtw_malloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p=_rtw_malloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline u8* dbg_rtw_zmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p = _rtw_zmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline void dbg_rtw_mfree(u8 *pbuf, u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	_rtw_mfree((pbuf), (sz));

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, sz
	);
}

inline struct sk_buff * dbg_rtw_skb_alloc(unsigned int size, const enum mstat_f flags, const char *func, int line)
{
	struct sk_buff *skb;
	unsigned int truesize = 0;

	skb = _rtw_skb_alloc(size);

	if(skb)
		truesize = skb->truesize;

	if(!skb || truesize < size || match_mstat_sniff_rules(flags, truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d), skb:%p, truesize=%u\n", func, line, __FUNCTION__, size, skb, truesize);

	rtw_mstat_update(
		flags
		, skb ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb;
}

inline void dbg_rtw_skb_free(struct sk_buff *skb, const enum mstat_f flags, const char *func, int line)
{
	unsigned int truesize = skb->truesize;

	if(match_mstat_sniff_rules(flags, truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s, truesize=%u\n", func, line, __FUNCTION__, truesize);

	_rtw_skb_free(skb);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, truesize
	);
}

inline struct sk_buff *dbg_rtw_skb_copy(const struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line)
{
	struct sk_buff *skb_cp;
	unsigned int truesize = skb->truesize;
	unsigned int cp_truesize = 0;

	skb_cp = _rtw_skb_copy(skb);
	if(skb_cp)
		cp_truesize = skb_cp->truesize;

	if(!skb_cp || cp_truesize < truesize || match_mstat_sniff_rules(flags, cp_truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%u), skb_cp:%p, cp_truesize=%u\n", func, line, __FUNCTION__, truesize, skb_cp, cp_truesize);

	rtw_mstat_update(
		flags
		, skb_cp ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb_cp;
}

inline struct sk_buff *dbg_rtw_skb_clone(struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line)
{
	struct sk_buff *skb_cl;
	unsigned int truesize = skb->truesize;
	unsigned int cl_truesize = 0;

	skb_cl = _rtw_skb_clone(skb);
	if(skb_cl)
		cl_truesize = skb_cl->truesize;

	if(!skb_cl || cl_truesize < truesize || match_mstat_sniff_rules(flags, cl_truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%u), skb_cl:%p, cl_truesize=%u\n", func, line, __FUNCTION__, truesize, skb_cl, cl_truesize);

	rtw_mstat_update(
		flags
		, skb_cl ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb_cl;
}

inline int dbg_rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb, const enum mstat_f flags, const char *func, int line)
{
	int ret;
	unsigned int truesize = skb->truesize;

	if(match_mstat_sniff_rules(flags, truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s, truesize=%u\n", func, line, __FUNCTION__, truesize);

	ret = _rtw_netif_rx(ndev, skb);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, truesize
	);

	return ret;
}

inline void dbg_rtw_skb_queue_purge(struct sk_buff_head *list, enum mstat_f flags, const char *func, int line)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		dbg_rtw_skb_free(skb, flags, func, line);
}

inline void *dbg_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma, const enum mstat_f flags, const char *func, int line)
{
	void *p;

	if(match_mstat_sniff_rules(flags, size))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, size);

	p = _rtw_usb_buffer_alloc(dev, size, dma);

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, size
	);

	return p;
}

inline void dbg_rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma, const enum mstat_f flags, const char *func, int line)
{
	if(match_mstat_sniff_rules(flags, size))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, size);

	_rtw_usb_buffer_free(dev, size, addr, dma);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, size
	);
}
#endif /* DBG_MEM_ALLOC */

void* rtw_malloc2d(int h, int w, int size)
{
	int j;

	void **a = (void **) rtw_zmalloc( h*sizeof(void *) + h*w*size );
	if(a == NULL)
	{
		DBG_871X("%s: alloc memory fail!\n", __FUNCTION__);
		return NULL;
	}

	for( j=0; j<h; j++ )
		a[j] = ((char *)(a+h)) + j*w*size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	rtw_mfree((u8 *)pbuf, h*sizeof(void*) + w*h*size);
}

void _rtw_memcpy(void* dst, void* src, u32 sz)
{
	memcpy(dst, src, sz);
}

int	_rtw_memcmp(void *dst, void *src, u32 sz)
{

//under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
}

void _rtw_memset(void *pbuf, int c, u32 sz)
{
        memset(pbuf, c, sz);
}

void _rtw_init_listhead(_list *list)
{
        INIT_LIST_HEAD(list);
}


/*
For the following list_xxx operations,
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	rtw_is_list_empty(_list *phead)
{
	if (list_empty(phead))
		return _TRUE;
	else
		return _FALSE;
}

void rtw_list_insert_head(_list *plist, _list *phead)
{
	list_add(plist, phead);
}

void rtw_list_insert_tail(_list *plist, _list *phead)
{
	list_add_tail(plist, phead);
}

/*
Caller must check if the list is empty before calling rtw_list_delete
*/

void _rtw_init_sema(_sema	*sema, int init_val)
{
	sema_init(sema, init_val);
}

void _rtw_free_sema(_sema	*sema)
{
}

void _rtw_up_sema(_sema	*sema)
{
	up(sema);
}

u32 _rtw_down_sema(_sema *sema)
{
	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;
}



void	_rtw_mutex_init(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_init(pmutex);
#else
	init_MUTEX(pmutex);
#endif
}

void	_rtw_mutex_free(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_destroy(pmutex);
#endif
}

void	_rtw_spinlock_init(_lock *plock)
{
	spin_lock_init(plock);
}

void	_rtw_spinlock_free(_lock *plock)
{
}

void	_rtw_spinlock(_lock	*plock)
{
	spin_lock(plock);
}

void	_rtw_spinunlock(_lock *plock)
{
	spin_unlock(plock);
}


void	_rtw_spinlock_ex(_lock	*plock)
{
	spin_lock(plock);
}

void	_rtw_spinunlock_ex(_lock *plock)
{
	spin_unlock(plock);
}

void	_rtw_init_queue(_queue	*pqueue)
{
	_rtw_init_listhead(&(pqueue->queue));

	_rtw_spinlock_init(&(pqueue->lock));
}

u32	  _rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}


u32 rtw_end_of_queue_search(_list *head, _list *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}


u32	rtw_get_current_time(void)
{
	return jiffies;
}

inline u32 rtw_systime_to_ms(u32 systime)
{
	return systime * 1000 / HZ;
}

inline u32 rtw_ms_to_systime(u32 ms)
{
	return ms * HZ / 1000;
}

// the input parameter start use the same unit as returned by rtw_get_current_time
inline s32 rtw_get_passing_time_ms(u32 start)
{
	return rtw_systime_to_ms(jiffies-start);
}

inline s32 rtw_get_time_interval_ms(u32 start, u32 end)
{
	return rtw_systime_to_ms(end-start);
}


void rtw_sleep_schedulable(int ms)
{
    u32 delta;

    delta = (ms * HZ)/1000;//(ms)
    if (delta == 0) {
        delta = 1;// 1 ms
    }
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
}


void rtw_msleep_os(int ms)
{
	msleep((unsigned int)ms);
}

void rtw_usleep_os(int us)
{
      // msleep((unsigned int)us);
      if ( 1 < (us/1000) )
                msleep(1);
      else
		msleep( (us/1000) + 1);
}


#ifdef DBG_DELAY_OS
void _rtw_mdelay_os(int ms, const char *func, const int line)
{
	#if 0
	if(ms>10)
		DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);
		rtw_msleep_os(ms);
	return;
	#endif


	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);

	mdelay((unsigned long)ms);
}

void _rtw_udelay_os(int us, const char *func, const int line)
{

	#if 0
	if(us > 1000) {
	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);
		rtw_usleep_os(us);
		return;
	}
	#endif


	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);

	udelay((unsigned long)us);
}
#else
void rtw_mdelay_os(int ms)
{
	mdelay((unsigned long)ms);
}

void rtw_udelay_os(int us)
{
      udelay((unsigned long)us);
}
#endif

void rtw_yield_os()
{
	yield();
}

#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"

#ifdef CONFIG_WAKELOCK
static struct wake_lock rtw_suspend_lock;
#elif defined(CONFIG_ANDROID_POWER)
static android_suspend_lock_t rtw_suspend_lock ={
	.name = RTW_SUSPEND_LOCK_NAME
};
#endif

inline void rtw_suspend_lock_init()
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_init(&rtw_suspend_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_LOCK_NAME);
	#elif defined(CONFIG_ANDROID_POWER)
	android_init_suspend_lock(&rtw_suspend_lock);
	#endif
}

inline void rtw_suspend_lock_uninit()
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_destroy(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_uninit_suspend_lock(&rtw_suspend_lock);
	#endif
}

inline void rtw_lock_suspend()
{
	#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_lock);
	#endif
}

inline void rtw_unlock_suspend()
{
	#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_lock);
	#endif
}

inline void rtw_lock_suspend_timeout(u32 timeout_ms)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_lock, rtw_ms_to_systime(timeout_ms));
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_lock, rtw_ms_to_systime(timeout_ms));
	#endif
}

inline void ATOMIC_SET(ATOMIC_T *v, int i)
{
	atomic_set(v,i);
}

inline int ATOMIC_READ(ATOMIC_T *v)
{
	return atomic_read(v);
}

inline void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	atomic_add(i,v);
}
inline void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	atomic_sub(i,v);
}

inline void ATOMIC_INC(ATOMIC_T *v)
{
	atomic_inc(v);
}

inline void ATOMIC_DEC(ATOMIC_T *v)
{
	atomic_dec(v);
}

inline int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	return atomic_add_return(i,v);
}

inline int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	return atomic_sub_return(i,v);
}

inline int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	return atomic_inc_return(v);
}

inline int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	return atomic_dec_return(v);
}


/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, char *path, int flag, int mode)
{
	struct file *fp;

	fp=filp_open(path, flag, mode);
	if(IS_ERR(fp)) {
		*fpp=NULL;
		return PTR_ERR(fp);
	}
	else {
		*fpp=fp;
		return 0;
	}
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp)
{
	filp_close(fp,NULL);
	return 0;
}

static int readFile(struct file *fp,char *buf,int len)
{
	int rlen=0, sum=0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	if (!(fp->f_mode & FMODE_CAN_READ))
#else
	if (!fp->f_op || !fp->f_op->read)
#endif
		return -EPERM;

	while(sum<len) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
		rlen = __vfs_read_alt(fp, buf+sum, len-sum, &fp->f_pos);
#else
		rlen = fp->f_op->read(fp, buf+sum, len-sum, &fp->f_pos);
#endif
		if(rlen>0)
			sum+=rlen;
		else if(0 != rlen)
			return rlen;
		else
			break;
	}

	return  sum;

}

static int writeFile(struct file *fp,char *buf,int len)
{
	int wlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->write)
		return -EPERM;

	while(sum<len) {
		wlen=fp->f_op->write(fp,buf+sum,len-sum, &fp->f_pos);
		if(wlen>0)
			sum+=wlen;
		else if(0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(char *path)
{
	struct file *fp;
	int ret = 0;
	mm_segment_t oldfs;
	char buf;

	fp=filp_open(path, O_RDONLY, 0);
	if(IS_ERR(fp)) {
		ret = PTR_ERR(fp);
	}
	else {
		oldfs = get_fs(); set_fs(get_ds());

		if(1!=readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);

		set_fs(oldfs);
		filp_close(fp,NULL);
	}
	return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(char *path, u8* buf, u32 sz)
{
	int ret =-1;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp,path, O_RDONLY, 0)) ){
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=readFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s readFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(char *path, u8* buf, u32 sz)
{
	int ret =0;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp, path, O_CREAT|O_WRONLY, 0666)) ) {
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=writeFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s writeFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable(char *path)
{
	if(isFileReadable(path) == 0)
		return _TRUE;
	else
		return _FALSE;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{
	int ret =retriveFromFile(path, buf, sz);
	return ret>=0?ret:0;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(char *path, u8* buf, u32 sz)
{
	int ret =storeToFile(path, buf, sz);
	return ret>=0?ret:0;
}

#if 1 //#ifdef MEM_ALLOC_REFINE_ADAPTOR
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);
	pnpi->priv=old_priv;
	pnpi->sizeof_priv=sizeof_priv;

RETURN:
	return pnetdev;
}

struct net_device *rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);

	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(pnetdev);
		pnetdev = NULL;
		goto RETURN;
	}

	pnpi->sizeof_priv=sizeof_priv;
RETURN:
	return pnetdev;
}

void rtw_free_netdev(struct net_device * netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;

	if(!netdev)
		goto RETURN;

	pnpi = netdev_priv(netdev);

	if(!pnpi->priv)
		goto RETURN;

	rtw_vmfree(pnpi->priv, pnpi->sizeof_priv);
	free_netdev(netdev);

RETURN:
	return;
}

/*
* Jeff: this function should be called under ioctl (rtnl_lock is accquired) while
* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
*/
int rtw_change_ifname(_adapter *padapter, const char *ifname)
{
	struct net_device *pnetdev;
	struct net_device *cur_pnetdev = padapter->pnetdev;
	struct rereg_nd_name_data *rereg_priv;
	int ret;

	if(!padapter)
		goto error;

	rereg_priv = &padapter->rereg_nd_name_priv;

	//free the old_pnetdev
	if(rereg_priv->old_pnetdev) {
		free_netdev(rereg_priv->old_pnetdev);
		rereg_priv->old_pnetdev = NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if(!rtnl_is_locked())
		unregister_netdev(cur_pnetdev);
	else
#endif
		unregister_netdevice(cur_pnetdev);

	rereg_priv->old_pnetdev=cur_pnetdev;

	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)  {
		ret = -1;
		goto error;
	}

	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(adapter_to_dvobj(padapter)));

	rtw_init_netdev_name(pnetdev, ifname);

	_rtw_memcpy(pnetdev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if(!rtnl_is_locked())
		ret = register_netdev(pnetdev);
	else
#endif
		ret = register_netdevice(pnetdev);

	if ( ret != 0) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("register_netdev() failed\n"));
		goto error;
	}

	return 0;

error:

	return -1;

}
#endif //MEM_ALLOC_REFINE_ADAPTOR

u64 rtw_modular64(u64 x, u64 y)
{
	return do_div(x, y);
}

u64 rtw_division64(u64 x, u64 y)
{
	do_div(x, y);
	return x;
}

void rtw_buf_free(u8 **buf, u32 *buf_len)
{
	u32 ori_len;

	if (!buf || !buf_len)
		return;

	ori_len = *buf_len;

	if (*buf) {
		u32 tmp_buf_len = *buf_len;
		*buf_len = 0;
		rtw_mfree(*buf, tmp_buf_len);
		*buf = NULL;
	}
}

void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len)
{
	u32 ori_len = 0, dup_len = 0;
	u8 *ori = NULL;
	u8 *dup = NULL;

	if (!buf || !buf_len)
		return;

	if (!src || !src_len)
		goto keep_ori;

	/* duplicate src */
	dup = rtw_malloc(src_len);
	if (dup) {
		dup_len = src_len;
		_rtw_memcpy(dup, src, dup_len);
	}

keep_ori:
	ori = *buf;
	ori_len = *buf_len;

	/* replace buf with dup */
	*buf_len = 0;
	*buf = dup;
	*buf_len = dup_len;

	/* free ori */
	if (ori && ori_len > 0)
		rtw_mfree(ori, ori_len);
}


/**
 * rtw_cbuf_full - test if cbuf is full
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is full
 */
inline bool rtw_cbuf_full(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read-1)? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_empty - test if cbuf is empty
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is empty
 */
inline bool rtw_cbuf_empty(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read)? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_push - push a pointer into cbuf
 * @cbuf: pointer of struct rtw_cbuf
 * @buf: pointer to push in
 *
 * Lock free operation, be careful of the use scheme
 * Returns: _TRUE push success
 */
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf)
{
	if (rtw_cbuf_full(cbuf))
		return _FAIL;

	if (0)
		DBG_871X("%s on %u\n", __func__, cbuf->write);
	cbuf->bufs[cbuf->write] = buf;
	cbuf->write = (cbuf->write+1)%cbuf->size;

	return _SUCCESS;
}

/**
 * rtw_cbuf_pop - pop a pointer from cbuf
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Lock free operation, be careful of the use scheme
 * Returns: pointer popped out
 */
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf)
{
	void *buf;
	if (rtw_cbuf_empty(cbuf))
		return NULL;

	if (0)
		DBG_871X("%s on %u\n", __func__, cbuf->read);
	buf = cbuf->bufs[cbuf->read];
	cbuf->read = (cbuf->read+1)%cbuf->size;

	return buf;
}

/**
 * rtw_cbuf_alloc - allocte a rtw_cbuf with given size and do initialization
 * @size: size of pointer
 *
 * Returns: pointer of srtuct rtw_cbuf, NULL for allocation failure
 */
struct rtw_cbuf *rtw_cbuf_alloc(u32 size)
{
	struct rtw_cbuf *cbuf;

	cbuf = (struct rtw_cbuf *)rtw_malloc(sizeof(*cbuf) + sizeof(void*)*size);

	if (cbuf) {
		cbuf->write = cbuf->read = 0;
		cbuf->size = size;
	}

	return cbuf;
}

/**
 * rtw_cbuf_free - free the given rtw_cbuf
 * @cbuf: pointer of struct rtw_cbuf to free
 */
void rtw_cbuf_free(struct rtw_cbuf *cbuf)
{
	rtw_mfree((u8*)cbuf, sizeof(*cbuf) + sizeof(void*)*cbuf->size);
}
