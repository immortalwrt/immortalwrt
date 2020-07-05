# Module ngx-event-resolver-module
---
## Instructions

Common resovler in event modules, just like http resolver, stream resolver in nginx, but can be used by http modules, stream modules and other modules

## Directives

	Syntax  : resolver address ... [valid=time] [ipv6=on|off];
	Default : -
	Context : events

Configures name servers used to resolve names into addresses, for example:

> resolver 127.0.0.1 [::1]:5353;

An address can be specified as a domain name or IP address, and an optional port. If port is not specified, the port 53 is used. Name servers are queried in a round-robin fashion.

By default, nginx will look up both IPv4 and IPv6 addresses while resolving. If looking up of IPv6 addresses is not desired, the ipv6=off parameter can be specified.

By default, nginx caches answers using the TTL value of a response. The optional valid parameter allows overriding it:

> resolver 127.0.0.1 [::1]:5353 valid=30s;

	Syntax  : resolver_timeout time;
	Default : resolver_timeout 60s;
	Context : events

Sets a timeout for name resolution, for example:

> resolver\_timeout 5s;

Example:

	events {
		resolver                  192.168.84.254 valid=20s;
		resolver_timeout          10s;
	}

## API

**header file**

For using this API, You should include the header file as below:

	#include "ngx_event_resolver.h"

**resolver a domain**

	void ngx_event_resolver_start_resolver(ngx_str_t *domain,
        ngx_event_resolver_handler_pt h, void *data);

- return value:

	None

- paras:

	- domain: domain for resolving
	- h     : callback handler
	- data  : data for callback

h's protype is:

	typedef void (* ngx_event_resolver_handler_pt)(void *data,
        ngx_resolver_addr_t *addrs, ngx_uint_t naddrs);

- return value:

	None

- paras:

	- data  : user private data set in ngx\_event\_resolver\_start\_resolver
	- addrs : addrs resolv by DNS
	- naddrs: number of addrs resolv by DNS


## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-toolkit-module/
	make && make install

## Example

See t/ngx\_event\_resolver\_test\_module.c as reference

Build:

	./configure --with-debug --add-module=/path/to/nginx-toolkit-module/ --add-module=/path/to/nginx-toolkit-module/t
	make && make install

Configure:

	events {
		resolver                  192.168.84.254 114.114.114.114 valid=20s;
		resolver_timeout          10s;
	}

	http {

		...

		server {

			...

			location /event_resolver_test/ {
				event_resolver_test;
			}
		}
	}

Install bind server

	/path/to/nginx-toolkit-module/t/dns_install.sh

modify /var/named/test.com.zone dns ip address to fit your enviroment

Test:

- add domain for resolving

		curl -v "http://127.0.0.1/event_resolver_test/domain?domain=www.test.com"
