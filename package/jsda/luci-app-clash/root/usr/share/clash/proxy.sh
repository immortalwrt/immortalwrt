#!/bin/sh /etc/rc.common
. /lib/functions.sh
if pidof clash >/dev/null; then
/etc/init.d/clash stop 2>/dev/null
else
uci set clash.config.enable=1 2> /dev/null
uci commit clash 2> /dev/null
fi
enable_create=$(uci get clash.config.enable_servers 2>/dev/null)
if [ "$enable_create" == "1" ];then
status=$(ps|grep -c /usr/share/clash/proxy.sh)
[ "$status" -gt "3" ] && exit 0

CONFIG_YAML_RULE="/usr/share/clash/custom_rule.yaml"
SERVER_FILE="/tmp/servers.yaml"
CONFIG_YAML="/etc/clash/config.yaml"
CONFIG_YAML_BAK="/etc/clash/config.bak"
TEMP_FILE="/tmp/dns_temp.yaml"
SERVERS="/tmp/servers_temp.yaml"
Proxy_Group="/tmp/Proxy_Group"
Proxy_Group_url="/tmp/Proxy_url"
RULE_PROXY="/tmp/tempserv.yaml"

servers_set()
{
   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "server" "$section" "server" ""
   config_get "port" "$section" "port" ""
   config_get "cipher" "$section" "cipher" ""
   config_get "password" "$section" "password" ""
   config_get "securitys" "$section" "securitys" ""
   config_get "udp" "$section" "udp" ""
   config_get "obfs" "$section" "obfs" ""
   config_get "obfs_vmess" "$section" "obfs_vmess" ""
   config_get "host" "$section" "host" ""
   config_get "custom" "$section" "custom" ""
   config_get "custom_host" "$section" "custom_host" ""
   config_get "tls" "$section" "tls" ""
   config_get "tls_custom" "$section" "tls_custom" ""
   config_get "skip_cert_verify" "$section" "skip_cert_verify" ""
   config_get "path" "$section" "path" ""
   config_get "alterId" "$section" "alterId" ""
   config_get "uuid" "$section" "uuid" ""
   config_get "auth_name" "$section" "auth_name" ""
   config_get "auth_pass" "$section" "auth_pass" ""
   
	
   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$server" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      name="Server"
   fi
   
   if [ -z "$port" ]; then
      return
   fi
   
   if [ ! -z "$udp" ] && [ "$obfs" ] || [ "$obfs" = " " ]; then
      udpp=", udp: $udp"
   fi
   
   if [ "$obfs" != "none" ]; then
      if [ "$obfs" = "websocket" ]; then
         obfss="plugin: v2ray-plugin"
      else
         obfss="plugin: obfs"
      fi
   else
      obfs=""
   fi
   
   if [ "$obfs_vmess" = "websocket" ]; then
      	obfs_vmesss=", network: ws"
   fi   
   
   if [ ! -z "$host" ]; then
      host="host: $host"
   fi
   
   if [ ! -z "$custom" ] && [ "$type" = "vmess" ]; then
      custom=", ws-headers: { Host: $custom }"
   fi
   
   if [ "$tls" = "true" ] && [ "$type" = "vmess" ]; then
      tlss=", tls: $tls"
   elif [ "$tls" = "true" ]; then
      tlss=", tls: $tls"
   fi
   
   if [ ! -z "$path" ]; then
      if [ "$type" != "vmess" ]; then
         paths="path: '$path'"
      else
         path=", ws-path: $path"
      fi
   fi

   if [ "$skip_cert_verify" = "true" ] && [ "$type" != "ss" ]; then
      skip_cert_verifys=", skip-cert-verify: $skip_cert_verify"
   elif [ "$skip_cert_verify" = "true" ]; then
      skip_cert_verifys=", skip-cert-verify: $skip_cert_verify"
   fi

   
   if [ "$type" = "ss" ] && [ "$obfs" = " " ]; then
      echo "- { name: \"$name\", type: $type, server: $server, port: $port, cipher: $cipher, password: "$password"$udpp }" >>$SERVER_FILE
   elif [ "$type" = "ss" ] && [ "$obfs" = "websocket" ] || [ "$obfs" = "tls" ] ||[ "$obfs" = "http" ]; then
cat >> "$SERVER_FILE" <<-EOF
- name: "$name"
  type: $type
  server: $server
  port: $port
  cipher: $cipher
  password: "$password"
EOF
  if [ ! -z "$udp" ]; then
cat >> "$SERVER_FILE" <<-EOF
  udp: $udp
EOF
  fi
  if [ ! -z "$obfss" ] && [ ! "$host" ]; then
cat >> "$SERVER_FILE" <<-EOF
  $obfss
  plugin-opts:
    mode: $obfs
EOF
  fi
  if [ ! -z "$obfss" ] && [ "$host" ]; then
cat >> "$SERVER_FILE" <<-EOF
  $obfss
  plugin-opts:
    mode: $obfs
    $host
EOF
  fi
  if [ "$tls_custom" = "true" ] && [ "$type" = "ss" ]; then
cat >> "$SERVER_FILE" <<-EOF
    tls: true
EOF
  fi
   if [ "$skip_cert_verify" = "true" ] && [ "$type" = "ss" ]; then
cat >> "$SERVER_FILE" <<-EOF
    skip_cert_verifys: true
EOF
  fi

  if [ ! -z "$path" ]; then
cat >> "$SERVER_FILE" <<-EOF
    $paths
EOF
  fi

  if [ ! -z "$custom_host" ]; then
cat >> "$SERVER_FILE" <<-EOF
    host: "$custom_host"
EOF
  fi

  if [ ! -z "$custom" ]; then
cat >> "$SERVER_FILE" <<-EOF
    headers:
      custom: $custom
EOF
  fi
   fi
   
   if [ "$type" = "vmess" ]; then
      echo "- { name: \"$name\", type: $type, server: $server, port: $port, uuid: $uuid, alterId: $alterId, cipher: $securitys$obfs_vmesss$path$custom$tlss$skip_cert_verifys }" >>$SERVER_FILE
   fi
   
   if [ "$type" = "socks5" ] || [ "$type" = "http" ]; then
      echo "- { name: \"$name\", type: $type, server: $server, port: $port, username: $auth_name, password: $auth_pass$skip_cert_verify$tls }" >>$SERVER_FILE
   fi
}

config_load clash
config_foreach servers_set "servers"

size=$(ls -l $SERVER_FILE|awk '{print $5}')
if [ $size -ne 0 ]; then
sed -i "1i\Proxy:" $SERVER_FILE

egrep '^ {0,}-' $SERVER_FILE |grep name: |awk -F 'name: ' '{print $2}' |sed 's/,.*//' >$Proxy_Group 2>&1
sed -i "s/^ \{0,\}/      - /" $Proxy_Group 2>/dev/null 

cat >> "$Proxy_Group_url" <<-EOF
  - name: Auto - UrlTest
    type: url-test
    proxies:
EOF
cat $Proxy_Group >> $Proxy_Group_url 2>/dev/null
cat >> "$Proxy_Group_url" <<-EOF
    url: http://www.gstatic.com/generate_204
    interval: "600"
  - name: Proxy
    type: select
    proxies:
      - Auto - UrlTest
      - DIRECT
EOF
cat $Proxy_Group >> $Proxy_Group_url 2>/dev/null

sed -i "1i  " $Proxy_Group_url
sed -i "2i\Proxy Group:" $Proxy_Group_url

cat $Proxy_Group_url $CONFIG_YAML_RULE > $RULE_PROXY

mode=$(uci get clash.config.mode 2>/dev/null)
da_password=$(uci get clash.config.dash_pass 2>/dev/null)
redir_port=$(uci get clash.config.redir_port 2>/dev/null)
http_port=$(uci get clash.config.http_port 2>/dev/null)
socks_port=$(uci get clash.config.socks_port 2>/dev/null)
dash_port=$(uci get clash.config.dash_port 2>/dev/null)
log_level=$(uci get clash.config.level 2>/dev/null)
			
cat >> "$TEMP_FILE" <<-EOF		
	port: ${http_port}
	socks-port: ${socks_port}
	redir-port: ${redir_port}
	allow-lan: true
	mode: Rule
	log-level: ${log_level}
	external-controller: 0.0.0.0:${dash_port}
	secret: '${da_password}'
	external-ui: "/usr/share/clash/dashboard"
				
	#experimental:
	#  ignore-resolve-fail: true

	#local SOCKS5/HTTP(S) server
	#authentication:
	# - "user1:pass1"
	# - "user2:pass2"

	dns:
	 enable: true
	 listen: 0.0.0.0:5300
	 enhanced-mode: fake-ip
	 fake-ip-range: 198.18.0.1/24
	 # hosts:
	 #   '*.clash.dev': 127.0.0.1
	 #   'alpha.clash.dev': '::1'
	 nameserver: 
	  - 101.132.183.99
	  - 8.8.8.8
	  - 119.29.29.29 
	  - 114.114.114.114
	  - 114.114.115.115    
	  - tls://dns.rubyfish.cn:853
	  - https://1.1.1.1/dns-query 	 
 
EOF

cat $TEMP_FILE $SERVER_FILE > $SERVERS

if [ -f $CONFIG_YAML ]; then
rm -rf $CONFIG_YAML
fi 

cat $SERVERS $RULE_PROXY > $CONFIG_YAML
rm -rf  $SERVERS $RULE_PROXY $Proxy_Group $TEMP_FILE $Proxy_Group_url
fi
rm -rf  $SERVER_FILE
fi
/etc/init.d/clash restart 2>/dev/null