# ngx-poold
---
## Instructions

Pool debug for check whether pool is destroy or destroy twice

## API

**NGX\_CREATE\_POOL**

	#define NGX_CREATE_POOL(size, log)

Replace NGX\_CREATE\_POOL instead of ngx\_create\_pool, it will record position creating pool. must use with NGX\_DESTROY\_POOL

**NGX\_DESTROY\_POOL**

	#define NGX_DESTROY_POOL(pool)

Replace NGX\_DESTROY\_POOL instead of ngx\_destroy\_pool, it will delete info which NGX\_CREATE\_POOL add. If pool not register in record pool, the pool will not destroy, and log delete twice log and position call the NGX\_DESTROY\_POOL.