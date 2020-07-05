# Module nginx-toolkit-module
---
## Instructions

Nginx event toolkit modules. It contains modules below, and will add more in the future.

- [ngx-event-timer-module](doc/ngx-event-timer-module.md)

	Independent timer for nginx

- [ngx-event-resolver-module](doc/ngx-event-resolver-module.md)

	Common resovler in event modules, just like http resolver, stream resolver in nginx, but can be used by http modules, stream modules and other modules

- [ngx-dynamic-resolver-module](doc/ngx-dynamic-resolver-module.md)

	System will resolver domain in dynamic resolver every few seconds configured

	The module will return addr whose domain is resolved in dynamic resolver, otherwise, the module will add domain into dynamic resolver, resolv domain by event resolver, and call callback when resolved

- [ngx-dynamic-conf-module](doc/ngx-dynamic-conf-module.md)

	System will reload conf when nginx dynamic config file change. Developer can use this module to reload file without reload nginx worker

	Now it support NGX\_CORE\_MODULE and NGX\_HTTP\_MODULE

- [ngx-map](doc/ngx-map.md)

	A map implement use ngx\_rbtree

- [ngx-rbuf](doc/ngx-rbuf.md)

	A recycled chainbuf for nginx

- [ngx-toolkit-misc](doc/ngx-toolkit-misc.md)

	Misc toolkit functions
