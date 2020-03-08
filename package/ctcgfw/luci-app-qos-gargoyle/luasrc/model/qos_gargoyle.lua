-- Copyright 2017 Xingwang Liao <kuoruan@gmail.com>
-- Licensed to the public under the Apache License 2.0.

module("luci.model.qos_gargoyle", package.seeall)

function has_ndpi()
	return luci.sys.call("lsmod | cut -d ' ' -f1 | grep -q 'xt_ndpi'") == 0
end

function cbi_add_dpi_protocols(field)
	local util = require "luci.util"

	local dpi_protocols = {}

	for line in util.execi("iptables -m ndpi --help 2>/dev/null | grep '^--'") do
		local _, _, protocol, name = line:find("%-%-([^%s]+) Match for ([^%s]+)")

		if protocol and name then
			dpi_protocols[protocol] = name
		end
	end

	for p, n in util.kspairs(dpi_protocols) do
		field:value(p, n)
	end
end

function get_wan()
	local network = require "luci.model.network".init()
	local bundle = network:get_status_by_route("0.0.0.0", 0)
	local net, stat
	for k, v in pairs(bundle) do
		net = k
		stat = v
		break
	end
	return net and network:network(net, stat.proto)
end
