-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local rtorrent = require "rtorrent"
local common = require "luci.model.cbi.rtorrent.common"

local hash = arg[1]
local details = rtorrent.batchcall({"name"}, hash, "d.")

local format, total = {}, {}

function format.url(r, v)
	total["url"] = (total["url"] or 0) + 1
end

function format.scrape_downloaded(r, v)
	total["scrape_downloaded"] = (total["scrape_downloaded"] or 0) + v
	return tostring(v)
end

function format.scrape_complete(r, v)
	total["scrape_complete"] = (total["scrape_complete"] or 0) + v
	return tostring(v)
end

function format.scrape_incomplete(r, v)
	total["scrape_incomplete"] = (total["scrape_incomplete"] or 0) + v
	return tostring(v)
end

function format.scrape_time_last(r, v)
	return common.human_time(os.time() - v)
end

function add_summary(list)
 	table.insert(list, {
 		["url"] = "TOTAL: " .. total["url"] .. " pcs.",
		["scrape_downloaded"] = tostring(total["scrape_downloaded"]),
		["scrape_complete"] = tostring(total["scrape_complete"]),
		["scrape_incomplete"] = tostring(total["scrape_incomplete"]),
		["is_enabled"] = "%hidden%"
 	})
end

local list = rtorrent.multicall("t.", hash, 0, "is_enabled", "url", "scrape_downloaded", "scrape_complete",
	"scrape_incomplete", "scrape_time_last")

for _, r in ipairs(list) do
	for k, v in pairs(r) do
		r[k] = format[k] and format[k](r, v) or tostring(v)
	end
end

f = SimpleForm("rtorrent", details["name"])
f.redirect = luci.dispatcher.build_url("admin/rtorrent/main")

if #list > 1 then add_summary(list) end
t = f:section(Table, list)
t.template = "rtorrent/list"
t.pages = common.get_torrent_pages(hash)
t.page = "Tracker List"

AbstractValue.tooltip = function(self, s) self.hint = s return self end

t:option(DummyValue, "url", "Url")
t:option(DummyValue, "scrape_downloaded", "D"):tooltip("Downloaded")
t:option(DummyValue, "scrape_complete", "S"):tooltip("Seeders")
t:option(DummyValue, "scrape_incomplete", "L"):tooltip("Leechers")
t:option(DummyValue, "scrape_time_last", "Updated"):tooltip("Last update time").rawhtml = true

enabled = t:option(Flag, "is_enabled", "Enabled")
enabled.template = "rtorrent/fvalue"
enabled.rmempty = false
enabled.rawhtml = true

function enabled.write(self, section, value)
	if value ~= tostring(list[section].is_enabled) then
		rtorrent.call("t.is_enabled.set", hash .. ":t" .. (section - 1), tonumber(value))
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/trackers/%s" % hash))
	end
end

add = f:field(Value, "add_tracker", "Add tracker")
function add.write(self, section, value)
	rtorrent.call("d.tracker.insert", hash, table.getn(list), value)
	luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/trackers/%s" % hash))
end

return f

