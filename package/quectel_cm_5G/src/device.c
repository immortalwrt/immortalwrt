/******************************************************************************
  @file    device.c
  @brief   QMI device dirver.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <limits.h>
#include <linux/usbdevice_fs.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <time.h>
#include <pthread.h>

#include "QMIThread.h"
#include "ethtool-copy.h"

#define CM_MAX_PATHLEN 256

#define CM_INVALID_VAL (~((int)0))
/* get first line from file 'fname'
 * And convert the content into a hex number, then return this number */
static int file_get_value(const char *fname, int base)
{
    FILE *fp = NULL;
    int num;
    char buff[32 + 1] = {'\0'};
    char *endptr = NULL;

    fp = fopen(fname, "r");
    if (!fp) goto error;
    if (fgets(buff, sizeof(buff), fp) == NULL)
        goto error;
    fclose(fp);

    num = (int)strtol(buff, &endptr, base);
    if (errno == ERANGE && (num == LONG_MAX || num == LONG_MIN))
        goto error;
    /* if there is no digit in buff */
    if (endptr == buff)
        goto error;
    return num;

error:
    if (fp) fclose(fp);
    return CM_INVALID_VAL;
}

/*
 * This function will search the directory 'dirname' and return the first child.
 * '.' and '..' is ignored by default
 */
int dir_get_child(const char *dirname, char *buff, unsigned bufsize)
{
    struct dirent *entptr = NULL;
    DIR *dirptr = opendir(dirname);
    if (!dirptr)
        goto error;
    while ((entptr = readdir(dirptr))) {
        if (entptr->d_name[0] == '.')
            continue;
        snprintf(buff, bufsize, "%s", entptr->d_name);
        break;
    }

    closedir(dirptr);
    return 0;
error:
    buff[0] = '\0';
    if (dirptr) closedir(dirptr);
    return -1;
}

int conf_get_val(const char *fname, const char *key)
{
    char buff[CM_MAX_BUFF] = {'\0'};
    FILE *fp = fopen(fname, "r");
    if (!fp)
        goto error;
    
    while (fgets(buff, CM_MAX_BUFF, fp)) {
        char prefix[CM_MAX_BUFF] = {'\0'};
        char tail[CM_MAX_BUFF] = {'\0'};
        /* To eliminate cppcheck warnning: Assume string length is no more than 15 */
        sscanf(buff, "%15[^=]=%15s", prefix, tail);
        if (!strncasecmp(prefix, key, strlen(key))) {
            fclose(fp);
            return atoi(tail);
        }
    }

error:
    fclose(fp);
    return CM_INVALID_VAL;
}

static int detect_path_cdc_wdm_or_qcqmi(char *path, size_t len)
{
    size_t offset = strlen(path);

    if (!access(path, R_OK))
    {
        path[offset] = '\0';
        strcat(path, "/GobiQMI");
        if (!access(path, R_OK))
            return 0;

        path[offset] = '\0';
        strcat(path, "/usbmisc");
        if (!access(path, R_OK))
            return 0;

        path[offset] = '\0';
        strcat(path, "/usb");
        if (!access(path, R_OK))
            return 0;
    }

    return -1;
}

/* To detect the device info of the modem.
 * return:
 *  FALSE -> fail
 *  TRUE -> ok
 */
BOOL qmidevice_detect(char *qmichannel, char *usbnet_adapter, unsigned bufsize, int *pbusnum, int *pdevnum) {
    struct dirent* ent = NULL;
    DIR *pDir;
    const char *rootdir = "/sys/bus/usb/devices";
    struct {
        char path[255*2];
        char uevent[255*3];
    } *pl;
    pl = (typeof(pl)) malloc(sizeof(*pl));
    memset(pl, 0x00, sizeof(*pl));

    pDir = opendir(rootdir);
    if (!pDir) {
        dbg_time("opendir %s failed: %s", rootdir, strerror(errno));
        goto error;
    }

    while ((ent = readdir(pDir)) != NULL)  {
        int idVendor;
        int idProduct;
        char netcard[32+1] = {'\0'};
        char device[32+1] = {'\0'};
        char devname[32+1+6] = {'\0'};
        int busnum, devnum;

        snprintf(pl->path, sizeof(pl->path), "%s/%s/idVendor", rootdir, ent->d_name);
        idVendor = file_get_value(pl->path, 16);

        snprintf(pl->path, sizeof(pl->path), "%s/%s/idProduct", rootdir, ent->d_name);
        idProduct = file_get_value(pl->path, 16);

        if (idVendor != 0x05c6 && idVendor != 0x2c7c)
            continue;
        
        snprintf(pl->path, sizeof(pl->path), "%s/%s/busnum", rootdir, ent->d_name);
        busnum = file_get_value(pl->path, 10);
        snprintf(pl->path, sizeof(pl->path), "%s/%s/devnum", rootdir, ent->d_name);
        devnum = file_get_value(pl->path, 10);
        dbg_time("Find %s/%s idVendor=0x%x idProduct=0x%x, bus=0x%03x, dev=0x%03x", 
            rootdir, ent->d_name, idVendor, idProduct, busnum, devnum);

        /* get network interface */
        snprintf(pl->path, sizeof(pl->path), "%s/%s:1.4/net", rootdir, ent->d_name);
        dir_get_child(pl->path, netcard, sizeof(netcard));
        if (netcard[0] == '\0') {
            snprintf(pl->path, sizeof(pl->path), "%s/%s:1.8/net", rootdir, ent->d_name); //for EM12's MBIM
            dir_get_child(pl->path, netcard, sizeof(netcard));
        }
        if (netcard[0] == '\0')
            continue;

        if (usbnet_adapter[0] && strcmp(usbnet_adapter, netcard))
            continue;

        pl->path[strlen(pl->path)-strlen("/net")] = '\0';
        if (detect_path_cdc_wdm_or_qcqmi(pl->path, sizeof(pl->path)))
            continue;

        /* get device */
        dir_get_child(pl->path, device, sizeof(device));
        if (device[0] == '\0')
            continue;

        /* There is a chance that, no device(qcqmiX|cdc-wdmX) is generated. We should warn user about that! */
        snprintf(devname, sizeof(devname), "/dev/%s", device);
        if (access(devname, R_OK | F_OK) && errno == ENOENT) {
            int major;
            int minor;
            int ret;

            dbg_time("%s access failed, errno: %d (%s)", devname, errno, strerror(errno));
            snprintf(pl->uevent, sizeof(pl->uevent), "%s/%s/uevent", pl->path, device);
            major = conf_get_val(pl->uevent, "MAJOR");
            minor = conf_get_val(pl->uevent, "MINOR");
            if(major == CM_INVALID_VAL || minor == CM_INVALID_VAL)
                dbg_time("get major and minor failed");

            ret = mknod(devname, S_IFCHR|0666, (((major & 0xfff) << 8) | (minor & 0xff) | ((minor & 0xfff00) << 12)));
            if (ret)
                dbg_time("please mknod %s c %d %d", devname, major, minor);
        }

        if (netcard[0] && device[0]) {
            snprintf(qmichannel, bufsize, "/dev/%s", device);
            snprintf(usbnet_adapter, bufsize, "%s", netcard);
            dbg_time("Auto find qmichannel = %s", qmichannel);
            dbg_time("Auto find usbnet_adapter = %s", usbnet_adapter);
			*pbusnum = busnum;
			*pdevnum = devnum;
            break;
        }
    }
    closedir(pDir);
    
    if (qmichannel[0] == '\0' || usbnet_adapter[0] == '\0') {
        dbg_time("network interface '%s' or qmidev '%s' is not exist", usbnet_adapter, qmichannel);
        goto error;
    }
    free(pl);
    return TRUE;
error:
    free(pl);
    return FALSE;
}

int mhidevice_detect(char *qmichannel, char *usbnet_adapter, PROFILE_T *profile) {
    if (!access("/sys/class/net/pcie_mhi0", F_OK))
        strcpy(usbnet_adapter, "pcie_mhi0");
    else if (!access("/sys/class/net/rmnet_mhi0", F_OK))
        strcpy(usbnet_adapter, "rmnet_mhi0");
    else {
        dbg_time("qmidevice_detect failed");
        goto error;
    }

    if (!access("/dev/mhi_MBIM", F_OK)) {
        strcpy(qmichannel, "/dev/mhi_MBIM");
        profile->software_interface = SOFTWARE_MBIM;
    }
    else if (!access("/dev/mhi_QMI0", F_OK)) {
        strcpy(qmichannel, "/dev/mhi_QMI0");
        profile->software_interface = SOFTWARE_QMI;
    }
    else {
        goto error;
    }

    return 1;
error:
    return 0;
}

#define USB_CLASS_COMM			2
#define USB_CLASS_VENDOR_SPEC		0xff
#define USB_CDC_SUBCLASS_MBIM			0x0e

int get_driver_type(PROFILE_T *profile)
{
    char path[CM_MAX_PATHLEN+1] = {'\0'};
    int bInterfaceClass;
    int type = DRV_INVALID;

    snprintf(path, sizeof(path), "/sys/class/net/%s/device/bInterfaceClass", profile->usbnet_adapter);
    bInterfaceClass = file_get_value(path, 16);

    /* QMI_WWAN */
    if (bInterfaceClass == USB_CLASS_VENDOR_SPEC)
        type = SOFTWARE_QMI;

    /* CDC_MBIM */
    if (bInterfaceClass == USB_CLASS_COMM)
        type = SOFTWARE_MBIM;

    return type;
}

struct usbfs_getdriver
{
    unsigned int interface;
    char driver[255 + 1];
};

struct usbfs_ioctl
{
    int ifno;       /* interface 0..N ; negative numbers reserved */
    int ioctl_code; /* MUST encode size + direction of data so the
			 * macros in <asm/ioctl.h> give correct values */
    void *data;     /* param buffer (in, or out) */
};

#define IOCTL_USBFS_DISCONNECT	_IO('U', 22)
#define IOCTL_USBFS_CONNECT	_IO('U', 23)

int usbfs_is_kernel_driver_alive(int fd, int ifnum)
{
    struct usbfs_getdriver getdrv;
    getdrv.interface = ifnum;
    if (ioctl(fd, USBDEVFS_GETDRIVER, &getdrv) < 0) {
        dbg_time("%s ioctl USBDEVFS_GETDRIVER failed, kernel driver may be inactive", __func__);
        return 0;
    }
    dbg_time("%s find interface %d has match the driver %s", __func__, ifnum, getdrv.driver);
    return 1;
}

void usbfs_detach_kernel_driver(int fd, int ifnum)
{
    struct usbfs_ioctl operate;
    operate.data = NULL;
    operate.ifno = ifnum;
    operate.ioctl_code = IOCTL_USBFS_DISCONNECT;
    if (ioctl(fd, USBDEVFS_IOCTL, &operate) < 0) {
        dbg_time("%s detach kernel driver failed", __func__);
    } else {
        dbg_time("%s detach kernel driver success", __func__);
    }
}

void usbfs_attach_kernel_driver(int fd, int ifnum)
{
    struct usbfs_ioctl operate;
    operate.data = NULL;
    operate.ifno = ifnum;
    operate.ioctl_code = IOCTL_USBFS_CONNECT;
    if (ioctl(fd, USBDEVFS_IOCTL, &operate) < 0) {
        dbg_time("%s detach kernel driver failed", __func__);
    } else {
        dbg_time("%s detach kernel driver success", __func__);
    }
}

int reattach_driver(PROFILE_T *profile)
{
    int ifnum = 4;
    int fd;
    char devpath[128] = {'\0'};
    snprintf(devpath, sizeof(devpath), "/dev/bus/usb/%03d/%03d", profile->busnum, profile->devnum);
    fd = open(devpath, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        dbg_time("%s fail to open %s", __func__, devpath);
        return -1;
    }
    usbfs_detach_kernel_driver(fd, ifnum);
    usbfs_attach_kernel_driver(fd, ifnum);
	close(fd);
    return 0;
}

#define SIOCETHTOOL     0x8946
int ql_get_netcard_driver_info(const char *devname)
{
    int fd = -1;
    struct ethtool_drvinfo drvinfo;
    struct ifreq ifr;	/* ifreq suitable for ethtool ioctl */

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, devname);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        dbg_time("Cannot get control socket: errno(%d)(%s)", errno, strerror(errno));
        return -1;
    }

    drvinfo.cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (void *)&drvinfo;

    if (ioctl(fd, SIOCETHTOOL, &ifr) < 0) {
        dbg_time("ioctl() error: errno(%d)(%s)", errno, strerror(errno));
        return -1;
    }

    dbg_time("netcard driver = %s, driver version = %s", drvinfo.driver, drvinfo.version);
	
	close(fd);

    return 0;
}

void *catch_log(void *arg)
{
    PROFILE_T *profile = (PROFILE_T *)arg;
    int nreads = 0;
    char buff[256], tbuff[256+32];
    time_t t;
    struct tm *tm;
    char filter[10];

    sprintf(filter, ":%d:%03d:", profile->busnum, profile->devnum);

    while(1) {
        nreads = read(profile->usbmon_fd, buff, sizeof(buff));
		if (nreads <= 0) {
			if (errno == EINTR)
				continue;
			else
				break;
		}

        buff[nreads] = '\0';   // printf("%s", buff);

        if (!strstr(buff, filter))
            continue;

        time(&t);
        tm = localtime(&t);

        sprintf(tbuff, "%04d/%02d/%02d_%02d:%02d:%02d %s", 
                tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, buff);

        write(profile->usbmon_logfile_fd, tbuff, strlen(tbuff));
    }

    return NULL;
}

int ql_capture_usbmon_log(PROFILE_T *profile, const char *log_path)
{
    char usbmon_path[64];
    pthread_t pt;
    pthread_attr_t attr;

    if (access("/sys/kernel/debug/usb", F_OK)) {
        dbg_time("debugfs is not mount, please execute \"mount -t debugfs none_debugs /sys/kernel/debug\"");
        return -1;
    }
    if (access("/sys/kernel/debug/usb/usbmon", F_OK)) {
        dbg_time("usbmon is not load, please execute \"modprobe usbmon\" or \"insmod usbmon.ko\"");
        return -1;
    }

    sprintf(usbmon_path, "/sys/kernel/debug/usb/usbmon/%du", profile->busnum);
    profile->usbmon_fd = open(usbmon_path, O_RDONLY);
    if (profile->usbmon_fd < 0) {
        dbg_time("open %s error(%d) (%s)", usbmon_path, errno, strerror(errno));
        return -1;
    }

    profile->usbmon_logfile_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (profile->usbmon_logfile_fd < 0) {
        dbg_time("open %s error(%d) (%s)", log_path, errno, strerror(errno));
        close(profile->usbmon_fd);
		profile->usbmon_fd = -1;
        return -1;
    }

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&pt, &attr, catch_log, (void *)profile);

    return 0;
} 

void ql_stop_usbmon_log(PROFILE_T *profile) {
	if (profile->usbmon_fd > 0)
		close(profile->usbmon_fd);
	if (profile->usbmon_logfile_fd > 0)
		close(profile->usbmon_logfile_fd);
}
