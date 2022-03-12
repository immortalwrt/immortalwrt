#ifdef OS_ABL_SUPPORT

#include <linux/module.h>
#include "rt_config.h"

#ifndef MT76XX_COMBO_DUAL_DRIVER_SUPPORT
EXPORT_SYMBOL(RTMP_DRV_OPS_FUNCTION);
#endif /* MT76XX_COMBO_DUAL_DRIVER_SUPPORT */

#endif /* OS_ABL_SUPPORT */
