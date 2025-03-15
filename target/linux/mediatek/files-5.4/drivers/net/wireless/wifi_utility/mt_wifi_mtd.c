#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/concat.h>
#include <linux/mtd/partitions.h>
#if defined (CONFIG_MIPS)
#include <asm/addrspace.h>
#endif

int mt_mtd_write_nm_wifi(char *name, loff_t to, size_t len, const u_char *buf)
{
	int ret = -1;
	size_t rdlen, wrlen;
	struct mtd_info *mtd;
	struct erase_info ei;
	u_char *bak = NULL;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return -1;

	if (len > mtd->erasesize) {
		put_mtd_device(mtd);
		return -E2BIG;
	}

	bak = kmalloc(mtd->erasesize, GFP_KERNEL);
	if (bak == NULL) {
		put_mtd_device(mtd);
		return -ENOMEM;
	}

	ret = mtd_read(mtd, 0, mtd->erasesize, &rdlen, bak);

	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}

	if (rdlen != mtd->erasesize)
		printk("warning: ra_mtd_write: rdlen is not equal to erasesize\n");

	memcpy(bak + to, buf, len);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0))
	ei.mtd = mtd;
	ei.callback = NULL;
	ei.priv = 0;
#endif
	ei.addr = 0;
	ei.len = mtd->erasesize;
	ret = mtd_erase(mtd, &ei);

	if (ret != 0) {
		put_mtd_device(mtd);
		kfree(bak);
		return ret;
	}

	ret = mtd_write(mtd, 0, mtd->erasesize, &wrlen, bak);



	put_mtd_device(mtd);
	kfree(bak);
	return ret;
}
EXPORT_SYMBOL(mt_mtd_write_nm_wifi);


int mt_mtd_read_nm_wifi(char *name, loff_t from, size_t len, u_char *buf)
{
	int ret;
	size_t rdlen;
	struct mtd_info *mtd;

	mtd = get_mtd_device_nm(name);
	if (IS_ERR(mtd))
		return -1;

	ret = mtd_read(mtd, from, len, &rdlen, buf);

	if (rdlen != len)
			printk("warning: ra_mtd_read_nm: rdlen is not equal to len\n");

	put_mtd_device(mtd);

	return ret;
}
EXPORT_SYMBOL(mt_mtd_read_nm_wifi);
