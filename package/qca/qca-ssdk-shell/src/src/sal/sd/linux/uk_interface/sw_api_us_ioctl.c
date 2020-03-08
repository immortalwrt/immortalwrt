/*
 * Copyright (c) 2014, 2017-2018, The Linux Foundation. All rights reserved.
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

#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#include <fcntl.h>
#include "sw.h"
#include "sw_api.h"
#include "sw_api_us.h"

#define MISC_CHR_DEV       10
static int glb_socket_fd = 0;

sw_error_t
sw_uk_if(unsigned long arg_val[SW_MAX_API_PARAM])
{
    ioctl(glb_socket_fd, SIOCDEVPRIVATE, arg_val);
    return SW_OK;
}

#ifndef SHELL_DEV
#define SHELL_DEV "/dev/switch_ssdk"
#endif

sw_error_t
sw_uk_init(a_uint32_t nl_prot)
{
    if (!glb_socket_fd)
    {
        /* even mknod fail we not quit, perhaps the device node exist already */
#if defined UK_MINOR_DEV
        mknod(SHELL_DEV, S_IFCHR, makedev(MISC_CHR_DEV, UK_MINOR_DEV));
#else
        mknod(SHELL_DEV, S_IFCHR, makedev(MISC_CHR_DEV, nl_prot));
#endif
        if ((glb_socket_fd = open(SHELL_DEV, O_RDWR)) < 0)
        {
            return SW_INIT_ERROR;
        }
    }

    return SW_OK;
}

sw_error_t
sw_uk_cleanup(void)
{
    close(glb_socket_fd);
    glb_socket_fd = 0;
#if 0
    remove("/dev/switch_ssdk");
#endif
    return SW_OK;
}

