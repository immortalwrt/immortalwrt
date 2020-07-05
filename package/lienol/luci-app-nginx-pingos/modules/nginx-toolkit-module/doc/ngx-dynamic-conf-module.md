# Module ngx-dynamic-conf-module
---
## Instructions

System will reload conf when nginx dynamic config file change. Developer can use this module to reload file without reload nginx worker.

Now it support NGX\_CORE\_MODULE and NGX\_HTTP\_MODULE

## Directives

	Syntax  : dynamic_conf dynamic_file time;
	Default : -
	Context : main

Set dynamic config file and interval system checked file changed.

	Syntax  : dynamic_log log_file level;
	Default : -
	Context : main

Set dynamic conf load log file and log level. If not configured, use cycle log as default.

Example:

	dynamic_conf    conf/nginx_dynamic.conf 10;
	dynamic_log     logs/error_dynamic.log  info;

## API

### MAIN dynamic conf

**header file**

For using this API, You should include the header file as below:

	#include "ngx_dynamic_conf.h"

**dynamic module define**

	typedef struct {
	    ngx_str_t               name;
	    void                 *(*create_conf)(ngx_conf_t *cf);
	    char                 *(*init_conf)(ngx_conf_t *cf, void *conf);
	} ngx_dynamic_core_module_t;

dynamic conf module define as below

	ngx_module_t  ngx_test_module = {
	    NGX_MODULE_V1,
	    &ngx_test_module_ctx,                   /* module context */
	    ngx_test_commands,                      /* module directives */
	    NGX_CORE_MODULE,                        /* module type */
	    NULL,                                   /* init master */
	    NULL,                                   /* init module */
	    NULL,                                   /* init process */
	    NULL,                                   /* init thread */
	    NULL,                                   /* exit thread */
	    NULL,                                   /* exit process */
	    NULL,                                   /* exit master */
	    (uintptr_t) &ngx_test_module_dctx,      /* module dynamic context */
	    (uintptr_t) ngx_test_dcommands,         /* module dynamic directives */
	    NGX_MODULE_V1_DYNAMIC_PADDING
	};

**module dynamic context** struct define as above, **module dynamic directives** define use ngx\_command\_t. Use ngx\_dynamic\_core\_test\_module define in t/ngx\_dynamic\_conf\_test\_module.c as reference

**ngx\_dynamic\_conf\_parse**

	ngx_int_t ngx_dynamic_conf_parse(ngx_conf_t *cf, unsigned init)

- return value:

	- return NGX\_OK for successd, NGX\_ERROR for failed

- paras:

	- cf   : ngx\_conf\_t passed from ngx\_dynamic\_conf\_load_conf
	- init : only ngx\_dynamic\_conf\_load\_conf set 1, otherwise set 0

This interface is supported for other dynamic conf module, such as ngx\_conf\_parse

**ngx\_dynamic\_regex\_compile**

	typedef struct {
		ngx_regex_t            *regex;
		ngx_str_t               name;
	} ngx_dynamic_regex_t;

	ngx_dynamic_regex_t *ngx_dynamic_regex_compile(ngx_conf_t *cf,
			ngx_regex_compile_t *rc);

- return value:

	- return regex context

- paras:

	- cf: ngx\_conf\_t passed in dynamic cmd handler
	- rc: regex options

compile regex

**ngx\_get\_dconf**

	void *ngx_get_dconf(ngx_module_t *m)
	
return NGX\_CORE\_MODULE dynamic config for module

### HTTP dynamic conf

**header file**

For using this API, You should include the header file as below:

	#include "ngx_dynamic_conf.h"

**dynamic module define**

dynamic conf module define is same as MAIN dynamic conf

http dynamic conf context define as below:

	typedef struct {
	    void       *(*create_main_conf)(ngx_conf_t *cf);
	    char       *(*init_main_conf)(ngx_conf_t *cf, void *conf);
	
	    void       *(*create_srv_conf)(ngx_conf_t *cf);
	    char       *(*init_srv_conf)(ngx_conf_t *cf, void *conf);
	
	    void       *(*create_loc_conf)(ngx_conf_t *cf);
	    char       *(*init_loc_conf)(ngx_conf_t *cf, void *conf);
	} ngx_http_dynamic_module_t;

**notice:** http dynamic conf do not support merge

**ngx\_http\_get\_module\_main\_dconf**

	void *ngx_http_get_module_main_dconf(ngx_http_request_t *r, ngx_module_t *m);

return http request main dynamic conf for module m

**ngx\_http\_get\_module\_srv\_dconf**

	void *ngx_http_get_module_srv_dconf(ngx_http_request_t *r, ngx_module_t *m);

return http request srv dynamic conf for module m

**ngx\_http\_get\_module\_loc\_dconf**

	void *ngx_http_get_module_loc_dconf(ngx_http_request_t *r, ngx_module_t *m);

return http request loc dynamic conf for module m

## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-toolkit-module/
	make && make install

## Example

See

- t/ngx\_dynamic\_conf\_test\_module.c as MAIN conf for usage of dynamic conf
- t/ngx\_http\_dynamic\_test\_module.c as HTTP conf for usage of http dynamic conf

**Build:**

	./configure --with-debug --add-module=/path/to/nginx-toolkit-module/ --add-module=/path/to/nginx-toolkit-module/t
	make && make install

**Configure:**

	dynamic_conf    conf/nginx_dynamic.conf 10;
	dynamic_log     logs/error_dynamic.log  info;

	http {

		...

		server {

			...

			location /dynamic_conf_test/ {
				dynamic_conf_test;
			}
		}
	}

**Dynamic Configure:**

	dynamic_test_i  200;
	dynamic_test_s  hello_world;
	
	http {
	    main_int    1000;
	    main_str    gogogo;
	
	    #defult server
	    server {
	        srv_int         1;
	        srv_str         default;
	    }
	
	    #wildcard_head
	    server {
	        srv_int         2;
	        srv_str         wildcard_head;
	        serverid        baidu;
	        server_name     *.baidu.com;
	    }
	
	    #wildcard_tail
	    server {
	        srv_int         3;
	        srv_str         wildcard_tail;
	        serverid        google;
	        server_name     www.google.*;
	    }
	
	    #hash
	    server {
	        srv_int         4;
	        srv_str         hash;
	        serverid        sina;
	        server_name     sports.sina.com.cn;
	
	        location = / {
	            loc_int     1;
	            loc_str     =/;
	        }
	
	        location / {
	            loc_int     2;
	            loc_str     /;
	        }
	
	        location ^~ /test1/ {
	            loc_int     3;
	            loc_str     ^~/test1/;
	        }
	
	        location ~* \.(gif|jpg|jpeg)$ {
	            loc_int     4;
	            loc_str     ~*\.(gif|jpg|jpeg)$;
	        }
	
	        location /test {
	            loc_int     5;
	            loc_str     /test;
	        }
	    }
	
	    #pcre
	    server {
	        srv_int         5;
	        srv_str         pcre;
	        serverid        test;
	        server_name     ~^flv(?!.*(dl\.))[A-Za-z0-9]*\.test\.com$;
	    }
	
	    #multi
	    server {
	        srv_int         6;
	        srv_str         multi;
	        serverid        others;
	        server_name     ~^flv(?!.*(dl\.))[A-Za-z0-9]*\.haha\.com$ www.sohu.com;
	        server_name     *.qq.com;
	    }
	}

**Test:**

- Main for dynamic config

	get conf configured in dynamic config file for test module

		curl -v 'http://127.0.0.1/dynamic_conf_test/test'

	change config in dynamic config, the test api will return new config value after dynamic conf refresh

- Main for http

		curl -v 'http://127.0.0.1/'

	change config in http block of dynamic config, the test api will return new config value after dynamic conf refresh

- Server for http

	- defult server

			curl -v 'http://127.0.0.1/http_dynamic_test/test'
			curl -v -H 'Host: github.com' 'http://127.0.0.1/http_dynamic_test/test'

	- wildcard_head

			curl -v -H 'Host: map.baidu.com' 'http://127.0.0.1/http_dynamic_test/test'

	- wildcard_tail

			curl -v -H 'Host: www.google.co.jp' 'http://127.0.0.1/http_dynamic_test/test'

	- hash

			curl -v -H 'Host: sports.sina.com.cn' 'http://127.0.0.1/http_dynamic_test/test'

	- pcre

			curl -v -H 'Host: flvdl7a8e4223.test.com' 'http://127.0.0.1/http_dynamic_test/test'

	- multi

			curl -v -H 'Host: flvdl7a8e4223.haha.com' 'http://127.0.0.1/http_dynamic_test/test'
			curl -v -H 'Host: www.sohu.com' 'http://127.0.0.1/http_dynamic_test/test'
			curl -v -H 'Host: v.qq.com' 'http://127.0.0.1/http_dynamic_test/test'

- Location for http

	- no location

			curl -v -H 'Host: flvdl7a8e4223.haha.com' 'http://127.0.0.1/'

	- location = /

			curl -v -H 'Host: sports.sina.com.cn' 'http://127.0.0.1/'

	- location /

			curl -v -H 'Host: sports.sina.com.cn' 'http://127.0.0.1/t'

	- location ^~ /test1/

			curl -v -H 'Host: sports.sina.com.cn' 'http://127.0.0.1/test1/123'

	- ~* \.(gif|jpg|jpeg)$

			curl -v -H 'Host: sports.sina.com.cn' 'http://127.0.0.1/test/123.gif'

	- /test

			curl -v -H 'Host: sports.sina.com.cn' 'http://127.0.0.1/test/123'
