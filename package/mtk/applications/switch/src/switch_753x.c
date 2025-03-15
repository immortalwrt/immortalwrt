/*
 * switch_753x.c: set for 753x switch
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#include "switch_netlink.h"
#include "switch_ioctl.h"
#include "switch_fun.h"

struct mt753x_attr *attres;
int chip_name;
bool nl_init_flag;

static void usage(char *cmd)
{
	printf("==================Usage===============================================================================================================================\n");

	/* 1. basic operations */
	printf("1) mt753x switch Basic operations=================================================================================================================>>>>\n");
	printf(" 1.1) %s devs							- list switch device id and model name  \n", cmd);
	printf(" 1.2) %s sysctl							- show the ways to access kenerl driver: netlink or ioctl \n", cmd);
	printf(" 1.3) %s reset							- sw reset switch fsm and registers\n", cmd);
	printf(" 1.4) %s reg r [offset]						- read the reg with default switch \n", cmd);
	printf(" 1.5) %s reg w [offset] [value]					- write the reg with default switch \n", cmd);
	printf(" 1.6) %s reg d [offset]						- dump the reg with default switch\n", cmd);
	printf(" 1.7) %s dev [devid] reg r [addr]				- read the reg with the switch devid  \n", cmd);
	printf(" 1.8) %s dev [devid] reg w [addr] [value] 			- write the regs with the switch devid \n", cmd);
	printf(" 1.9) %s dev [devid] reg d [addr]				- dump the regs with the switch devid \n", cmd);
	printf("																			\n");

	/* 2. phy operations */
	printf("2) mt753x switch PHY operations===================================================================================================================>>>>\n");
	printf(" 2.1) %s phy							- dump all phy registers (clause 22)\n", cmd);
	printf(" 2.2) %s phy [phy_addr]						- dump phy register of specific port (clause 22)\n", cmd);
	printf(" 2.3) %s phy cl22 r [port_num] [phy_reg]			- read specific phy register of specific port by clause 22\n", cmd);
	printf(" 2.4) %s phy cl22 w [port_num] [phy_reg] [value]		- write specific phy register of specific port by clause 22\n", cmd);
	printf(" 2.5) %s phy cl45 r [port_num] [dev_num] [phy_reg]		- read specific phy register of specific port by clause 45\n", cmd);
	printf(" 2.6) %s phy cl45 w [port_num] [dev_num] [phy_reg] [value]	- write specific phy register of specific port by clause 45\n", cmd);
	printf(" 2.7) %s phy fc [port_num] [enable 0|1]				- set switch phy flow control, port is 0~4, enable is 1, disable is 0 \n", cmd);
	printf(" 2.8) %s phy an [port_num] [enable 0|1]				- set switch phy auto-negotiation, port is 0~4, enable is 1, disable is 0 \n", cmd);
	printf(" 2.9) %s trreg r [port_num] [ch_addr] [node_addr] [data_addr]	- read phy token-ring of specific port\n", cmd);
	printf(" 2.10) %s trreg w [port_num] [ch_addr] [node_addr] [data_addr]	- write phy token-ring of specific port\n", cmd);
	printf("		[high_value] [low_value]									\n");
	printf(" 2.11) %s crossover [port_num] [mode auto|mdi|mdix]		- switch auto or force mdi/mdix mode for crossover cable\n", cmd);
	printf("																			\n");

	/* 3. mac operations */
	printf("3) mt753x switch MAC operations====================================================================================================================>>>>\n");
	printf(" 3.1) %s dump							- dump switch mac table\n", cmd);
	printf(" 3.2) %s clear							- clear switch mac table\n", cmd);
	printf(" 3.3) %s add [mac] [portmap]					- add an entry (with portmap) to switch mac table\n", cmd);
	printf(" 3.4) %s add [mac] [portmap] [vlan id]				- add an entry (with portmap, vlan id) to switch mac table\n", cmd);
	printf(" 3.5) %s add [mac] [portmap] [vlan id] [age]			- add an entry (with portmap, vlan id, age out time) to switch mac table\n", cmd);
	printf(" 3.6) %s del mac [mac] vid [vid]				- delete an entry from switch mac table\n", cmd);
	printf(" 3.7) %s del mac [mac] fid [fid]				- delete an entry from switch mac table\n", cmd);
	printf(" 3.8) %s search mac [mac] vid [vid]				- search an entry with specific mac and vid\n", cmd);
	printf(" 3.9) %s search mac [mac] fid [fid]				- search an entry with specific mac and fid\n", cmd);
	printf(" 3.10) %s filt [mac]						- add a SA filtering entry (with portmap 1111111) to switch mac table\n", cmd);
	printf(" 3.11) %s filt [mac] [portmap]					- add a SA filtering entry (with portmap)to switch mac table\n", cmd);
	printf(" 3.12) %s filt [mac] [portmap] [vlan id				- add a SA filtering entry (with portmap, vlan id)to switch mac table\n", cmd);
	printf(" 3.13) %s filt [mac] [portmap] [vlan id] [age]			- add a SA filtering entry (with portmap, vlan id, age out time) to switch table\n", cmd);
	printf(" 3.14) %s arl aging [active:0|1] [time:1~65536]			- set switch arl aging timeout value \n", cmd);
	printf(" 3.15) %s macctl fc [enable|disable]				- set switch mac global flow control,enable is 1, disable is 0 \n", cmd);
	printf("																			\n");

	/* 4. mib counter operations */
	printf("4) mt753x switch mib counter operations============================================================================================================>>>>\n");
	printf(" 4.1) %s esw_cnt get						-get switch mib counters          \n", cmd);
	printf(" 4.2) %s esw_cnt clear						-clear switch mib counters         \n", cmd);
	printf(" 4.3) %s output_queue_cnt get					-get switch output queue counters \n", cmd);
	printf(" 4.4) %s free_page get						-get switch system free page counters  \n", cmd);
	printf("																			\n");

	/* 5. acl function operations */
	printf("5) mt753x switch acl function operations============================================================================================================>>>>\n");
	printf(" 5.1) %s acl enable [port] [port_enable:0|1]			- set switch acl function enabled, port is 0~6,enable is 1, disable is 0  \n", cmd);
	printf(" 5.2) %s acl etype add [ethtype] [portmap]			- drop L2 ethertype packets \n", cmd);
	printf(" 5.3) %s acl dmac add [mac] [portmap]				- drop L2 dest-Mac packets \n", cmd);
	printf(" 5.4) %s acl dip add [dip] [portmap]				- drop dip packets \n", cmd);
	printf(" 5.5) %s acl port add [sport] [portmap]				- drop L4 UDP/TCP source port packets\n", cmd);
	printf(" 5.6) %s acl L4 add [2byes] [portmap]				- drop L4 packets with 2bytes payload\n", cmd);
	printf(" 5.7) %s acl acltbl-add  [tbl_idx:0~63/255] [vawd1] [vawd2]	- set switch acl table new entry, max index-7530:63,7531:255 \n", cmd);
	printf(" 5.8) %s acl masktbl-add [tbl_idx:0~31/127] [vawd1] [vawd2]	- set switch acl mask table new entry, max index-7530:31,7531:127   \n", cmd);
	printf(" 5.9) %s acl ruletbl-add [tbl_idx:0~31/127] [vawd1] [vawd2]	- set switch acl rule table new entry, max index-7530:31,7531:127  \n", cmd);
	printf(" 5.10) %s acl ratetbl-add [tbl_idx:0~31] [vawd1] [vawd2] 	- set switch acl rate table new entry  \n", cmd);
	printf(" 5.11) %s acl dip meter [dip] [portmap][meter:kbps]		- rate limit dip packets \n", cmd);
	printf(" 5.12) %s acl dip trtcm [dip] [portmap][CIR:kbps][CBS][PIR][PBS]- TrTCM dip packets \n", cmd);
	printf(" 5.13) %s acl dip modup [dip] [portmap][usr_pri]		- modify usr priority from ACL \n", cmd);
	printf(" 5.14) %s acl dip pppoe [dip] [portmap]				- pppoe header removal \n", cmd);
	printf("																			\n");

	/* 6. dip table operations */
	printf("6) mt753x switch dip table operations=================================================================================================================>>>>\n");
	printf(" 6.1) %s dip dump						- dump switch dip table\n", cmd);
	printf(" 6.2) %s dip clear						- clear switch dip table\n", cmd);
	printf(" 6.3) %s dip add [dip] [portmap]				- add a dip entry to switch table\n", cmd);
	printf(" 6.4) %s dip del [dip]						- del a dip entry to switch table\n", cmd);
	printf("																			\n");

	/* 7. sip table operations */
	printf("7) mt753x switch sip table operations=================================================================================================================>>>>\n");
	printf(" 7.1) %s sip dump						- dump switch sip table\n", cmd);
	printf(" 7.2) %s sip clear						- clear switch sip table\n", cmd);
	printf(" 7.3) %s sip add [sip] [dip] [portmap]				- add a sip entry to switch table\n", cmd);
	printf(" 7.4) %s sip del [sip] [dip]					- del a sip entry to switch table\n", cmd);
	printf("																			\n");

	/* 8. vlan table operations */
	printf("8) mt753x switch sip table operations====================================================================================================================>>>>\n");
	printf(" 8.1) %s vlan dump (egtag)					- dump switch vlan table (with per port eg_tag setting)\n", cmd);
	printf(" 8.2) %s vlan set [fid:0~7] [vid] [portmap]			- set vlan id and associated member at switch vlan table\n", cmd);
	printf("			([stag:0~4095] [eg_con:0|1] [egtagPortMap 0:untagged 2:tagged]) \n");
	printf("			Full Example: %s vlan set 0 3 10000100 0 0 20000200\n", cmd);
	printf(" 8.3) %s vlan vid [vlan idx] [active:0|1] [vid] [portMap] 	- set switch vlan vid elements  \n", cmd);
	printf("			[egtagPortMap] [ivl_en] [fid] [stag]							 \n");
	printf(" 8.4) %s vlan pvid (dump | [port] [pvid] )				- set switch vlan pvid  \n", cmd);
	printf(" 8.5) %s vlan acc-frm [port] [acceptable_frame_type:0~3]	- set switch vlan acceptable_frame type : admit all frames: 0, \n", cmd);
	printf("									admit only vlan-taged frames: 1,admit only untagged or priority-tagged frames: 2, reserved:3 \n");
	printf(" 8.6) %s vlan port-attr [port] [attr:0~3]			- set switch vlan port attribute: user port: 0, statck port: 1, \n", cmd);
	printf("									translation port: 2, transparent port:3        \n");
	printf(" 8.7) %s vlan port-mode [port] [mode:0~3]			- set switch vlan port mode : port matrix mode: 0, fallback mode: 1,  \n", cmd);
	printf("									check mode: 2, security mode:3                    \n");
	printf(" 8.8) %s vlan eg-tag-pvc [port] [eg_tag:0~7]			- set switch vlan eg tag pvc : disable: 0, consistent: 1, reserved: 2, \n", cmd);
	printf("									reserved:3,untagged:4,swap:5,tagged:6, stack:7                 \n");
	printf(" 8.9) %s vlan eg-tag-pcr [port] [eg_tag:0~3]			- set switch vlan eg tag pcr : untagged: 0, swap: 1, tagged: 2, stack:3 \n", cmd);
	printf("																			\n");

	/* 9. rate limit operations */
	printf("9) mt753x switch rate limit operations=================================================================================================================>>>>\n");
	printf(" 9.1) %s ratectl [in_ex_gress:0|1] [port] [rate]		- set switch port ingress(1) or egress(0) rate  \n", cmd);
	printf(" 9.2) %s ingress-rate on [port] [Kbps]				- set ingress rate limit on port n (n= 0~ switch max port) \n", cmd);
	printf(" 9.3) %s egress-rate on [port] [Kbps]				- set egress rate limit on port n (n= 0~ switch max port) \n", cmd);
	printf(" 9.4) %s ingress-rate off [port]				- disable ingress rate limit on port n (n= 0~ switch max port) \n", cmd);
	printf(" 9.5) %s egress-rate off [port]					- disable egress rate limit on port n (n= 0~ switch max port)\n", cmd);
	printf("																			\n");

	/* 10. igmp operations */
	printf("10) mt753x igmp operations===============================================================================================================================>>>>\n");
	printf(" 10.1) %s igmpsnoop on [leaky_en] [wan_num]			- turn on IGMP snoop and router port learning\n", cmd);
	printf("									leaky_en: 1 or 0. default 0; wan_num: 0 or 4. default 4\n");
	printf(" 10.2) %s igmpsnoop off						- turn off IGMP snoop and router port learning\n", cmd);
	printf(" 10.3) %s igmpsnoop enable [port#]				- enable IGMP HW leave/join/Squery/Gquery\n", cmd);
	printf(" 10.4) %s igmpsnoop disable [port#]				- disable IGMP HW leave/join/Squery/Gquery\n", cmd);
	printf("																			\n");

	/* 11. QoS operations */
	printf("11) mt753x QoS operations================================================================================================================================>>>>\n");
	printf(" 11.1) %s qos sch [port:0~6] [queue:0~7] [shaper:min|max] [type:rr:0|sp:1|wfq:2]     - set switch qos sch type\n", cmd);
	printf(" 11.2) %s qos base [port:0~6] [base]					- set switch qos base(UPW); port-based:0, tag-based:1, \n", cmd);
	printf("									dscp-based:2, acl-based:3, arl-based:4, stag-based:5   \n");
	printf(" 11.3) %s qos port-weight [port:0~6] [q0] [q1][q2][q3]		- set switch qos port queue weight; \n", cmd);
	printf("				[q4][q5][q6][q7]				 [qn]: the weight of queue n, range: 1~16     \n");
	printf(" 11.4) %s qos port-prio [port:0~6] [prio:0~7]			- set switch port qos user priority;  port is 0~6, priority is 0~7  \n", cmd);
	printf(" 11.5) %s qos dscp-prio [dscp:0~63] [prio:0~7]			- set switch qos dscp user priority;  dscp is 0~63, priority is 0~7  \n", cmd);
	printf(" 11.6) %s qos prio-qmap [port:0~6] [prio:0~7]  [queue:0~7]			- set switch qos priority queue map; priority is 0~7,queue is 0~7  \n", cmd);
	printf("																			\n");

	/*12. port mirror operations*/
	printf(" 12) mt753x port mirror operations========================================================================================================================>>>>\n");
	printf(" 12.1) %s mirror monitor [port]					- enable port mirror and indicate monitor port number\n", cmd);
	printf(" 12.2) %s mirror target  [port]					- set port mirror target\n", cmd);
	printf("			[direction| 0:off, 1:rx, 2:tx, 3:all]					\n");
	printf(" 12.3) %s mirror enable [mirror_en:0|1] [mirror_port: 0-6]	- set switch mirror function enable(1) or disabled(0) for port 0~6  \n", cmd);
	printf(" 12.4) %s mirror port-based [port] [port_tx_mir:0|1]		- set switch mirror port: target tx/rx/acl/vlan/igmp\n", cmd);
	printf("				[port_rx_mir:0|1] [acl_mir:0|1]						\n");
	printf("				[vlan_mis:0|1] [igmp_mir:0|1]						\n");
	printf("																			\n");

	/*13. stp function*/
	printf(" 13) mt753x stp operations===============================================================================================================================>>>>\n");
	printf(" 13.1) %s stp [port] [fid] [state]				- set switch spanning tree state, port is 0~6, fid is 0~7,  \n", cmd);
	printf("									state is 0~3(Disable/Discarding:0,Blocking/Listening/Discarding:1,) \n");
	printf("									Learning:2,Forwarding:3 \n");
	printf("																			\n");

	/*14. collision pool operations*/
	printf("14) mt753x collision pool operations========================================================================================================================>>>>\n");
	printf(" 14.1) %s collision-pool enable [enable 0|1]			- enable or disable collision pool\n", cmd);
	printf(" 14.2) %s collision-pool mac dump				- dump collision pool mac table\n", cmd);
	printf(" 14.3) %s collision-pool dip dump				- dump collision pool dip table\n", cmd);
	printf(" 14.4) %s collision-pool sip dump				- dump collision pool sip table\n", cmd);
	printf("																			\n");

	/*15. pfc(priority flow control) operations*/
	printf("15) mt753x pfc(priority flow control) operations==============================================================================================================>>>>\n");
	printf(" 15.1) %s pfc enable [port] [enable 0|1]			- enable or disable port's pfc \n", cmd);
	printf(" 15.2) %s pfc rx_counter [port]					- get port n pfc 8 up rx counter \n", cmd);
	printf(" 15.3) %s pfc tx_counter [port]					- get port n pfc 8 up rx counter \n", cmd);
	printf("																			\n");

	/*15. pfc(priority flow control) operations*/
	printf("16) mt753x EEE(802.3az) operations==============================================================================================================>>>>\n");
	printf(" 16.1) %s eee enable [enable 0|1] ([portMap])			- enable or disable EEE (by portMap)\n", cmd);
	printf(" 16.2) %s eee dump ([port])					- dump EEE capability (by port)\n", cmd);
	printf("																			\n");

	exit_free();
	exit(0);
}

static void parse_reg_cmd(int argc, char *argv[], int len)
{
	unsigned int val;
	unsigned int off;
	int i, j;

	if (!strncmp(argv[len - 3], "reg", 4)) {
		if (argv[len - 2][0] == 'r') {
			off = strtoul(argv[len - 1], NULL, 16);
			reg_read(off, &val);
			printf(" Read reg=%x, value=%x\n", off, val);
		} else if (argv[len - 2][0] == 'w') {
			off = strtoul(argv[len - 1], NULL, 16);
			if (argc != len + 1)
				usage(argv[0]);
			val = strtoul(argv[len], NULL, 16);
			reg_write(off, val);
			printf(" Write reg=%x, value=%x\n", off, val);
		} else if (argv[len - 2][0] == 'd') {
			off = strtoul(argv[len - 1], NULL, 16);
			for (i = 0; i < 16; i++) {
				printf("0x%08x: ", off + 0x10 * i);
				for (j = 0; j < 4; j++) {
					reg_read(off + i * 0x10 + j * 0x4, &val);
					printf(" 0x%08x", val);
				}
				printf("\n");
			}
		} else
			usage(argv[0]);
	} else
		usage(argv[0]);
}

static int get_chip_name()
{
	int temp;
	FILE *fp = NULL;
	char buff[255];

	/*judge 7530*/
	reg_read((0x7ffc), &temp);
	temp = temp >> 16;
	if (temp == 0x7530)
		return temp;
	/*judge 7531*/
	reg_read(0x781c, &temp);
	temp = temp >> 16;
	if (temp == 0x7531)
		return temp;

	/*judge jaguar embedded switch*/
	fp = fopen("/proc/device-tree/compatible", "r");
	if (fp != NULL) {
		temp = -1;
		if (fgets(buff, 255, (FILE *)fp) && strstr(buff, "mt7988"))
			temp = 0x7988;

		fclose(fp);
		return temp;
	}

	return -1;
}

static int phy_operate(int argc, char *argv[])
{
	unsigned int port_num;
	unsigned int dev_num;
	unsigned int value, cl_value;
	unsigned int reg;
	int ret = 0, cl_ret = 0;
	char op;

	if (strncmp(argv[2], "cl22", 4) && strncmp(argv[2], "cl45", 4))
		usage(argv[0]);

	op = argv[3][0];

	switch(op) {
		case 'r':
			reg = strtoul(argv[argc-1], NULL, 0);
			if (argc == 6) {
				port_num = strtoul(argv[argc-2], NULL, 0);
				ret = mii_mgr_read(port_num, reg, &value);
				if (ret < 0)
					printf(" Phy read reg fail\n");
				else
					printf(" Phy read reg=0x%x, value=0x%x\n", reg, value);
			} else if (argc == 7) {
				dev_num = strtoul(argv[argc-2], NULL, 0);
				port_num = strtoul(argv[argc-3], NULL, 0);
				ret = mii_mgr_c45_read(port_num, dev_num, reg, &value);
				if (ret < 0)
					printf(" Phy read reg fail\n");
				else
					printf(" Phy read reg=0x%x, value=0x%x\n", reg, value);
			} else
				ret = phy_dump(32);
			break;
		case 'w':
			reg = strtoul(argv[argc-2], NULL, 0);
			value = strtoul(argv[argc-1], NULL, 0);
			if (argc == 7) {
				port_num = strtoul(argv[argc-3], NULL, 0);
				ret = mii_mgr_write(port_num, reg, value);
				cl_ret = mii_mgr_read(port_num, reg, &cl_value);
				if (cl_ret < 0)
					printf(" Phy read reg fail\n");
				else
					printf(" Phy read reg=0x%x, value=0x%x\n", reg, cl_value);
			}
			else if (argc == 8) {
				dev_num = strtoul(argv[argc-3], NULL, 0);
				port_num = strtoul(argv[argc-4], NULL, 0);
				ret = mii_mgr_c45_write(port_num, dev_num, reg, value);
				cl_ret = mii_mgr_c45_read(port_num, dev_num, reg, &cl_value);
				if (cl_ret < 0)
					printf(" Phy read reg fail\n");
				else
					printf(" Phy read reg=0x%x, value=0x%x\n", reg, cl_value);
			}
			else
				usage(argv[0]);
			break;
		default:
			break;
	}

	return ret;
}


int main(int argc, char *argv[])
{
	int err;

	attres = (struct mt753x_attr *)malloc(sizeof(struct mt753x_attr));
	attres->dev_id = -1;
	attres->port_num = -1;
	attres->phy_dev = -1;
	nl_init_flag = true;

	/* dsa netlink family might not be enabled. Try gsw netlink family. */
	err = mt753x_netlink_init(MT753X_DSA_GENL_NAME);
	if (!err)
		chip_name = get_chip_name();

	if (err < 0) {
		err = mt753x_netlink_init(MT753X_GENL_NAME);
		if (!err)
			chip_name = get_chip_name();
	}
	
	if (err < 0) {
		err = switch_ioctl_init();
		if (!err) {
			nl_init_flag = false;
			chip_name = get_chip_name();
			if (chip_name < 0) {
				printf("no chip unsupport or chip id is invalid!\n");
				exit_free();
				exit(0);
			}
		}
	}

	if (argc < 2)
		usage(argv[0]);

	if (!strcmp(argv[1], "dev")) {
		attres->dev_id = strtoul(argv[2], NULL, 0);
		argv += 2;
		argc -= 2;
		if (argc < 2)
			usage(argv[0]);

	}

	if (argc == 2) {
		if (!strcmp(argv[1], "devs")) {
			attres->type = MT753X_ATTR_TYPE_MESG;
			mt753x_list_swdev(attres, MT753X_CMD_REQUEST);
		} else if (!strncmp(argv[1], "dump", 5)) {
			table_dump();
		} else if (!strncmp(argv[1], "clear", 6)) {
			table_clear();
			printf("done.\n");
		} else if (!strncmp(argv[1], "reset", 5)) {
			switch_reset(argc, argv);
		} else if (!strncmp(argv[1], "phy", 4)) {
			phy_dump(32); //dump all phy register
		} else if (!strncmp(argv[1], "sysctl", 7)) {
			if (nl_init_flag)
				printf("netlink(%s)\n",MT753X_GENL_NAME);
			else
				printf("ioctl(%s)\n",ETH_DEVNAME);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "arl", 4)) {
		if (!strncmp(argv[2], "aging", 6))
			doArlAging(argc, argv);
	} else if (!strncmp(argv[1], "esw_cnt", 8)) {
		if (!strncmp(argv[2], "get", 4))
			read_mib_counters();
		else if (!strncmp(argv[2], "clear", 6))
			clear_mib_counters();
		else
			usage(argv[0]);
	}else if (!strncmp(argv[1], "output_queue_cnt", 17)) {
		if (!strncmp(argv[2], "get", 4))
			read_output_queue_counters();
		else
			usage(argv[0]);
	}else if (!strncmp(argv[1], "free_page", 10)) {
		if (!strncmp(argv[2], "get", 4))
			read_free_page_counters();
		else
			usage(argv[0]);
	}
	else if (!strncmp(argv[1], "ratectl", 8))
		rate_control(argc, argv);
	else if (!strncmp(argv[1], "add", 4))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "filt", 5))
		table_add(argc, argv);
	else if (!strncmp(argv[1], "del", 4)) {
		if (!strncmp(argv[4], "fid", 4))
			table_del_fid(argc, argv);
		else if (!strncmp(argv[4], "vid", 4))
			table_del_vid(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "search", 7)) {
		if (!strncmp(argv[4], "fid", 4))
			table_search_mac_fid(argc, argv);
		else if (!strncmp(argv[4], "vid", 4))
			table_search_mac_vid(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "phy", 4)) {
		if (argc == 3) {
			int phy_addr = strtoul(argv[2], NULL, 0);
			if (phy_addr < 0 || phy_addr > 31)
				usage(argv[0]);
			phy_dump(phy_addr);
		} else if (argc == 5) {
			if (!strncmp(argv[2], "fc", 2))
				phy_set_fc(argc, argv);
			else if (!strncmp(argv[2], "an", 2))
				phy_set_an(argc, argv);
			else
				phy_dump(32);
		} else
			phy_operate(argc, argv);
	} else if (!strncmp(argv[1], "trreg", 4)) {
		if (rw_phy_token_ring(argc, argv) < 0)
			usage(argv[0]);
	} else if (!strncmp(argv[1], "macctl", 7)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "fc", 3))
			global_set_mac_fc(argc, argv);
		else if (!strncmp(argv[2], "pfc", 4))
			set_mac_pfc(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "qos", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "sch", 4))
			qos_sch_select(argc, argv);
		else if (!strncmp(argv[2], "base", 5))
			qos_set_base(argc, argv);
		else if (!strncmp(argv[2], "port-weight", 12))
			qos_wfq_set_weight(argc, argv);
		else if (!strncmp(argv[2], "port-prio", 10))
			qos_set_portpri(argc, argv);
		else if (!strncmp(argv[2], "dscp-prio", 10))
			qos_set_dscppri(argc, argv);
		else if (!strncmp(argv[2], "prio-qmap", 10))
			qos_pri_mapping_queue(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "stp", 3)) {
		if (argc < 3)
			usage(argv[0]);
		else
			doStp(argc, argv);
	} else if (!strncmp(argv[1], "sip", 5)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			sip_dump();
		else if (!strncmp(argv[2], "add", 4))
			sip_add(argc, argv);
		else if (!strncmp(argv[2], "del", 4))
			sip_del(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			sip_clear();
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "dip", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			dip_dump();
		else if (!strncmp(argv[2], "add", 4))
			dip_add(argc, argv);
		else if (!strncmp(argv[2], "del", 4))
			dip_del(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			dip_clear();
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "mirror", 7)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "monitor", 8))
			set_mirror_to(argc, argv);
		else if (!strncmp(argv[2], "target", 7))
			set_mirror_from(argc, argv);
		else if (!strncmp(argv[2], "enable", 7))
			doMirrorEn(argc, argv);
		else if (!strncmp(argv[2], "port-based", 11))
			doMirrorPortBased(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "acl", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dip", 4)) {
			if (!strncmp(argv[3], "add", 4))
				acl_dip_add(argc, argv);
			else if (!strncmp(argv[3], "modup", 6))
				acl_dip_modify(argc, argv);
			else if (!strncmp(argv[3], "pppoe", 6))
				acl_dip_pppoe(argc, argv);
			else if (!strncmp(argv[3], "trtcm", 4))
				acl_dip_trtcm(argc, argv);
			else if (!strncmp(argv[3], "meter", 6))
				acl_dip_meter(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "dmac", 6)) {
			if (!strncmp(argv[3], "add", 4))
				acl_mac_add(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "etype", 6)) {
			if (!strncmp(argv[3], "add", 4))
				acl_ethertype(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "port", 5)) {
			if (!strncmp(argv[3], "add", 4))
				acl_sp_add(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "L4", 5)) {
			if (!strncmp(argv[3], "add", 4))
				acl_l4_add(argc, argv);
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "enable", 7))
			acl_port_enable(argc, argv);
		else if (!strncmp(argv[2], "acltbl-add", 11))
			acl_table_add(argc, argv);
		else if (!strncmp(argv[2], "masktbl-add", 12))
			acl_mask_table_add(argc, argv);
		else if (!strncmp(argv[2], "ruletbl-add", 12))
			acl_rule_table_add(argc, argv);
		else if (!strncmp(argv[2], "ratetbl-add", 12))
			acl_rate_table_add(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "vlan", 5)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "dump", 5))
			vlan_dump(argc, argv);
		else if (!strncmp(argv[2], "set", 4))
			vlan_set(argc, argv);
		else if (!strncmp(argv[2], "clear", 6))
			vlan_clear(argc, argv);
		else if (!strncmp(argv[2], "vid", 4))
			doVlanSetVid(argc, argv);
		else if (!strncmp(argv[2], "pvid", 5))
			doVlanSetPvid(argc, argv);
		else if (!strncmp(argv[2], "acc-frm", 8))
			doVlanSetAccFrm(argc, argv);
		else if (!strncmp(argv[2], "port-attr", 10))
			doVlanSetPortAttr(argc, argv);
		else if (!strncmp(argv[2], "port-mode", 10))
			doVlanSetPortMode(argc, argv);
		else if (!strncmp(argv[2], "eg-tag-pcr", 11))
			doVlanSetEgressTagPCR(argc, argv);
		else if (!strncmp(argv[2], "eg-tag-pvc", 11))
			doVlanSetEgressTagPVC(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "reg", 4)) {
		parse_reg_cmd(argc, argv, 4);
	} else if (!strncmp(argv[1], "ingress-rate", 6)) {
		int port = 0, bw = 0;
		if (argv[2][1] == 'n') {
			port = strtoul(argv[3], NULL, 0);
			bw = strtoul(argv[4], NULL, 0);
			if (ingress_rate_set(1, port, bw) == 0)
				printf("switch port=%d, bw=%d\n", port, bw);
		}
		else if (argv[2][1] == 'f') {
			if (argc != 4)
				usage(argv[0]);
			port = strtoul(argv[3], NULL, 0);
			if (ingress_rate_set(0, port, bw) == 0)
				printf("switch port=%d ingress rate limit off\n", port);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "egress-rate", 6)) {
		int port = 0, bw = 0;
		if (argv[2][1] == 'n') {
			port = strtoul(argv[3], NULL, 0);
			bw = strtoul(argv[4], NULL, 0);
			if (egress_rate_set(1, port, bw) == 0)
				printf("switch port=%d, bw=%d\n", port, bw);
		} else if (argv[2][1] == 'f') {
			if (argc != 4)
				usage(argv[0]);
			port = strtoul(argv[3], NULL, 0);
			if (egress_rate_set(0, port, bw) == 0)
				printf("switch port=%d egress rate limit off\n", port);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "igmpsnoop", 10)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "on", 3))
			igmp_on(argc, argv);
		else if (!strncmp(argv[2], "off", 4))
			igmp_off();
		else if (!strncmp(argv[2], "enable", 7))
			igmp_enable(argc, argv);
		else if (!strncmp(argv[2], "disable", 8))
			igmp_disable(argc, argv);
		else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "collision-pool", 15)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "enable", 7))
			collision_pool_enable(argc, argv);
		else if (!strncmp(argv[2], "mac", 4)){
			if (!strncmp(argv[3], "dump", 5))
				collision_pool_mac_dump();
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "dip", 4)){
			if (!strncmp(argv[3], "dump", 5))
				collision_pool_dip_dump();
			else
				usage(argv[0]);
		} else if (!strncmp(argv[2], "sip", 4)){
			if (!strncmp(argv[3], "dump", 5))
				collision_pool_sip_dump();
			else
				usage(argv[0]);
			}
		else
			usage(argv[0]);
	}  else if (!strncmp(argv[1], "pfc", 15)) {
		if (argc < 4 || argc > 5)
			usage(argv[0]);
		if (!strncmp(argv[2], "enable", 7))
			set_mac_pfc(argc, argv);
		else if (!strncmp(argv[2], "rx_counter", 11)){
			pfc_get_rx_counter(argc, argv);
		} else if (!strncmp(argv[2], "tx_counter", 11)){
			pfc_get_tx_counter(argc, argv);
		} else
			usage(argv[0]);
	} else if (!strncmp(argv[1], "crossover", 10)) {
		if (argc < 4)
			usage(argv[0]);
		else
			phy_crossover(argc, argv);
	} else if (!strncmp(argv[1], "eee", 4)) {
		if (argc < 3)
			usage(argv[0]);
		if (!strncmp(argv[2], "enable", 7) ||
			 !strncmp(argv[2], "disable", 8))
			eee_enable(argc, argv);
		else if (!strncmp(argv[2], "dump", 5))
			eee_dump(argc, argv);
		else
			usage(argv[0]);
	} else
		usage(argv[0]);

	exit_free();
	return 0;
}
