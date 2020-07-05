# Module ngx-event-timer-module
---
## Instructions

Independent timer for nginx

## Directives

	Syntax  : worker_timers number;
	Default : worker_timers 1024;
	Context : events

Sets the maximum number of timers that can be used in worker process.

	events {
		...
		worker_timers       1024;
	}

## API

**header file**

For using this API, You should include the header file as below:

	#include "ngx_event_timer_module.h"

**registe domain**

	ngx_int_t ngx_event_timer_add_timer(ngx_msec_t tv,
	        ngx_timer_handler_pt h, void *data);

- return value:

	return timerid for successd, NGX_ERROR for failed.

	Error:

	- h is NULL
	- not enough timer for assigned

- paras:

	- tv   : timer interval to trigger handler
	- h    : timer handler
	- data : data of h para

Register a timer handler, timer interval is tv, measured by millisecond. When timer triggered, h will be called, using data as function parameters.

h's protype is:

	typedef void (* ngx_timer_handler_pt)(void *data);

- return value:

	None

- paras:

	- data: data set in ngx\_event\_timer\_add\_timer, for paras transmit

**del timer**

	void ngx_event_timer_del_timer(ngx_uint_t timerid);

- return value:

	void

- paras:

	- timerid: return by ngx\_event\_timer\_add\_timer

Deregister timer handler.

## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-toolkit-module/
	make && make install

## Example

See t/ngx\_event\_timer\_test\_module.c as reference

Build:

	./configure --add-module=/path/to/nginx-toolkit-module/t/ --add-module=/path/to/nginx-toolkit-module/
	make && make install

Configure:

	location /event_timer_test/ {
		event_timer_test;
	}

Test:

add timer

	curl -XPOST -v "http://127.0.0.1/event_timer_test/timer?time=3s"

Test module will start a timer when process init. It will log after 3 seconds, set in args:

	2016/12/10 18:48:37 [error] 20295#0: event timer test timer handler

del timer

	curl -XDELETE -v "http://127.0.0.1/event_timer_test/timer?timerid=1"

Test module will stop a timer whose timerid is 1
