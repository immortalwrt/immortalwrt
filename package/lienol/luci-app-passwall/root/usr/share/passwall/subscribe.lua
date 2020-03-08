#!/usr/bin/lua

------------------------------------------------
-- This file from luci-app-ssr-plus transplant to luci-app-passwall
-- This file is part of the luci-app-ssr-plus subscribe.lua
-- @author William Chan <root@williamchan.me>
------------------------------------------------
require 'nixio'
require 'uci'
require 'luci.util'
require 'luci.jsonc'
require 'luci.sys'

-- these global functions are accessed all the time by the event handler
-- so caching them is worth the effort
local luci = luci
local tinsert = table.insert
local ssub, slen, schar, sbyte, sformat, sgsub = string.sub, string.len, string.char, string.byte, string.format, string.gsub
local jsonParse, jsonStringify = luci.jsonc.parse, luci.jsonc.stringify
local b64decode = nixio.bin.b64decode
local nodeResult = {} -- update result
local application = 'passwall'
local uciType = 'nodes'
local ucic2 = uci.cursor()
local arg2 = arg[2]
ucic2:revert(application)

local log = function(...)
	if arg2 then
		local result = os.date("%Y-%m-%d %H:%M:%S: ") .. table.concat({...}, " ")
		if arg2 == "log" then
			local f, err = io.open("/var/log/passwall.log", "a")
			if f and err == nil then
				f:write(result .. "\n")
				f:close()
			end
		elseif arg2 == "print" then
			print(result)
		end
	end
end

-- 获取各项动态配置的当前服务器，可以用 get 和 set， get必须要获取到节点表
local CONFIG = {}
do
	local function import_config(protocol)
		local node_num = ucic2:get(application, "@global_other[0]", protocol .. "_node_num") or 1
		for i = 1, node_num, 1 do
			local name = string.upper(protocol)
			local option = protocol .. "_node" .. i
			local szType = "@global[0]"
			local json = {
				remarks = name .. "节点" .. i,
				type = szType,
				option = option,
				set = function(server)
					ucic2:set(application, szType, option, server)
				end
			}
			CONFIG[name .. "_NODE" .. i] = json
		end
	end
	import_config("tcp")
	import_config("udp")
	import_config("socks5")

	for k, v in pairs(CONFIG) do
		local currentNode
		if v.get then
			currentNode = v.get()
		else
			local cfgid = ucic2:get(application, v.type, v.option)
			if cfgid then
				currentNode = ucic2:get_all(application, cfgid)
			end
		end
		if currentNode then
			CONFIG[k].currentNode = currentNode
		else
			CONFIG[k] = nil
		end
	end
end

-- 分割字符串
local function split(full, sep)
	if full then
		full = full:gsub("%z", "") -- 这里不是很清楚 有时候结尾带个\0
		local off, result = 1, {}
		while true do
			local nStart, nEnd = full:find(sep, off)
			if not nEnd then
				local res = ssub(full, off, slen(full))
				if #res > 0 then -- 过滤掉 \0
					tinsert(result, res)
				end
				break
			else
				tinsert(result, ssub(full, off, nStart - 1))
				off = nEnd + 1
			end
		end
		return result
	end
	return {}
end
-- urlencode
-- local function get_urlencode(c) return sformat("%%%02X", sbyte(c)) end

-- local function urlEncode(szText)
-- 	local str = szText:gsub("([^0-9a-zA-Z ])", get_urlencode)
-- 	str = str:gsub(" ", "+")
-- 	return str
-- end

local function get_urldecode(h) return schar(tonumber(h, 16)) end
local function UrlDecode(szText)
	return szText:gsub("+", " "):gsub("%%(%x%x)", get_urldecode)
end

-- trim
local function trim(text)
	if not text or text == "" then return "" end
	return (sgsub(text, "^%s*(.-)%s*$", "%1"))
end

-- base64
local function base64Decode(text)
	local raw = text
	if not text then return '' end
	text = text:gsub("%z", "")
	text = text:gsub("_", "/")
	text = text:gsub("-", "+")
	local mod4 = #text % 4
	text = text .. string.sub('====', mod4 + 1)
	local result = b64decode(text)
	if result then
		return result:gsub("%z", "")
	else
		return raw
	end
end
-- 处理数据
local function processData(szType, content, add_mode)
	local result = {
		timeout = 60,
		add_mode = add_mode,
		is_sub = add_mode == '导入' and 0 or 1
	}
	if szType == 'ssr' then
		local dat = split(content, "/%?")
		local hostInfo = split(dat[1], ':')
		result.type = "SSR"
		result.address = hostInfo[1]
		result.port = hostInfo[2]
		result.protocol = hostInfo[3]
		result.ssr_encrypt_method = hostInfo[4]
		result.obfs = hostInfo[5]
		result.password = base64Decode(hostInfo[6])
		local params = {}
		for _, v in pairs(split(dat[2], '&')) do
			local t = split(v, '=')
			params[t[1]] = t[2]
		end
		result.obfs_param = base64Decode(params.obfsparam)
		result.protocol_param = base64Decode(params.protoparam)
		local group = base64Decode(params.group)
		if group then result.group = group end
		result.remarks = base64Decode(params.remarks)
	elseif szType == 'vmess' then
		local info = jsonParse(content)
		result.type = 'V2ray'
		result.address = info.add
		result.port = info.port
		result.v2ray_protocol = 'vmess'
		result.v2ray_transport = info.net
		result.v2ray_VMess_alterId = info.aid
		result.v2ray_VMess_id = info.id
		result.remarks = info.ps
		-- result.v2ray_mux = 1
		-- result.v2ray_mux_concurrency = 8
		if info.net == 'ws' then
			result.v2ray_ws_host = info.host
			result.v2ray_ws_path = info.path
		end
		if info.net == 'h2' then
			result.v2ray_h2_host = info.host
			result.v2ray_h2_path = info.path
		end
		if info.net == 'tcp' then
			result.v2ray_tcp_guise = info.type
			result.v2ray_tcp_guise_http_host = info.host
			result.v2ray_tcp_guise_http_path = info.path
		end
		if info.net == 'kcp' then
			result.v2ray_mkcp_guise = info.type
			result.v2ray_mkcp_mtu = 1350
			result.v2ray_mkcp_tti = 50
			result.v2ray_mkcp_uplinkCapacity = 5
			result.v2ray_mkcp_downlinkCapacity = 20
			result.v2ray_mkcp_readBufferSize = 2
			result.v2ray_mkcp_writeBufferSize = 2
		end
		if info.net == 'quic' then
			result.v2ray_quic_guise = info.type
			result.v2ray_quic_key = info.key
			result.v2ray_quic_security = info.securty
		end
		if not info.security then result.v2ray_security = "auto" end
		if info.tls == "tls" or info.tls == "1" then
			result.v2ray_stream_security = "tls"
			result.tls_serverName = info.host
			result.tls_allowInsecure = 1
		else
			result.v2ray_stream_security = "none"
		end
	elseif szType == "ss" then
		local idx_sp = 0
		local alias = ""
		if content:find("#") then
			idx_sp = content:find("#")
			alias = content:sub(idx_sp + 1, -1)
		end
		local info = content:sub(1, idx_sp - 1)
		local hostInfo = split(base64Decode(info), "@")
		local host = split(hostInfo[2], ":")
		local userinfo = base64Decode(hostInfo[1])
		local method = userinfo:sub(1, userinfo:find(":") - 1)
		local password = userinfo:sub(userinfo:find(":") + 1, #userinfo)
		result.remarks = UrlDecode(alias)
		result.type = "SS"
		result.address = host[1]
		if host[2]:find("/%?") then
			local query = split(host[2], "/%?")
			result.port = query[1]
			local params = {}
			for _, v in pairs(split(query[2], '&')) do
				local t = split(v, '=')
				params[t[1]] = t[2]
			end
			if params.plugin then
				local plugin_info = UrlDecode(params.plugin)
				local idx_pn = plugin_info:find(";")
				if idx_pn then
					result.ss_plugin = plugin_info:sub(1, idx_pn - 1)
					result.ss_plugin_opts =
						plugin_info:sub(idx_pn + 1, #plugin_info)
				else
					result.ss_plugin = plugin_info
				end
			end
		else
			result.port = host[2]
		end
		result.ss_encrypt_method = method
		result.password = password
	elseif szType == "trojan" then
		local alias = ""
		if content:find("#") then
			local idx_sp = content:find("#")
			alias = content:sub(idx_sp + 1, -1)
			content = content:sub(0, idx_sp - 1)
		end
		local Info = split(content, "@")
		if Info then
			local address, port, peer
			local password = Info[1]
			local allowInsecure = 1
			local params = {}
			local hostInfo = split(Info[2], ":")
			if hostInfo then
				address = hostInfo[1]
				hostInfo = split(hostInfo[2], "?")
				if hostInfo then
					port = hostInfo[1]
					for _, v in pairs(split(hostInfo[2], '&')) do
						local t = split(v, '=')
						params[t[1]] = t[2]
					end
					if params.allowInsecure then
						allowInsecure = params.allowInsecure
					end
					if params.peer then peer = params.peer end
				end
			end
			result.type = "Trojan"
			result.address = address
			result.port = port
			result.password = password
			result.tls_allowInsecure = allowInsecure
			result.tls_serverName = peer
			result.remarks = UrlDecode(alias)
		end
	elseif szType == "ssd" then
		result.type = "SS"
		result.address = content.server
		result.port = content.port
		result.password = content.password
		result.ss_encrypt_method = content.encryption
		result.ss_plugin = content.plugin
		result.ss_plugin_opts = content.plugin_options
		result.group = content.airport
		result.remarks = content.remarks
	else
		log('暂时不支持' .. szType ..
				"类型的节点订阅，跳过此节点。")
		return nil
	end
	if not result.remarks then
		result.remarks = result.address .. ':' .. result.port
	end
	return result
end
-- wget
local function wget(url)
	local stdout = luci.sys.exec('/usr/bin/wget --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.122 Safari/537.36" --no-check-certificate -t 3 -T 10 -O- "' .. url .. '"')
	return trim(stdout)
end

local function truncate_nodes()
	local is_stop = 0
	local function clear(type)
		local node_num = ucic2:get(application, "@global_other[0]", type .. "_node_num") or 1
		for i = 1, node_num, 1 do
			local node = ucic2:get(application, "@global[0]", type.."_node" .. i)
			if node then
				local is_sub_node = ucic2:get(application, node, "is_sub") or 0
				if is_sub_node == "1" then
					is_stop = 1
					ucic2:set(application, '@global[0]', type.."_node" .. i, "nil")
				end
			end
		end
	end
	clear("tcp")
	clear("udp")
	clear("socks5")

	ucic2:foreach(application, uciType, function(node)
		if (node.is_sub or node.hashkey) and node.add_mode ~= '导入' then
			ucic2:delete(application, node['.name'])
		end
	end)
	ucic2:commit(application)

	if is_stop == 1 then
		luci.sys.call("/etc/init.d/" .. application .. " restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
	end
	log('在线订阅节点已全部删除')
end

local function select_node(nodes, config)
	local server
	-- 第一优先级 IP + 端口
	for id, node in pairs(nodes) do
		if node.address .. ':' .. node.port == config.currentNode.address .. ':' .. config.currentNode.port then
			log('选择【' .. config.remarks .. '】第一匹配节点：' .. node.remarks)
			server = id
			break
		end
	end
	-- 第二优先级 IP
	if not server then
		for id, node in pairs(nodes) do
			if node.address == config.currentNode.address then
				log('选择【' .. config.remarks .. '】第二匹配节点：' .. node.remarks)
				server = id
				break
			end
		end
	end
	-- 第三优先级备注
	if not server then
		for id, node in pairs(nodes) do
			if node.remarks == config.currentNode.remarks then
				log('选择【' .. config.remarks .. '】第三匹配节点：' .. node.remarks)
				server = id
				break
			end
		end
	end
	-- 第四 cfgid
	if not server then
		for id, node in pairs(nodes) do
			if id == config.currentNode['.name'] then
				log('选择【' .. config.remarks .. '】第四匹配节点：' .. node.remarks)
				server = id
				break
			end
		end
	end
	-- 还不行 随便找一个
	if not server then
		server = ucic2:get(application, '@' .. uciType .. '[0]')
		if server then
			log('无法找到最匹配的节点，当前已更换为' .. ucic2:get_all(application, server).remarks)
		end
	end
	if server then
		config.set(server)
	end
end

local function update_node()
	if next(nodeResult) == nil then
		log("更新失败，没有可用的节点信息")
		return
	end
	-- delet all for subscribe nodes
	ucic2:foreach(application, uciType, function(node)
		if (node.is_sub or node.hashkey) and node.add_mode ~= '导入' then
			ucic2:delete(application, node['.name'])
		end
	end)
	for _, v in ipairs(nodeResult) do
		for _, vv in ipairs(v) do
			local cfgid = ucic2:add(application, uciType)
			for kkk, vvv in pairs(vv) do
				ucic2:set(application, cfgid, kkk, vvv)
			end
		end
	end
	ucic2:commit(application)

	if next(CONFIG) then
		local nodes = {}
		local ucic3 = uci.cursor()
		ucic3:foreach(application, uciType, function(node)
			if node.port and node.address and node.remarks then
				nodes[node['.name']] = node
			end
		end)
		for _, config in pairs(CONFIG) do
			select_node(nodes, config)
		end
		ucic3:commit(application)
		luci.sys.call("/etc/init.d/" .. application .. " restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
	end
end

local function parse_link(raw, remark, manual)
	if raw and #raw > 0 then
		local add_mode
		local nodes, szType
		local all_nodes = {}
		tinsert(nodeResult, all_nodes)
		-- SSD 似乎是这种格式 ssd:// 开头的
		if raw:find('ssd://') then
			szType = 'ssd'
			add_mode = remark
			local nEnd = select(2, raw:find('ssd://'))
			nodes = base64Decode(raw:sub(nEnd + 1, #raw))
			nodes = jsonParse(nodes)
			local extra = {
				airport = nodes.airport,
				port = nodes.port,
				encryption = nodes.encryption,
				password = nodes.password
			}
			local servers = {}
			-- SS里面包着 干脆直接这样
			for _, server in ipairs(nodes.servers) do
				tinsert(servers, setmetatable(server, { __index = extra }))
			end
			nodes = servers
		else
			-- ssd 外的格式
			if manual then
				nodes = split(raw:gsub(" ", "\n"), "\n")
				add_mode = '导入'
			else
				nodes = split(base64Decode(raw):gsub(" ", "\n"), "\n")
				add_mode = remark
			end
		end

		for _, v in ipairs(nodes) do
			if v then
				local result
				if szType == 'ssd' then
					result = processData(szType, v, add_mode)
				elseif not szType then
					local node = trim(v)
					local dat = split(node, "://")
					if dat and dat[1] and dat[2] then
						if dat[1] == 'ss' or dat[1] == 'trojan' then
							result = processData(dat[1], dat[2], add_mode)
						else
							result = processData(dat[1], base64Decode(dat[2]), add_mode)
						end
					end
				else
					log('跳过未知类型: ' .. szType)
				end
				-- log(result)
				if result then
					if result.remarks:find("过期时间") or
						result.remarks:find("剩余流量") or
						result.remarks:find("QQ群") or
						result.remarks:find("官网") or not result.address or
						result.address:match("[^0-9a-zA-Z%-%_%.%s]") or -- 中文做地址的 也没有人拿中文域名搞，就算中文域也有Puny Code SB 机场
						not result.address:find("%.") or -- 虽然没有.也算域，不过应该没有人会这样干吧
						result.address:sub(#result.address) == "." -- 结尾是.
					then
						log('丢弃无效节点: ' .. result.type .. ' 节点, ' .. result.remarks)
					else
						tinsert(all_nodes, result)
					end
				end
			end
		end
		log('成功解析节点数量: ' .. #nodes)
	else
		if not manual then
			log('获取到的节点内容为空...')
		end
	end
end

local execute = function()
	-- exec
	do
		ucic2:foreach(application, "subscribe_list", function(obj)
			local enabled = obj.enabled or nil
			if enabled and enabled == "1" then
				local remark = obj.remark
				local url = obj.url
				log('正在订阅: ' .. url)
				local raw = wget(url)
				parse_link(raw, remark)
			end
		end)
		-- diff
		update_node()
	end
end

if arg[1] then
	if arg[1] == "start" then
		local count = luci.sys.exec("echo -n $(uci show " .. application .. " | grep @subscribe_list | grep -c \"enabled='1'\")")
		if count and tonumber(count) > 0 then
			log('开始订阅...')
			xpcall(execute, function(e)
				log(e)
				log(debug.traceback())
				log('发生错误, 正在恢复服务')
			end)
			log('订阅完毕...')
		else
			log('未设置或启用订阅, 请检查设置...')
		end
	elseif arg[1] == "add" then
		local f = assert(io.open("/tmp/links.conf", 'r'))
		local content = f:read('*all')
		f:close()
		local nodes = split(content:gsub(" ", "\n"), "\n")
		for _, raw in ipairs(nodes) do
			parse_link(raw, nil, 1)
		end
		update_node()
	elseif arg[1] == "truncate" then
		truncate_nodes()
	elseif arg[1] == "test" then
		for k, v in pairs(CONFIG) do
			print(k, v)
		end
	end
end
