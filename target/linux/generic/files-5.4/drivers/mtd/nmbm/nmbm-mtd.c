// SPDX-License-Identifier: GPL-2.0
/*
 * MTD layer for NAND Mapped-block Management (NMBM)
 *
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/flashchip.h>
#include <linux/mtd/partitions.h>
#include <linux/of_platform.h>
#include <linux/kern_levels.h>

#include "nmbm-private.h"
#include "nmbm-debug.h"

#define NMBM_MAX_RATIO_DEFAULT			1
#define NMBM_MAX_BLOCKS_DEFAULT			256

struct nmbm_mtd {
	struct mtd_info upper;
	struct mtd_info *lower;

	struct nmbm_instance *ni;
	uint8_t *page_cache;

	flstate_t state;
	spinlock_t lock;
	wait_queue_head_t wq;

	struct device *dev;
	struct list_head node;
};

struct list_head nmbm_devs;
static DEFINE_MUTEX(nmbm_devs_lock);

static int nmbm_lower_read_page(void *arg, uint64_t addr, void *buf, void *oob,
				enum nmbm_oob_mode mode)
{
	struct nmbm_mtd *nm = arg;
	struct mtd_oob_ops ops;
	int ret;

	memset(&ops, 0, sizeof(ops));

	switch (mode) {
	case NMBM_MODE_PLACE_OOB:
		ops.mode = MTD_OPS_PLACE_OOB;
		break;
	case NMBM_MODE_AUTO_OOB:
		ops.mode = MTD_OPS_AUTO_OOB;
		break;
	case NMBM_MODE_RAW:
		ops.mode = MTD_OPS_RAW;
		break;
	default:
		pr_debug("%s: unsupported NMBM mode: %u\n", __func__, mode);
		return -ENOTSUPP;
	}

	if (buf) {
		ops.datbuf = buf;
		ops.len = nm->lower->writesize;
	}

	if (oob) {
		ops.oobbuf = oob;
		ops.ooblen = mtd_oobavail(nm->lower, &ops);
	}

	ret = mtd_read_oob(nm->lower, addr, &ops);
	nm->upper.ecc_stats.corrected = nm->lower->ecc_stats.corrected;
	nm->upper.ecc_stats.failed = nm->lower->ecc_stats.failed;

	/* Report error on failure (including ecc error) */
	if (ret < 0 && ret != -EUCLEAN)
		return ret;

	/*
	 * Since mtd_read_oob() won't report exact bitflips, what we can know
	 * is whether bitflips exceeds the threshold.
	 * We want the -EUCLEAN to be passed to the upper layer, but not the
	 * error value itself. To achieve this, report bitflips above the
	 * threshold.
	 */

	if (ret == -EUCLEAN) {
		return min_t(u32, nm->lower->bitflip_threshold + 1,
			     nm->lower->ecc_strength);
	}

	/* For bitflips less than the threshold, return 0 */
	return 0;
}

static int nmbm_lower_write_page(void *arg, uint64_t addr, const void *buf,
				 const void *oob, enum nmbm_oob_mode mode)
{
	struct nmbm_mtd *nm = arg;
	struct mtd_oob_ops ops;

	memset(&ops, 0, sizeof(ops));

	switch (mode) {
	case NMBM_MODE_PLACE_OOB:
		ops.mode = MTD_OPS_PLACE_OOB;
		break;
	case NMBM_MODE_AUTO_OOB:
		ops.mode = MTD_OPS_AUTO_OOB;
		break;
	case NMBM_MODE_RAW:
		ops.mode = MTD_OPS_RAW;
		break;
	default:
		pr_debug("%s: unsupported NMBM mode: %u\n", __func__, mode);
		return -ENOTSUPP;
	}

	if (buf) {
		ops.datbuf = (uint8_t *)buf;
		ops.len = nm->lower->writesize;
	}

	if (oob) {
		ops.oobbuf = (uint8_t *)oob;
		ops.ooblen = mtd_oobavail(nm->lower, &ops);
	}

	return mtd_write_oob(nm->lower, addr, &ops);
}

static int nmbm_lower_erase_block(void *arg, uint64_t addr)
{
	struct nmbm_mtd *nm = arg;
	struct erase_info ei;

	memset(&ei, 0, sizeof(ei));

	ei.addr = addr;
	ei.len = nm->lower->erasesize;

	return mtd_erase(nm->lower, &ei);
}

static int nmbm_lower_is_bad_block(void *arg, uint64_t addr)
{
	struct nmbm_mtd *nm = arg;

	return mtd_block_isbad(nm->lower, addr);
}

static int nmbm_lower_mark_bad_block(void *arg, uint64_t addr)
{
	struct nmbm_mtd *nm = arg;

	return mtd_block_markbad(nm->lower, addr);
}

static void nmbm_lower_log(void *arg, enum nmbm_log_category level,
			   const char *fmt, va_list ap)
{
	struct nmbm_mtd *nm = arg;
	char *msg;
	char *kl;

	msg = kvasprintf(GFP_KERNEL, fmt, ap);
	if (!msg) {
		dev_warn(nm->dev, "unable to print log\n");
		return;
	}

	switch (level) {
	case NMBM_LOG_DEBUG:
		kl = KERN_DEBUG;
		break;
	case NMBM_LOG_WARN:
		kl = KERN_WARNING;
		break;
	case NMBM_LOG_ERR:
		kl = KERN_ERR;
		break;
	case NMBM_LOG_EMERG:
		kl = KERN_EMERG;
		break;
	default:
		kl = KERN_INFO ;
	}

	dev_printk(kl, nm->dev, "%s", msg);

	kfree(msg);
}

static int nmbm_get_device(struct nmbm_mtd *nm, int new_state)
{
	DECLARE_WAITQUEUE(wait, current);

retry:
	spin_lock(&nm->lock);

	if (nm->state == FL_READY) {
		nm->state = new_state;
		spin_unlock(&nm->lock);
		return 0;
	}

	if (new_state == FL_PM_SUSPENDED) {
		if (nm->state == FL_PM_SUSPENDED) {
			spin_unlock(&nm->lock);
			return 0;
		}
	}

	set_current_state(TASK_UNINTERRUPTIBLE);
	add_wait_queue(&nm->wq, &wait);
	spin_unlock(&nm->lock);
	schedule();
	remove_wait_queue(&nm->wq, &wait);
	goto retry;
}

static void nmbm_release_device(struct nmbm_mtd *nm)
{
	spin_lock(&nm->lock);
	nm->state = FL_READY;
	wake_up(&nm->wq);
	spin_unlock(&nm->lock);
}

static int nmbm_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct nmbm_mtd *nm = container_of(mtd, struct nmbm_mtd, upper);
	int ret;

	nmbm_get_device(nm, FL_ERASING);

	ret = nmbm_erase_block_range(nm->ni, instr->addr, instr->len,
				     &instr->fail_addr);

	nmbm_release_device(nm);

	if (!ret)
		return 0;

	return -EIO;
}

static int nmbm_mtd_read_data(struct nmbm_mtd *nm, uint64_t addr,
			      struct mtd_oob_ops *ops, enum nmbm_oob_mode mode)
{
	size_t len, ooblen, maxooblen, chklen;
	uint32_t col, ooboffs;
	uint8_t *datcache, *oobcache;
	bool has_ecc_err = false;
	int ret, max_bitflips = 0;

	col = addr & nm->lower->writesize_mask;
	addr &= ~nm->lower->writesize_mask;
	maxooblen = mtd_oobavail(nm->lower, ops);
	ooboffs = ops->ooboffs;
	ooblen = ops->ooblen;
	len = ops->len;

	datcache = len ? nm->page_cache : NULL;
	oobcache = ooblen ? nm->page_cache + nm->lower->writesize : NULL;

	ops->oobretlen = 0;
	ops->retlen = 0;

	while (len || ooblen) {
		ret = nmbm_read_single_page(nm->ni, addr, datcache, oobcache,
					    mode);
		if (ret < 0 && ret != -EBADMSG)
			return ret;

		/* Continue reading on ecc error */
		if (ret == -EBADMSG)
			has_ecc_err = true;

		/* Record the maximum bitflips between pages */
		if (ret > max_bitflips)
			max_bitflips = ret;

		if (len) {
			/* Move data */
			chklen = nm->lower->writesize - col;
			if (chklen > len)
				chklen = len;

			memcpy(ops->datbuf + ops->retlen, datcache + col,
			       chklen);
			len -= chklen;
			col = 0; /* (col + chklen) %  */
			ops->retlen += chklen;
		}

		if (ooblen) {
			/* Move oob */
			chklen = maxooblen - ooboffs;
			if (chklen > ooblen)
				chklen = ooblen;

			memcpy(ops->oobbuf + ops->oobretlen, oobcache + ooboffs,
			       chklen);
			ooblen -= chklen;
			ooboffs = 0; /* (ooboffs + chklen) % maxooblen; */
			ops->oobretlen += chklen;
		}

		addr += nm->lower->writesize;
	}

	if (has_ecc_err)
		return -EBADMSG;

	return max_bitflips;
}

static int nmbm_mtd_read_oob(struct mtd_info *mtd, loff_t from,
			     struct mtd_oob_ops *ops)
{
	struct nmbm_mtd *nm = container_of(mtd, struct nmbm_mtd, upper);
	uint32_t maxooblen;
	enum nmbm_oob_mode mode;
	int ret;

	if (!ops->oobbuf && !ops->datbuf) {
		if (ops->ooblen || ops->len)
			return -EINVAL;

		return 0;
	}

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
		mode = NMBM_MODE_PLACE_OOB;
		break;
	case MTD_OPS_AUTO_OOB:
		mode = NMBM_MODE_AUTO_OOB;
		break;
	case MTD_OPS_RAW:
		mode = NMBM_MODE_RAW;
		break;
	default:
		pr_debug("%s: unsupported oob mode: %u\n", __func__, ops->mode);
		return -ENOTSUPP;
	}

	maxooblen = mtd_oobavail(mtd, ops);

	/* Do not allow read past end of device */
	if (ops->datbuf && (from + ops->len) > mtd->size) {
		pr_debug("%s: attempt to read beyond end of device\n",
			 __func__);
		return -EINVAL;
	}

	if (!ops->oobbuf) {
		nmbm_get_device(nm, FL_READING);

		/* Optimized for reading data only */
		ret = nmbm_read_range(nm->ni, from, ops->len, ops->datbuf,
				      mode, &ops->retlen);

		nmbm_release_device(nm);

		return ret;
	}

	if (unlikely(ops->ooboffs >= maxooblen)) {
		pr_debug("%s: attempt to start read outside oob\n",
			__func__);
		return -EINVAL;
	}

	if (unlikely(from >= mtd->size ||
	    ops->ooboffs + ops->ooblen > ((mtd->size >> mtd->writesize_shift) -
	    (from >> mtd->writesize_shift)) * maxooblen)) {
		pr_debug("%s: attempt to read beyond end of device\n",
				__func__);
		return -EINVAL;
	}

	nmbm_get_device(nm, FL_READING);
	ret = nmbm_mtd_read_data(nm, from, ops, mode);
	nmbm_release_device(nm);

	return ret;
}

static int nmbm_mtd_write_data(struct nmbm_mtd *nm, uint64_t addr,
			       struct mtd_oob_ops *ops, enum nmbm_oob_mode mode)
{
	size_t len, ooblen, maxooblen, chklen;
	uint32_t col, ooboffs;
	uint8_t *datcache, *oobcache;
	int ret;

	col = addr & nm->lower->writesize_mask;
	addr &= ~nm->lower->writesize_mask;
	maxooblen = mtd_oobavail(nm->lower, ops);
	ooboffs = ops->ooboffs;
	ooblen = ops->ooblen;
	len = ops->len;

	datcache = len ? nm->page_cache : NULL;
	oobcache = ooblen ? nm->page_cache + nm->lower->writesize : NULL;

	ops->oobretlen = 0;
	ops->retlen = 0;

	while (len || ooblen) {
		if (len) {
			/* Move data */
			chklen = nm->lower->writesize - col;
			if (chklen > len)
				chklen = len;

			memset(datcache, 0xff, col);
			memcpy(datcache + col, ops->datbuf + ops->retlen,
			       chklen);
			memset(datcache + col + chklen, 0xff,
			       nm->lower->writesize - col - chklen);
			len -= chklen;
			col = 0; /* (col + chklen) %  */
			ops->retlen += chklen;
		}

		if (ooblen) {
			/* Move oob */
			chklen = maxooblen - ooboffs;
			if (chklen > ooblen)
				chklen = ooblen;

			memset(oobcache, 0xff, ooboffs);
			memcpy(oobcache + ooboffs,
			       ops->oobbuf + ops->oobretlen, chklen);
			memset(oobcache + ooboffs + chklen, 0xff,
			       nm->lower->oobsize - ooboffs - chklen);
			ooblen -= chklen;
			ooboffs = 0; /* (ooboffs + chklen) % maxooblen; */
			ops->oobretlen += chklen;
		}

		ret = nmbm_write_single_page(nm->ni, addr, datcache, oobcache,
					     mode);
		if (ret)
			return ret;

		addr += nm->lower->writesize;
	}

	return 0;
}

static int nmbm_mtd_write_oob(struct mtd_info *mtd, loff_t to,
			      struct mtd_oob_ops *ops)
{
	struct nmbm_mtd *nm = container_of(mtd, struct nmbm_mtd, upper);
	enum nmbm_oob_mode mode;
	uint32_t maxooblen;
	int ret;

	if (!ops->oobbuf && !ops->datbuf) {
		if (ops->ooblen || ops->len)
			return -EINVAL;

		return 0;
	}

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
		mode = NMBM_MODE_PLACE_OOB;
		break;
	case MTD_OPS_AUTO_OOB:
		mode = NMBM_MODE_AUTO_OOB;
		break;
	case MTD_OPS_RAW:
		mode = NMBM_MODE_RAW;
		break;
	default:
		pr_debug("%s: unsupported oob mode: %u\n", __func__,
			 ops->mode);
		return -ENOTSUPP;
	}

	maxooblen = mtd_oobavail(mtd, ops);

	/* Do not allow write past end of device */
	if (ops->datbuf && (to + ops->len) > mtd->size) {
		pr_debug("%s: attempt to write beyond end of device\n",
			 __func__);
		return -EINVAL;
	}

	if (!ops->oobbuf) {
		nmbm_get_device(nm, FL_WRITING);

		/* Optimized for writing data only */
		ret = nmbm_write_range(nm->ni, to, ops->len, ops->datbuf,
				       mode, &ops->retlen);

		nmbm_release_device(nm);

		return ret;
	}

	if (unlikely(ops->ooboffs >= maxooblen)) {
		pr_debug("%s: attempt to start write outside oob\n",
			__func__);
		return -EINVAL;
	}

	if (unlikely(to >= mtd->size ||
	    ops->ooboffs + ops->ooblen > ((mtd->size >> mtd->writesize_shift) -
	    (to >> mtd->writesize_shift)) * maxooblen)) {
		pr_debug("%s: attempt to write beyond end of device\n",
				__func__);
		return -EINVAL;
	}

	nmbm_get_device(nm, FL_WRITING);
	ret = nmbm_mtd_write_data(nm, to, ops, mode);
	nmbm_release_device(nm);

	return ret;
}

static int nmbm_mtd_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	struct nmbm_mtd *nm = container_of(mtd, struct nmbm_mtd, upper);
	int ret;

	nmbm_get_device(nm, FL_READING);
	ret = nmbm_check_bad_block(nm->ni, offs);
	nmbm_release_device(nm);

	return ret;
}

static int nmbm_mtd_block_markbad(struct mtd_info *mtd, loff_t offs)
{
	struct nmbm_mtd *nm = container_of(mtd, struct nmbm_mtd, upper);
	int ret;

	nmbm_get_device(nm, FL_WRITING);
	ret = nmbm_mark_bad_block(nm->ni, offs);
	nmbm_release_device(nm);

	return ret;
}

static void nmbm_mtd_shutdown(struct mtd_info *mtd)
{
	struct nmbm_mtd *nm = container_of(mtd, struct nmbm_mtd, upper);

	nmbm_get_device(nm, FL_PM_SUSPENDED);
}

static int nmbm_probe(struct platform_device *pdev)
{
	struct device_node *mtd_np, *np = pdev->dev.of_node;
	uint32_t max_ratio, max_reserved_blocks, alloc_size;
	bool forced_create, empty_page_ecc_ok;
	struct nmbm_lower_device nld;
	struct mtd_info *lower, *mtd;
	struct nmbm_mtd *nm;
	const char *mtdname;
	int ret;

	mtd_np = of_parse_phandle(np, "lower-mtd-device", 0);
	if (mtd_np) {
		lower = get_mtd_device_by_node(mtd_np);
		if (!IS_ERR(lower))
			goto do_attach_mtd;

		dev_dbg(&pdev->dev, "failed to find mtd device by phandle\n");
		return -EPROBE_DEFER;
	}

	ret = of_property_read_string(np, "lower-mtd-name", &mtdname);
	if (!ret) {
		lower = get_mtd_device_nm(mtdname);
		if (!IS_ERR(lower))
			goto do_attach_mtd;

		dev_dbg(&pdev->dev, "failed to find mtd device by name '%s'\n",
			mtdname);
		return -EPROBE_DEFER;
	}

do_attach_mtd:
	if (of_property_read_u32(np, "max-ratio", &max_ratio))
		max_ratio = NMBM_MAX_RATIO_DEFAULT;

	if (of_property_read_u32(np, "max-reserved-blocks",
				 &max_reserved_blocks))
		max_reserved_blocks = NMBM_MAX_BLOCKS_DEFAULT;

	forced_create = of_property_read_bool(np, "forced-create");
	empty_page_ecc_ok = of_property_read_bool(np,
						  "empty-page-ecc-protected");

	memset(&nld, 0, sizeof(nld));

	nld.flags = 0;

	if (forced_create)
		nld.flags |= NMBM_F_CREATE;

	if (empty_page_ecc_ok)
		nld.flags |= NMBM_F_EMPTY_PAGE_ECC_OK;

	nld.max_ratio = max_ratio;
	nld.max_reserved_blocks = max_reserved_blocks;

	nld.size = lower->size;
	nld.erasesize = lower->erasesize;
	nld.writesize = lower->writesize;
	nld.oobsize = lower->oobsize;
	nld.oobavail = lower->oobavail;

	nld.read_page = nmbm_lower_read_page;
	nld.write_page = nmbm_lower_write_page;
	nld.erase_block = nmbm_lower_erase_block;
	nld.is_bad_block = nmbm_lower_is_bad_block;
	nld.mark_bad_block = nmbm_lower_mark_bad_block;

	nld.logprint = nmbm_lower_log;

	alloc_size = nmbm_calc_structure_size(&nld);

	nm = devm_kzalloc(&pdev->dev, sizeof(*nm) + alloc_size +
			  lower->writesize + lower->oobsize, GFP_KERNEL);
	if (!nm) {
		ret = -ENOMEM;
		goto out;
	}

	nm->ni = (void *)nm + sizeof(*nm);
	nm->page_cache = (uint8_t *)nm->ni + alloc_size;
	nm->lower = lower;
	nm->dev = &pdev->dev;

	INIT_LIST_HEAD(&nm->node);
	spin_lock_init(&nm->lock);
	init_waitqueue_head(&nm->wq);

	nld.arg = nm;

	ret = nmbm_attach(&nld, nm->ni);
	if (ret)
		goto out;

	/* Initialize upper mtd */
	mtd = &nm->upper;

	mtd->owner = THIS_MODULE;
	mtd->dev.parent = &pdev->dev;
	mtd->type = lower->type;
	mtd->flags = lower->flags;

	mtd->size = (uint64_t)nm->ni->data_block_count * lower->erasesize;
	mtd->erasesize = lower->erasesize;
	mtd->writesize = lower->writesize;
	mtd->writebufsize = lower->writesize;
	mtd->oobsize = lower->oobsize;
	mtd->oobavail = lower->oobavail;

	mtd->erasesize_shift = lower->erasesize_shift;
	mtd->writesize_shift = lower->writesize_shift;
	mtd->erasesize_mask = lower->erasesize_mask;
	mtd->writesize_mask = lower->writesize_mask;

	mtd->bitflip_threshold = lower->bitflip_threshold;

	mtd->ooblayout = lower->ooblayout;

	mtd->ecc_step_size = lower->ecc_step_size;
	mtd->ecc_strength = lower->ecc_strength;

	mtd->numeraseregions = lower->numeraseregions;
	mtd->eraseregions = lower->eraseregions;

	mtd->_erase = nmbm_mtd_erase;
	mtd->_read_oob = nmbm_mtd_read_oob;
	mtd->_write_oob = nmbm_mtd_write_oob;
	mtd->_block_isbad = nmbm_mtd_block_isbad;
	mtd->_block_markbad = nmbm_mtd_block_markbad;
	mtd->_reboot = nmbm_mtd_shutdown;

	mtd_set_of_node(mtd, np);

	ret = mtd_device_register(mtd, NULL, 0);
	if (ret) {
		dev_err(&pdev->dev, "failed to register mtd device\n");
		nmbm_detach(nm->ni);
		goto out;
	}

	platform_set_drvdata(pdev, nm);

	mutex_lock(&nmbm_devs_lock);
	list_add_tail(&nm->node, &nmbm_devs);
	mutex_unlock(&nmbm_devs_lock);

	return 0;

out:
	if (nm)
		devm_kfree(&pdev->dev, nm);

	put_mtd_device(lower);

	return ret;
}

static int nmbm_remove(struct platform_device *pdev)
{
	struct nmbm_mtd *nm = platform_get_drvdata(pdev);
	struct mtd_info *lower = nm->lower;
	int ret;

	ret = mtd_device_unregister(&nm->upper);
	if (ret)
		return ret;

	nmbm_detach(nm->ni);

	mutex_lock(&nmbm_devs_lock);
	list_add_tail(&nm->node, &nmbm_devs);
	mutex_unlock(&nmbm_devs_lock);

	devm_kfree(&pdev->dev, nm);

	put_mtd_device(lower);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id nmbm_ids[] = {
	{ .compatible = "generic,nmbm" },
	{ },
};

MODULE_DEVICE_TABLE(of, nmbm_ids);

static struct platform_driver nmbm_driver = {
	.probe = nmbm_probe,
	.remove = nmbm_remove,
	.driver = {
		.name = "nmbm",
		.of_match_table = nmbm_ids,
	},
};

static int __init nmbm_init(void)
{
	int ret;

	INIT_LIST_HEAD(&nmbm_devs);

	ret = platform_driver_register(&nmbm_driver);
	if (ret) {
		pr_err("failed to register nmbm driver\n");
		return ret;
	}

	return 0;
}
module_init(nmbm_init);

static void __exit nmbm_exit(void)
{
	platform_driver_unregister(&nmbm_driver);
}
module_exit(nmbm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Weijie Gao <weijie.gao@mediatek.com>");
MODULE_DESCRIPTION("NAND mapping block management");
