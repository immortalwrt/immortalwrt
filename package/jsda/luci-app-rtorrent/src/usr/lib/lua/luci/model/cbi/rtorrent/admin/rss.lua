-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

local common = require "luci.model.cbi.rtorrent.common"
local nixio = require "nixio"

m = Map("rtorrent", "Admin - RSS Downloader")

s = m:section(TypedSection, "rss-feed")
s.addremove = true
s.anonymous = true
s.sortable = true
s.template = "cbi/tblsection"
s.render = function(self, section, scope)
	luci.template.render("rtorrent/tabmenu", { self = {
		pages = common.get_admin_pages(),
		page = "RSS"
	}})
	TypedSection.render(self, section, scope)
end

name = s:option(Value, "name", "Name")
name.rmempty = false

url = s:option(Value, "url", "RSS Feed URL")
url.size = "65"
url.rmempty = false

enabled = s:option(Flag, "enabled", "Enabled")
enabled.rmempty = false

t = m:section(NamedSection, "logging", "rss", "Logging")

feed_logging = t:option(Flag, "feed_logging", "Enable RSS feed logging")

feed_logfile = t:option(Value, "feed_logfile", "RSS feed logfile")
feed_logfile:depends("feed_logging", 1)

function feed_logfile.validate(self, value, section)
	local parent_folder = nixio.fs.dirname(value)
	if parent_folder == "." or nixio.fs.stat(parent_folder, "type") ~= "dir" then
		return nil, "Wrong filename, please use absolute path!"
	end
	return value
end

return m

