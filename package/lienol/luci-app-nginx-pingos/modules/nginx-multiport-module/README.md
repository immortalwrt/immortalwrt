# Module nginx-multiport-module
---
## Instructions

Every worker process can bind own port, user can visit specific worker process by using the port.

- [ngx-stream-zone-module](doc/ngx-stream-zone-module.md)

	Record stream's owner worker process slot

- [ngx-http-broadcast-module](doc/ngx-http-broadcast-module.md)

	Broadcast HTTP request to all worker processes when receive HTTP request

## Directives

### multi\_listen

	Syntax  : multi_listen multiport relationport;
	Default : None;
	Context : events

multiport can configured as below:

	address:port
	port
	unix:path

when configured with IPv4 or IPv6 port, worker process listen port plus with worker process's slot. For Example, we start four workers, add configured multiport with 9000. worker 0 will listen 9000, worker 1 will listen 9001, worker 2 will listen 9002, worker 3 will listen 9003

when configured with unix path, worker will listen path plus with suffix of worker process's slot. For Example, we start four workers, add configured multiport with unix:/tmp/http. worker 0 will listen /tmp/http.0, worker 1 will listen /tmp/http.1, worker 2 will listen /tmp/http.2, worker 3 will listen /tmp/http.3


relationport must configured same as listen directives in http server, rtmp server, stream server or other server

### inner\_proxy

	Syntax  : inner_proxy multiport uri;
	Default : None;
	Context : http, server, location

- multiport: configured in multi_listen
- uri: uri for inner_proxy, configured as below

        location /multiport_test/ {
            inner_proxy unix:/tmp/http.sock.80 /inner_proxy;
            multiport_test;
        }

        location /inner_proxy/ {
            rewrite ^/inner_proxy/(.*):/(.*) /$2 break;
            proxy_pass http://$1:;
        }

	As example above, if send subrequest to process whose workerid is 0, the uri will change to /inner_proxy/unix:/tmp/http.sock.80.0:/multiport_test/xxx
	
	proxy_pass will send current request to process 0 as inner proxy request.

## API

- ngx\_multiport\_get\_port

		ngx_int_t ngx_event_multiport_get_port(ngx_pool_t *pool, ngx_str_t *port, ngx_str_t *multiport, ngx_int_t pslot);

	- para:

		pool: pool for port memory alloc
		port: process real listen port while process\_slot is pslot
		multiport: port configure for processes, format as below:
		
			port only: port
			IPv4: host:port     host must be ipaddr of IPv4 or *
			IPv6: [host]:port   host must be ipaddr of IPv6
			Unix: unix:/path
		
		pslot: process\_slot, process\_slot of other worker process can get through ngx\_process\_slot\_get\_slot

	- return value:

		NGX\_OK for successd, NGX\_ERROR for failed

- ngx\_multiport\_get\_slot

		ngx_int_t ngx_multiport_get_slot(ngx_uint_t wpid);

	- para:

		wpid: worker process id, 0 to ccf->worker_processes - 1

	- return value:

		ngx_process_slot for successd, NGX_ERROR for failed

- ngx\_http\_inner\_proxy\_request

		ngx_int_t ngx_http_inner_proxy_request(ngx_http_request_t *r, ngx_int_t pslot);

	send a inner proxy request to specific process, must use with directives inner\_proxy
	
	- paras:

		- r: http request for send inner request to sibling worker
		- pslot: sibling worker ngx_process_slot
	
	- return values:

		- NGX_OK: for successd
		- NGX_ERROR: for failed
		- NGX_DECLINED: for not configured or send inner proxy to self

## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-multiport-module/
	make && make install

## Example

See t/ngx\_http\_process\_slot\_test\_module.c as reference

**Build**:

	./configure --with-debug --with-ipv6 --add-module=/path/to/nginx-multiport-module/t/ --add-module=/path/to/nginx-multiport-module/ --add-module=/path/to/echo-nginx-module/

	make && make install

**Configure**:

	worker_processes  4;

	events {
		...

		multi_listen 9000 80;
		multi_listen unix:/tmp/http.sock.80 80;
	}
	
	http {
		...

		server {
			...
	
			location /multiport_test/ {
				inner_proxy unix:/tmp/http.sock.80 /inner_proxy;
				multiport_test;
			}

			location /inner_proxy/ {
				rewrite ^/inner_proxy/(.*):/(.*) /$2 break;
				proxy_pass http://$1:;
			}
		}
	}

**Test for API**:

	$ curl http://192.168.84.254/multiport_test/123
	TEST cases 19, 19 pass

If request send to worker1 to worker3, the request will proxy to worker 0. will get log as below:

	2017/10/14 20:45:44 [error] 20065#0: *6 multiport test handler, client: 192.168.84.1, server: localhost, request: "GET /multiport_test/123 HTTP/1.1", host: "192.168.84.254:9003"
	2017/10/14 20:45:44 [error] 20065#0: *6 inner proxy return 0, client: 192.168.84.1, server: localhost, request: "GET /multiport_test/123 HTTP/1.1", host: "192.168.84.254:9003"
	2017/10/14 20:45:44 [error] 20062#0: *8 multiport test handler, client: unix:, server: localhost, request: "GET //multiport_test/123 HTTP/1.0", host: "localhost"

**Test for multiport**:

	curl -v http://127.0.0.1/
	curl -v http://127.0.0.1:9000/
	curl -v http://127.0.0.1:9001/
	curl -v http://127.0.0.1:9002/
	curl -v http://127.0.0.1:9003/

	curl -v --unix-socket /tmp/http.sock.80.0 http:/
	curl -v --unix-socket /tmp/http.sock.80.1 http:/
	curl -v --unix-socket /tmp/http.sock.80.2 http:/
	curl -v --unix-socket /tmp/http.sock.80.3 http:/

Tests will get the same result, for port 9000 will always send to worker process 0, 9001 to worker process 1 and so on
