local ucursor = require "luci.model.uci".cursor()
local json = require "luci.jsonc"
local proto = arg[1] 
local local_port = arg[2]
local Socks_user = arg[3]
local Socks_pass = arg[4]


local v2ray = {
	log = {
		--error = "/var/log/v2ray.log",
		loglevel = "warning"
	},
	-- 传入连接
	inbound = {
		port = local_port,
		protocol = proto,
		settings = {
			auth = "password",
			accounts = {
				{
					user = Socks_user,
					pass = Socks_pass
				}
			}
		}
	},
	-- 传出连接
	outbound = {
		protocol = "freedom"
	},
	-- 额外传出连接
	outboundDetour = {
		{
			protocol = "blackhole",
			tag = "blocked"
		}
	}
}
print(json.stringify(v2ray,1))