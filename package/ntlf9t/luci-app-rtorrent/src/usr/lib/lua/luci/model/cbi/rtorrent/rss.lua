-- Copyright 2014-2018 Sandor Balazsi <sandor.balazsi@gmail.com>
-- Licensed to the public under the GNU General Public License.

m = Map("rtorrent", "RSS Downloader")

s = m:section(TypedSection, "rss-rule")
s.addremove = true
s.anonymous = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/rtorrent/rss/%s")
s.template_addremove = "rtorrent/rss_addrule"

function s.parse(self, ...)
	TypedSection.parse(self, ...)

	local newrule_name = m:formvalue("_newrule.name")
	local newrule_submit = m:formvalue("_newrule.submit")

	if newrule_submit then
		newrule = TypedSection.create(self, section)
		self.map:set(newrule, "name", newrule_name)

		m.uci:save("rtorrent")
		luci.http.redirect(luci.dispatcher.build_url("admin/rtorrent/rss", newrule))
	end
end

name = s:option(DummyValue, "name", "Name")
name.width = "30%"

match = s:option(DummyValue, "match", "Match")
match.width = "60%"

enabled = s:option(Flag, "enabled", "Enabled")
enabled.rmempty = false

return m

