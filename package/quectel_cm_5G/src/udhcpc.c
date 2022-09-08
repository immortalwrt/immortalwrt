/******************************************************************************
  @file    udhcpc.c
  @brief   call DHCP tools to obtain IP address.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>

#include "util.h"
#include "QMIThread.h"

static int ql_system(const char *shell_cmd) {
    dbg_time("%s", shell_cmd);
    return system(shell_cmd);
}

static void ifc_init_ifr(const char *name, struct ifreq *ifr)
{
    memset(ifr, 0, sizeof(struct ifreq));
    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    ifr->ifr_name[IFNAMSIZ - 1] = 0;
}

static void ql_set_mtu(const char *ifname, int ifru_mtu) {
    int inet_sock;
    struct ifreq ifr;

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (inet_sock > 0) {
        ifc_init_ifr(ifname, &ifr);

        if (!ioctl(inet_sock, SIOCGIFMTU, &ifr)) {
            if (ifr.ifr_ifru.ifru_mtu != ifru_mtu) {
                dbg_time("change mtu %d -> %d", ifr.ifr_ifru.ifru_mtu , ifru_mtu);
                ifr.ifr_ifru.ifru_mtu = ifru_mtu;
                ioctl(inet_sock, SIOCSIFMTU, &ifr);
            }
        }

        close(inet_sock);
    }
}

static int ifc_get_addr(const char *name, in_addr_t *addr)
{
    int inet_sock;
    struct ifreq ifr;
    int ret = 0;

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0);

    ifc_init_ifr(name, &ifr);
    if (addr != NULL) {
        ret = ioctl(inet_sock, SIOCGIFADDR, &ifr);
        if (ret < 0) {
            *addr = 0;
        } else {
            *addr = ((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr.s_addr;
        }
    }
    return ret;
}

static int ql_raw_ip_mode_check(const char *ifname) {
    int fd;
    in_addr_t addr;
    char raw_ip[128];
    char shell_cmd[128];
    char mode[2] = "X";
    int mode_change = 0;

    ifc_get_addr(ifname, &addr);
    if (addr)
        return 0;
    
    snprintf(raw_ip, sizeof(raw_ip), "/sys/class/net/%s/qmi/raw_ip", ifname);
    if (access(raw_ip, F_OK))
        return 0;

    fd = open(raw_ip, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (fd < 0) {
        dbg_time("%s %d fail to open(%s), errno:%d (%s)", __FILE__, __LINE__, raw_ip, errno, strerror(errno));
        return 0;
    }

    read(fd, mode, 2);
    if (mode[0] == '0' || mode[0] == 'N') {
        dbg_time("File:%s Line:%d udhcpc fail to get ip address, try next:", __func__, __LINE__);
        snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s down", ifname);
        ql_system(shell_cmd);
        dbg_time("echo Y > /sys/class/net/%s/qmi/raw_ip", ifname);
        mode[0] = 'Y';
        write(fd, mode, 2);
        mode_change = 1;
        snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s up", ifname);
        ql_system(shell_cmd);
    }

    close(fd);
    return mode_change;
}

static void* udhcpc_thread_function(void* arg) {
    FILE * udhcpc_fp;
    char *udhcpc_cmd = (char *)arg;

    if (udhcpc_cmd == NULL)
        return NULL;

    dbg_time("%s", udhcpc_cmd);
    udhcpc_fp = popen(udhcpc_cmd, "r");
    free(udhcpc_cmd);
    if (udhcpc_fp) {
        char buf[0xff];

        buf[sizeof(buf)-1] = '\0';
        while((fgets(buf, sizeof(buf)-1, udhcpc_fp)) != NULL) {
            if ((strlen(buf) > 1) && (buf[strlen(buf) - 1] == '\n'))
                buf[strlen(buf) - 1] = '\0';
            dbg_time("%s", buf);
        }

        pclose(udhcpc_fp);
    }

    return NULL;
}

//#define USE_DHCLIENT
#ifdef USE_DHCLIENT
static int dhclient_alive = 0;
#endif
static int dibbler_client_alive = 0;

void ql_get_driver_rmnet_info(PROFILE_T *profile, RMNET_INFO *rmnet_info) {
    int ifc_ctl_sock;
    struct ifreq ifr;
    int rc;
    int request = 0x89F3;
    unsigned char data[512];

    memset(rmnet_info, 0x00, sizeof(*rmnet_info));

    ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ifc_ctl_sock <= 0) {
        dbg_time("socket() failed: %s\n", strerror(errno));
        return;
    }
    
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, profile->usbnet_adapter, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;    
    ifr.ifr_ifru.ifru_data = (void *)data;
        
    rc = ioctl(ifc_ctl_sock, request, &ifr);
    if (rc < 0) {
        dbg_time("ioctl(0x%x, qmap_settings) failed: %s, rc=%d", request, strerror(errno), rc);
    }
    else {
        memcpy(rmnet_info, data, sizeof(*rmnet_info));
    }

    close(ifc_ctl_sock);
}

void ql_set_driver_qmap_setting(PROFILE_T *profile, QMAP_SETTING *qmap_settings) {
    int ifc_ctl_sock;
    struct ifreq ifr;
    int rc;
    int request = 0x89F2;

    ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ifc_ctl_sock <= 0) {
        dbg_time("socket() failed: %s\n", strerror(errno));
        return;
    }
    
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, profile->usbnet_adapter, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;    
    ifr.ifr_ifru.ifru_data = (void *)qmap_settings;
        
    rc = ioctl(ifc_ctl_sock, request, &ifr);
    if (rc < 0) {
        dbg_time("ioctl(0x%x, qmap_settings) failed: %s, rc=%d", request, strerror(errno), rc);
    }

    close(ifc_ctl_sock);	
}

void ql_set_driver_link_state(PROFILE_T *profile, int link_state) {
    char link_file[128];
    int fd;
    int new_state = 0;

    snprintf(link_file, sizeof(link_file), "/sys/class/net/%s/link_state", profile->usbnet_adapter);
    fd = open(link_file, O_RDWR | O_NONBLOCK | O_NOCTTY);
    if (fd == -1) {
        if (errno != ENOENT)
            dbg_time("Fail to access %s, errno: %d (%s)", link_file, errno, strerror(errno));
        return;
    }

    if (profile->qmap_mode <= 1)
        new_state = !!link_state;
    else {
        //0x80 means link off this pdp
        new_state = (link_state ? 0x00 : 0x80) + profile->pdp;
    }

    snprintf(link_file, sizeof(link_file), "%d\n", new_state);
    write(fd, link_file, sizeof(link_file));

    if (link_state == 0 && profile->qmap_mode > 1) {
        size_t rc;
        
        lseek(fd, 0, SEEK_SET);
        rc = read(fd, link_file, sizeof(link_file));
        if (rc > 1 && (!strcasecmp(link_file, "0\n") || !strcasecmp(link_file, "0x0\n"))) {
            snprintf(link_file, sizeof(link_file), "ifconfig %s down", profile->usbnet_adapter);
            ql_system(link_file);
        }
    }

    close(fd);
}

void udhcpc_start(PROFILE_T *profile) {
    char *ifname = profile->usbnet_adapter;
    char shell_cmd[128];

    ql_set_driver_link_state(profile, 1);

    if (profile->qmapnet_adapter) {
        ifname = profile->qmapnet_adapter;
    }

    if (profile->rawIP && profile->ipv4.Address && profile->ipv4.Mtu) {
        ql_set_mtu(ifname, (profile->ipv4.Mtu));
    }

    if (strcmp(ifname, profile->usbnet_adapter)) {
        snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s up", profile->usbnet_adapter);
        ql_system(shell_cmd);
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s up", ifname);
    ql_system(shell_cmd);
	
	if (profile->ipv4.Address) {
		unsigned char *pcscf1 = (unsigned char *)&profile->PCSCFIpv4Addr1;
		unsigned char *pcscf2 = (unsigned char *)&profile->PCSCFIpv4Addr2;
	
		if (pcscf1 && *pcscf1)
			dbg_time("pcscf1: %d.%d.%d.%d", pcscf1[3], pcscf1[2], pcscf1[1], pcscf1[0]);
		if (pcscf2 && *pcscf2)
			dbg_time("pcscf2: %d.%d.%d.%d", pcscf2[3], pcscf2[2], pcscf2[1], pcscf2[0]);
	}

    if (profile->ipv6.Address[0] && profile->ipv6.PrefixLengthIPAddr) {
	    unsigned char *pcscf1 = (unsigned char *)&profile->PCSCFIpv6Addr1;
	    unsigned char *pcscf2 = (unsigned char *)&profile->PCSCFIpv6Addr2;
	    
	    if (pcscf1 && pcscf1[0])
	        dbg_time("pcscf1: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", 
	            pcscf1[0], pcscf1[1], pcscf1[2], pcscf1[3], pcscf1[4], pcscf1[5], pcscf1[6], pcscf1[7], pcscf1[8], 
	            pcscf1[9], pcscf1[10], pcscf1[11], pcscf1[12], pcscf1[13], pcscf1[14], pcscf1[15]);
	    if (pcscf2 && pcscf2[0])
	        dbg_time("pcscf2: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", 
	            pcscf2[0], pcscf2[1], pcscf2[2], pcscf2[3], pcscf2[4], pcscf2[5], pcscf2[6], pcscf2[7], pcscf2[8], 
	            pcscf2[9], pcscf2[10], pcscf2[11], pcscf2[12], pcscf2[13], pcscf2[14], pcscf2[15]);
	}

#if 1 //for bridge mode, only one public IP, so do udhcpc manually
    if (ql_bridge_mode_detect(profile)) {
        return;
    }
#endif

//because must use udhcpc to obtain IP when working on ETH mode,
//so it is better also use udhcpc to obtain IP when working on IP mode.
//use the same policy for all modules
#if 0
    if (profile->rawIP != 0) //mdm9x07/ec25,ec20 R2.0
    {
        if (profile->ipv4.Address) {
            unsigned char *ip = (unsigned char *)&profile->ipv4.Address;
            unsigned char *gw = (unsigned char *)&profile->ipv4.Gateway;
            unsigned char *netmask = (unsigned char *)&profile->ipv4.SubnetMask;
            unsigned char *dns1 = (unsigned char *)&profile->ipv4.DnsPrimary;
            unsigned char *dns2 = (unsigned char *)&profile->ipv4.DnsSecondary;

            snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s %d.%d.%d.%d netmask %d.%d.%d.%d",ifname,
                ip[3], ip[2], ip[1], ip[0], netmask[3], netmask[2], netmask[1], netmask[0]);
            ql_system(shell_cmd);

            //Resetting default routes
            snprintf(shell_cmd, sizeof(shell_cmd), "route del default gw 0.0.0.0 dev %s", ifname);
            while(!system(shell_cmd));

            snprintf(shell_cmd, sizeof(shell_cmd), "route add default gw %d.%d.%d.%d dev %s metric 0", gw[3], gw[2], gw[1], gw[0], ifname);
            ql_system(shell_cmd);

            //Adding DNS
            if (profile->ipv4.DnsSecondary == 0)
                profile->ipv4.DnsSecondary = profile->ipv4.DnsPrimary;

            if (dns1[0]) {
                dbg_time("Adding DNS %d.%d.%d.%d %d.%d.%d.%d", dns1[3], dns1[2], dns1[1], dns1[0], dns2[3], dns2[2], dns2[1], dns2[0]);
                snprintf(shell_cmd, sizeof(shell_cmd), "echo -n \"nameserver %d.%d.%d.%d\nnameserver %d.%d.%d.%d\n\" > /etc/resolv.conf",
                    dns1[3], dns1[2], dns1[1], dns1[0], dns2[3], dns2[2], dns2[1], dns2[0]);
                system(shell_cmd);
            }
        }


        if (profile->ipv6.Address[0] && profile->ipv6.PrefixLengthIPAddr) {
            unsigned char *ip = (unsigned char *)profile->ipv6.Address;
#if 1
            snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s inet6 add %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d",
                ifname, ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15], profile->ipv6.PrefixLengthIPAddr);
#else
            snprintf(shell_cmd, sizeof(shell_cmd), "ip -6 addr add %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d dev %s",
                ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15], profile->ipv6.PrefixLengthIPAddr, ifname);
#endif
            ql_system(shell_cmd);
            snprintf(shell_cmd, sizeof(shell_cmd), "route -A inet6 add default dev %s", ifname);
            ql_system(shell_cmd);
        }
        return;
    }
#endif

/* Do DHCP using busybox tools */
    {
        char udhcpc_cmd[128];
        pthread_attr_t udhcpc_thread_attr;
        pthread_t udhcpc_thread_id;

        pthread_attr_init(&udhcpc_thread_attr);
        pthread_attr_setdetachstate(&udhcpc_thread_attr, PTHREAD_CREATE_DETACHED);

        if (profile->ipv4.Address) {
#ifdef USE_DHCLIENT
            snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "dhclient -4 -d --no-pid %s", ifname);
            dhclient_alive++;
#else
            if (access("/usr/share/udhcpc/default.script", X_OK)) {
                dbg_time("Fail to access /usr/share/udhcpc/default.script, errno: %d (%s)", errno, strerror(errno));
            }

            //-f,--foreground    Run in foreground
            //-b,--background    Background if lease is not obtained
            //-n,--now        Exit if lease is not obtained
            //-q,--quit        Exit after obtaining lease
            //-t,--retries N        Send up to N discover packets (default 3)
            snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "busybox udhcpc -f -n -q -t 5 -i %s", ifname);
#endif

#if 1 //for OpenWrt
            if (!access("/lib/netifd/dhcp.script", X_OK) && !access("/sbin/ifup", X_OK) && !access("/sbin/ifstatus", X_OK)) {
                dbg_time("you are use OpenWrt?");
                dbg_time("should not calling udhcpc manually?");
                dbg_time("should modify /etc/config/network as below?");
                dbg_time("config interface wan");
                dbg_time("\toption ifname	%s", ifname);
                dbg_time("\toption proto	dhcp");
                dbg_time("should use \"/sbin/ifstaus wan\" to check %s 's status?", ifname);
            }
#endif

#ifdef USE_DHCLIENT            
            pthread_create(&udhcpc_thread_id, &udhcpc_thread_attr, udhcpc_thread_function, (void*)strdup(udhcpc_cmd));
            sleep(1);
#else
            pthread_create(&udhcpc_thread_id, NULL, udhcpc_thread_function, (void*)strdup(udhcpc_cmd));
            pthread_join(udhcpc_thread_id, NULL);
            if (ql_raw_ip_mode_check(ifname)) {
                pthread_create(&udhcpc_thread_id, NULL, udhcpc_thread_function, (void*)strdup(udhcpc_cmd));
                pthread_join(udhcpc_thread_id, NULL);
            }
#endif
        }

        if (profile->ipv6.Address[0] && profile->ipv6.PrefixLengthIPAddr) {
            const unsigned char *d = (unsigned char *)&profile->ipv6.Address;
#if 1
			//module do not support DHCPv6, only support 'Router Solicit'
			//and it seem if enable /proc/sys/net/ipv6/conf/all/forwarding, Kernel do not send RS
			const char *forward_file = "/proc/sys/net/ipv6/conf/all/forwarding";
			int forward_fd = open(forward_file, O_RDONLY);
			if (forward_fd > 0) {
				char forward_state[2];
				read(forward_fd, forward_state, 2);
				if (forward_state[0] == '1') {
					//dbg_time("%s enabled, kernel maybe donot send 'Router Solicit'", forward_file);
				}
				close(forward_fd);
			}

            snprintf(shell_cmd, sizeof(shell_cmd),
                "ip -%d address add %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%u dev %s",
                6, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                profile->ipv6.PrefixLengthIPAddr, ifname);
            ql_system(shell_cmd);

            //ping6 www.qq.com
            d = (unsigned char *)&profile->ipv6.Gateway;
            snprintf(shell_cmd, sizeof(shell_cmd),
                "ip -%d route add default via %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x dev %s",
                6, d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15],
                ifname);
            ql_system(shell_cmd);

            if (profile->ipv6.DnsPrimary[0] || profile->ipv6.DnsSecondary[0]) {
                char dns1str[128], dns2str[128];

                if (profile->ipv6.DnsPrimary[0]) {
                    d = profile->ipv6.DnsPrimary;
                    snprintf(dns1str, sizeof(dns1str), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
                }

                if (profile->ipv6.DnsSecondary[0]) {
                    d = profile->ipv6.DnsSecondary;
                    snprintf(dns2str, sizeof(dns2str), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7], d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
                }
                
                update_resolv_conf(6, ifname, profile->ipv6.DnsPrimary[0] ? dns1str : NULL, profile->ipv6.DnsSecondary ? dns2str : NULL);
            }
#else
#ifdef USE_DHCLIENT
            snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "dhclient -6 -d --no-pid %s",  ifname);
            dhclient_alive++;
#else
            /*
                DHCPv6: Dibbler - a portable DHCPv6
                1. download from http://klub.com.pl/dhcpv6/
                2. cross-compile
                    2.1 ./configure --host=arm-linux-gnueabihf
                    2.2 copy dibbler-client to your board
                3. mkdir -p /var/log/dibbler/ /var/lib/ on your board
                4. create /etc/dibbler/client.conf on your board, the content is
                    log-mode short
                    log-level 7
                    iface wwan0 {
                        ia
                        option dns-server
                    }
                 5. run "dibbler-client start" to get ipV6 address
                 6. run "route -A inet6 add default dev wwan0" to add default route
            */
            snprintf(shell_cmd, sizeof(shell_cmd), "route -A inet6 add default %s", ifname);
            ql_system(shell_cmd);
            snprintf(udhcpc_cmd, sizeof(udhcpc_cmd), "dibbler-client run");
            dibbler_client_alive++;
#endif

            pthread_create(&udhcpc_thread_id, &udhcpc_thread_attr, udhcpc_thread_function, (void*)strdup(udhcpc_cmd));
#endif
        }
    }
}

void udhcpc_stop(PROFILE_T *profile) {
    char *ifname = profile->usbnet_adapter;
    char shell_cmd[128];

    ql_set_driver_link_state(profile, 0);

    if (profile->qmapnet_adapter) {
        ifname = profile->qmapnet_adapter;
    }

#ifdef USE_DHCLIENT
    if (dhclient_alive) {
        system("killall dhclient");
        dhclient_alive = 0;
    }
#endif
    if (dibbler_client_alive) {
        system("killall dibbler-client");
        dibbler_client_alive = 0;
    }

    snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s down", ifname);
    ql_system(shell_cmd);
    snprintf(shell_cmd, sizeof(shell_cmd), "ifconfig %s 0.0.0.0", ifname);
    ql_system(shell_cmd);
}
