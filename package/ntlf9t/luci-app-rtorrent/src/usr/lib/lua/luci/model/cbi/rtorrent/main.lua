-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

-- Custom fields:
-- d.custom1: tags (space delimited) 
-- d.custom2: tracker favicon 
-- d.custom3: url of torrent file  
-- d.custom4: not used
-- d.custom5: when "1": delete files from disk on erase

local common = require "luci.model.cbi.rtorrent.common"
local rtorrent = require "rtorrent"
require "luci.model.cbi.rtorrent.string"

local selected, format, total = {}, {}, {}

local methods = { "hash", "name", "size_bytes", "bytes_done", "hashing", "state", "complete",
	"peers_accounted", "peers_complete", "down.rate", "up.rate", "ratio", "up.total", 
	"timestamp.started", "timestamp.finished", "custom1", "custom2" }

function status(d)
	if     d["hashing"] > 0 then return "hash"
	elseif d["state"] == 0 then return "stop"
	elseif d["state"] > 0 then
		if d["complete"] == 0 then return "down"
		else return "seed" end
	else return "unknown" end
end

function eta(d)
	if d["bytes_done"] < d["size_bytes"] then
		if d["down_rate"] > 0 then
			return common.human_time((d["size_bytes"] - d["bytes_done"]) / d["down_rate"])
		else return "&#8734;" end
	else return "--" end
end

function favicon(d)
	if not d["custom2"] or d["custom2"] == "" then
		d["custom2"] = "/luci-static/resources/icons/unknown_tracker.png"
		for _, t in pairs(rtorrent.multicall("t.", d["hash"], 0, "url", "is_enabled")) do
			if t["is_enabled"] then
				local domain = t["url"]:match("[%w%.:/]*[%./](%w+%.%w+)")
				if domain then
					local icon = "http://" .. domain .. "/favicon.ico"
					if common.get(icon) then
						d["custom2"] = icon
						break
					end
				end
			end
		end
		rtorrent.call("d.custom2.set", d["hash"], d["custom2"])
	end
 	return d["custom2"]
end

function has_tag(tags, tag)
	for _, t in ipairs(tags) do
		if t.name:lower() == tag:lower() then return true end
	end
	return false
end

function get_tags(details)
	local l = {}
	local has_incomplete = false
	for _, d in ipairs(details) do
		for _, p in ipairs(d["custom1"]:split()) do
			if not has_tag(l, p) then
				table.insert(l, {name = p:ucfirst(), link = luci.dispatcher.build_url("admin/rtorrent/main/%s" % p)})
			end
		end
		if d["complete"] == 0 then
			has_incomplete = true
		end
	end
	if has_incomplete then
		table.insert(l, {name = "Incomplete", link = luci.dispatcher.build_url("admin/rtorrent/main/incomplete")})
	end
	return l
end

function filter(details, page)
	local filtered = {}
	for _, d in ipairs(details) do
		if string.find(" " .. d["custom1"] .. " ", " " .. page .. " ") then
			table.insert(filtered, d)
		end
		if page == "incomplete" and d["complete"] == 0 then
			table.insert(filtered, d)
		end
	end
	return filtered
end

function format.icon(d, v)
	return "<img src=\"" .. v .. "\" />"
end

function format.name(d, v)
	total["name"] = (total["name"] or 0) + 1
	local url = luci.dispatcher.build_url("admin/rtorrent/files/" .. d["hash"])
	return "<a href=\"%s\">%s</a>" % {url, v}
end

function format.size_bytes(d, v)
	total["size_bytes"] = (total["size_bytes"] or 0) + v
	return "<div title=\"%s B\">%s</div>" % {v, common.human_size(v)}
end

function format.done_percent(d, v)
	return string.format("%.1f%%", v)
end

function format.status(d, v)
	return common.div(v, v == "stop" and "red", v == "seed" and "blue",
		v == "down" and "green", v == "hash" and "green")
end

function format.down_rate(d, v)
	total["down_rate"] = (total["down_rate"] or 0) + v
	return string.format("%.2f", v / 1000)
end

function format.up_rate(d, v)
	total["up_rate"] = (total["up_rate"] or 0) + v
	return string.format("%.2f", v / 1000)
end

function format.ratio(d, v)
	return common.div(string.format("%.2f", v / 1000), v < 1000 and "red" or "green")
	--	"title: Total uploaded: " .. common.human_size(d["up_total"]))
end

function format.eta(d, v)
	local download_started = d["timestamp_started"] == 0
		and "not yet started" or os.date("!%Y-%m-%d %H:%M:%S", d["timestamp_started"])
	local download_finished = d["timestamp_finished"] == 0
		and "not yet finished" or os.date("!%Y-%m-%d %H:%M:%S", d["timestamp_finished"])
	return "<div title=\"Download started: %s&#13;Download finished: %s\">%s</div>" % {
		download_started, download_finished, v }
end

function add_custom(details)
	for _, d in ipairs(details) do
		-- refactor: swap favicon (custom1) and tags (custom2)
		if d["custom1"]:ends(".ico") or d["custom1"]:ends(".png") then
			local temp = d["custom1"]
			d["custom1"] = d["custom2"]
			d["custom2"] = temp
			rtorrent.call("d.custom1.set", d["hash"], d["custom1"])
			rtorrent.call("d.custom2.set", d["hash"], d["custom2"])
		end
		d["status"] = status(d)
		d["done_percent"] = 100.0 * d["bytes_done"] / d["size_bytes"]
		d["eta"] = eta(d)
		d["icon"] = favicon(d)
		d["custom1"] = (d["custom1"] == "") and "all" or d["custom1"]
	end
end

function add_summary(details)
 	table.insert(details, {
 		["name"] = "TOTAL: " .. total["name"] .. " pcs.",
 		["size_bytes"] = common.human_size(total["size_bytes"]),
 		["down_rate"] = string.format("%.2f", total["down_rate"] / 1000),
 		["up_rate"] =  string.format("%.2f", total["up_rate"] / 1000),
 		["select"] = "%hidden%"
 	})
end

function html_format(details)
	table.sort(details, function(a, b) return a["name"] < b["name"] end)
	for _, d in ipairs(details) do
		for m, v in pairs(d) do
			d[m] = format[m] and format[m](d, v) or tostring(v)
		end
	end
end

f = SimpleForm("rtorrent", "Torrent List")
f.reset = false
f.submit = false

local details = rtorrent.multicall("d.", "default", unpack(methods))
add_custom(details)
local tags = get_tags(details)
local user = luci.dispatcher.context.authuser
local page = arg[1] or (has_tag(tags, user) and user or "all")
local filtered = filter(details, page)

html_format(filtered)
if #filtered > 1 then add_summary(filtered) end
t = f:section(Table, filtered)
t.template = "rtorrent/list"
t.pages = tags
t.page = page
t.headcol = 2

AbstractValue.tooltip = function(self, s) self.hint = s return self end

t:option(DummyValue, "icon").rawhtml = true
t:option(DummyValue, "name", "Name").rawhtml = true
t:option(DummyValue, "size_bytes", "Size"):tooltip("Full size of torrent").rawhtml = true
t:option(DummyValue, "done_percent", "Done"):tooltip("Download done percent").rawhtml = true
t:option(DummyValue, "status", "Status").rawhtml = true
t:option(DummyValue, "peers_accounted", "&uarr;"):tooltip("Seeder count").rawhtml = true
t:option(DummyValue, "peers_complete", "&darr;"):tooltip("Leecher count").rawhtml = true
t:option(DummyValue, "down_rate", "Down<br />Speed"):tooltip("Download speed in kb/s").rawhtml = true
t:option(DummyValue, "up_rate", "Up<br />Speed"):tooltip("Upload speed in kb/s").rawhtml = true
t:option(DummyValue, "ratio", "Ratio"):tooltip("Download/upload ratio").rawhtml = true
t:option(DummyValue, "eta", "ETA"):tooltip("Estimated Time of Arrival").rawhtml = true

select = t:option(Flag, "select")
select.template = "rtorrent/fvalue"

function select.write(self, section, value)
	table.insert(selected, filtered[section].hash)
end

s = f:section(SimpleSection)
s.template = "rtorrent/buttonsection"
s.style = "float: right;"

start = s:option(Button, "start", "start")
start.template = "rtorrent/button"
start.inputstyle = "apply"

function start.write(self, section, value)
	if next(selected) ~= nil then
		for _, hash in ipairs(selected) do
			rtorrent.call("d.open", hash)
			rtorrent.call("d.start", hash)
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/main/" .. page))
	end
end

stop = s:option(Button, "stop", "stop")
stop.template = "rtorrent/button"
stop.inputstyle = "reset"

function stop.write(self, section, value)
	if next(selected) ~= nil then
		for _, hash in ipairs(selected) do
			rtorrent.call("d.stop", hash)
			rtorrent.call("d.close", hash)
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/main/" .. page))
	end
end

remove = s:option(Button, "remove", "remove")
remove.template = "rtorrent/button"
remove.inputstyle = "remove"

function remove.write(self, section, value)
	if next(selected) ~= nil then
		for _, hash in ipairs(selected) do
			rtorrent.call("d.close", hash)
			rtorrent.call("d.erase", hash)
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/main/" .. page))
	end
end

r = f:section(SimpleSection)
r.template = "rtorrent/buttonsection"
r.style = "float: right;"

delete = r:option(Button, "delete", "remove and delete data")
delete.template = "rtorrent/button"
delete.inputstyle = "remove"

function delete.write(self, section, value)
	if next(selected) ~= nil then
		for _, hash in ipairs(selected) do
			rtorrent.call("d.custom5.set", hash, "1")
			rtorrent.call("d.close", hash)
			rtorrent.call("d.erase", hash)
		end
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/main/" .. page))
	end
end

return f

