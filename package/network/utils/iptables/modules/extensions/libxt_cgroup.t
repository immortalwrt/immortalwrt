:INPUT,OUTPUT,POSTROUTING
*mangle
-m cgroup --cgroup 1;=;OK
-m cgroup ! --cgroup 1;=;OK
-m cgroup --path "/";=;OK
-m cgroup ! --path "/";=;OK
-m cgroup --cgroup 1 --path "/";;FAIL
-m cgroup ;;FAIL
