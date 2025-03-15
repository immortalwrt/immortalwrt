/* $Id: upnpurns.h,v 1.1 2011/05/13 15:32:53 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2011 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef UPNPURNS_H_INCLUDED
#define UPNPURNS_H_INCLUDED

#include "config.h"

#ifdef IGD_V2
/* IGD v2 */
#define DEVICE_TYPE_IGD     "urn:schemas-upnp-org:device:InternetGatewayDevice:2"
#define DEVICE_TYPE_WAN     "urn:schemas-upnp-org:device:WANDevice:2"
#define DEVICE_TYPE_WANC    "urn:schemas-upnp-org:device:WANConnectionDevice:2"
#define SERVICE_TYPE_WANIPC "urn:schemas-upnp-org:service:WANIPConnection:2"
#define SERVICE_ID_WANIPC   "urn:upnp-org:serviceId:WANIPConn1"
#else
/* IGD v1 */
#define DEVICE_TYPE_IGD     "urn:schemas-upnp-org:device:InternetGatewayDevice:1"
#define DEVICE_TYPE_WAN     "urn:schemas-upnp-org:device:WANDevice:1"
#define DEVICE_TYPE_WANC    "urn:schemas-upnp-org:device:WANConnectionDevice:1"
#define SERVICE_TYPE_WANIPC "urn:schemas-upnp-org:service:WANIPConnection:1"
#define SERVICE_ID_WANIPC   "urn:upnp-org:serviceId:WANIPConn1"
#endif

#endif

