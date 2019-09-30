-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local ipairs, string, tostring, tonumber, table = ipairs, string, tostring, tonumber, table
local assert, type, unpack = assert, type, unpack

local nixio = require "nixio"
local socket = require "socket"
local xmlrpc = require "xmlrpc"
local scgi = require "xmlrpc.scgi"

local SCGI_ADDRESS = "localhost"
local SCGI_PORT = 5000

local HTTP_AUTH_USER = "rtorrent"
local HTTP_AUTH_PASSWORD = "czNz9JwdcLYcGDcVnZbQ"

module "rtorrent"

function map(array, func)
	local new_array = {}
	for i, v in ipairs(array) do
		new_array[i] = func(v)
	end
	return new_array
end

function alter(prefix, methods, postfix)
	methods = map(methods, function(method)
		if method == 0 then return method end
		if prefix then method = prefix .. method end
		if postfix then method = method .. postfix end
		return method
	end)
	return methods
end

function format(method_type, res, methods)
	local formatted = {}
	for _, r in ipairs(res) do
		local item = {}
		for i, v in ipairs(r) do
			item[methods[method_type == "d." and i or i + 1]:gsub("%.", "_")] = v
		end
		table.insert(formatted, item)
	end
	return formatted
end

function call(method, ...)
	local ok, res = scgi.call(SCGI_ADDRESS, SCGI_PORT, method, ...)
	if not ok and res == "socket connect failed" then
		assert(ok, "\n\nFailed to connect to rtorrent: rpc port not reachable!\n"
			.. "Possible reasons:\n"
			.. "- not the rpc version of rtorrent is installed\n"
			.. "- scgi port is not defined in .rtorrent.rc (scgi_port = 127.0.0.1:5000)\n"
			.. "- rtorrent is not running (ps | grep rtorrent)\n")
	end
	assert(ok, string.format("XML-RPC call failed on client: %s", tostring(res)))
	return res
end

function rpc(xml)
	local auth = "Basic " .. nixio.bin.b64encode(HTTP_AUTH_USER .. ":" .. HTTP_AUTH_PASSWORD)
	if auth ~= nixio.getenv("HTTP_AUTHORIZATION") then
		return 'Status: 401 Unauthorized\r\n'
			.. 'WWW-Authenticate: Basic realm="rTorrent"\r\n\r\n'
	end
	local sock = socket.connect(SCGI_ADDRESS, SCGI_PORT)
	if sock ~= nil then
		sock:send(scgi.netstring(xml))
		local err, code, headers, body = scgi.receive(sock)
		if tonumber(code) == 200 then
			return 'Status: 200 OK\r\n'
				.. 'Content-Type: application/xml\r\n\r\n' .. body
		end
	end
	return 'Status: 500 Internal Server Error\r\n\r\n'
end

function multicall(method_type, filter, ...)
	local res = (method_type == "d.")
		and call(method_type .. "multicall2", "", filter, unpack(alter(method_type, {...}, "=")))
		or call(method_type .. "multicall", filter, unpack(alter(method_type, {...}, "=")))
	return format(method_type, res, {...})
end

function batchcall(methods, params, prefix, postfix)
	local p = type(params) == "table" and params or { params }
	local methods_array = {}
	for _, m in ipairs(alter(prefix, methods, postfix)) do
		table.insert(methods_array, {
			["methodName"] = m,
			["params"] = xmlrpc.newTypedValue(p, "array")
		})
	end
	local res = {}
	for i, r in ipairs(call("system.multicall", xmlrpc.newTypedValue(methods_array, "array"))) do
		res[methods[i]] = r[1]
	end
	return res
end

