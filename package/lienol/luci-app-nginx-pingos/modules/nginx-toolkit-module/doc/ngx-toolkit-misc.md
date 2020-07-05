# ngx-toolkit-misc
---
## Instructions

Misc toolkit functions

## API

**header file**

For using this API, You should include the header file as below:

	#include "ngx_toolkit_misc.h"

**structure**

	/*
	 * scheme://[user@]host[:port]/path[?args][#fragment]
	 */
	typedef struct {
	    ngx_str_t                   scheme;
	    ngx_str_t                   user;
	    ngx_str_t                   host;
	    ngx_str_t                   port;
	    ngx_str_t                   path;
	    ngx_str_t                   args;
	    ngx_str_t                   fragment;
	
	    ngx_str_t                   host_with_port; /* host[:port] */
	    ngx_str_t                   uri_with_args;  /* /path[?args][#fragment] */
	} ngx_request_url_t;

**ngx\_parse\_request\_url**

	ngx_int_t ngx_parse_request_url(ngx_request_url_t *request_url, ngx_str_t *url);

parse request url format as: scheme://[user@]host[:port]/path[?args][#fragment]

- return value:

	- NGX_OK: parse success
	- NGX_ERROR: request url format error

- paras:

	- request_url: url parse result return to user, all paras in request url is segment point to url
	- url: request url for parse


**ngx\_request\_port**

	in_port_t ngx_request_port(ngx_str_t *scheme, ngx_str_t *port);

convert port to in_port_t according to scheme and port

- return value:

	- If port is set to correct number range in [1, 65535], return port
	- If port is set to non correct value, return 0
	- If port is not set, return default value for scheme:

		- 80 for http
		- 443 for https
		- 1935 for rtmp
		- 0 for others now

- values:

	- scheme : sheme string like http https or rtmp
	- port   : port for convert to in_port_t


**ngx\_md5\_file**

	#define NGX_MD5KEY_LEN  32

	ngx_int_t ngx_md5_file(ngx_fd_t fd, u_char md5key[NGX_MD5KEY_LEN]);

calculating file md5key as md5sum in shell

- return value:

	- NGX_OK: calculating success
	- NGX_ERROR: calculating error such as file is not exist

- paras:

	- fd: file desc for calculating md5key
	- md5key: md5key result