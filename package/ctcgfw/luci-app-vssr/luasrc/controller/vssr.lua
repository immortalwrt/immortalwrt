-- Copyright (C) 2018 jerrykuku <jerrykuku@qq.com>
-- Licensed to the public under the GNU General Public License v3.
module("luci.controller.vssr", package.seeall)

function index()
    if not nixio.fs.access("/etc/config/vssr") then 
      return 
end

    if nixio.fs.access("/usr/bin/ssr-redir") then
	entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false	
        entry({"admin", "vpn", "vssr"},
              alias("admin", "vpn", "vssr", "client"), _("Hello World"), 10).dependent =
            true -- 首页
        entry({"admin", "vpn", "vssr", "client"}, cbi("vssr/client"),
              _("SSR Client"), 10).leaf = true -- 基本设置
        entry({"admin", "vpn", "vssr", "servers"}, cbi("vssr/servers"),
              _("Node List"), 11).leaf = true -- 服务器节点
        entry({"admin", "vpn", "vssr", "servers"},
              arcombine(cbi("vssr/servers"), cbi("vssr/client-config")),
              _("Node List"), 11).leaf = true -- 编辑节点
      entry({"admin", "vpn", "vssr", "subscription"},cbi("vssr/subscription"),
             _("Subscription"),12).leaf = true
        entry({"admin", "vpn", "vssr", "control"}, cbi("vssr/control"),
              _("Access Control"), 13).leaf = true -- 访问控制
       entry({"admin", "vpn", "vssr", "servers-list"}, cbi("vssr/servers-list"),
              _("Severs Nodes"), 14).leaf = true
      entry({"admin", "vpn", "vssr", "appointlist"},form("vssr/appointlist"),
             _("Appointlist List"), 15).leaf = true

        entry({"admin", "vpn", "vssr", "udp2raw"},cbi("vssr/udp2raw"),
            _("udp2raw tunnel"),16).leaf = true

        entry({"admin", "vpn", "vssr", "advanced"}, cbi("vssr/advanced"),
              _("Advanced Settings"), 17).leaf = true -- 高级设置
    elseif nixio.fs.access("/usr/bin/ssr-server") then
        entry({"admin", "vpn", "vssr"},
              alias("admin", "vpn", "vssr", "server"), _("vssr"), 10).dependent =
            true
    else
        return
    end

    if nixio.fs.access("/usr/bin/ssr-server") then
        entry({"admin", "vpn", "vssr", "server"},
              arcombine(cbi("vssr/server"), cbi("vssr/server-config")),
              _("SSR Server"), 20).leaf = true
    end
entry({"admin", "vpn", "vssr", "status"},form("vssr/status"),_("Status"), 23).leaf = true
    
entry({"admin", "vpn", "vssr", "logview"}, cbi("vssr/logview", {hideapplybtn=true, hidesavebtn=true, hideresetbtn=true}), _("Log") ,80).leaf=true
      

    entry({"admin", "vpn", "vssr", "refresh"}, call("refresh_data")) -- 更新白名单和GFWLIST
    entry({"admin", "vpn", "vssr", "checkport"}, call("check_port")) -- 检测单个端口并返回Ping
    entry({"admin", "vpn", "vssr", "checkports"}, call("check_ports"))
    entry({"admin", "vpn", "vssr", "ping"}, call("act_ping")).leaf=true
   entry({"admin", "vpn", "vssr", "fileread"}, call("act_read"), nil).leaf=true
    entry({"admin", "vpn", "vssr", "run"}, call("act_status")) -- 检测全局服务器状态
    entry({"admin", "vpn", "vssr", "change"}, call("change_node")) -- 切换节点
    entry({"admin", "vpn", "vssr", "allserver"}, call("get_servers")) -- 获取所有节点Json
    entry({"admin", "vpn", "vssr", "subscribe"}, call("get_subscribe")) -- 执行订阅
    entry({"admin", "vpn", "vssr", "flag"}, call("get_flag")) -- 获取节点国旗 iso code
    entry({"admin", "vpn", "vssr", "ip"}, call("check_ip")) -- 获取ip情况

end

-- 执行订阅
function get_subscribe()

    local cjson = require "cjson"
    local e = {}
    local uci = luci.model.uci.cursor()
    local auto_update = luci.http.formvalue("auto_update")
    local auto_update_time = luci.http.formvalue("auto_update_time")
    local proxy = luci.http.formvalue("proxy")
    local subscribe_url = luci.http.formvalue("subscribe_url")
    if subscribe_url ~= "[]" then
        local cmd1 = 'uci set vssr.@server_subscribe[0].auto_update="' ..
                         auto_update .. '"'
        local cmd2 = 'uci set vssr.@server_subscribe[0].auto_update_time="' ..
                         auto_update_time .. '"'
        local cmd3 = 'uci set vssr.@server_subscribe[0].proxy="' .. proxy .. '"'
        luci.sys.call('uci delete vssr.@server_subscribe[0].subscribe_url ')
        luci.sys.call(cmd1)
        luci.sys.call(cmd2)
        luci.sys.call(cmd3)
        for k, v in ipairs(cjson.decode(subscribe_url)) do
            luci.sys.call(
                'uci add_list vssr.@server_subscribe[0].subscribe_url="' .. v ..
                    '"')
        end
        luci.sys.call('uci commit vssr')
        luci.sys.call(
            "nohup /usr/bin/lua /usr/share/vssr/subscribe.lua >/www/check_update.htm 2>/dev/null &")
        
        e.error = 0
    else
        e.error = 1
    end

    luci.http.prepare_content("application/json")
    luci.http.write_json(e)

end


-- 获取所有节点
function get_servers()
    local uci = luci.model.uci.cursor()
    local server_table = {}
    uci:foreach("vssr", "servers", function(s)
        local e = {}
        e["name"] = s[".name"]
        local t1 = luci.sys.exec(
                       "ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'" %
                           s["server"])
        e["t1"] = t1
        table.insert(server_table, e)
    end)
    luci.http.prepare_content("application/json")
    luci.http.write_json(server_table)
end

-- 切换节点
function change_node()
    local e = {}
    local uci = luci.model.uci.cursor()
    local sid = luci.http.formvalue("set")
    local name = ""
    uci:foreach("vssr", "global", function(s) name = s[".name"] end)
    e.status = false
    e.sid = sid
    if sid ~= "" then
        uci:set("vssr", name, "global_server", sid)
        uci:commit("vssr")
     luci.sys.call("/etc/init.d/vssr restart")
        e.status = true
    end
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

-- 检测全局服务器状态
function act_status()
    math.randomseed(os.time())
    local e = {}
    -- 全局服务器
   e.global=luci.sys.call("ps -w | grep ssr-retcp | grep -v grep >/dev/null") == 0 

--检测kcptun状态
    if tonumber(luci.sys.exec("ps -w | grep kcptun-client |grep -v grep| wc -l"))>0 then
		e.kcptun= true  
                end
--检测HTTP代理状态
                 if luci.sys.call("pidof privoxy >/dev/null") == 0 then
		e.privoxy= true  
                  end
      --检测chinadns状态
	if tonumber(luci.sys.exec("ps -w | grep chinadns |grep -v grep| wc -l"))>0 then
		e.chinadns= true
	elseif tonumber(luci.sys.exec("ps -w | grep dnsparsing |grep -v grep| wc -l"))>0 then
		e.chinadns= true
	elseif tonumber(luci.sys.exec("ps -w | grep dnscrypt-proxy |grep -v grep| wc -l"))>0 then
		e.chinadns= true
                   elseif tonumber(luci.sys.exec("ps -w | grep pdnsd |grep -v grep| wc -l"))>0 then
		e.chinadns= true
           
               elseif tonumber(luci.sys.exec("ps -w | grep dnsforwarder |grep -v grep| wc -l"))>0 then
		e.chinadns= true
             
          end
   --检测SOCKS5状态
	if tonumber(luci.sys.exec("ps -w | grep ssr-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
	elseif tonumber(luci.sys.exec("ps -w | grep ss-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
	elseif tonumber(luci.sys.exec("ps -w | grep v2-ssr-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
                  elseif tonumber(luci.sys.exec("ps -w | grep trojan-ssr-local |grep -v grep| wc -l"))>0 then
		e.SOCKS5= true
	end  
--检测UDP2RAW状态
                 if tonumber(luci.sys.exec("ps -w | grep udp2raw |grep -v grep| wc -l"))>0 then
		e.udp2raw= true  
                  end
--检测UDPspeeder状态
                 if tonumber(luci.sys.exec("ps -w | grep udpspeeder |grep -v grep| wc -l"))>0 then
		e.udpspeeder= true  
                  end
 --检测服务端状态
	if tonumber(luci.sys.exec("ps -w | grep ssr-server |grep -v grep| wc -l"))>0 then
		e.server= true
                    end 

                  if luci.sys.call("pidof ssr-server >/dev/null") == 0 then
                   e.ssr_server= true

                   end 
	if luci.sys.call("pidof ss-server >/dev/null") == 0 then
		e.ss_server= true

                     end 

	if luci.sys.call("ps -w | grep v2ray-server | grep -v grep >/dev/null") == 0 then
		e.v2_server= true


	end    
    -- 检测游戏模式状态
       e.game = false
    if tonumber(luci.sys.exec("ps -w | grep ssr-reudp |grep -v grep| wc -l"))>0 then
        e.game= true
    else
        if tonumber(luci.sys.exec("ps -w | grep ssr-retcp |grep \"\\-u\"|grep -v grep| wc -l"))>0 then
            e.game= true
        end
    end

    -- 检测国外通道
    --[[    http=require("socket.http")
	http.TIMEOUT = 1
	result=http.request("http://ip111cn.appspot.com/?z="..math.random(1,100000))
	if result then
		e.google = result
	else
		e.google = false
	end
	
	result1=http.request("http://45.32.164.128/ip.php?z="..math.random(1,100000))
	if result1 then
		e.outboard = result1
	else
		e.outboard = false
	end
	--]]

   -- 检测国内通道
    e.baidu = false
    sret = luci.sys.call("/usr/bin/ssr-check www.baidu.com 80 3 1")
    if sret == 0 then
        e.baidu =  true
    end
    
    -- 检测国外通道
    e.google = false
    sret = luci.sys.call("/usr/bin/ssr-check www.google.com 80 3 1")
    if sret == 0 then
        e.google =  true
    end
    

    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function act_ping()
	local e = {}
	local domain = luci.http.formvalue("domain")
	local port = luci.http.formvalue("port")
	e.index = luci.http.formvalue("index")
	local iret = luci.sys.call(" ipset add ss_spec_wan_ac " .. domain .. " 2>/dev/null")
	local socket = nixio.socket("inet", "stream")
	socket:setopt("socket", "rcvtimeo", 3)
	socket:setopt("socket", "sndtimeo", 3)
	e.socket = socket:connect(domain, port)
	socket:close()
	e.ping = luci.sys.exec("ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'" % domain)
	if (iret == 0) then
		luci.sys.call(" ipset del ss_spec_wan_ac " .. domain)
	end
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end




function check_status()
	local set ="/usr/bin/ssr-check www." .. luci.http.formvalue("set") .. ".com 80 3 1"
	sret=luci.sys.call(set)
	if sret== 0 then
		retstring ="0"
	else
		retstring ="1"
	end	

	luci.http.prepare_content("application/json")
	luci.http.write_json({ ret=retstring })
end

-- 刷新检测文件
function refresh_data()
local set =luci.http.formvalue("set")
local icount =0

if set == "gfw_data" then
	refresh_cmd="wget-ssl --no-check-certificate https://cdn.jsdelivr.net/gh/gfwlist/gfwlist/gfwlist.txt -O /tmp/gfw.b64"
	sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
	if sret== 0 then
	luci.sys.call("/usr/bin/vssr-gfw")
	icount = luci.sys.exec("cat /tmp/gfwnew.txt | wc -l")
	if tonumber(icount)>1000 then
	oldcount=luci.sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l")
	if tonumber(icount) ~= tonumber(oldcount) then
		luci.sys.exec("cp -f /tmp/gfwnew.txt /etc/dnsmasq.ssr/gfw_list.conf")
		luci.sys.exec("cp -f /tmp/gfwnew.txt /tmp/dnsmasq.ssr/gfw_list.conf")
		luci.sys.call("/etc/init.d/dnsmasq restart")
		retstring=tostring(math.ceil(tonumber(icount)/2))
	else
		retstring ="0"
	end
	else
	retstring ="-1"
	end
	luci.sys.exec("rm -f /tmp/gfwnew.txt ")
else
	retstring ="-1"
end
elseif set == "ip_data" then
	if (luci.model.uci.cursor():get_first('vssr', 'global', 'chnroute', '0') == '1') then
		refresh_cmd="wget-ssl --no-check-certificate -O - " .. luci.model.uci.cursor():get_first('vssr', 'global', 'chnroute_url', 'https://ispip.clang.cn/all_cn.txt') .. ' > /tmp/china_ssr.txt 2>/dev/null'
	else
		refresh_cmd="wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest'  2>/dev/null| awk -F\\| '/CN\\|ipv4/ { printf(\"%s/%d\\n\", $4, 32-log($5)/log(2)) }' > /tmp/china_ssr.txt"
	end
	sret=luci.sys.call(refresh_cmd)
	icount = luci.sys.exec("cat /tmp/china_ssr.txt | wc -l")
	if sret== 0 and tonumber(icount)>1000 then
		oldcount=luci.sys.exec("cat /etc/china_ssr.txt | wc -l")
		if tonumber(icount) ~= tonumber(oldcount) then
			luci.sys.exec("cp -f /tmp/china_ssr.txt /etc/china_ssr.txt")
			retstring=tostring(tonumber(icount))
		else
			retstring ="0"
		end
	else
		retstring ="-1"
	end
	luci.sys.exec("rm -f /tmp/china_ssr.txt ")
else
if nixio.fs.access("/usr/bin/wget-ssl") then
	refresh_cmd="wget-ssl --no-check-certificate -O - ".. luci.model.uci.cursor():get_first('vssr', 'global', 'adblock_url','https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt') .." > /tmp/adnew.conf"
end
sret=luci.sys.call(refresh_cmd .. " 2>/dev/null")
if sret== 0 then
	luci.sys.call("/usr/bin/vssr-ad")
	icount = luci.sys.exec("cat /tmp/ad.conf | wc -l")
	if tonumber(icount)>1000 then
	if nixio.fs.access("/etc/dnsmasq.ssr/ad.conf") then
		oldcount=luci.sys.exec("cat /etc/dnsmasq.ssr/ad.conf | wc -l")
	else
		oldcount=0
	end
	if tonumber(icount) ~= tonumber(oldcount) then
		luci.sys.exec("cp -f /tmp/ad.conf /etc/dnsmasq.ssr/ad.conf")
		luci.sys.exec("cp -f /tmp/ad.conf /tmp/dnsmasq.ssr/ad.conf")
		luci.sys.call("/etc/init.d/dnsmasq restart")
		retstring=tostring(math.ceil(tonumber(icount)))
	else
		retstring ="0"
	end
	else
	retstring ="-1"
	end
	luci.sys.exec("rm -f /tmp/ad.conf")
else
	retstring ="-1"
end
end
luci.http.prepare_content("application/json")
luci.http.write_json({ ret=retstring ,retcount=icount})
end



-- 检测所有服务器
function check_ports()
    local set = ""
    local retstring = "<br /><br />"
    local s
    local server_name = ""
    local vssr = "vssr"
    local uci = luci.model.uci.cursor()
    local iret = 1

    uci:foreach(
        vssr,
        "servers",
        function(s)
            if s.alias then
                server_name = s.alias
            elseif s.server and s.server_port then
                server_name = "%s:%s" % {s.server, s.server_port}
            end
            iret = luci.sys.call(" ipset add ss_spec_wan_ac " .. s.server .. " 2>/dev/null")
            socket = nixio.socket("inet", "stream")
            socket:setopt("socket", "rcvtimeo", 3)
            socket:setopt("socket", "sndtimeo", 3)
            ret = socket:connect(s.server, s.server_port)
            if tostring(ret) == "true" then
                socket:close()
                retstring = retstring .. "<font color='green'>[" .. server_name .. "] OK.</font><br />"
            else
                retstring = retstring .. "<font color='red'>[" .. server_name .. "] Error.</font><br />"
            end
            if iret == 0 then
                luci.sys.call(" ipset del ss_spec_wan_ac " .. s.server)
            end
        end
    )

    luci.http.prepare_content("application/json")
    luci.http.write_json({ret = retstring})
end


-- 检测单个节点状态并返回连接速度
function check_port()

    local e = {}
    -- e.index=luci.http.formvalue("host")
    local t1 = luci.sys.exec(
                   "ping -c 1 -W 1 %q 2>&1 | grep -o 'time=[0-9]*.[0-9]' | awk -F '=' '{print$2}'" %
                       luci.http.formvalue("host"))
    luci.http.prepare_content("application/json")
    luci.http.write_json({ret = 1, used = t1})

end

function JudgeIPString(ipStr)
    if type(ipStr) ~= "string" then return false end

    -- 判断长度
    local len = string.len(ipStr)
    if len < 7 or len > 15 then -- 长度不对
        return false
    end

    -- 判断出现的非数字字符
    local point = string.find(ipStr, "%p", 1) -- 字符"."出现的位置
    local pointNum = 0 -- 字符"."出现的次数 正常ip有3个"."
    while point ~= nil do
        if string.sub(ipStr, point, point) ~= "." then -- 得到非数字符号不是字符"."
            return false
        end
        pointNum = pointNum + 1
        point = string.find(ipStr, "%p", point + 1)
        if pointNum > 3 then return false end
    end
    if pointNum ~= 3 then -- 不是正确的ip格式
        return false
    end

    -- 判断数字对不对
    local num = {}
    for w in string.gmatch(ipStr, "%d+") do
        num[#num + 1] = w
        local kk = tonumber(w)
        if kk == nil or kk > 255 then -- 不是数字或超过ip正常取值范围了
            return false
        end
    end

    if #num ~= 4 then -- 不是4段数字
        return false
    end

    return ipStr
end

-- 检测 当前节点ip 和 网站访问情况
function check_ip()
    local e = {}
    http = require("socket.http")
    http.TIMEOUT = 1

    result = luci.sys.exec("curl -s https://api.ip.sb/ip")
    if JudgeIPString(result) then
        local cmd = '/usr/share/vssr/getip.sh '..result
        e.outboard = result
        e.outboardip = luci.sys.exec(cmd)
    else
        e.outboard = false
    end

    -- 检测国内通道
    e.baidu = false
    sret1 = luci.sys.call("/usr/bin/ssr-check www.baidu.com 80 3 1")
    if sret1 == 0 then e.baidu = true end

    e.taobao = false
    sret2 = luci.sys.call("/usr/bin/ssr-check www.taobao.com 80 3 1")
    if sret2 == 0 then e.taobao = true end

    -- 检测国外通道
    e.google = false
    sret3 = luci.sys.call("/usr/bin/ssr-check www.google.com 80 3 1")
    if sret3 == 0 then e.google = true end

    e.youtube = false
    sret4 = luci.sys.call("/usr/bin/ssr-check www.youtube.com 80 3 1")
    if sret4 == 0 then e.youtube = true end

    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

-- 获取节点国旗 iso code
function get_flag()
    local e = {}
    local host = luci.http.formvalue("host")
    local remark = luci.http.formvalue("remark")
    local cmd1 = '/usr/share/vssr/getflag.sh "' .. remark .. '" ' .. host
    e.host = host
    e.flag = luci.sys.exec(cmd1)
    luci.http.prepare_content("application/json")
    luci.http.write_json(e)
end

function act_read(lfile)
	local NXFS = require "nixio.fs"
	local HTTP = require "luci.http"
	local lfile = HTTP.formvalue("lfile")
	local ldata={}
	ldata[#ldata+1] = NXFS.readfile(lfile) or "_nofile_"
	if ldata[1] == "" then
		ldata[1] = "_nodata_"
	end
	HTTP.prepare_content("application/json")
	HTTP.write_json(ldata)
end
