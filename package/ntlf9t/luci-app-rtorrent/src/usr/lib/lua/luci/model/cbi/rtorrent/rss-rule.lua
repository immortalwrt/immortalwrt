-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local rtorrent = require "rtorrent"

m = Map("rtorrent")
m.redirect = luci.dispatcher.build_url("admin/rtorrent/rss")

local name = m:get(arg[1], "name")
m.title = "%s - %s" %{"RSS Downloader", name or "(Unnamed rule)"}

s = m:section(NamedSection, arg[1], "rss-rule")
s.anonymous = true
s.addremove = false

enabled = s:option(Flag, "enabled", "Enabled")
enabled.rmempty = false

name = s:option(Value, "name", "Name")
name.rmempty = false

match = s:option(TextValue, "match", "Match (lua regex)")
match.template = "rtorrent/tvalue"
match.rmempty = false
match.rows = 1

exclude = s:option(TextValue, "exclude", "Exclude (lua regex)")
exclude.rows = 1

s:option(Value, "minsize", "Min size (MiB):")
s:option(Value, "maxsize", "Max size (MiB):")

feed = s:option(MultiValue, "feed", "Feed")
feed.delimiter = ";"
m.uci:foreach(m.config, "rss-feed", function(f)
	feed:value(f.name, f.name)
end)

tags = s:option(Value, "tags", "Add tags")
tags.default = "all"

destdir = s:option(Value, "destdir", "Download directory")
destdir.default = rtorrent.call("directory.default")
destdir.datatype = "directory"
destdir.rmempty = false

autostart = s:option(Flag, "autostart", "Start download")
autostart.default  = "1"
autostart.rmempty = false

return m

