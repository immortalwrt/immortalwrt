#ifndef __MTK_WED_H
#define __MTK_WED_H

#include <linux/kernel.h>
#include <linux/rcupdate.h>
#include <linux/regmap.h>
#include <linux/pci.h>

#define MTK_WED_TX_QUEUES		2

struct mtk_wed_hw;
struct mtk_wdma_desc;

struct mtk_wed_ring {
	struct mtk_wdma_desc *desc;
	dma_addr_t desc_phys;
	int size;

	u32 reg_base;
	void __iomem *wpdma;
};

struct mtk_wed_device {
};

struct mtk_wed_ops {
	int (*attach)(struct mtk_wed_device *dev);
	int (*tx_ring_setup)(struct mtk_wed_device *dev, int ring,
			     void __iomem *regs);
	int (*txfree_ring_setup)(struct mtk_wed_device *dev,
				 void __iomem *regs);
	void (*detach)(struct mtk_wed_device *dev);

	void (*stop)(struct mtk_wed_device *dev);
	void (*start)(struct mtk_wed_device *dev, u32 irq_mask);
	void (*reset_dma)(struct mtk_wed_device *dev);

	u32 (*reg_read)(struct mtk_wed_device *dev, u32 reg);
	void (*reg_write)(struct mtk_wed_device *dev, u32 reg, u32 val);

	u32 (*irq_get)(struct mtk_wed_device *dev, u32 mask);
	void (*irq_set_mask)(struct mtk_wed_device *dev, u32 mask);
};

extern const struct mtk_wed_ops __rcu *mtk_soc_wed_ops;

static inline int
mtk_wed_device_attach(struct mtk_wed_device *dev)
{
	int ret = -ENODEV;

	return ret;
}

static inline bool
mtk_wed_get_rx_capa(struct mtk_wed_device *dev)
{
	return false;
}

static inline bool mtk_wed_device_active(struct mtk_wed_device *dev)
{
	return false;
}
#define mtk_wed_device_detach(_dev) do {} while (0)
#define mtk_wed_device_start(_dev, _mask) do {} while (0)
#define mtk_wed_device_tx_ring_setup(_dev, _ring, _regs) -ENODEV
#define mtk_wed_device_txfree_ring_setup(_dev, _ring, _regs) -ENODEV
#define mtk_wed_device_reg_read(_dev, _reg) 0
#define mtk_wed_device_reg_write(_dev, _reg, _val) do {} while (0)
#define mtk_wed_device_irq_get(_dev, _mask) 0
#define mtk_wed_device_irq_set_mask(_dev, _mask) do {} while (0)
#define mtk_wed_device_rx_ring_setup(_dev, _ring, _regs) -ENODEV
#define mtk_wed_device_ppe_check(_dev, _skb, _reason, _hash)  do {} while (0)
#define mtk_wed_device_update_msg(_dev, _id, _msg, _len) -ENODEV

#endif
