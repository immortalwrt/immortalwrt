-- Copyright 2011 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.tools.status", package.seeall)

local uci = require "luci.model.uci".cursor()
local ipc = require "luci.ip"

local function dhcp_leases_common(family)
	local rv = { }
	local nfs = require "nixio.fs"
	local sys = require "luci.sys"
	local leasefile = "/tmp/dhcp.leases"

	uci:foreach("dhcp", "dnsmasq",
		function(s)
			if s.leasefile and nfs.access(s.leasefile) then
				leasefile = s.leasefile
				return false
			end
		end)

	local fd = io.open(leasefile, "r")
	if fd then
		while true do
			local ln = fd:read("*l")
			if not ln then
				break
			else
				local ts, mac, ip, name, duid = ln:match("^(%d+) (%S+) (%S+) (%S+) (%S+)")
				local expire = tonumber(ts) or 0
				if ts and mac and ip and name and duid then
					if family == 4 and not ip:match(":") then
						rv[#rv+1] = {
							expires  = (expire ~= 0) and os.difftime(expire, os.time()),
							macaddr  = ipc.checkmac(mac) or "00:00:00:00:00:00",
							ipaddr   = ip,
							hostname = (name ~= "*") and name
						}
					elseif family == 6 and ip:match(":") then
						rv[#rv+1] = {
							expires  = (expire ~= 0) and os.difftime(expire, os.time()),
							ip6addr  = ip,
							duid     = (duid ~= "*") and duid,
							hostname = (name ~= "*") and name
						}
					end
				end
			end
		end
		fd:close()
	end

	local lease6file = "/tmp/hosts/odhcpd"
	uci:foreach("dhcp", "odhcpd",
		function(t)
			if t.leasefile and nfs.access(t.leasefile) then
				lease6file = t.leasefile
				return false
			end
		end)
	local fd = io.open(lease6file, "r")
	if fd then
		while true do
			local ln = fd:read("*l")
			if not ln then
				break
			else
				local iface, duid, iaid, name, ts, id, length, ip = ln:match("^# (%S+) (%S+) (%S+) (%S+) (-?%d+) (%S+) (%S+) (.*)")
				local expire = tonumber(ts) or 0
				if ip and iaid ~= "ipv4" and family == 6 then
					rv[#rv+1] = {
						expires  = (expire >= 0) and os.difftime(expire, os.time()),
						duid     = duid,
						ip6addr  = ip,
						hostname = (name ~= "-") and name
					}
				elseif ip and iaid == "ipv4" and family == 4 then
					rv[#rv+1] = {
						expires  = (expire >= 0) and os.difftime(expire, os.time()),
						macaddr  = sys.net.duid_to_mac(duid) or "00:00:00:00:00:00",
						ipaddr   = ip,
						hostname = (name ~= "-") and name
					}
				end
			end
		end
		fd:close()
	end

	if family == 6 then
		local _, lease
		local hosts = sys.net.host_hints()
		for _, lease in ipairs(rv) do
			local mac = sys.net.duid_to_mac(lease.duid)
			local host = mac and hosts[mac]
			if host then
				if not lease.name then
					lease.host_hint = host.name or host.ipv4 or host.ipv6
				elseif host.name and lease.hostname ~= host.name then
					lease.host_hint = host.name
				end
			end
		end
	end

	return rv
end

function dhcp_leases()
	return dhcp_leases_common(4)
end

function dhcp6_leases()
	return dhcp_leases_common(6)
end

function wifi_networks()
	local rv = { }
	local ntm = require "luci.model.network".init()

	local dev
	for _, dev in ipairs(ntm:get_wifidevs()) do
		local rd = {
			up       = dev:is_up(),
			device   = dev:name(),
			name     = dev:get_i18n(),
			networks = { }
		}

		local net
		for _, net in ipairs(dev:get_wifinets()) do
			local a, an = nil, 0
			for _, a in pairs(net:assoclist() or {}) do
				an = an + 1
			end

			rd.networks[#rd.networks+1] = {
				name       = net:shortname(),
				link       = net:adminlink(),
				up         = net:is_up(),
				mode       = net:active_mode(),
				ssid       = net:active_ssid(),
				bssid      = net:active_bssid(),
				encryption = net:active_encryption(),
				frequency  = net:frequency(),
				channel    = net:channel(),
				signal     = net:signal(),
				quality    = net:signal_percent(),
				noise      = net:noise(),
				bitrate    = net:bitrate(),
				ifname     = net:ifname(),
				country    = net:country(),
				txpower    = net:txpower(),
				txpoweroff = net:txpower_offset(),
				num_assoc  = an,
				disabled   = (dev:get("disabled") == "1" or
				             net:get("disabled") == "1")
			}
		end

		rv[#rv+1] = rd
	end

	return rv
end

function wifi_network(id)
	local ntm = require "luci.model.network".init()
	local net = ntm:get_wifinet(id)
	if net then
		local dev = net:get_device()
		if dev then
			return {
				id         = id,
				name       = net:shortname(),
				link       = net:adminlink(),
				up         = net:is_up(),
				mode       = net:active_mode(),
				ssid       = net:active_ssid(),
				bssid      = net:active_bssid(),
				encryption = net:active_encryption(),
				frequency  = net:frequency(),
				channel    = net:channel(),
				signal     = net:signal(),
				quality    = net:signal_percent(),
				noise      = net:noise(),
				bitrate    = net:bitrate(),
				ifname     = net:ifname(),
				country    = net:country(),
				txpower    = net:txpower(),
				txpoweroff = net:txpower_offset(),
				disabled   = (dev:get("disabled") == "1" or
				              net:get("disabled") == "1"),
				device     = {
					up     = dev:is_up(),
					device = dev:name(),
					name   = dev:get_i18n()
				}
			}
		end
	end
	return { }
end

function wifi_assoclist()
	local sys = require "luci.sys"
	local ntm = require "luci.model.network".init()
	local hosts = sys.net.host_hints()

	local assoc = {}
	local _, dev, net, bss

	for _, dev in ipairs(ntm:get_wifidevs()) do
		local radioname = dev:get_i18n()

		for _, net in ipairs(dev:get_wifinets()) do
			local netname = net:shortname()
			local netlink = net:adminlink()
			local ifname  = net:ifname()

			for _, bss in pairs(net:assoclist() or {}) do
				local host = hosts[_]

				bss.bssid  = _
				bss.ifname = ifname
				bss.radio  = radioname
				bss.name   = netname
				bss.link   = netlink

				bss.host_name = (host) and (host.name or host.ipv4 or host.ipv6)
				bss.host_hint = (host and host.name and (host.ipv4 or host.ipv6)) and (host.ipv4 or host.ipv6)

				assoc[#assoc+1] = bss
			end
		end
	end

	table.sort(assoc, function(a, b)
		if a.radio ~= b.radio then
			return a.radio < b.radio
		elseif a.ifname ~= b.ifname then
			return a.ifname < b.ifname
		else
			return a.bssid < b.bssid
		end
	end)

	return assoc
end