# ngx-stream-zone-module
---
## Instructions

Record stream's owner worker process slot

## Directives

### stream\_zone

	Syntax  : stream_zone buckets=$nbuckets streams=$nstreams;
	Default : None;
	Context : main

nbuckets is hash buckect number, nstreams is max streams system can store

nbuckets is recommended use a prime number

## API

**header file**

For using this API, You should include the header file as below:

	#include "ngx_stream_zone_module.h"

**ngx\_stream\_zone\_insert\_stream**

	ngx_int_t ngx_stream_zone_insert_stream(ngx_str_t *name);

- para:

	name: stream name

- return value:

	process\_slot for owner of stream, NGX\_ERROR for error

**ngx\_stream\_zone\_delete\_stream**

	void ngx_stream_zone_delete_stream(ngx_str_t *name);

- para:

	name: stream name

**ngx\_stream\_zone\_state**

	ngx_chain_t *ngx_stream_zone_state(ngx_http_request_t *r, ngx_flag_t detail);

- para:

	- r: http request to query status of rbuf
	- detail: print stream detail in log

- return value:

	chain of stream zone state for returning to http client

## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-multiport-module/
	make && make install

## Example

See t/ngx\_stream\_zone\_test\_module.c as reference

**Build**:

	./configure --with-debug --with-ipv6 --add-module=/path/to/nginx-multiport-module/t/ --add-module=/path/to/nginx-multiport-module/
	make && make install

**Configure**:

	stream_zone  buckets=10007 streams=10000;

**Test**:

	curl -XPOST -v "http://127.0.0.1:9001/stream_zone_test/ab?stream=test"
	curl -XPOST -v "http://127.0.0.1:9002/stream_zone_test/ab?stream=test1"
	curl -XPOST -v "http://127.0.0.1:9003/stream_zone_test/ab?stream=test2"
	
	curl -XPOST -v "http://127.0.0.1:9003/stream_zone_test/ab?stream=test"
	
	curl -XDELETE -v "http://127.0.0.1:9000/stream_zone_test/ab?stream=test3"
	curl -XDELETE -v "http://127.0.0.1:9002/stream_zone_test/ab?stream=test1"
	curl -XDELETE -v "http://127.0.0.1:9001/stream_zone_test/ab?stream=test2"
	
	curl -XGET -v "http://127.0.0.1:9001/stream_zone_test/ab"