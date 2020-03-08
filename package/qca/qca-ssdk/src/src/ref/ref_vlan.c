/*
 * Copyright (c) 2012, 2014, 2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "sw.h"
#include "ssdk_init.h"
#include "fal_init.h"
#include "fal_misc.h"
#include "fal_mib.h"
#include "fal_port_ctrl.h"
#include "fal_portvlan.h"
#include "fal_fdb.h"
#include "fal_stp.h"
#include "fal_igmp.h"
#include "fal_qos.h"
#include "fal_acl.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "ssdk_init.h"
#include <linux/kconfig.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/types.h>
//#include <asm/mach-types.h>
#include <generated/autoconf.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,14,0))
#include <linux/ar8216_platform.h>
#endif
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include "ssdk_plat.h"
#include "ref_vlan.h"
#ifdef BOARD_AR71XX
#include "ssdk_uci.h"
#endif


extern ssdk_chip_type SSDK_CURRENT_CHIP_TYPE;


int
qca_ar8327_sw_enable_vlan0(a_uint32_t dev_id, a_bool_t enable, a_uint8_t portmap)
{
    fal_vlan_t entry;
    fal_acl_rule_t rule;
    int i = 0;

    memset(&entry, 0, sizeof(fal_vlan_t));
    memset(&rule, 0, sizeof(fal_acl_rule_t));
    for (i = 0; i < AR8327_NUM_PORTS; i ++) {
        fal_port_tls_set(dev_id, i, A_FALSE);
        fal_port_vlan_propagation_set(dev_id, i, FAL_VLAN_PROPAGATION_REPLACE);
    }

    if (enable) {
        entry.fid = 0;
        entry.mem_ports = portmap;
        entry.unmodify_ports = portmap;
        entry.vid = 0;
        fal_vlan_entry_append(dev_id, &entry);
        for (i = 0; i < AR8327_NUM_PORTS; i++) {
            if (portmap & (0x1 << i)) {
                fal_port_egvlanmode_set(dev_id, i, FAL_EG_UNTOUCHED);
                fal_port_tls_set(dev_id, i, A_TRUE);
                fal_port_vlan_propagation_set(dev_id, i, FAL_VLAN_PROPAGATION_DISABLE);
                fal_acl_port_udf_profile_set(dev_id, i, FAL_ACL_UDF_TYPE_L2, 12, 4);
            }
        }

        fal_acl_list_creat(dev_id, 0, 0);
        rule.rule_type = FAL_ACL_RULE_UDF;
        rule.udf_len = 4;
        rule.udf_val[0] = 0x81;
        rule.udf_val[1] = 0;
        rule.udf_val[2] = 0;
        rule.udf_val[3] = 0;
        rule.udf_mask[0] = 0xff;
        rule.udf_mask[1] = 0xff;
        rule.udf_mask[2] = 0xf;
        rule.udf_mask[3] = 0xff;
        FAL_FIELD_FLG_SET(rule.field_flg, FAL_ACL_FIELD_UDF);
        FAL_ACTION_FLG_SET(rule.action_flg, FAL_ACL_ACTION_REMARK_LOOKUP_VID);
        fal_acl_rule_add(dev_id, 0, 0, 1, &rule);
        for (i = 0; i < AR8327_NUM_PORTS; i ++) {
            fal_acl_list_unbind(dev_id, 0, 0, 0, i);
            if (portmap & (0x1 << i)) {
                fal_acl_list_bind(dev_id, 0, 0, 0, i);
            }
        }
        fal_acl_status_set(dev_id, A_TRUE);
    }
    else {
        fal_acl_rule_delete(dev_id, 0, 0, 1);
    }

    return 0;
}

#if defined(IN_SWCONFIG)
int
qca_ar8327_sw_set_vlan(struct switch_dev *dev,
                       const struct switch_attr *attr,
                       struct switch_val *val)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);

    priv->vlan = !!val->value.i;

    #ifdef BOARD_AR71XX
    if(SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA) {
		ssdk_uci_sw_set_vlan(attr, val);
    }
    #endif

    return 0;
}

int
qca_ar8327_sw_get_vlan(struct switch_dev *dev,
                       const struct switch_attr *attr,
                       struct switch_val *val)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);

    val->value.i = priv->vlan;

    return 0;
}

int
qca_ar8327_sw_set_vid(struct switch_dev *dev,
                      const struct switch_attr *attr,
                      struct switch_val *val)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);

    priv->vlan_id[val->port_vlan] = val->value.i;

#ifdef BOARD_AR71XX
    if(SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA) {
		ssdk_uci_sw_set_vid(attr, val);
    }
#endif

    return 0;
}

int
qca_ar8327_sw_get_vid(struct switch_dev *dev,
                      const struct switch_attr *attr,
                      struct switch_val *val)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);

    val->value.i = priv->vlan_id[val->port_vlan];

    return 0;
}

int
qca_ar8327_sw_get_pvid(struct switch_dev *dev, int port, int *vlan)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);

    *vlan = priv->pvid[port];

    return 0;
}

int
qca_ar8327_sw_set_pvid(struct switch_dev *dev, int port, int vlan)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);

    /* make sure no invalid PVIDs get set */
    if (vlan >= dev->vlans)
        return -1;

    priv->pvid[port] = vlan;

#ifdef BOARD_AR71XX
		if(SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA) {
			ssdk_uci_sw_set_pvid(port, vlan);
		}
#endif

    return 0;
}

int
qca_ar8327_sw_get_ports(struct switch_dev *dev, struct switch_val *val)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    a_uint8_t ports = priv->vlan_table[val->port_vlan];
    int i;

    val->len = 0;
    for (i = 0; i < dev->ports; i++) {
        struct switch_port *p;

        if (!(ports & (1 << i)))
            continue;

        p = &val->value.ports[val->len++];
        p->id = i;
        if (priv->vlan_tagged[val->port_vlan] & (1 << i))
            p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
        else
            p->flags = 0;

        /*Handle for VLAN 0*/
        if (val->port_vlan == 0)
            p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
    }

    return 0;
}

int
qca_ar8327_sw_set_ports(struct switch_dev *dev, struct switch_val *val)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    a_uint8_t *vt = &priv->vlan_table[val->port_vlan];
    int i;

#ifdef BOARD_AR71XX
	if(SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA) {
		ssdk_uci_sw_set_ports(val);
	}
#endif

    /*Handle for VLAN 0*/
    if (val->port_vlan == 0) {
        priv->vlan_table[0] = 0;
        for (i = 0; i < val->len; i++) {
            struct switch_port *p = &val->value.ports[i];
            priv->vlan_table[0] |= (1 << p->id);
        }

        return 0;
    }
	if (priv->vlan_id[val->port_vlan] == 0)
		priv->vlan_id[val->port_vlan] = val->port_vlan;
    *vt = 0;
    for (i = 0; i < val->len; i++) {
        struct switch_port *p = &val->value.ports[i];

        if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
            priv->vlan_tagged[val->port_vlan] |= (1 << p->id);
        } else {
            priv->vlan_tagged[val->port_vlan] &= ~(1 << p->id);
            priv->pvid[p->id] = val->port_vlan;
        }

        *vt |= 1 << p->id;
    }

    return 0;
}

int
qca_ar8327_sw_hw_apply(struct switch_dev *dev)
{
    struct qca_phy_priv *priv = qca_phy_priv_get(dev);
    fal_pbmp_t *portmask = NULL;
    int i, j;

    if (priv->version == QCA_VER_HPPE) {
        return 0;
    }

    portmask = aos_mem_alloc(sizeof(fal_pbmp_t) * dev->ports);
    if (portmask == NULL) {
        SSDK_ERROR("%s: portmask malloc failed. \n", __func__);
        return -1;
    }

    mutex_lock(&priv->reg_mutex);

    memset(portmask, 0, sizeof(*portmask));
    if (!priv->init) {
        /*Handle VLAN 0 entry*/
        if (priv->vlan_id[0] == 0 && priv->vlan_table[0] == 0) {
            qca_ar8327_sw_enable_vlan0(priv->device_id, A_FALSE, 0);
        }

        /* calculate the port destination masks and load vlans
         * into the vlan translation unit */
        for (j = 0; j < AR8327_MAX_VLANS; j++) {
            u8 vp = priv->vlan_table[j];

            if (!vp) {
                fal_vlan_delete(priv->device_id, priv->vlan_id[j]);
                continue;
            }
            fal_vlan_delete(priv->device_id, priv->vlan_id[j]);
            fal_vlan_create(priv->device_id, priv->vlan_id[j]);

            for (i = 0; i < dev->ports; i++) {
                u8 mask = (1 << i);
                if (vp & mask) {
                    fal_vlan_member_add(priv->device_id, priv->vlan_id[j], i,
                           (mask & priv->vlan_tagged[j])? FAL_EG_TAGGED : FAL_EG_UNTAGGED);
                    portmask[i] |= vp & ~mask;
                }
            }
	    	if (SSDK_CURRENT_CHIP_TYPE == CHIP_SHIVA)
				fal_vlan_member_update(priv->device_id,priv->vlan_id[j],vp,0);
        }

        /*Hanlde VLAN 0 entry*/
        if (priv->vlan_id[0] == 0 && priv->vlan_table[0]) {
            qca_ar8327_sw_enable_vlan0(priv->device_id,A_TRUE, priv->vlan_table[0]);
        }

    } else {
        /* vlan disabled:
         * isolate all ports, but connect them to the cpu port */
        for (i = 0; i < dev->ports; i++) {
            if (i == AR8327_PORT_CPU)
                continue;

            portmask[i] = 1 << AR8327_PORT_CPU;
            portmask[AR8327_PORT_CPU] |= (1 << i);
        }
    }

    /* update the port destination mask registers and tag settings */
    for (i = 0; i < dev->ports; i++) {
        int pvid;
        fal_pt_1qmode_t ingressMode;
        fal_pt_1q_egmode_t egressMode;

        if (priv->vlan) {
            pvid = priv->vlan_id[priv->pvid[i]];
            ingressMode = FAL_1Q_SECURE;
        } else {
            pvid = 0;
            ingressMode = FAL_1Q_DISABLE;
        }
        egressMode = FAL_EG_UNTOUCHED;

        fal_port_1qmode_set(priv->device_id, i, ingressMode);
        fal_port_egvlanmode_set(priv->device_id, i, egressMode);
        fal_port_default_cvid_set(priv->device_id, i, pvid);
        fal_portvlan_member_update(priv->device_id, i, portmask[i]);
    }

    aos_mem_free(portmask);
    portmask = NULL;

    mutex_unlock(&priv->reg_mutex);

    return 0;
}
#endif


