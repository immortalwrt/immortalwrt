-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local rtorrent = require "rtorrent"
local nixio = require "nixio"
local common = require "luci.model.cbi.rtorrent.common"

local hash = arg[1]
local details = rtorrent.batchcall({"name", "base_path"}, hash, "d.")
local files = rtorrent.multicall("f.", hash, 0, "path", "path_depth", "path_components", "size_bytes",
	"size_chunks", "completed_chunks", "priority", "frozen_path")

local format, total = {}, {}

function format.icon(r, v)
	local icon_path = "/luci-static/resources/icons/filetypes"
	local ext = v:match("%.([^%.]+)$")
	if ext and nixio.fs.stat("/www/%s/%s.png" % {icon_path, ext:lower()}, "type") then
		return "%s/%s.png" % {icon_path, ext:lower()}
	end
	return "%s/%s.png" % {icon_path, "file"}
end

function format.dir(r, v)
	return "<img style=\"vertical-align: text-top;\" src=\"/luci-static/resources/icons/filetypes/dir.png\" /> " .. v
end

function format.file(r, v)
	total["name"] = (total["name"] or 0) + 1
	local url = luci.dispatcher.build_url("admin/rtorrent/download/") .. nixio.bin.b64encode(r["frozen_path"])
	local link = r["chunks_percent"] == 100 and "<a href=\"" .. url .. "\">" .. v .. "</a>" or v
	return "<img style=\"vertical-align: middle;\" src=\"" .. format["icon"](r, v) .. "\" /> " .. link
end

function format.size_bytes(r, v)
	total["size_bytes"] = (total["size_bytes"] or 0) + v
	return common.human_size(v)
end

function format.chunks_percent(r, v)
	return common.div(string.format("%.1f%%", v), v < 100 and "red")
end

function format.priority(r, v)
	return tostring(v)
end

function add_custom(files)
	for i, r in ipairs(files) do
		r["id"] = i
		r["chunks_percent"] = r["completed_chunks"] * 100.0 / r["size_chunks"]
	end
end

function add_summary(list)
 	table.insert(list, {
 		["name"] = "TOTAL: " .. total["name"] .. " pcs.",
 		["size_bytes"] = common.human_size(total["size_bytes"]),
		["priority"] = "%hidden%"
 	})
end

function path_compare(a, b)
	if a["path_depth"] ~= b["path_depth"] and (a["path_depth"] == 1 or b["path_depth"] == 1) then
		return a["path_depth"] > b["path_depth"]
	end
	return a["path"] < b["path"]
end

local list, last_path = {}, {}
add_custom(files)
table.sort(files, path_compare)
for _, r in ipairs(files) do
	for i, p in ipairs(r["path_components"]) do
		if last_path[i] ~= p then
			local t = i == #r["path_components"] and "file" or "dir"
			local n = {}
			if t == "file" then
				for m, v in pairs(r) do
					n[m] = format[m] and format[m](r, v) or tostring(v)
				end
			else
				n["priority"] = "%hidden%"
			end
			n["name"] = string.rep("&emsp;", i - 1) .. format[t](r, p)
			table.insert(list, n)
		end
		last_path[i] = p
	end
end

f = SimpleForm("rtorrent", details["name"])
f.redirect = luci.dispatcher.build_url("admin/rtorrent/main")
if nixio.fs.stat(details["base_path"], "type") == "dir" and table.getn(list) > 1 then
	f.cancel = "Download all"
else
	f.cancel = false
end

if #list > 1 then add_summary(list) end
t = f:section(Table, list)
t.template = "rtorrent/list"
t.pages = common.get_torrent_pages(hash)
t.page = "File List"

AbstractValue.tooltip = function(self, s) self.hint = s return self end

t:option(DummyValue, "name", "Name").rawhtml = true
t:option(DummyValue, "size_bytes", "Size")
t:option(DummyValue, "chunks_percent", "Done"):tooltip("Download done percent").rawhtml = true
prio = t:option(ListValue, "priority", "Priority"):tooltip("Rotate priority")
prio.template = "rtorrent/lvalue"
prio.onclick = [[
	var inputs = document.getElementsByClassName("cbi-input-select");
	for (var i = 0; i < inputs.length; i++) {
		if (inputs[i].selectedIndex < inputs[i].length - 1) {
			inputs[i].selectedIndex++;
		} else {
			inputs[i].selectedIndex = 0;
		}
	}
]]
prio:value("0", "off")
prio:value("1", "normal")
prio:value("2", "high")

function prio.write(self, section, value)
	rtorrent.call("f.priority.set", hash .. ":f" .. (list[tonumber(section)].id - 1), tonumber(value))
	luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/files/%s" % hash))
end

function f:on_cancel()
	luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/downloadall/")
		.. nixio.bin.b64encode(details["base_path"]))
end

return f

