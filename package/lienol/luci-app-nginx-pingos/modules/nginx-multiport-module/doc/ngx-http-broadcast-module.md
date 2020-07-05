# ngx-http-broadcast-module
---
## Instructions

Broadcast HTTP request to all worker processes when receive HTTP request

## Directives

### broadcast

	Syntax  : broadcast multiport uri;
	Default : None;
	Context : location

- multiport is multi_listen port configured in event
- uri is http proxy_pass uri configured as below


		location /auth_proxy/ {
			rewrite ^/auth_proxy/(.*) /auth break;
			proxy_pass http://$1:;
		}


## Build

cd to NGINX source directory & run this:

	./configure --add-module=/path/to/nginx-multiport-module/
	make && make install

## Example

**Build**:

	./configure --with-debug --with-ipv6 --add-module=/path/to/nginx-multiport-module/t/ --add-module=/path/to/nginx-multiport-module/ --add-module=/path/to/echo-nginx-module/
	make && make install

**Configure**:

	events {
		...
		multi_listen unix:/tmp/http.sock.80 80;
	}
	
	
	http {
		...	

		server {
			listen       80;
			server_name  localhost;

			...

			location / {
				broadcast unix:/tmp/http.sock.80 /auth_proxy;
			}

			location /auth_proxy/ {
				rewrite ^/auth_proxy/(.*) /auth break;
				proxy_pass http://$1:;
			}

			location /auth {
			#	return 403;
				echo "auth";
				echo $scheme://$host$uri?$args;
			}
		}
	}

**Test**:

	curl -v 'http://192.168.84.254/aa?a=b&c=d'

curl will get all response content if worker not return non 200 response