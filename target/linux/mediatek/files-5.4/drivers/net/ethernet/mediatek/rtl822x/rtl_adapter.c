#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/if_vlan.h>
#include <linux/reset.h>
#include <linux/tcp.h>
#include <linux/interrupt.h>
#include <linux/mdio.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>

#include "../mtk_eth_soc.h"
#include "rtl_adapter.h"
#include "rtl8226_typedef.h"

bool MmdPhyWrite(HANDLE unit, uint16_t devad, uint16_t addr, uint16_t value)
{
    struct mtk_eth *eth = unit.eth;
    if (eth != NULL)
    {
        mtk_mmd_write(eth, unit.addr, devad, addr, value);
    }
    return TRUE;
}

bool MmdPhyRead(HANDLE unit, uint16_t devad, uint16_t addr, uint16_t *value)
{
    struct mtk_eth *eth = unit.eth;
    if (eth != NULL)
    {
        int val = mtk_mmd_read(eth, unit.addr, devad, addr);
        *value = (uint16_t)val;
    }
    return TRUE;
}