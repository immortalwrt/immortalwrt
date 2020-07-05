# ngx-rbuf
---
## Instructions

A recycled chainbuf for nginx

## API

**header file**

For using this API, You should include the header file as below:

	#include "ngx_rbuf.h"

**ngx\_get\_chainbuf**

	ngx_chain_t *ngx_get_chainbuf(size_t size, ngx_flag_t alloc_rbuf);

Interface for get a chain with buf, if alloc\_rbuf is set to 1, rbuf will alloc a buf with size set in paras; if alloc\_rbuf is set to 0, size is nonsense here.

Notice, for recycled buf better memory manager, the size should be same for the same usage. Such as chunk size in nginx rtmp module.


**ngx\_put\_chainbuf**

	void ngx_put_chainbuf(ngx_chain_t *cl);

Interface for recycle chain with buf alloc from rbuf

**ngx\_rbuf\_statee**

	ngx_chain_t *ngx_rbuf_state(ngx_http_request_t *r);

Interface for query rbuf state, result:

	ngx_rbuf_nalloc_node: 1
	ngx_rbuf_nalloc_buf: 1
	ngx_rbuf_nfree_buf: 1
	ngx_rbuf_nalloc_chain: 3
	ngx_rbuf_nalloc_chain: 3
