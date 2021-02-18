module("luci.controller.wrtbwmon", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/wrtbwmon") then
		return
	end
	entry({"admin", "network", "usage"},
		alias("admin", "network", "usage", "details"),
		 _("Traffic Status"), 60)
	entry({"admin", "network", "usage", "details"},
		template("wrtbwmon"),
		_("Details"), 10).leaf=true
	entry({"admin", "network", "usage", "config"},
		arcombine(cbi("wrtbwmon/config")),
		_("Configuration"), 20).leaf=true
	entry({"admin", "network", "usage", "custom"},
		form("wrtbwmon/custom"),
		_("User file"), 30).leaf=true
end

