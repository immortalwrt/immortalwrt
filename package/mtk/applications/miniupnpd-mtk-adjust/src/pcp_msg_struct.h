/* $Id: pcp_msg_struct.h,v 1.3 2013/12/16 16:02:19 nanard Exp $ */
/* MiniUPnP project
 * Website : http://miniupnp.free.fr/
 * Author : Peter Tatrai

Copyright (c) 2013 by Cisco Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PCP_MSG_STRUCT_H_INCLUDED
#define PCP_MSG_STRUCT_H_INCLUDED

#define PCP_OPCODE_ANNOUNCE   0
#define PCP_OPCODE_MAP        1
#define PCP_OPCODE_PEER       2
#ifdef  PCP_SADSCP
#define PCP_OPCODE_SADSCP     3
#endif

/* Possible response codes sent by server, as a result of client request*/
#define PCP_SUCCESS                     0

#define PCP_ERR_UNSUPP_VERSION          1
/** The version number at the start of the PCP Request
 * header is not recognized by this PCP server.  This is a long
 * lifetime error.  This document describes PCP version 2.
 */

#define PCP_ERR_NOT_AUTHORIZED          2
/**The requested operation is disabled for this PCP
 * client, or the PCP client requested an operation that cannot be
 * fulfilled by the PCP server's security policy.  This is a long
 * lifetime error.
 */

#define PCP_ERR_MALFORMED_REQUEST       3
/**The request could not be successfully parsed.
 * This is a long lifetime error.
 */

#define PCP_ERR_UNSUPP_OPCODE           4
/** Unsupported Opcode.  This is a long lifetime error.
 */

#define PCP_ERR_UNSUPP_OPTION           5
/**Unsupported Option.  This error only occurs if the
 * Option is in the mandatory-to-process range.  This is a long
 * lifetime error.
 */

#define PCP_ERR_MALFORMED_OPTION        6
/**Malformed Option (e.g., appears too many times,
 * invalid length).  This is a long lifetime error.
 */

#define PCP_ERR_NETWORK_FAILURE         7
/**The PCP server or the device it controls are
 * experiencing a network failure of some sort (e.g., has not
 * obtained an External IP address).  This is a short lifetime error.
 */

#define PCP_ERR_NO_RESOURCES            8
/**Request is well-formed and valid, but the server has
 * insufficient resources to complete the requested operation at this
 * time.  For example, the NAT device cannot create more mappings at
 * this time, is short of CPU cycles or memory, or is unable to
 * handle the request due to some other temporary condition.  The
 * same request may succeed in the future.  This is a system-wide
 * error, different from USER_EX_QUOTA.  This can be used as a catch-
 * all error, should no other error message be suitable.  This is a
 * short lifetime error.
 */

#define PCP_ERR_UNSUPP_PROTOCOL         9
/**Unsupported transport protocol, e.g.  SCTP in a
 * NAT that handles only UDP and TCP.  This is a long lifetime error.
 */

#define PCP_ERR_USER_EX_QUOTA           10
/** This attempt to create a new mapping would exceed
 * this subscriber's port quota.  This is a short lifetime error.
 */

#define PCP_ERR_CANNOT_PROVIDE_EXTERNAL 11
/** The suggested external port and/or
 * external address cannot be provided.  This error MUST only be
 * returned for:
 *      *  MAP requests that included the PREFER_FAILURE Option
 *          (normal MAP requests will return an available external port)
 *      *  MAP requests for the SCTP protocol (PREFER_FAILURE is implied)
 *      *  PEER requests
 */

#define PCP_ERR_ADDRESS_MISMATCH        12
/** The source IP address of the request packet does
 * not match the contents of the PCP Client's IP Address field, due
 * to an unexpected NAT on the path between the PCP client and the
 * PCP-controlled NAT or firewall.  This is a long lifetime error.
 */

#define PCP_ERR_EXCESSIVE_REMOTE_PEERS  13
/** The PCP server was not able to create the
 * filters in this request.  This result code MUST only be returned
 * if the MAP request contained the FILTER Option.  See Section 13.3
 * for processing information.  This is a long lifetime error.
 */

typedef enum pcp_options  {
    PCP_OPTION_3RD_PARTY = 1,
    PCP_OPTION_PREF_FAIL = 2,
    PCP_OPTION_FILTER = 3,
#ifdef PCP_FLOWP
    PCP_OPTION_FLOW_PRIORITY = 4,  /*TODO: change it to correct value*/
#endif
} pcp_options_t;


/* PCP common request header*/
#if 0
typedef struct pcp_request {
  uint8_t     ver;
  uint8_t     r_opcode;
  uint16_t    reserved;
  uint32_t    req_lifetime;
  struct in6_addr    ip; /* ipv4 will be represented
                        by the ipv4 mapped ipv6 */
  uint8_t     next_data[0];
} pcp_request_t;
#endif
#define PCP_COMMON_REQUEST_SIZE (24)

/* PCP common response header*/
#if 0
typedef struct pcp_response {
  uint8_t     ver;
  uint8_t     r_opcode;    /* R indicates Request (0) or Response (1)
                              Opcode is 7 bit value specifying operation MAP or PEER */
  uint8_t     reserved;    /* reserved bits, must be 0 on transmission and must be ignored on reception */
  uint8_t     result_code; /*  */
  uint32_t    lifetime;    /* an unsigned 32-bit integer, in seconds {0, 2^32-1}*/
  uint32_t    epochtime;   /* epoch indicates how long has PCP server had its current mappings
                              it increases by 1 every second */
  uint32_t    reserved1[3];/* For requests that were successfully parsed this must be sent as 0 */
  uint8_t     next_data[0];
} pcp_response_t;
#endif
#define PCP_COMMON_RESPONSE_SIZE (24)


#if 0
typedef struct pcp_options_hdr {
  uint8_t     code;             /* Most significant bit indicates if this option is mandatory (0) or optional (1) */
  uint8_t     reserved;         /* MUST be set to 0 on transmission and MUST be ignored on reception */
  uint16_t    len;              /* indicates the length of the enclosed data in octets (see RFC) */
  uint8_t     next_data[0];     /* */
} pcp_options_hdr_t;
#endif
#define PCP_OPTION_HDR_SIZE (4)

/* same for both request and response */
#if 0
typedef struct pcp_map_v2 {
  uint32_t    nonce[3];
  uint8_t     protocol;   /* 6 = TCP, 17 = UDP, 0 = 'all protocols' */
  uint8_t     reserved[3];
  uint16_t    int_port;   /* 0 indicates 'all ports' */
  uint16_t    ext_port;   /* suggested external port */
  struct in6_addr ext_ip; /* suggested external IP address
      * ipv4 will be represented by the ipv4 mapped ipv6 */
  uint8_t     next_data[0];
} pcp_map_v2_t;
#endif
#define PCP_MAP_V2_SIZE (36)

#if 0
/* same for both request and response */
typedef struct pcp_map_v1 {
  uint8_t     protocol;
  uint8_t     reserved[3];
  uint16_t    int_port;
  uint16_t    ext_port;
  struct in6_addr ext_ip; /* ipv4 will be represented
                            by the ipv4 mapped ipv6 */
  uint8_t     next_data[0];
} pcp_map_v1_t;
#endif
#define PCP_MAP_V1_SIZE (24)

/* same for both request and response */
#if 0
typedef struct pcp_peer_v1 {
  uint8_t     protocol;
  uint8_t     reserved[3];
  uint16_t    int_port;
  uint16_t    ext_port;
  struct in6_addr ext_ip; /* ipv4 will be represented
                            by the ipv4 mapped ipv6 */
  uint16_t    peer_port;
  uint16_t    reserved1;
  struct in6_addr peer_ip;
  uint8_t     next_data[0];
} pcp_peer_v1_t;
#endif
#define PCP_PEER_V1_SIZE (44)

/* same for both request and response */
#if 0
typedef struct pcp_peer_v2 {
  uint32_t    nonce[3];
  uint8_t     protocol;
  uint8_t     reserved[3];
  uint16_t    int_port;
  uint16_t    ext_port;
  struct in6_addr ext_ip; /* ipv4 will be represented
                            by the ipv4 mapped ipv6 */
  uint16_t    peer_port;
  uint16_t    reserved1;
  struct in6_addr peer_ip;
  uint8_t     next_data[0];
} pcp_peer_v2_t;
#endif
#define PCP_PEER_V2_SIZE (56)

#ifdef PCP_SADSCP
#if 0
typedef struct pcp_sadscp_req {
    uint32_t  nonce[3];
    uint8_t   tolerance_fields;
    uint8_t   app_name_length;
    char      app_name[0];
}  pcp_sadscp_req_t;
#endif
#define PCP_SADSCP_REQ_SIZE (14)

#if 0
typedef struct pcp_sadscp_resp {
    uint32_t  nonce[3];
    uint8_t   a_r_dscp_value;
    uint8_t   reserved[3];
}  pcp_sadscp_resp_t;
#endif
#define PCP_SADSCP_MASK ((1<<6)-1)
#endif /* PCP_SADSCP */

#if 0
typedef struct pcp_prefer_fail_option {
   uint8_t   option;
   uint8_t   reserved;
   uint16_t  len;
   uint8_t   next_data[0];
} pcp_prefer_fail_option_t;
#endif
#define PCP_PREFER_FAIL_OPTION_SIZE (4)

#if 0
typedef struct pcp_3rd_party_option{
   uint8_t  option;
   uint8_t  reserved;
   uint16_t len;
   struct in6_addr ip;
   uint8_t  next_data[0];
} pcp_3rd_party_option_t;
#endif
#define PCP_3RD_PARTY_OPTION_SIZE (20)

#ifdef PCP_FLOWP
#if 0
typedef struct pcp_flow_priority_option{
   uint8_t  option;
   uint8_t  reserved;
   uint16_t len;
   uint8_t  dscp_up;
   uint8_t  dscp_down;
   uint8_t  reserved2;
   /* most significant bit is used for response */
   uint8_t  response_bit;
   uint8_t  next_data[0];
} pcp_flow_priority_option_t;
#endif
#define PCP_DSCP_MASK ((1<<6)-1)
#define PCP_FLOW_PRIORITY_OPTION_SIZE (8)
#endif

#if 0
typedef struct pcp_filter_option {
    uint8_t  option;
    uint8_t  reserved1;
    uint16_t len;
    uint8_t  reserved2;
    uint8_t  prefix_len;
    uint16_t peer_port;
    struct in6_addr peer_ip;
}pcp_filter_option_t;
#endif
#define PCP_FILTER_OPTION_SIZE (24)

#endif /* PCP_MSG_STRUCT_H_INCLUDED */
