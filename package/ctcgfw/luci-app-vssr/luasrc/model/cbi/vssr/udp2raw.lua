-- Copyright (C) 2017 yushi studio <ywb94@qq.com> github.com/ywb94
-- Copyright (C) 2018 lean <coolsnowwolf@gmail.com> github.com/coolsnowwolf
-- Licensed to the public under the GNU General Public License v3.

local m, s, sec, o, kcp_enable
local vssr = "vssr"


m = Map(vssr)

local server_table = {}
local encrypt_methods = {
	"none",
	"table",
	"rc4",
	"rc4-md5-6",
	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",	
	"bf-cfb",
	"camellia-128-cfb",
	"camellia-192-cfb",
	"camellia-256-cfb",
	"cast5-cfb",
	"des-cfb",
	"idea-cfb",
	"rc2-cfb",
	"seed-cfb",
	"salsa20",
	"chacha20",
	"chacha20-ietf",
}

local protocol = {
	"origin",
	"verify_deflate",		
	"auth_sha1_v4",
	"auth_aes128_sha1",
	"auth_aes128_md5",
	"auth_chain_a",
	"auth_chain_b",
	"auth_chain_c",
	"auth_chain_d",
	"auth_chain_e",
	"auth_chain_f",
}

obfs = {
	"plain",
	"http_simple",
	"http_post",
	"random_head",	
	"tls1.2_ticket_auth",
}

local raw_mode = {
	"faketcp",
	"udp",
	"icmp",
}

local seq_mode = {
	"0",
	"1",
	"2",
	"3",
	"4",
}

local cipher_mode = {
	"none",
	"xor",
	"aes128cbc",
}

local auth_mode = {
	"none",
	"simple",
	"md5",
	"crc32",
}

local speeder_mode = {
	"0",
	"1",
}




-- [[ udp2raw ]]--
if nixio.fs.access("/usr/bin/udp2raw") then

s = m:section(TypedSection, "udp2raw", translate(" UDP2raw  "))
s.anonymous = true

o = s:option(Flag, "udp2raw_enable", translate("Enable udp2raw"))
o.default = 0
o.rmempty = false

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "key", translate("Password"))
o.password = true
o.rmempty = false

o = s:option(ListValue, "raw_mode", translate("Raw Mode"))
for _, v in ipairs(raw_mode) do o:value(v) end
o.default = "faketcp"
o.rmempty = false

o = s:option(ListValue, "seq_mode", translate("Seq Mode"))
for _, v in ipairs(seq_mode) do o:value(v) end
o.default = "3"
o.rmempty = false

o = s:option(ListValue, "cipher_mode", translate("Cipher Mode"))
for _, v in ipairs(cipher_mode) do o:value(v) end
o.default = "xor"
o.rmempty = false

o = s:option(ListValue, "auth_mode", translate("Auth Mode"))
for _, v in ipairs(auth_mode) do o:value(v) end
o.default = "simple"
o.rmempty = false

end

-- [[ udpspeeder ]]--
if nixio.fs.access("/usr/bin/udpspeeder") then

s = m:section(TypedSection, "udpspeeder", translate("UDPspeeder"))
s.anonymous = true

o = s:option(Flag, "udpspeeder_enable", translate("Enable UDPspeeder"))
o.default = 0
o.rmempty = false

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "key", translate("Password"))
o.password = true
o.rmempty = false

o = s:option(ListValue, "speeder_mode", translate("Speeder Mode"))
for _, v in ipairs(speeder_mode) do o:value(v) end
o.default = "0"
o.rmempty = false

o = s:option(Value, "fec", translate("Fec"))
o.default = "20:10"
o.rmempty = false

o = s:option(Value, "mtu", translate("Mtu"))
o.datatype = "uinteger"
o.default = 1250
o.rmempty = false

o = s:option(Value, "queue_len", translate("Queue Len"))
o.datatype = "uinteger"
o.default = 200
o.rmempty = false

o = s:option(Value, "timeout", translate("Fec Timeout"))
o.datatype = "uinteger"
o.default = 8
o.rmempty = false

end



return m
