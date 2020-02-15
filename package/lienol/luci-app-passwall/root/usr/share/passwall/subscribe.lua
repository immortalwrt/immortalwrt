#!/usr/bin/lua
------------------------------------------------
-- This file from luci-app-ssr-plus transplant to luci-app-passwall
-- This file is part of the luci-app-ssr-plus subscribe.lua
-- @author William Chan <root@williamchan.me>
------------------------------------------------
require 'nixio'
require 'luci.util'
require 'luci.jsonc'
require 'luci.sys'

local params = arg[1]

-- these global functions are accessed all the time by the event handler
-- so caching them is worth the effort
local luci = luci
local tinsert = table.insert
local ssub, slen, schar, sbyte, sformat, sgsub = string.sub, string.len, string.char, string.byte, string.format, string.gsub
local jsonParse, jsonStringify = luci.jsonc.parse, luci.jsonc.stringify
local b64decode = nixio.bin.b64decode
local cache = {}
local nodeResult = setmetatable({}, { __index = cache })  -- update result
local name = 'passwall'
local uciType = 'nodes'
local ucic = luci.model.uci.cursor()
local api = require "luci.model.cbi.passwall.api.api"

local log = function(...)
	print(os.date("%Y-%m-%d %H:%M:%S: ") .. table.concat({ ... }, " "))
end

-- 分割字符串
local function split(full, sep)
	full = full:gsub("%z", "")  -- 这里不是很清楚 有时候结尾带个\0
	local off, result = 1, {}
	while true do
		local nEnd = full:find(sep, off)
		if not nEnd then
			local res = ssub(full, off, slen(full))
			if #res > 0 then -- 过滤掉 \0
				tinsert(result, res)
			end
			break
		else
			tinsert(result, ssub(full, off, nEnd - 1))
			off = nEnd + slen(sep)
		end
	end
	return result
end
-- urlencode
local function get_urlencode(c)
	return sformat("%%%02X", sbyte(c))
end

local function urlEncode(szText)
	local str = szText:gsub("([^0-9a-zA-Z ])", get_urlencode)
	str = str:gsub(" ", "+")
	return str
end

local function get_urldecode(h)
	return schar(tonumber(h, 16))
end
local function UrlDecode(szText)
	return szText:gsub("+", " "):gsub("%%(%x%x)", get_urldecode)
end

-- trim
local function trim(text)
	if not text or text == "" then
		return ""
	end
	return (sgsub(text, "^%s*(.-)%s*$", "%1"))
end
-- md5
local function md5(content)
	local stdout = luci.sys.exec('echo \"' .. urlEncode(content) .. '\" | md5sum | cut -d \" \"  -f1')
	-- assert(nixio.errno() == 0)
	return trim(stdout)
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
		is_sub = (add_mode and add_mode == "导入") and 0 or 1
	}
	result.hashkey = type(content) == 'string' and md5(content) or md5(jsonStringify(content))
	if szType == 'ssr' then
		local dat = split(content, "/\\?")
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
		result.obfs_param = base64Decode(params.bfsparam)
		result.protocol_param = base64Decode(params.protoparam)
		local group = base64Decode(params.group)
		if group then
			result.group = group
		end
		result.remarks = base64Decode(params.remarks)
	elseif szType == 'vmess' then
		local info = jsonParse(content)
		result.type = 'V2ray'
		result.address = info.add
		result.port = info.port
		result.v2ray_transport = info.net
		result.v2ray_VMess_alterId = info.aid
		result.v2ray_VMess_id = info.id
		result.remarks = info.ps
		result.v2ray_mux = 1
		result.v2ray_mux_concurrency = 8
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
		if not info.security then
			result.v2ray_security = "auto"
		end
		if info.tls == "tls" or info.tls == "1" then
			result.v2ray_stream_security = "tls"
			result.tls_serverName = info.host
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
		if host[2]:find("/\\?") then
			local query = split(host[2], "/\\?")
			result.port = query[1]
			local params = {}
			for _, v in pairs(split(query[2], '&')) do
				local t = split(v, '=')
				params[t[1]] = t[2]
			end
			if params.lugin then
				local plugin_info = UrlDecode(params.lugin)
				local idx_pn = plugin_info:find(";")
				if idx_pn then
						result.ss_plugin = plugin_info:sub(1, idx_pn - 1)
						result.ss_plugin_opts = plugin_info:sub(idx_pn + 1, #plugin_info)
				else
						result.ss_plugin = plugin_info
				end
			end
		else
			result.port = host[2]
		end
		result.ss_encrypt_method = method
		result.password = password
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
	end
	if not result.remarks then
		result.remarks = result.address .. ':' .. result.port
	end
	return result
end
-- wget
local function wget(url)
	local stdout = luci.sys.exec('/usr/bin/wget --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36" --no-check-certificate -t 3 -T 10 -O- "' .. url .. '"')
	return trim(stdout)
end

local function truncate_nodes()
	local is_stop = 0
	local tcp_node_num = ucic:get_first(name, "global_other", "tcp_node_num", 1)
	local udp_node_num = ucic:get_first(name, "global_other", "udp_node_num", 1)
	local socks5_node_num = ucic:get_first(name, "global_other", "socks5_node_num", 1)
	for i = 1, tcp_node_num, 1 do
		local node = ucic:get_first(name, "global", "tcp_node"..i, nil)
		if node and node ~= "nil" then
			local is_sub_node = api.uci_get_type_id(node, "is_sub", "0")
			if is_sub_node == "1" then
				is_stop = 1
				ucic:set(name, ucic:get_first(name, 'global'), "tcp_node"..i, "nil")
			end
		end
	end

	for i = 1, udp_node_num, 1 do
		local node = ucic:get_first(name, "global", "udp_node"..i, nil)
		if node and node ~= "nil" then
			local is_sub_node = api.uci_get_type_id(node, "is_sub", "0")
			if is_sub_node == "1" then
				is_stop = 1
				ucic:set(name, ucic:get_first(name, 'global'), "udp_node"..i, "nil")
			end
		end
	end

	for i = 1, socks5_node_num, 1 do
		local node = ucic:get_first(name, "global", "socks5_node"..i, nil)
		if node and node ~= "nil" then
			local is_sub_node = api.uci_get_type_id(node, "is_sub", "0")
			if is_sub_node == "1" then
				is_stop = 1
				ucic:set(name, ucic:get_first(name, 'global'), "socks5_node"..i, "nil")
			end
		end
	end

	ucic:foreach(name, uciType, function(old)
		if old.is_sub and old.is_sub == "1" then
			ucic:delete(name, old['.name'])
		end
	end)
	ucic:commit(name)

	if is_stop == 1 then
		luci.sys.call("/etc/init.d/" .. name .. " restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
	end
	log('在线订阅节点已全部删除')
end

local function update_node(manual)
	assert(next(nodeResult), "node result is empty")
	local add, del = 0, 0
	ucic:foreach(name, uciType, function(old)
		if old.grouphashkey or old.hashkey then -- 没有 hash 的不参与删除
			if manual == 0 and (old.is_sub and old.is_sub == "1") then
				if not nodeResult[old.grouphashkey] or not nodeResult[old.grouphashkey][old.hashkey] then
					ucic:delete(name, old['.name'])
					del = del + 1
				else
					local dat = nodeResult[old.grouphashkey][old.hashkey]
					ucic:tset(name, old['.name'], dat)
					-- 标记一下
					setmetatable(nodeResult[old.grouphashkey][old.hashkey], { __index =  { _ignore = true } })
				end
			elseif manual == 1 and (old.add_mode and old.add_mode == "导入") then
				if not nodeResult[old.grouphashkey] or not nodeResult[old.grouphashkey][old.hashkey] then
					ucic:delete(name, old['.name'])
					del = del + 1
				else
					local dat = nodeResult[old.grouphashkey][old.hashkey]
					ucic:tset(name, old['.name'], dat)
					-- 标记一下
					setmetatable(nodeResult[old.grouphashkey][old.hashkey], { __index =  { _ignore = true } })
				end
			end
		else
			--log('忽略手动添加的节点: ' .. old.remarks)
		end
	end)
	for k, v in ipairs(nodeResult) do
		for kk, vv in ipairs(v) do
			if not vv._ignore then
				local section = ucic:add(name, uciType)
				ucic:tset(name, section, vv)
				add = add + 1
			end

		end
	end
	ucic:commit(name)
	-- 如果节点已经不见了把帮换一个
	local globalServer = ucic:get_first(name, 'global', 'tcp_node1', '')
	local firstServer = ucic:get_first(name, uciType)
	if not ucic:get(name, globalServer) then
		if firstServer then
			ucic:set(name, ucic:get_first(name, 'global'), 'tcp_node1', firstServer)
			ucic:commit(name)
			log('当前主服务器已更新，正在自动更换。')
		end
		luci.sys.call("/etc/init.d/" .. name .. " restart > /dev/null 2>&1 &") -- 不加&的话日志会出现的更早
	end
	log('新增节点数量: ' ..add, '删除节点数量: ' .. del)
end

local function parse_link(raw, md5_str, manual)
	if raw and #raw > 0 then
		local add_mode
		local nodes, szType
		local groupHash = md5_str or md5(raw)
		cache[groupHash] = {}
		tinsert(nodeResult, {})
		local index = #nodeResult
		-- SSD 似乎是这种格式 ssd:// 开头的
		if raw:find('ssd://') then
			szType = 'ssd'
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
						if dat[1] == 'ss' then
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
						result.remarks:find("官网") or
						not result.address
					then
						log('丢弃无效节点: ' .. result.type ..' 节点, ' .. result.remarks)
					else
						--log('成功解析: ' .. result.type ..' 节点, ' .. result.remarks)
						result.grouphashkey = groupHash
						tinsert(nodeResult[index], result)
						cache[groupHash][result.hashkey] = nodeResult[index][#nodeResult[index]]
					end
				end
			end
		end
		log('成功解析节点数量: ' ..#nodes)
	end
end

local execute = function()
	-- exec
	do
		ucic:foreach(name, "subscribe_list", function(obj)
			local enabled = obj.enabled or nil
			if enabled and enabled == "1" then
				local remark = obj.remark
				local url = obj.url
				local md5_str = md5(url)
				local raw = wget(url)
				parse_link(raw, md5_str)
			end
		end)
	end
	-- diff
	do
		update_node(0)
	end
end

if arg[1] then
	if arg[1] == "start" then
		count = luci.sys.exec("echo -n $(uci show " .. name .." | grep @subscribe_list | grep -c \"enabled='1'\")")
		if count and tonumber(count) > 0 then
			log('开始订阅...')
			xpcall(execute, function(e)
				log(e)
				log(debug.traceback())
				log('发生错误, 正在恢复服务')
			end)
			log('订阅完毕...')
		end
	elseif arg[1] == "add" then
		local f = assert(io.open("/tmp/links.conf",'r'))
		local content = f:read('*all')
		f:close()
		local nodes = split(content:gsub(" ", "\n"), "\n")
		for _, raw in ipairs(nodes) do
			local md5_str = md5(raw)
			parse_link(raw, md5_str, 1)
		end
		update_node(1)
	elseif arg[1] == "truncate" then
		truncate_nodes()
	end
end