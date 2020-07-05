# Module ngx-dynamic-resolver-module
---
## Instructions

System will resolver domain in dynamic resolver every few seconds configured.

The module will return addr whose domain is resolved in dynamic resolver, otherwise, the module will add domain into dynamic resolver, resolv domain by event resolver, and call callback when resolved.

## Directives

	Syntax  : dynamic_refresh_interval time;
	Default : dynamic_refresh_interval 5m;
	Context : events

Set time interval for DNS query frequency, 0 for shutdown this function.

	Syntax  : dynamic_domain_buckets number;
	Default : dynamic_domain_buckets 101;
	Context : events

Bucket for dynamic resolver domain hash table. Use prime key is Recommended.

Example:

	events {
		resolver                  192.168.84.254 valid=1m;
		dynamic_refresh_interval  5m;
		dynamic_domain_buckets    1001;
	}

## API

**header file**

For using this API, You should include the header file as below:

	#include "ngx_dynamic_resolver.h"

**start resolver**

	void ngx_dynamic_resolver_start_resolver(ngx_str_t *domain,
        ngx_dynamic_resolver_handler_pt h, void *data);

- return value:

	- None

- paras:

	- domain: domain for DNS query
	- h     : callback handler
	- data  : data for callback

h's protype is:

	typedef void (* ngx_dynamic_resolver_handler_pt)(void *data,
        struct sockaddr *sa, socklen_t socklen);

- return value:

	- None

- paras:

	- data   : user private data set in ngx\_dynamic\_resolver\_start\_resolver
	- sa     : sock address get
	- socklen: sock address len, 0 for get none address

**gethostbyname**

	socklen_t ngx_dynamic_resolver_gethostbyname(ngx_str_t *domain, struct sockaddr *sa);

- return value:

	- socklen for successd
	- 0 for failed

- paras:

	- domain: domain for query
	- sa     : sock address get

**add domain**

	void ngx_dynamic_resolver_add_domain(ngx_str_t *domain);

- return value:

	- None

- paras:

	- domain: domain for query


**del domain**

	void ngx_dynamic_resolver_del_domain(ngx_str_t *domain);

- return value:

	- None

- paras:

	- domain: domain for DNS query

## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-toolkit-module/
	make && make install

## Example

See t/ngx\_dynamic\_resolver\_test\_module.c as reference

Build:

	./configure --with-debug --add-module=/path/to/nginx-toolkit-module/ --add-module=/path/to/nginx-toolkit-module/t
	make && make install

Configure:

	events {
		resolver                  192.168.84.254 valid=1m;
		dynamic_refresh_interval  5m;
	}

	http {

		...

		server {

			...

			location /dynamic_resolver_test/ {
				dynamic_resolver_test  192.168.84.4  www.test1.com;
			}
		}
	}

Test:

- sync resolver

		curl -v 'http://127.0.0.1/dynamic_resolver_test/resolver?domain=www.test1.com&sync=1'

- start resolver

		curl -v 'http://127.0.0.1/dynamic_resolver_test/resolver?domain=www.test.com'

- del domain

		curl -XDELETE -v 'http://127.0.0.1/dynamic_resolver_test/resolver?domain=www.test.com'
