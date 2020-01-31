-- Copyright 2003-2010 Kepler Project
-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- XML-RPC over SCGI.

local error, tonumber, tostring, unpack = error, tonumber, tostring, unpack

local socket= require"socket"
local string= require"string"
local xmlrpc= require"xmlrpc"

module("xmlrpc.scgi")

---------------------------------------------------------------------
-- Call a remote method.
-- @param addr String with the address of the SCGI server.
-- @param port The port of the SCGI server.
-- @param method String with the name of the method to be called.
-- @return Table with the response(could be a `fault' or a `params'
--	XML-RPC element).
---------------------------------------------------------------------
function call(addr, port, method, ...)
	local request_body = xmlrpc.clEncode(method, ...)
	local sock = socket.connect(addr, port)
	if sock == nil then
		return false, "socket connect failed"
	end
	sock:send(netstring(request_body))
	local err, code, headers, body = receive(sock)

	if tonumber(code) == 200 then
		return xmlrpc.clDecode(body)
	else
		error(tostring(err or code))
	end
end

---------------------------------------------------------------------
-- Encode message as netstring
-- @param request_body String with the message
-- @return String with the encoded message
---------------------------------------------------------------------
function netstring(request_body)
	local null = "\0"
	local content_length = "CONTENT_LENGTH" .. null .. string.len(request_body) .. null
	local scgi_enable = "SCGI" .. null .. "1" .. null
	local request_method = "REQUEST_METHOD" .. null .. "POST" .. null
	local server_protocol = "SERVER_PROTOCOL" .. null .. "HTTP/1.1" .. null
	local header = content_length .. scgi_enable .. request_method .. server_protocol
	return string.len(header) .. ":" .. header .. "," .. request_body
end

---------------------------------------------------------------------
-- Receive and parse socket response
-- @param sock Socket instance
-- @return Headers, body and error codes
---------------------------------------------------------------------
function receive(sock)
	local line, body, err
	local headers = {}

	line, err = sock:receive()
	if err then return err, "500" end
	while line ~= "" do
		local name, value = socket.skip(2, string.find(line, "^(.-):%s*(.*)"))
		if not(name and value) then return "malformed reponse header: " .. line, "500" end
		headers[string.lower(name)] = value

		line, err = sock:receive()
		if err then return err, "500" end
	end

	body = sock:receive(headers["content-length"])
	local code = socket.skip(2, string.find(headers["status"], "^(%d%d%d)"))
	return err, code, headers, body
end

