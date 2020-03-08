-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local rtorrent = require "rtorrent"
local common = require "luci.model.cbi.rtorrent.common"

local hash = arg[1]
local details = rtorrent.batchcall({"name", "custom1", "timestamp.started", "timestamp.finished"}, hash, "d.")

f = SimpleForm("rtorrent", details["name"])
f.redirect = luci.dispatcher.build_url("admin/rtorrent/main")

t = f:section(Table, list)
t.template = "rtorrent/list"
t.pages = common.get_torrent_pages(hash)
t.page = "Info"

hash_id = f:field(DummyValue, "hash", "Hash")
function hash_id.cfgvalue(self, section)
	return hash
end

started = f:field(DummyValue, "started", "Download started")
started.value = details["timestamp.started"] == 0
	and "not yet started"
	or os.date("!%Y-%m-%d %H:%M:%S", details["timestamp.started"])

finished = f:field(DummyValue, "finished", "Download finished")
finished.value = details["timestamp.finished"] == 0
	and "not yet finished"
	or os.date("!%Y-%m-%d %H:%M:%S", details["timestamp.finished"])

tags = f:field(Value, "tags", "Tags")
tags.default = details["custom1"]
tags.rmempty = false

function tags.write(self, section, value)
	rtorrent.call("d.custom1.set", hash, value)
end

function f.handle(self, state, data)    
	if state == FORM_VALID then
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/info/") .. hash)
	end
	return true
end

return f

