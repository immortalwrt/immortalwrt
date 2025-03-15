/*
* switch_fun.h: switch function sets
*/
#ifndef SWITCH_FUN_H
#define SWITCH_FUN_H

#include <stdbool.h>

#define MT7530_T10_TEST_CONTROL 0x145

#define MAX_PORT 6
#define MAX_PHY_PORT 5
#define CONFIG_MTK_7531_DVT 1

extern int chip_name;
extern struct mt753x_attr *attres;
extern bool nl_init_flag;

/*basic operation*/
int reg_read(unsigned int offset, unsigned int *value);
int reg_write(unsigned int offset, unsigned int value);
int mii_mgr_read(unsigned int port_num, unsigned int reg, unsigned int *value);
int mii_mgr_write(unsigned int port_num, unsigned int reg, unsigned int value);
int mii_mgr_c45_read(unsigned int port_num, unsigned int dev, unsigned int reg, unsigned int *value);
int mii_mgr_c45_write(unsigned int port_num, unsigned int dev, unsigned int reg, unsigned int value);

/*phy setting*/
int phy_dump(int phy_addr);
void phy_crossover(int argc, char *argv[]);
int rw_phy_token_ring(int argc, char *argv[]);
/*arl setting*/
void doArlAging(int argc, char *argv[]);

/*acl setting*/
void acl_mac_add(int argc, char *argv[]);
void acl_dip_meter(int argc, char *argv[]);
void acl_dip_trtcm(int argc, char *argv[]);
void acl_ethertype(int argc, char *argv[]);
void acl_ethertype(int argc, char *argv[]);
void acl_dip_modify(int argc, char *argv[]);
void acl_dip_pppoe(int argc, char *argv[]);
void acl_dip_add(int argc, char *argv[]);
void acl_l4_add(int argc, char *argv[]);
void acl_sp_add(int argc, char *argv[]);

void acl_port_enable(int argc, char *argv[]);
void acl_table_add(int argc, char *argv[]);
void acl_mask_table_add(int argc, char *argv[]);
void acl_rule_table_add(int argc, char *argv[]);
void acl_rate_table_add(int argc, char *argv[]);

/*dip table*/
void dip_dump(void);
void dip_add(int argc, char *argv[]);
void dip_del(int argc, char *argv[]);
void dip_clear(void);

/*sip table*/
void sip_dump(void);
void sip_add(int argc, char *argv[]);
void sip_del(int argc, char *argv[]);
void sip_clear(void);

/*stp*/
void doStp(int argc, char *argv[]);

/*mac table*/
void table_dump(void);
void table_add(int argc, char *argv[]);
void table_search_mac_vid(int argc, char *argv[]);
void table_search_mac_fid(int argc, char *argv[]);
void table_del_fid(int argc, char *argv[]);
void table_del_vid(int argc, char *argv[]);
void table_clear(void);

/*vlan table*/
void vlan_dump(int argc, char *argv[]);
void vlan_clear(int argc, char *argv[]);
void vlan_set(int argc, char *argv[]);

void doVlanSetPvid(int argc, char *argv[]);
void doVlanSetVid(int argc, char *argv[]);
void doVlanSetAccFrm(int argc, char *argv[]);
void doVlanSetPortAttr(int argc, char *argv[]);
void doVlanSetPortMode(int argc, char *argv[]);
void doVlanSetEgressTagPCR(int argc, char *argv[]);
void doVlanSetEgressTagPVC(int argc, char *argv[]);

/*igmp function*/
void igmp_on(int argc, char *argv[]);
void igmp_off();
void igmp_disable(int argc, char *argv[]);
void igmp_enable(int argc, char *argv[]);

/*mirror function*/
void set_mirror_to(int argc, char *argv[]);
void set_mirror_from(int argc, char *argv[]);
void doMirrorPortBased(int argc, char *argv[]);
void doMirrorEn(int argc, char *argv[]);

/*rate control*/
void rate_control(int argc, char *argv[]);
int ingress_rate_set(int on_off, unsigned int port, unsigned int bw);
int egress_rate_set(int on_off, int port, int bw);

/*QoS*/
int qos_sch_select(int argc, char *argv[]);
void qos_set_base(int argc, char *argv[]);
void qos_wfq_set_weight(int argc, char *argv[]);
void qos_set_portpri(int argc, char *argv[]);
void qos_set_dscppri(int argc, char *argv[]);
void qos_pri_mapping_queue(int argc, char *argv[]);

/*flow control*/
int global_set_mac_fc(int argc, char *argv[]);
int phy_set_fc(int argc, char *argv[]);
int phy_set_an(int argc, char *argv[]);

/* collision pool functions */
int collision_pool_enable(int argc, char *argv[]);
void collision_pool_mac_dump();
void collision_pool_dip_dump();
void collision_pool_sip_dump();

/*pfc functions*/
int set_mac_pfc(int argc, char *argv[]);
void pfc_get_rx_counter(int argc, char *argv[]);
void pfc_get_tx_counter(int argc, char *argv[]);

/*switch reset*/
int switch_reset(int argc, char *argv[]);

/* EEE(802.3az) function  */
void eee_enable(int argc, char *argv[]);
void eee_dump(int argc, char *argv[]);

void read_mib_counters();
void clear_mib_counters();
void read_output_queue_counters();
void read_free_page_counters();

void phy_crossover(int argc, char *argv[]);
void exit_free();
#endif
